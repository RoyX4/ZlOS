#!/usr/bin/env python3
"""Generate a normalized, attributable decision/reversal/deprecation ledger."""

from __future__ import annotations

import argparse
import copy
from datetime import date
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile


HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
OUTPUT = HERE / "decision-ledger.json"
LEGACY_SOURCE = "kernel/docs/DECISIONS.md"
STATUSES = {"ACTIVE", "QUALIFIED", "SUPERSEDED"}
ACTIONS = {"DECISION", "MIGRATION", "DEPRECATION", "REVERSAL",
           "CORRECTION", "OPERATIONS"}


RECORDS = (
    {
        "id": "DEC-0001", "date": "2026-08-17", "action": "DECISION",
        "status": "ACTIVE", "features": ["ZL-043", "GR-007"],
        "decision": "Keep low-level window mechanisms in C and desktop policy in zl until the language/runtime can own the required structures safely.",
        "rationale": "The current kernel zl subset cannot express the required window list and mechanism boundary.",
        "source": "kernel/docs/DECISIONS.md",
        "needle": "**1. Mechanism in C, policy in zl.**", "legacy_labels": ["1"],
        "supersedes": [], "replaced_by": None, "obsolete_evidence": [],
    },
    {
        "id": "DEC-0002", "date": "2026-08-17", "action": "DECISION",
        "status": "SUPERSEDED", "features": ["GR-015", "GR-020"],
        "decision": "Treat two framebuffer functions as the complete clipping boundary.",
        "rationale": "The original source inspection missed three direct back-buffer writers.",
        "source": "kernel/docs/DECISIONS.md",
        "needle": "**6. The clip rectangle is the keystone, and it is ~~two functions~~ FIVE.**",
        "legacy_labels": ["6"], "supersedes": [], "replaced_by": "DEC-0003",
        "obsolete_evidence": ["The old two-function source claim", "The old 4K-fallback assumption"],
    },
    {
        "id": "DEC-0003", "date": "2026-08-18", "action": "REVERSAL",
        "status": "ACTIVE", "features": ["GR-015", "GR-020"],
        "decision": "Enforce clipping in all five back-buffer write paths by folding the scissor into loop bounds.",
        "rationale": "Measured escape pixels refuted the two-function boundary without adding a per-pixel hot-path branch.",
        "source": "kernel/docs/DECISIONS.md",
        "needle": "**Three more functions write the back buffer",
        "legacy_labels": ["6"], "supersedes": ["DEC-0002"],
        "replaced_by": None, "obsolete_evidence": [],
    },
    {
        "id": "DEC-0004", "date": "2026-08-17", "action": "DEPRECATION",
        "status": "ACTIVE", "features": ["GR-037", "GR-012"],
        "decision": "Retire snapshot-and-sticker window dragging after damage repaint is authoritative; retain only cursor save-under.",
        "rationale": "The old path imposed a window-size ceiling, artifacts and fixed memory cost.",
        "source": "kernel/docs/DECISIONS.md",
        "needle": "**7. Delete the snapshot-and-sticker drag machinery**",
        "legacy_labels": ["7"], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": ["Snapshot/sticker drag screenshots do not prove the retained compositor path"],
    },
    {
        "id": "DEC-0005", "date": "2026-08-17", "action": "DECISION",
        "status": "QUALIFIED", "features": ["DV-045", "DV-047", "GR-025"],
        "decision": "Do not port a Linux-scale GPU stack; implement bounded native mechanisms while borrowing public hardware knowledge, not donor code.",
        "rationale": "A complete i915/Mesa stack assumes kernel facilities outside zlOS, but bounded display/blitter providers remain valid product work.",
        "source": "kernel/docs/DECISIONS.md",
        "needle": "**20. `intel.c` already has the correct relationship: borrow Linux's knowledge,",
        "legacy_labels": ["17", "20"], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": ["The phrase do not write a GPU driver cannot be used to reject bounded native providers"],
    },
    {
        "id": "DEC-0006", "date": "2026-08-18", "action": "MIGRATION",
        "status": "ACTIVE", "features": ["EV-007", "EV-008"],
        "decision": "All four kernel compiler routes consume one ordered kernel/SOURCES manifest.",
        "rationale": "Four hand-maintained lists repeatedly produced route-specific source drift.",
        "source": "kernel/SOURCES",
        "needle": "# SOURCES - the kernel sources that EVERY target compiles.",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": ["A green single-route build no longer proves source-list parity"],
    },
    {
        "id": "DEC-0007", "date": "2026-08-18", "action": "DECISION",
        "status": "SUPERSEDED", "features": ["WB-001", "WB-048"],
        "decision": "Classify a zlOS browser as unbounded by comparing only maximal Chromium/Ladybird implementations.",
        "rationale": "The comparison measured the maximal implementation rather than the bounded capability.",
        "source": "kernel/docs/browser-status.md",
        "needle": "> **A browser** | Unbounded. Chromium is 6.38M lines",
        "legacy_labels": [], "supersedes": [], "replaced_by": "DEC-0008",
        "obsolete_evidence": ["The original browser rejection", "The original 2,900-line optimistic count"],
    },
    {
        "id": "DEC-0008", "date": "2026-08-19", "action": "REVERSAL",
        "status": "ACTIVE", "features": ["WB-001", "WB-048"],
        "decision": "Build the bounded document-browser capability while explicitly refusing moving-target Chrome parity.",
        "rationale": "Fetcher, parser, box model and bounded scripting have finite contracts and current implementation evidence.",
        "source": "kernel/docs/browser-status.md",
        "needle": "**Every number there is correct and the conclusion does not follow.**",
        "legacy_labels": [], "supersedes": ["DEC-0007"], "replaced_by": None,
        "obsolete_evidence": [],
    },
    {
        "id": "DEC-0009", "date": "2026-08-22", "action": "CORRECTION",
        "status": "ACTIVE", "features": ["EV-011", "AP-002"],
        "decision": "Replace the false-green aggregate application check with exact manifest, route and lifecycle evidence.",
        "rationale": "The old aggregate printed Maze missing while still returning success.",
        "source": "kernel/HANDOFF.md",
        "needle": "It replaces the old aggregate `apps53.py` green that printed `Maze exists NO`",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": ["The old apps53.py aggregate green"],
    },
    {
        "id": "DEC-0010", "date": "2026-08-22", "action": "DECISION",
        "status": "ACTIVE", "features": ["EV-009", "EV-010", "EV-015"],
        "decision": "Never promote one artifact or route from another route's runtime or physical evidence.",
        "rationale": "Similar names and parent/payload relationships do not prove the exact bytes executed on another route.",
        "source": "kernel/docs/artifact-and-boot-route-registry-2026-08-22.md",
        "needle": "runtime or hardware claim merely because a similarly named artifact passed.",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": ["Route-family or controller-family evidence propagation"],
    },
    {
        "id": "DEC-0011", "date": "2026-08-22", "action": "CORRECTION",
        "status": "ACTIVE", "features": ["DV-045"],
        "decision": "Label Intel modeset writes as host-harness proved and boot-unreachable until a native kernel caller and receipt exist.",
        "rationale": "The real driver can light the panel from the harness, but no kernel path arms it.",
        "source": "kernel/HANDOFF.md",
        "needle": "**Nothing in the kernel arms `lt_armed`.** This runs from the host harness only.",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": ["Any claim that the current zlOS boot can light the Intel panel itself"],
    },
    {
        "id": "DEC-0012", "date": "2026-08-22", "action": "CORRECTION",
        "status": "ACTIVE", "features": ["DV-031"],
        "decision": "Classify I2C-HID as a raw transport, not an input provider, until report decoding and session routing exist.",
        "rationale": "Raw bytes are exposed but no x/y/button decoder exists.",
        "source": "kernel/HANDOFF.md",
        "needle": "it is a **transport with no decoder**.",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": ["Any I2C-HID input-provider completion claim based on raw-byte diagnostics"],
    },
    {
        "id": "DEC-0013", "date": "2026-08-23", "action": "DECISION",
        "status": "ACTIVE", "features": ["EV-003", "OP-001"],
        "decision": "Block public release while build inputs lack an established repository redistribution grant.",
        "rationale": "Source possession and successful builds do not create a distribution license.",
        "source": "kernel/HANDOFF.md",
        "needle": "There is currently no repository license file, so public release is explicitly",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": [],
    },
    {
        "id": "DEC-0014", "date": "2026-08-24", "action": "DECISION",
        "status": "ACTIVE", "features": ["EV-002"],
        "decision": "Call the deterministic archive exact build-input recovery, not signed/off-host/whole-repository custody.",
        "rationale": "The archive remains unsigned and in the same uncommitted worktree with zero off-host copies.",
        "source": "kernel/docs/mp00-source-snapshot-2026-08-24.md",
        "needle": "This is `PARTIAL_CURRENT`, not complete source custody.",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": [],
    },
    {
        "id": "DEC-0015", "date": "2026-08-23", "action": "DECISION",
        "status": "ACTIVE", "features": ["EV-004", "EV-005"],
        "decision": "Dependency verification checks exact invoked/resolved bytes and never silently refreshes the lock.",
        "rationale": "PATH aliases, package upgrades and firmware drift must make the gate red until inspected.",
        "source": "kernel/docs/dependency-lock-truth-2026-08-23.md",
        "needle": "The landing gate checks but never rewrites this file.",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": [],
    },
    {
        "id": "DEC-0016", "date": "2026-08-24", "action": "DECISION",
        "status": "ACTIVE", "features": ["EV-007"],
        "decision": "Represent conservative build-identity inputs without active edges as scope-only, never compiled.",
        "rationale": "Dropping them hides identity scope; calling them compiled invents reachability.",
        "source": "kernel/docs/mp00-build-graph-2026-08-24.md",
        "needle": "`declared-scope-only`; it does not call it compiled",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": [],
    },
    {
        "id": "DEC-0017", "date": "2026-08-24", "action": "OPERATIONS",
        "status": "ACTIVE", "features": ["EV-008", "OP-023"],
        "decision": "Forbid unrestricted full landing gates and require the resource-contained launcher and idle-host doctor.",
        "rationale": "Overlapping build/editor/graphics pressure preceded a host freeze and forced shutdown.",
        "source": "kernel/docs/host-freeze-and-gate-containment-2026-08-24.md",
        "needle": "The complete gate is no longer allowed to run unrestricted.",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": ["The interrupted 09:11 landing-gate attempt is not green"],
    },
    {
        "id": "DEC-0018", "date": "2026-08-24", "action": "DECISION",
        "status": "ACTIVE", "features": ["EV-018", "AU-028"],
        "decision": "Prove the event envelope as a host-only single-owner core before changing shipped build identity or claiming an audit service.",
        "rationale": "Target integration requires an idle contained rebuild and later per-CPU/IRQ/service ownership.",
        "source": "kernel/docs/mp00-structured-event-trace-2026-08-24.md",
        "needle": "This is `PARTIAL_CURRENT`. It is not in `kernel/SOURCES`",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": [],
    },
    {
        "id": "DEC-0019", "date": "2026-08-24", "action": "CORRECTION",
        "status": "ACTIVE", "features": ["EV-008"],
        "decision": "Keep the complete landing gate red/unverified until a contained run finishes and its final receipt is captured.",
        "rationale": "The last attempt stopped during EFI build and cannot support an aggregate green claim.",
        "source": "kernel/docs/host-freeze-and-gate-containment-2026-08-24.md",
        "needle": "There is no completed full landing-gate receipt after this incident.",
        "legacy_labels": [], "supersedes": [], "replaced_by": None,
        "obsolete_evidence": ["Partial pre-freeze gate output"],
    },
)

