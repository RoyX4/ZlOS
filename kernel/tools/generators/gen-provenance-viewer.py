#!/usr/bin/env python3
"""Generate a self-contained, read-only zlOS provenance viewer."""

from __future__ import annotations

import argparse
import copy
from collections import Counter
import hashlib
from html import escape
from html.parser import HTMLParser
import json
import os
from pathlib import Path
import tempfile


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
ROOT = KERNEL_ROOT.parent
METADATA = KERNEL_ROOT / "metadata"
JSON_OUTPUT = METADATA / "provenance-viewer.json"
HTML_OUTPUT = KERNEL_ROOT / "docs" / "provenance-viewer.html"
INPUTS = (
    "build-identity.json",
    "docs/receipts/source-snapshot-2026-08-24.json",
    "license-registry.json",
    "dependency-lock.json",
    "toolchain-manifest.json",
    "artifact-registry.json",
    "app-manifest.json",
    "app-evidence.json",
    "tests/host/test-inventory.json",
    "tests/host/test-run-receipt.json",
    "docs/receipts/benchmark-host-2026-08-23.json",
    "visual-registry.json",
    "accessibility-registry.json",
    "security-registry.json",
    "observability-registry.json",
    "decision-ledger.json",
    "release-notes.json",
)

OPEN_GAPS = {
    "runtime_zlos_app_route_missing": True,
    "current_screenshot_receipt_missing": True,
    "per_app_permission_grants_missing": True,
    "cryptographic_signatures_missing": True,
    "live_health_updates_missing": True,
    "target_accessibility_workflow_missing": True,
    "authenticated_remote_serving_missing": True,
    "public_release_missing": True,
    "current_artifact_snapshot_missing": False,
    "current_qemu_evidence_missing": False,
    "current_host_test_receipt_missing": True,
    "current_host_benchmark_missing": True,
}

