#!/usr/bin/env python3
"""Record current accessibility proof without equating input tests to a11y."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
METADATA = KERNEL_ROOT / "metadata"
OUTPUT = METADATA / "accessibility-registry.json"
RUN_RECEIPT = KERNEL_ROOT / "tests/host/test-run-receipt.json"

CAPABILITIES = (
    ("keyboard-input-parity", "HOST_PASSED", ("inputtest", "inputtest_hid"), "PS/2 and USB key/modifier agreement; not an app workflow"),
    ("focus-lifecycle", "HOST_PASSED", ("wmtest",), "focus, Alt+Tab chain and close-to-next-focus in host compositor"),
    ("visible-focus", "HOST_PASSED", ("uitest",), "widget focus treatment asserted in host renderer"),
    ("non-stealing-notifications", "HOST_PASSED", ("toasttest",), "toast structurally cannot take window focus"),
    ("density-scaling", "PARTIAL_HOST_PASSED", ("uitest", "fbtext"), "1x/2x metrics and glyph density only; no user a11y workflow"),
    ("keyboard-only-app-workflows", "MISSING", (), "no all-62 keyboard-only open/use/close receipt"),
    ("semantic-tree", "MISSING", (), "no roles/names/states/relations tree transport"),
    ("screen-reader-speech-braille", "MISSING", (), "no assistive provider or privacy/lifecycle proof"),
    ("magnifier-large-text-cursor", "MISSING", (), "no magnifier provider or user settings route"),
    ("high-contrast-color-filters", "MISSING", (), "no high-contrast theme/filter qualification"),
    ("captions-transcripts", "MISSING", (), "no caption provider or media workflow"),
    ("switch-dwell-control", "MISSING", (), "no switch scanning or dwell input workflow"),
    ("sticky-filter-slow-remap-keys", "MISSING", (), "no accessibility keyboard policy layer"),
    ("reduced-motion", "PARTIAL_HOST_PASSED", ("wmtest",), "animations-off control is tested; no user policy or complete motion audit"),
    ("localization-rtl", "MISSING", (), "no locale/RTL/non-Latin app workflow proof"),
)


def sha256(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.accessibility-proof-registry.v1":
        raise ValueError("wrong accessibility schema")
    if value.get("result") != "PASS_WITH_OPEN_GAPS":
        raise ValueError("accessibility registry overpromoted partial proof")
    rows = value.get("capabilities", [])
    if [row.get("id") for row in rows] != [row[0] for row in CAPABILITIES]:
        raise ValueError("accessibility capability set/order drift")
    if [row.get("status") for row in rows] != [row[1] for row in CAPABILITIES]:
        raise ValueError("accessibility evidence status drift")
    expected_counts = {
        "capabilities": 15,
        "host_passed": 4,
        "partial_host_passed": 2,
        "missing": 9,
        "complete_target_workflows": 0,
        "current_build_bound_qemu_workflows": 0,
    }
    if value.get("counts") != expected_counts:
        raise ValueError("accessibility counts drift")
    expected_gaps = [row[0] for row in CAPABILITIES if row[1] != "HOST_PASSED"]
    if value.get("open_gaps") != expected_gaps:
        raise ValueError("accessibility open gaps were hidden")
    if len(value.get("build_identity", "")) != 64 or len(value.get("host_test_receipt_sha256", "")) != 64:
        raise ValueError("accessibility evidence identity missing")
    if value.get("visual_registry", {}).get("current_build_bound") != 0:
        raise ValueError("unearned current visual focus proof")


def build() -> dict:
    receipt = json.loads(RUN_RECEIPT.read_text())
    passed = {row["name"] for row in receipt["results"] if row.get("status") == "passed"}
    visual = json.loads((METADATA / "visual-registry.json").read_text())
    rows = []
    for capability_id, status, targets, detail in CAPABILITIES:
        missing = sorted(set(targets) - passed)
        if missing:
            raise ValueError(f"{capability_id}: mapped host proof is not passing: {missing}")
        rows.append({
            "id": capability_id,
            "status": status,
            "passed_host_targets": list(targets),
            "detail": detail,
            "evidence_ceiling": "host behavior only" if targets else "no current evidence",
        })
    value = {
        "schema": "zlos.accessibility-proof-registry.v1",
        "result": "PASS_WITH_OPEN_GAPS",
        "build_identity": json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"],
        "host_test_receipt_sha256": sha256(RUN_RECEIPT),
        "source_identities": {
            path: sha256(KERNEL_ROOT / path)
            for path in (
                "src/drivers/input/input.c", "src/graphics/ui/ui.c",
                "src/graphics/windowing/wm.c", "tests/host/inputtest_hid.c",
                "tests/host/uitest.c", "tests/host/wmtest.c", "tests/host/toasttest.c",
            )
        },
        "visual_registry": {
            "path": "kernel/metadata/visual-registry.json",
            "sha256": sha256(METADATA / "visual-registry.json"),
            "current_build_bound": visual["counts"]["current_build_bound"],
        },
        "capabilities": rows,
        "counts": {
            "capabilities": len(rows),
            "host_passed": sum(row["status"] == "HOST_PASSED" for row in rows),
            "partial_host_passed": sum(row["status"] == "PARTIAL_HOST_PASSED" for row in rows),
            "missing": sum(row["status"] == "MISSING" for row in rows),
            "complete_target_workflows": 0,
            "current_build_bound_qemu_workflows": 0,
        },
        "open_gaps": [row["id"] for row in rows if row["status"] != "HOST_PASSED"],
        "evidence_ceiling": "host input/focus/scale primitives only; no complete assistive or target workflow promotion",
        "weakest_link": "semantic tree and assistive providers do not exist; no all-app keyboard, QEMU or native a11y workflow receipt",
        "generator": {"path": "kernel/gen-accessibility-registry.py", "sha256": sha256(Path(__file__).resolve())},
    }
    validate(value)
    return value


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["capabilities"].pop()
    mutations["missing-capability"] = missing
    promoted = copy.deepcopy(value)
    next(row for row in promoted["capabilities"] if row["id"] == "semantic-tree")["status"] = "HOST_PASSED"
    mutations["invented-semantic-tree"] = promoted
    hidden = copy.deepcopy(value)
    hidden["open_gaps"].remove("screen-reader-speech-braille")
    mutations["hidden-assistive-gap"] = hidden
    target = copy.deepcopy(value)
    target["counts"]["complete_target_workflows"] = 1
    mutations["target-overclaim"] = target
    visual = copy.deepcopy(value)
    visual["visual_registry"]["current_build_bound"] = 46
    mutations["unearned-visual-proof"] = visual
    caught = []
    for name, mutated in mutations.items():
        try:
            validate(mutated)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"accessibility selftest mutation escaped: {name}")
    print("accessibility selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    with tempfile.NamedTemporaryFile("w", dir=HERE, delete=False, encoding="utf-8") as handle:
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
    if not args.write and not args.check:
        args.check = True
    try:
        value = build()
        if args.selftest:
            selftest(value)
        if args.write:
            write_atomic(value)
        if args.check and (not OUTPUT.is_file() or json.loads(OUTPUT.read_text()) != value):
            raise ValueError("accessibility-registry.json is missing or stale")
        print(
            "accessibility: PASS_WITH_OPEN_GAPS: "
            f"{value['counts']['host_passed']} host-passed, "
            f"{value['counts']['partial_host_passed']} partial, {value['counts']['missing']} missing"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"accessibility: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