OPEN_GAPS = {
    "legacy_labels_without_normalized_semantics": 0,
    "system_wide_source_inventory_complete": False,
    "signed_or_independently_approved": False,
    "automatic_migration_from_changed_source": False,
    "user_visible_history_viewer": False,
}


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def legacy_labels() -> list[str]:
    text = (ROOT / LEGACY_SOURCE).read_text()
    numbers = set(re.findall(r"^\*\*(\d+)\.", text, re.MULTILINE))
    numbers.update(re.findall(r"^###\s+#(\d+)\b", text, re.MULTILINE))
    exec_labels = {"E" + value for value in
                   re.findall(r"^###\s+#E(\d+)\b", text, re.MULTILINE)}
    if {int(value) for value in numbers} != set(range(1, 44)) or \
            exec_labels != {"E1", "E2", "E3", "E4"}:
        raise ValueError("legacy decision label inventory drift")
    return [str(value) for value in range(1, 44)] + ["E1", "E2", "E3", "E4"]


def legacy_feature_scope(label: str) -> list[str]:
    if label.startswith("E"):
        return ["ZL-043", "KR-010"]
    number = int(label)
    if number <= 16:
        return ["GR-015", "DS-001"]
    if number <= 24:
        return ["GR-025", "DV-045"]
    if number <= 28:
        return ["EV-011", "DS-001"]
    return ["DS-001", "GR-015"]


