# MP-14: Deliver the full application families as real workflows

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/apps/; kernel/src/graphics/; kernel/src/web/; stdlib/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-14` exports: One complete application workflow per family foundation; every named destination retains its own completion gate.

The handoff enables only its named subset. `CLOSE-14` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-14.01 — Sequence file/document/PDF/office tools after document models, restricted parsers and storage transactions

Sequence file/document/PDF/office tools after document models, restricted parsers and storage transactions.

**Requires:** `D-01`, `D-02`, `D-16`, `D-19`, `H-00`, `H-08`, `H-09`, `H-10`, `H-11`, `H-13`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-14.02 — Build spreadsheet calculation/import/export and presentation edit/play/export against explicit format and rendering contracts

Build spreadsheet calculation/import/export and presentation edit/play/export against explicit format and rendering contracts.

**Requires:** `M-14.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-14 — Bounded development handoff: Deliver the full application families as real workflows

One complete application workflow per family foundation; every named destination retains its own completion gate.

**Requires:** `M-14.02`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-14.03 — Deepen image, paint, vector, 3D, animation, audio and video workflows using owned media and decoder services

Deepen image, paint, vector, 3D, animation, audio and video workflows using owned media and decoder services.

**Requires:** `M-14.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-14.04 — Build mail/chat/calendar/contacts/tasks/notes and knowledge tools with account, offline, sync/conflict and privacy states

Build mail/chat/calendar/contacts/tasks/notes and knowledge tools with account, offline, sync/conflict and privacy states.

**Requires:** `M-14.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-14.05 — Deliver print/scan, maps/weather/RSS, accessibility and administration clients using real authorized providers

Deliver print/scan, maps/weather/RSS, accessibility and administration clients using real authorized providers.

**Requires:** `M-14.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-14.06 — For each named application, implement create/open/action/edit/save/export/close/reopen as applicable

For each named application, implement create/open/action/edit/save/export/close/reopen as applicable.

**Requires:** `M-14.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-14.07 — Test malformed content, denied grants, cancellation, service loss, disk full, crash recovery and update/uninstall

Test malformed content, denied grants, cancellation, service loss, disk full, crash recovery and update/uninstall.

