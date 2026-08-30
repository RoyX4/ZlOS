#!/usr/bin/env python3
"""Generate the conservative 906-row implementation maturity ledger."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import subprocess
import sys
from collections import Counter
from pathlib import Path

import validate_master_program as master


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "docs/program/FEATURE-STATUS.json"
DEFAULT_EVIDENCE_ROOT = ROOT.parent / "zl-linux-master-program-foundation"

OVERRIDES = {
    "EV-001": ("PROVED_CURRENT", (("plan", "docs/program/FEATURE-MAP.md"),),
               ("python3 tools/gen_feature_status.py --check --selftest",),
               ("docs/program/FEATURE-STATUS.json",),
               ("missing feature", "duplicate feature", "unproved promotion"),
               (), "generated 906-row ledger is current; later feature receipts are still mostly absent"),
    "EV-002": ("PARTIAL_CURRENT", (("implementation", "kernel/docs/receipts/source-snapshot-2026-08-24.json"),),
               ("python3 kernel/gen-source-snapshot.py --check --selftest",),
               ("kernel/docs/receipts/source-snapshot-2026-08-24.json",
                "kernel/docs/receipts/source-snapshot-build-inputs-2026-08-24.tar"),
               ("missing input", "archive hash drift", "archive byte corruption", "invented custody"),
               ("0 off-host copies", "unsigned receipt", "not a whole-repository snapshot"),
               "all 123 exact build inputs are reconstructable, but the archive remains unsigned in the same uncommitted worktree"),
    "EV-003": ("PARTIAL_CURRENT", (("implementation", "kernel/license-registry.json"),),
               ("python3 kernel/gen-license-registry.py --check --selftest",),
               ("kernel/license-registry.json",), ("invented grant", "false release green"),
               ("0 license files", "123 inputs lack an established redistribution grant"),
               "build-input inventory exists, but public redistribution authority is absent"),
    "EV-004": ("PARTIAL_CURRENT", (("implementation", "kernel/dependency-lock.json"),),
               ("python3 kernel/gen-dependency-lock.py --check --selftest",),
               ("kernel/dependency-lock.json",),
               ("binary drift", "missing firmware identity", "missing package",
                "unresolved package dependency", "invented source-archive custody"),
               ("156 package source archives absent", "offline rebuild not proved"),
               "15 commands, 2 firmware blobs, 91 runtime files and 156 recursively installed packages are exact; source archives and offline reconstruction remain absent"),
    "EV-005": ("PARTIAL_CURRENT", (("implementation", "kernel/toolchain-manifest.json"),
                                    ("implementation", "kernel/dependency-lock.json")),
               ("python3 kernel/gen-toolchain-manifest.py --check --selftest",),
               ("kernel/toolchain-manifest.json", "kernel/dependency-lock.json"),
               ("missing target lane", "wrong LLP64", "missing warning policy",
                "external header drift", "environment injection"),
               ("0 hermetic builds", "unsigned toolchain", "compiler/sysroot sources unarchived"),
               "7 tools, 4 ABI lanes and 82 external headers are exact locally, but the toolchain is not hermetic or source-custodied"),
    "EV-006": ("PROVED_CURRENT", (("implementation", "kernel/docs/receipts/reproducible-build-2026-08-22.json"),),
               ("python3 kernel/check-reproducible-build.py --check --selftest",),
               ("kernel/docs/receipts/reproducible-build-2026-08-22.json",),
               ("byte drift in every artifact", "missing artifact", "stale resume snapshot"), (),
               "proved for the current nine-artifact recipe set; future artifacts must join separately"),
    "EV-007": ("PARTIAL_CURRENT", (("implementation", "kernel/build-graph.json"),
                                    ("implementation", "kernel/artifact-registry.json")),
               ("python3 kernel/gen-build-graph.py --check --selftest",),
               ("kernel/build-graph.json", "kernel/artifact-registry.json"),
               ("missing source", "missing lane", "orphan source", "missing artifact"),
               ("1 conservative scope-only header", "no per-object binary receipts",
                "future package/service outputs absent"),
               "all 123 inputs and 9 artifacts have graph positions across 4 lanes, but intermediate object provenance remains source-derived"),
    "EV-008": ("PROVED_CURRENT", (("implementation", "kernel/wrapper-registry.json"),
                                     ("implementation", "kernel/adversarial-registry.json")),
               ("python3 kernel/gen-wrapper-registry.py --check --selftest",
                "python3 kernel/check-land-gate.py --selftest"),
               ("kernel/wrapper-registry.json", "kernel/adversarial-registry.json"),
               ("missing wrapper", "duplicate wrapper", "missing policy", "hidden authority boundary",
                "deleted verifier", "optional verifier", "missing route", "masked final exit", "masked child failure"),
               (), "all 141 current shebang wrappers are inventoried; the sole contained landing authority has 66 mandatory seams and rejects child/final exit masking"),
    "EV-009": ("PROVED_CURRENT", (("implementation", "kernel/artifact-registry.json"),
                                   ("implementation", "kernel/app-evidence.json")),
               ("python3 kernel/gen-artifact-registry.py --check --selftest",),
               ("kernel/artifact-registry.json", "kernel/app-evidence.json"),
               ("artifact mismatch", "wrong build identity"), (),
               "current shipped artifacts and QEMU routes identify the exact build; physical hash proof remains separate"),
    "EV-010": ("PROVED_CURRENT", (("plan", "docs/program/PROOF-GATES.md"),
                                   ("implementation", "kernel/evidence-registry.json")),
               ("python3 kernel/gen-evidence-registry.py --check --selftest",),
               ("docs/program/PROOF-GATES.md", "kernel/evidence-registry.json"),
               ("physical overclaim", "hidden hardware skip"), (),
               "the vocabulary and joined receipt preserve distinct evidence ceilings"),
    "EV-011": ("PROVED_CURRENT", (("plan", "docs/program/FEATURE-MAP.md"),),
               ("python3 tools/gen_feature_status.py --check --selftest",),
               ("docs/program/FEATURE-STATUS.json",), ("hidden gap", "unproved promotion"), (),
               "every feature has a conservative current maturity row; most remain planned/unproved"),
    "EV-012": ("PROVED_CURRENT", (("implementation", "kernel/tests/host/test-inventory.json"),
                                   ("implementation", "kernel/tests/host/test-run-receipt.json")),
               ("python3 kernel/run-host-tests.py --selftest",),
               ("kernel/tests/host/test-inventory.json", "kernel/tests/host/test-run-receipt.json"),
               ("missing target", "promoted instrument", "hidden skip"), (),
               "62 current targets are classified and receipted; 53 automatic commands include 48 passes, 3 hardware skips and later target suites must join"),
    "EV-013": ("PARTIAL_CURRENT", (("implementation", "kernel/adversarial-registry.json"),),
               ("python3 kernel/gen-adversarial-registry.py --check --selftest",),
               ("kernel/adversarial-registry.json",), ("19 planted verifier mutations",),
               ("one mutation per every future required field is not yet possible",),
               "19 current authorities prove red, not every future feature verifier"),
    "EV-014": ("PARTIAL_CURRENT", (("implementation", "kernel/docs/receipts/benchmark-host-2026-08-23.json"),),
               ("python3 kernel/run-benchmarks.py --check --selftest",),
               ("kernel/docs/receipts/benchmark-host-2026-08-23.json",),
               ("hidden regression", "invented percentile", "target overclaim"),
               ("4 frame metrics over budget", "no percentiles/peaks/native-target distribution"),
               "host frame receipt exists, but it is not a complete performance qualification"),
    "EV-015": ("PARTIAL_CURRENT", (("implementation", "kernel/artifact-registry.json"),),
               ("python3 kernel/gen-artifact-registry.py --check --selftest",),
               ("kernel/artifact-registry.json",), ("physical overclaim",),
               ("0 of 9 exact artifact hashes have native physical proof",),
               "hardware evidence is explicitly unpromoted; complete physical receipts remain open"),
    "EV-016": ("PARTIAL_CURRENT", (("implementation", "kernel/visual-registry.json"),),
               ("python3 kernel/gen-visual-registry.py --check --selftest",),
               ("kernel/visual-registry.json",),
               ("invented build binding", "hidden variant gap"),
               ("46 visual assets are unbound to the current build", "6 variant dimensions are open"),
               "assets are hashed and measured, but none is a current screenshot/video receipt"),
    "EV-017": ("PARTIAL_CURRENT", (("implementation", "kernel/observability-registry.json"),),
               ("python3 kernel/gen-observability-registry.py --check --selftest",),
               ("kernel/observability-registry.json",),
               ("overpromoted fault frame", "invented general registers", "invented raw log", "hidden crash gap"),
               ("0 durable crash receipts", "no general-register frame, symbols, persistence or recovery"),
               "QEMU proves a bounded checksummed control-frame record and exact invalid-opcode IP before halt; durable crash bundles, general registers, symbols and recovery remain absent"),
    "EV-018": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/tests/host/trace_event.c"),
                   ("implementation", "kernel/tests/host/trace_event.h"),
                   ("implementation", "kernel/event-schema.json"),
                   ("implementation", "kernel/observability-registry.json")),
               ("python3 kernel/verify-event-trace.py --check --selftest",
                "python3 kernel/gen-event-schema.py --check --selftest",
                "python3 kernel/gen-observability-registry.py --check --selftest"),
               ("kernel/docs/receipts/event-trace-host-2026-08-24.json",
                "kernel/event-schema.json", "kernel/observability-registry.json"),
               ("missing event field", "overlapping event field", "hidden overwrite",
                "invented concurrency", "invented target emitter", "hidden observability gap"),
               ("0 target emitters", "single-owner core only", "no QEMU/native route",
                "no durable tamper-evident anchor or service export policy"),
               "28-field pointer-free envelope and bounded queue pass 37 host checks across 3 compile lanes; the core is not compiled into or emitted by booted zlOS"),
    "EV-019": ("PARTIAL_CURRENT", (("implementation", "kernel/adversarial-registry.json"),),
               ("python3 kernel/gen-adversarial-registry.py --check --selftest",),
               ("kernel/adversarial-registry.json",), ("hidden failure gap",),
               ("all 7 failure families remain non-exhaustive",),
               "allocation/queue/I/O/lifecycle are partial; provider/service/package injection is absent"),
    "EV-020": ("PARTIAL_CURRENT", (("implementation", "kernel/adversarial-registry.json"),),
               ("python3 kernel/gen-adversarial-registry.py --check --selftest",),
               ("kernel/adversarial-registry.json",), ("unearned ELF promotion", "hidden hostile gap"),
               ("ELF/archive/font/IPC absent",),
               "5 of 9 hostile families have executed host proof; PNG is now sanitizer-gated"),
    "EV-021": ("PARTIAL_CURRENT", (("implementation", "kernel/docs/receipts/benchmark-host-2026-08-23.json"),),
               ("python3 kernel/run-benchmarks.py --check --selftest",),
               ("kernel/docs/receipts/benchmark-host-2026-08-23.json",),
               ("hidden performance regression",),
               ("input/I/O/network/launch/memory/build budgets are not joined",),
               "frame budget is partly machine-checked; the complete performance registry is not"),
    "EV-022": ("PARTIAL_CURRENT", (("implementation", "kernel/visual-registry.json"),),
               ("python3 kernel/gen-visual-registry.py --check --selftest",),
               ("kernel/visual-registry.json",),
               ("missing asset", "unearned variant promotion"),
               ("no current build-bound golden", "scale/theme/locale/a11y/state/backend matrix incomplete"),
               "visual assets and BIOS/UEFI pairs are inventoried, not promoted as regression goldens"),
    "EV-023": ("PARTIAL_CURRENT", (("implementation", "kernel/accessibility-registry.json"),),
               ("python3 kernel/gen-accessibility-registry.py --check --selftest",),
               ("kernel/accessibility-registry.json",),
               ("invented semantic tree", "hidden assistive gap", "target overclaim"),
               ("9 capabilities missing", "0 complete target workflows"),
               "keyboard/focus primitives pass on host; semantic and assistive workflows remain absent"),
    "EV-024": ("PARTIAL_CURRENT", (("implementation", "kernel/security-registry.json"),),
               ("python3 kernel/gen-security-registry.py --check --selftest",),
               ("kernel/security-registry.json",),
               ("production overclaim", "hidden DMA risk", "invented DMA proof"),
               ("17 residual-risk claims open", "0 production-complete security claims"),
               "static/host/QEMU evidence exists for six limited areas; eleven security foundations are missing"),
    "EV-025": ("PARTIAL_CURRENT", (("plan", "docs/program/VALIDATION-RECEIPT.md"),),
               ("python3 tools/validate_master_program.py --self-test",),
               ("docs/program/VALIDATION-RECEIPT.md",), ("missing feature", "unknown contract"),
               ("no independent reviewer receipt exists for the active implementation batch",),
               "research plans were adversarially reviewed; active code promotion still needs independent rejection authority"),
    "EV-026": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/decision-ledger.json"),
                   ("plan", "docs/program/VALIDATION-RECEIPT.md")),
               ("python3 kernel/gen-decision-ledger.py --check --selftest",
                "python3 tools/validate_master_program.py"),
               ("kernel/decision-ledger.json",
                "kernel/docs/mp00-decision-ledger-2026-08-24.md"),
               ("missing decision", "unknown status", "invented source",
                "lost replacement", "deleted obsolete evidence", "hidden history gap"),
               ("system-wide decision inventory is incomplete",
                "no signed or independent approval", "no history viewer"),
               "19 high-impact decisions/reversals and all 47 legacy decision sections have exact normalized source semantics; repository-wide history, approval and the target viewer remain open"),
    "EV-027": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/release-notes.json"),
                   ("plan", "docs/program/VALIDATION-RECEIPT.md")),
               ("python3 kernel/gen-release-notes.py --check --selftest",),
               ("kernel/release-notes.json", "kernel/docs/UNRELEASED-CHANGELOG.md",
                "kernel/docs/mp00-release-notes-2026-08-24.md"),
               ("missing change", "invented release", "hidden release block",
                "missing known issue", "missing migration recovery",
                "deleted superseded history", "unknown release feature"),
               ("0 published entries", "no version/channel/signature",
                "public redistribution blocked", "migration inventory incomplete",
                "no previous signed rollback generation"),
               "17 source-bound change candidates, 12 blockers and 3 recovery paths are generated; this remains an unreleased development batch with zero published entries"),
    "EV-028": ("PARTIAL_CURRENT", (
                   ("implementation", "kernel/provenance-viewer.json"),
                   ("plan", "docs/program/VALIDATION-RECEIPT.md")),
               ("python3 kernel/gen-provenance-viewer.py --check --selftest",),
               ("kernel/provenance-viewer.json", "kernel/docs/provenance-viewer.html",
                "kernel/docs/mp00-provenance-viewer-2026-08-24.md"),
               ("missing artifact", "invented signature", "hidden license block",
                "invented app permission", "invented runtime route",
                "missing focus style", "external request", "missing HTML artifact"),
               ("no booted zlOS app route", "0 signatures",
                "0 per-app permission grants", "no live health stream",
                "no current screenshot or target accessibility receipt"),
               "self-contained host viewer projects 9 artifacts, 62 apps, 17 security claims and 11 health areas; it remains static and untrusted as a release attestation"),
    "KR-032": ("PARTIAL_CURRENT", (("implementation", "kernel/crash.c"),
                                     ("implementation", "kernel/idt.c"),
                                     ("implementation", "kernel/docs/receipts/cpu-fault-invalid-opcode-qemu-2026-08-23.json")),
               ("python3 kernel/verify-crash.py --run --no-build --selftest",
                "kernel/tests/host/crashtest"),
               ("kernel/kernel.elf", "kernel/zlOS.iso"),
               ("wrong vector", "wrong checksum", "wrong symbol", "stale artifact", "guest not halted"),
               ("no general-register capture", "no nested-fault recovery", "no user-fault offender-only containment"),
               "all CPU exception vectors have typed entry frames and QEMU proves the vector-6 stop path; nesting, spurious-interrupt ownership and recoverable user-fault containment remain unproved"),
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def phase_dependencies() -> dict[str, list[str]]:
    text = master.read(master.PROGRAM / "PHASES.md")
    result = {}
    for phase, raw in master.DEPENDENCY_ROW_RE.findall(text):
        result[phase] = [] if raw.strip() == "none" else [part.strip() for part in raw.split(",")]
    return result


def evidence_entry(kind: str, relative: str, evidence_root: Path) -> dict:
    root = ROOT if kind == "plan" else evidence_root
    path = root / relative
    if not path.is_file():
        raise ValueError(f"missing {kind} evidence: {path}")
    value = None
    if path.suffix == ".json":
        value = json.loads(path.read_text())
    return {
        "root": kind,
        "path": relative,
        "sha256": digest(path),
        "schema": value.get("schema") if isinstance(value, dict) else None,
        "result": value.get("result", value.get("outcome")) if isinstance(value, dict) else "SOURCE",
    }


def git_snapshot(path: Path) -> dict:
    def run(*args: str) -> str:
        return subprocess.check_output(args, cwd=path, text=True).strip()
    status = subprocess.check_output(
        ("git", "status", "--porcelain=v1", "-uall"), cwd=path, text=True
    )
    return {
        "path": str(path),
        "head": run("git", "rev-parse", "HEAD"),
        "branch": run("git", "branch", "--show-current"),
        "dirty": bool(status),
        "status_sha256": hashlib.sha256(status.encode()).hexdigest(),
    }


def validate_decision_feature_links(decisions: dict, feature_ids: set[str]) -> None:
    if decisions.get("schema") != "zlos.decision-ledger.v1" \
            or decisions.get("result") != "PASS_WITH_OPEN_GAPS":
        raise ValueError("decision ledger is absent or overpromoted")
    unknown = sorted({
        feature_id
        for record in decisions.get("records", [])
        for feature_id in record.get("features", [])
        if feature_id not in feature_ids
    })
    if unknown:
        raise ValueError("decision ledger references unknown feature IDs: " + ", ".join(unknown))


def validate_release_feature_links(release_notes: dict, feature_ids: set[str]) -> None:
    if release_notes.get("schema") != "zlos.release-notes.v1" \
            or release_notes.get("result") != "PASS_UNRELEASED_WITH_BLOCKERS":
        raise ValueError("release notes are absent or overpromoted")
    linked = [
        feature_id
        for section in ("changes", "known_issues")
        for record in release_notes.get(section, [])
        for feature_id in record.get("feature_ids", [])
    ]
    unknown = sorted(set(linked) - feature_ids)
    if unknown:
        raise ValueError("release notes reference unknown feature IDs: " + ", ".join(unknown))


def validate_provenance_feature_link(provenance: dict, feature_ids: set[str]) -> None:
    if provenance.get("schema") != "zlos.provenance-viewer.v1" \
            or provenance.get("result") != "PASS_STATIC_VIEWER_WITH_OPEN_GAPS":
        raise ValueError("provenance viewer is absent or overpromoted")
    if provenance.get("feature_id") not in feature_ids:
        raise ValueError("provenance viewer references an unknown feature ID")


def build(evidence_root: Path) -> dict:
    source_text = master.read(master.SOURCE)
    features = master.parse_features(source_text)
    master.validate_features(features)
    dependencies = phase_dependencies()
    joined = json.loads((evidence_root / "kernel/evidence-registry.json").read_text())
    if joined.get("result") != "PASS_WITH_OPEN_GAPS":
        raise ValueError("implementation evidence registry is absent or overpromoted")
    decisions = json.loads((evidence_root / "kernel/decision-ledger.json").read_text())
    release_notes = json.loads((evidence_root / "kernel/release-notes.json").read_text())
    provenance = json.loads((evidence_root / "kernel/provenance-viewer.json").read_text())
    feature_ids = {row["id"] for row in features}
    validate_decision_feature_links(decisions, feature_ids)
    validate_release_feature_links(release_notes, feature_ids)
    validate_provenance_feature_link(provenance, feature_ids)
    if joined.get("counts", {}).get("decision_records") != len(decisions["records"]) \
            or joined.get("open_gaps", {}).get("decision_legacy_semantics_open") \
            != decisions.get("coverage", {}).get("legacy_labels_remaining"):
        raise ValueError("joined decision-ledger evidence drift")
    if joined.get("counts", {}).get("release_change_candidates") \
            != release_notes.get("counts", {}).get("unreleased_change_candidates") \
            or joined.get("open_gaps", {}).get("release_publication_blocked") is not True:
        raise ValueError("joined release-note evidence drift")
    if joined.get("counts", {}).get("provenance_applications") \
            != provenance.get("counts", {}).get("applications") \
            or joined.get("open_gaps", {}).get("provenance_runtime_route_missing") is not True:
        raise ValueError("joined provenance-viewer evidence drift")
    rows = []
    for feature in features:
        phase, workstream = master.DOMAIN[feature["prefix"]]
        override = OVERRIDES.get(feature["id"])
        if override:
            status, refs, routes, artifacts, tests, gaps, weakest = override
            evidence = [evidence_entry(kind, path, evidence_root) for kind, path in refs]
            if feature["id"] == "EV-008":
                weakest = (
                    f"{joined['counts']['landing_gate_mandatory_seams']} mandatory landing seams "
                    "are guarded, not every repository command path"
                )
        else:
            status = "PLANNED_UNPROVED"
            routes, artifacts, tests, evidence = (), (), (), []
            gaps = ("no feature-specific implementation receipt is joined",)
            weakest = "the complete acceptance contract is planned but unproved"
        rows.append({
            "id": feature["id"],
            "feature": feature["feature"],
            "research_state": feature["state"],
            "primary_phase": phase,
            "phase_dependencies": dependencies[phase],
            "owner": workstream,
            "maturity": status,
            "acceptance_contract": feature["meaning"],
            "routes": list(routes),
            "artifacts": list(artifacts),
            "tests": list(tests),
            "evidence": evidence,
            "known_gaps": list(gaps),
            "rejected_claims": ["source/build/host/QEMU proof does not imply native-hardware completion"],
            "weakest_evidence": weakest,
        })
    counts = Counter(row["maturity"] for row in rows)
    return {
        "schema": "zlos.feature-status.v1",
        "result": "PASS_WITH_OPEN_GAPS",
        "source_catalogue_sha256": hashlib.sha256(source_text.encode()).hexdigest(),
        "feature_map_sha256": digest(ROOT / "docs/program/FEATURE-MAP.md"),
        "implementation_evidence_registry_sha256": digest(evidence_root / "kernel/evidence-registry.json"),
        "decision_ledger_sha256": digest(evidence_root / "kernel/decision-ledger.json"),
        "release_notes_sha256": digest(evidence_root / "kernel/release-notes.json"),
        "provenance_viewer_sha256": digest(evidence_root / "kernel/provenance-viewer.json"),
        "build_identity": joined["build_identity"],
        "implementation_tree": git_snapshot(evidence_root),
        "counts": {"total": len(rows), **dict(sorted(counts.items()))},
        "global_blockers": {
            "public_release_blocked": joined["open_gaps"]["public_release_blocked"],
            "source_snapshot_off_host_missing": joined["open_gaps"]["source_snapshot_off_host_missing"],
            "source_snapshot_signature_missing": joined["open_gaps"]["source_snapshot_signature_missing"],
            "whole_repository_snapshot_missing": joined["open_gaps"]["whole_repository_snapshot_missing"],
            "hermetic_toolchain_missing": joined["open_gaps"]["hermetic_toolchain_missing"],
            "signed_toolchain_attestation_missing": joined["open_gaps"]["signed_toolchain_attestation_missing"],
            "toolchain_source_archive_missing": joined["open_gaps"]["toolchain_source_archive_missing"],
            "dependency_offline_rebuild_missing": joined["open_gaps"]["dependency_offline_rebuild_missing"],
            "dependency_source_archives_missing": joined["open_gaps"]["dependency_source_archives_missing"],
            "per_object_provenance_receipts_missing": joined["open_gaps"]["per_object_provenance_receipts_missing"],
            "build_graph_scope_only_inputs": joined["open_gaps"]["build_graph_scope_only_inputs"],
            "future_build_graph_outputs_missing": joined["open_gaps"]["future_build_graph_outputs_missing"],
            "physical_exact_hash_artifacts": joined["open_gaps"]["physical_exact_hash_artifacts"],
            "failure_injection_open_families": joined["open_gaps"]["failure_injection_open_families"],
            "hostile_corpus_open_families": joined["open_gaps"]["hostile_corpus_open_families"],
            "performance_over_budget": joined["open_gaps"]["performance_over_budget"],
            "visual_unbound_assets": joined["open_gaps"]["visual_unbound_assets"],
            "accessibility_missing_capabilities": joined["open_gaps"]["accessibility_missing_capabilities"],
            "security_open_claims": joined["open_gaps"]["security_open_claims"],
            "decision_legacy_semantics_open": joined["open_gaps"]["decision_legacy_semantics_open"],
            "decision_system_inventory_missing": joined["open_gaps"]["decision_system_inventory_missing"],
            "decision_independent_approval_missing": joined["open_gaps"]["decision_independent_approval_missing"],
            "release_generation_missing": joined["open_gaps"]["release_generation_missing"],
            "release_publication_blocked": joined["open_gaps"]["release_publication_blocked"],
            "release_signature_missing": joined["open_gaps"]["release_signature_missing"],
            "release_published_entries_missing": joined["open_gaps"]["release_published_entries_missing"],
            "release_migration_inventory_incomplete": joined["open_gaps"]["release_migration_inventory_incomplete"],
            "release_previous_rollback_missing": joined["open_gaps"]["release_previous_rollback_missing"],
            "provenance_runtime_route_missing": joined["open_gaps"]["provenance_runtime_route_missing"],
            "provenance_current_screenshot_missing": joined["open_gaps"]["provenance_current_screenshot_missing"],
            "provenance_permissions_missing": joined["open_gaps"]["provenance_permissions_missing"],
            "provenance_signatures_missing": joined["open_gaps"]["provenance_signatures_missing"],
            "provenance_live_health_missing": joined["open_gaps"]["provenance_live_health_missing"],
            "provenance_target_accessibility_missing": joined["open_gaps"]["provenance_target_accessibility_missing"],
            "provenance_remote_auth_missing": joined["open_gaps"]["provenance_remote_auth_missing"],
            "provenance_public_release_missing": joined["open_gaps"]["provenance_public_release_missing"],
            "observability_open_capabilities": joined["open_gaps"]["observability_open_capabilities"],
        },
        "features": rows,
        "generator": {"path": "tools/gen_feature_status.py", "sha256": digest(Path(__file__).resolve())},
        "evidence_ceiling": "conservative plan-to-current-evidence join; unlisted current assets are not promoted",
    }


def selftest(value: dict, evidence_root: Path) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["features"].pop()
    mutations["missing-feature"] = missing
    duplicate = copy.deepcopy(value)
    duplicate["features"][-1]["id"] = duplicate["features"][0]["id"]
    mutations["duplicate-feature"] = duplicate
    dependency = copy.deepcopy(value)
    dependency["features"][100]["phase_dependencies"] = ["MP-99"]
    mutations["unknown-dependency"] = dependency
    promotion = copy.deepcopy(value)
    target = next(row for row in promotion["features"] if row["maturity"] == "PLANNED_UNPROVED")
    target["maturity"] = "PROVED_CURRENT"
    mutations["unproved-promotion"] = promotion
    hidden = copy.deepcopy(value)
    hidden["global_blockers"]["public_release_blocked"] = False
    mutations["hidden-release-block"] = hidden
    custody = copy.deepcopy(value)
    custody["global_blockers"]["source_snapshot_off_host_missing"] = False
    mutations["invented-source-custody"] = custody
    hermetic = copy.deepcopy(value)
    hermetic["global_blockers"]["hermetic_toolchain_missing"] = False
    mutations["invented-hermetic-toolchain"] = hermetic
    graph = copy.deepcopy(value)
    graph["global_blockers"]["build_graph_scope_only_inputs"] = 0
    mutations["hidden-build-graph-superset"] = graph
    identity = copy.deepcopy(value)
    identity["build_identity"] = "short"
    mutations["missing-build-identity"] = identity
    caught = []
    for name, mutant in mutations.items():
        try:
            master.validate_feature_status_value(mutant)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"feature-status selftest mutation escaped: {name}")
    decisions = json.loads((evidence_root / "kernel/decision-ledger.json").read_text())
    decision_mutant = copy.deepcopy(decisions)
    decision_mutant["records"][0]["features"] = ["EV-999"]
    feature_ids = {row["id"] for row in value["features"]}
    try:
        validate_decision_feature_links(decision_mutant, feature_ids)
    except ValueError:
        caught.append("unknown-decision-feature")
    else:
        raise ValueError("feature-status selftest mutation escaped: unknown-decision-feature")
    release_notes = json.loads((evidence_root / "kernel/release-notes.json").read_text())
    release_mutant = copy.deepcopy(release_notes)
    release_mutant["known_issues"][0]["feature_ids"] = ["EV-999"]
    try:
        validate_release_feature_links(release_mutant, feature_ids)
    except ValueError:
        caught.append("unknown-release-feature")
    else:
        raise ValueError("feature-status selftest mutation escaped: unknown-release-feature")
    provenance = json.loads((evidence_root / "kernel/provenance-viewer.json").read_text())
    provenance_mutant = copy.deepcopy(provenance)
    provenance_mutant["feature_id"] = "EV-999"
    try:
        validate_provenance_feature_link(provenance_mutant, feature_ids)
    except ValueError:
        caught.append("unknown-provenance-feature")
    else:
        raise ValueError("feature-status selftest mutation escaped: unknown-provenance-feature")
    print("feature-status selftest: caught " + ", ".join(caught))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--evidence-root", type=Path, default=DEFAULT_EVIDENCE_ROOT)
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        value = build(args.evidence_root.resolve())
        master.validate_feature_status_value(value)
        if args.selftest:
            selftest(value, args.evidence_root.resolve())
        if args.write:
            OUTPUT.write_text(json.dumps(value, indent=2) + "\n")
        elif json.loads(OUTPUT.read_text()) != value:
            raise ValueError("FEATURE-STATUS.json is stale; run --write and inspect the diff")
        print("feature-status: PASS_WITH_OPEN_GAPS: " + json.dumps(value["counts"], sort_keys=True))
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.CalledProcessError) as error:
        print(f"feature-status: FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