def legacy_semantics() -> list[dict]:
    path = ROOT / LEGACY_SOURCE
    text = path.read_text()
    plain_re = re.compile(r"^\*\*(?P<plain>\d+)\.\s*(?P<title>.*?)\*\*", re.MULTILINE | re.DOTALL)
    section_re = re.compile(r"^###\s+#(?P<section>(?:\d+|E\d+))\s*\|\s*(?P<title>.+)$", re.MULTILINE)
    found: dict[str, tuple[re.Match, str]] = {}
    matches = list(plain_re.finditer(text)) + list(section_re.finditer(text))
    for match in sorted(matches, key=lambda item: item.start()):
        label = match.groupdict().get("plain") or match.groupdict().get("section")
        if label in found:
            continue
        title = re.sub(r"[`*_~]", "", match.group("title")).strip().rstrip(".")
        title = " ".join(title.split())
        found[label] = (match, title)
    expected = legacy_labels()
    if set(found) != set(expected):
        raise ValueError(f"legacy semantic heading set drift: {sorted(found)!r}")
    ordered_matches = sorted((match.start(), label, match, title)
                             for label, (match, title) in found.items())
    next_start = {label: (ordered_matches[index + 1][0] if index + 1 < len(ordered_matches) else len(text))
                  for index, (_, label, _, _) in enumerate(ordered_matches)}
    rows = []
    for label in expected:
        match, title = found[label]
        section = text[match.start():next_start[label]].rstrip() + "\n"
        rows.append({
            "label": label,
            "title": title,
            "feature_scope": legacy_feature_scope(label),
            "source_line": text.count("\n", 0, match.start()) + 1,
            "source_heading": match.group(0),
            "section_sha256": hashlib.sha256(section.encode()).hexdigest(),
            "semantics_status": "NORMALIZED_SOURCE_HEADING_AND_SECTION",
            "evidence_ceiling": "source-normalized decision section; implementation effect requires feature evidence",
        })
    return rows


