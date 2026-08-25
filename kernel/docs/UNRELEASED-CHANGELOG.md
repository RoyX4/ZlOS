# zlOS unreleased development changelog

> **UNRELEASED — not a public release.** No version, channel, signature or
> redistribution authority exists. These are source-bound change candidates,
> not promises that every listed feature is shipped or runtime-complete.

Build identity: `85027b159c9a594045c2f900e5971bb3408dd418dd61a373625425fba9030d13`.

## Change candidates

| Decision | Audience | Visible | Change | Compatibility |
|---|---|---:|---|---|
| DEC-0001 | DEVELOPERS | no | Keep low-level window mechanisms in C and desktop policy in zl until the language/runtime can own the required structures safely. | No public ABI change; preserve the current C/zl boundary until the replacement language/runtime contract is proved. |
| DEC-0003 | USERS_AND_DEVELOPERS | yes | Enforce clipping in all five back-buffer write paths by folding the scissor into loop bounds. | Rendering output must retain the five-path clipping oracle at every supported mode. |
| DEC-0004 | USERS_AND_DEVELOPERS | yes | Retire snapshot-and-sticker window dragging after damage repaint is authoritative; retain only cursor save-under. | Keep the existing drag fallback until retained damage repaint passes equivalent host and target gates. |
| DEC-0005 | DEVELOPERS | no | Do not port a Linux-scale GPU stack; implement bounded native mechanisms while borrowing public hardware knowledge, not donor code. | Intel display/blitter work remains bounded native-provider work; Linux i915/Mesa ABI compatibility is not promised. |
| DEC-0006 | DEVELOPERS | no | All four kernel compiler routes consume one ordered kernel/SOURCES manifest. | Every kernel compile route must consume kernel/SOURCES; route-local source lists are unsupported. |
| DEC-0008 | USERS_AND_DEVELOPERS | yes | Build the bounded document-browser capability while explicitly refusing moving-target Chrome parity. | The supported browser contract is the explicit bounded compatibility ledger, not Chrome pixel/API parity. |
| DEC-0009 | USERS_AND_DEVELOPERS | yes | Replace the false-green aggregate application check with exact manifest, route and lifecycle evidence. | Application identities and routes must match the generated manifest; blank or dead catalogue IDs are invalid. |
| DEC-0010 | DEVELOPERS | no | Never promote one artifact or route from another route's runtime or physical evidence. | A receipt applies only to the exact artifact and route it names. |
| DEC-0011 | USERS_AND_DEVELOPERS | yes | Label Intel modeset writes as host-harness proved and boot-unreachable until a native kernel caller and receipt exist. | Intel host-harness modesetting is not native-boot display compatibility. |
| DEC-0012 | USERS_AND_DEVELOPERS | yes | Classify I2C-HID as a raw transport, not an input provider, until report decoding and session routing exist. | I2C-HID raw transport diagnostics are not input-event compatibility. |
| DEC-0013 | DISTRIBUTORS | no | Block public release while build inputs lack an established repository redistribution grant. | No public redistribution channel is supported until licensing authority exists. |
| DEC-0014 | DEVELOPERS | no | Call the deterministic archive exact build-input recovery, not signed/off-host/whole-repository custody. | The local source archive is recovery material, not a signed/off-host repository release. |
| DEC-0015 | DEVELOPERS | no | Dependency verification checks exact invoked/resolved bytes and never silently refreshes the lock. | Tool or firmware byte drift requires an inspected lock refresh; silent compatibility is rejected. |
| DEC-0016 | DEVELOPERS | no | Represent conservative build-identity inputs without active edges as scope-only, never compiled. | Scope-only inputs affect identity but are not claimed as compiled objects. |
| DEC-0017 | OPERATORS | no | Forbid unrestricted full landing gates and require the resource-contained launcher and idle-host doctor. | Uncontained full landing-gate invocation is unsupported on this host. |
| DEC-0018 | DEVELOPERS | no | Prove the event envelope as a host-only single-owner core before changing shipped build identity or claiming an audit service. | The structured-event core is host-only and provides no shipped event-service ABI yet. |
| DEC-0019 | OPERATORS | no | Keep the complete landing gate red/unverified until a contained run finishes and its final receipt is captured. | The interrupted complete gate provides no green compatibility or release claim. |

