#!/usr/bin/env python3
"""Generate the physical-test matrix and validate exact-hash hardware receipts."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import tempfile
from datetime import datetime
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
REPO_ROOT = KERNEL_ROOT.parent
METADATA = KERNEL_ROOT / "metadata"
RECEIPT_DIR = KERNEL_ROOT / "docs/receipts/hardware"
OUTPUT = METADATA / "hardware-receipt-plan.json"
TEMPLATE = METADATA / "hardware-receipt-template.json"
BUILD_IDENTITY = METADATA / "build-identity.json"
REPRO_RECEIPT = KERNEL_ROOT / "docs/receipts/reproducible-build-2026-08-22.json"

SCENARIOS = (
    "first-use", "reuse", "warm-reboot", "cold-boot", "reset",
    "teardown", "recovery",
)
EVIDENCE_KINDS = (
    "artifact-readback", "machine-inventory", "firmware-inventory",
    "device-topology", "boot-log", "screen-capture", "interaction-log",
    "storage-readback", "teardown-log", "recovery-log",
)
ROUTE_SPECS = (
    ("grub-bios32", "zlOS.iso", "kernel.elf", "BIOS", 32, "secondary-compatibility"),
    ("grub-uefi32", "zlOS.iso", "kernel.elf", "UEFI", 32, "secondary-compatibility"),
    ("grub-bios64", "zlOS64.iso", "kernel64.elf", "BIOS", 64, "secondary-compatibility"),
    ("grub-uefi64", "zlOS64.iso", "kernel64.elf", "UEFI", 64, "secondary-compatibility"),
    ("raw-bios", "zlOS.img", "kernel_raw.elf", "BIOS", 32, "secondary-compatibility"),
    ("native-uefi64", "zlOS-usb.img", "BOOTX64.EFI", "UEFI", 64, "primary-h2-thinkpad"),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def canonical_sha256(value: dict) -> str:
    subject = copy.deepcopy(value)
    subject.pop("content_sha256", None)
    payload = json.dumps(subject, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(payload).hexdigest()


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def fail(message: str) -> None:
    raise ValueError(message)


def relative_evidence_path(raw: str) -> Path:
    path = Path(raw)
    if path.is_absolute() or ".." in path.parts:
        fail(f"raw evidence path escapes repository: {raw}")
    resolved = (REPO_ROOT / path).resolve()
    try:
        resolved.relative_to(REPO_ROOT.resolve())
    except ValueError:
        fail(f"raw evidence path escapes repository: {raw}")
    return resolved


def parse_time(raw: str, label: str) -> datetime:
    if not isinstance(raw, str) or not raw.endswith("Z"):
        fail(f"{label}: timestamp must be UTC RFC3339")
    try:
        return datetime.fromisoformat(raw[:-1] + "+00:00")
    except ValueError:
        fail(f"{label}: invalid timestamp")


def exact_artifact(name: str, artifacts: dict) -> dict:
    row = artifacts.get(name)
    if not isinstance(row, dict) or len(row.get("sha256", "")) != 64 \
            or not isinstance(row.get("bytes"), int) or row["bytes"] <= 0:
        fail(f"{name}: missing exact artifact identity")
    return {"name": name, "sha256": row["sha256"], "bytes": row["bytes"]}


def expected_cases(repro: dict) -> list[dict]:
    artifacts = repro.get("artifacts", {})
    cases = []
    for route, medium, payload, firmware, bits, target_class in ROUTE_SPECS:
        cases.append({
            "case_id": f"physical-{route}",
            "route": route,
            "target_class": target_class,
            "firmware": firmware,
            "entry_bits": bits,
            "boot_medium": exact_artifact(medium, artifacts),
            "payload": exact_artifact(payload, artifacts),
            "required_scenarios": list(SCENARIOS),
            "required_evidence_kinds": list(EVIDENCE_KINDS),
            "required_device_classes": [
                "display", "storage", "keyboard", "pointer", "network",
            ],
        })
    return cases


def validate_machine(machine: dict) -> None:
    required_strings = ("manufacturer", "product", "board", "system_uuid_sha256")
    if any(not isinstance(machine.get(key), str) or not machine[key].strip()
           for key in required_strings):
        fail("machine identity is incomplete")
    if len(machine["system_uuid_sha256"]) != 64:
        fail("machine UUID must be stored as a SHA-256 digest")
    firmware = machine.get("firmware", {})
    if any(not isinstance(firmware.get(key), str) or not firmware[key].strip()
           for key in ("type", "vendor", "version", "release_date")):
        fail("firmware identity is incomplete")
    cpu = machine.get("cpu", {})
    if any(not isinstance(cpu.get(key), str) or not cpu[key].strip()
           for key in ("vendor", "model")) \
            or not isinstance(cpu.get("logical_processors"), int) \
            or cpu["logical_processors"] <= 0:
        fail("CPU topology is incomplete")
    if not isinstance(machine.get("memory_bytes"), int) or machine["memory_bytes"] <= 0:
        fail("memory topology is incomplete")
    devices = machine.get("devices", {})
    for device_class in ("display", "storage", "keyboard", "pointer", "network"):
        rows = devices.get(device_class)
        if not isinstance(rows, list) or not rows:
            fail(f"machine topology is missing {device_class}")
        for row in rows:
            if not isinstance(row, dict) or not row.get("bus") or not row.get("identity"):
                fail(f"machine {device_class} identity is incomplete")


def validate_evidence(rows: list, required_kinds: list[str]) -> None:
    if not isinstance(rows, list) or not rows:
        fail("raw evidence is absent")
    kinds = []
    paths = set()
    for row in rows:
        kind = row.get("kind")
        raw_path = row.get("path")
        if kind not in EVIDENCE_KINDS or not isinstance(raw_path, str):
            fail("raw evidence has an unknown kind or path")
        if raw_path in paths:
            fail(f"raw evidence path is reused: {raw_path}")
        paths.add(raw_path)
        path = relative_evidence_path(raw_path)
        if not path.is_file():
            fail(f"raw evidence is missing: {raw_path}")
        if row.get("bytes") != path.stat().st_size or row.get("sha256") != sha256(path):
            fail(f"raw evidence identity mismatch: {raw_path}")
        if not row.get("captured_at_utc"):
            fail(f"raw evidence has no capture time: {raw_path}")
        parse_time(row["captured_at_utc"], raw_path)
        kinds.append(kind)
    missing = sorted(set(required_kinds) - set(kinds))
    if missing:
        fail("raw evidence kinds are missing: " + ", ".join(missing))


def validate_receipt(receipt: dict, plan: dict) -> None:
    if receipt.get("schema") != "zlos.hardware-receipt.v1":
        fail("wrong hardware receipt schema")
    if receipt.get("result") != "PASS":
        fail("only a complete PASS receipt can promote physical evidence")
    if receipt.get("build_identity") != plan.get("build_identity"):
        fail("hardware receipt build identity mismatch")
    cases = {row["case_id"]: row for row in plan.get("device_matrix", [])}
    case = cases.get(receipt.get("case_id"))
    if case is None or receipt.get("route") != case["route"]:
        fail("hardware receipt route/case mismatch")
    started = parse_time(receipt.get("started_at_utc"), "receipt start")
    completed = parse_time(receipt.get("completed_at_utc"), "receipt completion")
    if completed <= started:
        fail("hardware receipt completion is not after start")
    operator = receipt.get("operator", {})
    if not operator.get("id") or operator.get("observed_in_person") is not True:
        fail("physical operator attestation is absent")
    validate_machine(receipt.get("machine", {}))
    if receipt["machine"]["firmware"]["type"] != case["firmware"]:
        fail("observed firmware mode does not match matrix case")
    observed = receipt.get("artifacts", {})
    for label in ("boot_medium", "payload"):
        if observed.get(label) != case[label]:
            fail(f"hardware receipt {label} identity mismatch")
    readback = receipt.get("boot_media_readback", {})
    if readback.get("sha256") != case["boot_medium"]["sha256"] \
            or readback.get("bytes") != case["boot_medium"]["bytes"]:
        fail("physical boot-media readback does not match exact artifact")
    if not readback.get("device") or not readback.get("serial_sha256") \
            or len(readback["serial_sha256"]) != 64:
        fail("physical boot-media identity is incomplete")
    scenarios = receipt.get("scenarios", [])
    if [row.get("id") for row in scenarios] != case["required_scenarios"]:
        fail("hardware lifecycle scenario set/order is incomplete")
    for row in scenarios:
        if row.get("result") != "PASS" or not row.get("observation"):
            fail(f"hardware scenario did not pass: {row.get('id')}")
    validate_evidence(receipt.get("raw_evidence"), case["required_evidence_kinds"])
    if receipt.get("content_sha256") != canonical_sha256(receipt):
        fail("hardware receipt content digest mismatch")


def template(plan: dict) -> dict:
    first = plan["device_matrix"][0]
    return {
        "schema": "zlos.hardware-receipt-template.v1",
        "result": "NOT_RUN",
        "instructions": "Copy into kernel/docs/receipts/hardware only after a real observed run; replace every placeholder and use zlos.hardware-receipt.v1.",
        "build_identity": plan["build_identity"],
        "case_id": first["case_id"],
        "route": first["route"],
        "started_at_utc": "REQUIRED",
        "completed_at_utc": "REQUIRED",
        "operator": {"id": "REQUIRED", "observed_in_person": False},
        "machine": {
            "manufacturer": "REQUIRED", "product": "REQUIRED", "board": "REQUIRED",
            "system_uuid_sha256": "REQUIRED",
            "firmware": {"type": first["firmware"], "vendor": "REQUIRED", "version": "REQUIRED", "release_date": "REQUIRED"},
            "cpu": {"vendor": "REQUIRED", "model": "REQUIRED", "logical_processors": 0},
            "memory_bytes": 0,
            "devices": {key: [{"bus": "REQUIRED", "identity": "REQUIRED"}]
                        for key in first["required_device_classes"]},
        },
        "artifacts": {"boot_medium": first["boot_medium"], "payload": first["payload"]},
        "boot_media_readback": {"device": "REQUIRED", "serial_sha256": "REQUIRED", "sha256": "REQUIRED", "bytes": 0},
        "scenarios": [{"id": scenario, "result": "NOT_RUN", "observation": "REQUIRED"}
                      for scenario in first["required_scenarios"]],
        "raw_evidence": [{"kind": kind, "path": "REQUIRED", "sha256": "REQUIRED", "bytes": 0, "captured_at_utc": "REQUIRED"}
                         for kind in first["required_evidence_kinds"]],
        "content_sha256": "REQUIRED_AFTER_ALL_FIELDS_ARE_FINAL",
    }


def build() -> dict:
    identity = load(BUILD_IDENTITY)
    repro = load(REPRO_RECEIPT)
    build_id = identity.get("identity_sha256")
    if repro.get("schema") != "zlos.reproducible-build-receipt.v1" \
            or repro.get("result") != "PASS" \
            or repro.get("build_identity", {}).get("id") != build_id:
        fail("reproducible artifact receipt is absent, failed or foreign")
    matrix = expected_cases(repro)
    plan = {
        "schema": "zlos.hardware-receipt-plan.v1",
        "result": "READY_FOR_PHYSICAL_EXECUTION_WITHOUT_RECEIPTS",
        "build_identity": build_id,
        "inputs": {
            "build_identity": {"path": "kernel/metadata/build-identity.json", "sha256": sha256(BUILD_IDENTITY)},
            "reproducible_artifacts": {"path": "kernel/docs/receipts/reproducible-build-2026-08-22.json", "sha256": sha256(REPRO_RECEIPT)},
        },
        "receipt_schema": "zlos.hardware-receipt.v1",
        "device_matrix": matrix,
        "validated_receipts": [],
        "counts": {
            "matrix_cases": len(matrix), "validated_receipts": 0,
            "physically_passed_routes": 0, "physically_passed_artifacts": 0,
            "required_scenarios_per_case": len(SCENARIOS),
            "required_evidence_kinds_per_case": len(EVIDENCE_KINDS),
        },
        "physical_status": "NOT_RUN",
        "evidence_ceiling": "current exact-hash physical execution plan and hostile receipt validator only; zero hardware runs, routes or artifact hashes are promoted",
        "weakest_link": "a human-observed physical run with durable raw evidence is still required for every matrix case",
        "generator": {"path": "kernel/tools/generators/gen-hardware-receipt-plan.py", "sha256": sha256(Path(__file__).resolve())},
    }
    for path in sorted(RECEIPT_DIR.glob("*.json")) if RECEIPT_DIR.exists() else []:
        receipt = load(path)
        validate_receipt(receipt, plan)
        plan["validated_receipts"].append({
            "path": str(path.relative_to(REPO_ROOT)), "sha256": sha256(path),
            "case_id": receipt["case_id"], "route": receipt["route"],
            "artifacts": receipt["artifacts"],
        })
    routes = {row["route"] for row in plan["validated_receipts"]}
    artifacts = {artifact["name"] for row in plan["validated_receipts"]
                 for artifact in row["artifacts"].values()}
    plan["counts"]["validated_receipts"] = len(plan["validated_receipts"])
    plan["counts"]["physically_passed_routes"] = len(routes)
    plan["counts"]["physically_passed_artifacts"] = len(artifacts)
    if plan["validated_receipts"]:
        plan["result"] = "PARTIAL_PHYSICAL_EVIDENCE"
        plan["physical_status"] = "PARTIAL"
    if len(routes) == len(matrix):
        plan["result"] = "PASS"
        plan["physical_status"] = "ALL_MATRIX_CASES_PASSED"
    return plan


def validate_plan(value: dict) -> None:
    if value.get("schema") != "zlos.hardware-receipt-plan.v1":
        fail("wrong hardware plan schema")
    if [row.get("route") for row in value.get("device_matrix", [])] != [row[0] for row in ROUTE_SPECS]:
        fail("physical route matrix set/order drift")
    counts = value.get("counts", {})
    if counts.get("matrix_cases") != 6 or counts.get("required_scenarios_per_case") != 7 \
            or counts.get("required_evidence_kinds_per_case") != 10:
        fail("physical matrix counts drift")
    receipts = value.get("validated_receipts", [])
    if counts.get("validated_receipts") != len(receipts):
        fail("validated receipt count drift")
    if not receipts and (value.get("result") != "READY_FOR_PHYSICAL_EXECUTION_WITHOUT_RECEIPTS"
                         or value.get("physical_status") != "NOT_RUN"
                         or counts.get("physically_passed_routes") != 0
                         or counts.get("physically_passed_artifacts") != 0):
        fail("physical evidence was invented without receipts")


def synthetic_receipt(plan: dict, directory: Path) -> dict:
    case = plan["device_matrix"][0]
    evidence = []
    for index, kind in enumerate(case["required_evidence_kinds"]):
        path = directory / f"evidence-{index}.txt"
        path.write_text(f"synthetic selftest evidence for {kind}\n")
        evidence.append({
            "kind": kind, "path": str(path.relative_to(REPO_ROOT)),
            "sha256": sha256(path), "bytes": path.stat().st_size,
            "captured_at_utc": "2026-08-29T01:02:03Z",
        })
    receipt = {
        "schema": "zlos.hardware-receipt.v1", "result": "PASS",
        "build_identity": plan["build_identity"], "case_id": case["case_id"],
        "route": case["route"], "started_at_utc": "2026-08-29T01:00:00Z",
        "completed_at_utc": "2026-08-29T01:10:00Z",
        "operator": {"id": "selftest", "observed_in_person": True},
        "machine": {
            "manufacturer": "selftest", "product": "selftest", "board": "selftest",
            "system_uuid_sha256": "1" * 64,
            "firmware": {"type": case["firmware"], "vendor": "selftest", "version": "1", "release_date": "2026-01-01"},
            "cpu": {"vendor": "selftest", "model": "selftest", "logical_processors": 1},
            "memory_bytes": 1,
            "devices": {key: [{"bus": "selftest", "identity": key}]
                        for key in case["required_device_classes"]},
        },
        "artifacts": {"boot_medium": case["boot_medium"], "payload": case["payload"]},
        "boot_media_readback": {"device": "selftest", "serial_sha256": "2" * 64,
                                "sha256": case["boot_medium"]["sha256"], "bytes": case["boot_medium"]["bytes"]},
        "scenarios": [{"id": scenario, "result": "PASS", "observation": "selftest"}
                      for scenario in case["required_scenarios"]],
        "raw_evidence": evidence,
    }
    receipt["content_sha256"] = canonical_sha256(receipt)
    return receipt


def selftest(plan: dict) -> None:
    with tempfile.TemporaryDirectory(prefix="zlos-hardware-receipt-", dir=REPO_ROOT) as raw:
        valid = synthetic_receipt(plan, Path(raw))
        validate_receipt(valid, plan)
        mutations = []
        wrong_artifact = copy.deepcopy(valid)
        wrong_artifact["artifacts"]["boot_medium"]["sha256"] = "0" * 64
        mutations.append(("wrong-artifact", wrong_artifact))
        missing_firmware = copy.deepcopy(valid)
        missing_firmware["machine"]["firmware"].pop("version")
        mutations.append(("missing-firmware", missing_firmware))
        missing_topology = copy.deepcopy(valid)
        missing_topology["machine"]["devices"]["display"] = []
        mutations.append(("missing-topology", missing_topology))
        missing_raw = copy.deepcopy(valid)
        missing_raw["raw_evidence"].pop()
        mutations.append(("missing-raw-evidence", missing_raw))
        missing_scenario = copy.deepcopy(valid)
        missing_scenario["scenarios"].pop()
        mutations.append(("missing-lifecycle", missing_scenario))
        invented_pass = copy.deepcopy(valid)
        invented_pass["operator"]["observed_in_person"] = False
        mutations.append(("invented-physical-pass", invented_pass))
        stale_build = copy.deepcopy(valid)
        stale_build["build_identity"] = "f" * 64
        mutations.append(("foreign-build", stale_build))
        bad_readback = copy.deepcopy(valid)
        bad_readback["boot_media_readback"]["sha256"] = "f" * 64
        mutations.append(("bad-media-readback", bad_readback))
        path_escape = copy.deepcopy(valid)
        path_escape["raw_evidence"][0]["path"] = "../outside.log"
        mutations.append(("evidence-path-escape", path_escape))
        hidden_change = copy.deepcopy(valid)
        hidden_change["machine"]["product"] = "changed after digest"
        mutations.append(("content-digest-drift", hidden_change))
        caught = []
        for name, mutation in mutations:
            try:
                validate_receipt(mutation, plan)
            except ValueError:
                caught.append(name)
            else:
                fail(f"selftest mutation escaped: {name}")
    empty_overclaim = copy.deepcopy(plan)
    empty_overclaim["physical_status"] = "ALL_MATRIX_CASES_PASSED"
    try:
        validate_plan(empty_overclaim)
    except ValueError:
        caught.append("zero-receipt-overclaim")
    else:
        fail("selftest mutation escaped: zero-receipt-overclaim")
    print("hardware-receipt selftest: caught " + ", ".join(caught))


def serialized(value: dict) -> str:
    return json.dumps(value, indent=2, sort_keys=False) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--validate-receipt", type=Path)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    try:
        plan = build()
        validate_plan(plan)
        if args.validate_receipt:
            validate_receipt(load(args.validate_receipt), plan)
            print(f"hardware receipt: PASS: {args.validate_receipt}")
            return 0
        wanted = serialized(plan)
        wanted_template = serialized(template(plan))
        if args.write:
            for path, content in ((OUTPUT, wanted), (TEMPLATE, wanted_template)):
                temp = path.with_suffix(path.suffix + ".tmp")
                temp.write_text(content)
                os.replace(temp, path)
        else:
            if not OUTPUT.exists() or OUTPUT.read_text() != wanted:
                fail("hardware receipt plan is stale; rerun with --write")
            if not TEMPLATE.exists() or TEMPLATE.read_text() != wanted_template:
                fail("hardware receipt template is stale; rerun with --write")
        if args.selftest:
            selftest(plan)
        print(
            "hardware receipt plan: " + plan["result"] + ": "
            f"{plan['counts']['matrix_cases']} routes, "
            f"{plan['counts']['validated_receipts']} physical receipts"
        )
        return 0
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as error:
        print(f"hardware receipt plan: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