def source_row(record: dict) -> dict:
    path = ROOT / record["source"]
    text = path.read_text()
    occurrences = text.count(record["needle"])
    if occurrences != 1:
        raise ValueError(f"{record['id']}: source needle occurs {occurrences} times")
    return {"path": record["source"], "sha256": sha256(path),
            "needle": record["needle"]}


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.decision-ledger.v1":
        raise ValueError("wrong decision-ledger schema")
    if value.get("result") != "PASS_WITH_OPEN_GAPS":
        raise ValueError("decision ledger hid its coverage gaps")
    if value.get("build_identity") != json.loads(
            (HERE / "build-identity.json").read_text())["identity_sha256"]:
        raise ValueError("decision ledger has stale build context")
    records = value.get("records", [])
    if [row.get("id") for row in records] != [row["id"] for row in RECORDS] or \
            len({row.get("id") for row in records}) != len(RECORDS):
        raise ValueError("decision identity/order drift")
    by_id = {row["id"]: row for row in records}
    canonical_by_id = {row["id"]: row for row in RECORDS}
    for row in records:
        if row.get("status") not in STATUSES or row.get("action") not in ACTIONS:
            raise ValueError(f"{row.get('id')}: unknown status/action")
        try:
            date.fromisoformat(row.get("date", ""))
        except ValueError as error:
            raise ValueError(f"{row.get('id')}: invalid date") from error
        if not row.get("decision") or not row.get("rationale") or \
                not row.get("features") or len(row.get("source", {}).get("sha256", "")) != 64:
            raise ValueError(f"{row.get('id')}: incomplete decision evidence")
        if row["source"] != source_row(canonical_by_id[row["id"]]):
            raise ValueError(f"{row['id']}: source evidence drift")
        for target in row.get("supersedes", []):
            if target not in by_id or target == row["id"]:
                raise ValueError(f"{row['id']}: invalid supersedes link")
            if by_id[target].get("replaced_by") != row["id"]:
                raise ValueError(f"{row['id']}: replacement link is not reciprocal")
        replacement = row.get("replaced_by")
        if row["status"] == "SUPERSEDED":
            if replacement not in by_id or not row.get("obsolete_evidence"):
                raise ValueError(f"{row['id']}: superseded record lacks replacement/history")
            if row["id"] not in by_id[replacement].get("supersedes", []):
                raise ValueError(f"{row['id']}: replacement does not point back")
        elif replacement is not None:
            raise ValueError(f"{row['id']}: active record has a replacement")
        if row["action"] == "REVERSAL" and not row.get("supersedes"):
            raise ValueError(f"{row['id']}: reversal has no reversed decision")
    for start in by_id:
        seen = set()
        current = start
        while by_id[current].get("replaced_by"):
            current = by_id[current]["replaced_by"]
            if current in seen:
                raise ValueError("decision replacement cycle")
            seen.add(current)
    legacy = value.get("legacy_source_inventory", {})
    labels = legacy_labels()
    if legacy != {"path": LEGACY_SOURCE, "sha256": sha256(ROOT / LEGACY_SOURCE),
                  "labels": labels, "labels_count": 47}:
        raise ValueError("legacy decision inventory drift")
    semantics = value.get("legacy_semantics", [])
    if semantics != legacy_semantics() or [row.get("label") for row in semantics] != labels:
        raise ValueError("legacy normalized semantics drift")
    if any(len(row.get("section_sha256", "")) != 64 or not row.get("title")
           or not row.get("feature_scope") for row in semantics):
        raise ValueError("legacy semantic identity/scope missing")
    normalized_labels = [row["label"] for row in semantics]
    if value.get("coverage") != {
            "normalized_records": 19,
            "legacy_labels_indexed": 47,
            "legacy_labels_semantically_normalized": 47,
            "legacy_labels_remaining": 0,
            "normalized_legacy_labels": normalized_labels,
    }:
        raise ValueError("decision coverage accounting drift")
    expected_status = {name: sum(row["status"] == name for row in records)
                       for name in sorted(STATUSES)}
    expected_action = {name: sum(row["action"] == name for row in records)
                       for name in sorted(ACTIONS)}
    if value.get("counts") != {"records": 19, "status": expected_status,
                               "action": expected_action}:
        raise ValueError("decision counts drift")
    if value.get("open_gaps") != OPEN_GAPS:
        raise ValueError("decision-ledger gaps were hidden")


