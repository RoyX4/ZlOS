# MP-13: Turn the browser into an isolated and capable web platform

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/src/web/; kernel/src/net/; kernel/src/graphics/; kernel/apps/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-13` exports: Browser UI and restricted content/network/decoder workers running a declared real corpus.

The handoff enables only its named subset. `CLOSE-13` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-13.01 — Preserve current supported sites and offline fixtures as a versioned corpus with exact expected behavior

Preserve current supported sites and offline fixtures as a versioned corpus with exact expected behavior.

**Requires:** `D-01`, `D-02`, `D-18`, `H-00`, `H-08`, `H-10`, `H-11`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-13.02 — Split browser UI, networking, storage, content, script and decoding ownership behind restricted processes

Split browser UI, networking, storage, content, script and decoding ownership behind restricted processes.

**Requires:** `M-13.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-13 — Bounded development handoff: Turn the browser into an isolated and capable web platform

Browser UI and restricted content/network/decoder workers running a declared real corpus.

**Requires:** `M-13.02`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-13.03 — Implement URL/navigation, redirects, history, bookmarks, tabs, downloads and session recovery through real services

Implement URL/navigation, redirects, history, bookmarks, tabs, downloads and session recovery through real services.

**Requires:** `M-13.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-13.04 — Deepen HTML parsing, DOM, CSS cascade/layout, text, forms, painting and compositing using bounded data structures

Deepen HTML parsing, DOM, CSS cascade/layout, text, forms, painting and compositing using bounded data structures.

**Requires:** `M-13.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-13.05 — Define JavaScript/DOM/events, scheduling, cancellation and worker budgets

Define JavaScript/DOM/events, scheduling, cancellation and worker budgets; isolate parser/script failure.

**Requires:** `M-13.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-13.06 — Implement origin, cookie/storage/cache, CSP, certificate and site-permission policies before admitting hostile real content

Implement origin, cookie/storage/cache, CSP, certificate and site-permission policies before admitting hostile real content.

**Requires:** `M-13.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-13.07 — Add developer tools and accessibility views backed by actual document and network state

Add developer tools and accessibility views backed by actual document and network state.

**Requires:** `M-13.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-13.08 — Run malformed, oversized, reentrant, cross-origin and interrupted-load corpora with memory/latency budgets

Run malformed, oversized, reentrant, cross-origin and interrupted-load corpora with memory/latency budgets.

**Requires:** `M-13.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-13.09 — Promote individual web capabilities and workflows only after behavior, security and crash-containment evidence exists

Promote individual web capabilities and workflows only after behavior, security and crash-containment evidence exists.