**Requires:** `M-14.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-14.08 — Retain keyboard, assistive, locale, scaling and measured responsiveness requirements for every workflow, including error states

Retain keyboard, assistive, locale, scaling and measured responsiveness requirements for every workflow, including error states.

**Requires:** `M-14.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-PD-001](#f-pd-001) | feature | file manager |
| [F-PD-002](#f-pd-002) | feature | file properties |
| [F-PD-003](#f-pd-003) | feature | disk usage explorer |
| [F-PD-004](#f-pd-004) | feature | archive manager |
| [F-PD-005](#f-pd-005) | feature | text editor |
| [F-PD-006](#f-pd-006) | feature | code editor |
| [F-PD-007](#f-pd-007) | feature | notes |
| [F-PD-008](#f-pd-008) | feature | sticky notes |
| [F-PD-009](#f-pd-009) | feature | document viewer |
| [F-PD-010](#f-pd-010) | feature | PDF viewer |
| [F-PD-011](#f-pd-011) | feature | ebook/reader |
| [F-PD-012](#f-pd-012) | feature | word processor |
| [F-PD-013](#f-pd-013) | feature | spreadsheet |
| [F-PD-014](#f-pd-014) | feature | presentation editor |
| [F-PD-015](#f-pd-015) | feature | presentation viewer |
| [F-PD-016](#f-pd-016) | feature | plain/rich clipboard integration |
| [F-PD-017](#f-pd-017) | feature | spelling/dictionary |
| [F-PD-018](#f-pd-018) | feature | grammar/writing assistance |
| [F-PD-019](#f-pd-019) | feature | diff/merge |
| [F-PD-020](#f-pd-020) | feature | hex viewer/editor |
| [F-PD-021](#f-pd-021) | feature | checksum verifier |
| [F-PD-022](#f-pd-022) | feature | regex tester |
| [F-PD-023](#f-pd-023) | feature | calculator |
| [F-PD-024](#f-pd-024) | feature | unit converter |
| [F-PD-025](#f-pd-025) | feature | base converter |
| [F-PD-026](#f-pd-026) | feature | color picker |
| [F-PD-027](#f-pd-027) | feature | calendar |
| [F-PD-028](#f-pd-028) | feature | tasks/todo |
| [F-PD-029](#f-pd-029) | feature | contacts |
| [F-PD-030](#f-pd-030) | feature | knowledge base |
| [F-PD-031](#f-pd-031) | feature | file picker service app |
| [F-PD-032](#f-pd-032) | feature | print preview |
| [F-PD-033](#f-pd-033) | feature | print queue |
| [F-PD-034](#f-pd-034) | feature | scanner/document capture |
| [F-PD-035](#f-pd-035) | feature | OCR |
| [F-PD-036](#f-pd-036) | feature | recent documents |
| [F-PD-037](#f-pd-037) | feature | templates |
| [F-PD-038](#f-pd-038) | feature | import/export broker |
| [F-PD-039](#f-pd-039) | feature | document collaboration |
| [F-PD-040](#f-pd-040) | feature | document recovery center |
| [F-PD-041](#f-pd-041) | feature | cloud/network drive |
| [F-PD-042](#f-pd-042) | feature | cross-device document sync |
| [F-PD-043](#f-pd-043) | feature | citation/reference manager |
| [F-MD-001](#f-md-001) | feature | image viewer |
| [F-MD-002](#f-md-002) | feature | gallery/photo library |
| [F-MD-003](#f-md-003) | feature | paint/raster editor |
| [F-MD-004](#f-md-004) | feature | vector drawing |
| [F-MD-005](#f-md-005) | feature | screenshot editor |
| [F-MD-006](#f-md-006) | feature | audio player |
| [F-MD-007](#f-md-007) | feature | video player |
| [F-MD-008](#f-md-008) | feature | music library |
| [F-MD-009](#f-md-009) | feature | voice recorder |
| [F-MD-010](#f-md-010) | feature | audio editor |
| [F-MD-011](#f-md-011) | feature | synthesizer |
| [F-MD-012](#f-md-012) | feature | mixer |
| [F-MD-013](#f-md-013) | feature | camera app |
| [F-MD-014](#f-md-014) | feature | screen recorder |
| [F-MD-015](#f-md-015) | feature | media converter |
| [F-MD-016](#f-md-016) | feature | animation/demo studio |
| [F-MD-017](#f-md-017) | feature | 3D viewer/demo |
| [F-MD-018](#f-md-018) | feature | font atlas/inspector |
| [F-MD-019](#f-md-019) | feature | framebuffer/renderer inspector |
| [F-MD-020](#f-md-020) | feature | mail |
| [F-MD-021](#f-md-021) | feature | messaging/chat |
| [F-MD-022](#f-md-022) | feature | RSS/news reader |
| [F-MD-023](#f-md-023) | feature | maps |
| [F-MD-024](#f-md-024) | feature | weather |
| [F-MD-025](#f-md-025) | feature | clock |
| [F-MD-026](#f-md-026) | feature | timer/stopwatch |
| [F-MD-027](#f-md-027) | feature | alarms |
| [F-MD-028](#f-md-028) | feature | password manager UI |
| [F-MD-029](#f-md-029) | feature | download manager |
| [F-MD-030](#f-md-030) | feature | sharing center |
| [F-MD-031](#f-md-031) | feature | voice/video calling |
| [F-MD-032](#f-md-032) | feature | podcast/radio |
| [F-MD-033](#f-md-033) | feature | screen sharing |
| [F-ST-001](#f-st-001) | feature | Settings application |
| [F-ST-002](#f-st-002) | feature | display settings |
| [F-ST-003](#f-st-003) | feature | appearance settings |
| [F-ST-004](#f-st-004) | feature | input settings |
| [F-ST-005](#f-st-005) | feature | audio settings |
| [F-ST-006](#f-st-006) | feature | network settings |
| [F-ST-007](#f-st-007) | feature | accounts settings |
| [F-ST-008](#f-st-008) | feature | application settings |
| [F-ST-009](#f-st-009) | feature | accessibility settings |
| [F-ST-010](#f-st-010) | feature | locale/time settings |
| [F-ST-011](#f-st-011) | feature | power settings |
| [F-ST-012](#f-st-012) | feature | privacy settings |
| [F-ST-013](#f-st-013) | feature | update settings |
| [F-ST-014](#f-st-014) | feature | storage settings |
| [F-ST-015](#f-st-015) | feature | device manager |
| [F-ST-016](#f-st-016) | feature | printer/scanner settings |
| [F-ST-017](#f-st-017) | feature | system information |
| [F-ST-018](#f-st-018) | feature | system monitor |
| [F-ST-019](#f-st-019) | feature | process manager |
| [F-ST-020](#f-st-020) | feature | services manager |
| [F-ST-021](#f-st-021) | feature | driver monitor |
| [F-ST-022](#f-st-022) | feature | network monitor |
| [F-ST-023](#f-st-023) | feature | storage monitor |
| [F-ST-024](#f-st-024) | feature | graphics monitor |
| [F-ST-025](#f-st-025) | feature | audio monitor |
| [F-ST-026](#f-st-026) | feature | kernel/system log viewer |
| [F-ST-027](#f-st-027) | feature | crash center |
| [F-ST-028](#f-st-028) | feature | benchmark app |
| [F-ST-029](#f-st-029) | feature | diagnostics suite |
| [F-ST-030](#f-st-030) | feature | health dashboard |
| [F-ST-031](#f-st-031) | feature | disk repair UI |
| [F-ST-032](#f-st-032) | feature | boot recovery UI |
| [F-ST-033](#f-st-033) | feature | factory/user reset |
| [F-ST-034](#f-st-034) | feature | support bundle |
| [F-ST-035](#f-st-035) | feature | About/product identity |
| [F-ST-036](#f-st-036) | feature | security center |
| [F-ST-037](#f-st-037) | feature | active sessions |
| [C-P8.5](#c-p8-5) | contract | system driver diagnostics |
| [C-DA-31O](#c-da-31o) | contract | office document creation suite |
| [C-DA-33](#c-da-33) | contract | media applications |
| [C-DA-33P](#c-da-33p) | contract | print, spool and scan services/apps |
| [C-DA-34](#c-da-34) | contract | software centre and provenance UI |
| [C-VX-34](#c-vx-34) | contract | Welcome, account and onboarding |
| [C-VX-35](#c-vx-35) | contract | Device and permission manager |
| [C-VX-36](#c-vx-36) | contract | Software, update and rollback UI |
| [C-VX-37](#c-vx-37) | contract | Crash and recovery center |
| [C-VX-38](#c-vx-38) | contract | Document and PDF family |
| [C-VX-39](#c-vx-39) | contract | Spreadsheet |
| [C-VX-40](#c-vx-40) | contract | Presentation |
| [C-VX-41](#c-vx-41) | contract | Print, spool and scan |
| [C-VX-42](#c-vx-42) | contract | Communications and personal information |
| [C-VX-43](#c-vx-43) | contract | Media, audio and creation |
| [C-VX-44](#c-vx-44) | contract | Camera and capture |
| [C-VX-45](#c-vx-45) | contract | Development workbench |
| [T-USB-013](#t-usb-013) | target | USB printer class |
| [T-PERIPH-001](#t-periph-001) | target | printer provider |
| [T-PERIPH-002](#t-periph-002) | target | scanner provider |
| [T-SVC-094](#t-svc-094) | target | Media Library/Metadata Service |
| [T-SVC-097](#t-svc-097) | target | Communication Notification Broker |
| [T-SVC-098](#t-svc-098) | target | Realtime Call/Conference Service |
| [T-SVC-104](#t-svc-104) | target | Print Spooler |
| [T-SVC-105](#t-svc-105) | target | Scan Service |
| [T-SVC-108](#t-svc-108) | target | Spellcheck/Dictionary Service |
| [T-SVC-109](#t-svc-109) | target | Calendar/Alarm Service |
| [T-SVC-110](#t-svc-110) | target | Contacts Service |
| [T-SVC-111](#t-svc-111) | target | Mail Service |
| [T-SVC-112](#t-svc-112) | target | Messaging/Presence Service |
| [T-APP-001](#t-app-001) | target | Welcome/Onboarding |
| [T-APP-004](#t-app-004) | target | Notification Center |
| [T-APP-005](#t-app-005) | target | Background Tasks |
| [T-APP-006](#t-app-006) | target | Permissions/Privacy Dashboard |
| [T-APP-007](#t-app-007) | target | User and Account Manager |
| [T-APP-010](#t-app-010) | target | Device Manager |
| [T-APP-011](#t-app-011) | target | Software Center |
| [T-APP-012](#t-app-012) | target | Update and Rollback Center |
| [T-APP-013](#t-app-013) | target | Crash/Recovery Center |
| [T-APP-014](#t-app-014) | target | Power/Battery Center |
| [T-APP-015](#t-app-015) | target | Removable Media Center |
| [T-APP-020](#t-app-020) | target | Advanced File Manager |
| [T-APP-021](#t-app-021) | target | Document Viewer |
| [T-APP-022](#t-app-022) | target | PDF Viewer |
| [T-APP-023](#t-app-023) | target | Ebook Reader |
| [T-APP-024](#t-app-024) | target | Word Processor |
| [T-APP-025](#t-app-025) | target | Spreadsheet |
| [T-APP-026](#t-app-026) | target | Presentation Editor/Player |
| [T-APP-027](#t-app-027) | target | Notes/Knowledge Base |
| [T-APP-028](#t-app-028) | target | Markdown Editor/Preview |
| [T-APP-029](#t-app-029) | target | PDF/Document Annotator |
| [T-APP-030](#t-app-030) | target | OCR/Document Capture |
| [T-APP-031](#t-app-031) | target | Archive/Compression Workbench |
| [T-APP-032](#t-app-032) | target | Search |
| [T-APP-033](#t-app-033) | target | Dictionary/Thesaurus |
| [T-APP-034](#t-app-034) | target | Font Manager |
| [T-APP-035](#t-app-035) | target | Print Queue and Printer Setup |
| [T-APP-036](#t-app-036) | target | Scanner |
| [T-APP-040](#t-app-040) | target | Media Library/Player |
| [T-APP-041](#t-app-041) | target | Music Player |
| [T-APP-042](#t-app-042) | target | Video Player |
| [T-APP-043](#t-app-043) | target | Audio Recorder |
| [T-APP-044](#t-app-044) | target | Audio Mixer/Patchbay |
| [T-APP-045](#t-app-045) | target | Music/Synth/Sequencer |
| [T-APP-046](#t-app-046) | target | Camera |
| [T-APP-047](#t-app-047) | target | Screen Capture |
| [T-APP-048](#t-app-048) | target | Screen Recorder |
| [T-APP-049](#t-app-049) | target | Advanced Raster Editor |
| [T-APP-050](#t-app-050) | target | Vector Graphics Editor |
| [T-APP-051](#t-app-051) | target | 3D Model/Scene Viewer |
| [T-APP-052](#t-app-052) | target | Animation Editor/Player |
| [T-APP-053](#t-app-053) | target | Video Editor |
| [T-APP-054](#t-app-054) | target | Image Metadata/Batch Tool |
| [T-APP-055](#t-app-055) | target | Voice/Podcast Tool |
| [T-APP-060](#t-app-060) | target | Mail |
| [T-APP-061](#t-app-061) | target | Calendar |
| [T-APP-062](#t-app-062) | target | Contacts |
| [T-APP-063](#t-app-063) | target | Tasks/Reminders |
| [T-APP-064](#t-app-064) | target | Messaging/Chat |
| [T-APP-065](#t-app-065) | target | Calls/Conference |
| [T-APP-066](#t-app-066) | target | RSS/News Reader |
| [T-APP-067](#t-app-067) | target | Weather |
| [T-APP-068](#t-app-068) | target | Maps |
| [T-APP-070](#t-app-070) | target | Remote Desktop Client |

<a id="f-pd-001"></a>
## F-PD-001 — file manager

**Original requirement:** places/tree/list/grid, search, open, copy/move/rename/trash, progress/cancel and permissions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-001.01 — Reconcile existing file manager**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file manager. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: places/tree/list/grid, search, open, copy/move/rename/trash, progress/cancel and permissions
- [ ] **F-PD-001.02 — Freeze the exact contract for file manager**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: places/tree/list/grid, search, open, copy/move/rename/trash, progress/cancel and permissions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-001.03 — Implement/prove: places/tree/list/grid**
  - Action: For file manager, implement or reuse and verify this exact obligation: places/tree/list/grid. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for places/tree/list/grid; retain observable state/resource expectations.
- [ ] **F-PD-001.04 — Implement/prove: search**
  - Action: For file manager, implement or reuse and verify this exact obligation: search. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for search; retain observable state/resource expectations.
- [ ] **F-PD-001.05 — Implement/prove: open**
  - Action: For file manager, implement or reuse and verify this exact obligation: open. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for open; retain observable state/resource expectations.
- [ ] **F-PD-001.06 — Implement/prove: copy/move/rename/trash**
  - Action: For file manager, implement or reuse and verify this exact obligation: copy/move/rename/trash. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for copy/move/rename/trash; retain observable state/resource expectations.
- [ ] **F-PD-001.07 — Implement/prove: progress/cancel and permissions**
  - Action: For file manager, implement or reuse and verify this exact obligation: progress/cancel and permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for progress/cancel and permissions; retain observable state/resource expectations.
- [ ] **F-PD-001.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file manager: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-001.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-001.09 — Integrate into the real consumer and runtime route**
  - Action: Wire file manager into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-001.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-001.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file manager as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-001.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-002"></a>
## F-PD-002 — file properties

**Original requirement:** type/size/location/times/owner/permissions/hash/provenance and safe edits

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-002.01 — Reconcile existing file properties**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file properties. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: type/size/location/times/owner/permissions/hash/provenance and safe edits
- [ ] **F-PD-002.02 — Freeze the exact contract for file properties**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: type/size/location/times/owner/permissions/hash/provenance and safe edits. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-002.03 — Implement/prove: type/size/location/times/owner/permissions/hash/provenance and safe edits**
  - Action: For file properties, implement or reuse and verify this exact obligation: type/size/location/times/owner/permissions/hash/provenance and safe edits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for type/size/location/times/owner/permissions/hash/provenance and safe edits; retain observable state/resource expectations.
- [ ] **F-PD-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file properties: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire file properties into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file properties as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-003"></a>
## F-PD-003 — disk usage explorer

**Original requirement:** permission-aware cancelable snapshot with largest consumers and removable media

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-003.01 — Reconcile existing disk usage explorer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for disk usage explorer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: permission-aware cancelable snapshot with largest consumers and removable media
- [ ] **F-PD-003.02 — Freeze the exact contract for disk usage explorer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: permission-aware cancelable snapshot with largest consumers and removable media. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-003.03 — Implement/prove: permission-aware cancelable snapshot with largest consumers and removable media**
  - Action: For disk usage explorer, implement or reuse and verify this exact obligation: permission-aware cancelable snapshot with largest consumers and removable media. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permission-aware cancelable snapshot with largest consumers and removable media; retain observable state/resource expectations.
- [ ] **F-PD-003.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to disk usage explorer: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-003.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-003.05 — Integrate into the real consumer and runtime route**
  - Action: Wire disk usage explorer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-003.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-003.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for disk usage explorer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-003.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-004"></a>
## F-PD-004 — archive manager

**Original requirement:** list/create/extract/test with traversal/link/bomb checks and transactional destination

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-004.01 — Reconcile existing archive manager**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for archive manager. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: list/create/extract/test with traversal/link/bomb checks and transactional destination
- [ ] **F-PD-004.02 — Freeze the exact contract for archive manager**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: list/create/extract/test with traversal/link/bomb checks and transactional destination. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-004.03 — Implement/prove: list/create/extract/test with traversal/link/bomb checks and transactional destination**
  - Action: For archive manager, implement or reuse and verify this exact obligation: list/create/extract/test with traversal/link/bomb checks and transactional destination. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for list/create/extract/test with traversal/link/bomb checks and transactional destination; retain observable state/resource expectations.
- [ ] **F-PD-004.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to archive manager: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-004.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-004.05 — Integrate into the real consumer and runtime route**
  - Action: Wire archive manager into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-004.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-004.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for archive manager as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-004.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-005"></a>
## F-PD-005 — text editor

**Original requirement:** multi-document text, selection, find/replace, undo, encoding, autosave/recovery, atomic save

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-005.01 — Reconcile existing text editor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for text editor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: multi-document text, selection, find/replace, undo, encoding, autosave/recovery, atomic save
- [ ] **F-PD-005.02 — Freeze the exact contract for text editor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: multi-document text, selection, find/replace, undo, encoding, autosave/recovery, atomic save. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-005.03 — Implement/prove: multi-document text**
  - Action: For text editor, implement or reuse and verify this exact obligation: multi-document text. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for multi-document text; retain observable state/resource expectations.
- [ ] **F-PD-005.04 — Implement/prove: selection**
  - Action: For text editor, implement or reuse and verify this exact obligation: selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for selection; retain observable state/resource expectations.
- [ ] **F-PD-005.05 — Implement/prove: find/replace**
  - Action: For text editor, implement or reuse and verify this exact obligation: find/replace. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for find/replace; retain observable state/resource expectations.
- [ ] **F-PD-005.06 — Implement/prove: undo**
  - Action: For text editor, implement or reuse and verify this exact obligation: undo. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for undo; retain observable state/resource expectations.
- [ ] **F-PD-005.07 — Implement/prove: encoding**
  - Action: For text editor, implement or reuse and verify this exact obligation: encoding. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-005.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for encoding; retain observable state/resource expectations.
- [ ] **F-PD-005.08 — Implement/prove: autosave/recovery**
  - Action: For text editor, implement or reuse and verify this exact obligation: autosave/recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-005.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for autosave/recovery; retain observable state/resource expectations.
- [ ] **F-PD-005.09 — Implement/prove: atomic save**
  - Action: For text editor, implement or reuse and verify this exact obligation: atomic save. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-005.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atomic save; retain observable state/resource expectations.
- [ ] **F-PD-005.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to text editor: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-005.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-005.11 — Integrate into the real consumer and runtime route**
  - Action: Wire text editor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-005.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-005.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for text editor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-005.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-006"></a>
## F-PD-006 — code editor

**Original requirement:** syntax/LSP-like diagnostics, project search, symbols, build/debug and extension isolation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-006.01 — Reconcile existing code editor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for code editor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: syntax/LSP-like diagnostics, project search, symbols, build/debug and extension isolation
- [ ] **F-PD-006.02 — Freeze the exact contract for code editor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: syntax/LSP-like diagnostics, project search, symbols, build/debug and extension isolation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-006.03 — Implement/prove: syntax/LSP-like diagnostics**
  - Action: For code editor, implement or reuse and verify this exact obligation: syntax/LSP-like diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for syntax/LSP-like diagnostics; retain observable state/resource expectations.
- [ ] **F-PD-006.04 — Implement/prove: project search**
  - Action: For code editor, implement or reuse and verify this exact obligation: project search. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for project search; retain observable state/resource expectations.
- [ ] **F-PD-006.05 — Implement/prove: symbols**
  - Action: For code editor, implement or reuse and verify this exact obligation: symbols. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for symbols; retain observable state/resource expectations.
- [ ] **F-PD-006.06 — Implement/prove: build/debug and extension isolation**
  - Action: For code editor, implement or reuse and verify this exact obligation: build/debug and extension isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-006.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for build/debug and extension isolation; retain observable state/resource expectations.
- [ ] **F-PD-006.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to code editor: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-006.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-006.08 — Integrate into the real consumer and runtime route**
  - Action: Wire code editor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-006.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-006.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for code editor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-006.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-007"></a>
## F-PD-007 — notes

**Original requirement:** durable Markdown/rich notes, folders/tags/search/links/autosave/export and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-007.01 — Reconcile existing notes**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for notes. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: durable Markdown/rich notes, folders/tags/search/links/autosave/export and privacy
- [ ] **F-PD-007.02 — Freeze the exact contract for notes**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: durable Markdown/rich notes, folders/tags/search/links/autosave/export and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-007.03 — Implement/prove: durable Markdown/rich notes**
  - Action: For notes, implement or reuse and verify this exact obligation: durable Markdown/rich notes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for durable Markdown/rich notes; retain observable state/resource expectations.
- [ ] **F-PD-007.04 — Implement/prove: folders/tags/search/links/autosave/export and privacy**
  - Action: For notes, implement or reuse and verify this exact obligation: folders/tags/search/links/autosave/export and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for folders/tags/search/links/autosave/export and privacy; retain observable state/resource expectations.
- [ ] **F-PD-007.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to notes: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-007.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-007.06 — Integrate into the real consumer and runtime route**
  - Action: Wire notes into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-007.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-007.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for notes as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-007.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-008"></a>
## F-PD-008 — sticky notes

**Original requirement:** small persistent desktop notes, per-user state, recovery and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-008.01 — Reconcile existing sticky notes**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for sticky notes. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: small persistent desktop notes, per-user state, recovery and privacy
- [ ] **F-PD-008.02 — Freeze the exact contract for sticky notes**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: small persistent desktop notes, per-user state, recovery and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-008.03 — Implement/prove: small persistent desktop notes**
  - Action: For sticky notes, implement or reuse and verify this exact obligation: small persistent desktop notes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for small persistent desktop notes; retain observable state/resource expectations.
- [ ] **F-PD-008.04 — Implement/prove: per-user state**
  - Action: For sticky notes, implement or reuse and verify this exact obligation: per-user state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-user state; retain observable state/resource expectations.
- [ ] **F-PD-008.05 — Implement/prove: recovery and privacy**
  - Action: For sticky notes, implement or reuse and verify this exact obligation: recovery and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recovery and privacy; retain observable state/resource expectations.
- [ ] **F-PD-008.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to sticky notes: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-008.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-008.07 — Integrate into the real consumer and runtime route**
  - Action: Wire sticky notes into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-008.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-008.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for sticky notes as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-008.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-009"></a>
## F-PD-009 — document viewer

**Original requirement:** paged/reflow text, outline/search/select/copy/annotations and accessibility

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-009.01 — Reconcile existing document viewer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for document viewer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: paged/reflow text, outline/search/select/copy/annotations and accessibility
- [ ] **F-PD-009.02 — Freeze the exact contract for document viewer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: paged/reflow text, outline/search/select/copy/annotations and accessibility. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-009.03 — Implement/prove: paged/reflow text**
  - Action: For document viewer, implement or reuse and verify this exact obligation: paged/reflow text. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for paged/reflow text; retain observable state/resource expectations.
- [ ] **F-PD-009.04 — Implement/prove: outline/search/select/copy/annotations and accessibility**
  - Action: For document viewer, implement or reuse and verify this exact obligation: outline/search/select/copy/annotations and accessibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for outline/search/select/copy/annotations and accessibility; retain observable state/resource expectations.
- [ ] **F-PD-009.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to document viewer: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-009.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-009.06 — Integrate into the real consumer and runtime route**
  - Action: Wire document viewer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-009.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-009.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for document viewer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-009.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-010"></a>
## F-PD-010 — PDF viewer

**Original requirement:** isolated parser/rasterizer, pages/search/text/links/forms/annotations/print

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-010.01 — Reconcile existing PDF viewer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for PDF viewer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated parser/rasterizer, pages/search/text/links/forms/annotations/print
- [ ] **F-PD-010.02 — Freeze the exact contract for PDF viewer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated parser/rasterizer, pages/search/text/links/forms/annotations/print. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-010.03 — Implement/prove: isolated parser/rasterizer**
  - Action: For PDF viewer, implement or reuse and verify this exact obligation: isolated parser/rasterizer. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated parser/rasterizer; retain observable state/resource expectations.
- [ ] **F-PD-010.04 — Implement/prove: pages/search/text/links/forms/annotations/print**
  - Action: For PDF viewer, implement or reuse and verify this exact obligation: pages/search/text/links/forms/annotations/print. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pages/search/text/links/forms/annotations/print; retain observable state/resource expectations.
- [ ] **F-PD-010.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to PDF viewer: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-010.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-010.06 — Integrate into the real consumer and runtime route**
  - Action: Wire PDF viewer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-010.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-010.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for PDF viewer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-010.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-011"></a>
## F-PD-011 — ebook/reader

**Original requirement:** EPUB-like package/parser isolation, navigation, typography, bookmarks and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-011.01 — Reconcile existing ebook/reader**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for ebook/reader. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: EPUB-like package/parser isolation, navigation, typography, bookmarks and a11y
- [ ] **F-PD-011.02 — Freeze the exact contract for ebook/reader**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: EPUB-like package/parser isolation, navigation, typography, bookmarks and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-011.03 — Implement/prove: EPUB-like package/parser isolation**
  - Action: For ebook/reader, implement or reuse and verify this exact obligation: EPUB-like package/parser isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for EPUB-like package/parser isolation; retain observable state/resource expectations.
- [ ] **F-PD-011.04 — Implement/prove: navigation**
  - Action: For ebook/reader, implement or reuse and verify this exact obligation: navigation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for navigation; retain observable state/resource expectations.
- [ ] **F-PD-011.05 — Implement/prove: typography**
  - Action: For ebook/reader, implement or reuse and verify this exact obligation: typography. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typography; retain observable state/resource expectations.
- [ ] **F-PD-011.06 — Implement/prove: bookmarks and a11y**
  - Action: For ebook/reader, implement or reuse and verify this exact obligation: bookmarks and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-011.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bookmarks and a11y; retain observable state/resource expectations.
- [ ] **F-PD-011.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to ebook/reader: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-011.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-011.08 — Integrate into the real consumer and runtime route**
  - Action: Wire ebook/reader into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-011.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-011.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for ebook/reader as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-011.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-012"></a>
## F-PD-012 — word processor

**Original requirement:** structured document editing, styles, layout, tables/images, change tracking, import/export

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-012.01 — Reconcile existing word processor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for word processor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: structured document editing, styles, layout, tables/images, change tracking, import/export
- [ ] **F-PD-012.02 — Freeze the exact contract for word processor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: structured document editing, styles, layout, tables/images, change tracking, import/export. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-012.03 — Implement/prove: structured document editing**
  - Action: For word processor, implement or reuse and verify this exact obligation: structured document editing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for structured document editing; retain observable state/resource expectations.
- [ ] **F-PD-012.04 — Implement/prove: styles**
  - Action: For word processor, implement or reuse and verify this exact obligation: styles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for styles; retain observable state/resource expectations.
- [ ] **F-PD-012.05 — Implement/prove: layout**
  - Action: For word processor, implement or reuse and verify this exact obligation: layout. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for layout; retain observable state/resource expectations.
- [ ] **F-PD-012.06 — Implement/prove: tables/images**
  - Action: For word processor, implement or reuse and verify this exact obligation: tables/images. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-012.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tables/images; retain observable state/resource expectations.
- [ ] **F-PD-012.07 — Implement/prove: change tracking**
  - Action: For word processor, implement or reuse and verify this exact obligation: change tracking. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-012.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for change tracking; retain observable state/resource expectations.
- [ ] **F-PD-012.08 — Implement/prove: import/export**
  - Action: For word processor, implement or reuse and verify this exact obligation: import/export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-012.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for import/export; retain observable state/resource expectations.
- [ ] **F-PD-012.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to word processor: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-012.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-012.10 — Integrate into the real consumer and runtime route**
  - Action: Wire word processor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-012.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-012.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for word processor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-012.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-013"></a>
## F-PD-013 — spreadsheet

**Original requirement:** typed cells/formulas/dependencies/recalc, sheets, charting, import/export and accessibility

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-013.01 — Reconcile existing spreadsheet**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for spreadsheet. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: typed cells/formulas/dependencies/recalc, sheets, charting, import/export and accessibility
- [ ] **F-PD-013.02 — Freeze the exact contract for spreadsheet**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: typed cells/formulas/dependencies/recalc, sheets, charting, import/export and accessibility. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-013.03 — Implement/prove: typed cells/formulas/dependencies/recalc**
  - Action: For spreadsheet, implement or reuse and verify this exact obligation: typed cells/formulas/dependencies/recalc. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typed cells/formulas/dependencies/recalc; retain observable state/resource expectations.
- [ ] **F-PD-013.04 — Implement/prove: sheets**
  - Action: For spreadsheet, implement or reuse and verify this exact obligation: sheets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sheets; retain observable state/resource expectations.
- [ ] **F-PD-013.05 — Implement/prove: charting**
  - Action: For spreadsheet, implement or reuse and verify this exact obligation: charting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for charting; retain observable state/resource expectations.
- [ ] **F-PD-013.06 — Implement/prove: import/export and accessibility**
  - Action: For spreadsheet, implement or reuse and verify this exact obligation: import/export and accessibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-013.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for import/export and accessibility; retain observable state/resource expectations.
- [ ] **F-PD-013.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to spreadsheet: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-013.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-013.08 — Integrate into the real consumer and runtime route**
  - Action: Wire spreadsheet into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-013.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-013.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for spreadsheet as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-013.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-014"></a>
## F-PD-014 — presentation editor

**Original requirement:** slides/layouts/themes/media/notes/presenter view, import/export and playback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-014.01 — Reconcile existing presentation editor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for presentation editor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: slides/layouts/themes/media/notes/presenter view, import/export and playback
- [ ] **F-PD-014.02 — Freeze the exact contract for presentation editor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: slides/layouts/themes/media/notes/presenter view, import/export and playback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-014.03 — Implement/prove: slides/layouts/themes/media/notes/presenter view**
  - Action: For presentation editor, implement or reuse and verify this exact obligation: slides/layouts/themes/media/notes/presenter view. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for slides/layouts/themes/media/notes/presenter view; retain observable state/resource expectations.
- [ ] **F-PD-014.04 — Implement/prove: import/export and playback**
  - Action: For presentation editor, implement or reuse and verify this exact obligation: import/export and playback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for import/export and playback; retain observable state/resource expectations.
- [ ] **F-PD-014.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to presentation editor: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-014.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-014.06 — Integrate into the real consumer and runtime route**
  - Action: Wire presentation editor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-014.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-014.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for presentation editor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-014.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-015"></a>
## F-PD-015 — presentation viewer

**Original requirement:** safe deck playback, display selection, controls and accessible alternatives

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-015.01 — Reconcile existing presentation viewer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for presentation viewer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: safe deck playback, display selection, controls and accessible alternatives
- [ ] **F-PD-015.02 — Freeze the exact contract for presentation viewer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: safe deck playback, display selection, controls and accessible alternatives. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-015.03 — Implement/prove: safe deck playback**
  - Action: For presentation viewer, implement or reuse and verify this exact obligation: safe deck playback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe deck playback; retain observable state/resource expectations.
- [ ] **F-PD-015.04 — Implement/prove: display selection**
  - Action: For presentation viewer, implement or reuse and verify this exact obligation: display selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for display selection; retain observable state/resource expectations.
- [ ] **F-PD-015.05 — Implement/prove: controls and accessible alternatives**
  - Action: For presentation viewer, implement or reuse and verify this exact obligation: controls and accessible alternatives. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-015.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for controls and accessible alternatives; retain observable state/resource expectations.
- [ ] **F-PD-015.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to presentation viewer: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-015.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-015.07 — Integrate into the real consumer and runtime route**
  - Action: Wire presentation viewer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-015.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-015.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for presentation viewer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-015.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-016"></a>
## F-PD-016 — plain/rich clipboard integration

**Original requirement:** format offers, sanitization, source attribution and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-016.01 — Reconcile existing plain/rich clipboard integration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for plain/rich clipboard integration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: format offers, sanitization, source attribution and privacy
- [ ] **F-PD-016.02 — Freeze the exact contract for plain/rich clipboard integration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: format offers, sanitization, source attribution and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-016.03 — Implement/prove: format offers**
  - Action: For plain/rich clipboard integration, implement or reuse and verify this exact obligation: format offers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for format offers; retain observable state/resource expectations.
- [ ] **F-PD-016.04 — Implement/prove: sanitization**
  - Action: For plain/rich clipboard integration, implement or reuse and verify this exact obligation: sanitization. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sanitization; retain observable state/resource expectations.
- [ ] **F-PD-016.05 — Implement/prove: source attribution and privacy**
  - Action: For plain/rich clipboard integration, implement or reuse and verify this exact obligation: source attribution and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source attribution and privacy; retain observable state/resource expectations.
- [ ] **F-PD-016.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to plain/rich clipboard integration: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-016.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-016.07 — Integrate into the real consumer and runtime route**
  - Action: Wire plain/rich clipboard integration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-016.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-016.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for plain/rich clipboard integration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-016.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-017"></a>
## F-PD-017 — spelling/dictionary

**Original requirement:** language dictionaries, suggestions, custom words, provenance and offline behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-017.01 — Reconcile existing spelling/dictionary**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for spelling/dictionary. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: language dictionaries, suggestions, custom words, provenance and offline behavior
- [ ] **F-PD-017.02 — Freeze the exact contract for spelling/dictionary**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: language dictionaries, suggestions, custom words, provenance and offline behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-017.03 — Implement/prove: language dictionaries**
  - Action: For spelling/dictionary, implement or reuse and verify this exact obligation: language dictionaries. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for language dictionaries; retain observable state/resource expectations.
- [ ] **F-PD-017.04 — Implement/prove: suggestions**
  - Action: For spelling/dictionary, implement or reuse and verify this exact obligation: suggestions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for suggestions; retain observable state/resource expectations.
- [ ] **F-PD-017.05 — Implement/prove: custom words**
  - Action: For spelling/dictionary, implement or reuse and verify this exact obligation: custom words. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for custom words; retain observable state/resource expectations.
- [ ] **F-PD-017.06 — Implement/prove: provenance and offline behavior**
  - Action: For spelling/dictionary, implement or reuse and verify this exact obligation: provenance and offline behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-017.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance and offline behavior; retain observable state/resource expectations.
- [ ] **F-PD-017.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to spelling/dictionary: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-017.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-017.08 — Integrate into the real consumer and runtime route**
  - Action: Wire spelling/dictionary into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-017.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-017.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for spelling/dictionary as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-017.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-018"></a>
## F-PD-018 — grammar/writing assistance

**Original requirement:** local/remote policy, provenance, privacy, reversible suggestions and accessibility

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-018.01 — Reconcile existing grammar/writing assistance**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for grammar/writing assistance. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: local/remote policy, provenance, privacy, reversible suggestions and accessibility
- [ ] **F-PD-018.02 — Freeze the exact contract for grammar/writing assistance**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: local/remote policy, provenance, privacy, reversible suggestions and accessibility. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-018.03 — Implement/prove: local/remote policy**
  - Action: For grammar/writing assistance, implement or reuse and verify this exact obligation: local/remote policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for local/remote policy; retain observable state/resource expectations.
- [ ] **F-PD-018.04 — Implement/prove: provenance**
  - Action: For grammar/writing assistance, implement or reuse and verify this exact obligation: provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance; retain observable state/resource expectations.
- [ ] **F-PD-018.05 — Implement/prove: privacy**
  - Action: For grammar/writing assistance, implement or reuse and verify this exact obligation: privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy; retain observable state/resource expectations.
- [ ] **F-PD-018.06 — Implement/prove: reversible suggestions and accessibility**
  - Action: For grammar/writing assistance, implement or reuse and verify this exact obligation: reversible suggestions and accessibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-018.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reversible suggestions and accessibility; retain observable state/resource expectations.
- [ ] **F-PD-018.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to grammar/writing assistance: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-018.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-018.08 — Integrate into the real consumer and runtime route**
  - Action: Wire grammar/writing assistance into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-018.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-018.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for grammar/writing assistance as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-018.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-019"></a>
## F-PD-019 — diff/merge

**Original requirement:** scalable line/word/syntax compare, files, conflicts, apply and undo

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-019.01 — Reconcile existing diff/merge**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for diff/merge. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: scalable line/word/syntax compare, files, conflicts, apply and undo
- [ ] **F-PD-019.02 — Freeze the exact contract for diff/merge**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: scalable line/word/syntax compare, files, conflicts, apply and undo. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-019.03 — Implement/prove: scalable line/word/syntax compare**
  - Action: For diff/merge, implement or reuse and verify this exact obligation: scalable line/word/syntax compare. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scalable line/word/syntax compare; retain observable state/resource expectations.
- [ ] **F-PD-019.04 — Implement/prove: files**
  - Action: For diff/merge, implement or reuse and verify this exact obligation: files. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for files; retain observable state/resource expectations.
- [ ] **F-PD-019.05 — Implement/prove: conflicts**
  - Action: For diff/merge, implement or reuse and verify this exact obligation: conflicts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conflicts; retain observable state/resource expectations.
- [ ] **F-PD-019.06 — Implement/prove: apply and undo**
  - Action: For diff/merge, implement or reuse and verify this exact obligation: apply and undo. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-019.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for apply and undo; retain observable state/resource expectations.
- [ ] **F-PD-019.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to diff/merge: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-019.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-019.08 — Integrate into the real consumer and runtime route**
  - Action: Wire diff/merge into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-019.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-019.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for diff/merge as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-019.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-020"></a>
## F-PD-020 — hex viewer/editor

**Original requirement:** paged huge files/devices, offsets/search/types, read-only default and guarded writes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-020.01 — Reconcile existing hex viewer/editor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for hex viewer/editor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: paged huge files/devices, offsets/search/types, read-only default and guarded writes
- [ ] **F-PD-020.02 — Freeze the exact contract for hex viewer/editor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: paged huge files/devices, offsets/search/types, read-only default and guarded writes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-020.03 — Implement/prove: paged huge files/devices**
  - Action: For hex viewer/editor, implement or reuse and verify this exact obligation: paged huge files/devices. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for paged huge files/devices; retain observable state/resource expectations.
- [ ] **F-PD-020.04 — Implement/prove: offsets/search/types**
  - Action: For hex viewer/editor, implement or reuse and verify this exact obligation: offsets/search/types. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for offsets/search/types; retain observable state/resource expectations.
- [ ] **F-PD-020.05 — Implement/prove: read-only default and guarded writes**
  - Action: For hex viewer/editor, implement or reuse and verify this exact obligation: read-only default and guarded writes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-020.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for read-only default and guarded writes; retain observable state/resource expectations.
- [ ] **F-PD-020.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to hex viewer/editor: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-020.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-020.07 — Integrate into the real consumer and runtime route**
  - Action: Wire hex viewer/editor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-020.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-020.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for hex viewer/editor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-020.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-021"></a>
## F-PD-021 — checksum verifier

**Original requirement:** streaming algorithms, compare workflow, file handles, provenance and clear mismatch

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-021.01 — Reconcile existing checksum verifier**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for checksum verifier. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: streaming algorithms, compare workflow, file handles, provenance and clear mismatch
- [ ] **F-PD-021.02 — Freeze the exact contract for checksum verifier**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: streaming algorithms, compare workflow, file handles, provenance and clear mismatch. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-021.03 — Implement/prove: streaming algorithms**
  - Action: For checksum verifier, implement or reuse and verify this exact obligation: streaming algorithms. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for streaming algorithms; retain observable state/resource expectations.
- [ ] **F-PD-021.04 — Implement/prove: compare workflow**
  - Action: For checksum verifier, implement or reuse and verify this exact obligation: compare workflow. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compare workflow; retain observable state/resource expectations.
- [ ] **F-PD-021.05 — Implement/prove: file handles**
  - Action: For checksum verifier, implement or reuse and verify this exact obligation: file handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for file handles; retain observable state/resource expectations.
- [ ] **F-PD-021.06 — Implement/prove: provenance and clear mismatch**
  - Action: For checksum verifier, implement or reuse and verify this exact obligation: provenance and clear mismatch. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-021.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance and clear mismatch; retain observable state/resource expectations.
- [ ] **F-PD-021.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to checksum verifier: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-021.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-021.08 — Integrate into the real consumer and runtime route**
  - Action: Wire checksum verifier into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-021.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-021.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for checksum verifier as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-021.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-022"></a>
## F-PD-022 — regex tester

**Original requirement:** named dialect, limits/timeouts, captures/highlights and malformed-input UX

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-022.01 — Reconcile existing regex tester**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for regex tester. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: named dialect, limits/timeouts, captures/highlights and malformed-input UX
- [ ] **F-PD-022.02 — Freeze the exact contract for regex tester**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: named dialect, limits/timeouts, captures/highlights and malformed-input UX. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-022.03 — Implement/prove: named dialect**
  - Action: For regex tester, implement or reuse and verify this exact obligation: named dialect. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for named dialect; retain observable state/resource expectations.
- [ ] **F-PD-022.04 — Implement/prove: limits/timeouts**
  - Action: For regex tester, implement or reuse and verify this exact obligation: limits/timeouts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for limits/timeouts; retain observable state/resource expectations.
- [ ] **F-PD-022.05 — Implement/prove: captures/highlights and malformed-input UX**
  - Action: For regex tester, implement or reuse and verify this exact obligation: captures/highlights and malformed-input UX. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for captures/highlights and malformed-input UX; retain observable state/resource expectations.
- [ ] **F-PD-022.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to regex tester: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-022.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-022.07 — Integrate into the real consumer and runtime route**
  - Action: Wire regex tester into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-022.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-022.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for regex tester as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-022.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-023"></a>
## F-PD-023 — calculator

**Original requirement:** expression parser, precision, history, scientific/programmer modes and errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-023.01 — Reconcile existing calculator**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for calculator. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: expression parser, precision, history, scientific/programmer modes and errors
- [ ] **F-PD-023.02 — Freeze the exact contract for calculator**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: expression parser, precision, history, scientific/programmer modes and errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-023.03 — Implement/prove: expression parser**
  - Action: For calculator, implement or reuse and verify this exact obligation: expression parser. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for expression parser; retain observable state/resource expectations.
- [ ] **F-PD-023.04 — Implement/prove: precision**
  - Action: For calculator, implement or reuse and verify this exact obligation: precision. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for precision; retain observable state/resource expectations.
- [ ] **F-PD-023.05 — Implement/prove: history**
  - Action: For calculator, implement or reuse and verify this exact obligation: history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for history; retain observable state/resource expectations.
- [ ] **F-PD-023.06 — Implement/prove: scientific/programmer modes and errors**
  - Action: For calculator, implement or reuse and verify this exact obligation: scientific/programmer modes and errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-023.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scientific/programmer modes and errors; retain observable state/resource expectations.
- [ ] **F-PD-023.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to calculator: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-023.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-023.08 — Integrate into the real consumer and runtime route**
  - Action: Wire calculator into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-023.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-023.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for calculator as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-023.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-024"></a>
## F-PD-024 — unit converter

**Original requirement:** versioned unit metadata, precision, locale, categories, favorites and history

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-024.01 — Reconcile existing unit converter**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for unit converter. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: versioned unit metadata, precision, locale, categories, favorites and history
- [ ] **F-PD-024.02 — Freeze the exact contract for unit converter**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: versioned unit metadata, precision, locale, categories, favorites and history. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-024.03 — Implement/prove: versioned unit metadata**
  - Action: For unit converter, implement or reuse and verify this exact obligation: versioned unit metadata. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioned unit metadata; retain observable state/resource expectations.
- [ ] **F-PD-024.04 — Implement/prove: precision**
  - Action: For unit converter, implement or reuse and verify this exact obligation: precision. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for precision; retain observable state/resource expectations.
- [ ] **F-PD-024.05 — Implement/prove: locale**
  - Action: For unit converter, implement or reuse and verify this exact obligation: locale. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-024.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for locale; retain observable state/resource expectations.
- [ ] **F-PD-024.06 — Implement/prove: categories**
  - Action: For unit converter, implement or reuse and verify this exact obligation: categories. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-024.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for categories; retain observable state/resource expectations.
- [ ] **F-PD-024.07 — Implement/prove: favorites and history**
  - Action: For unit converter, implement or reuse and verify this exact obligation: favorites and history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-024.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for favorites and history; retain observable state/resource expectations.
- [ ] **F-PD-024.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to unit converter: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-024.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-024.09 — Integrate into the real consumer and runtime route**
  - Action: Wire unit converter into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-024.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-024.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for unit converter as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-024.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-025"></a>
## F-PD-025 — base converter

**Original requirement:** signed/arbitrary-width values, representation, overflow and copy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-025.01 — Reconcile existing base converter**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for base converter. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed/arbitrary-width values, representation, overflow and copy
- [ ] **F-PD-025.02 — Freeze the exact contract for base converter**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed/arbitrary-width values, representation, overflow and copy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-025.03 — Implement/prove: signed/arbitrary-width values**
  - Action: For base converter, implement or reuse and verify this exact obligation: signed/arbitrary-width values. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed/arbitrary-width values; retain observable state/resource expectations.
- [ ] **F-PD-025.04 — Implement/prove: representation**
  - Action: For base converter, implement or reuse and verify this exact obligation: representation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for representation; retain observable state/resource expectations.
- [ ] **F-PD-025.05 — Implement/prove: overflow and copy**
  - Action: For base converter, implement or reuse and verify this exact obligation: overflow and copy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for overflow and copy; retain observable state/resource expectations.
- [ ] **F-PD-025.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to base converter: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-025.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-025.07 — Integrate into the real consumer and runtime route**
  - Action: Wire base converter into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-025.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-025.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for base converter as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-025.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-026"></a>
## F-PD-026 — color picker

**Original requirement:** palette/history/formats/contrast and user-mediated screen sampling

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-026.01 — Reconcile existing color picker**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for color picker. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: palette/history/formats/contrast and user-mediated screen sampling
- [ ] **F-PD-026.02 — Freeze the exact contract for color picker**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: palette/history/formats/contrast and user-mediated screen sampling. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-026.03 — Implement/prove: palette/history/formats/contrast and user-mediated screen sampling**
  - Action: For color picker, implement or reuse and verify this exact obligation: palette/history/formats/contrast and user-mediated screen sampling. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for palette/history/formats/contrast and user-mediated screen sampling; retain observable state/resource expectations.
- [ ] **F-PD-026.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to color picker: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-026.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-026.05 — Integrate into the real consumer and runtime route**
  - Action: Wire color picker into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-026.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-026.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for color picker as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-026.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-027"></a>
## F-PD-027 — calendar

**Original requirement:** local calendars, events/recurrence/reminders/timezones/import/export and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-027.01 — Reconcile existing calendar**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for calendar. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: local calendars, events/recurrence/reminders/timezones/import/export and privacy
- [ ] **F-PD-027.02 — Freeze the exact contract for calendar**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: local calendars, events/recurrence/reminders/timezones/import/export and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-027.03 — Implement/prove: local calendars**
  - Action: For calendar, implement or reuse and verify this exact obligation: local calendars. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for local calendars; retain observable state/resource expectations.
- [ ] **F-PD-027.04 — Implement/prove: events/recurrence/reminders/timezones/import/export and privacy**
  - Action: For calendar, implement or reuse and verify this exact obligation: events/recurrence/reminders/timezones/import/export and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for events/recurrence/reminders/timezones/import/export and privacy; retain observable state/resource expectations.
- [ ] **F-PD-027.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to calendar: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-027.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-027.06 — Integrate into the real consumer and runtime route**
  - Action: Wire calendar into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-027.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-027.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for calendar as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-027.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-028"></a>
## F-PD-028 — tasks/todo

**Original requirement:** lists, due/recurrence/priorities/reminders/search and offline data

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-028.01 — Reconcile existing tasks/todo**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for tasks/todo. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: lists, due/recurrence/priorities/reminders/search and offline data
- [ ] **F-PD-028.02 — Freeze the exact contract for tasks/todo**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: lists, due/recurrence/priorities/reminders/search and offline data. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-028.03 — Implement/prove: lists**
  - Action: For tasks/todo, implement or reuse and verify this exact obligation: lists. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lists; retain observable state/resource expectations.
- [ ] **F-PD-028.04 — Implement/prove: due/recurrence/priorities/reminders/search and offline data**
  - Action: For tasks/todo, implement or reuse and verify this exact obligation: due/recurrence/priorities/reminders/search and offline data. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for due/recurrence/priorities/reminders/search and offline data; retain observable state/resource expectations.
- [ ] **F-PD-028.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to tasks/todo: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-028.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-028.06 — Integrate into the real consumer and runtime route**
  - Action: Wire tasks/todo into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-028.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-028.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for tasks/todo as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-028.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-029"></a>
## F-PD-029 — contacts

**Original requirement:** structured contacts, groups/search/import/export/account privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-029.01 — Reconcile existing contacts**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for contacts. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: structured contacts, groups/search/import/export/account privacy
- [ ] **F-PD-029.02 — Freeze the exact contract for contacts**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: structured contacts, groups/search/import/export/account privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-029.03 — Implement/prove: structured contacts**
  - Action: For contacts, implement or reuse and verify this exact obligation: structured contacts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for structured contacts; retain observable state/resource expectations.
- [ ] **F-PD-029.04 — Implement/prove: groups/search/import/export/account privacy**
  - Action: For contacts, implement or reuse and verify this exact obligation: groups/search/import/export/account privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for groups/search/import/export/account privacy; retain observable state/resource expectations.
- [ ] **F-PD-029.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to contacts: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-029.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-029.06 — Integrate into the real consumer and runtime route**
  - Action: Wire contacts into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-029.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-029.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for contacts as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-029.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-030"></a>
## F-PD-030 — knowledge base

**Original requirement:** notes/documents/links/search/citations/provenance and agent-readable handles

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-030.01 — Reconcile existing knowledge base**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for knowledge base. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: notes/documents/links/search/citations/provenance and agent-readable handles
- [ ] **F-PD-030.02 — Freeze the exact contract for knowledge base**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: notes/documents/links/search/citations/provenance and agent-readable handles. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-030.03 — Implement/prove: notes/documents/links/search/citations/provenance and agent-readable handles**
  - Action: For knowledge base, implement or reuse and verify this exact obligation: notes/documents/links/search/citations/provenance and agent-readable handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for notes/documents/links/search/citations/provenance and agent-readable handles; retain observable state/resource expectations.
- [ ] **F-PD-030.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to knowledge base: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-030.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-030.05 — Integrate into the real consumer and runtime route**
  - Action: Wire knowledge base into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-030.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-030.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for knowledge base as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-030.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-031"></a>
## F-PD-031 — file picker service app

**Original requirement:** reusable open/save workflows, recent/places/search and scoped returns

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-031.01 — Reconcile existing file picker service app**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file picker service app. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: reusable open/save workflows, recent/places/search and scoped returns
- [ ] **F-PD-031.02 — Freeze the exact contract for file picker service app**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: reusable open/save workflows, recent/places/search and scoped returns. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-031.03 — Implement/prove: reusable open/save workflows**
  - Action: For file picker service app, implement or reuse and verify this exact obligation: reusable open/save workflows. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reusable open/save workflows; retain observable state/resource expectations.
- [ ] **F-PD-031.04 — Implement/prove: recent/places/search and scoped returns**
  - Action: For file picker service app, implement or reuse and verify this exact obligation: recent/places/search and scoped returns. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recent/places/search and scoped returns; retain observable state/resource expectations.
- [ ] **F-PD-031.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file picker service app: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-031.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-031.06 — Integrate into the real consumer and runtime route**
  - Action: Wire file picker service app into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-031.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-031.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file picker service app as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-031.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-032"></a>
## F-PD-032 — print preview

**Original requirement:** pagination/options/scale/color/accessibility before job submission

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-032.01 — Reconcile existing print preview**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for print preview. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: pagination/options/scale/color/accessibility before job submission
- [ ] **F-PD-032.02 — Freeze the exact contract for print preview**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: pagination/options/scale/color/accessibility before job submission. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-032.03 — Implement/prove: pagination/options/scale/color/accessibility before job submission**
  - Action: For print preview, implement or reuse and verify this exact obligation: pagination/options/scale/color/accessibility before job submission. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pagination/options/scale/color/accessibility before job submission; retain observable state/resource expectations.
- [ ] **F-PD-032.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to print preview: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-032.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-032.05 — Integrate into the real consumer and runtime route**
  - Action: Wire print preview into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-032.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-032.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for print preview as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-032.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-033"></a>
## F-PD-033 — print queue

**Original requirement:** printer/jobs/status/reorder/pause/cancel/errors and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-033.01 — Reconcile existing print queue**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for print queue. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: printer/jobs/status/reorder/pause/cancel/errors and privacy
- [ ] **F-PD-033.02 — Freeze the exact contract for print queue**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: printer/jobs/status/reorder/pause/cancel/errors and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-033.03 — Implement/prove: printer/jobs/status/reorder/pause/cancel/errors and privacy**
  - Action: For print queue, implement or reuse and verify this exact obligation: printer/jobs/status/reorder/pause/cancel/errors and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for printer/jobs/status/reorder/pause/cancel/errors and privacy; retain observable state/resource expectations.
- [ ] **F-PD-033.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to print queue: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-033.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-033.05 — Integrate into the real consumer and runtime route**
  - Action: Wire print queue into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-033.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-033.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for print queue as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-033.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-034"></a>
## F-PD-034 — scanner/document capture

**Original requirement:** source/settings/preview/crop/OCR/save/privacy and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-034.01 — Reconcile existing scanner/document capture**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for scanner/document capture. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: source/settings/preview/crop/OCR/save/privacy and cancellation
- [ ] **F-PD-034.02 — Freeze the exact contract for scanner/document capture**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: source/settings/preview/crop/OCR/save/privacy and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-034.03 — Implement/prove: source/settings/preview/crop/OCR/save/privacy and cancellation**
  - Action: For scanner/document capture, implement or reuse and verify this exact obligation: source/settings/preview/crop/OCR/save/privacy and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source/settings/preview/crop/OCR/save/privacy and cancellation; retain observable state/resource expectations.
- [ ] **F-PD-034.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to scanner/document capture: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-034.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-034.05 — Integrate into the real consumer and runtime route**
  - Action: Wire scanner/document capture into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-034.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-034.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for scanner/document capture as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-034.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-035"></a>
## F-PD-035 — OCR

**Original requirement:** isolated image/text extraction, language, confidence, layout and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-035.01 — Reconcile existing OCR**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for OCR. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated image/text extraction, language, confidence, layout and privacy
- [ ] **F-PD-035.02 — Freeze the exact contract for OCR**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated image/text extraction, language, confidence, layout and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-035.03 — Implement/prove: isolated image/text extraction**
  - Action: For OCR, implement or reuse and verify this exact obligation: isolated image/text extraction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated image/text extraction; retain observable state/resource expectations.
- [ ] **F-PD-035.04 — Implement/prove: language**
  - Action: For OCR, implement or reuse and verify this exact obligation: language. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-035.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for language; retain observable state/resource expectations.
- [ ] **F-PD-035.05 — Implement/prove: confidence**
  - Action: For OCR, implement or reuse and verify this exact obligation: confidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-035.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for confidence; retain observable state/resource expectations.
- [ ] **F-PD-035.06 — Implement/prove: layout and privacy**
  - Action: For OCR, implement or reuse and verify this exact obligation: layout and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-035.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for layout and privacy; retain observable state/resource expectations.
- [ ] **F-PD-035.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to OCR: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-035.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-035.08 — Integrate into the real consumer and runtime route**
  - Action: Wire OCR into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-035.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-035.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for OCR as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-035.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-036"></a>
## F-PD-036 — recent documents

**Original requirement:** user-scoped provenance-aware history, missing items, privacy and clear

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-036.01 — Reconcile existing recent documents**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for recent documents. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: user-scoped provenance-aware history, missing items, privacy and clear
- [ ] **F-PD-036.02 — Freeze the exact contract for recent documents**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: user-scoped provenance-aware history, missing items, privacy and clear. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-036.03 — Implement/prove: user-scoped provenance-aware history**
  - Action: For recent documents, implement or reuse and verify this exact obligation: user-scoped provenance-aware history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user-scoped provenance-aware history; retain observable state/resource expectations.
- [ ] **F-PD-036.04 — Implement/prove: missing items**
  - Action: For recent documents, implement or reuse and verify this exact obligation: missing items. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-036.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for missing items; retain observable state/resource expectations.
- [ ] **F-PD-036.05 — Implement/prove: privacy and clear**
  - Action: For recent documents, implement or reuse and verify this exact obligation: privacy and clear. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-036.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and clear; retain observable state/resource expectations.
- [ ] **F-PD-036.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to recent documents: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-036.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-036.07 — Integrate into the real consumer and runtime route**
  - Action: Wire recent documents into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-036.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-036.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for recent documents as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-036.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-037"></a>
## F-PD-037 — templates

**Original requirement:** versioned document/sheet/deck templates with origin/license and preview

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-037.01 — Reconcile existing templates**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for templates. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: versioned document/sheet/deck templates with origin/license and preview
- [ ] **F-PD-037.02 — Freeze the exact contract for templates**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: versioned document/sheet/deck templates with origin/license and preview. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-037.03 — Implement/prove: versioned document/sheet/deck templates with origin/license and preview**
  - Action: For templates, implement or reuse and verify this exact obligation: versioned document/sheet/deck templates with origin/license and preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioned document/sheet/deck templates with origin/license and preview; retain observable state/resource expectations.
- [ ] **F-PD-037.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to templates: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-037.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-037.05 — Integrate into the real consumer and runtime route**
  - Action: Wire templates into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-037.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-037.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for templates as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-037.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-038"></a>
## F-PD-038 — import/export broker

**Original requirement:** explicit formats, isolated conversions, loss report and atomic output

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-038.01 — Reconcile existing import/export broker**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for import/export broker. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit formats, isolated conversions, loss report and atomic output
- [ ] **F-PD-038.02 — Freeze the exact contract for import/export broker**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit formats, isolated conversions, loss report and atomic output. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-038.03 — Implement/prove: explicit formats**
  - Action: For import/export broker, implement or reuse and verify this exact obligation: explicit formats. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit formats; retain observable state/resource expectations.
- [ ] **F-PD-038.04 — Implement/prove: isolated conversions**
  - Action: For import/export broker, implement or reuse and verify this exact obligation: isolated conversions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-038.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated conversions; retain observable state/resource expectations.
- [ ] **F-PD-038.05 — Implement/prove: loss report and atomic output**
  - Action: For import/export broker, implement or reuse and verify this exact obligation: loss report and atomic output. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-038.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for loss report and atomic output; retain observable state/resource expectations.
- [ ] **F-PD-038.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to import/export broker: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-038.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-038.07 — Integrate into the real consumer and runtime route**
  - Action: Wire import/export broker into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-038.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-038.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for import/export broker as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-038.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-039"></a>
## F-PD-039 — document collaboration

**Original requirement:** identity, sharing, conflict/merge, offline, audit and encryption

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-039.01 — Reconcile existing document collaboration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for document collaboration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: identity, sharing, conflict/merge, offline, audit and encryption
- [ ] **F-PD-039.02 — Freeze the exact contract for document collaboration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: identity, sharing, conflict/merge, offline, audit and encryption. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-039.03 — Implement/prove: identity**
  - Action: For document collaboration, implement or reuse and verify this exact obligation: identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for identity; retain observable state/resource expectations.
- [ ] **F-PD-039.04 — Implement/prove: sharing**
  - Action: For document collaboration, implement or reuse and verify this exact obligation: sharing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-039.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sharing; retain observable state/resource expectations.
- [ ] **F-PD-039.05 — Implement/prove: conflict/merge**
  - Action: For document collaboration, implement or reuse and verify this exact obligation: conflict/merge. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-039.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conflict/merge; retain observable state/resource expectations.
- [ ] **F-PD-039.06 — Implement/prove: offline**
  - Action: For document collaboration, implement or reuse and verify this exact obligation: offline. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-039.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for offline; retain observable state/resource expectations.
- [ ] **F-PD-039.07 — Implement/prove: audit and encryption**
  - Action: For document collaboration, implement or reuse and verify this exact obligation: audit and encryption. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-039.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for audit and encryption; retain observable state/resource expectations.
- [ ] **F-PD-039.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to document collaboration: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-039.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-039.09 — Integrate into the real consumer and runtime route**
  - Action: Wire document collaboration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-039.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-039.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for document collaboration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-039.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-040"></a>
## F-PD-040 — document recovery center

**Original requirement:** discover autosaves/crash copies, compare, restore/discard and provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-040.01 — Reconcile existing document recovery center**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for document recovery center. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: discover autosaves/crash copies, compare, restore/discard and provenance
- [ ] **F-PD-040.02 — Freeze the exact contract for document recovery center**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: discover autosaves/crash copies, compare, restore/discard and provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-040.03 — Implement/prove: discover autosaves/crash copies**
  - Action: For document recovery center, implement or reuse and verify this exact obligation: discover autosaves/crash copies. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discover autosaves/crash copies; retain observable state/resource expectations.
- [ ] **F-PD-040.04 — Implement/prove: compare**
  - Action: For document recovery center, implement or reuse and verify this exact obligation: compare. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-040.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compare; retain observable state/resource expectations.
- [ ] **F-PD-040.05 — Implement/prove: restore/discard and provenance**
  - Action: For document recovery center, implement or reuse and verify this exact obligation: restore/discard and provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-040.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for restore/discard and provenance; retain observable state/resource expectations.
- [ ] **F-PD-040.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to document recovery center: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-040.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-040.07 — Integrate into the real consumer and runtime route**
  - Action: Wire document recovery center into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-040.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-040.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for document recovery center as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-040.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-041"></a>
## F-PD-041 — cloud/network drive

**Original requirement:** account-scoped mount/cache/offline/conflict/quota and safe disconnect

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-041.01 — Reconcile existing cloud/network drive**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for cloud/network drive. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: account-scoped mount/cache/offline/conflict/quota and safe disconnect
- [ ] **F-PD-041.02 — Freeze the exact contract for cloud/network drive**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: account-scoped mount/cache/offline/conflict/quota and safe disconnect. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-041.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-041.03 — Implement/prove: account-scoped mount/cache/offline/conflict/quota and safe disconnect**
  - Action: For cloud/network drive, implement or reuse and verify this exact obligation: account-scoped mount/cache/offline/conflict/quota and safe disconnect. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-041.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for account-scoped mount/cache/offline/conflict/quota and safe disconnect; retain observable state/resource expectations.
- [ ] **F-PD-041.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to cloud/network drive: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-041.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-041.05 — Integrate into the real consumer and runtime route**
  - Action: Wire cloud/network drive into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-041.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-041.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for cloud/network drive as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-041.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-042"></a>
## F-PD-042 — cross-device document sync

**Original requirement:** encrypted versioned sync, conflicts, deletion recovery and user-visible state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-042.01 — Reconcile existing cross-device document sync**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for cross-device document sync. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: encrypted versioned sync, conflicts, deletion recovery and user-visible state
- [ ] **F-PD-042.02 — Freeze the exact contract for cross-device document sync**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: encrypted versioned sync, conflicts, deletion recovery and user-visible state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-042.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-042.03 — Implement/prove: encrypted versioned sync**
  - Action: For cross-device document sync, implement or reuse and verify this exact obligation: encrypted versioned sync. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-042.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for encrypted versioned sync; retain observable state/resource expectations.
- [ ] **F-PD-042.04 — Implement/prove: conflicts**
  - Action: For cross-device document sync, implement or reuse and verify this exact obligation: conflicts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-042.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conflicts; retain observable state/resource expectations.
- [ ] **F-PD-042.05 — Implement/prove: deletion recovery and user-visible state**
  - Action: For cross-device document sync, implement or reuse and verify this exact obligation: deletion recovery and user-visible state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-042.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deletion recovery and user-visible state; retain observable state/resource expectations.
- [ ] **F-PD-042.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to cross-device document sync: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-042.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-042.07 — Integrate into the real consumer and runtime route**
  - Action: Wire cross-device document sync into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-042.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-042.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for cross-device document sync as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-042.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-pd-043"></a>
## F-PD-043 — citation/reference manager

**Original requirement:** sources/metadata/attachments/collections/export and provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-PD-043.01 — Reconcile existing citation/reference manager**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for citation/reference manager. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: sources/metadata/attachments/collections/export and provenance
- [ ] **F-PD-043.02 — Freeze the exact contract for citation/reference manager**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: sources/metadata/attachments/collections/export and provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-PD-043.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-PD-043.03 — Implement/prove: sources/metadata/attachments/collections/export and provenance**
  - Action: For citation/reference manager, implement or reuse and verify this exact obligation: sources/metadata/attachments/collections/export and provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-PD-043.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sources/metadata/attachments/collections/export and provenance; retain observable state/resource expectations.
- [ ] **F-PD-043.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to citation/reference manager: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-PD-043.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-PD-043.05 — Integrate into the real consumer and runtime route**
  - Action: Wire citation/reference manager into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-PD-043.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-PD-043.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for citation/reference manager as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-PD-043.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-001"></a>
## F-MD-001 — image viewer

**Original requirement:** folder navigation, zoom/pan/rotate/fullscreen, metadata, profiles and errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-001.01 — Reconcile existing image viewer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for image viewer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: folder navigation, zoom/pan/rotate/fullscreen, metadata, profiles and errors
- [ ] **F-MD-001.02 — Freeze the exact contract for image viewer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: folder navigation, zoom/pan/rotate/fullscreen, metadata, profiles and errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-001.03 — Implement/prove: folder navigation**
  - Action: For image viewer, implement or reuse and verify this exact obligation: folder navigation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for folder navigation; retain observable state/resource expectations.
- [ ] **F-MD-001.04 — Implement/prove: zoom/pan/rotate/fullscreen**
  - Action: For image viewer, implement or reuse and verify this exact obligation: zoom/pan/rotate/fullscreen. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for zoom/pan/rotate/fullscreen; retain observable state/resource expectations.
- [ ] **F-MD-001.05 — Implement/prove: metadata**
  - Action: For image viewer, implement or reuse and verify this exact obligation: metadata. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for metadata; retain observable state/resource expectations.
- [ ] **F-MD-001.06 — Implement/prove: profiles and errors**
  - Action: For image viewer, implement or reuse and verify this exact obligation: profiles and errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for profiles and errors; retain observable state/resource expectations.
- [ ] **F-MD-001.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to image viewer: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-001.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-001.08 — Integrate into the real consumer and runtime route**
  - Action: Wire image viewer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-001.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-001.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for image viewer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-001.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-002"></a>
## F-MD-002 — gallery/photo library

**Original requirement:** import, albums, search, metadata, thumbnails, duplicates, removable media and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-002.01 — Reconcile existing gallery/photo library**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for gallery/photo library. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: import, albums, search, metadata, thumbnails, duplicates, removable media and privacy
- [ ] **F-MD-002.02 — Freeze the exact contract for gallery/photo library**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: import, albums, search, metadata, thumbnails, duplicates, removable media and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-002.03 — Implement/prove: import**
  - Action: For gallery/photo library, implement or reuse and verify this exact obligation: import. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for import; retain observable state/resource expectations.
- [ ] **F-MD-002.04 — Implement/prove: albums**
  - Action: For gallery/photo library, implement or reuse and verify this exact obligation: albums. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for albums; retain observable state/resource expectations.
- [ ] **F-MD-002.05 — Implement/prove: search**
  - Action: For gallery/photo library, implement or reuse and verify this exact obligation: search. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-002.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for search; retain observable state/resource expectations.
- [ ] **F-MD-002.06 — Implement/prove: metadata**
  - Action: For gallery/photo library, implement or reuse and verify this exact obligation: metadata. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-002.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for metadata; retain observable state/resource expectations.
- [ ] **F-MD-002.07 — Implement/prove: thumbnails**
  - Action: For gallery/photo library, implement or reuse and verify this exact obligation: thumbnails. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-002.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for thumbnails; retain observable state/resource expectations.
- [ ] **F-MD-002.08 — Implement/prove: duplicates**
  - Action: For gallery/photo library, implement or reuse and verify this exact obligation: duplicates. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-002.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for duplicates; retain observable state/resource expectations.
- [ ] **F-MD-002.09 — Implement/prove: removable media and privacy**
  - Action: For gallery/photo library, implement or reuse and verify this exact obligation: removable media and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-002.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for removable media and privacy; retain observable state/resource expectations.
- [ ] **F-MD-002.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to gallery/photo library: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-002.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-002.11 — Integrate into the real consumer and runtime route**
  - Action: Wire gallery/photo library into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-002.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-002.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for gallery/photo library as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-002.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-003"></a>
## F-MD-003 — paint/raster editor

**Original requirement:** tools, brushes, palette, selection, layers, history, formats, export and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-003.01 — Reconcile existing paint/raster editor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for paint/raster editor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: tools, brushes, palette, selection, layers, history, formats, export and recovery
- [ ] **F-MD-003.02 — Freeze the exact contract for paint/raster editor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: tools, brushes, palette, selection, layers, history, formats, export and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-003.03 — Implement/prove: tools**
  - Action: For paint/raster editor, implement or reuse and verify this exact obligation: tools. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tools; retain observable state/resource expectations.
- [ ] **F-MD-003.04 — Implement/prove: brushes**
  - Action: For paint/raster editor, implement or reuse and verify this exact obligation: brushes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for brushes; retain observable state/resource expectations.
- [ ] **F-MD-003.05 — Implement/prove: palette**
  - Action: For paint/raster editor, implement or reuse and verify this exact obligation: palette. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for palette; retain observable state/resource expectations.
- [ ] **F-MD-003.06 — Implement/prove: selection**
  - Action: For paint/raster editor, implement or reuse and verify this exact obligation: selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-003.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for selection; retain observable state/resource expectations.
- [ ] **F-MD-003.07 — Implement/prove: layers**
  - Action: For paint/raster editor, implement or reuse and verify this exact obligation: layers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-003.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for layers; retain observable state/resource expectations.
- [ ] **F-MD-003.08 — Implement/prove: history**
  - Action: For paint/raster editor, implement or reuse and verify this exact obligation: history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-003.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for history; retain observable state/resource expectations.
- [ ] **F-MD-003.09 — Implement/prove: formats**
  - Action: For paint/raster editor, implement or reuse and verify this exact obligation: formats. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-003.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for formats; retain observable state/resource expectations.
- [ ] **F-MD-003.10 — Implement/prove: export and recovery**
  - Action: For paint/raster editor, implement or reuse and verify this exact obligation: export and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-003.09.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for export and recovery; retain observable state/resource expectations.
- [ ] **F-MD-003.11 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to paint/raster editor: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-003.10.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-003.12 — Integrate into the real consumer and runtime route**
  - Action: Wire paint/raster editor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-003.11.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-003.13 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for paint/raster editor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-003.12.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-004"></a>
## F-MD-004 — vector drawing

**Original requirement:** paths/shapes/text/layers/transforms/import/export and accessibility

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-004.01 — Reconcile existing vector drawing**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for vector drawing. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: paths/shapes/text/layers/transforms/import/export and accessibility
- [ ] **F-MD-004.02 — Freeze the exact contract for vector drawing**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: paths/shapes/text/layers/transforms/import/export and accessibility. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-004.03 — Implement/prove: paths/shapes/text/layers/transforms/import/export and accessibility**
  - Action: For vector drawing, implement or reuse and verify this exact obligation: paths/shapes/text/layers/transforms/import/export and accessibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for paths/shapes/text/layers/transforms/import/export and accessibility; retain observable state/resource expectations.
- [ ] **F-MD-004.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to vector drawing: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-004.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-004.05 — Integrate into the real consumer and runtime route**
  - Action: Wire vector drawing into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-004.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-004.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for vector drawing as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-004.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-005"></a>
## F-MD-005 — screenshot editor

**Original requirement:** portal capture, crop/annotate/redact/export and protected-content policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-005.01 — Reconcile existing screenshot editor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for screenshot editor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: portal capture, crop/annotate/redact/export and protected-content policy
- [ ] **F-MD-005.02 — Freeze the exact contract for screenshot editor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: portal capture, crop/annotate/redact/export and protected-content policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-005.03 — Implement/prove: portal capture**
  - Action: For screenshot editor, implement or reuse and verify this exact obligation: portal capture. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for portal capture; retain observable state/resource expectations.
- [ ] **F-MD-005.04 — Implement/prove: crop/annotate/redact/export and protected-content policy**
  - Action: For screenshot editor, implement or reuse and verify this exact obligation: crop/annotate/redact/export and protected-content policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for crop/annotate/redact/export and protected-content policy; retain observable state/resource expectations.
- [ ] **F-MD-005.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to screenshot editor: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-005.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-005.06 — Integrate into the real consumer and runtime route**
  - Action: Wire screenshot editor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-005.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-005.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for screenshot editor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-005.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-006"></a>
## F-MD-006 — audio player

**Original requirement:** library/queue/playback/seek/volume/metadata and media-session integration

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-006.01 — Reconcile existing audio player**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio player. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: library/queue/playback/seek/volume/metadata and media-session integration
- [ ] **F-MD-006.02 — Freeze the exact contract for audio player**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: library/queue/playback/seek/volume/metadata and media-session integration. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-006.03 — Implement/prove: library/queue/playback/seek/volume/metadata and media-session integration**
  - Action: For audio player, implement or reuse and verify this exact obligation: library/queue/playback/seek/volume/metadata and media-session integration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for library/queue/playback/seek/volume/metadata and media-session integration; retain observable state/resource expectations.
- [ ] **F-MD-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio player: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire audio player into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio player as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-007"></a>
## F-MD-007 — video player

**Original requirement:** demux/decode, seek/subtitles/fullscreen/audio sync and crash containment

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-007.01 — Reconcile existing video player**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for video player. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: demux/decode, seek/subtitles/fullscreen/audio sync and crash containment
- [ ] **F-MD-007.02 — Freeze the exact contract for video player**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: demux/decode, seek/subtitles/fullscreen/audio sync and crash containment. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-007.03 — Implement/prove: demux/decode**
  - Action: For video player, implement or reuse and verify this exact obligation: demux/decode. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for demux/decode; retain observable state/resource expectations.
- [ ] **F-MD-007.04 — Implement/prove: seek/subtitles/fullscreen/audio sync and crash containment**
  - Action: For video player, implement or reuse and verify this exact obligation: seek/subtitles/fullscreen/audio sync and crash containment. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for seek/subtitles/fullscreen/audio sync and crash containment; retain observable state/resource expectations.
- [ ] **F-MD-007.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to video player: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-007.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-007.06 — Integrate into the real consumer and runtime route**
  - Action: Wire video player into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-007.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-007.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for video player as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-007.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-008"></a>
## F-MD-008 — music library

**Original requirement:** indexed metadata/art/playlists/search/import and removable media

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-008.01 — Reconcile existing music library**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for music library. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: indexed metadata/art/playlists/search/import and removable media
- [ ] **F-MD-008.02 — Freeze the exact contract for music library**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: indexed metadata/art/playlists/search/import and removable media. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-008.03 — Implement/prove: indexed metadata/art/playlists/search/import and removable media**
  - Action: For music library, implement or reuse and verify this exact obligation: indexed metadata/art/playlists/search/import and removable media. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for indexed metadata/art/playlists/search/import and removable media; retain observable state/resource expectations.
- [ ] **F-MD-008.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to music library: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-008.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-008.05 — Integrate into the real consumer and runtime route**
  - Action: Wire music library into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-008.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-008.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for music library as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-008.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-009"></a>
## F-MD-009 — voice recorder

**Original requirement:** microphone consent, level, pause/resume, format, naming and atomic save

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-009.01 — Reconcile existing voice recorder**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for voice recorder. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: microphone consent, level, pause/resume, format, naming and atomic save
- [ ] **F-MD-009.02 — Freeze the exact contract for voice recorder**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: microphone consent, level, pause/resume, format, naming and atomic save. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-009.03 — Implement/prove: microphone consent**
  - Action: For voice recorder, implement or reuse and verify this exact obligation: microphone consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for microphone consent; retain observable state/resource expectations.
- [ ] **F-MD-009.04 — Implement/prove: level**
  - Action: For voice recorder, implement or reuse and verify this exact obligation: level. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for level; retain observable state/resource expectations.
- [ ] **F-MD-009.05 — Implement/prove: pause/resume**
  - Action: For voice recorder, implement or reuse and verify this exact obligation: pause/resume. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pause/resume; retain observable state/resource expectations.
- [ ] **F-MD-009.06 — Implement/prove: format**
  - Action: For voice recorder, implement or reuse and verify this exact obligation: format. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-009.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for format; retain observable state/resource expectations.
- [ ] **F-MD-009.07 — Implement/prove: naming and atomic save**
  - Action: For voice recorder, implement or reuse and verify this exact obligation: naming and atomic save. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-009.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for naming and atomic save; retain observable state/resource expectations.
- [ ] **F-MD-009.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to voice recorder: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-009.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-009.09 — Integrate into the real consumer and runtime route**
  - Action: Wire voice recorder into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-009.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-009.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for voice recorder as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-009.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-010"></a>
## F-MD-010 — audio editor

**Original requirement:** waveform, selection, cut/mix/effects/undo/export and bounded processing

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-010.01 — Reconcile existing audio editor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio editor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: waveform, selection, cut/mix/effects/undo/export and bounded processing
- [ ] **F-MD-010.02 — Freeze the exact contract for audio editor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: waveform, selection, cut/mix/effects/undo/export and bounded processing. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-010.03 — Implement/prove: waveform**
  - Action: For audio editor, implement or reuse and verify this exact obligation: waveform. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for waveform; retain observable state/resource expectations.
- [ ] **F-MD-010.04 — Implement/prove: selection**
  - Action: For audio editor, implement or reuse and verify this exact obligation: selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for selection; retain observable state/resource expectations.
- [ ] **F-MD-010.05 — Implement/prove: cut/mix/effects/undo/export and bounded processing**
  - Action: For audio editor, implement or reuse and verify this exact obligation: cut/mix/effects/undo/export and bounded processing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cut/mix/effects/undo/export and bounded processing; retain observable state/resource expectations.
- [ ] **F-MD-010.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio editor: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-010.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-010.07 — Integrate into the real consumer and runtime route**
  - Action: Wire audio editor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-010.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-010.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio editor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-010.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-011"></a>
## F-MD-011 — synthesizer

**Original requirement:** notes/oscillators/envelopes/filters/presets/MIDI, realtime budget and export

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-011.01 — Reconcile existing synthesizer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for synthesizer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: notes/oscillators/envelopes/filters/presets/MIDI, realtime budget and export
- [ ] **F-MD-011.02 — Freeze the exact contract for synthesizer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: notes/oscillators/envelopes/filters/presets/MIDI, realtime budget and export. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-011.03 — Implement/prove: notes/oscillators/envelopes/filters/presets/MIDI**
  - Action: For synthesizer, implement or reuse and verify this exact obligation: notes/oscillators/envelopes/filters/presets/MIDI. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for notes/oscillators/envelopes/filters/presets/MIDI; retain observable state/resource expectations.
- [ ] **F-MD-011.04 — Implement/prove: realtime budget and export**
  - Action: For synthesizer, implement or reuse and verify this exact obligation: realtime budget and export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for realtime budget and export; retain observable state/resource expectations.
- [ ] **F-MD-011.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to synthesizer: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-011.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-011.06 — Integrate into the real consumer and runtime route**
  - Action: Wire synthesizer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-011.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-011.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for synthesizer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-011.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-012"></a>
## F-MD-012 — mixer

**Original requirement:** track/input/output routing, meters, gain/pan/effects/record and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-012.01 — Reconcile existing mixer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for mixer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: track/input/output routing, meters, gain/pan/effects/record and recovery
- [ ] **F-MD-012.02 — Freeze the exact contract for mixer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: track/input/output routing, meters, gain/pan/effects/record and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-012.03 — Implement/prove: track/input/output routing**
  - Action: For mixer, implement or reuse and verify this exact obligation: track/input/output routing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for track/input/output routing; retain observable state/resource expectations.
- [ ] **F-MD-012.04 — Implement/prove: meters**
  - Action: For mixer, implement or reuse and verify this exact obligation: meters. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for meters; retain observable state/resource expectations.
- [ ] **F-MD-012.05 — Implement/prove: gain/pan/effects/record and recovery**
  - Action: For mixer, implement or reuse and verify this exact obligation: gain/pan/effects/record and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for gain/pan/effects/record and recovery; retain observable state/resource expectations.
- [ ] **F-MD-012.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to mixer: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-012.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-012.07 — Integrate into the real consumer and runtime route**
  - Action: Wire mixer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-012.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-012.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for mixer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-012.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-013"></a>
## F-MD-013 — camera app

**Original requirement:** live preview, capture/video, device/settings, privacy indicator and gallery handoff

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-013.01 — Reconcile existing camera app**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for camera app. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: live preview, capture/video, device/settings, privacy indicator and gallery handoff
- [ ] **F-MD-013.02 — Freeze the exact contract for camera app**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: live preview, capture/video, device/settings, privacy indicator and gallery handoff. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-013.03 — Implement/prove: live preview**
  - Action: For camera app, implement or reuse and verify this exact obligation: live preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for live preview; retain observable state/resource expectations.
- [ ] **F-MD-013.04 — Implement/prove: capture/video**
  - Action: For camera app, implement or reuse and verify this exact obligation: capture/video. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capture/video; retain observable state/resource expectations.
- [ ] **F-MD-013.05 — Implement/prove: device/settings**
  - Action: For camera app, implement or reuse and verify this exact obligation: device/settings. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device/settings; retain observable state/resource expectations.
- [ ] **F-MD-013.06 — Implement/prove: privacy indicator and gallery handoff**
  - Action: For camera app, implement or reuse and verify this exact obligation: privacy indicator and gallery handoff. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-013.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy indicator and gallery handoff; retain observable state/resource expectations.
- [ ] **F-MD-013.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to camera app: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-013.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-013.08 — Integrate into the real consumer and runtime route**
  - Action: Wire camera app into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-013.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-013.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for camera app as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-013.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-014"></a>
## F-MD-014 — screen recorder

**Original requirement:** target/audio/quality/indicator/protected content, pause and atomic finalize

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-014.01 — Reconcile existing screen recorder**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for screen recorder. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: target/audio/quality/indicator/protected content, pause and atomic finalize
- [ ] **F-MD-014.02 — Freeze the exact contract for screen recorder**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: target/audio/quality/indicator/protected content, pause and atomic finalize. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-014.03 — Implement/prove: target/audio/quality/indicator/protected content**
  - Action: For screen recorder, implement or reuse and verify this exact obligation: target/audio/quality/indicator/protected content. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for target/audio/quality/indicator/protected content; retain observable state/resource expectations.
- [ ] **F-MD-014.04 — Implement/prove: pause and atomic finalize**
  - Action: For screen recorder, implement or reuse and verify this exact obligation: pause and atomic finalize. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pause and atomic finalize; retain observable state/resource expectations.
- [ ] **F-MD-014.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to screen recorder: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-014.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-014.06 — Integrate into the real consumer and runtime route**
  - Action: Wire screen recorder into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-014.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-014.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for screen recorder as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-014.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-015"></a>
## F-MD-015 — media converter

**Original requirement:** isolated decode/encode, formats/presets/progress/cancel and provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-015.01 — Reconcile existing media converter**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for media converter. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated decode/encode, formats/presets/progress/cancel and provenance
- [ ] **F-MD-015.02 — Freeze the exact contract for media converter**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated decode/encode, formats/presets/progress/cancel and provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-015.03 — Implement/prove: isolated decode/encode**
  - Action: For media converter, implement or reuse and verify this exact obligation: isolated decode/encode. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated decode/encode; retain observable state/resource expectations.
- [ ] **F-MD-015.04 — Implement/prove: formats/presets/progress/cancel and provenance**
  - Action: For media converter, implement or reuse and verify this exact obligation: formats/presets/progress/cancel and provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for formats/presets/progress/cancel and provenance; retain observable state/resource expectations.
- [ ] **F-MD-015.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to media converter: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-015.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-015.06 — Integrate into the real consumer and runtime route**
  - Action: Wire media converter into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-015.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-015.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for media converter as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-015.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-016"></a>
## F-MD-016 — animation/demo studio

**Original requirement:** timeline/keyframes/assets/export and deterministic visual-regression mode

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-016.01 — Reconcile existing animation/demo studio**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for animation/demo studio. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: timeline/keyframes/assets/export and deterministic visual-regression mode
- [ ] **F-MD-016.02 — Freeze the exact contract for animation/demo studio**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: timeline/keyframes/assets/export and deterministic visual-regression mode. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-016.03 — Implement/prove: timeline/keyframes/assets/export and deterministic visual-regression mode**
  - Action: For animation/demo studio, implement or reuse and verify this exact obligation: timeline/keyframes/assets/export and deterministic visual-regression mode. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeline/keyframes/assets/export and deterministic visual-regression mode; retain observable state/resource expectations.
- [ ] **F-MD-016.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to animation/demo studio: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-016.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-016.05 — Integrate into the real consumer and runtime route**
  - Action: Wire animation/demo studio into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-016.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-016.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for animation/demo studio as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-016.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-017"></a>
## F-MD-017 — 3D viewer/demo

**Original requirement:** scene/camera/input/assets/render backend and performance receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-017.01 — Reconcile existing 3D viewer/demo**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for 3D viewer/demo. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: scene/camera/input/assets/render backend and performance receipts
- [ ] **F-MD-017.02 — Freeze the exact contract for 3D viewer/demo**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: scene/camera/input/assets/render backend and performance receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-017.03 — Implement/prove: scene/camera/input/assets/render backend and performance receipts**
  - Action: For 3D viewer/demo, implement or reuse and verify this exact obligation: scene/camera/input/assets/render backend and performance receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scene/camera/input/assets/render backend and performance receipts; retain observable state/resource expectations.
- [ ] **F-MD-017.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to 3D viewer/demo: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-017.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-017.05 — Integrate into the real consumer and runtime route**
  - Action: Wire 3D viewer/demo into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-017.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-017.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for 3D viewer/demo as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-017.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-018"></a>
## F-MD-018 — font atlas/inspector

**Original requirement:** coverage/search/metrics/shaping/fallback/license and regression views

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-018.01 — Reconcile existing font atlas/inspector**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for font atlas/inspector. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: coverage/search/metrics/shaping/fallback/license and regression views
- [ ] **F-MD-018.02 — Freeze the exact contract for font atlas/inspector**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: coverage/search/metrics/shaping/fallback/license and regression views. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-018.03 — Implement/prove: coverage/search/metrics/shaping/fallback/license and regression views**
  - Action: For font atlas/inspector, implement or reuse and verify this exact obligation: coverage/search/metrics/shaping/fallback/license and regression views. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for coverage/search/metrics/shaping/fallback/license and regression views; retain observable state/resource expectations.
- [ ] **F-MD-018.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to font atlas/inspector: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-018.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-018.05 — Integrate into the real consumer and runtime route**
  - Action: Wire font atlas/inspector into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-018.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-018.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for font atlas/inspector as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-018.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-019"></a>
## F-MD-019 — framebuffer/renderer inspector

**Original requirement:** capture, formats/layers/damage/performance with privileged read-only access

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-019.01 — Reconcile existing framebuffer/renderer inspector**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for framebuffer/renderer inspector. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: capture, formats/layers/damage/performance with privileged read-only access
- [ ] **F-MD-019.02 — Freeze the exact contract for framebuffer/renderer inspector**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: capture, formats/layers/damage/performance with privileged read-only access. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-019.03 — Implement/prove: capture**
  - Action: For framebuffer/renderer inspector, implement or reuse and verify this exact obligation: capture. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capture; retain observable state/resource expectations.
- [ ] **F-MD-019.04 — Implement/prove: formats/layers/damage/performance with privileged read-only access**
  - Action: For framebuffer/renderer inspector, implement or reuse and verify this exact obligation: formats/layers/damage/performance with privileged read-only access. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for formats/layers/damage/performance with privileged read-only access; retain observable state/resource expectations.
- [ ] **F-MD-019.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to framebuffer/renderer inspector: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-019.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-019.06 — Integrate into the real consumer and runtime route**
  - Action: Wire framebuffer/renderer inspector into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-019.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-019.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for framebuffer/renderer inspector as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-019.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-020"></a>
## F-MD-020 — mail

**Original requirement:** accounts, sync, folders/search/compose/attachments/offline/security/privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-020.01 — Reconcile existing mail**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for mail. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: accounts, sync, folders/search/compose/attachments/offline/security/privacy
- [ ] **F-MD-020.02 — Freeze the exact contract for mail**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: accounts, sync, folders/search/compose/attachments/offline/security/privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-020.03 — Implement/prove: accounts**
  - Action: For mail, implement or reuse and verify this exact obligation: accounts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for accounts; retain observable state/resource expectations.
- [ ] **F-MD-020.04 — Implement/prove: sync**
  - Action: For mail, implement or reuse and verify this exact obligation: sync. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sync; retain observable state/resource expectations.
- [ ] **F-MD-020.05 — Implement/prove: folders/search/compose/attachments/offline/security/privacy**
  - Action: For mail, implement or reuse and verify this exact obligation: folders/search/compose/attachments/offline/security/privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-020.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for folders/search/compose/attachments/offline/security/privacy; retain observable state/resource expectations.
- [ ] **F-MD-020.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to mail: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-020.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-020.07 — Integrate into the real consumer and runtime route**
  - Action: Wire mail into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-020.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-020.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for mail as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-020.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-021"></a>
## F-MD-021 — messaging/chat

**Original requirement:** identity, conversations, attachments, notifications, encryption policy and offline

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-021.01 — Reconcile existing messaging/chat**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for messaging/chat. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: identity, conversations, attachments, notifications, encryption policy and offline
- [ ] **F-MD-021.02 — Freeze the exact contract for messaging/chat**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: identity, conversations, attachments, notifications, encryption policy and offline. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-021.03 — Implement/prove: identity**
  - Action: For messaging/chat, implement or reuse and verify this exact obligation: identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for identity; retain observable state/resource expectations.
- [ ] **F-MD-021.04 — Implement/prove: conversations**
  - Action: For messaging/chat, implement or reuse and verify this exact obligation: conversations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conversations; retain observable state/resource expectations.
- [ ] **F-MD-021.05 — Implement/prove: attachments**
  - Action: For messaging/chat, implement or reuse and verify this exact obligation: attachments. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for attachments; retain observable state/resource expectations.
- [ ] **F-MD-021.06 — Implement/prove: notifications**
  - Action: For messaging/chat, implement or reuse and verify this exact obligation: notifications. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-021.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for notifications; retain observable state/resource expectations.
- [ ] **F-MD-021.07 — Implement/prove: encryption policy and offline**
  - Action: For messaging/chat, implement or reuse and verify this exact obligation: encryption policy and offline. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-021.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for encryption policy and offline; retain observable state/resource expectations.
- [ ] **F-MD-021.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to messaging/chat: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-021.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-021.09 — Integrate into the real consumer and runtime route**
  - Action: Wire messaging/chat into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-021.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-021.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for messaging/chat as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-021.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-022"></a>
## F-MD-022 — RSS/news reader

**Original requirement:** feeds, update, articles, offline, read state, import/export and safe web content

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-022.01 — Reconcile existing RSS/news reader**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for RSS/news reader. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: feeds, update, articles, offline, read state, import/export and safe web content
- [ ] **F-MD-022.02 — Freeze the exact contract for RSS/news reader**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: feeds, update, articles, offline, read state, import/export and safe web content. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-022.03 — Implement/prove: feeds**
  - Action: For RSS/news reader, implement or reuse and verify this exact obligation: feeds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for feeds; retain observable state/resource expectations.
- [ ] **F-MD-022.04 — Implement/prove: update**
  - Action: For RSS/news reader, implement or reuse and verify this exact obligation: update. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for update; retain observable state/resource expectations.
- [ ] **F-MD-022.05 — Implement/prove: articles**
  - Action: For RSS/news reader, implement or reuse and verify this exact obligation: articles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for articles; retain observable state/resource expectations.
- [ ] **F-MD-022.06 — Implement/prove: offline**
  - Action: For RSS/news reader, implement or reuse and verify this exact obligation: offline. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for offline; retain observable state/resource expectations.
- [ ] **F-MD-022.07 — Implement/prove: read state**
  - Action: For RSS/news reader, implement or reuse and verify this exact obligation: read state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-022.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for read state; retain observable state/resource expectations.
- [ ] **F-MD-022.08 — Implement/prove: import/export and safe web content**
  - Action: For RSS/news reader, implement or reuse and verify this exact obligation: import/export and safe web content. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-022.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for import/export and safe web content; retain observable state/resource expectations.
- [ ] **F-MD-022.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to RSS/news reader: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-022.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-022.10 — Integrate into the real consumer and runtime route**
  - Action: Wire RSS/news reader into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-022.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-022.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for RSS/news reader as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-022.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-023"></a>
## F-MD-023 — maps

**Original requirement:** tiles/search/routes/locations/offline/cache/privacy and accessibility

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-023.01 — Reconcile existing maps**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for maps. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: tiles/search/routes/locations/offline/cache/privacy and accessibility
- [ ] **F-MD-023.02 — Freeze the exact contract for maps**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: tiles/search/routes/locations/offline/cache/privacy and accessibility. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-023.03 — Implement/prove: tiles/search/routes/locations/offline/cache/privacy and accessibility**
  - Action: For maps, implement or reuse and verify this exact obligation: tiles/search/routes/locations/offline/cache/privacy and accessibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tiles/search/routes/locations/offline/cache/privacy and accessibility; retain observable state/resource expectations.
- [ ] **F-MD-023.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to maps: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-023.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-023.05 — Integrate into the real consumer and runtime route**
  - Action: Wire maps into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-023.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-023.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for maps as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-023.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-024"></a>
## F-MD-024 — weather

**Original requirement:** real provider/provenance, location consent, forecast/units/offline and errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-024.01 — Reconcile existing weather**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for weather. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: real provider/provenance, location consent, forecast/units/offline and errors
- [ ] **F-MD-024.02 — Freeze the exact contract for weather**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: real provider/provenance, location consent, forecast/units/offline and errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-024.03 — Implement/prove: real provider/provenance**
  - Action: For weather, implement or reuse and verify this exact obligation: real provider/provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for real provider/provenance; retain observable state/resource expectations.
- [ ] **F-MD-024.04 — Implement/prove: location consent**
  - Action: For weather, implement or reuse and verify this exact obligation: location consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for location consent; retain observable state/resource expectations.
- [ ] **F-MD-024.05 — Implement/prove: forecast/units/offline and errors**
  - Action: For weather, implement or reuse and verify this exact obligation: forecast/units/offline and errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-024.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for forecast/units/offline and errors; retain observable state/resource expectations.
- [ ] **F-MD-024.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to weather: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-024.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-024.07 — Integrate into the real consumer and runtime route**
  - Action: Wire weather into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-024.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-024.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for weather as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-024.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-025"></a>
## F-MD-025 — clock

**Original requirement:** locale/timezone, multiple clocks and trustworthy wall time

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-025.01 — Reconcile existing clock**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for clock. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: locale/timezone, multiple clocks and trustworthy wall time
- [ ] **F-MD-025.02 — Freeze the exact contract for clock**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: locale/timezone, multiple clocks and trustworthy wall time. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-025.03 — Implement/prove: locale/timezone**
  - Action: For clock, implement or reuse and verify this exact obligation: locale/timezone. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for locale/timezone; retain observable state/resource expectations.
- [ ] **F-MD-025.04 — Implement/prove: multiple clocks and trustworthy wall time**
  - Action: For clock, implement or reuse and verify this exact obligation: multiple clocks and trustworthy wall time. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for multiple clocks and trustworthy wall time; retain observable state/resource expectations.
- [ ] **F-MD-025.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to clock: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-025.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-025.06 — Integrate into the real consumer and runtime route**
  - Action: Wire clock into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-025.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-025.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for clock as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-025.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-026"></a>
## F-MD-026 — timer/stopwatch

**Original requirement:** monotonic countdown/laps, background/suspend, alarms and notifications

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-026.01 — Reconcile existing timer/stopwatch**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for timer/stopwatch. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: monotonic countdown/laps, background/suspend, alarms and notifications
- [ ] **F-MD-026.02 — Freeze the exact contract for timer/stopwatch**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: monotonic countdown/laps, background/suspend, alarms and notifications. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-026.03 — Implement/prove: monotonic countdown/laps**
  - Action: For timer/stopwatch, implement or reuse and verify this exact obligation: monotonic countdown/laps. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for monotonic countdown/laps; retain observable state/resource expectations.
- [ ] **F-MD-026.04 — Implement/prove: background/suspend**
  - Action: For timer/stopwatch, implement or reuse and verify this exact obligation: background/suspend. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for background/suspend; retain observable state/resource expectations.
- [ ] **F-MD-026.05 — Implement/prove: alarms and notifications**
  - Action: For timer/stopwatch, implement or reuse and verify this exact obligation: alarms and notifications. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for alarms and notifications; retain observable state/resource expectations.
- [ ] **F-MD-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to timer/stopwatch: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire timer/stopwatch into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for timer/stopwatch as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-027"></a>
## F-MD-027 — alarms

**Original requirement:** recurrence/sound/notification/suspend wake and dismissal/snooze

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-027.01 — Reconcile existing alarms**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for alarms. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: recurrence/sound/notification/suspend wake and dismissal/snooze
- [ ] **F-MD-027.02 — Freeze the exact contract for alarms**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: recurrence/sound/notification/suspend wake and dismissal/snooze. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-027.03 — Implement/prove: recurrence/sound/notification/suspend wake and dismissal/snooze**
  - Action: For alarms, implement or reuse and verify this exact obligation: recurrence/sound/notification/suspend wake and dismissal/snooze. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recurrence/sound/notification/suspend wake and dismissal/snooze; retain observable state/resource expectations.
- [ ] **F-MD-027.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to alarms: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-027.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-027.05 — Integrate into the real consumer and runtime route**
  - Action: Wire alarms into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-027.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-027.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for alarms as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-027.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-028"></a>
## F-MD-028 — password manager UI

**Original requirement:** human unlock, scoped fill/copy, generator, audit, export and secret-zero policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-028.01 — Reconcile existing password manager UI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for password manager UI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: human unlock, scoped fill/copy, generator, audit, export and secret-zero policy
- [ ] **F-MD-028.02 — Freeze the exact contract for password manager UI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: human unlock, scoped fill/copy, generator, audit, export and secret-zero policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-028.03 — Implement/prove: human unlock**
  - Action: For password manager UI, implement or reuse and verify this exact obligation: human unlock. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for human unlock; retain observable state/resource expectations.
- [ ] **F-MD-028.04 — Implement/prove: scoped fill/copy**
  - Action: For password manager UI, implement or reuse and verify this exact obligation: scoped fill/copy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scoped fill/copy; retain observable state/resource expectations.
- [ ] **F-MD-028.05 — Implement/prove: generator**
  - Action: For password manager UI, implement or reuse and verify this exact obligation: generator. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generator; retain observable state/resource expectations.
- [ ] **F-MD-028.06 — Implement/prove: audit**
  - Action: For password manager UI, implement or reuse and verify this exact obligation: audit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-028.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for audit; retain observable state/resource expectations.
- [ ] **F-MD-028.07 — Implement/prove: export and secret-zero policy**
  - Action: For password manager UI, implement or reuse and verify this exact obligation: export and secret-zero policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-028.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for export and secret-zero policy; retain observable state/resource expectations.
- [ ] **F-MD-028.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to password manager UI: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-028.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-028.09 — Integrate into the real consumer and runtime route**
  - Action: Wire password manager UI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-028.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-028.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for password manager UI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-028.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-029"></a>
## F-MD-029 — download manager

**Original requirement:** source/destination/progress/pause/resume/cancel/hash/quarantine and history

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-029.01 — Reconcile existing download manager**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for download manager. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: source/destination/progress/pause/resume/cancel/hash/quarantine and history
- [ ] **F-MD-029.02 — Freeze the exact contract for download manager**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: source/destination/progress/pause/resume/cancel/hash/quarantine and history. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-029.03 — Implement/prove: source/destination/progress/pause/resume/cancel/hash/quarantine and history**
  - Action: For download manager, implement or reuse and verify this exact obligation: source/destination/progress/pause/resume/cancel/hash/quarantine and history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source/destination/progress/pause/resume/cancel/hash/quarantine and history; retain observable state/resource expectations.
- [ ] **F-MD-029.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to download manager: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-029.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-029.05 — Integrate into the real consumer and runtime route**
  - Action: Wire download manager into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-029.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-029.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for download manager as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-029.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-030"></a>
## F-MD-030 — sharing center

**Original requirement:** nearby/network/app targets, user consent, progress, revoke and audit

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-030.01 — Reconcile existing sharing center**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for sharing center. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: nearby/network/app targets, user consent, progress, revoke and audit
- [ ] **F-MD-030.02 — Freeze the exact contract for sharing center**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: nearby/network/app targets, user consent, progress, revoke and audit. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-030.03 — Implement/prove: nearby/network/app targets**
  - Action: For sharing center, implement or reuse and verify this exact obligation: nearby/network/app targets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for nearby/network/app targets; retain observable state/resource expectations.
- [ ] **F-MD-030.04 — Implement/prove: user consent**
  - Action: For sharing center, implement or reuse and verify this exact obligation: user consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user consent; retain observable state/resource expectations.
- [ ] **F-MD-030.05 — Implement/prove: progress**
  - Action: For sharing center, implement or reuse and verify this exact obligation: progress. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-030.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for progress; retain observable state/resource expectations.
- [ ] **F-MD-030.06 — Implement/prove: revoke and audit**
  - Action: For sharing center, implement or reuse and verify this exact obligation: revoke and audit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-030.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for revoke and audit; retain observable state/resource expectations.
- [ ] **F-MD-030.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to sharing center: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-030.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-030.08 — Integrate into the real consumer and runtime route**
  - Action: Wire sharing center into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-030.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-030.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for sharing center as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-030.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-031"></a>
## F-MD-031 — voice/video calling

**Original requirement:** identity/signaling/media encryption/devices/quality/privacy and failure recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-031.01 — Reconcile existing voice/video calling**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for voice/video calling. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: identity/signaling/media encryption/devices/quality/privacy and failure recovery
- [ ] **F-MD-031.02 — Freeze the exact contract for voice/video calling**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: identity/signaling/media encryption/devices/quality/privacy and failure recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-031.03 — Implement/prove: identity/signaling/media encryption/devices/quality/privacy and failure recovery**
  - Action: For voice/video calling, implement or reuse and verify this exact obligation: identity/signaling/media encryption/devices/quality/privacy and failure recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for identity/signaling/media encryption/devices/quality/privacy and failure recovery; retain observable state/resource expectations.
- [ ] **F-MD-031.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to voice/video calling: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-031.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-031.05 — Integrate into the real consumer and runtime route**
  - Action: Wire voice/video calling into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-031.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-031.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for voice/video calling as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-031.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-032"></a>
## F-MD-032 — podcast/radio

**Original requirement:** feeds/streams/downloads/queue/metadata/offline and network/audio integration

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-032.01 — Reconcile existing podcast/radio**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for podcast/radio. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: feeds/streams/downloads/queue/metadata/offline and network/audio integration
- [ ] **F-MD-032.02 — Freeze the exact contract for podcast/radio**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: feeds/streams/downloads/queue/metadata/offline and network/audio integration. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-032.03 — Implement/prove: feeds/streams/downloads/queue/metadata/offline and network/audio integration**
  - Action: For podcast/radio, implement or reuse and verify this exact obligation: feeds/streams/downloads/queue/metadata/offline and network/audio integration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for feeds/streams/downloads/queue/metadata/offline and network/audio integration; retain observable state/resource expectations.
- [ ] **F-MD-032.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to podcast/radio: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-032.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-032.05 — Integrate into the real consumer and runtime route**
  - Action: Wire podcast/radio into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-032.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-032.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for podcast/radio as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-032.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-md-033"></a>
## F-MD-033 — screen sharing

**Original requirement:** target consent/indicator/protected content/input policy and revocation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-MD-033.01 — Reconcile existing screen sharing**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for screen sharing. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: target consent/indicator/protected content/input policy and revocation
- [ ] **F-MD-033.02 — Freeze the exact contract for screen sharing**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: target consent/indicator/protected content/input policy and revocation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-MD-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-MD-033.03 — Implement/prove: target consent/indicator/protected content/input policy and revocation**
  - Action: For screen sharing, implement or reuse and verify this exact obligation: target consent/indicator/protected content/input policy and revocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-MD-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for target consent/indicator/protected content/input policy and revocation; retain observable state/resource expectations.
- [ ] **F-MD-033.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to screen sharing: malformed media/message; decoder/resource exhaustion; account/grant denial; offline/conflict; cancelled sync/export; capture revocation; crash and durable reopen. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-MD-033.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-MD-033.05 — Integrate into the real consumer and runtime route**
  - Action: Wire screen sharing into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-MD-033.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-MD-033.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for screen sharing as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-MD-033.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-001"></a>
## F-ST-001 — Settings application

**Original requirement:** searchable categories, effective state, validation, preview, apply/rollback and profiles

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-001.01 — Reconcile existing Settings application**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Settings application. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: searchable categories, effective state, validation, preview, apply/rollback and profiles
- [ ] **F-ST-001.02 — Freeze the exact contract for Settings application**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: searchable categories, effective state, validation, preview, apply/rollback and profiles. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-001.03 — Implement/prove: searchable categories**
  - Action: For Settings application, implement or reuse and verify this exact obligation: searchable categories. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for searchable categories; retain observable state/resource expectations.
- [ ] **F-ST-001.04 — Implement/prove: effective state**
  - Action: For Settings application, implement or reuse and verify this exact obligation: effective state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for effective state; retain observable state/resource expectations.
- [ ] **F-ST-001.05 — Implement/prove: validation**
  - Action: For Settings application, implement or reuse and verify this exact obligation: validation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validation; retain observable state/resource expectations.
- [ ] **F-ST-001.06 — Implement/prove: preview**
  - Action: For Settings application, implement or reuse and verify this exact obligation: preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for preview; retain observable state/resource expectations.
- [ ] **F-ST-001.07 — Implement/prove: apply/rollback and profiles**
  - Action: For Settings application, implement or reuse and verify this exact obligation: apply/rollback and profiles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for apply/rollback and profiles; retain observable state/resource expectations.
- [ ] **F-ST-001.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Settings application: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-001.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-001.09 — Integrate into the real consumer and runtime route**
  - Action: Wire Settings application into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-001.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-001.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Settings application as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-001.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-002"></a>
## F-ST-002 — display settings

**Original requirement:** modes/scales/rotation/topology/night/color with timed rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-002.01 — Reconcile existing display settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for display settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: modes/scales/rotation/topology/night/color with timed rollback
- [ ] **F-ST-002.02 — Freeze the exact contract for display settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: modes/scales/rotation/topology/night/color with timed rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-002.03 — Implement/prove: modes/scales/rotation/topology/night/color with timed rollback**
  - Action: For display settings, implement or reuse and verify this exact obligation: modes/scales/rotation/topology/night/color with timed rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for modes/scales/rotation/topology/night/color with timed rollback; retain observable state/resource expectations.
- [ ] **F-ST-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to display settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire display settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for display settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-003"></a>
## F-ST-003 — appearance settings

**Original requirement:** theme/wallpaper/density/font/icons/effects/reduced motion with live preview

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-003.01 — Reconcile existing appearance settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for appearance settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: theme/wallpaper/density/font/icons/effects/reduced motion with live preview
- [ ] **F-ST-003.02 — Freeze the exact contract for appearance settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: theme/wallpaper/density/font/icons/effects/reduced motion with live preview. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-003.03 — Implement/prove: theme/wallpaper/density/font/icons/effects/reduced motion with live preview**
  - Action: For appearance settings, implement or reuse and verify this exact obligation: theme/wallpaper/density/font/icons/effects/reduced motion with live preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for theme/wallpaper/density/font/icons/effects/reduced motion with live preview; retain observable state/resource expectations.
- [ ] **F-ST-003.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to appearance settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-003.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-003.05 — Integrate into the real consumer and runtime route**
  - Action: Wire appearance settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-003.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-003.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for appearance settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-003.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-004"></a>
## F-ST-004 — input settings

**Original requirement:** devices/layouts/repeat/pointer speed/acceleration/buttons/calibration/remap

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-004.01 — Reconcile existing input settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for input settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: devices/layouts/repeat/pointer speed/acceleration/buttons/calibration/remap
- [ ] **F-ST-004.02 — Freeze the exact contract for input settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: devices/layouts/repeat/pointer speed/acceleration/buttons/calibration/remap. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-004.03 — Implement/prove: devices/layouts/repeat/pointer speed/acceleration/buttons/calibration/remap**
  - Action: For input settings, implement or reuse and verify this exact obligation: devices/layouts/repeat/pointer speed/acceleration/buttons/calibration/remap. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for devices/layouts/repeat/pointer speed/acceleration/buttons/calibration/remap; retain observable state/resource expectations.
- [ ] **F-ST-004.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to input settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-004.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-004.05 — Integrate into the real consumer and runtime route**
  - Action: Wire input settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-004.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-004.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for input settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-004.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-005"></a>
## F-ST-005 — audio settings

**Original requirement:** devices/routes/volume/balance/test/per-app streams and errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-005.01 — Reconcile existing audio settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: devices/routes/volume/balance/test/per-app streams and errors
- [ ] **F-ST-005.02 — Freeze the exact contract for audio settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: devices/routes/volume/balance/test/per-app streams and errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-005.03 — Implement/prove: devices/routes/volume/balance/test/per-app streams and errors**
  - Action: For audio settings, implement or reuse and verify this exact obligation: devices/routes/volume/balance/test/per-app streams and errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for devices/routes/volume/balance/test/per-app streams and errors; retain observable state/resource expectations.
- [ ] **F-ST-005.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-005.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-005.05 — Integrate into the real consumer and runtime route**
  - Action: Wire audio settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-005.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-005.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-005.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-006"></a>
## F-ST-006 — network settings

**Original requirement:** interfaces/Wi-Fi/profiles/IP/DNS/proxy/VPN/firewall and diagnostics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-006.01 — Reconcile existing network settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for network settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: interfaces/Wi-Fi/profiles/IP/DNS/proxy/VPN/firewall and diagnostics
- [ ] **F-ST-006.02 — Freeze the exact contract for network settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: interfaces/Wi-Fi/profiles/IP/DNS/proxy/VPN/firewall and diagnostics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-006.03 — Implement/prove: interfaces/Wi-Fi/profiles/IP/DNS/proxy/VPN/firewall and diagnostics**
  - Action: For network settings, implement or reuse and verify this exact obligation: interfaces/Wi-Fi/profiles/IP/DNS/proxy/VPN/firewall and diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for interfaces/Wi-Fi/profiles/IP/DNS/proxy/VPN/firewall and diagnostics; retain observable state/resource expectations.
- [ ] **F-ST-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to network settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire network settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for network settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-007"></a>
## F-ST-007 — accounts settings

**Original requirement:** users/groups/password/recovery/admin/session and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-007.01 — Reconcile existing accounts settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for accounts settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: users/groups/password/recovery/admin/session and privacy
- [ ] **F-ST-007.02 — Freeze the exact contract for accounts settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: users/groups/password/recovery/admin/session and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-007.03 — Implement/prove: users/groups/password/recovery/admin/session and privacy**
  - Action: For accounts settings, implement or reuse and verify this exact obligation: users/groups/password/recovery/admin/session and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for users/groups/password/recovery/admin/session and privacy; retain observable state/resource expectations.
- [ ] **F-ST-007.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to accounts settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-007.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-007.05 — Integrate into the real consumer and runtime route**
  - Action: Wire accounts settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-007.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-007.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for accounts settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-007.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-008"></a>
## F-ST-008 — application settings

**Original requirement:** defaults, permissions, storage, background, notifications, repair/uninstall

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-008.01 — Reconcile existing application settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for application settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: defaults, permissions, storage, background, notifications, repair/uninstall
- [ ] **F-ST-008.02 — Freeze the exact contract for application settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: defaults, permissions, storage, background, notifications, repair/uninstall. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-008.03 — Implement/prove: defaults**
  - Action: For application settings, implement or reuse and verify this exact obligation: defaults. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for defaults; retain observable state/resource expectations.
- [ ] **F-ST-008.04 — Implement/prove: permissions**
  - Action: For application settings, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-ST-008.05 — Implement/prove: storage**
  - Action: For application settings, implement or reuse and verify this exact obligation: storage. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for storage; retain observable state/resource expectations.
- [ ] **F-ST-008.06 — Implement/prove: background**
  - Action: For application settings, implement or reuse and verify this exact obligation: background. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-008.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for background; retain observable state/resource expectations.
- [ ] **F-ST-008.07 — Implement/prove: notifications**
  - Action: For application settings, implement or reuse and verify this exact obligation: notifications. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-008.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for notifications; retain observable state/resource expectations.
- [ ] **F-ST-008.08 — Implement/prove: repair/uninstall**
  - Action: For application settings, implement or reuse and verify this exact obligation: repair/uninstall. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-008.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for repair/uninstall; retain observable state/resource expectations.
- [ ] **F-ST-008.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to application settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-008.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-008.10 — Integrate into the real consumer and runtime route**
  - Action: Wire application settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-008.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-008.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for application settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-008.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-009"></a>
## F-ST-009 — accessibility settings

**Original requirement:** all assistive providers/preferences with preview/test/rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-009.01 — Reconcile existing accessibility settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for accessibility settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: all assistive providers/preferences with preview/test/rollback
- [ ] **F-ST-009.02 — Freeze the exact contract for accessibility settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: all assistive providers/preferences with preview/test/rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-009.03 — Implement/prove: all assistive providers/preferences with preview/test/rollback**
  - Action: For accessibility settings, implement or reuse and verify this exact obligation: all assistive providers/preferences with preview/test/rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for all assistive providers/preferences with preview/test/rollback; retain observable state/resource expectations.
- [ ] **F-ST-009.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to accessibility settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-009.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-009.05 — Integrate into the real consumer and runtime route**
  - Action: Wire accessibility settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-009.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-009.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for accessibility settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-009.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-010"></a>
## F-ST-010 — locale/time settings

**Original requirement:** language/format/timezone/clock sync/keyboard/input methods

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-010.01 — Reconcile existing locale/time settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for locale/time settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: language/format/timezone/clock sync/keyboard/input methods
- [ ] **F-ST-010.02 — Freeze the exact contract for locale/time settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: language/format/timezone/clock sync/keyboard/input methods. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-010.03 — Implement/prove: language/format/timezone/clock sync/keyboard/input methods**
  - Action: For locale/time settings, implement or reuse and verify this exact obligation: language/format/timezone/clock sync/keyboard/input methods. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for language/format/timezone/clock sync/keyboard/input methods; retain observable state/resource expectations.
- [ ] **F-ST-010.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to locale/time settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-010.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-010.05 — Integrate into the real consumer and runtime route**
  - Action: Wire locale/time settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-010.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-010.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for locale/time settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-010.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-011"></a>
## F-ST-011 — power settings

**Original requirement:** sleep/display/battery/thermal/performance profiles and truthful capability

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-011.01 — Reconcile existing power settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for power settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: sleep/display/battery/thermal/performance profiles and truthful capability
- [ ] **F-ST-011.02 — Freeze the exact contract for power settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: sleep/display/battery/thermal/performance profiles and truthful capability. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-011.03 — Implement/prove: sleep/display/battery/thermal/performance profiles and truthful capability**
  - Action: For power settings, implement or reuse and verify this exact obligation: sleep/display/battery/thermal/performance profiles and truthful capability. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sleep/display/battery/thermal/performance profiles and truthful capability; retain observable state/resource expectations.
- [ ] **F-ST-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to power settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire power settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for power settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-012"></a>
## F-ST-012 — privacy settings

**Original requirement:** permission history, telemetry, recent data, capture, location and clearing

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-012.01 — Reconcile existing privacy settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for privacy settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: permission history, telemetry, recent data, capture, location and clearing
- [ ] **F-ST-012.02 — Freeze the exact contract for privacy settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: permission history, telemetry, recent data, capture, location and clearing. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-012.03 — Implement/prove: permission history**
  - Action: For privacy settings, implement or reuse and verify this exact obligation: permission history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permission history; retain observable state/resource expectations.
- [ ] **F-ST-012.04 — Implement/prove: telemetry**
  - Action: For privacy settings, implement or reuse and verify this exact obligation: telemetry. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for telemetry; retain observable state/resource expectations.
- [ ] **F-ST-012.05 — Implement/prove: recent data**
  - Action: For privacy settings, implement or reuse and verify this exact obligation: recent data. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recent data; retain observable state/resource expectations.
- [ ] **F-ST-012.06 — Implement/prove: capture**
  - Action: For privacy settings, implement or reuse and verify this exact obligation: capture. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-012.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capture; retain observable state/resource expectations.
- [ ] **F-ST-012.07 — Implement/prove: location and clearing**
  - Action: For privacy settings, implement or reuse and verify this exact obligation: location and clearing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-012.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for location and clearing; retain observable state/resource expectations.
- [ ] **F-ST-012.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to privacy settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-012.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-012.09 — Integrate into the real consumer and runtime route**
  - Action: Wire privacy settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-012.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-012.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for privacy settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-012.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-013"></a>
## F-ST-013 — update settings

**Original requirement:** channels/checks/download/install/schedule/history/rollback and provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-013.01 — Reconcile existing update settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for update settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: channels/checks/download/install/schedule/history/rollback and provenance
- [ ] **F-ST-013.02 — Freeze the exact contract for update settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: channels/checks/download/install/schedule/history/rollback and provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-013.03 — Implement/prove: channels/checks/download/install/schedule/history/rollback and provenance**
  - Action: For update settings, implement or reuse and verify this exact obligation: channels/checks/download/install/schedule/history/rollback and provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for channels/checks/download/install/schedule/history/rollback and provenance; retain observable state/resource expectations.
- [ ] **F-ST-013.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to update settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-013.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-013.05 — Integrate into the real consumer and runtime route**
  - Action: Wire update settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-013.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-013.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for update settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-013.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-014"></a>
## F-ST-014 — storage settings

**Original requirement:** volumes/usage/cleanup/encryption/removable/backup and disk health

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-014.01 — Reconcile existing storage settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for storage settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: volumes/usage/cleanup/encryption/removable/backup and disk health
- [ ] **F-ST-014.02 — Freeze the exact contract for storage settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: volumes/usage/cleanup/encryption/removable/backup and disk health. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-014.03 — Implement/prove: volumes/usage/cleanup/encryption/removable/backup and disk health**
  - Action: For storage settings, implement or reuse and verify this exact obligation: volumes/usage/cleanup/encryption/removable/backup and disk health. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for volumes/usage/cleanup/encryption/removable/backup and disk health; retain observable state/resource expectations.
- [ ] **F-ST-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to storage settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire storage settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for storage settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-015"></a>
## F-ST-015 — device manager

**Original requirement:** hardware/driver/resources/status/permissions/reset/eject/evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-015.01 — Reconcile existing device manager**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for device manager. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: hardware/driver/resources/status/permissions/reset/eject/evidence
- [ ] **F-ST-015.02 — Freeze the exact contract for device manager**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: hardware/driver/resources/status/permissions/reset/eject/evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-015.03 — Implement/prove: hardware/driver/resources/status/permissions/reset/eject/evidence**
  - Action: For device manager, implement or reuse and verify this exact obligation: hardware/driver/resources/status/permissions/reset/eject/evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hardware/driver/resources/status/permissions/reset/eject/evidence; retain observable state/resource expectations.
- [ ] **F-ST-015.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to device manager: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-015.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-015.05 — Integrate into the real consumer and runtime route**
  - Action: Wire device manager into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-015.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-015.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for device manager as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-015.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-016"></a>
## F-ST-016 — printer/scanner settings

**Original requirement:** discovery/defaults/options/queues/test/error and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-016.01 — Reconcile existing printer/scanner settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for printer/scanner settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: discovery/defaults/options/queues/test/error and privacy
- [ ] **F-ST-016.02 — Freeze the exact contract for printer/scanner settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: discovery/defaults/options/queues/test/error and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-016.03 — Implement/prove: discovery/defaults/options/queues/test/error and privacy**
  - Action: For printer/scanner settings, implement or reuse and verify this exact obligation: discovery/defaults/options/queues/test/error and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discovery/defaults/options/queues/test/error and privacy; retain observable state/resource expectations.
- [ ] **F-ST-016.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to printer/scanner settings: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-016.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-016.05 — Integrate into the real consumer and runtime route**
  - Action: Wire printer/scanner settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-016.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-016.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for printer/scanner settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-016.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-017"></a>
## F-ST-017 — system information

**Original requirement:** CPU/memory/firmware/devices/build/ABI/licenses/security and copy/export

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-017.01 — Reconcile existing system information**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for system information. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: CPU/memory/firmware/devices/build/ABI/licenses/security and copy/export
- [ ] **F-ST-017.02 — Freeze the exact contract for system information**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: CPU/memory/firmware/devices/build/ABI/licenses/security and copy/export. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-017.03 — Implement/prove: CPU/memory/firmware/devices/build/ABI/licenses/security and copy/export**
  - Action: For system information, implement or reuse and verify this exact obligation: CPU/memory/firmware/devices/build/ABI/licenses/security and copy/export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for CPU/memory/firmware/devices/build/ABI/licenses/security and copy/export; retain observable state/resource expectations.
- [ ] **F-ST-017.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to system information: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-017.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-017.05 — Integrate into the real consumer and runtime route**
  - Action: Wire system information into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-017.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-017.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for system information as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-017.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-018"></a>
## F-ST-018 — system monitor

**Original requirement:** processes/CPU/memory/I/O/network/graphics/services with units and control authority

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-018.01 — Reconcile existing system monitor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for system monitor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: processes/CPU/memory/I/O/network/graphics/services with units and control authority
- [ ] **F-ST-018.02 — Freeze the exact contract for system monitor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: processes/CPU/memory/I/O/network/graphics/services with units and control authority. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-018.03 — Implement/prove: processes/CPU/memory/I/O/network/graphics/services with units and control authority**
  - Action: For system monitor, implement or reuse and verify this exact obligation: processes/CPU/memory/I/O/network/graphics/services with units and control authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for processes/CPU/memory/I/O/network/graphics/services with units and control authority; retain observable state/resource expectations.
- [ ] **F-ST-018.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to system monitor: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-018.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-018.05 — Integrate into the real consumer and runtime route**
  - Action: Wire system monitor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-018.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-018.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for system monitor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-018.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-019"></a>
## F-ST-019 — process manager

**Original requirement:** inspect/filter/signal/priority/resources/details with permission checks

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-019.01 — Reconcile existing process manager**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for process manager. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: inspect/filter/signal/priority/resources/details with permission checks
- [ ] **F-ST-019.02 — Freeze the exact contract for process manager**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: inspect/filter/signal/priority/resources/details with permission checks. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-019.03 — Implement/prove: inspect/filter/signal/priority/resources/details with permission checks**
  - Action: For process manager, implement or reuse and verify this exact obligation: inspect/filter/signal/priority/resources/details with permission checks. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for inspect/filter/signal/priority/resources/details with permission checks; retain observable state/resource expectations.
- [ ] **F-ST-019.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to process manager: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-019.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-019.05 — Integrate into the real consumer and runtime route**
  - Action: Wire process manager into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-019.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-019.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for process manager as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-019.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-020"></a>
## F-ST-020 — services manager

**Original requirement:** supervisor state/dependencies/health/logs/start/stop/restart/quarantine and auth

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-020.01 — Reconcile existing services manager**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for services manager. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: supervisor state/dependencies/health/logs/start/stop/restart/quarantine and auth
- [ ] **F-ST-020.02 — Freeze the exact contract for services manager**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: supervisor state/dependencies/health/logs/start/stop/restart/quarantine and auth. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-020.03 — Implement/prove: supervisor state/dependencies/health/logs/start/stop/restart/quarantine and auth**
  - Action: For services manager, implement or reuse and verify this exact obligation: supervisor state/dependencies/health/logs/start/stop/restart/quarantine and auth. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for supervisor state/dependencies/health/logs/start/stop/restart/quarantine and auth; retain observable state/resource expectations.
- [ ] **F-ST-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to services manager: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire services manager into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for services manager as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-021"></a>
## F-ST-021 — driver monitor

**Original requirement:** lifecycle/resources/IRQs/DMA/faults/recovery and exact provider evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-021.01 — Reconcile existing driver monitor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for driver monitor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: lifecycle/resources/IRQs/DMA/faults/recovery and exact provider evidence
- [ ] **F-ST-021.02 — Freeze the exact contract for driver monitor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: lifecycle/resources/IRQs/DMA/faults/recovery and exact provider evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-021.03 — Implement/prove: lifecycle/resources/IRQs/DMA/faults/recovery and exact provider evidence**
  - Action: For driver monitor, implement or reuse and verify this exact obligation: lifecycle/resources/IRQs/DMA/faults/recovery and exact provider evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lifecycle/resources/IRQs/DMA/faults/recovery and exact provider evidence; retain observable state/resource expectations.
- [ ] **F-ST-021.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to driver monitor: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-021.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-021.05 — Integrate into the real consumer and runtime route**
  - Action: Wire driver monitor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-021.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-021.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for driver monitor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-021.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-022"></a>
## F-ST-022 — network monitor

**Original requirement:** link/routes/sockets/DNS/traffic/errors and privacy-aware process attribution

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-022.01 — Reconcile existing network monitor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for network monitor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: link/routes/sockets/DNS/traffic/errors and privacy-aware process attribution
- [ ] **F-ST-022.02 — Freeze the exact contract for network monitor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: link/routes/sockets/DNS/traffic/errors and privacy-aware process attribution. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-022.03 — Implement/prove: link/routes/sockets/DNS/traffic/errors and privacy-aware process attribution**
  - Action: For network monitor, implement or reuse and verify this exact obligation: link/routes/sockets/DNS/traffic/errors and privacy-aware process attribution. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for link/routes/sockets/DNS/traffic/errors and privacy-aware process attribution; retain observable state/resource expectations.
- [ ] **F-ST-022.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to network monitor: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-022.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-022.05 — Integrate into the real consumer and runtime route**
  - Action: Wire network monitor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-022.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-022.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for network monitor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-022.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-023"></a>
## F-ST-023 — storage monitor

**Original requirement:** devices/queues/cache/fs health/space/writeback/flush/errors and detach

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-023.01 — Reconcile existing storage monitor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for storage monitor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: devices/queues/cache/fs health/space/writeback/flush/errors and detach
- [ ] **F-ST-023.02 — Freeze the exact contract for storage monitor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: devices/queues/cache/fs health/space/writeback/flush/errors and detach. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-023.03 — Implement/prove: devices/queues/cache/fs health/space/writeback/flush/errors and detach**
  - Action: For storage monitor, implement or reuse and verify this exact obligation: devices/queues/cache/fs health/space/writeback/flush/errors and detach. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for devices/queues/cache/fs health/space/writeback/flush/errors and detach; retain observable state/resource expectations.
- [ ] **F-ST-023.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to storage monitor: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-023.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-023.05 — Integrate into the real consumer and runtime route**
  - Action: Wire storage monitor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-023.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-023.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for storage monitor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-023.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-024"></a>
## F-ST-024 — graphics monitor

**Original requirement:** mode/backend/frame phases/damage/cache/fences/late frames and capture

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-024.01 — Reconcile existing graphics monitor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for graphics monitor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: mode/backend/frame phases/damage/cache/fences/late frames and capture
- [ ] **F-ST-024.02 — Freeze the exact contract for graphics monitor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: mode/backend/frame phases/damage/cache/fences/late frames and capture. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-024.03 — Implement/prove: mode/backend/frame phases/damage/cache/fences/late frames and capture**
  - Action: For graphics monitor, implement or reuse and verify this exact obligation: mode/backend/frame phases/damage/cache/fences/late frames and capture. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mode/backend/frame phases/damage/cache/fences/late frames and capture; retain observable state/resource expectations.
- [ ] **F-ST-024.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to graphics monitor: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-024.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-024.05 — Integrate into the real consumer and runtime route**
  - Action: Wire graphics monitor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-024.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-024.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for graphics monitor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-024.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-025"></a>
## F-ST-025 — audio monitor

**Original requirement:** devices/streams/xruns/latency/clocks and permissions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-025.01 — Reconcile existing audio monitor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio monitor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: devices/streams/xruns/latency/clocks and permissions
- [ ] **F-ST-025.02 — Freeze the exact contract for audio monitor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: devices/streams/xruns/latency/clocks and permissions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-025.03 — Implement/prove: devices/streams/xruns/latency/clocks and permissions**
  - Action: For audio monitor, implement or reuse and verify this exact obligation: devices/streams/xruns/latency/clocks and permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for devices/streams/xruns/latency/clocks and permissions; retain observable state/resource expectations.
- [ ] **F-ST-025.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio monitor: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-025.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-025.05 — Integrate into the real consumer and runtime route**
  - Action: Wire audio monitor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-025.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-025.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio monitor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-025.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-026"></a>
## F-ST-026 — kernel/system log viewer

**Original requirement:** filter/search/correlation/redaction/persistent receipts/export and privilege

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-026.01 — Reconcile existing kernel/system log viewer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for kernel/system log viewer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: filter/search/correlation/redaction/persistent receipts/export and privilege
- [ ] **F-ST-026.02 — Freeze the exact contract for kernel/system log viewer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: filter/search/correlation/redaction/persistent receipts/export and privilege. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-026.03 — Implement/prove: filter/search/correlation/redaction/persistent receipts/export and privilege**
  - Action: For kernel/system log viewer, implement or reuse and verify this exact obligation: filter/search/correlation/redaction/persistent receipts/export and privilege. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for filter/search/correlation/redaction/persistent receipts/export and privilege; retain observable state/resource expectations.
- [ ] **F-ST-026.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to kernel/system log viewer: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-026.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-026.05 — Integrate into the real consumer and runtime route**
  - Action: Wire kernel/system log viewer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-026.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-026.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for kernel/system log viewer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-026.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-027"></a>
## F-ST-027 — crash center

**Original requirement:** app/service/kernel crashes, symbols, privacy, reports and recovery actions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-027.01 — Reconcile existing crash center**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for crash center. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: app/service/kernel crashes, symbols, privacy, reports and recovery actions
- [ ] **F-ST-027.02 — Freeze the exact contract for crash center**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: app/service/kernel crashes, symbols, privacy, reports and recovery actions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-027.03 — Implement/prove: app/service/kernel crashes**
  - Action: For crash center, implement or reuse and verify this exact obligation: app/service/kernel crashes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app/service/kernel crashes; retain observable state/resource expectations.
- [ ] **F-ST-027.04 — Implement/prove: symbols**
  - Action: For crash center, implement or reuse and verify this exact obligation: symbols. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for symbols; retain observable state/resource expectations.
- [ ] **F-ST-027.05 — Implement/prove: privacy**
  - Action: For crash center, implement or reuse and verify this exact obligation: privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy; retain observable state/resource expectations.
- [ ] **F-ST-027.06 — Implement/prove: reports and recovery actions**
  - Action: For crash center, implement or reuse and verify this exact obligation: reports and recovery actions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-027.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reports and recovery actions; retain observable state/resource expectations.
- [ ] **F-ST-027.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to crash center: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-027.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-027.08 — Integrate into the real consumer and runtime route**
  - Action: Wire crash center into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-027.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-027.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for crash center as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-027.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-028"></a>
## F-ST-028 — benchmark app

**Original requirement:** named reproducible workloads, comparisons, artifacts and thermal/host context

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-028.01 — Reconcile existing benchmark app**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for benchmark app. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: named reproducible workloads, comparisons, artifacts and thermal/host context
- [ ] **F-ST-028.02 — Freeze the exact contract for benchmark app**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: named reproducible workloads, comparisons, artifacts and thermal/host context. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-028.03 — Implement/prove: named reproducible workloads**
  - Action: For benchmark app, implement or reuse and verify this exact obligation: named reproducible workloads. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for named reproducible workloads; retain observable state/resource expectations.
- [ ] **F-ST-028.04 — Implement/prove: comparisons**
  - Action: For benchmark app, implement or reuse and verify this exact obligation: comparisons. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for comparisons; retain observable state/resource expectations.
- [ ] **F-ST-028.05 — Implement/prove: artifacts and thermal/host context**
  - Action: For benchmark app, implement or reuse and verify this exact obligation: artifacts and thermal/host context. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for artifacts and thermal/host context; retain observable state/resource expectations.
- [ ] **F-ST-028.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to benchmark app: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-028.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-028.07 — Integrate into the real consumer and runtime route**
  - Action: Wire benchmark app into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-028.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-028.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for benchmark app as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-028.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-029"></a>
## F-ST-029 — diagnostics suite

**Original requirement:** guided CPU/memory/storage/display/input/network/audio tests and export

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-029.01 — Reconcile existing diagnostics suite**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for diagnostics suite. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: guided CPU/memory/storage/display/input/network/audio tests and export
- [ ] **F-ST-029.02 — Freeze the exact contract for diagnostics suite**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: guided CPU/memory/storage/display/input/network/audio tests and export. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-029.03 — Implement/prove: guided CPU/memory/storage/display/input/network/audio tests and export**
  - Action: For diagnostics suite, implement or reuse and verify this exact obligation: guided CPU/memory/storage/display/input/network/audio tests and export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for guided CPU/memory/storage/display/input/network/audio tests and export; retain observable state/resource expectations.
- [ ] **F-ST-029.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to diagnostics suite: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-029.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-029.05 — Integrate into the real consumer and runtime route**
  - Action: Wire diagnostics suite into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-029.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-029.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for diagnostics suite as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-029.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-030"></a>
## F-ST-030 — health dashboard

**Original requirement:** service/device/update/backup/security state with actionable evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-030.01 — Reconcile existing health dashboard**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for health dashboard. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: service/device/update/backup/security state with actionable evidence
- [ ] **F-ST-030.02 — Freeze the exact contract for health dashboard**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: service/device/update/backup/security state with actionable evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-030.03 — Implement/prove: service/device/update/backup/security state with actionable evidence**
  - Action: For health dashboard, implement or reuse and verify this exact obligation: service/device/update/backup/security state with actionable evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for service/device/update/backup/security state with actionable evidence; retain observable state/resource expectations.
- [ ] **F-ST-030.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to health dashboard: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-030.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-030.05 — Integrate into the real consumer and runtime route**
  - Action: Wire health dashboard into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-030.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-030.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for health dashboard as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-030.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-031"></a>
## F-ST-031 — disk repair UI

**Original requirement:** read-only diagnosis, target confirmation, backup, repair plan/result and rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-031.01 — Reconcile existing disk repair UI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for disk repair UI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: read-only diagnosis, target confirmation, backup, repair plan/result and rollback
- [ ] **F-ST-031.02 — Freeze the exact contract for disk repair UI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: read-only diagnosis, target confirmation, backup, repair plan/result and rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-031.03 — Implement/prove: read-only diagnosis**
  - Action: For disk repair UI, implement or reuse and verify this exact obligation: read-only diagnosis. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for read-only diagnosis; retain observable state/resource expectations.
- [ ] **F-ST-031.04 — Implement/prove: target confirmation**
  - Action: For disk repair UI, implement or reuse and verify this exact obligation: target confirmation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for target confirmation; retain observable state/resource expectations.
- [ ] **F-ST-031.05 — Implement/prove: backup**
  - Action: For disk repair UI, implement or reuse and verify this exact obligation: backup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-031.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for backup; retain observable state/resource expectations.
- [ ] **F-ST-031.06 — Implement/prove: repair plan/result and rollback**
  - Action: For disk repair UI, implement or reuse and verify this exact obligation: repair plan/result and rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-031.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for repair plan/result and rollback; retain observable state/resource expectations.
- [ ] **F-ST-031.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to disk repair UI: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-031.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-031.08 — Integrate into the real consumer and runtime route**
  - Action: Wire disk repair UI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-031.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-031.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for disk repair UI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-031.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-032"></a>
## F-ST-032 — boot recovery UI

**Original requirement:** generations/safe mode/logs/export/repair with keyboard/a11y support

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-032.01 — Reconcile existing boot recovery UI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for boot recovery UI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generations/safe mode/logs/export/repair with keyboard/a11y support
- [ ] **F-ST-032.02 — Freeze the exact contract for boot recovery UI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generations/safe mode/logs/export/repair with keyboard/a11y support. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-032.03 — Implement/prove: generations/safe mode/logs/export/repair with keyboard/a11y support**
  - Action: For boot recovery UI, implement or reuse and verify this exact obligation: generations/safe mode/logs/export/repair with keyboard/a11y support. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generations/safe mode/logs/export/repair with keyboard/a11y support; retain observable state/resource expectations.
- [ ] **F-ST-032.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to boot recovery UI: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-032.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-032.05 — Integrate into the real consumer and runtime route**
  - Action: Wire boot recovery UI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-032.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-032.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for boot recovery UI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-032.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-033"></a>
## F-ST-033 — factory/user reset

**Original requirement:** exact retained/erased data, confirmation, backup, cryptographic erase and receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-033.01 — Reconcile existing factory/user reset**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for factory/user reset. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact retained/erased data, confirmation, backup, cryptographic erase and receipts
- [ ] **F-ST-033.02 — Freeze the exact contract for factory/user reset**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact retained/erased data, confirmation, backup, cryptographic erase and receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-033.03 — Implement/prove: exact retained/erased data**
  - Action: For factory/user reset, implement or reuse and verify this exact obligation: exact retained/erased data. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact retained/erased data; retain observable state/resource expectations.
- [ ] **F-ST-033.04 — Implement/prove: confirmation**
  - Action: For factory/user reset, implement or reuse and verify this exact obligation: confirmation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for confirmation; retain observable state/resource expectations.
- [ ] **F-ST-033.05 — Implement/prove: backup**
  - Action: For factory/user reset, implement or reuse and verify this exact obligation: backup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for backup; retain observable state/resource expectations.
- [ ] **F-ST-033.06 — Implement/prove: cryptographic erase and receipts**
  - Action: For factory/user reset, implement or reuse and verify this exact obligation: cryptographic erase and receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-033.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cryptographic erase and receipts; retain observable state/resource expectations.
- [ ] **F-ST-033.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to factory/user reset: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-033.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-033.08 — Integrate into the real consumer and runtime route**
  - Action: Wire factory/user reset into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-033.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-033.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for factory/user reset as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-033.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-034"></a>
## F-ST-034 — support bundle

**Original requirement:** user-reviewed redacted logs/config/hardware/crashes/artifact identities

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-034.01 — Reconcile existing support bundle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for support bundle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: user-reviewed redacted logs/config/hardware/crashes/artifact identities
- [ ] **F-ST-034.02 — Freeze the exact contract for support bundle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: user-reviewed redacted logs/config/hardware/crashes/artifact identities. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-034.03 — Implement/prove: user-reviewed redacted logs/config/hardware/crashes/artifact identities**
  - Action: For support bundle, implement or reuse and verify this exact obligation: user-reviewed redacted logs/config/hardware/crashes/artifact identities. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user-reviewed redacted logs/config/hardware/crashes/artifact identities; retain observable state/resource expectations.
- [ ] **F-ST-034.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to support bundle: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-034.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-034.05 — Integrate into the real consumer and runtime route**
  - Action: Wire support bundle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-034.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-034.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for support bundle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-034.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-035"></a>
## F-ST-035 — About/product identity

**Original requirement:** immutable version/build/origin/license/update/evidence, not hard-coded fake facts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-035.01 — Reconcile existing About/product identity**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for About/product identity. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: immutable version/build/origin/license/update/evidence, not hard-coded fake facts
- [ ] **F-ST-035.02 — Freeze the exact contract for About/product identity**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: immutable version/build/origin/license/update/evidence, not hard-coded fake facts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-035.03 — Implement/prove: immutable version/build/origin/license/update/evidence**
  - Action: For About/product identity, implement or reuse and verify this exact obligation: immutable version/build/origin/license/update/evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for immutable version/build/origin/license/update/evidence; retain observable state/resource expectations.
- [ ] **F-ST-035.04 — Implement/prove: not hard-coded fake facts**
  - Action: For About/product identity, implement or reuse and verify this exact obligation: not hard-coded fake facts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-035.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for not hard-coded fake facts; retain observable state/resource expectations.
- [ ] **F-ST-035.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to About/product identity: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-035.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-035.06 — Integrate into the real consumer and runtime route**
  - Action: Wire About/product identity into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-035.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-035.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for About/product identity as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-035.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-036"></a>
## F-ST-036 — security center

**Original requirement:** update/boot/firewall/permissions/keys/events/risk status with actionable evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-036.01 — Reconcile existing security center**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for security center. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: update/boot/firewall/permissions/keys/events/risk status with actionable evidence
- [ ] **F-ST-036.02 — Freeze the exact contract for security center**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: update/boot/firewall/permissions/keys/events/risk status with actionable evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-036.03 — Implement/prove: update/boot/firewall/permissions/keys/events/risk status with actionable evidence**
  - Action: For security center, implement or reuse and verify this exact obligation: update/boot/firewall/permissions/keys/events/risk status with actionable evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for update/boot/firewall/permissions/keys/events/risk status with actionable evidence; retain observable state/resource expectations.
- [ ] **F-ST-036.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to security center: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-036.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-036.05 — Integrate into the real consumer and runtime route**
  - Action: Wire security center into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-036.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-036.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for security center as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-036.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-st-037"></a>
## F-ST-037 — active sessions

**Original requirement:** local/remote/login identity, devices, activity, lock/terminate and audit

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ST-037.01 — Reconcile existing active sessions**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for active sessions. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-and-evidence comparison against the complete requirement: local/remote/login identity, devices, activity, lock/terminate and audit
- [ ] **F-ST-037.02 — Freeze the exact contract for active sessions**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: local/remote/login identity, devices, activity, lock/terminate and audit. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ST-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ST-037.03 — Implement/prove: local/remote/login identity**
  - Action: For active sessions, implement or reuse and verify this exact obligation: local/remote/login identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for local/remote/login identity; retain observable state/resource expectations.
- [ ] **F-ST-037.04 — Implement/prove: devices**
  - Action: For active sessions, implement or reuse and verify this exact obligation: devices. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-037.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for devices; retain observable state/resource expectations.
- [ ] **F-ST-037.05 — Implement/prove: activity**
  - Action: For active sessions, implement or reuse and verify this exact obligation: activity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-037.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for activity; retain observable state/resource expectations.
- [ ] **F-ST-037.06 — Implement/prove: lock/terminate and audit**
  - Action: For active sessions, implement or reuse and verify this exact obligation: lock/terminate and audit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ST-037.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lock/terminate and audit; retain observable state/resource expectations.
- [ ] **F-ST-037.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to active sessions: stale health data; denied admin action; unavailable provider; inaccurate success message; partial configuration commit; target mismatch; failed recovery; secret-bearing support output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ST-037.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ST-037.08 — Integrate into the real consumer and runtime route**
  - Action: Wire active sessions into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ST-037.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ST-037.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for active sessions as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ST-037.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p8-5"></a>
## C-P8.5 — system driver diagnostics

**Original requirement:** system driver diagnostics

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 465.

### Preserved original contract

- **Dependencies/current/provenance:** P5 Logger/System Monitor and P8 providers; Brook hazard ledgers/snarkOS telemetry; reject last-error hidden in debug console.
- **I/O and state:** bounded provider counters/events in; capability-filtered health views and ZLLOG summaries out.
- **Invariants/failure:** no secret/payload logging; counter overflow defined; reads cannot mutate driver; degraded/offline visible.
- **Deterministic proof:** counter/event schema, overflow/reset, permission, provider disappearance, decoder compatibility.
- **Target proof:** injected QEMU and physical failures appear in System Monitor and ZLLOG with matching IDs.
- **Receipt/removal:** schema/event/diagnosis match; old debug output remains diagnostic until consumers migrate.

### Execution steps

- [ ] **C-P8.5.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P8.5.02 — Resolve this contract's exact dependencies**
  - Action: P5 Logger/System Monitor and P8 providers; Brook hazard ledgers/snarkOS telemetry; reject last-error hidden in debug console. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P8.5.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P8.5.03 — I/O and state — system driver diagnostics**
  - Action: bounded provider counters/events in; capability-filtered health views and ZLLOG summaries out.
  - Requires: C-P8.5.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P8.5.
- [ ] **C-P8.5.04 — Invariants/failure — system driver diagnostics**
  - Action: no secret/payload logging; counter overflow defined; reads cannot mutate driver; degraded/offline visible.
  - Requires: C-P8.5.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P8.5.
- [ ] **C-P8.5.05 — Deterministic proof — system driver diagnostics**
  - Action: counter/event schema, overflow/reset, permission, provider disappearance, decoder compatibility.
  - Requires: C-P8.5.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P8.5.
- [ ] **C-P8.5.06 — Target proof — system driver diagnostics**
  - Action: injected QEMU and physical failures appear in System Monitor and ZLLOG with matching IDs.
  - Requires: C-P8.5.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P8.5.
- [ ] **C-P8.5.07 — Receipt/removal — system driver diagnostics**
  - Action: schema/event/diagnosis match; old debug output remains diagnostic until consumers migrate.
  - Requires: C-P8.5.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P8.5.
- [ ] **C-P8.5.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P8.5. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P8.5.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-31o"></a>
## C-DA-31O — office document creation suite

**Original requirement:** office document creation suite

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 765.

### Preserved original contract

**Depends on:** DA-20C, DA-21/21F, DA-23L, DA-25 and restricted parser workers.

**Apps:** Writer/document editor, Spreadsheet and Presenter.

**Deliver:** shared versioned document object models, undo/redo, autosave/recovery,
templates/styles, formulas/layout/slides, import/export workers, accessibility
semantics and print/export portal integration.

**Invariants:** hostile imported formats execute no code and have byte/time/memory/
output bounds; save is transactional; autosave never overwrites the last recoverable
version; formulas/layout cannot escape document authority; embedded media are
separate restricted objects.

**Proof:** corrupt/oversized import, formula cycles and resource exhaustion,
layout/font/image parser crash, disk full, crash during save/autosave, schema
migration/rollback, undo/redo persistence, keyboard/screen-reader operation and
round-trip fixtures with explicit fidelity limits.

### Execution steps

- [ ] **C-DA-31O.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-31O.02 — Resolve this contract's exact dependencies**
  - Action: DA-20C, DA-21/21F, DA-23L, DA-25 and restricted parser workers. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-31O.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-31O.03 — Apps — office document creation suite**
  - Action: Writer/document editor, Spreadsheet and Presenter.
  - Requires: C-DA-31O.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for DA-31O.
- [ ] **C-DA-31O.04 — Deliver — office document creation suite**
  - Action: shared versioned document object models, undo/redo, autosave/recovery, templates/styles, formulas/layout/slides, import/export workers, accessibility semantics and print/export portal integration.
  - Requires: C-DA-31O.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-31O.
- [ ] **C-DA-31O.05 — Invariants — office document creation suite**
  - Action: hostile imported formats execute no code and have byte/time/memory/ output bounds; save is transactional; autosave never overwrites the last recoverable version; formulas/layout cannot escape document authority; embedded media are separate restricted objects.
  - Requires: C-DA-31O.04.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-31O.
- [ ] **C-DA-31O.06 — Proof — office document creation suite**
  - Action: corrupt/oversized import, formula cycles and resource exhaustion, layout/font/image parser crash, disk full, crash during save/autosave, schema migration/rollback, undo/redo persistence, keyboard/screen-reader operation and round-trip fixtures with explicit fidelity limits.
  - Requires: C-DA-31O.05.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-31O.
- [ ] **C-DA-31O.07 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-31O. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-31O.06.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-33"></a>
## C-DA-33 — media applications

**Original requirement:** media applications

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 797.

### Preserved original contract

**Apps:** mixer, audio player/library, recorder, video player and camera after
providers exist.

**Deliver:** media library/index, playback queue, visible recording state and
restricted codecs. Device ownership remains with AudioServer/camera provider.

**Proof:** corrupt/hostile formats, seek/end-of-stream, underrun, route change,
library rescan, revoked file/microphone/camera access, decoder crash and exact UI
indication of capture.

### Execution steps

- [ ] **C-DA-33.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-33.02 — Resolve this contract's exact dependencies**
  - Action: Inherited phase and source-document dependency rules; inspect the complete source contract below. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-33.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-33.03 — Apps — media applications**
  - Action: mixer, audio player/library, recorder, video player and camera after providers exist.
  - Requires: C-DA-33.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for DA-33.
- [ ] **C-DA-33.04 — Deliver — media applications**
  - Action: media library/index, playback queue, visible recording state and restricted codecs. Device ownership remains with AudioServer/camera provider.
  - Requires: C-DA-33.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-33.
- [ ] **C-DA-33.05 — Proof — media applications**
  - Action: corrupt/hostile formats, seek/end-of-stream, underrun, route change, library rescan, revoked file/microphone/camera access, decoder crash and exact UI indication of capture.
  - Requires: C-DA-33.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-33.
- [ ] **C-DA-33.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-33. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-33.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-33p"></a>
## C-DA-33P — print, spool and scan services/apps

**Original requirement:** print, spool and scan services/apps

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 809.

### Preserved original contract

**Depends on:** DA-07/12/16 device contracts, DA-21F, DA-23L and session consent.

**Deliver:** printer/scanner providers; per-user Print Service queue and spool;
typed job/cancel/retry/status; bounded format-conversion worker; Scan Service with
page/image limits, destination handles and visible capture permission/indicator;
printer/scanner settings and job UI.

**Invariants:** apps never receive raw USB/device authority; spool data is private,
bounded and recoverable; cancellation and device/service death settle each job
once; completed/cancelled secret documents follow explicit retention/deletion;
capture cannot begin without current consent and visible indication.

**Proof:** unsupported printer/scanner, malformed descriptor/job/document, spool
disk full, paper/ink/device error, unplug mid-job/page, cancel/retry race, service
crash/restart, cross-user spool access, stale job handle, denied capture, oversized
scan and restricted converter crash.

### Execution steps

- [ ] **C-DA-33P.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-33P.02 — Resolve this contract's exact dependencies**
  - Action: DA-07/12/16 device contracts, DA-21F, DA-23L and session consent. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-33P.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-33P.03 — Deliver — print, spool and scan services/apps**
  - Action: printer/scanner providers; per-user Print Service queue and spool; typed job/cancel/retry/status; bounded format-conversion worker; Scan Service with page/image limits, destination handles and visible capture permission/indicator; printer/scanner settings and job UI.
  - Requires: C-DA-33P.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-33P.
- [ ] **C-DA-33P.04 — Invariants — print, spool and scan services/apps**
  - Action: apps never receive raw USB/device authority; spool data is private, bounded and recoverable; cancellation and device/service death settle each job once; completed/cancelled secret documents follow explicit retention/deletion; capture cannot begin without current consent and visible indication.
  - Requires: C-DA-33P.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-33P.
- [ ] **C-DA-33P.05 — Proof — print, spool and scan services/apps**
  - Action: unsupported printer/scanner, malformed descriptor/job/document, spool disk full, paper/ink/device error, unplug mid-job/page, cancel/retry race, service crash/restart, cross-user spool access, stale job handle, denied capture, oversized scan and restricted converter crash.
  - Requires: C-DA-33P.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-33P.
- [ ] **C-DA-33P.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-33P. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-33P.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-34"></a>
## C-DA-34 — software centre and provenance UI

**Original requirement:** software centre and provenance UI

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 828.

### Preserved original contract

**Depends on:** DA-24.

**Deliver:** package discovery, installed/update state, permissions, publisher/key,
artifact/proof receipts, migration/rollback and recovery UI.

**Proof:** offline/stale metadata, compromised key/revocation, partial download,
disk full, rollback, live app update, incompatible ABI and honest display of source-
only versus verified features.

### Execution steps

- [ ] **C-DA-34.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-34.02 — Resolve this contract's exact dependencies**
  - Action: DA-24. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-34.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-34.03 — Deliver — software centre and provenance UI**
  - Action: package discovery, installed/update state, permissions, publisher/key, artifact/proof receipts, migration/rollback and recovery UI.
  - Requires: C-DA-34.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-34.
- [ ] **C-DA-34.04 — Proof — software centre and provenance UI**
  - Action: offline/stale metadata, compromised key/revocation, partial download, disk full, rollback, live app update, incompatible ABI and honest display of source- only versus verified features.
  - Requires: C-DA-34.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-34.
- [ ] **C-DA-34.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-34. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-34.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-34"></a>
## C-VX-34 — Welcome, account and onboarding

**Original requirement:** Welcome, account and onboarding

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 584.

### Preserved original contract

**Depends on:** VX-13, VX-16, account/locale/network/accessibility services.

**Deliver:** resumable setup for account, locale, keyboard, accessibility,
network, privacy and recovery; safe skip and later re-entry.

**Proof:** offline setup, failed persistence, reboot every step, assistive-only
journey, existing account and migration.

### Execution steps

- [ ] **C-VX-34.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-34.02 — Resolve this contract's exact dependencies**
  - Action: VX-13, VX-16, account/locale/network/accessibility services. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-34.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-34.03 — Deliver — Welcome, account and onboarding**
  - Action: resumable setup for account, locale, keyboard, accessibility, network, privacy and recovery; safe skip and later re-entry.
  - Requires: C-VX-34.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-34.
- [ ] **C-VX-34.04 — Proof — Welcome, account and onboarding**
  - Action: offline setup, failed persistence, reboot every step, assistive-only journey, existing account and migration.
  - Requires: C-VX-34.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-34.
- [ ] **C-VX-34.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-34. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-34.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-35"></a>
## C-VX-35 — Device and permission manager

**Original requirement:** Device and permission manager

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 594.

### Preserved original contract

**Depends on:** VX-20, device/session authority.

**Deliver:** device/provider identity, health, lifecycle, driver/provenance,
permissions, recovery and removable media.

**Proof:** provider crash/rebind, device removal, stale generation, ordinary-user
MMIO/power/mount denial and recovery receipts.

### Execution steps

- [ ] **C-VX-35.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-35.02 — Resolve this contract's exact dependencies**
  - Action: VX-20, device/session authority. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-35.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-35.03 — Deliver — Device and permission manager**
  - Action: device/provider identity, health, lifecycle, driver/provenance, permissions, recovery and removable media.
  - Requires: C-VX-35.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-35.
- [ ] **C-VX-35.04 — Proof — Device and permission manager**
  - Action: provider crash/rebind, device removal, stale generation, ordinary-user MMIO/power/mount denial and recovery receipts.
  - Requires: C-VX-35.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-35.
- [ ] **C-VX-35.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-35. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-35.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-36"></a>
## C-VX-36 — Software, update and rollback UI

**Original requirement:** Software, update and rollback UI

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 604.

### Preserved original contract

**Depends on:** VX-17.

**Deliver:** signed catalogue, provenance, permission/version diff, progress,
cancel, update/rollback/uninstall and restart state.

**Proof:** signature/key revoke, dependency conflict, network/disk failure,
reboot mid-update, live app, rollback and exact package object census.

### Execution steps

- [ ] **C-VX-36.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-36.02 — Resolve this contract's exact dependencies**
  - Action: VX-17. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-36.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-36.03 — Deliver — Software, update and rollback UI**
  - Action: signed catalogue, provenance, permission/version diff, progress, cancel, update/rollback/uninstall and restart state.
  - Requires: C-VX-36.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-36.
- [ ] **C-VX-36.04 — Proof — Software, update and rollback UI**
  - Action: signature/key revoke, dependency conflict, network/disk failure, reboot mid-update, live app, rollback and exact package object census.
  - Requires: C-VX-36.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-36.
- [ ] **C-VX-36.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-36. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-36.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-37"></a>
## C-VX-37 — Crash and recovery center

**Original requirement:** Crash and recovery center

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 614.

### Preserved original contract

**Depends on:** VX-18, logger/crash service.

**Deliver:** source-attributed failures, redacted detail, restart/restore, safe
mode, quarantine, support bundle and state-recovery choice.

**Proof:** spoof/redaction, crash loop, incompatible snapshot, logger loss,
support-bundle permission and desktop continuity.

### Execution steps

- [ ] **C-VX-37.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-37.02 — Resolve this contract's exact dependencies**
  - Action: VX-18, logger/crash service. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-37.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-37.03 — Deliver — Crash and recovery center**
  - Action: source-attributed failures, redacted detail, restart/restore, safe mode, quarantine, support bundle and state-recovery choice.
  - Requires: C-VX-37.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-37.
- [ ] **C-VX-37.04 — Proof — Crash and recovery center**
  - Action: spoof/redaction, crash loop, incompatible snapshot, logger loss, support-bundle permission and desktop continuity.
  - Requires: C-VX-37.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-37.
- [ ] **C-VX-37.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-37. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-37.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-38"></a>
## C-VX-38 — Document and PDF family

**Original requirement:** Document and PDF family

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 624.

### Preserved original contract

**Depends on:** VX-21, VX-30.

**Deliver:** document reader/editor and isolated PDF/ebook reader with autosave,
recovery, search, annotations, print/export and accessibility.

**Proof:** hostile document/PDF, huge pages, missing fonts, disk full, decoder
crash, recovery, semantic reading order and print cancellation.

### Execution steps

- [ ] **C-VX-38.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-38.02 — Resolve this contract's exact dependencies**
  - Action: VX-21, VX-30. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-38.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-38.03 — Deliver — Document and PDF family**
  - Action: document reader/editor and isolated PDF/ebook reader with autosave, recovery, search, annotations, print/export and accessibility.
  - Requires: C-VX-38.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-38.
- [ ] **C-VX-38.04 — Proof — Document and PDF family**
  - Action: hostile document/PDF, huge pages, missing fonts, disk full, decoder crash, recovery, semantic reading order and print cancellation.
  - Requires: C-VX-38.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-38.
- [ ] **C-VX-38.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-38. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-38.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-39"></a>
## C-VX-39 — Spreadsheet

**Original requirement:** Spreadsheet

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 634.

### Preserved original contract

**Depends on:** VX-21.

**Deliver:** cells/formulas/dependency graph, deterministic recalculation,
import/export, charts, undo/autosave/recovery and accessibility grid.

**Proof:** cycles, precision/errors, huge sparse sheet, formula bombs, hostile
import, crash during recalc/save and semantic cell navigation.

### Execution steps

- [ ] **C-VX-39.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-39.02 — Resolve this contract's exact dependencies**
  - Action: VX-21. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-39.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-39.03 — Deliver — Spreadsheet**
  - Action: cells/formulas/dependency graph, deterministic recalculation, import/export, charts, undo/autosave/recovery and accessibility grid.
  - Requires: C-VX-39.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-39.
- [ ] **C-VX-39.04 — Proof — Spreadsheet**
  - Action: cycles, precision/errors, huge sparse sheet, formula bombs, hostile import, crash during recalc/save and semantic cell navigation.
  - Requires: C-VX-39.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-39.
- [ ] **C-VX-39.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-39. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-39.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-40"></a>
## C-VX-40 — Presentation

**Original requirement:** Presentation

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 644.

### Preserved original contract

**Depends on:** VX-21, VX-24.

**Deliver:** slide editor/player, assets, templates, notes, speaker/display mode,
export and recovery.

**Proof:** external-display loss, hostile assets, missing font, full-screen
authority, autosave crash, reduced motion and keyboard-only delivery.

### Execution steps

- [ ] **C-VX-40.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-40.02 — Resolve this contract's exact dependencies**
  - Action: VX-21, VX-24. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-40.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-40.03 — Deliver — Presentation**
  - Action: slide editor/player, assets, templates, notes, speaker/display mode, export and recovery.
  - Requires: C-VX-40.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-40.
- [ ] **C-VX-40.04 — Proof — Presentation**
  - Action: external-display loss, hostile assets, missing font, full-screen authority, autosave crash, reduced motion and keyboard-only delivery.
  - Requires: C-VX-40.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-40.
- [ ] **C-VX-40.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-40. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-40.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-41"></a>
## C-VX-41 — Print, spool and scan

**Original requirement:** Print, spool and scan

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 654.

### Preserved original contract

**Depends on:** VX-16, provider/service contracts.

**Deliver:** printer/scanner discovery, page setup/preview, owned spool jobs,
cancel/retry, progress, scan preview/crop/save and privacy indicators.

**Proof:** device removal, jam/error, wrong owner cancel, huge job, spool restart,
partial scan, capture denial and retained/removed data policy.

### Execution steps

- [ ] **C-VX-41.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-41.02 — Resolve this contract's exact dependencies**
  - Action: VX-16, provider/service contracts. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-41.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-41.03 — Deliver — Print, spool and scan**
  - Action: printer/scanner discovery, page setup/preview, owned spool jobs, cancel/retry, progress, scan preview/crop/save and privacy indicators.
  - Requires: C-VX-41.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-41.
- [ ] **C-VX-41.04 — Proof — Print, spool and scan**
  - Action: device removal, jam/error, wrong owner cancel, huge job, spool restart, partial scan, capture denial and retained/removed data policy.
  - Requires: C-VX-41.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-41.
- [ ] **C-VX-41.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-41. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-41.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-42"></a>
## C-VX-42 — Communications and personal information

**Original requirement:** Communications and personal information

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 664.

### Preserved original contract

**Depends on:** Browser/Network, Secrets, Notification, Locale services.

**Deliver:** account broker and Mail/Calendar/Contacts/Messaging clients with
offline queue, sync conflicts, attachments and notification privacy.

**Proof:** credential revoke, hostile message/attachment, clock/timezone,
duplicate/reordered sync, offline edit, conflict and account removal cleanup.

### Execution steps

- [ ] **C-VX-42.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-42.02 — Resolve this contract's exact dependencies**
  - Action: Browser/Network, Secrets, Notification, Locale services. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-42.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-42.03 — Deliver — Communications and personal information**
  - Action: account broker and Mail/Calendar/Contacts/Messaging clients with offline queue, sync conflicts, attachments and notification privacy.
  - Requires: C-VX-42.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-42.
- [ ] **C-VX-42.04 — Proof — Communications and personal information**
  - Action: credential revoke, hostile message/attachment, clock/timezone, duplicate/reordered sync, offline edit, conflict and account removal cleanup.
  - Requires: C-VX-42.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-42.
- [ ] **C-VX-42.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-42. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-42.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-43"></a>
## C-VX-43 — Media, audio and creation

**Original requirement:** Media, audio and creation

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 674.

### Preserved original contract

**Depends on:** AudioServer, VX-30, VX-24.

**Deliver:** library/player, recorder, mixer, synth/sequencer and richer image
creation with bounded streams, projects and permission state.

**Proof:** codec crash, xrun/device loss, sample-rate negotiation, saturation
goldens, microphone revoke, save/recovery and background behavior.

### Execution steps

- [ ] **C-VX-43.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-43.02 — Resolve this contract's exact dependencies**
  - Action: AudioServer, VX-30, VX-24. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-43.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-43.03 — Deliver — Media, audio and creation**
  - Action: library/player, recorder, mixer, synth/sequencer and richer image creation with bounded streams, projects and permission state.
  - Requires: C-VX-43.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-43.
- [ ] **C-VX-43.04 — Proof — Media, audio and creation**
  - Action: codec crash, xrun/device loss, sample-rate negotiation, saturation goldens, microphone revoke, save/recovery and background behavior.
  - Requires: C-VX-43.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-43.
- [ ] **C-VX-43.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-43. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-43.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-44"></a>
## C-VX-44 — Camera and capture

**Original requirement:** Camera and capture

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 684.

### Preserved original contract

**Depends on:** VX-16, media/device services.

**Deliver:** camera preview/capture and screen/window/region recording portal,
visible ownership indicator, protected-surface policy and output workflow.

**Proof:** lock/background denial, indicator cannot be hidden, device removal,
quota/disk full, protected content, app death and revocation.

### Execution steps

- [ ] **C-VX-44.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-44.02 — Resolve this contract's exact dependencies**
  - Action: VX-16, media/device services. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-44.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-44.03 — Deliver — Camera and capture**
  - Action: camera preview/capture and screen/window/region recording portal, visible ownership indicator, protected-surface policy and output workflow.
  - Requires: C-VX-44.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-44.
- [ ] **C-VX-44.04 — Proof — Camera and capture**
  - Action: lock/background denial, indicator cannot be hidden, device removal, quota/disk full, protected content, app death and revocation.
  - Requires: C-VX-44.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-44.
- [ ] **C-VX-44.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-44. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-44.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-45"></a>
## C-VX-45 — Development workbench

**Original requirement:** Development workbench

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 696.

### Preserved original contract

**Depends on:** VX-21, VX-22, compiler/package/debug services.

**Deliver:** source editor, build graph, diagnostics, debugger, profiler, trace,
package and provenance views as ordinary apps.

**Proof:** hostile project, build cancel, compiler crash, debug authority, huge
trace, source recovery and reproducible artifact receipt.

### Execution steps

- [ ] **C-VX-45.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-45.02 — Resolve this contract's exact dependencies**
  - Action: VX-21, VX-22, compiler/package/debug services. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-45.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-45.03 — Deliver — Development workbench**
  - Action: source editor, build graph, diagnostics, debugger, profiler, trace, package and provenance views as ordinary apps.
  - Requires: C-VX-45.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-45.
- [ ] **C-VX-45.04 — Proof — Development workbench**
  - Action: hostile project, build cancel, compiler crash, debug authority, huge trace, source recovery and reproducible artifact receipt.
  - Requires: C-VX-45.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-45.
- [ ] **C-VX-45.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-45. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-45.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-usb-013"></a>
## T-USB-013 — USB printer class

**Original requirement:** job/status/cancel and privacy

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 148.

### Execution steps

- [ ] **T-USB-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB printer class to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-013.02 — Specify the complete target boundary**
  - Action: USB printer class must supply: job/status/cancel and privacy. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB printer class through the shared platform contract, delivering every part of: job/status/cancel and privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-013.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB printer class.
- [ ] **T-USB-013.05 — Qualify and retain this target's own result**
  - Action: Bind USB printer class to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-periph-001"></a>
## T-PERIPH-001 — printer provider

**Original requirement:** capabilities/status/job/cancel

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 249.

### Execution steps

- [ ] **T-PERIPH-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve printer provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PERIPH-001.02 — Specify the complete target boundary**
  - Action: printer provider must supply: capabilities/status/job/cancel. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PERIPH-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PERIPH-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse printer provider through the shared platform contract, delivering every part of: capabilities/status/job/cancel. Do not fork a duplicate subsystem for this row.
  - Requires: T-PERIPH-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PERIPH-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PERIPH-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for printer provider.
- [ ] **T-PERIPH-001.05 — Qualify and retain this target's own result**
  - Action: Bind printer provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PERIPH-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-periph-002"></a>
## T-PERIPH-002 — scanner provider

**Original requirement:** capabilities/preview/capture/privacy

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 250.

### Execution steps

- [ ] **T-PERIPH-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve scanner provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PERIPH-002.02 — Specify the complete target boundary**
  - Action: scanner provider must supply: capabilities/preview/capture/privacy. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PERIPH-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PERIPH-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse scanner provider through the shared platform contract, delivering every part of: capabilities/preview/capture/privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-PERIPH-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PERIPH-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PERIPH-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for scanner provider.
- [ ] **T-PERIPH-002.05 — Qualify and retain this target's own result**
  - Action: Bind scanner provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PERIPH-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-094"></a>
## T-SVC-094 — Media Library/Metadata Service

**Original requirement:** index, tags, playlists, thumbnails and privacy

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 116.

### Execution steps

- [ ] **T-SVC-094.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Media Library/Metadata Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-094.02 — Specify the complete target boundary**
  - Action: Media Library/Metadata Service must supply: index, tags, playlists, thumbnails and privacy. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-094.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-094.03 — Implement the exact target behavior**
  - Action: Implement or reuse Media Library/Metadata Service through the shared platform contract, delivering every part of: index, tags, playlists, thumbnails and privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-094.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-094.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-094.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Media Library/Metadata Service.
- [ ] **T-SVC-094.05 — Qualify and retain this target's own result**
  - Action: Bind Media Library/Metadata Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-094.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-097"></a>
## T-SVC-097 — Communication Notification Broker

**Original requirement:** message/call events, privacy and quiet modes

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 119.

### Execution steps

- [ ] **T-SVC-097.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Communication Notification Broker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-097.02 — Specify the complete target boundary**
  - Action: Communication Notification Broker must supply: message/call events, privacy and quiet modes. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-097.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-097.03 — Implement the exact target behavior**
  - Action: Implement or reuse Communication Notification Broker through the shared platform contract, delivering every part of: message/call events, privacy and quiet modes. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-097.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-097.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-097.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Communication Notification Broker.
- [ ] **T-SVC-097.05 — Qualify and retain this target's own result**
  - Action: Bind Communication Notification Broker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-097.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-098"></a>
## T-SVC-098 — Realtime Call/Conference Service

**Original requirement:** media negotiation, permissions, network changes and teardown

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 120.

### Execution steps

- [ ] **T-SVC-098.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Realtime Call/Conference Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-098.02 — Specify the complete target boundary**
  - Action: Realtime Call/Conference Service must supply: media negotiation, permissions, network changes and teardown. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-098.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-098.03 — Implement the exact target behavior**
  - Action: Implement or reuse Realtime Call/Conference Service through the shared platform contract, delivering every part of: media negotiation, permissions, network changes and teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-098.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-098.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-098.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Realtime Call/Conference Service.
- [ ] **T-SVC-098.05 — Qualify and retain this target's own result**
  - Action: Bind Realtime Call/Conference Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-098.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-104"></a>
## T-SVC-104 — Print Spooler

**Original requirement:** queue, render, status, cancel, privacy and retry

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 130.

### Execution steps

- [ ] **T-SVC-104.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Print Spooler to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-104.02 — Specify the complete target boundary**
  - Action: Print Spooler must supply: queue, render, status, cancel, privacy and retry. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-104.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-104.03 — Implement the exact target behavior**
  - Action: Implement or reuse Print Spooler through the shared platform contract, delivering every part of: queue, render, status, cancel, privacy and retry. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-104.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-104.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-104.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Print Spooler.
- [ ] **T-SVC-104.05 — Qualify and retain this target's own result**
  - Action: Bind Print Spooler to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-104.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-105"></a>
## T-SVC-105 — Scan Service

**Original requirement:** provider selection, preview, capture and privacy

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 131.

### Execution steps

- [ ] **T-SVC-105.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Scan Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-105.02 — Specify the complete target boundary**
  - Action: Scan Service must supply: provider selection, preview, capture and privacy. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-105.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-105.03 — Implement the exact target behavior**
  - Action: Implement or reuse Scan Service through the shared platform contract, delivering every part of: provider selection, preview, capture and privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-105.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-105.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-105.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Scan Service.
- [ ] **T-SVC-105.05 — Qualify and retain this target's own result**
  - Action: Bind Scan Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-105.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-108"></a>
## T-SVC-108 — Spellcheck/Dictionary Service

**Original requirement:** language packs, privacy and custom dictionaries

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 134.

### Execution steps

- [ ] **T-SVC-108.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Spellcheck/Dictionary Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-108.02 — Specify the complete target boundary**
  - Action: Spellcheck/Dictionary Service must supply: language packs, privacy and custom dictionaries. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-108.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-108.03 — Implement the exact target behavior**
  - Action: Implement or reuse Spellcheck/Dictionary Service through the shared platform contract, delivering every part of: language packs, privacy and custom dictionaries. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-108.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-108.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-108.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Spellcheck/Dictionary Service.
- [ ] **T-SVC-108.05 — Qualify and retain this target's own result**
  - Action: Bind Spellcheck/Dictionary Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-108.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-109"></a>
## T-SVC-109 — Calendar/Alarm Service

**Original requirement:** wall/deadline separation, recurrence and wake policy

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 135.

### Execution steps

- [ ] **T-SVC-109.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Calendar/Alarm Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-109.02 — Specify the complete target boundary**
  - Action: Calendar/Alarm Service must supply: wall/deadline separation, recurrence and wake policy. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-109.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-109.03 — Implement the exact target behavior**
  - Action: Implement or reuse Calendar/Alarm Service through the shared platform contract, delivering every part of: wall/deadline separation, recurrence and wake policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-109.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-109.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-109.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Calendar/Alarm Service.
- [ ] **T-SVC-109.05 — Qualify and retain this target's own result**
  - Action: Bind Calendar/Alarm Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-109.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-110"></a>
## T-SVC-110 — Contacts Service

**Original requirement:** per-user data, grants, import/export and sync conflicts

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 136.

### Execution steps

- [ ] **T-SVC-110.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Contacts Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-110.02 — Specify the complete target boundary**
  - Action: Contacts Service must supply: per-user data, grants, import/export and sync conflicts. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-110.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-110.03 — Implement the exact target behavior**
  - Action: Implement or reuse Contacts Service through the shared platform contract, delivering every part of: per-user data, grants, import/export and sync conflicts. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-110.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-110.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-110.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Contacts Service.
- [ ] **T-SVC-110.05 — Qualify and retain this target's own result**
  - Action: Bind Contacts Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-110.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-111"></a>
## T-SVC-111 — Mail Service

**Original requirement:** accounts, transport, local store, search and security

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 137.

### Execution steps

- [ ] **T-SVC-111.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Mail Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-111.02 — Specify the complete target boundary**
  - Action: Mail Service must supply: accounts, transport, local store, search and security. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-111.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-111.03 — Implement the exact target behavior**
  - Action: Implement or reuse Mail Service through the shared platform contract, delivering every part of: accounts, transport, local store, search and security. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-111.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-111.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-111.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Mail Service.
- [ ] **T-SVC-111.05 — Qualify and retain this target's own result**
  - Action: Bind Mail Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-111.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-112"></a>
## T-SVC-112 — Messaging/Presence Service

**Original requirement:** accounts, delivery state, encryption policy and offline queues

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 138.

### Execution steps

- [ ] **T-SVC-112.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Messaging/Presence Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-112.02 — Specify the complete target boundary**
  - Action: Messaging/Presence Service must supply: accounts, delivery state, encryption policy and offline queues. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-112.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-112.03 — Implement the exact target behavior**
  - Action: Implement or reuse Messaging/Presence Service through the shared platform contract, delivering every part of: accounts, delivery state, encryption policy and offline queues. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-112.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-112.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-112.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Messaging/Presence Service.
- [ ] **T-SVC-112.05 — Qualify and retain this target's own result**
  - Action: Bind Messaging/Presence Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-112.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-001"></a>
## T-APP-001 — Welcome/Onboarding

**Original requirement:** resumable account, locale, input, a11y, network, privacy and recovery setup

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 128.

### Execution steps

- [ ] **T-APP-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Welcome/Onboarding to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-001.02 — Specify the complete target boundary**
  - Action: Welcome/Onboarding must supply: resumable account, locale, input, a11y, network, privacy and recovery setup. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse Welcome/Onboarding through the shared platform contract, delivering every part of: resumable account, locale, input, a11y, network, privacy and recovery setup. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-001.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Welcome/Onboarding.
- [ ] **T-APP-001.05 — Qualify and retain this target's own result**
  - Action: Bind Welcome/Onboarding to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-004"></a>
## T-APP-004 — Notification Center

**Original requirement:** history, grouping, actions, source controls and quiet mode

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 131.

### Execution steps

- [ ] **T-APP-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Notification Center to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-004.02 — Specify the complete target boundary**
  - Action: Notification Center must supply: history, grouping, actions, source controls and quiet mode. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse Notification Center through the shared platform contract, delivering every part of: history, grouping, actions, source controls and quiet mode. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-004.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Notification Center.
- [ ] **T-APP-004.05 — Qualify and retain this target's own result**
  - Action: Bind Notification Center to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-005"></a>
## T-APP-005 — Background Tasks

**Original requirement:** progress, pause/cancel, budgets and source identity

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 132.

### Execution steps

- [ ] **T-APP-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Background Tasks to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-005.02 — Specify the complete target boundary**
  - Action: Background Tasks must supply: progress, pause/cancel, budgets and source identity. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse Background Tasks through the shared platform contract, delivering every part of: progress, pause/cancel, budgets and source identity. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-005.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Background Tasks.
- [ ] **T-APP-005.05 — Qualify and retain this target's own result**
  - Action: Bind Background Tasks to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-006"></a>
## T-APP-006 — Permissions/Privacy Dashboard

**Original requirement:** inspect/change live app grants and history

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 133.

### Execution steps

- [ ] **T-APP-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Permissions/Privacy Dashboard to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-006.02 — Specify the complete target boundary**
  - Action: Permissions/Privacy Dashboard must supply: inspect/change live app grants and history. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse Permissions/Privacy Dashboard through the shared platform contract, delivering every part of: inspect/change live app grants and history. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-006.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Permissions/Privacy Dashboard.
- [ ] **T-APP-006.05 — Qualify and retain this target's own result**
  - Action: Bind Permissions/Privacy Dashboard to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-007"></a>
## T-APP-007 — User and Account Manager

**Original requirement:** users, credentials, groups, sessions and recovery

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 134.

### Execution steps

- [ ] **T-APP-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve User and Account Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-007.02 — Specify the complete target boundary**
  - Action: User and Account Manager must supply: users, credentials, groups, sessions and recovery. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse User and Account Manager through the shared platform contract, delivering every part of: users, credentials, groups, sessions and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-007.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for User and Account Manager.
- [ ] **T-APP-007.05 — Qualify and retain this target's own result**
  - Action: Bind User and Account Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-010"></a>
## T-APP-010 — Device Manager

**Original requirement:** provider identity, health, authority, recovery and receipts

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 137.

### Execution steps

- [ ] **T-APP-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Device Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-010.02 — Specify the complete target boundary**
  - Action: Device Manager must supply: provider identity, health, authority, recovery and receipts. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse Device Manager through the shared platform contract, delivering every part of: provider identity, health, authority, recovery and receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-010.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Device Manager.
- [ ] **T-APP-010.05 — Qualify and retain this target's own result**
  - Action: Bind Device Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-011"></a>
## T-APP-011 — Software Center

**Original requirement:** browse/install/update/remove with provenance and permissions

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 138.

### Execution steps

- [ ] **T-APP-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Software Center to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-011.02 — Specify the complete target boundary**
  - Action: Software Center must supply: browse/install/update/remove with provenance and permissions. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse Software Center through the shared platform contract, delivering every part of: browse/install/update/remove with provenance and permissions. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-011.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Software Center.
- [ ] **T-APP-011.05 — Qualify and retain this target's own result**
  - Action: Bind Software Center to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-012"></a>
## T-APP-012 — Update and Rollback Center

**Original requirement:** generations, staged update, restart and recovery

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 139.

### Execution steps

- [ ] **T-APP-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Update and Rollback Center to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-012.02 — Specify the complete target boundary**
  - Action: Update and Rollback Center must supply: generations, staged update, restart and recovery. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse Update and Rollback Center through the shared platform contract, delivering every part of: generations, staged update, restart and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-012.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Update and Rollback Center.
- [ ] **T-APP-012.05 — Qualify and retain this target's own result**
  - Action: Bind Update and Rollback Center to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-013"></a>
## T-APP-013 — Crash/Recovery Center

**Original requirement:** failures, restore, logs, safe mode and support bundle

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 140.

### Execution steps

- [ ] **T-APP-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Crash/Recovery Center to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-013.02 — Specify the complete target boundary**
  - Action: Crash/Recovery Center must supply: failures, restore, logs, safe mode and support bundle. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse Crash/Recovery Center through the shared platform contract, delivering every part of: failures, restore, logs, safe mode and support bundle. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-013.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Crash/Recovery Center.
- [ ] **T-APP-013.05 — Qualify and retain this target's own result**
  - Action: Bind Crash/Recovery Center to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-014"></a>
## T-APP-014 — Power/Battery Center

**Original requirement:** battery/thermal/profile/suspend/shutdown state

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 141.

### Execution steps

- [ ] **T-APP-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Power/Battery Center to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-014.02 — Specify the complete target boundary**
  - Action: Power/Battery Center must supply: battery/thermal/profile/suspend/shutdown state. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse Power/Battery Center through the shared platform contract, delivering every part of: battery/thermal/profile/suspend/shutdown state. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-014.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Power/Battery Center.
- [ ] **T-APP-014.05 — Qualify and retain this target's own result**
  - Action: Bind Power/Battery Center to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-015"></a>
## T-APP-015 — Removable Media Center

**Original requirement:** identify, mount, format, eject, repair and privacy

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 142.

### Execution steps

- [ ] **T-APP-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Removable Media Center to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-015.02 — Specify the complete target boundary**
  - Action: Removable Media Center must supply: identify, mount, format, eject, repair and privacy. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse Removable Media Center through the shared platform contract, delivering every part of: identify, mount, format, eject, repair and privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-015.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Removable Media Center.
- [ ] **T-APP-015.05 — Qualify and retain this target's own result**
  - Action: Bind Removable Media Center to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-020"></a>
## T-APP-020 — Advanced File Manager

**Original requirement:** tabs, search, preview, copy/move/trash, mounts and conflicts

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 149.

### Execution steps

- [ ] **T-APP-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Advanced File Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-020.02 — Specify the complete target boundary**
  - Action: Advanced File Manager must supply: tabs, search, preview, copy/move/trash, mounts and conflicts. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse Advanced File Manager through the shared platform contract, delivering every part of: tabs, search, preview, copy/move/trash, mounts and conflicts. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-020.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Advanced File Manager.
- [ ] **T-APP-020.05 — Qualify and retain this target's own result**
  - Action: Bind Advanced File Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-021"></a>
## T-APP-021 — Document Viewer

**Original requirement:** paginated documents, navigation, search and annotations

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 150.

### Execution steps

- [ ] **T-APP-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Document Viewer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-021.02 — Specify the complete target boundary**
  - Action: Document Viewer must supply: paginated documents, navigation, search and annotations. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse Document Viewer through the shared platform contract, delivering every part of: paginated documents, navigation, search and annotations. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-021.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Document Viewer.
- [ ] **T-APP-021.05 — Qualify and retain this target's own result**
  - Action: Bind Document Viewer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-022"></a>
## T-APP-022 — PDF Viewer

**Original requirement:** isolated PDF rendering, forms/links/search/print

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 151.

### Execution steps

- [ ] **T-APP-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PDF Viewer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-022.02 — Specify the complete target boundary**
  - Action: PDF Viewer must supply: isolated PDF rendering, forms/links/search/print. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse PDF Viewer through the shared platform contract, delivering every part of: isolated PDF rendering, forms/links/search/print. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-022.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PDF Viewer.
- [ ] **T-APP-022.05 — Qualify and retain this target's own result**
  - Action: Bind PDF Viewer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-023"></a>
## T-APP-023 — Ebook Reader

**Original requirement:** library, reflow, bookmarks, annotations and accessibility

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 152.

### Execution steps

- [ ] **T-APP-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Ebook Reader to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-023.02 — Specify the complete target boundary**
  - Action: Ebook Reader must supply: library, reflow, bookmarks, annotations and accessibility. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse Ebook Reader through the shared platform contract, delivering every part of: library, reflow, bookmarks, annotations and accessibility. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-023.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Ebook Reader.
- [ ] **T-APP-023.05 — Qualify and retain this target's own result**
  - Action: Bind Ebook Reader to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-024"></a>
## T-APP-024 — Word Processor

**Original requirement:** styled documents, layout, comments, autosave and export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 153.

### Execution steps

- [ ] **T-APP-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Word Processor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-024.02 — Specify the complete target boundary**
  - Action: Word Processor must supply: styled documents, layout, comments, autosave and export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse Word Processor through the shared platform contract, delivering every part of: styled documents, layout, comments, autosave and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-024.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Word Processor.
- [ ] **T-APP-024.05 — Qualify and retain this target's own result**
  - Action: Bind Word Processor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-025"></a>
## T-APP-025 — Spreadsheet

**Original requirement:** bounded formulas, cycles, deterministic recalc, charts/import/export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 154.

### Execution steps

- [ ] **T-APP-025.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Spreadsheet to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-025.02 — Specify the complete target boundary**
  - Action: Spreadsheet must supply: bounded formulas, cycles, deterministic recalc, charts/import/export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-025.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-025.03 — Implement the exact target behavior**
  - Action: Implement or reuse Spreadsheet through the shared platform contract, delivering every part of: bounded formulas, cycles, deterministic recalc, charts/import/export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-025.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-025.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-025.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Spreadsheet.
- [ ] **T-APP-025.05 — Qualify and retain this target's own result**
  - Action: Bind Spreadsheet to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-025.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-026"></a>
## T-APP-026 — Presentation Editor/Player

**Original requirement:** slides, assets, speaker view, full-screen and export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 155.

### Execution steps

- [ ] **T-APP-026.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Presentation Editor/Player to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-026.02 — Specify the complete target boundary**
  - Action: Presentation Editor/Player must supply: slides, assets, speaker view, full-screen and export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-026.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-026.03 — Implement the exact target behavior**
  - Action: Implement or reuse Presentation Editor/Player through the shared platform contract, delivering every part of: slides, assets, speaker view, full-screen and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-026.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-026.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-026.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Presentation Editor/Player.
- [ ] **T-APP-026.05 — Qualify and retain this target's own result**
  - Action: Bind Presentation Editor/Player to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-026.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-027"></a>
## T-APP-027 — Notes/Knowledge Base

**Original requirement:** linked notes, tags, search, attachments and sync-ready conflicts

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 156.

### Execution steps

- [ ] **T-APP-027.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Notes/Knowledge Base to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-027.02 — Specify the complete target boundary**
  - Action: Notes/Knowledge Base must supply: linked notes, tags, search, attachments and sync-ready conflicts. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-027.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-027.03 — Implement the exact target behavior**
  - Action: Implement or reuse Notes/Knowledge Base through the shared platform contract, delivering every part of: linked notes, tags, search, attachments and sync-ready conflicts. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-027.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-027.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-027.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Notes/Knowledge Base.
- [ ] **T-APP-027.05 — Qualify and retain this target's own result**
  - Action: Bind Notes/Knowledge Base to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-027.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-028"></a>
## T-APP-028 — Markdown Editor/Preview

**Original requirement:** source/preview, safe links/assets and export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 157.

### Execution steps

- [ ] **T-APP-028.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Markdown Editor/Preview to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-028.02 — Specify the complete target boundary**
  - Action: Markdown Editor/Preview must supply: source/preview, safe links/assets and export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-028.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-028.03 — Implement the exact target behavior**
  - Action: Implement or reuse Markdown Editor/Preview through the shared platform contract, delivering every part of: source/preview, safe links/assets and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-028.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-028.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-028.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Markdown Editor/Preview.
- [ ] **T-APP-028.05 — Qualify and retain this target's own result**
  - Action: Bind Markdown Editor/Preview to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-028.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-029"></a>
## T-APP-029 — PDF/Document Annotator

**Original requirement:** highlights, ink, comments, signatures and non-destructive save

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 158.

### Execution steps

- [ ] **T-APP-029.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PDF/Document Annotator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-029.02 — Specify the complete target boundary**
  - Action: PDF/Document Annotator must supply: highlights, ink, comments, signatures and non-destructive save. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-029.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-029.03 — Implement the exact target behavior**
  - Action: Implement or reuse PDF/Document Annotator through the shared platform contract, delivering every part of: highlights, ink, comments, signatures and non-destructive save. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-029.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-029.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-029.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PDF/Document Annotator.
- [ ] **T-APP-029.05 — Qualify and retain this target's own result**
  - Action: Bind PDF/Document Annotator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-029.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-030"></a>
## T-APP-030 — OCR/Document Capture

**Original requirement:** scan/image import, language models, correction and export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 159.

### Execution steps

- [ ] **T-APP-030.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve OCR/Document Capture to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-030.02 — Specify the complete target boundary**
  - Action: OCR/Document Capture must supply: scan/image import, language models, correction and export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-030.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-030.03 — Implement the exact target behavior**
  - Action: Implement or reuse OCR/Document Capture through the shared platform contract, delivering every part of: scan/image import, language models, correction and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-030.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-030.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-030.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for OCR/Document Capture.
- [ ] **T-APP-030.05 — Qualify and retain this target's own result**
  - Action: Bind OCR/Document Capture to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-030.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-031"></a>
## T-APP-031 — Archive/Compression Workbench

**Original requirement:** large archive browse/create/extract with safe recovery

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 160.

### Execution steps

- [ ] **T-APP-031.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Archive/Compression Workbench to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-031.02 — Specify the complete target boundary**
  - Action: Archive/Compression Workbench must supply: large archive browse/create/extract with safe recovery. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-031.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-031.03 — Implement the exact target behavior**
  - Action: Implement or reuse Archive/Compression Workbench through the shared platform contract, delivering every part of: large archive browse/create/extract with safe recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-031.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-031.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-031.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Archive/Compression Workbench.
- [ ] **T-APP-031.05 — Qualify and retain this target's own result**
  - Action: Bind Archive/Compression Workbench to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-031.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-032"></a>
## T-APP-032 — Search

**Original requirement:** system/content search respecting permissions and cancellation

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 161.

### Execution steps

- [ ] **T-APP-032.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Search to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-032.02 — Specify the complete target boundary**
  - Action: Search must supply: system/content search respecting permissions and cancellation. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-032.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-032.03 — Implement the exact target behavior**
  - Action: Implement or reuse Search through the shared platform contract, delivering every part of: system/content search respecting permissions and cancellation. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-032.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-032.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-032.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Search.
- [ ] **T-APP-032.05 — Qualify and retain this target's own result**
  - Action: Bind Search to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-032.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-033"></a>
## T-APP-033 — Dictionary/Thesaurus

**Original requirement:** local language data, definitions and writing integration

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 162.

### Execution steps

- [ ] **T-APP-033.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Dictionary/Thesaurus to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-033.02 — Specify the complete target boundary**
  - Action: Dictionary/Thesaurus must supply: local language data, definitions and writing integration. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-033.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-033.03 — Implement the exact target behavior**
  - Action: Implement or reuse Dictionary/Thesaurus through the shared platform contract, delivering every part of: local language data, definitions and writing integration. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-033.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-033.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-033.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Dictionary/Thesaurus.
- [ ] **T-APP-033.05 — Qualify and retain this target's own result**
  - Action: Bind Dictionary/Thesaurus to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-033.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-034"></a>
## T-APP-034 — Font Manager

**Original requirement:** preview/install/remove/license/conflict and fallback impact

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 163.

### Execution steps

- [ ] **T-APP-034.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Font Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-034.02 — Specify the complete target boundary**
  - Action: Font Manager must supply: preview/install/remove/license/conflict and fallback impact. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-034.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-034.03 — Implement the exact target behavior**
  - Action: Implement or reuse Font Manager through the shared platform contract, delivering every part of: preview/install/remove/license/conflict and fallback impact. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-034.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-034.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-034.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Font Manager.
- [ ] **T-APP-034.05 — Qualify and retain this target's own result**
  - Action: Bind Font Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-034.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-035"></a>
## T-APP-035 — Print Queue and Printer Setup

**Original requirement:** devices, previews, jobs, status, retry/cancel and privacy

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 164.

### Execution steps

- [ ] **T-APP-035.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Print Queue and Printer Setup to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-035.02 — Specify the complete target boundary**
  - Action: Print Queue and Printer Setup must supply: devices, previews, jobs, status, retry/cancel and privacy. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-035.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-035.03 — Implement the exact target behavior**
  - Action: Implement or reuse Print Queue and Printer Setup through the shared platform contract, delivering every part of: devices, previews, jobs, status, retry/cancel and privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-035.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-035.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-035.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Print Queue and Printer Setup.
- [ ] **T-APP-035.05 — Qualify and retain this target's own result**
  - Action: Bind Print Queue and Printer Setup to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-035.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-036"></a>
## T-APP-036 — Scanner

**Original requirement:** device, preview, region, quality, capture and document handoff

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 165.

### Execution steps

- [ ] **T-APP-036.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Scanner to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-036.02 — Specify the complete target boundary**
  - Action: Scanner must supply: device, preview, region, quality, capture and document handoff. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-036.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-036.03 — Implement the exact target behavior**
  - Action: Implement or reuse Scanner through the shared platform contract, delivering every part of: device, preview, region, quality, capture and document handoff. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-036.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-036.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-036.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Scanner.
- [ ] **T-APP-036.05 — Qualify and retain this target's own result**
  - Action: Bind Scanner to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-036.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-040"></a>
## T-APP-040 — Media Library/Player

**Original requirement:** library, playlists, metadata, audio/video playback and resume

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 171.

### Execution steps

- [ ] **T-APP-040.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Media Library/Player to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-040.02 — Specify the complete target boundary**
  - Action: Media Library/Player must supply: library, playlists, metadata, audio/video playback and resume. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-040.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-040.03 — Implement the exact target behavior**
  - Action: Implement or reuse Media Library/Player through the shared platform contract, delivering every part of: library, playlists, metadata, audio/video playback and resume. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-040.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-040.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-040.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Media Library/Player.
- [ ] **T-APP-040.05 — Qualify and retain this target's own result**
  - Action: Bind Media Library/Player to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-040.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-041"></a>
## T-APP-041 — Music Player

**Original requirement:** albums/playlists/queue/equalizer/metadata

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 172.

### Execution steps

- [ ] **T-APP-041.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Music Player to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-041.02 — Specify the complete target boundary**
  - Action: Music Player must supply: albums/playlists/queue/equalizer/metadata. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-041.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-041.03 — Implement the exact target behavior**
  - Action: Implement or reuse Music Player through the shared platform contract, delivering every part of: albums/playlists/queue/equalizer/metadata. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-041.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-041.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-041.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Music Player.
- [ ] **T-APP-041.05 — Qualify and retain this target's own result**
  - Action: Bind Music Player to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-041.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-042"></a>
## T-APP-042 — Video Player

**Original requirement:** seek, subtitles, tracks, full-screen and sync

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 173.

### Execution steps

- [ ] **T-APP-042.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Video Player to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-042.02 — Specify the complete target boundary**
  - Action: Video Player must supply: seek, subtitles, tracks, full-screen and sync. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-042.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-042.03 — Implement the exact target behavior**
  - Action: Implement or reuse Video Player through the shared platform contract, delivering every part of: seek, subtitles, tracks, full-screen and sync. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-042.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-042.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-042.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Video Player.
- [ ] **T-APP-042.05 — Qualify and retain this target's own result**
  - Action: Bind Video Player to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-042.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-043"></a>
## T-APP-043 — Audio Recorder

**Original requirement:** consent, meters, pause, project recovery and export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 174.

### Execution steps

- [ ] **T-APP-043.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Audio Recorder to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-043.02 — Specify the complete target boundary**
  - Action: Audio Recorder must supply: consent, meters, pause, project recovery and export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-043.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-043.03 — Implement the exact target behavior**
  - Action: Implement or reuse Audio Recorder through the shared platform contract, delivering every part of: consent, meters, pause, project recovery and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-043.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-043.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-043.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Audio Recorder.
- [ ] **T-APP-043.05 — Qualify and retain this target's own result**
  - Action: Bind Audio Recorder to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-043.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-044"></a>
## T-APP-044 — Audio Mixer/Patchbay

**Original requirement:** per-app/device routing, levels, mute and latency

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 175.

### Execution steps

- [ ] **T-APP-044.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Audio Mixer/Patchbay to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-044.02 — Specify the complete target boundary**
  - Action: Audio Mixer/Patchbay must supply: per-app/device routing, levels, mute and latency. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-044.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-044.03 — Implement the exact target behavior**
  - Action: Implement or reuse Audio Mixer/Patchbay through the shared platform contract, delivering every part of: per-app/device routing, levels, mute and latency. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-044.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-044.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-044.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Audio Mixer/Patchbay.
- [ ] **T-APP-044.05 — Qualify and retain this target's own result**
  - Action: Bind Audio Mixer/Patchbay to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-044.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-045"></a>
## T-APP-045 — Music/Synth/Sequencer

**Original requirement:** deterministic DSP, instruments, timeline and export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 176.

### Execution steps

- [ ] **T-APP-045.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Music/Synth/Sequencer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-045.02 — Specify the complete target boundary**
  - Action: Music/Synth/Sequencer must supply: deterministic DSP, instruments, timeline and export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-045.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-045.03 — Implement the exact target behavior**
  - Action: Implement or reuse Music/Synth/Sequencer through the shared platform contract, delivering every part of: deterministic DSP, instruments, timeline and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-045.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-045.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-045.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Music/Synth/Sequencer.
- [ ] **T-APP-045.05 — Qualify and retain this target's own result**
  - Action: Bind Music/Synth/Sequencer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-045.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-046"></a>
## T-APP-046 — Camera

**Original requirement:** permission, preview, capture, settings and gallery handoff

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 177.

### Execution steps

- [ ] **T-APP-046.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Camera to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-046.02 — Specify the complete target boundary**
  - Action: Camera must supply: permission, preview, capture, settings and gallery handoff. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-046.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-046.03 — Implement the exact target behavior**
  - Action: Implement or reuse Camera through the shared platform contract, delivering every part of: permission, preview, capture, settings and gallery handoff. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-046.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-046.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-046.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Camera.
- [ ] **T-APP-046.05 — Qualify and retain this target's own result**
  - Action: Bind Camera to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-046.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-047"></a>
## T-APP-047 — Screen Capture

**Original requirement:** region/window/display consent and protected-surface policy

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 178.

### Execution steps

- [ ] **T-APP-047.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Screen Capture to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-047.02 — Specify the complete target boundary**
  - Action: Screen Capture must supply: region/window/display consent and protected-surface policy. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-047.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-047.03 — Implement the exact target behavior**
  - Action: Implement or reuse Screen Capture through the shared platform contract, delivering every part of: region/window/display consent and protected-surface policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-047.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-047.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-047.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Screen Capture.
- [ ] **T-APP-047.05 — Qualify and retain this target's own result**
  - Action: Bind Screen Capture to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-047.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-048"></a>
## T-APP-048 — Screen Recorder

**Original requirement:** capture, audio choice, indicators, pause and recovery

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 179.

### Execution steps

- [ ] **T-APP-048.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Screen Recorder to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-048.02 — Specify the complete target boundary**
  - Action: Screen Recorder must supply: capture, audio choice, indicators, pause and recovery. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-048.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-048.03 — Implement the exact target behavior**
  - Action: Implement or reuse Screen Recorder through the shared platform contract, delivering every part of: capture, audio choice, indicators, pause and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-048.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-048.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-048.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Screen Recorder.
- [ ] **T-APP-048.05 — Qualify and retain this target's own result**
  - Action: Bind Screen Recorder to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-048.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-049"></a>
## T-APP-049 — Advanced Raster Editor

**Original requirement:** layers, masks, transforms, brushes, color and project format

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 180.

### Execution steps

- [ ] **T-APP-049.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Advanced Raster Editor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-049.02 — Specify the complete target boundary**
  - Action: Advanced Raster Editor must supply: layers, masks, transforms, brushes, color and project format. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-049.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-049.03 — Implement the exact target behavior**
  - Action: Implement or reuse Advanced Raster Editor through the shared platform contract, delivering every part of: layers, masks, transforms, brushes, color and project format. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-049.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-049.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-049.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Advanced Raster Editor.
- [ ] **T-APP-049.05 — Qualify and retain this target's own result**
  - Action: Bind Advanced Raster Editor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-049.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-050"></a>
## T-APP-050 — Vector Graphics Editor

**Original requirement:** paths/shapes/text/layers/import/export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 181.

### Execution steps

- [ ] **T-APP-050.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Vector Graphics Editor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-050.02 — Specify the complete target boundary**
  - Action: Vector Graphics Editor must supply: paths/shapes/text/layers/import/export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-050.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-050.03 — Implement the exact target behavior**
  - Action: Implement or reuse Vector Graphics Editor through the shared platform contract, delivering every part of: paths/shapes/text/layers/import/export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-050.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-050.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-050.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Vector Graphics Editor.
- [ ] **T-APP-050.05 — Qualify and retain this target's own result**
  - Action: Bind Vector Graphics Editor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-050.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-051"></a>
## T-APP-051 — 3D Model/Scene Viewer

**Original requirement:** assets, camera, materials, lighting and renderer fallback

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 182.

### Execution steps

- [ ] **T-APP-051.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve 3D Model/Scene Viewer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-051.02 — Specify the complete target boundary**
  - Action: 3D Model/Scene Viewer must supply: assets, camera, materials, lighting and renderer fallback. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-051.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-051.03 — Implement the exact target behavior**
  - Action: Implement or reuse 3D Model/Scene Viewer through the shared platform contract, delivering every part of: assets, camera, materials, lighting and renderer fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-051.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-051.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-051.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for 3D Model/Scene Viewer.
- [ ] **T-APP-051.05 — Qualify and retain this target's own result**
  - Action: Bind 3D Model/Scene Viewer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-051.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-052"></a>
## T-APP-052 — Animation Editor/Player

**Original requirement:** timeline, keyframes, easing, preview and export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 183.

### Execution steps

- [ ] **T-APP-052.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Animation Editor/Player to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-052.02 — Specify the complete target boundary**
  - Action: Animation Editor/Player must supply: timeline, keyframes, easing, preview and export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-052.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-052.03 — Implement the exact target behavior**
  - Action: Implement or reuse Animation Editor/Player through the shared platform contract, delivering every part of: timeline, keyframes, easing, preview and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-052.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-052.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-052.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Animation Editor/Player.
- [ ] **T-APP-052.05 — Qualify and retain this target's own result**
  - Action: Bind Animation Editor/Player to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-052.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-053"></a>
## T-APP-053 — Video Editor

**Original requirement:** timeline, clips, audio, effects, proxy/cache and recovery

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 184.

### Execution steps

- [ ] **T-APP-053.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Video Editor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-053.02 — Specify the complete target boundary**
  - Action: Video Editor must supply: timeline, clips, audio, effects, proxy/cache and recovery. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-053.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-053.03 — Implement the exact target behavior**
  - Action: Implement or reuse Video Editor through the shared platform contract, delivering every part of: timeline, clips, audio, effects, proxy/cache and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-053.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-053.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-053.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Video Editor.
- [ ] **T-APP-053.05 — Qualify and retain this target's own result**
  - Action: Bind Video Editor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-053.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-054"></a>
## T-APP-054 — Image Metadata/Batch Tool

**Original requirement:** inspect/edit metadata, convert/resize and reversible batch

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 185.

### Execution steps

- [ ] **T-APP-054.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Image Metadata/Batch Tool to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-054.02 — Specify the complete target boundary**
  - Action: Image Metadata/Batch Tool must supply: inspect/edit metadata, convert/resize and reversible batch. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-054.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-054.03 — Implement the exact target behavior**
  - Action: Implement or reuse Image Metadata/Batch Tool through the shared platform contract, delivering every part of: inspect/edit metadata, convert/resize and reversible batch. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-054.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-054.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-054.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Image Metadata/Batch Tool.
- [ ] **T-APP-054.05 — Qualify and retain this target's own result**
  - Action: Bind Image Metadata/Batch Tool to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-054.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-055"></a>
## T-APP-055 — Voice/Podcast Tool

**Original requirement:** multitrack record/edit, noise processing and export

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 186.

### Execution steps

- [ ] **T-APP-055.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Voice/Podcast Tool to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-055.02 — Specify the complete target boundary**
  - Action: Voice/Podcast Tool must supply: multitrack record/edit, noise processing and export. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-055.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-055.03 — Implement the exact target behavior**
  - Action: Implement or reuse Voice/Podcast Tool through the shared platform contract, delivering every part of: multitrack record/edit, noise processing and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-055.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-055.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-055.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Voice/Podcast Tool.
- [ ] **T-APP-055.05 — Qualify and retain this target's own result**
  - Action: Bind Voice/Podcast Tool to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-055.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-060"></a>
## T-APP-060 — Mail

**Original requirement:** accounts, folders, compose, attachments, search, offline and security

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 192.

### Execution steps

- [ ] **T-APP-060.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Mail to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-060.02 — Specify the complete target boundary**
  - Action: Mail must supply: accounts, folders, compose, attachments, search, offline and security. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-060.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-060.03 — Implement the exact target behavior**
  - Action: Implement or reuse Mail through the shared platform contract, delivering every part of: accounts, folders, compose, attachments, search, offline and security. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-060.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-060.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-060.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Mail.
- [ ] **T-APP-060.05 — Qualify and retain this target's own result**
  - Action: Bind Mail to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-060.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-061"></a>
## T-APP-061 — Calendar

**Original requirement:** calendars, recurrence, invitations, reminders and timezone

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 193.

### Execution steps

- [ ] **T-APP-061.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Calendar to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-061.02 — Specify the complete target boundary**
  - Action: Calendar must supply: calendars, recurrence, invitations, reminders and timezone. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-061.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-061.03 — Implement the exact target behavior**
  - Action: Implement or reuse Calendar through the shared platform contract, delivering every part of: calendars, recurrence, invitations, reminders and timezone. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-061.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-061.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-061.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Calendar.
- [ ] **T-APP-061.05 — Qualify and retain this target's own result**
  - Action: Bind Calendar to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-061.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-062"></a>
## T-APP-062 — Contacts

**Original requirement:** people/groups, import/export, permissions and account sync

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 194.

### Execution steps

- [ ] **T-APP-062.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Contacts to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-062.02 — Specify the complete target boundary**
  - Action: Contacts must supply: people/groups, import/export, permissions and account sync. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-062.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-062.03 — Implement the exact target behavior**
  - Action: Implement or reuse Contacts through the shared platform contract, delivering every part of: people/groups, import/export, permissions and account sync. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-062.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-062.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-062.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Contacts.
- [ ] **T-APP-062.05 — Qualify and retain this target's own result**
  - Action: Bind Contacts to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-062.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-063"></a>
## T-APP-063 — Tasks/Reminders

**Original requirement:** lists, due/recurring work, notifications and completion history

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 195.

### Execution steps

- [ ] **T-APP-063.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Tasks/Reminders to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-063.02 — Specify the complete target boundary**
  - Action: Tasks/Reminders must supply: lists, due/recurring work, notifications and completion history. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-063.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-063.03 — Implement the exact target behavior**
  - Action: Implement or reuse Tasks/Reminders through the shared platform contract, delivering every part of: lists, due/recurring work, notifications and completion history. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-063.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-063.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-063.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Tasks/Reminders.
- [ ] **T-APP-063.05 — Qualify and retain this target's own result**
  - Action: Bind Tasks/Reminders to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-063.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-064"></a>
## T-APP-064 — Messaging/Chat

**Original requirement:** conversations, attachments, presence, offline and privacy

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 196.

### Execution steps

- [ ] **T-APP-064.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Messaging/Chat to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-064.02 — Specify the complete target boundary**
  - Action: Messaging/Chat must supply: conversations, attachments, presence, offline and privacy. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-064.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-064.03 — Implement the exact target behavior**
  - Action: Implement or reuse Messaging/Chat through the shared platform contract, delivering every part of: conversations, attachments, presence, offline and privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-064.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-064.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-064.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Messaging/Chat.
- [ ] **T-APP-064.05 — Qualify and retain this target's own result**
  - Action: Bind Messaging/Chat to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-064.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-065"></a>
## T-APP-065 — Calls/Conference

**Original requirement:** audio/video calls, device/permission controls and network recovery

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 197.

### Execution steps

- [ ] **T-APP-065.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Calls/Conference to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-065.02 — Specify the complete target boundary**
  - Action: Calls/Conference must supply: audio/video calls, device/permission controls and network recovery. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-065.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-065.03 — Implement the exact target behavior**
  - Action: Implement or reuse Calls/Conference through the shared platform contract, delivering every part of: audio/video calls, device/permission controls and network recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-065.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-065.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-065.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Calls/Conference.
- [ ] **T-APP-065.05 — Qualify and retain this target's own result**
  - Action: Bind Calls/Conference to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-065.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-066"></a>
## T-APP-066 — RSS/News Reader

**Original requirement:** feeds, offline cache, reader mode and provenance

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 198.

### Execution steps

- [ ] **T-APP-066.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve RSS/News Reader to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-066.02 — Specify the complete target boundary**
  - Action: RSS/News Reader must supply: feeds, offline cache, reader mode and provenance. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-066.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-066.03 — Implement the exact target behavior**
  - Action: Implement or reuse RSS/News Reader through the shared platform contract, delivering every part of: feeds, offline cache, reader mode and provenance. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-066.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-066.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-066.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for RSS/News Reader.
- [ ] **T-APP-066.05 — Qualify and retain this target's own result**
  - Action: Bind RSS/News Reader to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-066.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-067"></a>
## T-APP-067 — Weather

**Original requirement:** consented location/manual places, cache and source/time identity

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 199.

### Execution steps

- [ ] **T-APP-067.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Weather to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-067.02 — Specify the complete target boundary**
  - Action: Weather must supply: consented location/manual places, cache and source/time identity. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-067.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-067.03 — Implement the exact target behavior**
  - Action: Implement or reuse Weather through the shared platform contract, delivering every part of: consented location/manual places, cache and source/time identity. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-067.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-067.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-067.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Weather.
- [ ] **T-APP-067.05 — Qualify and retain this target's own result**
  - Action: Bind Weather to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-067.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-068"></a>
## T-APP-068 — Maps

**Original requirement:** places, pan/zoom, routes, offline policy and location privacy

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 200.

### Execution steps

- [ ] **T-APP-068.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Maps to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-068.02 — Specify the complete target boundary**
  - Action: Maps must supply: places, pan/zoom, routes, offline policy and location privacy. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-068.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-068.03 — Implement the exact target behavior**
  - Action: Implement or reuse Maps through the shared platform contract, delivering every part of: places, pan/zoom, routes, offline policy and location privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-068.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-068.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-068.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Maps.
- [ ] **T-APP-068.05 — Qualify and retain this target's own result**
  - Action: Bind Maps to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-068.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-070"></a>
## T-APP-070 — Remote Desktop Client

**Original requirement:** authenticated sessions, display/input/clipboard controls

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 202.

### Execution steps

- [ ] **T-APP-070.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Remote Desktop Client to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, D-19, H-14.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-070.02 — Specify the complete target boundary**
  - Action: Remote Desktop Client must supply: authenticated sessions, display/input/clipboard controls. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-070.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-070.03 — Implement the exact target behavior**
  - Action: Implement or reuse Remote Desktop Client through the shared platform contract, delivering every part of: authenticated sessions, display/input/clipboard controls. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-070.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-070.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-070.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Remote Desktop Client.
- [ ] **T-APP-070.05 — Qualify and retain this target's own result**
  - Action: Bind Remote Desktop Client to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-070.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