def build() -> dict:
    records = []
    for source in RECORDS:
        row = copy.deepcopy(source)
        row["source"] = source_row(source)
        row["evidence_ceiling"] = "attributable source decision; runtime effect requires its feature receipt"
        records.append(row)
    labels = legacy_labels()
    semantics = legacy_semantics()
    normalized_labels = [row["label"] for row in semantics]
    value = {
        "schema": "zlos.decision-ledger.v1",
        "result": "PASS_WITH_OPEN_GAPS",
        "feature_id": "EV-026",
        "build_identity": json.loads((HERE / "build-identity.json").read_text())[
            "identity_sha256"],
        "records": records,
        "legacy_source_inventory": {
            "path": LEGACY_SOURCE, "sha256": sha256(ROOT / LEGACY_SOURCE),
            "labels": labels, "labels_count": len(labels),
        },
        "legacy_semantics": semantics,
        "coverage": {
            "normalized_records": len(records),
            "legacy_labels_indexed": len(labels),
            "legacy_labels_semantically_normalized": len(normalized_labels),
            "legacy_labels_remaining": len(labels) - len(normalized_labels),
            "normalized_legacy_labels": normalized_labels,
        },
        "counts": {
            "records": len(records),
            "status": {name: sum(row["status"] == name for row in records)
                       for name in sorted(STATUSES)},
            "action": {name: sum(row["action"] == name for row in records)
                       for name in sorted(ACTIONS)},
        },
        "open_gaps": OPEN_GAPS,
        "evidence_ceiling": "19 high-impact records plus source-normalized semantics for all 47 legacy labels; not complete system history or independent approval",
        "weakest_link": "system-wide non-DECISIONS.md sources, independent approval, automatic migration and target history UI remain open",
        "generator": {"path": "kernel/gen-decision-ledger.py",
                      "sha256": sha256(Path(__file__).resolve())},
    }
    validate(value)
    return value


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["records"].pop()
    mutations["missing-record"] = missing
    status = copy.deepcopy(value)
    status["records"][0]["status"] = "MAYBE"
    mutations["unknown-status"] = status
    source = copy.deepcopy(value)
    source["records"][0]["source"]["needle"] = "invented source"
    mutations["invented-source"] = source
    replacement = copy.deepcopy(value)
    replacement["records"][1]["replaced_by"] = None
    mutations["lost-replacement"] = replacement
    obsolete = copy.deepcopy(value)
    obsolete["records"][1]["obsolete_evidence"] = []
    mutations["deleted-obsolete-evidence"] = obsolete
    semantics = copy.deepcopy(value)
    semantics["legacy_semantics"].pop()
    mutations["missing-legacy-semantics"] = semantics
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"decision-ledger mutation escaped: {name}")
    print("decision-ledger selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    with tempfile.NamedTemporaryFile("w", dir=HERE, delete=False,
                                     encoding="utf-8") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, OUTPUT)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        if args.write:
            value = build()
            write_atomic(value)
        else:
            value = json.loads(OUTPUT.read_text())
            validate(value)
            if value != build():
                raise ValueError("decision ledger is stale")
        if args.selftest:
            selftest(value)
        coverage = value["coverage"]
        print("decision-ledger: PASS_WITH_OPEN_GAPS: "
              f"{coverage['normalized_records']} normalized, "
              f"{coverage['legacy_labels_indexed']} legacy indexed, "
              f"{coverage['legacy_labels_remaining']} legacy semantics open")
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"decision-ledger: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
