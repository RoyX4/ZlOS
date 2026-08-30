#!/usr/bin/env python3
"""Generate and validate the selected 23-row partial-feature closure program."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import sys
from collections import Counter
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
STATUS = ROOT / "docs/program/FEATURE-STATUS.json"
JSON_OUTPUT = ROOT / "docs/program/PARTIAL-CLOSURE.json"
MARKDOWN_OUTPUT = ROOT / "docs/program/PARTIAL-CLOSURE.md"

WAVES = (
    "H1_HOST_TRUTH",
    "H2_HOST_ADVERSARIAL",
    "Q1_CONTAINED_RUNTIME",
    "B1_CROSS_PHASE_FOUNDATION",
    "X1_EXTERNAL_AUTHORITY",
    "X2_PHYSICAL",
    "X3_INDEPENDENT",
    "X4_RELEASE",
)


def contract(wave: str, dependencies: tuple[str, ...], deliverables: tuple[str, ...],
             deterministic: tuple[str, ...], qemu: tuple[str, ...], external: tuple[str, ...],
             authority: tuple[str, ...], current: bool, promotion: str, next_action: str) -> dict:
    return {
        "wave": wave,
        "dependencies": list(dependencies),
        "deliverables": list(deliverables),
        "deterministic_proof": list(deterministic),
        "qemu_proof": list(qemu),
        "physical_or_external_proof": list(external),
        "external_authority": list(authority),
        "can_finish_in_current_environment": current,
        "promotion_rule": promotion,
        "next_action": next_action,
    }


CONTRACTS = {
    "EV-002": contract("X1_EXTERNAL_AUTHORITY", ("EV-001", "EV-004", "EV-005"),
        ("whole-repository deterministic archive including submodules, dirty patch and generated inputs", "signed snapshot manifest and restore procedure"),
        ("two clean restores reproduce every tracked and active dirty byte", "mutations to archive, manifest, submodule and patch fail closed"),
        ("restored tree rebuilds the exact registered artifacts",),
        ("off-host restore drill from an independently retained copy", "signature verification against a trusted public key"),
        ("trusted signing key", "off-host storage/custodian"), False,
        "Promote only when the whole repository is signed, independently retained off-host, restored, rebuilt and byte-matched.",
        "Extend the current build-input snapshot into a whole-tree snapshot and define the signing/restore envelope."),
    "EV-003": contract("X1_EXTERNAL_AUTHORITY", ("EV-002", "EV-004"),
        ("SPDX inventory for every first- and third-party file", "repository license decision", "redistribution notices and incompatible-license rejection"),
        ("every shipped byte resolves to a license and provenance record", "missing, conflicting and unknown licenses block release"),
        (), ("owner-approved license and required third-party permissions/notices",),
        ("repository owner legal/license choice", "third-party permission where required"), False,
        "Promote only after actual redistribution authority covers every shipped byte; an inventory alone is insufficient.",
        "Generate file-level SPDX candidates without inventing a grant, then request the owner license decision."),
    "EV-004": contract("H1_HOST_TRUTH", ("EV-002", "EV-003", "EV-005"),
        ("transitive source and binary SBOM", "locked package/source/archive identities", "offline dependency verification"),
        ("offline resolution succeeds from the lock and fails on any dependency mutation", "SBOM covers every build-graph input and artifact"),
        (), ("external source archives are retained under the snapshot custody contract",), (), True,
        "Promote when the current build resolves offline from a complete transitive SBOM with exact hashes and no undeclared dependency.",
        "Preserve the current offline archive receipt; hand off off-host custody to EV-002 and the hermetic rebuild to EV-005."),
    "EV-005": contract("X1_EXTERNAL_AUTHORITY", ("EV-002", "EV-004"),
        ("hermetic build environment", "compiler/sysroot source archive", "signed toolchain attestation"),
        ("two clean hermetic builds produce identical registered artifacts", "environment/path/time/network injection cannot affect output"),
        ("hermetic artifacts pass the contained boot matrix",),
        ("signature verification and independent source-archive restore",),
        ("trusted signing key", "off-host source custody"), False,
        "Promote only after the whole toolchain is source-custodied, hermetic, reproducible and signed.",
        "Define a network-disabled hermetic recipe around the existing seven-tool/four-ABI manifest."),
    "EV-007": contract("B1_CROSS_PHASE_FOUNDATION", ("EV-004", "EV-005", "EV-006"),
        ("per-object and generated-output receipts", "complete package/service/image output graph", "scope-only input elimination"),
        ("every output traces to exact inputs, command, environment and toolchain", "orphan input/output mutations fail"),
        ("booted image identity joins to the same graph root",),
        ("release artifacts join to signed provenance",), (), False,
        "Promote when every current build intermediate and shipped output has a reconstructable binary receipt; future phases extend the graph before landing.",
        "Emit per-object command/input/output hashes for the current four build lanes."),
    "EV-008": contract("H1_HOST_TRUTH", ("EV-012", "EV-013"),
        ("repository-wide executable/wrapper inventory", "strict exit propagation contract for every gate entry point"),
        ("each wrapper is forced to fail at every child command and returns nonzero", "deleted, optional and masked verifiers fail"),
        (), (), (), True,
        "Promote when every current executable verification/build wrapper is inventoried and every mandatory child failure propagates nonzero.",
        "Enumerate all executable and CI entry points, then mutation-test their child exit propagation."),
    "EV-013": contract("B1_CROSS_PHASE_FOUNDATION", ("EV-008", "EV-012", "EV-019", "EV-020"),
        ("one planted defect per required field of every verifier", "mutation inventory generated from live schemas"),
        ("every planted defect independently turns the owning gate red", "removed fields and new verifiers cannot escape inventory"),
        ("target-only fields have boot-path mutations",),
        ("release and physical receipt mutations are rejected by their custodians",), (), False,
        "Promote for a release only when every required field in every then-current authority has a demonstrated red mutation.",
        "Generate mutation obligations from the current JSON schemas and land-gate registry."),
    "EV-014": contract("X2_PHYSICAL", ("EV-005", "EV-009", "EV-021"),
        ("percentile, peak, distribution and machine/backend metadata", "budget and regression comparison receipt"),
        ("host measurements are repeatable and raw samples reconstruct summaries", "over-budget results remain red"),
        ("contained target collects frame/input/I/O/launch/memory samples",),
        ("native physical distribution on identified hardware with exact artifact hash",),
        ("physical target and controlled reboot access",), False,
        "Promote only when all registered budgets pass statistically useful target distributions, including required physical lanes.",
        "Preserve the current raw-sample/p50/p95/peak host receipt; collect equivalent contained-target and identified physical distributions before release promotion."),
    "EV-015": contract("X2_PHYSICAL", ("EV-009", "EV-014"),
        ("per-device hardware receipt schema", "first-use/reuse/reset/teardown/recovery evidence"),
        ("current validator rejects wrong artifact, firmware, topology, device, lane, lifecycle, digest or missing raw evidence",),
        ("QEMU remains a separate non-physical comparison lane",),
        ("all required devices boot the exact artifact and pass behavior/recovery probes",),
        ("physical machines/devices", "controlled cold/warm reboot and failure injection"), False,
        "Promote only when every required physical lane is bound to the exact artifact and raw device/firmware/topology evidence.",
        "Execute the six-case physical matrix on identified machines and retain the ten required raw-evidence classes without promoting unrun routes."),
    "EV-016": contract("Q1_CONTAINED_RUNTIME", ("EV-009", "EV-022", "EV-023"),
        ("current-build screenshot/video capture runner", "variant and interaction metadata"),
        ("asset hash, dimensions, build identity and capture metadata are internally consistent",),
        ("capture the current BIOS/UEFI UI across required states and variants",),
        ("required native display captures bind to physical receipts",),
        ("identified native display hardware",), False,
        "Promote when current-build captures cover every required variant/state and bind byte-for-byte to the artifact and visual registry.",
        "Extend the current exact-artifact BIOS/native-UEFI desktop/Paint receipt across the remaining scale, theme, locale, accessibility, state and backend matrix; retain physical display proof as external."),
    "EV-017": contract("Q1_CONTAINED_RUNTIME", ("EV-009", "EV-018", "EV-024", "KR-032"),
        ("bounded crash bundle with full registers, symbols, input/state identity and checksum", "durable persistence and recovery workflow"),
        ("truncation/corruption/stale-symbol/wrong-build mutations fail", "nested crash cannot overwrite the first cause"),
        ("kernel and user faults persist a bundle and recover according to policy",),
        ("physical power-loss persistence/recovery where required",), (), False,
        "Promote when kernel and user crash paths create durable exact-build bundles and recovery succeeds without corrupting prior evidence.",
        "Exercise error-code and nested/double-fault paths, then add checked symbol/unwind binding and a durable recovery-readable crash store."),
    "EV-018": contract("B1_CROSS_PHASE_FOUNDATION", ("EV-017", "EV-024", "AU-028"),
        ("target event emitters", "SMP/IRQ ownership and sequence contract", "service export/redaction policy", "durable tamper-evident anchor"),
        ("concurrency, wrap, quota, redaction and corruption tests preserve ordering and bounds",),
        ("kernel, driver and service events traverse the booted target route",),
        ("signed/exported audit anchor verified by an authorized custodian",), (), False,
        "Promote when pointer-free bounded events are emitted on target, safely exported/redacted and durably anchored under explicit authority.",
        "Compile the current event core into the kernel and emit boot, interrupt, fault and lifecycle events."),
        "EV-019": contract("B1_CROSS_PHASE_FOUNDATION", ("EV-013", "EV-020", "EV-024"),
        ("exhaustive allocation/queue/I/O/provider/service/package/lifecycle injection map", "rollback-state equivalence receipts"),
        ("every injectable step fails before/during/after commit with no leak, stale authority or partial publication",),
        ("target-only device/service failure paths are exercised",),
        ("physical detach/reset/power-loss injections cover required hardware",), (), False,
        "Promote for a release only when every then-current transactional seam has nth-step failure proof and exact rollback receipts.",
        "Preserve the current 32-position heap and 512-write page-table receipts; add pre/post equivalence injectors for queue, I/O, provider, service, package and lifecycle commit paths."),
    "EV-020": contract("B1_CROSS_PHASE_FOUNDATION", ("EV-013", "EV-019", "EV-024"),
        ("ELF, archive, font and typed-IPC hostile corpora", "minimized regression cases and sanitizer lanes"),
        ("truncation, overflow, overlap, recursion, malformed schema and quota cases fail safely",),
        ("target parsers reject the same corpus without kernel compromise",), (), (), False,
        "Promote when every parser/decoder boundary in the release has a versioned hostile corpus, sanitizer proof and target rejection path.",
        "Replay the current ELF/archive corpus on target; add font and IPC cases when those product parsers exist."),
    "EV-021": contract("Q1_CONTAINED_RUNTIME", ("EV-014", "EV-016", "EV-019", "EV-022"),
        ("joined frame/input/I/O/network/launch/memory/build budget registry", "raw samples and regression baselines"),
        ("every budget can be forced over and blocks promotion", "measurement overhead and environment are recorded"),
        ("contained target collects every current runtime metric",),
        ("required hardware lanes meet their budgets",), (), False,
        "Promote when every current performance budget has deterministic collection, a red regression mutation and required target evidence.",
        "Keep every measured over-budget category red; remeasure under a controlled load, add a timed product-artifact build distribution, then replace host-gate runtimes with contained-target latency distributions."),
    "EV-022": contract("Q1_CONTAINED_RUNTIME", ("EV-016", "EV-023", "EV-028"),
        ("current-build visual goldens", "scale/theme/locale/accessibility/state/backend matrix", "perceptual and structural diff policy"),
        ("one planted defect per required visual field or region turns the gate red",),
        ("capture and compare all required booted variants",),
        ("native display lane confirms required hardware-specific output",),
        ("identified native display hardware",), False,
        "Promote when every required UI route/state/variant has a current artifact-bound golden and rejecting visual/structural comparison.",
        "Expand the four strict current RGB goldens and four live-region masks across every required variant; retain native display comparison as external."),
    "EV-023": contract("B1_CROSS_PHASE_FOUNDATION", ("AX-001", "AX-003", "AP-001", "EV-022"),
        ("semantic tree and assistive-provider protocols", "keyboard/focus/screen-reader/magnifier/caption/switch workflows", "component/app coverage inventory"),
        ("every component exposes correct role/name/state/action and focus order", "privacy, disconnect and provider failure are bounded"),
        ("complete workflows run on the target with real providers",),
        ("physical assistive input/output lanes where required",), (), False,
        "Promote when every shipped component/app passes target keyboard, semantic and required assistive workflows; missing support stays red.",
        "Implement the semantic-tree transport and require every current primitive to expose a deterministic snapshot."),
    "EV-024": contract("B1_CROSS_PHASE_FOUNDATION", ("AU-001", "AU-006", "AU-012", "AU-024", "AU-028"),
        ("capability, credential, typed IPC, DMA, secrets, sandbox, package and audit foundations", "complete privileged-operation mediation matrix"),
        ("ordinary processes are denied every privileged operation without the exact live right", "revocation/stale-generation/peer-death/failure tests"),
        ("booted hostile processes cannot escape authority or crash the system",),
        ("IOMMU/DMA and hardware privilege lanes where required", "independent security refutation"),
        ("independent security reviewer for final promotion",), False,
        "Promote only when all production claims are independently refuted, residual risks are resolved/accepted, and complete mediation passes on target.",
        "Implement capability handles and bounded typed IPC before migrating privileged policy out of the kernel."),
    "EV-025": contract("X3_INDEPENDENT", ("EV-001", "EV-009", "EV-013", "EV-024"),
        ("independent review packet", "reviewer rejection/acceptance receipt", "immutable response and remediation links"),
        ("packet is exact-build complete and cannot omit failed evidence",),
        ("reviewer can reproduce the contained target gates",),
        ("review is executed by a genuinely independent principal with rejection authority",),
        ("independent reviewer/custodian",), False,
        "Promote only after an independent reviewer reproduces/refutes the exact batch and signs a receipt with genuine rejection authority.",
        "Generate the exact review packet and keep status partial until an independent reviewer acts."),
    "EV-026": contract("X3_INDEPENDENT", ("EV-001", "EV-025", "EV-027", "EV-028"),
        ("system-wide decision/reversal/deprecation inventory", "normalized meanings for all legacy labels", "history viewer and approval chain"),
        ("deleted/superseded decisions remain traceable and contradictions fail",),
        ("booted provenance viewer exposes the decision history read-only",),
        ("signed or independent approval for release-affecting decisions",),
        ("independent approver or trusted signing key",), False,
        "Promote when all system decisions are attributable, historically preserved, normalized and independently approved where release-affecting.",
        "Expand discovery beyond the now-normalized 47-label legacy source into system-wide decisions and approval history."),
    "EV-027": contract("X4_RELEASE", ("EV-002", "EV-003", "EV-005", "EV-025", "OP-001", "OP-002", "OP-004"),
        ("authorized version/channel manifest", "signed release notes", "complete migrations and rollback generation", "publication receipt"),
        ("release generation fails on license, signature, evidence, migration, rollback or known-issue omissions",),
        ("candidate installs/upgrades/rolls back in contained targets",),
        ("authorized public/internal channel publication and signature verification",),
        ("release authority", "trusted signing key", "legal redistribution authority"), False,
        "Promote only for a real authorized release with signed notes, passing migrations, a previous signed rollback generation and publication receipt.",
        "Keep generating UNRELEASED changes; do not fabricate a version or publication before the release authorities exist."),
    "EV-028": contract("B1_CROSS_PHASE_FOUNDATION", ("EV-003", "EV-009", "EV-023", "EV-024", "EV-027", "AP-001", "AP-006"),
        ("booted read-only provenance application", "live health and per-app permission views", "signature verification and authenticated remote portal", "current screenshot/a11y receipt"),
        ("viewer cannot mutate authority or hide stale/failed/untrusted state", "permission/signature/health gaps display explicitly"),
        ("target app joins live evidence and survives provider failure",),
        ("remote access is authenticated and release signatures verify",), (), False,
        "Promote when the booted target viewer presents live exact-build provenance, permissions, health and signatures with accessibility and authenticated remote proof.",
        "Define the read-only target data protocol; retain the static host viewer as a projection only."),
    "KR-032": contract("Q1_CONTAINED_RUNTIME", ("KR-006", "KR-007", "KR-009", "EV-017", "EV-024"),
        ("full general-register exception frame", "nested/double-fault emergency path", "spurious IRQ accounting", "user-fault offender-only termination"),
        ("all vectors preserve frame invariants and malformed/nested paths cannot recurse unboundedly",),
        ("kernel fault halts safely; user fault kills only the offender; desktop/system continue; spurious IRQs are acknowledged/accounted",),
        ("required physical interrupt-controller and fault lanes",), (), False,
        "Promote when kernel, nested, spurious and user-fault paths preserve full state and containment on QEMU plus required physical interrupt hardware.",
        "Exercise kernel-fatal error-code, nested/double-fault and spurious-interrupt paths, then extend the bounded two-slot Ring-3 containment proof to the general process lifecycle."),
}


def fail(message: str) -> None:
    raise ValueError(message)


def load_status() -> dict:
    if not STATUS.is_file():
        fail(f"missing status ledger: {STATUS}")
    return json.loads(STATUS.read_text(encoding="utf-8"))


def validate(value: dict, status: dict) -> None:
    if value.get("schema") != "zlos.partial-closure.v1" or value.get("result") != "PASS_WITH_EXTERNAL_GATES":
        fail("schema/result is wrong or external gates were hidden")
    canonical = {row["id"] for row in status.get("features", [])}
    status_map = {row["id"]: row for row in status.get("features", [])}
    expected_ids = list(CONTRACTS)
    rows = value.get("partials", [])
    ids = [row.get("id") for row in rows]
    if ids != expected_ids or len(ids) != 23 or len(set(ids)) != 23:
        fail(f"partial IDs differ: actual={ids!r}, expected={expected_ids!r}")
    for row in rows:
        status_row = status_map[row["id"]]
        if row.get("feature") != status_row.get("feature") or row.get("current_known_gaps") != status_row.get("known_gaps"):
            fail(f"{row.get('id')}: current status evidence/gaps drift")
        if row.get("current_maturity") != status_row.get("maturity"):
            fail(f"{row.get('id')}: current maturity drift")
        if row.get("wave") not in WAVES:
            fail(f"{row['id']}: unknown wave")
        dependencies = row.get("dependencies", [])
        if len(dependencies) != len(set(dependencies)) or any(dep not in canonical for dep in dependencies):
            fail(f"{row['id']}: unknown or duplicate dependency")
        if row["id"] in dependencies:
            fail(f"{row['id']}: self dependency")
        for field in ("deliverables", "deterministic_proof", "promotion_rule", "next_action"):
            if not row.get(field):
                fail(f"{row['id']}: empty {field}")
        for field in ("dependencies", "deliverables", "deterministic_proof", "qemu_proof",
                      "physical_or_external_proof", "external_authority"):
            if not isinstance(row.get(field), list) or any(not isinstance(item, str) or not item for item in row[field]):
                fail(f"{row['id']}: {field} is not a nonempty-string list")
        authority = row.get("external_authority", [])
        external_wave = row["wave"].startswith("X")
        if external_wave and not authority:
            fail(f"{row['id']}: external wave hides authority")
        if authority and row.get("can_finish_in_current_environment"):
            fail(f"{row['id']}: external authority falsely marked locally finishable")
        if row.get("can_finish_in_current_environment") and row["wave"] not in {"H1_HOST_TRUTH", "H2_HOST_ADVERSARIAL", "Q1_CONTAINED_RUNTIME"}:
            fail(f"{row['id']}: cross-phase/external work falsely marked locally finishable")
    counts = Counter(row["wave"] for row in rows)
    expected_counts = {wave: counts.get(wave, 0) for wave in WAVES}
    open_rows = [row for row in rows if row["current_maturity"] != "PROVED_CURRENT"]
    if value.get("counts") != {"total": 23, "proved_current": len(rows) - len(open_rows), "still_partial": sum(row["current_maturity"] == "PARTIAL_CURRENT" for row in rows), "locally_finishable_now": sum(bool(row["can_finish_in_current_environment"]) for row in open_rows), "external_authority_required": sum(bool(row["external_authority"]) for row in open_rows), "by_wave": expected_counts}:
        fail("closure counts drift")
    digest = hashlib.sha256(STATUS.read_bytes()).hexdigest()
    if value.get("feature_status_sha256") != digest:
        fail("closure ledger is stale against FEATURE-STATUS.json")


def build(status: dict) -> dict:
    rows = []
    status_map = {row["id"]: row for row in status["features"]}
    for feature_id in CONTRACTS:
        current = status_map[feature_id]
        row = {
            "id": current["id"],
            "feature": current["feature"],
            "current_evidence": current.get("evidence", []),
            "current_tests": current.get("tests", []),
            "current_known_gaps": current.get("known_gaps", []),
            "current_maturity": current["maturity"],
            **copy.deepcopy(CONTRACTS[current["id"]]),
        }
        rows.append(row)
    counts = Counter(row["wave"] for row in rows)
    open_rows = [row for row in rows if row["current_maturity"] != "PROVED_CURRENT"]
    return {
        "schema": "zlos.partial-closure.v1",
        "result": "PASS_WITH_EXTERNAL_GATES",
        "feature_status_sha256": hashlib.sha256(STATUS.read_bytes()).hexdigest(),
        "counts": {
            "total": len(rows),
            "proved_current": len(rows) - len(open_rows),
            "still_partial": sum(row["current_maturity"] == "PARTIAL_CURRENT" for row in rows),
            "locally_finishable_now": sum(bool(row["can_finish_in_current_environment"]) for row in open_rows),
            "external_authority_required": sum(bool(row["external_authority"]) for row in open_rows),
            "by_wave": {wave: counts.get(wave, 0) for wave in WAVES},
        },
        "partials": rows,
    }


def cell(values: list[str]) -> str:
    return "<br>".join(value.replace("|", "\\|") for value in values) if values else "none"


def render(value: dict) -> str:
    counts = value["counts"]
    lines = [
        "# Exact closure program for the selected 23-row partial batch",
        "",
        "Generated by `tools/gen_partial_closure.py`; do not hand-edit.",
        "",
        "Result: **PASS WITH EXTERNAL GATES**. This is the execution contract for",
        "closing the selected rows below; it is not a live inventory of every row",
        "currently marked `PARTIAL_CURRENT`, and it is not a claim",
        "that legal authority, trusted custody, physical hardware, release authority",
        "or independent review can be manufactured inside this checkout.",
        "",
        f"Exact batch rows: **{counts['total']}**. Promoted current: **{counts['proved_current']}**. Still partial: **{counts['still_partial']}**. Locally finishable under the current contract: **{counts['locally_finishable_now']}**. Rows with explicit external authority: **{counts['external_authority_required']}**.",
        "",
        "## Dependency flow",
        "",
        "```text",
        "H1 host truth -> H2 adversarial proof -> Q1 contained target proof",
        "       |                  |                    |",
        "       +---------- B1 cross-phase foundations +",
        "                          |",
        "             X1 custody/legal -> X2 physical",
        "                          +----> X3 independent review -> X4 release",
        "```",
        "",
        "A row is promoted only when its own promotion rule is satisfied. A later",
        "feature may add new obligations and reopen a release-level registry without",
        "invalidating the exact historical receipt for an earlier build.",
        "",
        "## All 23 rows",
        "",
        "| ID | Feature | Current status | Wave | Dependencies | Locally finishable now | External authority | Next action |",
        "|---|---|---|---|---|---|---|---|",
    ]
    for row in value["partials"]:
        local = "closed" if row["current_maturity"] == "PROVED_CURRENT" else "yes" if row["can_finish_in_current_environment"] else "no"
        lines.append(f"| {row['id']} | {row['feature']} | {row['current_maturity']} | {row['wave']} | {cell(row['dependencies'])} | {local} | {cell(row['external_authority'])} | {row['next_action'].replace('|', '\\|')} |")
    lines.extend(["", "## Detailed closure contracts", ""])
    for row in value["partials"]:
        lines.extend([
            f"### {row['id']} — {row['feature']}", "",
            f"- Current gaps: {cell(row['current_known_gaps'])}.",
            f"- Deliverables: {cell(row['deliverables'])}.",
            f"- Deterministic proof: {cell(row['deterministic_proof'])}.",
            f"- QEMU proof: {cell(row['qemu_proof'])}.",
            f"- Physical/external proof: {cell(row['physical_or_external_proof'])}.",
            f"- Promotion rule: {row['promotion_rule']}", "",
        ])
    lines.extend([
        "## Execution order",
        "",
        "1. EV-004 and EV-008 are closed for the current host-truth batch; preserve their receipts while downstream work proceeds.",
        "2. Add present-day adversarial/parser work without pretending future IPC, package or service seams already exist.",
        "3. When the machine is idle, run the contained QEMU work for EV-016, EV-022, EV-021, EV-017 and KR-032.",
        "4. Build the cross-phase security, accessibility, event, failure-injection and provenance foundations dependency-first.",
        "5. Execute physical, custody/legal, independent-review and release gates only with the named authority and retain immutable receipts.",
        "",
        "## Non-negotiable external gates",
        "",
        "- EV-002/EV-005 need a trusted signing key and off-host custody.",
        "- EV-003 needs an actual repository license decision and any required third-party permissions.",
        "- EV-014/EV-015 need identified physical hardware and controlled test access.",
        "- EV-025/EV-026/EV-024 need genuine independent rejection/approval where specified.",
        "- EV-027 needs authorized release, publication, signing and rollback generations.",
        "",
        "Until those events happen, the correct state is partial—not failed and not complete.",
        "",
    ])
    return "\n".join(lines)


def selftest(value: dict, status: dict) -> None:
    mutants = {}
    missing = copy.deepcopy(value); missing["partials"].pop(); mutants["missing-row"] = missing
    unknown = copy.deepcopy(value); unknown["partials"][0]["dependencies"].append("ZZ-999"); mutants["unknown-dependency"] = unknown
    hidden = copy.deepcopy(value); ext = next(row for row in hidden["partials"] if row["wave"].startswith("X")); ext["external_authority"] = []; mutants["hidden-authority"] = hidden
    false_local = copy.deepcopy(value); ext = next(row for row in false_local["partials"] if row["external_authority"]); ext["can_finish_in_current_environment"] = True; mutants["false-local-closure"] = false_local
    no_rule = copy.deepcopy(value); no_rule["partials"][0]["promotion_rule"] = ""; mutants["missing-promotion-rule"] = no_rule
    caught = []
    for name, mutant in mutants.items():
        try:
            validate(mutant, status)
        except ValueError:
            caught.append(name)
        else:
            fail(f"self-test mutation escaped: {name}")
    print("partial-closure self-test: PASS: " + ", ".join(caught))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if not args.write and not args.check:
        args.check = True
    try:
        status = load_status()
        value = build(status)
        validate(value, status)
        json_text = json.dumps(value, indent=2, sort_keys=False) + "\n"
        markdown_text = render(value)
        if args.write:
            JSON_OUTPUT.write_text(json_text, encoding="utf-8")
            MARKDOWN_OUTPUT.write_text(markdown_text, encoding="utf-8")
        if args.check:
            if not JSON_OUTPUT.is_file() or JSON_OUTPUT.read_text(encoding="utf-8") != json_text:
                fail("PARTIAL-CLOSURE.json is missing or stale; run --write")
            if not MARKDOWN_OUTPUT.is_file() or MARKDOWN_OUTPUT.read_text(encoding="utf-8") != markdown_text:
                fail("PARTIAL-CLOSURE.md is missing or stale; run --write")
        if args.selftest:
            selftest(value, status)
    except (KeyError, ValueError) as error:
        print(f"partial-closure: FAIL: {error}", file=sys.stderr)
        return 1
    print(f"partial-closure: PASS: batch={value['counts']['total']} proved={value['counts']['proved_current']} partial={value['counts']['still_partial']} locally_finishable={value['counts']['locally_finishable_now']} external_authority={value['counts']['external_authority_required']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