## Required migrations

### MIG-001 — developer-build-routes (required now)

Use kernel/SOURCES as the only ordered kernel source manifest for every compiler route.

- Verify: `kernel/tools/checks/verify-sources.sh`
- Recover: `kernel/tools/checks/verify-sources.sh --recover-only`

### MIG-002 — window-drag-rendering (deferred until replacement proof)

Retire snapshot-and-sticker dragging only after retained damage repaint passes equivalent gates.

- Verify: `kernel/hosttest/fbbench`
- Recover: `Keep the proven fallback until replacement evidence exists; do not delete first.`

### MIG-003 — operator-full-gate (required now)

Start a complete landing gate only through gates/run-land-gate-contained.sh on an idle host.

- Verify: `gates/run-land-gate-contained.sh doctor`
- Recover: `gates/run-land-gate-contained.sh stop`

## Known defects and blockers

| Issue | Severity | Summary | Current facts |
|---|---|---|---|
| ISSUE-001 | RELEASE_BLOCKER | Public redistribution authority is absent. | public_release_blocked=true, inputs_without_established_grant=123 |
| ISSUE-002 | RELEASE_BLOCKER | No current artifact hash has exact physical-hardware proof. | artifacts_without_physical_exact_hash_proof=9 |
| ISSUE-003 | OPEN_GAP | Host inventory retains hardware skips and non-runs. | hardware_skips=3, not_run=11 |
| ISSUE-004 | OPEN_REGRESSION | Some frame metrics exceed budget and no native-target distribution exists. | over_budget=4, native_target_measurements=0 |
| ISSUE-005 | OPEN_GAP | Visual assets are not bound to the current build. | assets=46, current_build_bound=0, variant_dimensions_open=6 |
| ISSUE-006 | OPEN_GAP | Accessibility capabilities and complete target workflows remain missing. | missing_capabilities=9, complete_target_workflows=0 |
| ISSUE-007 | SECURITY_BLOCKER | Security evidence is incomplete and no claim is production-complete. | missing_claims=11, production_complete=0 |
| ISSUE-008 | OPEN_GAP | Durable crash/audit records and booted structured-event emitters are absent. | missing_observability_capabilities=5, durable_crash_receipts=0, target_event_emitters=0 |
| ISSUE-009 | OPEN_GAP | Historical decision and release-note coverage is incomplete. | legacy_decision_semantics_open=0, released_generations=0 |
| ISSUE-010 | RELEASE_BLOCKER | The latest complete landing gate is unverified after interruption. | complete_gate_green=false, interrupted_decision="DEC-0019" |
| ISSUE-011 | RECOVERY_BLOCKER | Source custody has no off-host copy or signature. | off_host_copies=0, signed_attestation=false |
| ISSUE-012 | PORTABILITY_BLOCKER | The toolchain is exact locally but not hermetic or signed. | target_lanes=4, hermetic_builds=0, signed_attestations=0 |

## Recovery

- **REC-001 interrupted-sources-transaction (available):** detect with `kernel/tools/checks/verify-sources.sh --recover-only`; recover with `kernel/tools/checks/verify-sources.sh --recover-only`. The command must report a clean transaction before any build.
- **REC-002 contained-landing-gate (available):** detect with `gates/run-land-gate-contained.sh status`; recover with `gates/run-land-gate-contained.sh stop`. Then run doctor; do not bypass load, memory, compiler or QEMU refusal.
- **REC-003 previous-public-release-generation (unavailable):** detect with `No public release generation is recorded.`; recover with `Unavailable until a signed previous-generation artifact and selection receipt exist.`. This unavailable path remains a release blocker.

## Evidence ceiling

generated unreleased development changelog; not a versioned, signed, distributed or runtime-complete release.

Generated by `kernel/tools/generators/gen-release-notes.py`; do not hand-edit.