TOKENS = {
    "canvas": "#111210",
    "panel": "#1a1c18",
    "raised": "#23261f",
    "line": "#3a3f33",
    "text": "#f0eee6",
    "muted": "#adb0a4",
    "pass": "#7bc47f",
    "warn": "#e3b341",
    "fail": "#ff6b5f",
    "info": "#6bb6c9",
    "focus": "#f4cf67",
    "space_1": "0.375rem",
    "space_2": "0.75rem",
    "space_3": "1.125rem",
    "space_4": "1.75rem",
    "radius": "0.375rem",
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


def artifact_rows(artifact: dict) -> list[dict]:
    return [{"name": name, **row} for name, row in sorted(artifact["artifacts"].items())]


def application_rows(manifest: dict, evidence: dict) -> list[dict]:
    evidence_by_id = {row["id"]: row for row in evidence["entries"]}
    rows = []
    for source in manifest["entries"]:
        runtime = evidence_by_id[source["id"]]
        rows.append({
            "id": source["id"],
            "name": source["name"],
            "kind": source["kind"],
            "migration_phase": source["migration_phase"],
            "source_evidence": source["evidence"],
            "shipped_manifest_member": runtime["claims"]["shipped_manifest_member"],
            "qemu_open": runtime["claims"]["qemu_open"],
            "qemu_first_draw": runtime["claims"]["qemu_first_draw"],
            "qemu_close_no_window_leak": runtime["claims"]["qemu_close_no_window_leak"],
            "workflow_complete": runtime["claims"]["workflow_complete"],
            "declared_permission_grants": [],
        })
    return rows


def security_rows(security: dict) -> list[dict]:
    return [{
        "id": row["id"],
        "status": row["status"],
        "protected_asset": row["protected_asset"],
        "threat": row["threat"],
        "enforcement_point": row["enforcement_point"],
        "residual_risk": row["residual_risk"],
        "evidence_ceiling": row["evidence_ceiling"],
    } for row in security["claims"]]


def health_rows(docs: dict[str, dict]) -> list[dict]:
    identity = docs["build-identity.json"]
    license_counts = docs["license-registry.json"]["counts"]
    tests = docs["tests/host/test-run-receipt.json"]["counts"]
    benchmark = docs["docs/receipts/benchmark-host-2026-08-23.json"]["counts"]
    visual = docs["visual-registry.json"]["counts"]
    accessibility = docs["accessibility-registry.json"]["counts"]
    security = docs["security-registry.json"]["counts"]
    observability = docs["observability-registry.json"]["counts"]
    return [
        {"area": "Build identity", "status": "PASS",
         "detail": f"{len(identity['source_files_sha256'])} declared inputs bind one exact build-input identity."},
        {"area": "Source custody", "status": "BLOCKED",
         "detail": "Exact local archive exists; off-host copies and signatures are zero."},
        {"area": "Licensing", "status": "BLOCKED",
         "detail": f"{license_counts['build_inputs']} build inputs lack an established redistribution grant."},
        {"area": "Host tests", "status": "HISTORICAL",
         "detail": f"Dated receipt: {tests['passed']} pass, {tests['skipped-hardware']} hardware skip, {tests['not-run']} not run; not current-build bound."},
        {"area": "Performance", "status": "HISTORICAL_REGRESSION",
         "detail": f"Dated receipt: {benchmark['over_budget']} of {benchmark['measurements']} host frame metrics exceed budget; not current-build bound."},
        {"area": "Visual proof", "status": "GAP",
         "detail": f"{visual['assets']} assets inventoried; {visual['current_build_bound']} current-build screenshot receipts."},
        {"area": "Accessibility", "status": "GAP",
         "detail": f"{accessibility['missing']} capabilities missing; {accessibility['complete_target_workflows']} complete target workflows."},
        {"area": "Security", "status": "BLOCKED",
         "detail": f"{security['missing']} claims missing; {security['production_complete']} production-complete."},
        {"area": "Observability", "status": "GAP",
         "detail": f"{observability['missing']} capabilities missing; {observability['durable_crash_receipts']} durable crash receipts."},
        {"area": "Release", "status": "BLOCKED",
         "detail": "Unreleased development batch: no version, channel, signature or publication."},
        {"area": "Complete landing gate", "status": "UNVERIFIED",
         "detail": "The latest attempt was interrupted; no post-incident aggregate green exists."},
    ]


def build_model() -> dict:
    docs = documents()
    build_identity = docs["build-identity.json"]
    identity = build_identity["identity_sha256"]
    snapshot = docs["docs/receipts/source-snapshot-2026-08-24.json"]
    license_registry = docs["license-registry.json"]
    dependency = docs["dependency-lock.json"]
    toolchain = docs["toolchain-manifest.json"]
    artifact = docs["artifact-registry.json"]
    app_manifest = docs["app-manifest.json"]
    app_evidence = docs["app-evidence.json"]
    tests = docs["tests/host/test-run-receipt.json"]
    release = docs["release-notes.json"]
    decisions = docs["decision-ledger.json"]
    security = docs["security-registry.json"]
    visual = docs["visual-registry.json"]
    accessibility = docs["accessibility-registry.json"]
    observability = docs["observability-registry.json"]
    current_identities = (
        snapshot["build_identity"], license_registry["build_identity"],
        dependency["build_identity"], toolchain["build_identity"],
        release["build_identity"], decisions["build_identity"],
        security["build_identity"], visual["build_identity"],
        accessibility["build_identity"], observability["build_identity"],
    )
    if any(item != identity for item in current_identities):
        raise ValueError("provenance current inputs disagree on build identity")
    evidence_inputs = {
        "kernel/metadata/artifact-registry.json": artifact["build_identity"]["id"],
        "kernel/metadata/app-evidence.json": app_evidence["shipped_build_identity"]["id"],
        "kernel/docs/receipts/benchmark-host-2026-08-23.json": docs[
            "docs/receipts/benchmark-host-2026-08-23.json"]["build_identity"],
    }
    if any(len(item) != 64 for item in evidence_inputs.values()) \
            or evidence_inputs["kernel/metadata/artifact-registry.json"] != identity \
            or evidence_inputs["kernel/metadata/app-evidence.json"] != identity \
            or evidence_inputs["kernel/docs/receipts/benchmark-host-2026-08-23.json"] == identity:
        raise ValueError("provenance evidence boundary is invalid")
    apps = application_rows(app_manifest, app_evidence)
    artifacts = artifact_rows(artifact)
    security_claims = security_rows(security)
    health = health_rows(docs)
    signatures = {
        "release_notes_signed": release["release"]["signed"],
        "source_snapshot_signed": snapshot["open_gaps"]["signed_attestation"],
        "artifact_signatures": 0,
        "toolchain_attestations": 0,
        "package_signatures": 0,
    }
    model = {
        "schema": "zlos.provenance-viewer.v1",
        "result": "PASS_STATIC_VIEWER_WITH_OPEN_GAPS",
        "feature_id": "EV-028",
        "build_identity": identity,
        "viewer": {
            "format": "SELF_CONTAINED_HTML",
            "read_only": True,
            "locale": "en",
            "visual_direction": "flight-recorder evidence room",
            "external_requests": 0,
            "search": True,
            "status_filters": ["ALL", "PASS", "PARTIAL", "GAP", "BLOCKED", "UNVERIFIED", "REGRESSION"],
            "keyboard_navigation": True,
            "reduced_motion": True,
            "color_is_not_sole_status_signal": True,
        },
        "tokens": TOKENS,
        "inputs": [{
            "path": input_path(path).relative_to(ROOT).as_posix(),
            "sha256": sha256(input_path(path)),
            "schema": docs[path].get("schema"),
        } for path in INPUTS],
        "evidence_bindings": [{
            "path": path,
            "subject_build_identity": subject,
            "current_build_bound": subject == identity,
            "evidence_ceiling": ("current build-bound evidence" if subject == identity else
                                 "dated evidence for its named subject build only"),
        } for path, subject in evidence_inputs.items()],
        "identity": {
            "head": build_identity["git"]["head"],
            "branch": build_identity["git"]["branch"],
            "dirty": build_identity["git"]["dirty"],
            "release_state": release["release"]["state"],
            "release_version": release["release"]["version"],
            "release_channel": release["release"]["channel"],
        },
        "origin": {
            "build_input_count": snapshot["counts"]["archived_inputs"],
            "archive_bytes": snapshot["archive"]["bytes"],
            "archive_sha256": snapshot["archive"]["sha256"],
            "off_host_copies": snapshot["open_gaps"]["off_host_copies"],
            "whole_repository_snapshot": snapshot["open_gaps"]["whole_repository_snapshot"],
            "dependency_commands": len(dependency["commands"]),
            "firmware_blobs": len(dependency["firmware"]),
            "toolchain_lanes": [row["id"] for row in toolchain["target_lanes"]],
        },
        "licensing": {
            "build_inputs": license_registry["counts"]["build_inputs"],
            "declared_license_files": license_registry["counts"]["declared_license_files"],
            "inputs_with_established_redistribution_grant": license_registry["counts"]["inputs_with_established_redistribution_grant"],
            "public_release_blocked": license_registry["public_release_blocked"],
        },
        "signatures": signatures,
        "tests": {
            **tests["counts"],
            "qemu_boot_routes": len(artifact["boot_routes"]),
            "app_lifecycle_qemu_proved": app_evidence["counts"]["with_qemu_open_ready_close"],
            "qemu_current_build_bound": True,
            "host_current_build_bound": False,
        },
        "health": health,
        "artifacts": artifacts,
        "applications": apps,
        "security_and_permissions": security_claims,
        "changes": release["changes"],
        "counts": {
            "inputs": len(INPUTS),
            "health_areas": len(health),
            "artifacts": len(artifacts),
            "applications": len(apps),
            "applications_with_declared_permission_grants": sum(
                bool(row["declared_permission_grants"]) for row in apps),
            "security_claims": len(security_claims),
            "change_candidates": len(release["changes"]),
            "current_decisions": decisions["counts"]["status"]["ACTIVE"]
            + decisions["counts"]["status"]["QUALIFIED"],
            "cryptographic_signatures": sum(
                int(item) for item in signatures.values() if isinstance(item, bool)
            ) + sum(item for item in signatures.values() if isinstance(item, int)
                  and not isinstance(item, bool)),
        },
        "open_gaps": OPEN_GAPS,
        "generator": {
            "path": "kernel/tools/generators/gen-provenance-viewer.py",
            "sha256": sha256(Path(__file__).resolve()),
        },
        "evidence_ceiling": "current static provenance projection with current artifact/QEMU evidence and historical host metrics; not a booted zlOS provenance app, live monitor, authenticated remote service or signed release portal",
        "weakest_link": "permissions are security claim state rather than admitted per-app grants, signatures are absent, and the viewer has no current visual or target accessibility receipt",
    }
    validate_model(model, docs)
    return model


def validate_model(value: dict, docs: dict[str, dict]) -> None:
    if value.get("schema") != "zlos.provenance-viewer.v1" \
            or value.get("result") != "PASS_STATIC_VIEWER_WITH_OPEN_GAPS":
        raise ValueError("wrong provenance-viewer schema/result")
    identity = docs["build-identity.json"]["identity_sha256"]
    if value.get("build_identity") != identity:
        raise ValueError("stale provenance-viewer identity")
    if value.get("viewer") != {
        "format": "SELF_CONTAINED_HTML", "read_only": True, "locale": "en",
        "visual_direction": "flight-recorder evidence room", "external_requests": 0,
        "search": True,
        "status_filters": ["ALL", "PASS", "PARTIAL", "GAP", "BLOCKED", "UNVERIFIED", "REGRESSION"],
        "keyboard_navigation": True, "reduced_motion": True,
        "color_is_not_sole_status_signal": True,
    }:
        raise ValueError("viewer interaction/accessibility contract drift")
    if value.get("tokens") != TOKENS:
        raise ValueError("viewer design tokens drift")
    expected_inputs = [{
        "path": input_path(path).relative_to(ROOT).as_posix(), "sha256": sha256(input_path(path)),
        "schema": docs[path].get("schema"),
    } for path in INPUTS]
    if value.get("inputs") != expected_inputs:
        raise ValueError("viewer input closure drift")
    expected_bindings = [
        ("kernel/metadata/artifact-registry.json", docs["artifact-registry.json"]["build_identity"]["id"], True),
        ("kernel/metadata/app-evidence.json", docs["app-evidence.json"]["shipped_build_identity"]["id"], True),
        ("kernel/docs/receipts/benchmark-host-2026-08-23.json",
         docs["docs/receipts/benchmark-host-2026-08-23.json"]["build_identity"], False),
    ]
    if value.get("evidence_bindings") != [{
            "path": path, "subject_build_identity": subject,
            "current_build_bound": current,
            "evidence_ceiling": ("current build-bound evidence" if current else
                                 "dated evidence for its named subject build only"),
    } for path, subject, current in expected_bindings]:
        raise ValueError("viewer evidence binding drift")
    if value.get("artifacts") != artifact_rows(docs["artifact-registry.json"]):
        raise ValueError("viewer artifact projection drift")
    if value.get("applications") != application_rows(
            docs["app-manifest.json"], docs["app-evidence.json"]):
        raise ValueError("viewer application projection drift")
    if value.get("security_and_permissions") != security_rows(docs["security-registry.json"]):
        raise ValueError("viewer security projection drift")
    if value.get("health") != health_rows(docs):
        raise ValueError("viewer health projection drift")
    if value.get("changes") != docs["release-notes.json"]["changes"]:
        raise ValueError("viewer change projection drift")
    if value.get("tests", {}).get("qemu_current_build_bound") is not True \
            or value.get("tests", {}).get("host_current_build_bound") is not False:
        raise ValueError("viewer test evidence binding drift")
    if value.get("licensing", {}).get("public_release_blocked") is not True:
        raise ValueError("viewer hid release licensing block")
    if value.get("signatures") != {
        "release_notes_signed": False, "source_snapshot_signed": False,
        "artifact_signatures": 0, "toolchain_attestations": 0,
        "package_signatures": 0,
    }:
        raise ValueError("viewer invented cryptographic signatures")
    counts = value.get("counts", {})
    if counts != {
        "inputs": 17, "health_areas": 11, "artifacts": 9,
        "applications": 62, "applications_with_declared_permission_grants": 0,
        "security_claims": 17, "change_candidates": 17,
        "current_decisions": 17, "cryptographic_signatures": 0,
    }:
        raise ValueError("viewer counts drift")
    if value.get("open_gaps") != OPEN_GAPS:
        raise ValueError("viewer gaps were hidden")
    if len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("missing viewer generator identity")


def status_class(status: str) -> str:
    normalized = status.lower().replace("_", "-")
    if "pass" in normalized or "proved" in normalized:
        return "pass"
    if "block" in normalized or "missing" in normalized:
        return "blocked"
    if "regression" in normalized or "fail" in normalized:
        return "regression"
    if "partial" in normalized:
        return "partial"
    if "unverified" in normalized:
        return "unverified"
    return "gap"


def badge(status: str) -> str:
    return f'<span class="badge {status_class(status)}">{escape(status)}</span>'


def render_html(value: dict) -> str:
    identity = value["identity"]
    origin = value["origin"]
    licensing = value["licensing"]
    tests = value["tests"]
    health_rows_html = "\n".join(
        '<tr class="filterable" data-status="{state}" data-search="{search}">'
        '<th scope="row">{area}</th><td>{badge}</td><td>{detail}</td></tr>'.format(
            state=status_class(row["status"]),
            search=escape((row["area"] + " " + row["detail"]).lower(), quote=True),
            area=escape(row["area"]), badge=badge(row["status"]),
            detail=escape(row["detail"]),
        ) for row in value["health"]
    )
    artifact_html = "\n".join(
        '<tr class="filterable" data-status="unverified" data-search="{search}">'
        '<th scope="row">{name}</th><td><code>{sha}</code></td><td>{size}</td>'
        '<td>{proof}</td><td>{physical}</td></tr>'.format(
            search=escape((row["name"] + " " + row["description"]).lower(), quote=True),
            name=escape(row["name"]), sha=escape(row["sha256"][:16] + "…"),
            size=f"{row['bytes']:,}", proof=escape(row["proof_state"]),
            physical=badge(row["physical_hardware"]),
        ) for row in value["artifacts"]
    )
    app_html = "\n".join(
        '<tr class="filterable" data-status="{state}" data-search="{search}">'
        '<th scope="row">{id}</th><td>{name}</td><td>{kind}</td><td>{phase}</td>'
        '<td>{route}</td><td>{permissions}</td></tr>'.format(
            state="pass" if row["qemu_open"] and row["qemu_first_draw"] else "gap",
            search=escape((str(row["id"]) + " " + row["name"] + " " + row["kind"]).lower(), quote=True),
            id=row["id"], name=escape(row["name"]), kind=escape(row["kind"]),
            phase=escape(row["migration_phase"]),
            route=badge("QEMU_OPEN_DRAW" if row["qemu_open"] and row["qemu_first_draw"] else "GAP"),
            permissions=badge("MISSING"),
        ) for row in value["applications"]
    )
    security_html = "\n".join(
        '<tr class="filterable" data-status="{state}" data-search="{search}">'
        '<th scope="row">{id}</th><td>{status}</td><td>{asset}</td><td>{point}</td>'
        '<td>{risk}</td></tr>'.format(
            state=status_class(row["status"]),
            search=escape((row["id"] + " " + row["protected_asset"] + " "
                           + row["residual_risk"]).lower(), quote=True),
            id=escape(row["id"]), status=badge(row["status"]),
            asset=escape(row["protected_asset"]), point=escape(row["enforcement_point"]),
            risk=escape(row["residual_risk"]),
        ) for row in value["security_and_permissions"]
    )
    changes_html = "\n".join(
        '<tr class="filterable" data-status="partial" data-search="{search}">'
        '<th scope="row">{id}</th><td>{audience}</td><td>{features}</td>'
        '<td>{summary}</td><td>{compatibility}</td></tr>'.format(
            search=escape((row["decision_id"] + " " + row["summary"] + " "
                           + " ".join(row["feature_ids"])).lower(), quote=True),
            id=escape(row["decision_id"]), audience=escape(row["audience"]),
            features=escape(", ".join(row["feature_ids"])),
            summary=escape(row["summary"]), compatibility=escape(row["compatibility"]),
        ) for row in value["changes"]
    )
    token_css = "\n".join(f"      --{key.replace('_', '-')}: {item};"
                           for key, item in value["tokens"].items())
    return f'''<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="Content-Security-Policy" content="default-src 'none'; style-src 'unsafe-inline'; script-src 'unsafe-inline'; img-src data:">
  <title>zlOS provenance room — unreleased</title>
  <style>
    :root {{
{token_css}
      color-scheme: dark;
    }}
    * {{ box-sizing: border-box; }}
    html {{ scroll-behavior: smooth; }}
    body {{ margin: 0; background: var(--canvas); color: var(--text); font: 15px/1.55 system-ui, sans-serif; }}
    a {{ color: var(--info); }}
    a:focus-visible, button:focus-visible, input:focus-visible {{ outline: 3px solid var(--focus); outline-offset: 3px; }}
    .skip {{ position: fixed; left: var(--space-2); top: -5rem; z-index: 20; background: var(--focus); color: #111; padding: var(--space-2); }}
    .skip:focus {{ top: var(--space-2); }}
    .shell {{ display: grid; grid-template-columns: 17rem minmax(0, 1fr); min-height: 100vh; }}
    .rail {{ position: sticky; top: 0; height: 100vh; border-right: 1px solid var(--line); padding: var(--space-4); background: #151713; }}
    .kicker, .eyebrow {{ color: var(--warn); font: 700 0.72rem/1.2 ui-monospace, monospace; letter-spacing: 0.13em; text-transform: uppercase; }}
    .rail h1 {{ font: 600 1.55rem/1.1 Georgia, serif; margin: var(--space-2) 0 var(--space-4); }}
    .rail nav a {{ display: block; padding: var(--space-2) 0; border-bottom: 1px solid #292c25; text-decoration: none; color: var(--muted); }}
    .rail nav a:hover {{ color: var(--text); }}
    .identity {{ margin-top: var(--space-4); font: 0.72rem/1.5 ui-monospace, monospace; color: var(--muted); overflow-wrap: anywhere; }}
    main {{ min-width: 0; padding: clamp(1rem, 3vw, 3rem); }}
    .warning {{ border: 1px solid var(--fail); border-left-width: 0.5rem; padding: var(--space-3); background: #261917; font-weight: 700; }}
    .hero {{ display: grid; grid-template-columns: minmax(0, 2fr) minmax(16rem, 1fr); gap: var(--space-4); margin: var(--space-4) 0; }}
    .hero h2 {{ font: 600 clamp(2rem, 5vw, 4.8rem)/0.95 Georgia, serif; letter-spacing: -0.035em; margin: 0 0 var(--space-3); max-width: 13ch; }}
    .lede {{ color: var(--muted); max-width: 65ch; font-size: 1.05rem; }}
    .status-stack {{ border-top: 1px solid var(--line); }}
    .status-stack div {{ display: flex; justify-content: space-between; gap: 1rem; padding: var(--space-2) 0; border-bottom: 1px solid var(--line); }}
    section {{ margin: 4.5rem 0; scroll-margin-top: 1rem; }}
    section h2 {{ font: 600 2rem/1.1 Georgia, serif; margin: 0 0 var(--space-2); }}
    .section-note {{ color: var(--muted); max-width: 72ch; }}
    .controls {{ position: sticky; top: 0; z-index: 5; display: flex; flex-wrap: wrap; gap: var(--space-2); align-items: center; padding: var(--space-2); margin: var(--space-4) 0; border: 1px solid var(--line); background: rgba(17,18,16,0.96); }}
    .controls label {{ font-weight: 700; }}
    .controls input {{ min-width: min(26rem, 100%); flex: 1; color: var(--text); background: var(--panel); border: 1px solid var(--line); border-radius: var(--radius); padding: 0.65rem 0.8rem; }}
    button {{ color: var(--text); background: var(--raised); border: 1px solid var(--line); border-radius: var(--radius); padding: 0.55rem 0.75rem; cursor: pointer; }}
    button[aria-pressed="true"] {{ border-color: var(--focus); color: var(--focus); }}
    .table-wrap {{ overflow-x: auto; border: 1px solid var(--line); border-radius: var(--radius); }}
    table {{ width: 100%; border-collapse: collapse; background: var(--panel); }}
    caption {{ text-align: left; color: var(--muted); padding: var(--space-2); }}
    th, td {{ padding: 0.7rem 0.8rem; border-bottom: 1px solid #30342b; text-align: left; vertical-align: top; }}
    thead th {{ color: var(--warn); font: 700 0.72rem/1.2 ui-monospace, monospace; text-transform: uppercase; letter-spacing: 0.08em; background: #20231d; }}
    tbody th {{ font-family: ui-monospace, monospace; }}
    tbody tr:hover {{ background: #24271f; }}
    code {{ font-family: ui-monospace, monospace; color: #d8ddd0; }}
    .badge {{ display: inline-block; border: 1px solid currentColor; border-radius: 99rem; padding: 0.12rem 0.45rem; font: 700 0.68rem/1.3 ui-monospace, monospace; white-space: nowrap; }}
    .badge.pass {{ color: var(--pass); }} .badge.partial, .badge.gap, .badge.unverified {{ color: var(--warn); }}
    .badge.blocked, .badge.regression {{ color: var(--fail); }}
    .facts {{ display: grid; grid-template-columns: minmax(12rem, 1fr) minmax(0, 2fr); border-top: 1px solid var(--line); }}
    .facts dt, .facts dd {{ margin: 0; padding: var(--space-2); border-bottom: 1px solid var(--line); }}
    .facts dt {{ color: var(--muted); font-family: ui-monospace, monospace; }}
    .empty {{ padding: var(--space-4); color: var(--muted); text-align: center; }}
    [hidden] {{ display: none !important; }}
    @media (max-width: 850px) {{
      .shell {{ display: block; }} .rail {{ position: relative; width: auto; height: auto; border-right: 0; border-bottom: 1px solid var(--line); }}
      .rail nav {{ display: flex; flex-wrap: wrap; gap: 0 var(--space-3); }} .hero {{ grid-template-columns: 1fr; }} .facts {{ grid-template-columns: 1fr; }}
    }}
    @media (prefers-reduced-motion: reduce) {{ *, *::before, *::after {{ scroll-behavior: auto !important; transition-duration: 0.01ms !important; }} }}
  </style>
</head>
<body>
  <a class="skip" href="#main">Skip to provenance content</a>
  <div class="shell">
    <aside class="rail">
      <div class="kicker">zlOS / evidence room</div>
      <h1>Provenance, without promotion</h1>
      <nav aria-label="Provenance sections">
        <a href="#overview">Overview</a><a href="#health">Current health</a>
        <a href="#artifacts">Artifacts</a><a href="#applications">Applications</a>
        <a href="#security">Permissions &amp; security</a><a href="#changes">Changes</a>
      </nav>
      <div class="identity">BUILD<br>{escape(value['build_identity'])}</div>
    </aside>
    <main id="main">
      <div class="warning" role="status">UNRELEASED DEVELOPMENT BUILD · DIRTY TREE · PUBLIC RELEASE BLOCKED</div>
      <section id="overview" class="hero" aria-labelledby="overview-title">
        <div><div class="eyebrow">Exact identity / explicit gaps</div>
          <h2 id="overview-title">What these bytes actually prove.</h2>
          <p class="lede">A read-only projection of the current manifests. It shows version and origin, licensing and signatures, tests, security authority gaps, artifacts, applications and unreleased changes. It never converts host, QEMU or source evidence into physical completion.</p>
        </div>
        <div class="status-stack" aria-label="Release status">
          <div><span>State</span>{badge(identity['release_state'])}</div>
          <div><span>License</span>{badge('BLOCKED')}</div>
          <div><span>Signatures</span>{badge('0 PRESENT')}</div>
          <div><span>Physical hashes</span>{badge('0 PROVED')}</div>
        </div>
      </section>
      <dl class="facts">
        <dt>Commit</dt><dd><code>{escape(identity['head'])}</code></dd>
        <dt>Branch / dirty</dt><dd>{escape(identity['branch'])} / {str(identity['dirty']).lower()}</dd>
        <dt>Source archive</dt><dd>{origin['build_input_count']} inputs · {origin['archive_bytes']:,} bytes · <code>{escape(origin['archive_sha256'])}</code></dd>
        <dt>Custody</dt><dd>{origin['off_host_copies']} off-host copies · signed={str(value['signatures']['source_snapshot_signed']).lower()}</dd>
        <dt>Licensing</dt><dd>{licensing['inputs_with_established_redistribution_grant']} / {licensing['build_inputs']} inputs have an established redistribution grant</dd>
        <dt>Tests</dt><dd>{tests['passed']} host pass · {tests['skipped-hardware']} hardware skip · {tests['not-run']} not run · {tests['qemu_boot_routes']} QEMU boot routes</dd>
      </dl>
      <div class="controls" aria-label="Table filters">
        <label for="search">Search evidence</label><input id="search" type="search" autocomplete="off" placeholder="artifact, app, claim, change…">
        {''.join(f'<button type="button" data-filter="{item.lower()}" aria-pressed="{str(item == "ALL").lower()}" aria-controls="evidence-tables">{item}</button>' for item in value['viewer']['status_filters'])}
        <span id="result-count" aria-live="polite"></span>
      </div>
      <div id="evidence-tables">
        <section id="health" aria-labelledby="health-title"><h2 id="health-title">Current health</h2><p class="section-note">Status words remain visible; color is only reinforcement.</p>
          <div class="table-wrap"><table><caption>Eleven evidence areas for this exact build</caption><thead><tr><th>Area</th><th>Status</th><th>Evidence boundary</th></tr></thead><tbody>{health_rows_html}</tbody></table></div>
        </section>
        <section id="artifacts" aria-labelledby="artifacts-title"><h2 id="artifacts-title">Artifacts</h2><p class="section-note">Byte identity and QEMU routes do not imply exact physical-hardware execution.</p>
          <div class="table-wrap"><table><caption>Nine current reproducible artifacts</caption><thead><tr><th>Name</th><th>SHA-256</th><th>Bytes</th><th>Proof</th><th>Physical</th></tr></thead><tbody>{artifact_html}</tbody></table></div>
        </section>
        <section id="applications" aria-labelledby="apps-title"><h2 id="apps-title">Applications</h2><p class="section-note">All 62 identities have QEMU lifecycle evidence. Per-app permission grants are still absent and shown as missing.</p>
          <div class="table-wrap"><table><caption>Application identity, route and permission state</caption><thead><tr><th>ID</th><th>Name</th><th>Kind</th><th>Migration</th><th>Route proof</th><th>Permissions</th></tr></thead><tbody>{app_html}</tbody></table></div>
        </section>
        <section id="security" aria-labelledby="security-title"><h2 id="security-title">Permissions &amp; security</h2><p class="section-note">This is claim and residual-risk state, not an invented permissions dashboard.</p>
          <div class="table-wrap"><table><caption>Seventeen security claims and their enforcement ceilings</caption><thead><tr><th>Claim</th><th>Status</th><th>Protected asset</th><th>Enforcement</th><th>Residual risk</th></tr></thead><tbody>{security_html}</tbody></table></div>
        </section>
        <section id="changes" aria-labelledby="changes-title"><h2 id="changes-title">Unreleased changes</h2><p class="section-note">Source-bound candidates only. Published entries: zero.</p>
          <div class="table-wrap"><table><caption>Current decisions projected as unreleased change candidates</caption><thead><tr><th>Decision</th><th>Audience</th><th>Features</th><th>Change</th><th>Compatibility</th></tr></thead><tbody>{changes_html}</tbody></table></div>
        </section>
      </div>
      <p id="no-results" class="empty" hidden>No evidence rows match this search and status filter.</p>
    </main>
  </div>
  <script>
    (() => {{
      const input = document.querySelector('#search');
      const buttons = [...document.querySelectorAll('[data-filter]')];
      const rows = [...document.querySelectorAll('.filterable')];
      const count = document.querySelector('#result-count');
      const empty = document.querySelector('#no-results');
      let filter = 'all';
      const apply = () => {{
        const query = input.value.trim().toLowerCase();
        let visible = 0;
        rows.forEach((row) => {{
          const matchesText = !query || row.dataset.search.includes(query);
          const matchesState = filter === 'all' || row.dataset.status === filter;
          row.hidden = !(matchesText && matchesState);
          if (!row.hidden) visible += 1;
        }});
        count.textContent = `${{visible}} of ${{rows.length}} rows visible`;
        empty.hidden = visible !== 0;
      }};
      input.addEventListener('input', apply);
      buttons.forEach((button) => button.addEventListener('click', () => {{
        filter = button.dataset.filter;
        buttons.forEach((item) => item.setAttribute('aria-pressed', String(item === button)));
        apply();
      }}));
      apply();
    }})();
  </script>
</body>
</html>
'''


class StructureParser(HTMLParser):
    def __init__(self) -> None:
        super().__init__()
        self.tags = Counter()
        self.ids = set()
        self.lang = None

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        self.tags[tag] += 1
        values = dict(attrs)
        if values.get("id"):
            self.ids.add(values["id"])
        if tag == "html":
            self.lang = values.get("lang")


def validate_html(text: str, value: dict) -> None:
    required = (
        '<html lang="en">', 'class="skip"', '<main id="main">',
        'aria-label="Provenance sections"', 'aria-live="polite"',
        ':focus-visible', '@media (prefers-reduced-motion: reduce)',
        'Content-Security-Policy', 'UNRELEASED DEVELOPMENT BUILD',
    )
    missing = [item for item in required if item not in text]
    if missing:
        raise ValueError("viewer HTML missing required semantics: " + ", ".join(missing))
    forbidden = ("http://", "https://", "linear-gradient", "radial-gradient",
                 "innerHTML", "document.write", "fetch(")
    present = [item for item in forbidden if item in text]
    if present:
        raise ValueError("viewer HTML contains forbidden external/unsafe pattern: " + ", ".join(present))
    for row in value["artifacts"]:
        if escape(row["name"]) not in text:
            raise ValueError(f"viewer HTML omitted artifact {row['name']}")
    for row in value["applications"]:
        if escape(row["name"]) not in text:
            raise ValueError(f"viewer HTML omitted application {row['name']}")
    for row in value["security_and_permissions"]:
        if escape(row["id"]) not in text:
            raise ValueError(f"viewer HTML omitted security claim {row['id']}")
    for row in value["changes"]:
        if escape(row["decision_id"]) not in text:
            raise ValueError(f"viewer HTML omitted change {row['decision_id']}")
    parser = StructureParser()
    parser.feed(text)
    required_ids = {"main", "overview", "health", "artifacts", "applications",
                    "security", "changes", "search", "result-count", "no-results"}
    if parser.lang != "en" or not required_ids <= parser.ids:
        raise ValueError("viewer HTML landmark/identity structure drift")
    if parser.tags["main"] != 1 or parser.tags["nav"] != 1 \
            or parser.tags["table"] != 5 or parser.tags["button"] != 7 \
            or parser.tags["input"] != 1 or parser.tags["script"] != 1:
        raise ValueError("viewer HTML component count drift")


def selftest(value: dict, docs: dict[str, dict], html_text: str) -> None:
    mutations = {}
    artifact = copy.deepcopy(value)
    artifact["artifacts"].pop()
    mutations["missing-artifact"] = artifact
    signature = copy.deepcopy(value)
    signature["signatures"]["release_notes_signed"] = True
    mutations["invented-signature"] = signature
    license_mutant = copy.deepcopy(value)
    license_mutant["licensing"]["public_release_blocked"] = False
    mutations["hidden-license-block"] = license_mutant
    permission = copy.deepcopy(value)
    permission["applications"][0]["declared_permission_grants"] = ["ALL"]
    mutations["invented-app-permission"] = permission
    gap = copy.deepcopy(value)
    gap["open_gaps"]["runtime_zlos_app_route_missing"] = False
    mutations["invented-runtime-route"] = gap
    binding = copy.deepcopy(value)
    binding["evidence_bindings"][0]["current_build_bound"] = False
    mutations["lost-current-artifact-proof"] = binding
    caught = []
    for name, mutant in mutations.items():
        try:
            validate_model(mutant, docs)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"provenance-viewer mutation escaped: {name}")
    html_mutations = {
        "missing-focus-style": html_text.replace(":focus-visible", ":focus-removed"),
        "external-request": html_text.replace("</head>", '<link href="https://example.invalid/x.css"></head>', 1),
        "missing-html-artifact": html_text.replace(escape(value["artifacts"][0]["name"]), "OMITTED", 1),
    }
    for name, mutant in html_mutations.items():
        try:
            validate_html(mutant, value)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"provenance-viewer HTML mutation escaped: {name}")
    print("provenance-viewer selftest: caught " + ", ".join(caught))


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
        value = build_model()
        html_text = render_html(value)
        validate_html(html_text, value)
        if args.selftest:
            selftest(value, docs, html_text)
        json_text = json.dumps(value, indent=2) + "\n"
        if args.write:
            write_atomic(JSON_OUTPUT, json_text)
            write_atomic(HTML_OUTPUT, html_text)
        elif not JSON_OUTPUT.is_file() or JSON_OUTPUT.read_text() != json_text \
                or not HTML_OUTPUT.is_file() or HTML_OUTPUT.read_text() != html_text:
            raise ValueError("provenance-viewer outputs are missing or stale")
        print(
            "provenance-viewer: PASS_STATIC_VIEWER_WITH_OPEN_GAPS: "
            f"{value['counts']['artifacts']} artifacts, "
            f"{value['counts']['applications']} apps, "
            f"{value['counts']['security_claims']} security claims, "
            f"{value['counts']['cryptographic_signatures']} signatures"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"provenance-viewer: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
