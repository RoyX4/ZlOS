#!/usr/bin/env python3
"""Measure the current host-test build without changing product artifacts."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import platform
import statistics
import subprocess
import tempfile
import time
from datetime import datetime, timezone
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
HOST_ROOT = KERNEL_ROOT / "tests/host"
METADATA = KERNEL_ROOT / "metadata"
BUILD_SCRIPT = HOST_ROOT / "build.sh"
HOST_RECEIPT = HOST_ROOT / "test-run-receipt.json"
TEST_INVENTORY = METADATA / "test-inventory.json"
OUTPUT = KERNEL_ROOT / "docs/receipts/build-benchmark-host-2026-08-29.json"
SAMPLE_RUNS = 7
BUDGET_US = 60_000_000.0
RUNAWAY_TIMEOUT_S = 600


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def cpu_model() -> str:
    for line in Path("/proc/cpuinfo").read_text().splitlines():
        if line.startswith("model name"):
            return line.split(":", 1)[1].strip()
    return "unknown"


def inventory_counts() -> dict[str, int]:
    value = load(TEST_INVENTORY)
    counts = value.get("counts", {})
    required = ("compiled", "auto_run_targets")
    if any(not isinstance(counts.get(key), int) or counts[key] < 1 for key in required):
        raise ValueError("test inventory has no usable build/automatic target counts")
    return counts


def workload_scope() -> str:
    counts = inventory_counts()
    return (
        f"{counts['compiled']} compiled host-test targets; output identity "
        f"verification covers {counts['auto_run_targets']} automatic targets; "
        "not a product artifact build"
    )


def expected_executables(receipt: dict) -> dict[str, str]:
    rows = {
        row["name"]: row["executable_sha256"]
        for row in receipt.get("results", [])
        if row.get("executable_sha256")
    }
    expected = inventory_counts()["auto_run_targets"]
    if len(rows) != expected:
        raise ValueError(
            f"host receipt exposes {len(rows)} executable identities, expected {expected}"
        )
    return rows


def verify_executables(expected: dict[str, str]) -> None:
    drift = []
    for name, digest in expected.items():
        path = HOST_ROOT / name
        if not path.is_file() or sha256(path) != digest:
            drift.append(name)
    if drift:
        raise ValueError(f"host build changed current executable identities: {drift}")


def nearest_rank_p95(samples: list[float]) -> float:
    ordered = sorted(samples)
    index = max(0, min(len(ordered) - 1, int(len(ordered) * 0.95 + 0.999999) - 1))
    return ordered[index]


def execute() -> dict:
    host = load(HOST_RECEIPT)
    identity = load(METADATA / "build-identity.json")["identity_sha256"]
    if host.get("outcome") != "PASS" or host.get("build_identity") != identity \
            or host.get("inventory_sha256") != sha256(TEST_INVENTORY):
        raise ValueError("host receipt is failed, foreign or stale")
    expected = expected_executables(host)
    verify_executables(expected)

    samples_us = []
    raw_output = []
    for sample in range(SAMPLE_RUNS):
        started = time.monotonic_ns()
        completed = subprocess.run(
            [str(BUILD_SCRIPT)],
            cwd=HOST_ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=RUNAWAY_TIMEOUT_S,
            check=False,
        )
        elapsed_us = (time.monotonic_ns() - started) / 1000.0
        if completed.returncode != 0:
            raise ValueError(f"host build sample {sample + 1} failed with exit {completed.returncode}")
        verify_executables(expected)
        samples_us.append(elapsed_us)
        raw_output.append(completed.stdout.decode("utf-8", errors="replace"))

    p50 = statistics.median(samples_us)
    p95 = nearest_rank_p95(samples_us)
    peak = max(samples_us)
    within = p95 <= BUDGET_US
    return {
        "schema": "zlos.host-build-benchmark-receipt.v1",
        "result": "PASS" if within else "PASS_WITH_OPEN_REGRESSION",
        "recorded_utc": datetime.now(timezone.utc).isoformat(),
        "build_identity": identity,
        "host": {
            "node": platform.node(),
            "system": platform.system(),
            "release": platform.release(),
            "machine": platform.machine(),
            "cpu_model": cpu_model(),
            "logical_cpu_count": os.cpu_count(),
            "load_average_at_start": list(os.getloadavg()),
        },
        "workload": {
            "name": "host-test-suite-build",
            "command": ["kernel/tests/host/build.sh"],
            "scope": workload_scope(),
            "script_sha256": sha256(BUILD_SCRIPT),
            "test_inventory_sha256": sha256(TEST_INVENTORY),
            "host_receipt_sha256": sha256(HOST_RECEIPT),
            "verified_executables": len(expected),
            "executable_identities_unchanged_after_every_sample": True,
        },
        "method": {
            "clock": "host monotonic wall clock",
            "sampling": "7 independent sequential full host-test build-script runs",
            "percentiles": "p50 and nearest-rank p95",
            "peak": "maximum of 7 samples",
            "budget": "p95 <= 60 seconds as a host runaway/regression guard",
            "runaway_timeout_seconds": RUNAWAY_TIMEOUT_S,
        },
        "samples_us": samples_us,
        "sample_count": len(samples_us),
        "p50_us": p50,
        "p95_us": p95,
        "peak_us": peak,
        "budget_us": BUDGET_US,
        "within_budget": within,
        "raw_output_sha256": [sha256_bytes(item.encode()) for item in raw_output],
        "raw_output": raw_output,
        "open_regressions": [] if within else ["host-test-suite-build"],
        "evidence_ceiling": "current host-test build distribution with stable executable identities; not the product kernel/image build, QEMU or physical hardware",
        "weakest_link": "the product artifact build and target-side build remain unmeasured",
        "runner": {
            "path": "kernel/tools/run/run-build-benchmark.py",
            "sha256": sha256(Path(__file__).resolve()),
        },
    }


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.host-build-benchmark-receipt.v1":
        raise ValueError("wrong host-build benchmark schema")
    identity = load(METADATA / "build-identity.json")["identity_sha256"]
    if value.get("build_identity") != identity:
        raise ValueError("host-build benchmark belongs to another build")
    workload = value.get("workload", {})
    if workload.get("name") != "host-test-suite-build" \
            or workload.get("command") != ["kernel/tests/host/build.sh"] \
            or workload.get("scope") != workload_scope() \
            or workload.get("script_sha256") != sha256(BUILD_SCRIPT) \
            or workload.get("test_inventory_sha256") != sha256(TEST_INVENTORY) \
            or workload.get("host_receipt_sha256") != sha256(HOST_RECEIPT) \
            or workload.get("verified_executables") != inventory_counts()["auto_run_targets"] \
            or workload.get("executable_identities_unchanged_after_every_sample") is not True:
        raise ValueError("host-build benchmark inputs or output identities drifted")
    verify_executables(expected_executables(load(HOST_RECEIPT)))
    samples = value.get("samples_us", [])
    if len(samples) != SAMPLE_RUNS or value.get("sample_count") != SAMPLE_RUNS:
        raise ValueError("host-build sample distribution is incomplete")
    if value.get("p50_us") != statistics.median(samples) \
            or value.get("p95_us") != nearest_rank_p95(samples) \
            or value.get("peak_us") != max(samples):
        raise ValueError("host-build distribution summary drifted")
    if value.get("method", {}).get("runaway_timeout_seconds") != RUNAWAY_TIMEOUT_S:
        raise ValueError("host-build runaway timeout drifted")
    within = value.get("p95_us", BUDGET_US + 1) <= BUDGET_US
    if value.get("budget_us") != BUDGET_US or value.get("within_budget") != within \
            or value.get("result") != ("PASS" if within else "PASS_WITH_OPEN_REGRESSION") \
            or value.get("open_regressions") != ([] if within else ["host-test-suite-build"]):
        raise ValueError("host-build budget result or regression list drifted")
    raw = value.get("raw_output", [])
    if len(raw) != SAMPLE_RUNS \
            or [sha256_bytes(item.encode()) for item in raw] != value.get("raw_output_sha256"):
        raise ValueError("host-build raw output drifted")
    if value.get("runner", {}).get("sha256") != sha256(Path(__file__).resolve()):
        raise ValueError("host-build benchmark runner changed after the receipt")


def selftest(value: dict) -> None:
    mutations = {}
    summary = copy.deepcopy(value)
    summary["p95_us"] += 1
    mutations["distribution-summary-drift"] = summary
    identity = copy.deepcopy(value)
    identity["build_identity"] = "0" * 64
    mutations["foreign-build"] = identity
    output = copy.deepcopy(value)
    output["raw_output"][0] += "mutation"
    mutations["raw-output-drift"] = output
    executable = copy.deepcopy(value)
    executable["workload"]["verified_executables"] -= 1
    mutations["missing-executable-identity"] = executable
    target = copy.deepcopy(value)
    target["workload"]["scope"] = "native zlOS product build"
    mutations["target-overclaim"] = target
    timeout = copy.deepcopy(value)
    timeout["method"]["runaway_timeout_seconds"] = 0
    mutations["runaway-timeout-drift"] = timeout
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"host-build benchmark selftest mutation escaped: {name}")
    print("host-build benchmark selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", dir=OUTPUT.parent, delete=False,
                                     encoding="utf-8") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, OUTPUT)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.run == args.check:
        parser.error("choose exactly one of --run or --check")
    try:
        if args.run:
            value = execute()
            validate(value)
            if args.selftest:
                selftest(value)
            write_atomic(value)
        else:
            value = load(OUTPUT)
            validate(value)
            if args.selftest:
                selftest(value)
        print(
            "host-build benchmark: " + value["result"] + ": "
            f"p95={value['p95_us'] / 1_000_000.0:.3f}s, "
            f"peak={value['peak_us'] / 1_000_000.0:.3f}s, "
            f"samples={value['sample_count']}"
        )
        return 0
    except (OSError, ValueError, KeyError, TypeError, json.JSONDecodeError,
            subprocess.TimeoutExpired) as error:
        print(f"host-build benchmark: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
