#!/usr/bin/env python3
"""Join current frame and subsystem host budgets without inventing target latency."""

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
OUTPUT = METADATA / "performance-registry.json"
BENCHMARK_RECEIPT = KERNEL_ROOT / "docs/receipts/benchmark-host-2026-08-23.json"
HOST_RECEIPT = KERNEL_ROOT / "tests/host/test-run-receipt.json"
TEST_INVENTORY = METADATA / "test-inventory.json"
BUILD_RECEIPT = KERNEL_ROOT / "docs/receipts/build-benchmark-host-2026-08-29.json"

CATEGORY_ORDER = ("frame", "input", "io", "network", "launch", "memory", "build")
HOST_CATEGORIES = (
    ("input", ("inputtest",), 10_000.0),
    ("io", ("blocktest", "fstest"), 250_000.0),
    ("network", ("nettest", "tcptest"), 100_000.0),
    ("launch", ("exectest",), 10_000.0),
    ("memory", ("heaptest",), 100_000.0),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def host_target(receipt: dict, name: str) -> dict:
    rows = [row for row in receipt.get("results", []) if row.get("name") == name]
    if len(rows) != 1 or rows[0].get("status") != "passed":
        raise ValueError(f"current host receipt has no unique passing {name}")
    commands = rows[0].get("commands", [])
    if not commands or any(command.get("exit_code") != 0 or command.get("timed_out")
                           for command in commands):
        raise ValueError(f"{name}: current host commands are not all successful")
    return rows[0]


def host_category(receipt: dict, category: str, names: tuple[str, ...],
                  budget_us: float) -> dict:
    targets = []
    for name in names:
        row = host_target(receipt, name)
        elapsed_us = sum(command["elapsed_ns"] for command in row["commands"]) / 1000.0
        targets.append({
            "target_id": row["id"],
            "target": name,
            "elapsed_us": elapsed_us,
            "executable_sha256": row.get("executable_sha256"),
            "output_sha256": [command["output_sha256"] for command in row["commands"]],
        })
    observed_us = sum(row["elapsed_us"] for row in targets)
    return {
        "category": category,
        "status": "HOST_GATE_BUDGET_PASSED" if observed_us <= budget_us else "HOST_GATE_BUDGET_FAILED",
        "metric": "sum of current host-gate process wall times",
        "observed_us": observed_us,
        "budget_us": budget_us,
        "within_budget": observed_us <= budget_us,
        "targets": targets,
        "product_latency_claim": False,
    }


def frame_category(benchmark: dict) -> dict:
    metrics = []
    for row in benchmark.get("metrics", []):
        metrics.append({
            "name": row["name"],
            "route": row["route"],
            "sample_count": row["sample_count"],
            "p50_us": row["p50_us"],
            "p95_us": row["p95_us"],
            "peak_us": row["peak_us"],
            "budget_us": row["budget_us"],
            "within_budget": row["within_60hz_budget"],
        })
    if len(metrics) != 7:
        raise ValueError("current benchmark receipt does not contain seven frame metrics")
    failed = [row["name"] for row in metrics if not row["within_budget"]]
    return {
        "category": "frame",
        "status": "HOST_DISTRIBUTION_PASSED" if not failed else "HOST_DISTRIBUTION_FAILED",
        "metric": "p95 frame time from seven independent host process samples",
        "observed_us": max(row["p95_us"] for row in metrics),
        "budget_us": 16667.0,
        "within_budget": not failed,
        "metrics": metrics,
        "product_latency_claim": False,
    }


def build_category(receipt: dict, identity: str) -> dict:
    if receipt.get("build_identity") != identity \
            or receipt.get("schema") != "zlos.host-build-benchmark-receipt.v1" \
            or receipt.get("result") not in ("PASS", "PASS_WITH_OPEN_REGRESSION") \
            or receipt.get("sample_count") != 7:
        raise ValueError("host-build benchmark receipt is absent, foreign or incomplete")
    return {
        "category": "build",
        "status": ("HOST_BUILD_DISTRIBUTION_PASSED" if receipt["within_budget"]
                   else "HOST_BUILD_DISTRIBUTION_FAILED"),
        "metric": "p95 wall time for seven full host-test-suite builds",
        "observed_us": receipt["p95_us"],
        "budget_us": receipt["budget_us"],
        "within_budget": receipt["within_budget"],
        "sample_count": receipt["sample_count"],
        "p50_us": receipt["p50_us"],
        "p95_us": receipt["p95_us"],
        "peak_us": receipt["peak_us"],
        "verified_executables": receipt["workload"]["verified_executables"],
        "receipt_sha256": sha256(BUILD_RECEIPT),
        "product_latency_claim": False,
        "product_build_claim": False,
    }


def build() -> dict:
    benchmark = load(BENCHMARK_RECEIPT)
    host = load(HOST_RECEIPT)
    inventory = load(TEST_INVENTORY)
    build_receipt = load(BUILD_RECEIPT)
    identity = load(METADATA / "build-identity.json")["identity_sha256"]
    if benchmark.get("build_identity") != identity \
            or benchmark.get("result") not in ("PASS", "PASS_WITH_OPEN_REGRESSIONS"):
        raise ValueError("host benchmark receipt is failed or foreign")
    if host.get("build_identity") != identity or host.get("outcome") != "PASS":
        raise ValueError("host test receipt is failed or foreign")
    if host.get("inventory_sha256") != sha256(TEST_INVENTORY):
        raise ValueError("host test receipt does not bind the current inventory")

    categories = [frame_category(benchmark)]
    categories.extend(
        host_category(host, category, names, budget)
        for category, names, budget in HOST_CATEGORIES
    )
    categories.append(build_category(build_receipt, identity))
    regressions = [row["category"] for row in categories
                   if row.get("within_budget") is False]
    gaps = ["product-build", "product-latency", "native-target", "physical"]
    weakest_link = (
        "current host regressions: " + ", ".join(regressions) +
        "; product build and target latency remain unmeasured"
        if regressions else
        "product build and target latency remain unmeasured"
    )
    return {
        "schema": "zlos.performance-registry.v1",
        "result": "PASS_WITH_OPEN_GAPS" if not regressions else "OPEN_REGRESSIONS_AND_GAPS",
        "build_identity": identity,
        "inputs": {
            "benchmark_receipt": {
                "path": "kernel/docs/receipts/benchmark-host-2026-08-23.json",
                "sha256": sha256(BENCHMARK_RECEIPT),
            },
            "host_test_receipt": {
                "path": "kernel/tests/host/test-run-receipt.json",
                "sha256": sha256(HOST_RECEIPT),
            },
            "test_inventory": {
                "path": "kernel/metadata/test-inventory.json",
                "sha256": sha256(TEST_INVENTORY),
                "targets": inventory["counts"]["total"],
            },
            "build_benchmark_receipt": {
                "path": "kernel/docs/receipts/build-benchmark-host-2026-08-29.json",
                "sha256": sha256(BUILD_RECEIPT),
            },
        },
        "method": {
            "frame": "current benchmark p95 distributions",
            "subsystems": "single current host-gate process wall times, grouped by category",
            "budget_scope": "regression/runaway guard for this host gate only; not product latency",
        },
        "categories": categories,
        "counts": {
            "categories": len(categories),
            "host_budget_passed": sum(row.get("within_budget") is True for row in categories),
            "host_budget_failed": len(regressions),
            "missing": sum(row["status"] == "MISSING" for row in categories),
            "native_target_categories": 0,
            "physical_categories": 0,
        },
        "open_regressions": regressions,
        "open_gaps": gaps,
        "evidence_ceiling": "current host frame and host-test build distributions plus single-run gate wall-time guards for five categories; no product-build, product-latency, QEMU or physical promotion",
        "weakest_link": weakest_link,
        "generator": {
            "path": "kernel/tools/generators/gen-performance-registry.py",
            "sha256": sha256(Path(__file__).resolve()),
        },
    }


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.performance-registry.v1":
        raise ValueError("wrong performance-registry schema")
    if [row.get("category") for row in value.get("categories", [])] != list(CATEGORY_ORDER):
        raise ValueError("performance category set/order drift")
    current = build()
    if value.get("result") != current.get("result"):
        raise ValueError("current performance result drifted or was overpromoted")
    if value.get("counts") != current.get("counts"):
        raise ValueError("performance category counts drifted")
    if value.get("open_regressions") != current.get("open_regressions") \
            or value.get("open_gaps") != current.get("open_gaps"):
        raise ValueError("performance regressions or gaps were hidden")
    if any(row.get("product_latency_claim") is not False
           for row in value.get("categories", [])):
        raise ValueError("host gate runtime was promoted to product latency")
    if value != current:
        raise ValueError("performance registry differs from current inputs")


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["categories"].pop()
    mutations["missing-category"] = missing
    gap = copy.deepcopy(value)
    gap["open_gaps"].remove("product-build")
    mutations["hidden-product-build-gap"] = gap
    regression = copy.deepcopy(value)
    regression["open_regressions"] = []
    mutations["hidden-regression"] = regression
    summary = copy.deepcopy(value)
    summary["categories"][0]["metrics"][0]["p95_us"] += 1
    mutations["frame-summary-drift"] = summary
    identity = copy.deepcopy(value)
    identity["build_identity"] = "0" * 64
    mutations["foreign-build"] = identity
    promoted = copy.deepcopy(value)
    promoted["categories"][1]["product_latency_claim"] = True
    mutations["invented-product-latency"] = promoted
    target = copy.deepcopy(value)
    target["counts"]["native_target_categories"] = 6
    mutations["invented-target-proof"] = target
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"performance-registry selftest mutation escaped: {name}")
    print("performance-registry selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    with tempfile.NamedTemporaryFile("w", dir=OUTPUT.parent, delete=False,
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
    if not args.write and not args.check:
        args.check = True
    try:
        value = build()
        validate(value)
        if args.selftest:
            selftest(value)
        if args.write:
            write_atomic(value)
        if args.check and (not OUTPUT.is_file() or load(OUTPUT) != value):
            raise ValueError("performance-registry.json is missing or stale")
        counts = value["counts"]
        print(
            "performance-registry: " + value["result"] + ": "
            f"{counts['host_budget_passed']}/{counts['categories']} host categories passed, "
            f"{counts['host_budget_failed']} over budget, {counts['missing']} missing"
        )
        return 0
    except (OSError, ValueError, KeyError, TypeError, json.JSONDecodeError) as error:
        print(f"performance-registry: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