**Requires:** `M-13.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-WB-001](#f-wb-001) | feature | browser chrome |
| [F-WB-002](#f-wb-002) | feature | URL parser |
| [F-WB-003](#f-wb-003) | feature | navigation broker |
| [F-WB-004](#f-wb-004) | feature | browser profile |
| [F-WB-005](#f-wb-005) | feature | content-process isolation |
| [F-WB-006](#f-wb-006) | feature | request/network service |
| [F-WB-007](#f-wb-007) | feature | decoder isolation |
| [F-WB-008](#f-wb-008) | feature | HTML parser |
| [F-WB-009](#f-wb-009) | feature | DOM |
| [F-WB-010](#f-wb-010) | feature | CSS parser/cascade |
| [F-WB-011](#f-wb-011) | feature | style engine |
| [F-WB-012](#f-wb-012) | feature | layout engine |
| [F-WB-013](#f-wb-013) | feature | web paint/display list |
| [F-WB-014](#f-wb-014) | feature | browser compositing |
| [F-WB-015](#f-wb-015) | feature | JavaScript engine |
| [F-WB-016](#f-wb-016) | feature | event loop |
| [F-WB-017](#f-wb-017) | feature | Web Workers |
| [F-WB-018](#f-wb-018) | feature | WebAssembly |
| [F-WB-019](#f-wb-019) | feature | Canvas 2D |
| [F-WB-020](#f-wb-020) | feature | SVG |
| [F-WB-021](#f-wb-021) | feature | MathML |
| [F-WB-022](#f-wb-022) | feature | forms/controls |
| [F-WB-023](#f-wb-023) | feature | focus/selection/clipboard |
| [F-WB-024](#f-wb-024) | feature | scrolling |
| [F-WB-025](#f-wb-025) | feature | cookies |
| [F-WB-026](#f-wb-026) | feature | local/session storage |
| [F-WB-027](#f-wb-027) | feature | IndexedDB-like storage |
| [F-WB-028](#f-wb-028) | feature | cache |
| [F-WB-029](#f-wb-029) | feature | CORS/referrer/origin policy |
| [F-WB-030](#f-wb-030) | feature | CSP/mixed-content policy |
| [F-WB-031](#f-wb-031) | feature | permissions model |
| [F-WB-032](#f-wb-032) | feature | sandboxed frames |
| [F-WB-033](#f-wb-033) | feature | downloads |
| [F-WB-034](#f-wb-034) | feature | uploads/file chooser |
| [F-WB-035](#f-wb-035) | feature | bookmarks/history |
| [F-WB-036](#f-wb-036) | feature | tabs/windows |
| [F-WB-037](#f-wb-037) | feature | private browsing |
| [F-WB-038](#f-wb-038) | feature | password/autofill integration |
| [F-WB-039](#f-wb-039) | feature | certificate/security UI |
| [F-WB-040](#f-wb-040) | feature | page accessibility |
| [F-WB-041](#f-wb-041) | feature | developer tools |
| [F-WB-042](#f-wb-042) | feature | reader mode |
| [F-WB-043](#f-wb-043) | feature | find in page |
| [F-WB-044](#f-wb-044) | feature | print/PDF output |
| [F-WB-045](#f-wb-045) | feature | browser performance telemetry |
| [F-WB-046](#f-wb-046) | feature | hostile-page recovery |
| [F-WB-047](#f-wb-047) | feature | agent-readable web tools |
| [F-WB-048](#f-wb-048) | feature | web compatibility ledger |
| [F-WB-049](#f-wb-049) | feature | browser extensions |
| [F-WB-050](#f-wb-050) | feature | installable web apps |
| [C-P6.4](#c-p6-4) | contract | browser process split and durable data |
| [C-DA-30](#c-da-30) | contract | network and content applications |
| [C-VX-26](#c-vx-26) | contract | Browser IPC and process topology |
| [C-VX-27](#c-vx-27) | contract | Navigation and history transaction |
| [C-VX-28](#c-vx-28) | contract | Browser network, trust, cache and storage |
| [C-VX-29](#c-vx-29) | contract | Site content runtime |
| [C-VX-30](#c-vx-30) | contract | Decoder worker family |
| [C-VX-31](#c-vx-31) | contract | Browser chrome and permission UX |
| [C-VX-32](#c-vx-32) | contract | Web compatibility ladder |
| [C-VX-33](#c-vx-33) | contract | Browser end-to-end and hostile-page gate |
| [T-CUR-005](#t-cur-005) | target | Browser |
| [T-APP-069](#t-app-069) | target | Download Manager |

<a id="f-wb-001"></a>
## F-WB-001 — browser chrome

**Original requirement:** address/search, back/forward/reload/stop, tabs, downloads, permissions and errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-001.01 — Reconcile existing browser chrome**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for browser chrome. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: address/search, back/forward/reload/stop, tabs, downloads, permissions and errors
- [ ] **F-WB-001.02 — Freeze the exact contract for browser chrome**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: address/search, back/forward/reload/stop, tabs, downloads, permissions and errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-001.03 — Implement/prove: address/search**
  - Action: For browser chrome, implement or reuse and verify this exact obligation: address/search. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for address/search; retain observable state/resource expectations.
- [ ] **F-WB-001.04 — Implement/prove: back/forward/reload/stop**
  - Action: For browser chrome, implement or reuse and verify this exact obligation: back/forward/reload/stop. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for back/forward/reload/stop; retain observable state/resource expectations.
- [ ] **F-WB-001.05 — Implement/prove: tabs**
  - Action: For browser chrome, implement or reuse and verify this exact obligation: tabs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tabs; retain observable state/resource expectations.
- [ ] **F-WB-001.06 — Implement/prove: downloads**
  - Action: For browser chrome, implement or reuse and verify this exact obligation: downloads. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for downloads; retain observable state/resource expectations.
- [ ] **F-WB-001.07 — Implement/prove: permissions and errors**
  - Action: For browser chrome, implement or reuse and verify this exact obligation: permissions and errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions and errors; retain observable state/resource expectations.
- [ ] **F-WB-001.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to browser chrome: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-001.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-001.09 — Integrate into the real consumer and runtime route**
  - Action: Wire browser chrome into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-001.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-001.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for browser chrome as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-001.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-002"></a>
## F-WB-002 — URL parser

**Original requirement:** standards-oriented checked parsing, normalization, schemes and display safety

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-002.01 — Reconcile existing URL parser**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for URL parser. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: standards-oriented checked parsing, normalization, schemes and display safety
- [ ] **F-WB-002.02 — Freeze the exact contract for URL parser**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: standards-oriented checked parsing, normalization, schemes and display safety. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-002.03 — Implement/prove: standards-oriented checked parsing**
  - Action: For URL parser, implement or reuse and verify this exact obligation: standards-oriented checked parsing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for standards-oriented checked parsing; retain observable state/resource expectations.
- [ ] **F-WB-002.04 — Implement/prove: normalization**
  - Action: For URL parser, implement or reuse and verify this exact obligation: normalization. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for normalization; retain observable state/resource expectations.
- [ ] **F-WB-002.05 — Implement/prove: schemes and display safety**
  - Action: For URL parser, implement or reuse and verify this exact obligation: schemes and display safety. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-002.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for schemes and display safety; retain observable state/resource expectations.
- [ ] **F-WB-002.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to URL parser: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-002.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-002.07 — Integrate into the real consumer and runtime route**
  - Action: Wire URL parser into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-002.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-002.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for URL parser as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-002.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-003"></a>
## F-WB-003 — navigation broker

**Original requirement:** committed history, redirects, replacement cancellation, policy and provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-003.01 — Reconcile existing navigation broker**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for navigation broker. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: committed history, redirects, replacement cancellation, policy and provenance
- [ ] **F-WB-003.02 — Freeze the exact contract for navigation broker**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: committed history, redirects, replacement cancellation, policy and provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-003.03 — Implement/prove: committed history**
  - Action: For navigation broker, implement or reuse and verify this exact obligation: committed history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for committed history; retain observable state/resource expectations.
- [ ] **F-WB-003.04 — Implement/prove: redirects**
  - Action: For navigation broker, implement or reuse and verify this exact obligation: redirects. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for redirects; retain observable state/resource expectations.
- [ ] **F-WB-003.05 — Implement/prove: replacement cancellation**
  - Action: For navigation broker, implement or reuse and verify this exact obligation: replacement cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for replacement cancellation; retain observable state/resource expectations.
- [ ] **F-WB-003.06 — Implement/prove: policy and provenance**
  - Action: For navigation broker, implement or reuse and verify this exact obligation: policy and provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-003.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for policy and provenance; retain observable state/resource expectations.
- [ ] **F-WB-003.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to navigation broker: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-003.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-003.08 — Integrate into the real consumer and runtime route**
  - Action: Wire navigation broker into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-003.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-003.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for navigation broker as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-003.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-004"></a>
## F-WB-004 — browser profile

**Original requirement:** per-user history/bookmarks/settings/storage/downloads with private mode

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-004.01 — Reconcile existing browser profile**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for browser profile. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-user history/bookmarks/settings/storage/downloads with private mode
- [ ] **F-WB-004.02 — Freeze the exact contract for browser profile**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-user history/bookmarks/settings/storage/downloads with private mode. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-004.03 — Implement/prove: per-user history/bookmarks/settings/storage/downloads with private mode**
  - Action: For browser profile, implement or reuse and verify this exact obligation: per-user history/bookmarks/settings/storage/downloads with private mode. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-user history/bookmarks/settings/storage/downloads with private mode; retain observable state/resource expectations.
- [ ] **F-WB-004.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to browser profile: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-004.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-004.05 — Integrate into the real consumer and runtime route**
  - Action: Wire browser profile into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-004.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-004.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for browser profile as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-004.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-005"></a>
## F-WB-005 — content-process isolation

**Original requirement:** per-site/site-instance restricted processes with crash containment

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-005.01 — Reconcile existing content-process isolation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for content-process isolation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-site/site-instance restricted processes with crash containment
- [ ] **F-WB-005.02 — Freeze the exact contract for content-process isolation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-site/site-instance restricted processes with crash containment. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-005.03 — Implement/prove: per-site/site-instance restricted processes with crash containment**
  - Action: For content-process isolation, implement or reuse and verify this exact obligation: per-site/site-instance restricted processes with crash containment. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-site/site-instance restricted processes with crash containment; retain observable state/resource expectations.
- [ ] **F-WB-005.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to content-process isolation: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-005.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-005.05 — Integrate into the real consumer and runtime route**
  - Action: Wire content-process isolation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-005.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-005.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for content-process isolation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-005.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-006"></a>
## F-WB-006 — request/network service

**Original requirement:** DNS/TLS/HTTP outside content/UI with destination and response limits

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-006.01 — Reconcile existing request/network service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for request/network service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: DNS/TLS/HTTP outside content/UI with destination and response limits
- [ ] **F-WB-006.02 — Freeze the exact contract for request/network service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: DNS/TLS/HTTP outside content/UI with destination and response limits. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-006.03 — Implement/prove: DNS/TLS/HTTP outside content/UI with destination and response limits**
  - Action: For request/network service, implement or reuse and verify this exact obligation: DNS/TLS/HTTP outside content/UI with destination and response limits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DNS/TLS/HTTP outside content/UI with destination and response limits; retain observable state/resource expectations.
- [ ] **F-WB-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to request/network service: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire request/network service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for request/network service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-007"></a>
## F-WB-007 — decoder isolation

**Original requirement:** image/font/media/document parsers in disposable bounded processes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-007.01 — Reconcile existing decoder isolation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for decoder isolation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: image/font/media/document parsers in disposable bounded processes
- [ ] **F-WB-007.02 — Freeze the exact contract for decoder isolation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: image/font/media/document parsers in disposable bounded processes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-007.03 — Implement/prove: image/font/media/document parsers in disposable bounded processes**
  - Action: For decoder isolation, implement or reuse and verify this exact obligation: image/font/media/document parsers in disposable bounded processes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for image/font/media/document parsers in disposable bounded processes; retain observable state/resource expectations.
- [ ] **F-WB-007.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to decoder isolation: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-007.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-007.05 — Integrate into the real consumer and runtime route**
  - Action: Wire decoder isolation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-007.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-007.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for decoder isolation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-007.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-008"></a>
## F-WB-008 — HTML parser

**Original requirement:** streaming tree construction, malformed recovery, limits and hostile corpus

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-008.01 — Reconcile existing HTML parser**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for HTML parser. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: streaming tree construction, malformed recovery, limits and hostile corpus
- [ ] **F-WB-008.02 — Freeze the exact contract for HTML parser**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: streaming tree construction, malformed recovery, limits and hostile corpus. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-008.03 — Implement/prove: streaming tree construction**
  - Action: For HTML parser, implement or reuse and verify this exact obligation: streaming tree construction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for streaming tree construction; retain observable state/resource expectations.
- [ ] **F-WB-008.04 — Implement/prove: malformed recovery**
  - Action: For HTML parser, implement or reuse and verify this exact obligation: malformed recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for malformed recovery; retain observable state/resource expectations.
- [ ] **F-WB-008.05 — Implement/prove: limits and hostile corpus**
  - Action: For HTML parser, implement or reuse and verify this exact obligation: limits and hostile corpus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for limits and hostile corpus; retain observable state/resource expectations.
- [ ] **F-WB-008.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to HTML parser: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-008.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-008.07 — Integrate into the real consumer and runtime route**
  - Action: Wire HTML parser into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-008.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-008.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for HTML parser as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-008.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-009"></a>
## F-WB-009 — DOM

**Original requirement:** node/document/events/mutation/lifecycle with quotas and test ledger

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-009.01 — Reconcile existing DOM**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for DOM. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: node/document/events/mutation/lifecycle with quotas and test ledger
- [ ] **F-WB-009.02 — Freeze the exact contract for DOM**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: node/document/events/mutation/lifecycle with quotas and test ledger. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-009.03 — Implement/prove: node/document/events/mutation/lifecycle with quotas and test ledger**
  - Action: For DOM, implement or reuse and verify this exact obligation: node/document/events/mutation/lifecycle with quotas and test ledger. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for node/document/events/mutation/lifecycle with quotas and test ledger; retain observable state/resource expectations.
- [ ] **F-WB-009.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to DOM: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-009.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-009.05 — Integrate into the real consumer and runtime route**
  - Action: Wire DOM into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-009.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-009.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for DOM as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-009.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-010"></a>
## F-WB-010 — CSS parser/cascade

**Original requirement:** selectors, origins, inheritance, variables, media queries and explicit support table

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-010.01 — Reconcile existing CSS parser/cascade**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for CSS parser/cascade. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: selectors, origins, inheritance, variables, media queries and explicit support table
- [ ] **F-WB-010.02 — Freeze the exact contract for CSS parser/cascade**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: selectors, origins, inheritance, variables, media queries and explicit support table. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-010.03 — Implement/prove: selectors**
  - Action: For CSS parser/cascade, implement or reuse and verify this exact obligation: selectors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for selectors; retain observable state/resource expectations.
- [ ] **F-WB-010.04 — Implement/prove: origins**
  - Action: For CSS parser/cascade, implement or reuse and verify this exact obligation: origins. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for origins; retain observable state/resource expectations.
- [ ] **F-WB-010.05 — Implement/prove: inheritance**
  - Action: For CSS parser/cascade, implement or reuse and verify this exact obligation: inheritance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for inheritance; retain observable state/resource expectations.
- [ ] **F-WB-010.06 — Implement/prove: variables**
  - Action: For CSS parser/cascade, implement or reuse and verify this exact obligation: variables. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-010.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for variables; retain observable state/resource expectations.
- [ ] **F-WB-010.07 — Implement/prove: media queries and explicit support table**
  - Action: For CSS parser/cascade, implement or reuse and verify this exact obligation: media queries and explicit support table. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-010.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for media queries and explicit support table; retain observable state/resource expectations.
- [ ] **F-WB-010.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to CSS parser/cascade: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-010.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-010.09 — Integrate into the real consumer and runtime route**
  - Action: Wire CSS parser/cascade into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-010.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-010.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for CSS parser/cascade as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-010.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-011"></a>
## F-WB-011 — style engine

**Original requirement:** incremental invalidation, computed values, pseudo states/elements and budgets

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-011.01 — Reconcile existing style engine**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for style engine. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: incremental invalidation, computed values, pseudo states/elements and budgets
- [ ] **F-WB-011.02 — Freeze the exact contract for style engine**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: incremental invalidation, computed values, pseudo states/elements and budgets. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-011.03 — Implement/prove: incremental invalidation**
  - Action: For style engine, implement or reuse and verify this exact obligation: incremental invalidation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for incremental invalidation; retain observable state/resource expectations.
- [ ] **F-WB-011.04 — Implement/prove: computed values**
  - Action: For style engine, implement or reuse and verify this exact obligation: computed values. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for computed values; retain observable state/resource expectations.
- [ ] **F-WB-011.05 — Implement/prove: pseudo states/elements and budgets**
  - Action: For style engine, implement or reuse and verify this exact obligation: pseudo states/elements and budgets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pseudo states/elements and budgets; retain observable state/resource expectations.
- [ ] **F-WB-011.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to style engine: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-011.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-011.07 — Integrate into the real consumer and runtime route**
  - Action: Wire style engine into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-011.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-011.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for style engine as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-011.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-012"></a>
## F-WB-012 — layout engine

**Original requirement:** flow, flex, grid stages, intrinsic sizing, overflow, writing modes and tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-012.01 — Reconcile existing layout engine**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for layout engine. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: flow, flex, grid stages, intrinsic sizing, overflow, writing modes and tests
- [ ] **F-WB-012.02 — Freeze the exact contract for layout engine**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: flow, flex, grid stages, intrinsic sizing, overflow, writing modes and tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-012.03 — Implement/prove: flow**
  - Action: For layout engine, implement or reuse and verify this exact obligation: flow. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flow; retain observable state/resource expectations.
- [ ] **F-WB-012.04 — Implement/prove: flex**
  - Action: For layout engine, implement or reuse and verify this exact obligation: flex. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flex; retain observable state/resource expectations.
- [ ] **F-WB-012.05 — Implement/prove: grid stages**
  - Action: For layout engine, implement or reuse and verify this exact obligation: grid stages. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grid stages; retain observable state/resource expectations.
- [ ] **F-WB-012.06 — Implement/prove: intrinsic sizing**
  - Action: For layout engine, implement or reuse and verify this exact obligation: intrinsic sizing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-012.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for intrinsic sizing; retain observable state/resource expectations.
- [ ] **F-WB-012.07 — Implement/prove: overflow**
  - Action: For layout engine, implement or reuse and verify this exact obligation: overflow. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-012.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for overflow; retain observable state/resource expectations.
- [ ] **F-WB-012.08 — Implement/prove: writing modes and tests**
  - Action: For layout engine, implement or reuse and verify this exact obligation: writing modes and tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-012.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for writing modes and tests; retain observable state/resource expectations.
- [ ] **F-WB-012.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to layout engine: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-012.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-012.10 — Integrate into the real consumer and runtime route**
  - Action: Wire layout engine into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-012.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-012.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for layout engine as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-012.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-013"></a>
## F-WB-013 — web paint/display list

**Original requirement:** retained clips/transforms/text/images/layers and damage propagation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-013.01 — Reconcile existing web paint/display list**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for web paint/display list. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: retained clips/transforms/text/images/layers and damage propagation
- [ ] **F-WB-013.02 — Freeze the exact contract for web paint/display list**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: retained clips/transforms/text/images/layers and damage propagation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-013.03 — Implement/prove: retained clips/transforms/text/images/layers and damage propagation**
  - Action: For web paint/display list, implement or reuse and verify this exact obligation: retained clips/transforms/text/images/layers and damage propagation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retained clips/transforms/text/images/layers and damage propagation; retain observable state/resource expectations.
- [ ] **F-WB-013.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to web paint/display list: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-013.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-013.05 — Integrate into the real consumer and runtime route**
  - Action: Wire web paint/display list into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-013.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-013.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for web paint/display list as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-013.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-014"></a>
## F-WB-014 — browser compositing

**Original requirement:** scrolling/layers/animations mapped safely into system surfaces

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-014.01 — Reconcile existing browser compositing**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for browser compositing. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: scrolling/layers/animations mapped safely into system surfaces
- [ ] **F-WB-014.02 — Freeze the exact contract for browser compositing**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: scrolling/layers/animations mapped safely into system surfaces. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-014.03 — Implement/prove: scrolling/layers/animations mapped safely into system surfaces**
  - Action: For browser compositing, implement or reuse and verify this exact obligation: scrolling/layers/animations mapped safely into system surfaces. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scrolling/layers/animations mapped safely into system surfaces; retain observable state/resource expectations.
- [ ] **F-WB-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to browser compositing: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire browser compositing into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for browser compositing as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-015"></a>
## F-WB-015 — JavaScript engine

**Original requirement:** explicit language/API support, execution budgets, errors, GC and isolation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-015.01 — Reconcile existing JavaScript engine**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for JavaScript engine. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit language/API support, execution budgets, errors, GC and isolation
- [ ] **F-WB-015.02 — Freeze the exact contract for JavaScript engine**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit language/API support, execution budgets, errors, GC and isolation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-015.03 — Implement/prove: explicit language/API support**
  - Action: For JavaScript engine, implement or reuse and verify this exact obligation: explicit language/API support. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit language/API support; retain observable state/resource expectations.
- [ ] **F-WB-015.04 — Implement/prove: execution budgets**
  - Action: For JavaScript engine, implement or reuse and verify this exact obligation: execution budgets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for execution budgets; retain observable state/resource expectations.
- [ ] **F-WB-015.05 — Implement/prove: errors**
  - Action: For JavaScript engine, implement or reuse and verify this exact obligation: errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-015.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for errors; retain observable state/resource expectations.
- [ ] **F-WB-015.06 — Implement/prove: GC and isolation**
  - Action: For JavaScript engine, implement or reuse and verify this exact obligation: GC and isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-015.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for GC and isolation; retain observable state/resource expectations.
- [ ] **F-WB-015.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to JavaScript engine: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-015.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-015.08 — Integrate into the real consumer and runtime route**
  - Action: Wire JavaScript engine into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-015.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-015.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for JavaScript engine as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-015.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-016"></a>
## F-WB-016 — event loop

**Original requirement:** tasks/microtasks/timers/render steps, cancellation and background throttling

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-016.01 — Reconcile existing event loop**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for event loop. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: tasks/microtasks/timers/render steps, cancellation and background throttling
- [ ] **F-WB-016.02 — Freeze the exact contract for event loop**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: tasks/microtasks/timers/render steps, cancellation and background throttling. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-016.03 — Implement/prove: tasks/microtasks/timers/render steps**
  - Action: For event loop, implement or reuse and verify this exact obligation: tasks/microtasks/timers/render steps. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tasks/microtasks/timers/render steps; retain observable state/resource expectations.
- [ ] **F-WB-016.04 — Implement/prove: cancellation and background throttling**
  - Action: For event loop, implement or reuse and verify this exact obligation: cancellation and background throttling. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancellation and background throttling; retain observable state/resource expectations.
- [ ] **F-WB-016.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to event loop: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-016.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-016.06 — Integrate into the real consumer and runtime route**
  - Action: Wire event loop into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-016.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-016.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for event loop as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-016.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-017"></a>
## F-WB-017 — Web Workers

**Original requirement:** isolated workers, message quotas, termination and origin policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-017.01 — Reconcile existing Web Workers**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Web Workers. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated workers, message quotas, termination and origin policy
- [ ] **F-WB-017.02 — Freeze the exact contract for Web Workers**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated workers, message quotas, termination and origin policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-017.03 — Implement/prove: isolated workers**
  - Action: For Web Workers, implement or reuse and verify this exact obligation: isolated workers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated workers; retain observable state/resource expectations.
- [ ] **F-WB-017.04 — Implement/prove: message quotas**
  - Action: For Web Workers, implement or reuse and verify this exact obligation: message quotas. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for message quotas; retain observable state/resource expectations.
- [ ] **F-WB-017.05 — Implement/prove: termination and origin policy**
  - Action: For Web Workers, implement or reuse and verify this exact obligation: termination and origin policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for termination and origin policy; retain observable state/resource expectations.
- [ ] **F-WB-017.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Web Workers: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-017.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-017.07 — Integrate into the real consumer and runtime route**
  - Action: Wire Web Workers into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-017.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-017.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Web Workers as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-017.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-018"></a>
## F-WB-018 — WebAssembly

**Original requirement:** validated modules, bounded memory/tables, imports, execution budget and no fake exports

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-018.01 — Reconcile existing WebAssembly**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for WebAssembly. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: validated modules, bounded memory/tables, imports, execution budget and no fake exports
- [ ] **F-WB-018.02 — Freeze the exact contract for WebAssembly**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: validated modules, bounded memory/tables, imports, execution budget and no fake exports. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-018.03 — Implement/prove: validated modules**
  - Action: For WebAssembly, implement or reuse and verify this exact obligation: validated modules. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validated modules; retain observable state/resource expectations.
- [ ] **F-WB-018.04 — Implement/prove: bounded memory/tables**
  - Action: For WebAssembly, implement or reuse and verify this exact obligation: bounded memory/tables. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded memory/tables; retain observable state/resource expectations.
- [ ] **F-WB-018.05 — Implement/prove: imports**
  - Action: For WebAssembly, implement or reuse and verify this exact obligation: imports. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for imports; retain observable state/resource expectations.
- [ ] **F-WB-018.06 — Implement/prove: execution budget and no fake exports**
  - Action: For WebAssembly, implement or reuse and verify this exact obligation: execution budget and no fake exports. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-018.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for execution budget and no fake exports; retain observable state/resource expectations.
- [ ] **F-WB-018.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to WebAssembly: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-018.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-018.08 — Integrate into the real consumer and runtime route**
  - Action: Wire WebAssembly into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-018.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-018.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for WebAssembly as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-018.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-019"></a>
## F-WB-019 — Canvas 2D

**Original requirement:** bounded surfaces, drawing state, image/text integration and readback policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-019.01 — Reconcile existing Canvas 2D**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Canvas 2D. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded surfaces, drawing state, image/text integration and readback policy
- [ ] **F-WB-019.02 — Freeze the exact contract for Canvas 2D**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded surfaces, drawing state, image/text integration and readback policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-019.03 — Implement/prove: bounded surfaces**
  - Action: For Canvas 2D, implement or reuse and verify this exact obligation: bounded surfaces. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded surfaces; retain observable state/resource expectations.
- [ ] **F-WB-019.04 — Implement/prove: drawing state**
  - Action: For Canvas 2D, implement or reuse and verify this exact obligation: drawing state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drawing state; retain observable state/resource expectations.
- [ ] **F-WB-019.05 — Implement/prove: image/text integration and readback policy**
  - Action: For Canvas 2D, implement or reuse and verify this exact obligation: image/text integration and readback policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for image/text integration and readback policy; retain observable state/resource expectations.
- [ ] **F-WB-019.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Canvas 2D: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-019.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-019.07 — Integrate into the real consumer and runtime route**
  - Action: Wire Canvas 2D into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-019.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-019.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Canvas 2D as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-019.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-020"></a>
## F-WB-020 — SVG

**Original requirement:** explicit supported subset, geometry/text/paint, parser limits and fallbacks

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-020.01 — Reconcile existing SVG**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for SVG. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit supported subset, geometry/text/paint, parser limits and fallbacks
- [ ] **F-WB-020.02 — Freeze the exact contract for SVG**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit supported subset, geometry/text/paint, parser limits and fallbacks. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-020.03 — Implement/prove: explicit supported subset**
  - Action: For SVG, implement or reuse and verify this exact obligation: explicit supported subset. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit supported subset; retain observable state/resource expectations.
- [ ] **F-WB-020.04 — Implement/prove: geometry/text/paint**
  - Action: For SVG, implement or reuse and verify this exact obligation: geometry/text/paint. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for geometry/text/paint; retain observable state/resource expectations.
- [ ] **F-WB-020.05 — Implement/prove: parser limits and fallbacks**
  - Action: For SVG, implement or reuse and verify this exact obligation: parser limits and fallbacks. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-020.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for parser limits and fallbacks; retain observable state/resource expectations.
- [ ] **F-WB-020.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to SVG: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-020.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-020.07 — Integrate into the real consumer and runtime route**
  - Action: Wire SVG into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-020.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-020.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for SVG as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-020.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-021"></a>
## F-WB-021 — MathML

**Original requirement:** explicit layout/accessibility subset and tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-021.01 — Reconcile existing MathML**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for MathML. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit layout/accessibility subset and tests
- [ ] **F-WB-021.02 — Freeze the exact contract for MathML**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit layout/accessibility subset and tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-021.03 — Implement/prove: explicit layout/accessibility subset and tests**
  - Action: For MathML, implement or reuse and verify this exact obligation: explicit layout/accessibility subset and tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit layout/accessibility subset and tests; retain observable state/resource expectations.
- [ ] **F-WB-021.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to MathML: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-021.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-021.05 — Integrate into the real consumer and runtime route**
  - Action: Wire MathML into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-021.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-021.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for MathML as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-021.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-022"></a>
## F-WB-022 — forms/controls

**Original requirement:** values, labels, validation, submission, keyboard, autofill policy and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-022.01 — Reconcile existing forms/controls**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for forms/controls. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: values, labels, validation, submission, keyboard, autofill policy and a11y
- [ ] **F-WB-022.02 — Freeze the exact contract for forms/controls**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: values, labels, validation, submission, keyboard, autofill policy and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-022.03 — Implement/prove: values**
  - Action: For forms/controls, implement or reuse and verify this exact obligation: values. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for values; retain observable state/resource expectations.
- [ ] **F-WB-022.04 — Implement/prove: labels**
  - Action: For forms/controls, implement or reuse and verify this exact obligation: labels. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for labels; retain observable state/resource expectations.
- [ ] **F-WB-022.05 — Implement/prove: validation**
  - Action: For forms/controls, implement or reuse and verify this exact obligation: validation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validation; retain observable state/resource expectations.
- [ ] **F-WB-022.06 — Implement/prove: submission**
  - Action: For forms/controls, implement or reuse and verify this exact obligation: submission. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for submission; retain observable state/resource expectations.
- [ ] **F-WB-022.07 — Implement/prove: keyboard**
  - Action: For forms/controls, implement or reuse and verify this exact obligation: keyboard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-022.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard; retain observable state/resource expectations.
- [ ] **F-WB-022.08 — Implement/prove: autofill policy and a11y**
  - Action: For forms/controls, implement or reuse and verify this exact obligation: autofill policy and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-022.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for autofill policy and a11y; retain observable state/resource expectations.
- [ ] **F-WB-022.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to forms/controls: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-022.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-022.10 — Integrate into the real consumer and runtime route**
  - Action: Wire forms/controls into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-022.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-022.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for forms/controls as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-022.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-023"></a>
## F-WB-023 — focus/selection/clipboard

**Original requirement:** DOM focus, text selection, copy/paste permissions and system integration

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-023.01 — Reconcile existing focus/selection/clipboard**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for focus/selection/clipboard. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: DOM focus, text selection, copy/paste permissions and system integration
- [ ] **F-WB-023.02 — Freeze the exact contract for focus/selection/clipboard**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: DOM focus, text selection, copy/paste permissions and system integration. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-023.03 — Implement/prove: DOM focus**
  - Action: For focus/selection/clipboard, implement or reuse and verify this exact obligation: DOM focus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DOM focus; retain observable state/resource expectations.
- [ ] **F-WB-023.04 — Implement/prove: text selection**
  - Action: For focus/selection/clipboard, implement or reuse and verify this exact obligation: text selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for text selection; retain observable state/resource expectations.
- [ ] **F-WB-023.05 — Implement/prove: copy/paste permissions and system integration**
  - Action: For focus/selection/clipboard, implement or reuse and verify this exact obligation: copy/paste permissions and system integration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for copy/paste permissions and system integration; retain observable state/resource expectations.
- [ ] **F-WB-023.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to focus/selection/clipboard: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-023.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-023.07 — Integrate into the real consumer and runtime route**
  - Action: Wire focus/selection/clipboard into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-023.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-023.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for focus/selection/clipboard as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-023.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-024"></a>
## F-WB-024 — scrolling

**Original requirement:** viewport/element scroll, anchors, smooth/reduced motion and virtualization

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-024.01 — Reconcile existing scrolling**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for scrolling. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: viewport/element scroll, anchors, smooth/reduced motion and virtualization
- [ ] **F-WB-024.02 — Freeze the exact contract for scrolling**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: viewport/element scroll, anchors, smooth/reduced motion and virtualization. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-024.03 — Implement/prove: viewport/element scroll**
  - Action: For scrolling, implement or reuse and verify this exact obligation: viewport/element scroll. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for viewport/element scroll; retain observable state/resource expectations.
- [ ] **F-WB-024.04 — Implement/prove: anchors**
  - Action: For scrolling, implement or reuse and verify this exact obligation: anchors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for anchors; retain observable state/resource expectations.
- [ ] **F-WB-024.05 — Implement/prove: smooth/reduced motion and virtualization**
  - Action: For scrolling, implement or reuse and verify this exact obligation: smooth/reduced motion and virtualization. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-024.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for smooth/reduced motion and virtualization; retain observable state/resource expectations.
- [ ] **F-WB-024.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to scrolling: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-024.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-024.07 — Integrate into the real consumer and runtime route**
  - Action: Wire scrolling into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-024.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-024.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for scrolling as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-024.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-025"></a>
## F-WB-025 — cookies

**Original requirement:** domain/path/expiry/SameSite/Secure/HttpOnly, quotas and clearing

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-025.01 — Reconcile existing cookies**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for cookies. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: domain/path/expiry/SameSite/Secure/HttpOnly, quotas and clearing
- [ ] **F-WB-025.02 — Freeze the exact contract for cookies**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: domain/path/expiry/SameSite/Secure/HttpOnly, quotas and clearing. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-025.03 — Implement/prove: domain/path/expiry/SameSite/Secure/HttpOnly**
  - Action: For cookies, implement or reuse and verify this exact obligation: domain/path/expiry/SameSite/Secure/HttpOnly. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for domain/path/expiry/SameSite/Secure/HttpOnly; retain observable state/resource expectations.
- [ ] **F-WB-025.04 — Implement/prove: quotas and clearing**
  - Action: For cookies, implement or reuse and verify this exact obligation: quotas and clearing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quotas and clearing; retain observable state/resource expectations.
- [ ] **F-WB-025.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to cookies: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-025.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-025.06 — Integrate into the real consumer and runtime route**
  - Action: Wire cookies into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-025.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-025.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for cookies as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-025.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-026"></a>
## F-WB-026 — local/session storage

**Original requirement:** per-origin quotas, persistence/private mode, atomicity and eviction

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-026.01 — Reconcile existing local/session storage**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for local/session storage. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-origin quotas, persistence/private mode, atomicity and eviction
- [ ] **F-WB-026.02 — Freeze the exact contract for local/session storage**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-origin quotas, persistence/private mode, atomicity and eviction. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-026.03 — Implement/prove: per-origin quotas**
  - Action: For local/session storage, implement or reuse and verify this exact obligation: per-origin quotas. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-origin quotas; retain observable state/resource expectations.
- [ ] **F-WB-026.04 — Implement/prove: persistence/private mode**
  - Action: For local/session storage, implement or reuse and verify this exact obligation: persistence/private mode. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for persistence/private mode; retain observable state/resource expectations.
- [ ] **F-WB-026.05 — Implement/prove: atomicity and eviction**
  - Action: For local/session storage, implement or reuse and verify this exact obligation: atomicity and eviction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atomicity and eviction; retain observable state/resource expectations.
- [ ] **F-WB-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to local/session storage: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire local/session storage into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for local/session storage as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-027"></a>
## F-WB-027 — IndexedDB-like storage

**Original requirement:** transactional per-origin structured storage, quotas and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-027.01 — Reconcile existing IndexedDB-like storage**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for IndexedDB-like storage. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: transactional per-origin structured storage, quotas and recovery
- [ ] **F-WB-027.02 — Freeze the exact contract for IndexedDB-like storage**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: transactional per-origin structured storage, quotas and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-027.03 — Implement/prove: transactional per-origin structured storage**
  - Action: For IndexedDB-like storage, implement or reuse and verify this exact obligation: transactional per-origin structured storage. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for transactional per-origin structured storage; retain observable state/resource expectations.
- [ ] **F-WB-027.04 — Implement/prove: quotas and recovery**
  - Action: For IndexedDB-like storage, implement or reuse and verify this exact obligation: quotas and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quotas and recovery; retain observable state/resource expectations.
- [ ] **F-WB-027.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to IndexedDB-like storage: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-027.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-027.06 — Integrate into the real consumer and runtime route**
  - Action: Wire IndexedDB-like storage into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-027.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-027.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for IndexedDB-like storage as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-027.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-028"></a>
## F-WB-028 — cache

**Original requirement:** HTTP semantics, bounded storage, vary/revalidate/evict and private mode

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-028.01 — Reconcile existing cache**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for cache. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: HTTP semantics, bounded storage, vary/revalidate/evict and private mode
- [ ] **F-WB-028.02 — Freeze the exact contract for cache**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: HTTP semantics, bounded storage, vary/revalidate/evict and private mode. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-028.03 — Implement/prove: HTTP semantics**
  - Action: For cache, implement or reuse and verify this exact obligation: HTTP semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for HTTP semantics; retain observable state/resource expectations.
- [ ] **F-WB-028.04 — Implement/prove: bounded storage**
  - Action: For cache, implement or reuse and verify this exact obligation: bounded storage. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded storage; retain observable state/resource expectations.
- [ ] **F-WB-028.05 — Implement/prove: vary/revalidate/evict and private mode**
  - Action: For cache, implement or reuse and verify this exact obligation: vary/revalidate/evict and private mode. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for vary/revalidate/evict and private mode; retain observable state/resource expectations.
- [ ] **F-WB-028.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to cache: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-028.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-028.07 — Integrate into the real consumer and runtime route**
  - Action: Wire cache into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-028.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-028.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for cache as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-028.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-029"></a>
## F-WB-029 — CORS/referrer/origin policy

**Original requirement:** complete enforcement with preflight/cache and hostile tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-029.01 — Reconcile existing CORS/referrer/origin policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for CORS/referrer/origin policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: complete enforcement with preflight/cache and hostile tests
- [ ] **F-WB-029.02 — Freeze the exact contract for CORS/referrer/origin policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: complete enforcement with preflight/cache and hostile tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-029.03 — Implement/prove: complete enforcement with preflight/cache and hostile tests**
  - Action: For CORS/referrer/origin policy, implement or reuse and verify this exact obligation: complete enforcement with preflight/cache and hostile tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for complete enforcement with preflight/cache and hostile tests; retain observable state/resource expectations.
- [ ] **F-WB-029.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to CORS/referrer/origin policy: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-029.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-029.05 — Integrate into the real consumer and runtime route**
  - Action: Wire CORS/referrer/origin policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-029.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-029.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for CORS/referrer/origin policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-029.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-030"></a>
## F-WB-030 — CSP/mixed-content policy

**Original requirement:** enforce content sources, script policy, upgrade/block and reporting

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-030.01 — Reconcile existing CSP/mixed-content policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for CSP/mixed-content policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: enforce content sources, script policy, upgrade/block and reporting
- [ ] **F-WB-030.02 — Freeze the exact contract for CSP/mixed-content policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: enforce content sources, script policy, upgrade/block and reporting. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-030.03 — Implement/prove: enforce content sources**
  - Action: For CSP/mixed-content policy, implement or reuse and verify this exact obligation: enforce content sources. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for enforce content sources; retain observable state/resource expectations.
- [ ] **F-WB-030.04 — Implement/prove: script policy**
  - Action: For CSP/mixed-content policy, implement or reuse and verify this exact obligation: script policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for script policy; retain observable state/resource expectations.
- [ ] **F-WB-030.05 — Implement/prove: upgrade/block and reporting**
  - Action: For CSP/mixed-content policy, implement or reuse and verify this exact obligation: upgrade/block and reporting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-030.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for upgrade/block and reporting; retain observable state/resource expectations.
- [ ] **F-WB-030.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to CSP/mixed-content policy: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-030.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-030.07 — Integrate into the real consumer and runtime route**
  - Action: Wire CSP/mixed-content policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-030.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-030.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for CSP/mixed-content policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-030.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-031"></a>
## F-WB-031 — permissions model

**Original requirement:** origin-scoped camera/mic/location/clipboard/notification grants and dashboard

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-031.01 — Reconcile existing permissions model**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for permissions model. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: origin-scoped camera/mic/location/clipboard/notification grants and dashboard
- [ ] **F-WB-031.02 — Freeze the exact contract for permissions model**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: origin-scoped camera/mic/location/clipboard/notification grants and dashboard. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-031.03 — Implement/prove: origin-scoped camera/mic/location/clipboard/notification grants and dashboard**
  - Action: For permissions model, implement or reuse and verify this exact obligation: origin-scoped camera/mic/location/clipboard/notification grants and dashboard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for origin-scoped camera/mic/location/clipboard/notification grants and dashboard; retain observable state/resource expectations.
- [ ] **F-WB-031.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to permissions model: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-031.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-031.05 — Integrate into the real consumer and runtime route**
  - Action: Wire permissions model into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-031.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-031.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for permissions model as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-031.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-032"></a>
## F-WB-032 — sandboxed frames

**Original requirement:** iframe depth/resources/origin/sandbox/message policy and process boundary

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-032.01 — Reconcile existing sandboxed frames**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for sandboxed frames. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: iframe depth/resources/origin/sandbox/message policy and process boundary
- [ ] **F-WB-032.02 — Freeze the exact contract for sandboxed frames**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: iframe depth/resources/origin/sandbox/message policy and process boundary. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-032.03 — Implement/prove: iframe depth/resources/origin/sandbox/message policy and process boundary**
  - Action: For sandboxed frames, implement or reuse and verify this exact obligation: iframe depth/resources/origin/sandbox/message policy and process boundary. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for iframe depth/resources/origin/sandbox/message policy and process boundary; retain observable state/resource expectations.
- [ ] **F-WB-032.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to sandboxed frames: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-032.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-032.05 — Integrate into the real consumer and runtime route**
  - Action: Wire sandboxed frames into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-032.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-032.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for sandboxed frames as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-032.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-033"></a>
## F-WB-033 — downloads

**Original requirement:** user-mediated destination, size/progress/cancel/hash/quarantine and provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-033.01 — Reconcile existing downloads**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for downloads. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: user-mediated destination, size/progress/cancel/hash/quarantine and provenance
- [ ] **F-WB-033.02 — Freeze the exact contract for downloads**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: user-mediated destination, size/progress/cancel/hash/quarantine and provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-033.03 — Implement/prove: user-mediated destination**
  - Action: For downloads, implement or reuse and verify this exact obligation: user-mediated destination. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user-mediated destination; retain observable state/resource expectations.
- [ ] **F-WB-033.04 — Implement/prove: size/progress/cancel/hash/quarantine and provenance**
  - Action: For downloads, implement or reuse and verify this exact obligation: size/progress/cancel/hash/quarantine and provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for size/progress/cancel/hash/quarantine and provenance; retain observable state/resource expectations.
- [ ] **F-WB-033.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to downloads: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-033.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-033.06 — Integrate into the real consumer and runtime route**
  - Action: Wire downloads into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-033.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-033.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for downloads as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-033.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-034"></a>
## F-WB-034 — uploads/file chooser

**Original requirement:** portal-scoped file handles, names/metadata/privacy and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-034.01 — Reconcile existing uploads/file chooser**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for uploads/file chooser. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: portal-scoped file handles, names/metadata/privacy and cancellation
- [ ] **F-WB-034.02 — Freeze the exact contract for uploads/file chooser**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: portal-scoped file handles, names/metadata/privacy and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-034.03 — Implement/prove: portal-scoped file handles**
  - Action: For uploads/file chooser, implement or reuse and verify this exact obligation: portal-scoped file handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for portal-scoped file handles; retain observable state/resource expectations.
- [ ] **F-WB-034.04 — Implement/prove: names/metadata/privacy and cancellation**
  - Action: For uploads/file chooser, implement or reuse and verify this exact obligation: names/metadata/privacy and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-034.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for names/metadata/privacy and cancellation; retain observable state/resource expectations.
- [ ] **F-WB-034.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to uploads/file chooser: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-034.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-034.06 — Integrate into the real consumer and runtime route**
  - Action: Wire uploads/file chooser into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-034.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-034.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for uploads/file chooser as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-034.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-035"></a>
## F-WB-035 — bookmarks/history

**Original requirement:** durable searchable per-profile data, sync/export/privacy and clearing

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-035.01 — Reconcile existing bookmarks/history**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for bookmarks/history. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: durable searchable per-profile data, sync/export/privacy and clearing
- [ ] **F-WB-035.02 — Freeze the exact contract for bookmarks/history**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: durable searchable per-profile data, sync/export/privacy and clearing. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-035.03 — Implement/prove: durable searchable per-profile data**
  - Action: For bookmarks/history, implement or reuse and verify this exact obligation: durable searchable per-profile data. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for durable searchable per-profile data; retain observable state/resource expectations.
- [ ] **F-WB-035.04 — Implement/prove: sync/export/privacy and clearing**
  - Action: For bookmarks/history, implement or reuse and verify this exact obligation: sync/export/privacy and clearing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-035.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sync/export/privacy and clearing; retain observable state/resource expectations.
- [ ] **F-WB-035.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to bookmarks/history: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-035.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-035.06 — Integrate into the real consumer and runtime route**
  - Action: Wire bookmarks/history into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-035.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-035.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for bookmarks/history as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-035.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-036"></a>
## F-WB-036 — tabs/windows

**Original requirement:** lifecycle, suspension, crash state, restore, resource budget and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-036.01 — Reconcile existing tabs/windows**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for tabs/windows. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: lifecycle, suspension, crash state, restore, resource budget and a11y
- [ ] **F-WB-036.02 — Freeze the exact contract for tabs/windows**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: lifecycle, suspension, crash state, restore, resource budget and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-036.03 — Implement/prove: lifecycle**
  - Action: For tabs/windows, implement or reuse and verify this exact obligation: lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lifecycle; retain observable state/resource expectations.
- [ ] **F-WB-036.04 — Implement/prove: suspension**
  - Action: For tabs/windows, implement or reuse and verify this exact obligation: suspension. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-036.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for suspension; retain observable state/resource expectations.
- [ ] **F-WB-036.05 — Implement/prove: crash state**
  - Action: For tabs/windows, implement or reuse and verify this exact obligation: crash state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-036.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for crash state; retain observable state/resource expectations.
- [ ] **F-WB-036.06 — Implement/prove: restore**
  - Action: For tabs/windows, implement or reuse and verify this exact obligation: restore. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-036.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for restore; retain observable state/resource expectations.
- [ ] **F-WB-036.07 — Implement/prove: resource budget and a11y**
  - Action: For tabs/windows, implement or reuse and verify this exact obligation: resource budget and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-036.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resource budget and a11y; retain observable state/resource expectations.
- [ ] **F-WB-036.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to tabs/windows: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-036.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-036.09 — Integrate into the real consumer and runtime route**
  - Action: Wire tabs/windows into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-036.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-036.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for tabs/windows as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-036.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-037"></a>
## F-WB-037 — private browsing

**Original requirement:** isolated ephemeral storage/history/cache with explicit limits

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-037.01 — Reconcile existing private browsing**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for private browsing. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated ephemeral storage/history/cache with explicit limits
- [ ] **F-WB-037.02 — Freeze the exact contract for private browsing**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated ephemeral storage/history/cache with explicit limits. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-037.03 — Implement/prove: isolated ephemeral storage/history/cache with explicit limits**
  - Action: For private browsing, implement or reuse and verify this exact obligation: isolated ephemeral storage/history/cache with explicit limits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated ephemeral storage/history/cache with explicit limits; retain observable state/resource expectations.
- [ ] **F-WB-037.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to private browsing: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-037.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-037.05 — Integrate into the real consumer and runtime route**
  - Action: Wire private browsing into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-037.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-037.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for private browsing as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-037.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-038"></a>
## F-WB-038 — password/autofill integration

**Original requirement:** secrets handles, origin binding, consent, phishing resistance and no page access

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-038.01 — Reconcile existing password/autofill integration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for password/autofill integration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: secrets handles, origin binding, consent, phishing resistance and no page access
- [ ] **F-WB-038.02 — Freeze the exact contract for password/autofill integration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: secrets handles, origin binding, consent, phishing resistance and no page access. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-038.03 — Implement/prove: secrets handles**
  - Action: For password/autofill integration, implement or reuse and verify this exact obligation: secrets handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for secrets handles; retain observable state/resource expectations.
- [ ] **F-WB-038.04 — Implement/prove: origin binding**
  - Action: For password/autofill integration, implement or reuse and verify this exact obligation: origin binding. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-038.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for origin binding; retain observable state/resource expectations.
- [ ] **F-WB-038.05 — Implement/prove: consent**
  - Action: For password/autofill integration, implement or reuse and verify this exact obligation: consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-038.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for consent; retain observable state/resource expectations.
- [ ] **F-WB-038.06 — Implement/prove: phishing resistance and no page access**
  - Action: For password/autofill integration, implement or reuse and verify this exact obligation: phishing resistance and no page access. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-038.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for phishing resistance and no page access; retain observable state/resource expectations.
- [ ] **F-WB-038.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to password/autofill integration: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-038.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-038.08 — Integrate into the real consumer and runtime route**
  - Action: Wire password/autofill integration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-038.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-038.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for password/autofill integration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-038.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-039"></a>
## F-WB-039 — certificate/security UI

**Original requirement:** verified origin, connection/cert errors, override policy and details

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-039.01 — Reconcile existing certificate/security UI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for certificate/security UI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: verified origin, connection/cert errors, override policy and details
- [ ] **F-WB-039.02 — Freeze the exact contract for certificate/security UI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: verified origin, connection/cert errors, override policy and details. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-039.03 — Implement/prove: verified origin**
  - Action: For certificate/security UI, implement or reuse and verify this exact obligation: verified origin. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for verified origin; retain observable state/resource expectations.
- [ ] **F-WB-039.04 — Implement/prove: connection/cert errors**
  - Action: For certificate/security UI, implement or reuse and verify this exact obligation: connection/cert errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-039.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for connection/cert errors; retain observable state/resource expectations.
- [ ] **F-WB-039.05 — Implement/prove: override policy and details**
  - Action: For certificate/security UI, implement or reuse and verify this exact obligation: override policy and details. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-039.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for override policy and details; retain observable state/resource expectations.
- [ ] **F-WB-039.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to certificate/security UI: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-039.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-039.07 — Integrate into the real consumer and runtime route**
  - Action: Wire certificate/security UI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-039.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-039.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for certificate/security UI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-039.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-040"></a>
## F-WB-040 — page accessibility

**Original requirement:** semantic tree, names/roles/states, focus, live regions and screen reader

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-040.01 — Reconcile existing page accessibility**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for page accessibility. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: semantic tree, names/roles/states, focus, live regions and screen reader
- [ ] **F-WB-040.02 — Freeze the exact contract for page accessibility**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: semantic tree, names/roles/states, focus, live regions and screen reader. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-040.03 — Implement/prove: semantic tree**
  - Action: For page accessibility, implement or reuse and verify this exact obligation: semantic tree. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantic tree; retain observable state/resource expectations.
- [ ] **F-WB-040.04 — Implement/prove: names/roles/states**
  - Action: For page accessibility, implement or reuse and verify this exact obligation: names/roles/states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-040.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for names/roles/states; retain observable state/resource expectations.
- [ ] **F-WB-040.05 — Implement/prove: focus**
  - Action: For page accessibility, implement or reuse and verify this exact obligation: focus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-040.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus; retain observable state/resource expectations.
- [ ] **F-WB-040.06 — Implement/prove: live regions and screen reader**
  - Action: For page accessibility, implement or reuse and verify this exact obligation: live regions and screen reader. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-040.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for live regions and screen reader; retain observable state/resource expectations.
- [ ] **F-WB-040.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to page accessibility: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-040.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-040.08 — Integrate into the real consumer and runtime route**
  - Action: Wire page accessibility into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-040.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-040.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for page accessibility as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-040.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-041"></a>
## F-WB-041 — developer tools

**Original requirement:** DOM/style/layout/network/console/performance inspection with debug authority

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-041.01 — Reconcile existing developer tools**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for developer tools. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: DOM/style/layout/network/console/performance inspection with debug authority
- [ ] **F-WB-041.02 — Freeze the exact contract for developer tools**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: DOM/style/layout/network/console/performance inspection with debug authority. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-041.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-041.03 — Implement/prove: DOM/style/layout/network/console/performance inspection with debug authority**
  - Action: For developer tools, implement or reuse and verify this exact obligation: DOM/style/layout/network/console/performance inspection with debug authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-041.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DOM/style/layout/network/console/performance inspection with debug authority; retain observable state/resource expectations.
- [ ] **F-WB-041.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to developer tools: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-041.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-041.05 — Integrate into the real consumer and runtime route**
  - Action: Wire developer tools into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-041.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-041.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for developer tools as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-041.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-042"></a>
## F-WB-042 — reader mode

**Original requirement:** extracted readable content, provenance, images, typography and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-042.01 — Reconcile existing reader mode**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for reader mode. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: extracted readable content, provenance, images, typography and a11y
- [ ] **F-WB-042.02 — Freeze the exact contract for reader mode**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: extracted readable content, provenance, images, typography and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-042.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-042.03 — Implement/prove: extracted readable content**
  - Action: For reader mode, implement or reuse and verify this exact obligation: extracted readable content. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-042.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for extracted readable content; retain observable state/resource expectations.
- [ ] **F-WB-042.04 — Implement/prove: provenance**
  - Action: For reader mode, implement or reuse and verify this exact obligation: provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-042.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance; retain observable state/resource expectations.
- [ ] **F-WB-042.05 — Implement/prove: images**
  - Action: For reader mode, implement or reuse and verify this exact obligation: images. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-042.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for images; retain observable state/resource expectations.
- [ ] **F-WB-042.06 — Implement/prove: typography and a11y**
  - Action: For reader mode, implement or reuse and verify this exact obligation: typography and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-042.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typography and a11y; retain observable state/resource expectations.
- [ ] **F-WB-042.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to reader mode: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-042.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-042.08 — Integrate into the real consumer and runtime route**
  - Action: Wire reader mode into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-042.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-042.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for reader mode as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-042.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-043"></a>
## F-WB-043 — find in page

**Original requirement:** incremental cancelable Unicode search and highlighted semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-043.01 — Reconcile existing find in page**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for find in page. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: incremental cancelable Unicode search and highlighted semantics
- [ ] **F-WB-043.02 — Freeze the exact contract for find in page**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: incremental cancelable Unicode search and highlighted semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-043.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-043.03 — Implement/prove: incremental cancelable Unicode search and highlighted semantics**
  - Action: For find in page, implement or reuse and verify this exact obligation: incremental cancelable Unicode search and highlighted semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-043.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for incremental cancelable Unicode search and highlighted semantics; retain observable state/resource expectations.
- [ ] **F-WB-043.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to find in page: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-043.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-043.05 — Integrate into the real consumer and runtime route**
  - Action: Wire find in page into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-043.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-043.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for find in page as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-043.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-044"></a>
## F-WB-044 — print/PDF output

**Original requirement:** paged layout, preview, privacy and print service integration

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-044.01 — Reconcile existing print/PDF output**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for print/PDF output. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: paged layout, preview, privacy and print service integration
- [ ] **F-WB-044.02 — Freeze the exact contract for print/PDF output**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: paged layout, preview, privacy and print service integration. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-044.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-044.03 — Implement/prove: paged layout**
  - Action: For print/PDF output, implement or reuse and verify this exact obligation: paged layout. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-044.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for paged layout; retain observable state/resource expectations.
- [ ] **F-WB-044.04 — Implement/prove: preview**
  - Action: For print/PDF output, implement or reuse and verify this exact obligation: preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-044.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for preview; retain observable state/resource expectations.
- [ ] **F-WB-044.05 — Implement/prove: privacy and print service integration**
  - Action: For print/PDF output, implement or reuse and verify this exact obligation: privacy and print service integration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-044.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and print service integration; retain observable state/resource expectations.
- [ ] **F-WB-044.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to print/PDF output: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-044.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-044.07 — Integrate into the real consumer and runtime route**
  - Action: Wire print/PDF output into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-044.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-044.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for print/PDF output as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-044.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-045"></a>
## F-WB-045 — browser performance telemetry

**Original requirement:** navigation phases, long tasks, layout/paint, frame drops, memory and crashes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-045.01 — Reconcile existing browser performance telemetry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for browser performance telemetry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: navigation phases, long tasks, layout/paint, frame drops, memory and crashes
- [ ] **F-WB-045.02 — Freeze the exact contract for browser performance telemetry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: navigation phases, long tasks, layout/paint, frame drops, memory and crashes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-045.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-045.03 — Implement/prove: navigation phases**
  - Action: For browser performance telemetry, implement or reuse and verify this exact obligation: navigation phases. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-045.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for navigation phases; retain observable state/resource expectations.
- [ ] **F-WB-045.04 — Implement/prove: long tasks**
  - Action: For browser performance telemetry, implement or reuse and verify this exact obligation: long tasks. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-045.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for long tasks; retain observable state/resource expectations.
- [ ] **F-WB-045.05 — Implement/prove: layout/paint**
  - Action: For browser performance telemetry, implement or reuse and verify this exact obligation: layout/paint. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-045.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for layout/paint; retain observable state/resource expectations.
- [ ] **F-WB-045.06 — Implement/prove: frame drops**
  - Action: For browser performance telemetry, implement or reuse and verify this exact obligation: frame drops. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-045.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for frame drops; retain observable state/resource expectations.
- [ ] **F-WB-045.07 — Implement/prove: memory and crashes**
  - Action: For browser performance telemetry, implement or reuse and verify this exact obligation: memory and crashes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-045.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for memory and crashes; retain observable state/resource expectations.
- [ ] **F-WB-045.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to browser performance telemetry: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-045.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-045.09 — Integrate into the real consumer and runtime route**
  - Action: Wire browser performance telemetry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-045.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-045.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for browser performance telemetry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-045.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-046"></a>
## F-WB-046 — hostile-page recovery

**Original requirement:** stop/kill/reload content process while browser UI and session remain responsive

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-046.01 — Reconcile existing hostile-page recovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for hostile-page recovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stop/kill/reload content process while browser UI and session remain responsive
- [ ] **F-WB-046.02 — Freeze the exact contract for hostile-page recovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stop/kill/reload content process while browser UI and session remain responsive. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-046.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-046.03 — Implement/prove: stop/kill/reload content process while browser UI and session remain responsive**
  - Action: For hostile-page recovery, implement or reuse and verify this exact obligation: stop/kill/reload content process while browser UI and session remain responsive. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-046.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stop/kill/reload content process while browser UI and session remain responsive; retain observable state/resource expectations.
- [ ] **F-WB-046.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to hostile-page recovery: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-046.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-046.05 — Integrate into the real consumer and runtime route**
  - Action: Wire hostile-page recovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-046.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-046.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for hostile-page recovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-046.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-047"></a>
## F-WB-047 — agent-readable web tools

**Original requirement:** permissioned link/text/form/action model distinct from pixels and page authority

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-047.01 — Reconcile existing agent-readable web tools**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for agent-readable web tools. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: permissioned link/text/form/action model distinct from pixels and page authority
- [ ] **F-WB-047.02 — Freeze the exact contract for agent-readable web tools**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: permissioned link/text/form/action model distinct from pixels and page authority. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-047.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-047.03 — Implement/prove: permissioned link/text/form/action model distinct from pixels and page authority**
  - Action: For agent-readable web tools, implement or reuse and verify this exact obligation: permissioned link/text/form/action model distinct from pixels and page authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-047.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissioned link/text/form/action model distinct from pixels and page authority; retain observable state/resource expectations.
- [ ] **F-WB-047.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to agent-readable web tools: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-047.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-047.05 — Integrate into the real consumer and runtime route**
  - Action: Wire agent-readable web tools into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-047.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-047.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for agent-readable web tools as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-047.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-048"></a>
## F-WB-048 — web compatibility ledger

**Original requirement:** supported standards/tests/version and explicit gaps; never “Chrome parity” by implication

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-048.01 — Reconcile existing web compatibility ledger**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for web compatibility ledger. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: supported standards/tests/version and explicit gaps; never “Chrome parity” by implication
- [ ] **F-WB-048.02 — Freeze the exact contract for web compatibility ledger**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: supported standards/tests/version and explicit gaps; never “Chrome parity” by implication. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-048.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-048.03 — Implement/prove: supported standards/tests/version and explicit gaps**
  - Action: For web compatibility ledger, implement or reuse and verify this exact obligation: supported standards/tests/version and explicit gaps. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-048.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for supported standards/tests/version and explicit gaps; retain observable state/resource expectations.
- [ ] **F-WB-048.04 — Implement/prove: never “Chrome parity” by implication**
  - Action: For web compatibility ledger, implement or reuse and verify this exact obligation: never “Chrome parity” by implication. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-048.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for never “Chrome parity” by implication; retain observable state/resource expectations.
- [ ] **F-WB-048.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to web compatibility ledger: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-048.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-048.06 — Integrate into the real consumer and runtime route**
  - Action: Wire web compatibility ledger into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-048.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-048.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for web compatibility ledger as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-048.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-049"></a>
## F-WB-049 — browser extensions

**Original requirement:** signed restricted extension processes, declared page/network/UI grants and disable/recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-049.01 — Reconcile existing browser extensions**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for browser extensions. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed restricted extension processes, declared page/network/UI grants and disable/recovery
- [ ] **F-WB-049.02 — Freeze the exact contract for browser extensions**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed restricted extension processes, declared page/network/UI grants and disable/recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-049.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-049.03 — Implement/prove: signed restricted extension processes**
  - Action: For browser extensions, implement or reuse and verify this exact obligation: signed restricted extension processes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-049.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed restricted extension processes; retain observable state/resource expectations.
- [ ] **F-WB-049.04 — Implement/prove: declared page/network/UI grants and disable/recovery**
  - Action: For browser extensions, implement or reuse and verify this exact obligation: declared page/network/UI grants and disable/recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-049.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for declared page/network/UI grants and disable/recovery; retain observable state/resource expectations.
- [ ] **F-WB-049.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to browser extensions: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-049.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-049.06 — Integrate into the real consumer and runtime route**
  - Action: Wire browser extensions into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-049.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-049.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for browser extensions as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-049.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-wb-050"></a>
## F-WB-050 — installable web apps

**Original requirement:** manifest/service-worker/storage/notifications/offline package with origin-bound sandbox

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-WB-050.01 — Reconcile existing installable web apps**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for installable web apps. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-and-evidence comparison against the complete requirement: manifest/service-worker/storage/notifications/offline package with origin-bound sandbox
- [ ] **F-WB-050.02 — Freeze the exact contract for installable web apps**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: manifest/service-worker/storage/notifications/offline package with origin-bound sandbox. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-WB-050.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-WB-050.03 — Implement/prove: manifest/service-worker/storage/notifications/offline package with origin-bound sandbox**
  - Action: For installable web apps, implement or reuse and verify this exact obligation: manifest/service-worker/storage/notifications/offline package with origin-bound sandbox. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-WB-050.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for manifest/service-worker/storage/notifications/offline package with origin-bound sandbox; retain observable state/resource expectations.
- [ ] **F-WB-050.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to installable web apps: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-WB-050.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-WB-050.05 — Integrate into the real consumer and runtime route**
  - Action: Wire installable web apps into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-WB-050.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-WB-050.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for installable web apps as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-WB-050.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p6-4"></a>
## C-P6.4 — browser process split and durable data

**Original requirement:** browser process split and durable data

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 367.

### Preserved original contract

- **Dependencies/current/provenance:** P5 Window/File/parsers, P6.3 and current browser; Serenity browser processes; reject one privileged monolith and simulated compatibility.
- **I/O and state:** navigation/user events in; UI/content/request/decoder processes, surfaces and persisted history/bookmark/download out; navigation state explicit.
- **Invariants/failure:** content never gets raw NIC/filesystem/device; download destination is granted handle; process crash isolated; unsupported web feature visible.
- **Deterministic proof:** host render oracles, navigation cancellation, malformed content/decoder crash, process restart, persistence reboot, capability denial.
- **Target proof:** QEMU and physical link render/fetch/download; one process deliberately crashes while desktop/UI recover.
- **Receipt/removal:** scene/document/request/data hashes and process map; current browser remains fallback until workflow parity; remove kernel browser only after all gates.

### Execution steps

- [ ] **C-P6.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P6.4.02 — Resolve this contract's exact dependencies**
  - Action: P5 Window/File/parsers, P6.3 and current browser; Serenity browser processes; reject one privileged monolith and simulated compatibility. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P6.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P6.4.03 — I/O and state — browser process split and durable data**
  - Action: navigation/user events in; UI/content/request/decoder processes, surfaces and persisted history/bookmark/download out; navigation state explicit.
  - Requires: C-P6.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P6.4.
- [ ] **C-P6.4.04 — Invariants/failure — browser process split and durable data**
  - Action: content never gets raw NIC/filesystem/device; download destination is granted handle; process crash isolated; unsupported web feature visible.
  - Requires: C-P6.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P6.4.
- [ ] **C-P6.4.05 — Deterministic proof — browser process split and durable data**
  - Action: host render oracles, navigation cancellation, malformed content/decoder crash, process restart, persistence reboot, capability denial.
  - Requires: C-P6.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P6.4.
- [ ] **C-P6.4.06 — Target proof — browser process split and durable data**
  - Action: QEMU and physical link render/fetch/download; one process deliberately crashes while desktop/UI recover.
  - Requires: C-P6.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P6.4.
- [ ] **C-P6.4.07 — Receipt/removal — browser process split and durable data**
  - Action: scene/document/request/data hashes and process map; current browser remains fallback until workflow parity; remove kernel browser only after all gates.
  - Requires: C-P6.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P6.4.
- [ ] **C-P6.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P6.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P6.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-30"></a>
## C-DA-30 — network and content applications

**Original requirement:** network and content applications

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 741.

### Preserved original contract

**Apps:** Browser, download manager, package client, remote shell/file transfer and
later mail/calendar/contact clients.

**Deliver:** split UI, fetch/TLS and hostile parser/content processes with scoped
network/file rights and privacy controls for history/bookmarks/cache.

**Proof:** parser/network service crash, malicious page/file/certificate, download
path collision, revoked network, offline mode, cache/history deletion and no desktop
or credential compromise.

### Execution steps

- [ ] **C-DA-30.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-30.02 — Resolve this contract's exact dependencies**
  - Action: Inherited phase and source-document dependency rules; inspect the complete source contract below. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-30.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-30.03 — Apps — network and content applications**
  - Action: Browser, download manager, package client, remote shell/file transfer and later mail/calendar/contact clients.
  - Requires: C-DA-30.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for DA-30.
- [ ] **C-DA-30.04 — Deliver — network and content applications**
  - Action: split UI, fetch/TLS and hostile parser/content processes with scoped network/file rights and privacy controls for history/bookmarks/cache.
  - Requires: C-DA-30.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-30.
- [ ] **C-DA-30.05 — Proof — network and content applications**
  - Action: parser/network service crash, malicious page/file/certificate, download path collision, revoked network, offline mode, cache/history deletion and no desktop or credential compromise.
  - Requires: C-DA-30.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-30.
- [ ] **C-DA-30.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-30. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-30.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-26"></a>
## C-VX-26 — Browser IPC and process topology

**Original requirement:** Browser IPC and process topology

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 474.

### Preserved original contract

**Depends on:** VX-07, VX-11, VX-16, VX-18.

**Deliver:** browser UI, navigation broker, network, site content, decoder and
origin-storage roles with generated bounded schemas.

**Invariants:** content has no ambient file/device/secret access; sender identity
and endpoint authority checked; quotas/backpressure/deadlines/cancel/peer death
defined; no pointer crosses IPC.

**Proof:** oversized/unknown messages, queue full, nth-handle transfer failure,
peer death at every request stage, forged origin/identity and leak census.

### Execution steps

- [ ] **C-VX-26.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-26.02 — Resolve this contract's exact dependencies**
  - Action: VX-07, VX-11, VX-16, VX-18. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-26.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-26.03 — Deliver — Browser IPC and process topology**
  - Action: browser UI, navigation broker, network, site content, decoder and origin-storage roles with generated bounded schemas.
  - Requires: C-VX-26.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-26.
- [ ] **C-VX-26.04 — Invariants — Browser IPC and process topology**
  - Action: content has no ambient file/device/secret access; sender identity and endpoint authority checked; quotas/backpressure/deadlines/cancel/peer death defined; no pointer crosses IPC.
  - Requires: C-VX-26.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-26.
- [ ] **C-VX-26.05 — Proof — Browser IPC and process topology**
  - Action: oversized/unknown messages, queue full, nth-handle transfer failure, peer death at every request stage, forged origin/identity and leak census.
  - Requires: C-VX-26.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-26.
- [ ] **C-VX-26.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-26. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-26.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-27"></a>
## C-VX-27 — Navigation and history transaction

**Original requirement:** Navigation and history transaction

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 488.

### Preserved original contract

**Depends on:** VX-26.

**Deliver:** one navigation ID and commit point across resolve/connect/TLS/
request/redirect/response/parse/subresources; back/forward/reload/stop/history.

**Invariants:** old page remains before commit; subresource state cannot replace
document status; redirect and history bounds; cancel is idempotent; stale
completion cannot commit.

**Proof:** cancel/race/redirect loops, Back during load, document then failing
stylesheet/image, wrong navigation ID, content crash and restored history.

### Execution steps

- [ ] **C-VX-27.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-27.02 — Resolve this contract's exact dependencies**
  - Action: VX-26. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-27.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-27.03 — Deliver — Navigation and history transaction**
  - Action: one navigation ID and commit point across resolve/connect/TLS/ request/redirect/response/parse/subresources; back/forward/reload/stop/history.
  - Requires: C-VX-27.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-27.
- [ ] **C-VX-27.04 — Invariants — Navigation and history transaction**
  - Action: old page remains before commit; subresource state cannot replace document status; redirect and history bounds; cancel is idempotent; stale completion cannot commit.
  - Requires: C-VX-27.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-27.
- [ ] **C-VX-27.05 — Proof — Navigation and history transaction**
  - Action: cancel/race/redirect loops, Back during load, document then failing stylesheet/image, wrong navigation ID, content crash and restored history.
  - Requires: C-VX-27.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-27.
- [ ] **C-VX-27.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-27. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-27.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-28"></a>
## C-VX-28 — Browser network, trust, cache and storage

**Original requirement:** Browser network, trust, cache and storage

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 502.

### Preserved original contract

**Depends on:** VX-26, Network/Secrets/Clock services.

**Deliver:** origin-aware requests, DNS/TCP/TLS, trust UI data, cookies/cache/
storage quotas, private mode, clear/site-data and download quarantine metadata.

**Invariants:** padlock requires verified name/time/root/policy; origin partitioning;
private data is destroyed; cache never bypasses current auth/validation;
downloads do not execute on arrival.

**Proof:** certificate matrix, bad clock/entropy/root, cross-origin storage,
cache poisoning/revalidation, quota, private cleanup, corrupt partial download.

### Execution steps

- [ ] **C-VX-28.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-28.02 — Resolve this contract's exact dependencies**
  - Action: VX-26, Network/Secrets/Clock services. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-28.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-28.03 — Deliver — Browser network, trust, cache and storage**
  - Action: origin-aware requests, DNS/TCP/TLS, trust UI data, cookies/cache/ storage quotas, private mode, clear/site-data and download quarantine metadata.
  - Requires: C-VX-28.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-28.
- [ ] **C-VX-28.04 — Invariants — Browser network, trust, cache and storage**
  - Action: padlock requires verified name/time/root/policy; origin partitioning; private data is destroyed; cache never bypasses current auth/validation; downloads do not execute on arrival.
  - Requires: C-VX-28.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-28.
- [ ] **C-VX-28.05 — Proof — Browser network, trust, cache and storage**
  - Action: certificate matrix, bad clock/entropy/root, cross-origin storage, cache poisoning/revalidation, quota, private cleanup, corrupt partial download.
  - Requires: C-VX-28.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-28.
- [ ] **C-VX-28.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-28. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-28.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-29"></a>
## C-VX-29 — Site content runtime

**Original requirement:** Site content runtime

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 516.

### Preserved original contract

**Depends on:** VX-26, VX-27.

**Deliver:** preserve current HTML/CSS/layout/PNG/bounded JS, then add DOM events,
forms, dynamic reflow and bounded task/timer loop.

**Invariants:** input lengths and output quotas; parser overflow says so;
script step/time/memory limits; layout arithmetic checked; origin-bound APIs;
crash cannot affect browser UI or another site.

**Proof:** current corpus differential, parser/layout/JS fuzz, DOM/event mutation,
timer storm, narrow/huge viewports, site-process crash and cross-site denial.

### Execution steps

- [ ] **C-VX-29.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-29.02 — Resolve this contract's exact dependencies**
  - Action: VX-26, VX-27. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-29.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-29.03 — Deliver — Site content runtime**
  - Action: preserve current HTML/CSS/layout/PNG/bounded JS, then add DOM events, forms, dynamic reflow and bounded task/timer loop.
  - Requires: C-VX-29.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-29.
- [ ] **C-VX-29.04 — Invariants — Site content runtime**
  - Action: input lengths and output quotas; parser overflow says so; script step/time/memory limits; layout arithmetic checked; origin-bound APIs; crash cannot affect browser UI or another site.
  - Requires: C-VX-29.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-29.
- [ ] **C-VX-29.05 — Proof — Site content runtime**
  - Action: current corpus differential, parser/layout/JS fuzz, DOM/event mutation, timer storm, narrow/huge viewports, site-process crash and cross-site denial.
  - Requires: C-VX-29.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-29.
- [ ] **C-VX-29.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-29. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-29.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-30"></a>
## C-VX-30 — Decoder worker family

**Original requirement:** Decoder worker family

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 530.

### Preserved original contract

**Depends on:** VX-26.

**Deliver:** image/font/media/document worker protocol with length-first input,
format sniff/admission, decoded-size quota, cancellation and sealed output.

**Invariants:** decoder has no network/file handles; declared dimensions cannot
overflow; partial output never publishes as success; process death invalidates
output handles.

**Proof:** malformed/truncated/bomb/huge dimension corpus, cancellation, worker
crash, output mismatch and repeated decode leak check.

### Execution steps

- [ ] **C-VX-30.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-30.02 — Resolve this contract's exact dependencies**
  - Action: VX-26. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-30.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-30.03 — Deliver — Decoder worker family**
  - Action: image/font/media/document worker protocol with length-first input, format sniff/admission, decoded-size quota, cancellation and sealed output.
  - Requires: C-VX-30.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-30.
- [ ] **C-VX-30.04 — Invariants — Decoder worker family**
  - Action: decoder has no network/file handles; declared dimensions cannot overflow; partial output never publishes as success; process death invalidates output handles.
  - Requires: C-VX-30.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-30.
- [ ] **C-VX-30.05 — Proof — Decoder worker family**
  - Action: malformed/truncated/bomb/huge dimension corpus, cancellation, worker crash, output mismatch and repeated decode leak check.
  - Requires: C-VX-30.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-30.
- [ ] **C-VX-30.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-30. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-30.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-31"></a>
## C-VX-31 — Browser chrome and permission UX

**Original requirement:** Browser chrome and permission UX

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 544.

### Preserved original contract

**Depends on:** VX-07..10, VX-26..30.

**Deliver:** tabs, address/search, progress/stop, identity, permission indicator,
zoom/find, downloads, bookmarks/history, private mode, errors and crash reload.

**Invariants:** displayed URL/origin follows committed navigation; page cannot
draw/spoof browser chrome; permission ownership/revocation visible; keyboard and
screen-reader complete.

**Proof:** spoof pages, fullscreen/capture, long IDN/RTL URL, permission revoke,
download failure, tab/site crash, keyboard-only and 200% text.

### Execution steps

- [ ] **C-VX-31.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-31.02 — Resolve this contract's exact dependencies**
  - Action: VX-07..10, VX-26..30. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-31.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-31.03 — Deliver — Browser chrome and permission UX**
  - Action: tabs, address/search, progress/stop, identity, permission indicator, zoom/find, downloads, bookmarks/history, private mode, errors and crash reload.
  - Requires: C-VX-31.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-31.
- [ ] **C-VX-31.04 — Invariants — Browser chrome and permission UX**
  - Action: displayed URL/origin follows committed navigation; page cannot draw/spoof browser chrome; permission ownership/revocation visible; keyboard and screen-reader complete.
  - Requires: C-VX-31.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-31.
- [ ] **C-VX-31.05 — Proof — Browser chrome and permission UX**
  - Action: spoof pages, fullscreen/capture, long IDN/RTL URL, permission revoke, download failure, tab/site crash, keyboard-only and 200% text.
  - Requires: C-VX-31.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-31.
- [ ] **C-VX-31.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-31. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-31.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-32"></a>
## C-VX-32 — Web compatibility ladder

**Original requirement:** Web compatibility ladder

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 558.

### Preserved original contract

**Depends on:** VX-29.

**Deliver:** versioned supported-feature manifest and fixed local corpora for
forms/events, selectors/media, script modules, formats and accessibility.

**Invariants:** unsupported is explicit; no silent widening; external corpus
version/digest pinned; compatibility growth cannot weaken sandbox or quotas.

**Proof:** positive/negative/mutation cases per feature; browser exposes exact
engine profile in diagnostics.

### Execution steps

- [ ] **C-VX-32.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-32.02 — Resolve this contract's exact dependencies**
  - Action: VX-29. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-32.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-32.03 — Deliver — Web compatibility ladder**
  - Action: versioned supported-feature manifest and fixed local corpora for forms/events, selectors/media, script modules, formats and accessibility.
  - Requires: C-VX-32.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-32.
- [ ] **C-VX-32.04 — Invariants — Web compatibility ladder**
  - Action: unsupported is explicit; no silent widening; external corpus version/digest pinned; compatibility growth cannot weaken sandbox or quotas.
  - Requires: C-VX-32.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-32.
- [ ] **C-VX-32.05 — Proof — Web compatibility ladder**
  - Action: positive/negative/mutation cases per feature; browser exposes exact engine profile in diagnostics.
  - Requires: C-VX-32.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-32.
- [ ] **C-VX-32.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-32. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-32.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-33"></a>
## C-VX-33 — Browser end-to-end and hostile-page gate

**Original requirement:** Browser end-to-end and hostile-page gate

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 571.

### Preserved original contract

**Depends on:** VX-26..32.

**Deliver:** repeatable host and QEMU origin servers, packet fault injection,
screenshot oracle, process/resource telemetry and crash recovery matrix.

**Proof:** real current Wikipedia-like fixed corpus, redirects, loss/reorder,
TLS failures, huge pages, malicious parsers/scripts, content/decoder/network
death and continued desktop/browser-chrome responsiveness.

### Execution steps

- [ ] **C-VX-33.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-33.02 — Resolve this contract's exact dependencies**
  - Action: VX-26..32. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-33.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-33.03 — Deliver — Browser end-to-end and hostile-page gate**
  - Action: repeatable host and QEMU origin servers, packet fault injection, screenshot oracle, process/resource telemetry and crash recovery matrix.
  - Requires: C-VX-33.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-33.
- [ ] **C-VX-33.04 — Proof — Browser end-to-end and hostile-page gate**
  - Action: real current Wikipedia-like fixed corpus, redirects, loss/reorder, TLS failures, huge pages, malicious parsers/scripts, content/decoder/network death and continued desktop/browser-chrome responsiveness.
  - Requires: C-VX-33.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-33.
- [ ] **C-VX-33.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-33. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-33.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-cur-005"></a>
## T-CUR-005 — Browser

**Original requirement:** split browser UI/network/content/decoder platform

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 34.

### Execution steps

- [ ] **T-CUR-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Browser to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-005.02 — Specify the complete target boundary**
  - Action: Browser must supply: split browser UI/network/content/decoder platform. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse Browser through the shared platform contract, delivering every part of: split browser UI/network/content/decoder platform. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-005.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation.
  - Requires: T-CUR-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Browser.
- [ ] **T-CUR-005.05 — Qualify and retain this target's own result**
  - Action: Bind Browser to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-069"></a>
## T-APP-069 — Download Manager

**Original requirement:** progress, resume, verification, destination and quarantine

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 201.

### Execution steps

- [ ] **T-APP-069.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Download Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-18, H-13.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-069.02 — Specify the complete target boundary**
  - Action: Download Manager must supply: progress, resume, verification, destination and quarantine. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-069.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-069.03 — Implement the exact target behavior**
  - Action: Implement or reuse Download Manager through the shared platform contract, delivering every part of: progress, resume, verification, destination and quarantine. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-069.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-069.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/oversized/reentrant content; nested parse limits; cross-origin access; denied storage/download; script runaway; renderer death; late network result after navigation; cancellation.
  - Requires: T-APP-069.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Download Manager.
- [ ] **T-APP-069.05 — Qualify and retain this target's own result**
  - Action: Bind Download Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-069.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
