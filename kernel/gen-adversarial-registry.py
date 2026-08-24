#!/usr/bin/env python3
"""Record current verifier canaries, failure injection and hostile-input proof."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import subprocess
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
OUTPUT = HERE / "adversarial-registry.json"
RUN_RECEIPT = HERE / "hosttest/test-run-receipt.json"
INVENTORY = HERE / "hosttest/test-inventory.json"

CANARIES = (
    ("CAN-001", ("python3", "gen-build-identity.py", "--check", "--selftest"), "selftest: caught"),
    ("CAN-002", ("python3", "gen-app-manifest.py", "--check", "--selftest"), "selftest: caught"),
    ("CAN-003", ("python3", "check-build-contract.py", "--selftest"), "selftest: caught"),
    ("CAN-004", ("python3", "check-land-gate.py", "--selftest"), "selftest: caught"),
    ("CAN-005", ("python3", "check-boot-prereqs.py", "--selftest"), "selftest: caught"),
    ("CAN-006", ("python3", "check-elf-permissions.py", "--selftest"), "selftest: caught"),
    ("CAN-007", ("python3", "gen-dependency-lock.py", "--check", "--selftest"), "selftest: caught"),
    ("CAN-008", ("python3", "gen-license-registry.py", "--check", "--selftest"), "selftest: caught"),
    ("CAN-009", ("python3", "gen-init-registry.py", "--check", "--selftest"), "selftest: caught"),
    ("CAN-010", ("python3", "gen-app-evidence.py", "--check", "--selftest"), "selftest: caught"),
    ("CAN-011", ("python3", "gen-artifact-registry.py", "--check", "--selftest"), "selftest: caught"),
    ("CAN-012", ("python3", "gen-test-inventory.py", "--check", "--selftest"), "selftest: caught"),
    ("CAN-013", ("python3", "run-host-tests.py", "--selftest"), "selftest: caught"),
    ("CAN-014", ("python3", "check-reproducible-build.py", "--selftest"), "selftest: caught"),
    ("CAN-015", ("python3", "check-appids.py", "--selftest"), "selftest: planted"),
    ("CAN-016", ("python3", "hosttest/apps53.py", "--selftest"), "selftest: caught"),
    ("CAN-017", ("python3", "gen-boot-media-ids.py", "--check", "--selftest"), "mutations change IDs"),
    ("CAN-018", ("python3", "verify-crash.py", "--check", "--selftest"), "selftest: caught"),
    ("CAN-019", ("python3", "gen-wrapper-registry.py", "--check", "--selftest"), "selftest: caught"),
)

FAILURE_SEAMS = (
    ("allocation", "PARTIAL", ("arenatest", "heaptest"), "no deterministic every-nth-allocation failure sweep"),
    ("queue", "PARTIAL", ("systest", "tcptest"), "notification and TCP pressure only; not every queue"),
    ("io", "PARTIAL", ("fstest", "nettest", "tcptest"), "zlfs transient write and packet-loss paths only"),
    ("provider", "MISSING", (), "no start/admit/resource-reservation rollback injector"),
    ("service", "MISSING", (), "no service dependency-loss/crash/reconnect injector"),
    ("package", "MISSING", (), "no staged install/uninstall failure-at-every-step injector"),
    ("lifecycle", "PARTIAL", (), "62 app open-ready-close routes pass, but lifecycle failure is not injected"),
)

HOSTILE_FAMILIES = (
    ("elf", "MISSING", (), "no malformed ELF loader corpus"),
    ("archive", "MISSING", (), "no malformed archive/initramfs/package corpus"),
    ("filesystem", "HOST_PASSED", ("fstest",), "corrupt zlfs metadata, overflow and transient write rollback"),
    ("network", "HOST_PASSED", ("fuzz", "dnstest", "nettest", "tcptest", "httptest"), "scripted malformed packets and deterministic fuzz seed"),
    ("font", "MISSING", (), "no font parser exists and no hostile font corpus"),
    ("image", "HOST_PASSED", ("pngtest",), "146 sanitizer-backed accept/refuse checks over constructed PNG/DEFLATE inputs"),
    ("web", "HOST_PASSED", ("fuzz", "htmltest", "csstest", "jstest", "httptest"), "malformed HTML/CSS/JS/HTTP and bounded fuzz"),
    ("ipc", "MISSING", (), "no typed IPC envelope parser or hostile IPC corpus"),
    ("certificate", "HOST_PASSED", ("x509test",), "truncations, random bytes, forged root and time/host refusal"),
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def passed_targets(receipt: dict) -> set[str]:
    return {row["name"] for row in receipt["results"] if row.get("status") == "passed"}


def run_canaries() -> list[dict]:
    rows = []
    for canary_id, command, marker in CANARIES:
        completed = subprocess.run(
            command,
            cwd=HERE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=180,
            check=False,
        )
        output = completed.stdout
        if completed.returncode != 0 or marker not in output:
            raise ValueError(f"{canary_id} failed or did not prove its planted mutation")
        rows.append({
            "id": canary_id,
            "command": list(command),
            "exit_code": completed.returncode,
            "required_output_marker": marker,
            "output_sha256": hashlib.sha256(output.encode()).hexdigest(),
            "result": "PLANTED_MUTATION_CAUGHT",
        })
    return rows


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.adversarial-registry.v1":
        raise ValueError("wrong adversarial-registry schema")
    if value.get("result") != "PASS_WITH_OPEN_GAPS":
        raise ValueError("adversarial registry must preserve open gaps")
    canaries = value.get("verifier_canaries", [])
    if [row.get("id") for row in canaries] != [row[0] for row in CANARIES]:
        raise ValueError("verifier canary set/order drift")
    if any(row.get("result") != "PLANTED_MUTATION_CAUGHT" for row in canaries):
        raise ValueError("a verifier canary is not proved red")
    failures = value.get("failure_injection", [])
    if [row.get("family") for row in failures] != [row[0] for row in FAILURE_SEAMS]:
        raise ValueError("failure-injection family set/order drift")
    if [row.get("status") for row in failures] != [row[1] for row in FAILURE_SEAMS]:
        raise ValueError("failure-injection evidence status drift")
    hostile = value.get("hostile_corpus", [])
    if [row.get("family") for row in hostile] != [row[0] for row in HOSTILE_FAMILIES]:
        raise ValueError("hostile-corpus family set/order drift")
    if [row.get("status") for row in hostile] != [row[1] for row in HOSTILE_FAMILIES]:
        raise ValueError("hostile-corpus evidence status drift")
    expected_counts = {
        "verifier_canaries": 19,
        "verifier_canaries_caught": 19,
        "failure_seam_families": 7,
        "failure_seam_complete": 0,
        "failure_seam_partial": 4,
        "failure_seam_missing": 3,
        "hostile_families": 9,
        "hostile_host_passed": 5,
        "hostile_source_only": 0,
        "hostile_missing": 4,
    }
    if value.get("counts") != expected_counts:
        raise ValueError("adversarial evidence count drift")
    gaps = value.get("open_gaps", {})
    if gaps.get("failure_injection") != ["allocation", "queue", "io", "provider", "service", "package", "lifecycle"]:
        raise ValueError("failure-injection gaps were hidden")
    if gaps.get("hostile_corpus") != ["elf", "archive", "font", "ipc"]:
        raise ValueError("hostile-corpus gaps were hidden")
    if len(value.get("build_identity", "")) != 64:
        raise ValueError("missing build identity")
    for field in ("host_test_inventory_sha256", "host_test_receipt_sha256"):
        if len(value.get(field, "")) != 64:
            raise ValueError(f"missing {field}")


def build() -> dict:
    receipt = load(RUN_RECEIPT)
    inventory = load(INVENTORY)
    build_identity = load(HERE / "build-identity.json")["identity_sha256"]
    passed = passed_targets(receipt)
    inventory_names = {row["name"] for row in inventory["targets"]}

    failures = []
    for family, status, targets, gap in FAILURE_SEAMS:
        missing = sorted(set(targets) - passed)
        if missing:
            raise ValueError(f"{family}: mapped host proof is not passing: {missing}")
        failures.append({"family": family, "status": status, "passed_host_targets": list(targets), "open_gap": gap})

    hostile = []
    for family, status, targets, detail in HOSTILE_FAMILIES:
        missing = sorted(set(targets) - passed)
        if missing:
            raise ValueError(f"{family}: mapped hostile proof is not passing: {missing}")
        hostile.append({"family": family, "status": status, "passed_host_targets": list(targets), "detail": detail})
    if "pngtest" not in inventory_names or "pngtest" not in passed:
        raise ValueError("registered PNG hostile corpus is not passing")

    value = {
        "schema": "zlos.adversarial-registry.v1",
        "result": "PASS_WITH_OPEN_GAPS",
        "build_identity": build_identity,
        "host_test_inventory_sha256": sha256(INVENTORY),
        "host_test_receipt_sha256": sha256(RUN_RECEIPT),
        "verifier_canaries": run_canaries(),
        "failure_injection": failures,
        "hostile_corpus": hostile,
        "counts": {
            "verifier_canaries": 19,
            "verifier_canaries_caught": 19,
            "failure_seam_families": 7,
            "failure_seam_complete": 0,
            "failure_seam_partial": 4,
            "failure_seam_missing": 3,
            "hostile_families": 9,
            "hostile_host_passed": 5,
            "hostile_source_only": 0,
            "hostile_missing": 4,
        },
        "open_gaps": {
            "failure_injection": ["allocation", "queue", "io", "provider", "service", "package", "lifecycle"],
            "hostile_corpus": ["elf", "archive", "font", "ipc"],
        },
        "evidence_ceiling": "host canary and hostile-input evidence only; no completeness or target-hardware promotion",
        "weakest_link": "no failure family is exhaustive; ELF/archive/font/IPC corpora are absent",
        "generator": {"path": "kernel/gen-adversarial-registry.py", "sha256": sha256(Path(__file__).resolve())},
    }
    validate(value)
    return value


def selftest(value: dict) -> None:
    mutations = {}
    missing_canary = copy.deepcopy(value)
    missing_canary["verifier_canaries"].pop()
    mutations["missing-canary"] = missing_canary
    hidden_failure = copy.deepcopy(value)
    hidden_failure["open_gaps"]["failure_injection"].remove("provider")
    mutations["hidden-failure-gap"] = hidden_failure
    promoted_elf = copy.deepcopy(value)
    next(row for row in promoted_elf["hostile_corpus"] if row["family"] == "elf")["status"] = "HOST_PASSED"
    mutations["unearned-elf-promotion"] = promoted_elf
    hidden_hostile = copy.deepcopy(value)
    hidden_hostile["open_gaps"]["hostile_corpus"].remove("ipc")
    mutations["hidden-hostile-gap"] = hidden_hostile
    identity = copy.deepcopy(value)
    identity["build_identity"] = "short"
    mutations["missing-build-identity"] = identity
    caught = []
    for name, mutated in mutations.items():
        try:
            validate(mutated)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"adversarial-registry selftest mutation escaped: {name}")
    print("adversarial-registry selftest: caught " + ", ".join(caught))


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
        if args.check and (not OUTPUT.is_file() or load(OUTPUT) != value):
            raise ValueError("adversarial-registry.json is missing or stale")
        counts = value["counts"]
        print(
            "adversarial-registry: PASS_WITH_OPEN_GAPS: "
            f"{counts['verifier_canaries_caught']}/{counts['verifier_canaries']} canaries caught, "
            f"{counts['hostile_host_passed']}/{counts['hostile_families']} hostile families host-passed"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.TimeoutExpired) as error:
        print(f"adversarial-registry: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
