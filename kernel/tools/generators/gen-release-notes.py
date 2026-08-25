#!/usr/bin/env python3
"""Generate an honest unreleased zlOS changelog from current manifests."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
ROOT = KERNEL_ROOT.parent
METADATA = KERNEL_ROOT / "metadata"
JSON_OUTPUT = METADATA / "release-notes.json"
MARKDOWN_OUTPUT = KERNEL_ROOT / "docs" / "UNRELEASED-CHANGELOG.md"
INPUTS = (
    "build-identity.json",
    "decision-ledger.json",
    "artifact-registry.json",
    "app-evidence.json",
    "license-registry.json",
    "docs/receipts/source-snapshot-2026-08-24.json",
    "toolchain-manifest.json",
    "build-graph.json",
    "tests/host/test-run-receipt.json",
    "docs/receipts/benchmark-host-2026-08-23.json",
    "visual-registry.json",
    "accessibility-registry.json",
    "security-registry.json",
    "observability-registry.json",
)
OPERATIONAL_SOURCES = (
    "gates/run-land-gate-contained.sh",
    "kernel/tools/checks/verify-sources.sh",
)

AUDIENCE = {
    "DEC-0001": "DEVELOPERS",
    "DEC-0003": "USERS_AND_DEVELOPERS",
    "DEC-0004": "USERS_AND_DEVELOPERS",
    "DEC-0005": "DEVELOPERS",
    "DEC-0006": "DEVELOPERS",
    "DEC-0008": "USERS_AND_DEVELOPERS",
    "DEC-0009": "USERS_AND_DEVELOPERS",
    "DEC-0010": "DEVELOPERS",
    "DEC-0011": "USERS_AND_DEVELOPERS",
    "DEC-0012": "USERS_AND_DEVELOPERS",
    "DEC-0013": "DISTRIBUTORS",
    "DEC-0014": "DEVELOPERS",
    "DEC-0015": "DEVELOPERS",
    "DEC-0016": "DEVELOPERS",
    "DEC-0017": "OPERATORS",
    "DEC-0018": "DEVELOPERS",
    "DEC-0019": "OPERATORS",
}

COMPATIBILITY_NOTES = {
    "DEC-0001": "No public ABI change; preserve the current C/zl boundary until the replacement language/runtime contract is proved.",
    "DEC-0003": "Rendering output must retain the five-path clipping oracle at every supported mode.",
    "DEC-0004": "Keep the existing drag fallback until retained damage repaint passes equivalent host and target gates.",
    "DEC-0005": "Intel display/blitter work remains bounded native-provider work; Linux i915/Mesa ABI compatibility is not promised.",
    "DEC-0006": "Every kernel compile route must consume kernel/SOURCES; route-local source lists are unsupported.",
    "DEC-0008": "The supported browser contract is the explicit bounded compatibility ledger, not Chrome pixel/API parity.",
    "DEC-0009": "Application identities and routes must match the generated manifest; blank or dead catalogue IDs are invalid.",
    "DEC-0010": "A receipt applies only to the exact artifact and route it names.",
    "DEC-0011": "Intel host-harness modesetting is not native-boot display compatibility.",
    "DEC-0012": "I2C-HID raw transport diagnostics are not input-event compatibility.",
    "DEC-0013": "No public redistribution channel is supported until licensing authority exists.",
    "DEC-0014": "The local source archive is recovery material, not a signed/off-host repository release.",
    "DEC-0015": "Tool or firmware byte drift requires an inspected lock refresh; silent compatibility is rejected.",
    "DEC-0016": "Scope-only inputs affect identity but are not claimed as compiled objects.",
    "DEC-0017": "Uncontained full landing-gate invocation is unsupported on this host.",
    "DEC-0018": "The structured-event core is host-only and provides no shipped event-service ABI yet.",
    "DEC-0019": "The interrupted complete gate provides no green compatibility or release claim.",
}

USER_VISIBLE = {
    "DEC-0003", "DEC-0004", "DEC-0008", "DEC-0009", "DEC-0011", "DEC-0012"
}

OPEN_GAPS = {
    "released_generation_exists": False,
    "public_release_authorized": False,
    "signed_release_notes": False,
    "published_user_visible_entries": 0,
    "automatic_change_classification_complete": False,
    "user_data_migration_inventory_complete": False,
    "previous_release_rollback_artifact_available": False,
    "historical_release_series_complete": False,
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def input_path(relative: str) -> Path:
    if relative.startswith("docs/"):
        return KERNEL_ROOT / relative
    if relative.startswith("tests/"):
        return KERNEL_ROOT / relative
    return METADATA / relative


def load(relative: str) -> dict:
    return json.loads(input_path(relative).read_text())


def documents() -> dict[str, dict]:
    return {path: load(path) for path in INPUTS}


def current_changes(decisions: dict) -> list[dict]:
    records = [row for row in decisions["records"] if row["status"] != "SUPERSEDED"]
    expected_ids = {row["id"] for row in records}
    if set(AUDIENCE) != expected_ids or set(COMPATIBILITY_NOTES) != expected_ids:
        raise ValueError("release-note classification does not exactly cover current decisions")
    return [{
        "decision_id": row["id"],
        "date": row["date"],
        "change_type": row["action"],
        "decision_status": row["status"],
        "feature_ids": row["features"],
        "audience": AUDIENCE[row["id"]],
        "user_visible_candidate": row["id"] in USER_VISIBLE,
        "summary": row["decision"],
        "rationale": row["rationale"],
        "compatibility": COMPATIBILITY_NOTES[row["id"]],
        "evidence": {
            "path": "kernel/decision-ledger.json",
            "sha256": sha256(METADATA / "decision-ledger.json"),
            "record_id": row["id"],
        },
        "evidence_ceiling": "unreleased change candidate; not a published or runtime-complete release claim",
    } for row in records]


def superseded_history(decisions: dict) -> list[dict]:
    return [{
        "decision_id": row["id"],
        "replaced_by": row["replaced_by"],
        "obsolete_evidence": row["obsolete_evidence"],
    } for row in decisions["records"] if row["status"] == "SUPERSEDED"]


def migrations() -> list[dict]:
    return [
        {
            "id": "MIG-001",
            "decision_id": "DEC-0006",
            "scope": "developer-build-routes",
            "required_now": True,
            "instruction": "Use kernel/SOURCES as the only ordered kernel source manifest for every compiler route.",
            "verification": "kernel/verify-sources.sh",
            "recovery": "kernel/verify-sources.sh --recover-only",
        },
        {
            "id": "MIG-002",
            "decision_id": "DEC-0004",
            "scope": "window-drag-rendering",
            "required_now": False,
            "instruction": "Retire snapshot-and-sticker dragging only after retained damage repaint passes equivalent gates.",
            "verification": "kernel/tests/host/fbbench",
            "recovery": "Keep the proven fallback until replacement evidence exists; do not delete first.",
        },
        {
            "id": "MIG-003",
            "decision_id": "DEC-0017",
            "scope": "operator-full-gate",
            "required_now": True,
            "instruction": "Start a complete landing gate only through gates/run-land-gate-contained.sh on an idle host.",
            "verification": "gates/run-land-gate-contained.sh doctor",
            "recovery": "gates/run-land-gate-contained.sh stop",
        },
    ]


def known_issues(docs: dict[str, dict]) -> list[dict]:
    license_registry = docs["license-registry.json"]
    artifact = docs["artifact-registry.json"]
    tests = docs["tests/host/test-run-receipt.json"]
    benchmark = docs["docs/receipts/benchmark-host-2026-08-23.json"]
    visual = docs["visual-registry.json"]
    accessibility = docs["accessibility-registry.json"]
    security = docs["security-registry.json"]
    observability = docs["observability-registry.json"]
    decisions = docs["decision-ledger.json"]
    snapshot = docs["docs/receipts/source-snapshot-2026-08-24.json"]
    toolchain = docs["toolchain-manifest.json"]
    return [
        {
            "id": "ISSUE-001", "severity": "RELEASE_BLOCKER",
            "feature_ids": ["EV-003", "OP-001"],
            "summary": "Public redistribution authority is absent.",
            "facts": {
                "public_release_blocked": license_registry["public_release_blocked"],
                "inputs_without_established_grant": (
                    license_registry["counts"]["build_inputs"]
                    - license_registry["counts"]["inputs_with_established_redistribution_grant"]
                ),
            },
            "source": "kernel/license-registry.json",
        },
        {
            "id": "ISSUE-002", "severity": "RELEASE_BLOCKER",
            "feature_ids": ["EV-009", "EV-015"],
            "summary": "No current artifact hash has exact physical-hardware proof.",
            "facts": {"artifacts_without_physical_exact_hash_proof": len(artifact["artifacts"])},
            "source": "kernel/artifact-registry.json",
        },
        {
            "id": "ISSUE-003", "severity": "OPEN_GAP",
            "feature_ids": ["EV-012", "EV-015"],
            "summary": "Host inventory retains hardware skips and non-runs.",
            "facts": {
                "hardware_skips": tests["counts"]["skipped-hardware"],
                "not_run": tests["counts"]["not-run"],
            },
            "source": "kernel/tests/host/test-run-receipt.json",
        },
        {
            "id": "ISSUE-004", "severity": "OPEN_REGRESSION",
            "feature_ids": ["EV-014", "EV-021", "GR-020"],
            "summary": "Some frame metrics exceed budget and no native-target distribution exists.",
            "facts": {
                "over_budget": benchmark["counts"]["over_budget"],
                "native_target_measurements": benchmark["counts"]["native_target_measurements"],
            },
            "source": "kernel/docs/receipts/benchmark-host-2026-08-23.json",
        },
        {
            "id": "ISSUE-005", "severity": "OPEN_GAP",
            "feature_ids": ["EV-016", "EV-022"],
            "summary": "Visual assets are not bound to the current build.",
            "facts": {
                "assets": visual["counts"]["assets"],
                "current_build_bound": visual["counts"]["current_build_bound"],
                "variant_dimensions_open": visual["counts"]["variant_dimensions_open"],
            },
            "source": "kernel/visual-registry.json",
        },
        {
            "id": "ISSUE-006", "severity": "OPEN_GAP",
            "feature_ids": ["EV-023"],
            "summary": "Accessibility capabilities and complete target workflows remain missing.",
            "facts": {
                "missing_capabilities": accessibility["counts"]["missing"],
                "complete_target_workflows": accessibility["counts"]["complete_target_workflows"],
            },
            "source": "kernel/accessibility-registry.json",
        },
        {
            "id": "ISSUE-007", "severity": "SECURITY_BLOCKER",
            "feature_ids": ["EV-024"],
            "summary": "Security evidence is incomplete and no claim is production-complete.",
            "facts": {
                "missing_claims": security["counts"]["missing"],
                "production_complete": security["counts"]["production_complete"],
            },
            "source": "kernel/security-registry.json",
        },
        {
            "id": "ISSUE-008", "severity": "OPEN_GAP",
            "feature_ids": ["EV-017", "EV-018"],
            "summary": "Durable crash/audit records and booted structured-event emitters are absent.",
            "facts": {
                "missing_observability_capabilities": observability["counts"]["missing"],
                "durable_crash_receipts": observability["counts"]["durable_crash_receipts"],
                "target_event_emitters": 0,
            },
            "source": "kernel/observability-registry.json",
        },
        {
            "id": "ISSUE-009", "severity": "OPEN_GAP",
            "feature_ids": ["EV-026", "EV-027"],
            "summary": "Historical decision and release-note coverage is incomplete.",
            "facts": {
                "legacy_decision_semantics_open": decisions["coverage"]["legacy_labels_remaining"],
                "released_generations": 0,
            },
            "source": "kernel/decision-ledger.json",
        },
        {
            "id": "ISSUE-010", "severity": "RELEASE_BLOCKER",
            "feature_ids": ["EV-008", "OP-001"],
            "summary": "The latest complete landing gate is unverified after interruption.",
            "facts": {"complete_gate_green": False, "interrupted_decision": "DEC-0019"},
            "source": "kernel/decision-ledger.json",
        },
        {
            "id": "ISSUE-011", "severity": "RECOVERY_BLOCKER",
            "feature_ids": ["EV-002"],
            "summary": "Source custody has no off-host copy or signature.",
            "facts": {
                "off_host_copies": snapshot["open_gaps"]["off_host_copies"],
                "signed_attestation": snapshot["open_gaps"]["signed_attestation"],
            },
            "source": "kernel/docs/receipts/source-snapshot-2026-08-24.json",
        },
        {
            "id": "ISSUE-012", "severity": "PORTABILITY_BLOCKER",
            "feature_ids": ["EV-005"],
            "summary": "The toolchain is exact locally but not hermetic or signed.",
            "facts": {
                "target_lanes": toolchain["counts"]["target_lanes"],
                "hermetic_builds": 0,
                "signed_attestations": 0,
            },
            "source": "kernel/toolchain-manifest.json",
        },
    ]


def recovery_paths() -> list[dict]:
    return [
        {
            "id": "REC-001", "scope": "interrupted-sources-transaction",
            "available": True,
            "detect": "kernel/verify-sources.sh --recover-only",
            "recover": "kernel/verify-sources.sh --recover-only",
            "proof": "The command must report a clean transaction before any build.",
        },
        {
            "id": "REC-002", "scope": "contained-landing-gate",
            "available": True,
            "detect": "gates/run-land-gate-contained.sh status",
            "recover": "gates/run-land-gate-contained.sh stop",
            "proof": "Then run doctor; do not bypass load, memory, compiler or QEMU refusal.",
        },
        {
            "id": "REC-003", "scope": "previous-public-release-generation",
            "available": False,
            "detect": "No public release generation is recorded.",
            "recover": "Unavailable until a signed previous-generation artifact and selection receipt exist.",
            "proof": "This unavailable path remains a release blocker.",
        },
    ]


def compatibility(docs: dict[str, dict]) -> dict:
    artifact = docs["artifact-registry.json"]
    app = docs["app-evidence.json"]
    toolchain = docs["toolchain-manifest.json"]
    return {
        "build_identity": docs["build-identity.json"]["identity_sha256"],
        "dirty_development_build": docs["build-identity.json"]["git"]["dirty"],
        "artifact_names": sorted(artifact["artifacts"]),
        "qemu_boot_routes": sorted(artifact["boot_routes"]),
        "toolchain_lanes": [row["id"] for row in toolchain["target_lanes"]],
        "application_identities": app["counts"]["identities"],
        "application_lifecycle_qemu_proved": app["counts"]["with_qemu_open_ready_close"],
        "physical_exact_hash_proofs": 0,
        "public_distribution_supported": False,
    }


def validate(value: dict, docs: dict[str, dict]) -> None:
    if value.get("schema") != "zlos.release-notes.v1" \
            or value.get("result") != "PASS_UNRELEASED_WITH_BLOCKERS":
        raise ValueError("wrong release-notes schema/result")
    identity = docs["build-identity.json"]["identity_sha256"]
    if value.get("build_identity") != identity:
        raise ValueError("release notes have stale build identity")
    release = value.get("release", {})
    if release != {
        "state": "UNRELEASED_DEVELOPMENT",
        "version": None,
        "release_date": None,
        "channel": "NONE",
        "published_entries": 0,
        "signed": False,
        "public_release_blocked": True,
    }:
        raise ValueError("unreleased state was hidden or promoted")
    expected_inputs = [{
        "path": input_path(path).relative_to(ROOT).as_posix(),
        "sha256": sha256(input_path(path)),
        "schema": docs[path].get("schema"),
    } for path in INPUTS]
    if value.get("inputs") != expected_inputs:
        raise ValueError("release-note input closure drift")
    expected_operational = [{"path": path, "sha256": sha256(ROOT / path)}
                            for path in OPERATIONAL_SOURCES]
    if value.get("operational_sources") != expected_operational:
        raise ValueError("release recovery source drift")
    decisions = docs["decision-ledger.json"]
    if value.get("changes") != current_changes(decisions):
        raise ValueError("current change candidates drift")
    if value.get("superseded_history") != superseded_history(decisions):
        raise ValueError("superseded change history was lost")
    if value.get("migrations") != migrations():
        raise ValueError("migration instructions drift")
    if value.get("known_issues") != known_issues(docs):
        raise ValueError("known issue inventory drift")
    if value.get("recovery") != recovery_paths():
        raise ValueError("recovery contract drift")
    if value.get("compatibility") != compatibility(docs):
        raise ValueError("compatibility manifest drift")
    changes = value["changes"]
    expected_counts = {
        "unreleased_change_candidates": 17,
        "user_visible_candidates": 6,
        "published_entries": 0,
        "superseded_records_retained": 2,
        "migrations": 3,
        "known_issues": 12,
        "recovery_paths": 3,
        "available_recovery_paths": 2,
    }
    if value.get("counts") != expected_counts \
            or len(changes) != expected_counts["unreleased_change_candidates"] \
            or sum(row["user_visible_candidate"] for row in changes) != 6:
        raise ValueError("release-note counts drift")
    if value.get("open_gaps") != OPEN_GAPS:
        raise ValueError("release-note gaps were hidden")
    if len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("missing release-note generator identity")


def build() -> dict:
    docs = documents()
    identity = docs["build-identity.json"]["identity_sha256"]
    joined_identities = (
        docs["decision-ledger.json"]["build_identity"],
        docs["artifact-registry.json"]["build_identity"]["id"],
        docs["app-evidence.json"]["shipped_build_identity"]["id"],
        docs["license-registry.json"]["build_identity"],
        docs["docs/receipts/source-snapshot-2026-08-24.json"]["build_identity"],
        docs["toolchain-manifest.json"]["build_identity"],
        docs["build-graph.json"]["build_identity"],
        docs["docs/receipts/benchmark-host-2026-08-23.json"]["build_identity"],
        docs["visual-registry.json"]["build_identity"],
        docs["accessibility-registry.json"]["build_identity"],
        docs["security-registry.json"]["build_identity"],
        docs["observability-registry.json"]["build_identity"],
    )
    if any(item != identity for item in joined_identities):
        raise ValueError("release-note manifests disagree on build identity")
    if docs["license-registry.json"]["public_release_blocked"] is not True:
        raise ValueError("release notes require the current license release block")
    changes = current_changes(docs["decision-ledger.json"])
    value = {
        "schema": "zlos.release-notes.v1",
        "result": "PASS_UNRELEASED_WITH_BLOCKERS",
        "feature_id": "EV-027",
        "build_identity": identity,
        "release": {
            "state": "UNRELEASED_DEVELOPMENT",
            "version": None,
            "release_date": None,
            "channel": "NONE",
            "published_entries": 0,
            "signed": False,
            "public_release_blocked": True,
        },
        "inputs": [{
            "path": input_path(path).relative_to(ROOT).as_posix(),
            "sha256": sha256(input_path(path)),
            "schema": docs[path].get("schema"),
        } for path in INPUTS],
        "operational_sources": [{"path": path, "sha256": sha256(ROOT / path)}
                                for path in OPERATIONAL_SOURCES],
        "changes": changes,
        "superseded_history": superseded_history(docs["decision-ledger.json"]),
        "migrations": migrations(),
        "known_issues": known_issues(docs),
        "recovery": recovery_paths(),
        "compatibility": compatibility(docs),
        "counts": {
            "unreleased_change_candidates": len(changes),
            "user_visible_candidates": sum(row["user_visible_candidate"] for row in changes),
            "published_entries": 0,
            "superseded_records_retained": 2,
            "migrations": 3,
            "known_issues": 12,
            "recovery_paths": 3,
            "available_recovery_paths": 2,
        },
        "open_gaps": OPEN_GAPS,
        "generator": {
            "path": "kernel/tools/generators/gen-release-notes.py",
            "sha256": sha256(Path(__file__).resolve()),
        },
        "evidence_ceiling": "generated unreleased development changelog; not a versioned, signed, distributed or runtime-complete release",
        "weakest_link": "there is no public release generation, redistribution authority, signed note, complete migration inventory or previous-generation rollback artifact",
    }
    validate(value, docs)
    return value


def render_markdown(value: dict) -> str:
    lines = [
        "# zlOS unreleased development changelog",
        "",
        "> **UNRELEASED — not a public release.** No version, channel, signature or",
        "> redistribution authority exists. These are source-bound change candidates,",
        "> not promises that every listed feature is shipped or runtime-complete.",
        "",
        f"Build identity: `{value['build_identity']}`.",
        "",
        "## Change candidates",
        "",
        "| Decision | Audience | Visible | Change | Compatibility |",
        "|---|---|---:|---|---|",
    ]
    for row in value["changes"]:
        visible = "yes" if row["user_visible_candidate"] else "no"
        summary = row["summary"].replace("|", "\\|")
        compatibility_note = row["compatibility"].replace("|", "\\|")
        lines.append(
            f"| {row['decision_id']} | {row['audience']} | {visible} | "
            f"{summary} | {compatibility_note} |"
        )
    lines.extend([
        "",
        "## Required migrations",
        "",
    ])
    for row in value["migrations"]:
        timing = "required now" if row["required_now"] else "deferred until replacement proof"
        lines.extend([
            f"### {row['id']} — {row['scope']} ({timing})",
            "",
            row["instruction"],
            "",
            f"- Verify: `{row['verification']}`",
            f"- Recover: `{row['recovery']}`",
            "",
        ])
    lines.extend([
        "## Known defects and blockers",
        "",
        "| Issue | Severity | Summary | Current facts |",
        "|---|---|---|---|",
    ])
    for row in value["known_issues"]:
        facts = ", ".join(f"{key}={json.dumps(item, sort_keys=True)}"
                          for key, item in row["facts"].items())
        lines.append(
            f"| {row['id']} | {row['severity']} | {row['summary']} | {facts} |"
        )
    lines.extend([
        "",
        "## Recovery",
        "",
    ])
    for row in value["recovery"]:
        availability = "available" if row["available"] else "unavailable"
        lines.extend([
            f"- **{row['id']} {row['scope']} ({availability}):** detect with "
            f"`{row['detect']}`; recover with `{row['recover']}`. {row['proof']}",
        ])
    lines.extend([
        "",
        "## Evidence ceiling",
        "",
        value["evidence_ceiling"] + ".",
        "",
        "Generated by `kernel/gen-release-notes.py`; do not hand-edit.",
        "",
    ])
    return "\n".join(lines)


def selftest(value: dict, docs: dict[str, dict]) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["changes"].pop()
    mutations["missing-change"] = missing
    released = copy.deepcopy(value)
    released["release"]["state"] = "RELEASED"
    mutations["invented-release"] = released
    public = copy.deepcopy(value)
    public["release"]["public_release_blocked"] = False
    mutations["hidden-release-block"] = public
    issue = copy.deepcopy(value)
    issue["known_issues"].pop()
    mutations["missing-known-issue"] = issue
    recovery = copy.deepcopy(value)
    recovery["migrations"][0]["recovery"] = ""
    mutations["missing-migration-recovery"] = recovery
    history = copy.deepcopy(value)
    history["superseded_history"] = []
    mutations["deleted-superseded-history"] = history
    gap = copy.deepcopy(value)
    gap["open_gaps"]["historical_release_series_complete"] = True
    mutations["hidden-history-gap"] = gap
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant, docs)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"release-notes mutation escaped: {name}")
    print("release-notes selftest: caught " + ", ".join(caught))


def write_atomic(path: Path, text: str) -> None:
    with tempfile.NamedTemporaryFile("w", dir=path.parent, delete=False,
                                     encoding="utf-8") as handle:
        handle.write(text)
        temporary = Path(handle.name)
    os.replace(temporary, path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        docs = documents()
        value = build()
        if args.selftest:
            selftest(value, docs)
        json_text = json.dumps(value, indent=2) + "\n"
        markdown_text = render_markdown(value)
        if args.write:
            write_atomic(JSON_OUTPUT, json_text)
            write_atomic(MARKDOWN_OUTPUT, markdown_text)
        elif not JSON_OUTPUT.is_file() or JSON_OUTPUT.read_text() != json_text \
                or not MARKDOWN_OUTPUT.is_file() \
                or MARKDOWN_OUTPUT.read_text() != markdown_text:
            raise ValueError("release-note outputs are missing or stale")
        print(
            "release-notes: PASS_UNRELEASED_WITH_BLOCKERS: "
            f"{value['counts']['unreleased_change_candidates']} candidates, "
            f"{value['counts']['known_issues']} issues, "
            f"{value['counts']['published_entries']} published"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"release-notes: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
