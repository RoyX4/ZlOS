#!/usr/bin/env python3
"""Run and validate the current deterministic host performance receipt."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import platform
import re
import statistics
import subprocess
import tempfile
from datetime import datetime, timezone
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
METADATA = KERNEL_ROOT / "metadata"
OUTPUT = KERNEL_ROOT / "docs/receipts/benchmark-host-2026-08-23.json"
BENCHMARKS = (
    ("wmbench", KERNEL_ROOT / "tests/host/wmbench"),
    ("fbbench", KERNEL_ROOT / "tests/host/fbbench"),
)
FRAME_BUDGET_US = 16667.0
SAMPLE_RUNS = 7
LEGACY_V1_RUNNER_SHA256 = "f2149e243cc5564d8e51cc86477bac5d9554a83cb5fdd3dfe58c3adb7de4133c"


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def cpu_model() -> str:
    for line in Path("/proc/cpuinfo").read_text().splitlines():
        if line.startswith("model name"):
            return line.split(":", 1)[1].strip()
    return "unknown"


def metric(name: str, value_us: float, route: str, evidence: str) -> dict:
    return {
        "name": name,
        "route": route,
        "frame_us": value_us,
        "budget_us": FRAME_BUDGET_US,
        "within_60hz_budget": value_us <= FRAME_BUDGET_US,
        "evidence": evidence,
    }


def aggregate(rows_by_run: list[list[dict]]) -> list[dict]:
    names = [[row["name"] for row in rows] for rows in rows_by_run]
    if not names or any(current != names[0] for current in names):
        raise ValueError("benchmark metric identity drift between sample runs")
    result = []
    for index, name in enumerate(names[0]):
        samples = [rows[index]["frame_us"] for rows in rows_by_run]
        ordered = sorted(samples)
        p50 = statistics.median(ordered)
        p95 = ordered[max(0, min(len(ordered) - 1, int(len(ordered) * 0.95 + 0.999999) - 1))]
        peak = ordered[-1]
        row = copy.deepcopy(rows_by_run[0][index])
        row.update({
            "samples_us": samples,
            "sample_count": len(samples),
            "p50_us": p50,
            "p95_us": p95,
            "peak_us": peak,
            "frame_us": p95,
            "summary_statistic": "p95",
            "within_60hz_budget": p95 <= FRAME_BUDGET_US,
        })
        result.append(row)
    return result


def parse_wm(output: str) -> list[dict]:
    match = re.search(r"^  everything\s+\d+ cyc/frame\s+([0-9.]+) us/frame$", output, re.MULTILINE)
    if not match:
        raise ValueError("wmbench full-drag metric missing")
    return [metric("wm-drag-full", float(match.group(1)), "host shipping compositor logic", "T-host")]


def parse_fb(output: str) -> list[dict]:
    sections = re.split(r"(?=^=== \d+x\d+)", output, flags=re.MULTILINE)[1:]
    expected_modes = [(1920, 1200), (2560, 1440), (3840, 2160)]
    if len(sections) != len(expected_modes):
        raise ValueError("fbbench mode section count drift")
    rows = []
    for section, (width, height) in zip(sections, expected_modes):
        heading = re.match(r"=== (\d+)x(\d+)", section)
        if not heading or (int(heading.group(1)), int(heading.group(2))) != (width, height):
            raise ValueError("fbbench mode order drift")
        serial = re.search(r"^  WHOLE DESKTOP redraw\s+([0-9.]+) ms", section, re.MULTILINE)
        bands = re.search(r"^    4 bands\s+([0-9.]+)ms", section, re.MULTILINE)
        if not serial or not bands:
            raise ValueError(f"fbbench {width}x{height} metrics missing")
        rows.append(metric(
            f"fb-desktop-{width}x{height}-serial",
            float(serial.group(1)) * 1000.0,
            "host shipping serial framebuffer path",
            "T-host",
        ))
        rows.append(metric(
            f"fb-desktop-{width}x{height}-four-band",
            float(bands.group(1)) * 1000.0,
            "host parallel-band experiment; not the native zlOS route",
            "T-host-experimental",
        ))
    return rows


def execute() -> dict:
    outputs = {}
    executables = []
    rows_by_benchmark = {}
    for name, path in BENCHMARKS:
        if not path.is_file() or not os.access(path, os.X_OK):
            raise ValueError(f"benchmark executable missing: {path}")
        samples = []
        parsed = []
        for run_index in range(SAMPLE_RUNS):
            completed = subprocess.run(
                [str(path)], cwd=path.parent, stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT, timeout=180, check=False,
            )
            if completed.returncode != 0:
                raise ValueError(f"{name} sample {run_index + 1} failed with exit {completed.returncode}")
            output = completed.stdout.decode("utf-8", errors="replace")
            samples.append(output)
            parsed.append(parse_wm(output) if name == "wmbench" else parse_fb(output))
        outputs[name] = samples
        rows_by_benchmark[name] = parsed
        source = path.with_suffix(".c")
        executables.append({
            "name": name,
            "path": "kernel/tests/host/" + path.name,
            "sha256": sha256(path),
            "source_path": "kernel/tests/host/" + source.name,
            "source_sha256": sha256(source),
            "raw_output_sha256": [sha256_bytes(output.encode()) for output in samples],
        })
    metrics = aggregate(rows_by_benchmark["wmbench"]) + aggregate(rows_by_benchmark["fbbench"])
    failed = [row["name"] for row in metrics if not row["within_60hz_budget"]]
    return {
        "schema": "zlos.host-benchmark-receipt.v2",
        "result": "PASS_WITH_OPEN_REGRESSIONS" if failed else "PASS",
        "recorded_utc": datetime.now(timezone.utc).isoformat(),
        "build_identity": json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"],
        "host": {
            "node": platform.node(),
            "system": platform.system(),
            "release": platform.release(),
            "machine": platform.machine(),
            "cpu_model": cpu_model(),
            "logical_cpu_count": os.cpu_count(),
            "load_average_at_start": list(os.getloadavg()),
        },
        "method": {
            "clock": "benchmark-reported TSC",
            "sampling": "7 independent process runs; each wmbench sample is best of 7 interleaved and each fbbench sample is minimum of 7 internal runs",
            "percentiles": "p50 and nearest-rank p95 retained from 7 independent process samples",
            "peak": "maximum of 7 independent process samples retained",
            "budget": "p95 <= 16667 us for 60 Hz",
        },
        "executables": executables,
        "metrics": metrics,
        "counts": {
            "measurements": len(metrics),
            "within_budget": len(metrics) - len(failed),
            "over_budget": len(failed),
            "native_target_measurements": 0,
        },
        "open_regressions": failed,
        "raw_output": outputs,
        "evidence_ceiling": "host p50/p95/peak distributions on one contended desktop; no QEMU or native-hardware promotion",
        "weakest_link": (
            (", ".join(failed) + " exceed 16.667 ms; ") if failed else ""
        ) + "target frame-time distribution is unmeasured",
        "runner": {"path": "kernel/tools/run/run-benchmarks.py", "sha256": sha256(Path(__file__).resolve())},
    }


def validate_legacy_v1(value: dict) -> None:
    expected_names = [
        "wm-drag-full", "fb-desktop-1920x1200-serial", "fb-desktop-1920x1200-four-band",
        "fb-desktop-2560x1440-serial", "fb-desktop-2560x1440-four-band",
        "fb-desktop-3840x2160-serial", "fb-desktop-3840x2160-four-band",
    ]
    metrics = value.get("metrics", [])
    if [row.get("name") for row in metrics] != expected_names:
        raise ValueError("legacy benchmark metric set/order drift")
    regressions = [row["name"] for row in metrics if not row["within_60hz_budget"]]
    if value.get("open_regressions") != regressions:
        raise ValueError("legacy measured regressions were hidden")
    if value.get("counts") != {"measurements": 7, "within_budget": 7 - len(regressions),
                               "over_budget": len(regressions), "native_target_measurements": 0}:
        raise ValueError("legacy benchmark counts drift")
    if value.get("method", {}).get("percentiles") != "NOT_RECORDED" \
            or value.get("method", {}).get("peak") != "NOT_RECORDED":
        raise ValueError("legacy receipt invented a distribution")
    if value.get("build_identity") != json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"]:
        raise ValueError("legacy benchmark belongs to another build")
    raw = value.get("raw_output", {})
    for executable in value.get("executables", []):
        name = executable.get("name")
        if name not in raw or sha256_bytes(raw[name].encode()) != executable.get("raw_output_sha256"):
            raise ValueError(f"{name}: legacy raw output drift")
        executable_path = KERNEL_ROOT / Path(executable["path"]).relative_to("kernel")
        source_path = KERNEL_ROOT / Path(executable["source_path"]).relative_to("kernel")
        if not executable_path.is_file() or sha256(executable_path) != executable["sha256"] \
                or not source_path.is_file() or sha256(source_path) != executable["source_sha256"]:
            raise ValueError(f"{name}: legacy binary/source drift")
    if value.get("runner", {}).get("sha256") != LEGACY_V1_RUNNER_SHA256:
        raise ValueError("legacy benchmark runner identity drift")


def validate(value: dict) -> None:
    if value.get("schema") == "zlos.host-benchmark-receipt.v1":
        validate_legacy_v1(value)
        return
    if value.get("schema") != "zlos.host-benchmark-receipt.v2":
        raise ValueError("wrong benchmark schema")
    expected_names = [
        "wm-drag-full",
        "fb-desktop-1920x1200-serial", "fb-desktop-1920x1200-four-band",
        "fb-desktop-2560x1440-serial", "fb-desktop-2560x1440-four-band",
        "fb-desktop-3840x2160-serial", "fb-desktop-3840x2160-four-band",
    ]
    metrics = value.get("metrics", [])
    if [row.get("name") for row in metrics] != expected_names:
        raise ValueError("benchmark metric set/order drift")
    for row in metrics:
        samples = row.get("samples_us", [])
        if len(samples) != SAMPLE_RUNS or row.get("sample_count") != SAMPLE_RUNS:
            raise ValueError(f"{row.get('name')}: sample distribution missing")
        ordered = sorted(samples)
        expected_p50 = statistics.median(ordered)
        expected_p95 = ordered[max(0, min(len(ordered) - 1, int(len(ordered) * 0.95 + 0.999999) - 1))]
        if row.get("p50_us") != expected_p50 or row.get("p95_us") != expected_p95 \
                or row.get("peak_us") != ordered[-1] or row.get("frame_us") != expected_p95:
            raise ValueError(f"{row.get('name')}: distribution summary mismatch")
        if row.get("within_60hz_budget") != (row.get("frame_us", FRAME_BUDGET_US + 1) <= FRAME_BUDGET_US):
            raise ValueError(f"{row.get('name')}: budget result is inconsistent")
    regressions = [row["name"] for row in metrics if not row["within_60hz_budget"]]
    if value.get("open_regressions") != regressions:
        raise ValueError("measured frame regressions were hidden or invented")
    expected_result = "PASS_WITH_OPEN_REGRESSIONS" if regressions else "PASS"
    if value.get("result") != expected_result:
        raise ValueError("benchmark result disagrees with measured regressions")
    expected_counts = {"measurements": 7, "within_budget": 7 - len(regressions),
                       "over_budget": len(regressions), "native_target_measurements": 0}
    if value.get("counts") != expected_counts:
        raise ValueError("benchmark counts drift")
    if "p50" not in value.get("method", {}).get("percentiles", "") \
            or "maximum" not in value.get("method", {}).get("peak", ""):
        raise ValueError("measured distribution method is missing")
    if len(value.get("build_identity", "")) != 64:
        raise ValueError("missing build identity")
    current_build_identity = json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"]
    if value.get("build_identity") != current_build_identity:
        raise ValueError("benchmark receipt belongs to another build identity")
    raw = value.get("raw_output", {})
    for executable in value.get("executables", []):
        name = executable.get("name")
        if name not in raw or len(raw[name]) != SAMPLE_RUNS \
                or [sha256_bytes(output.encode()) for output in raw[name]] != executable.get("raw_output_sha256"):
            raise ValueError(f"{name}: raw benchmark output identity mismatch")
        if len(executable.get("sha256", "")) != 64 or len(executable.get("source_sha256", "")) != 64:
            raise ValueError(f"{name}: executable/source identity missing")
        executable_path = KERNEL_ROOT / Path(executable["path"]).relative_to("kernel")
        source_path = KERNEL_ROOT / Path(executable["source_path"]).relative_to("kernel")
        if not executable_path.is_file() or sha256(executable_path) != executable["sha256"]:
            raise ValueError(f"{name}: current executable differs from measured binary")
        if not source_path.is_file() or sha256(source_path) != executable["source_sha256"]:
            raise ValueError(f"{name}: current source differs from measured source")
    if value.get("runner", {}).get("sha256") != sha256(Path(__file__).resolve()):
        raise ValueError("benchmark runner changed after the receipt")


def selftest(value: dict) -> None:
    if value.get("schema") == "zlos.host-benchmark-receipt.v1":
        mutations = {}
        hidden = copy.deepcopy(value); hidden["open_regressions"] = []; mutations["hidden-regressions"] = hidden
        promoted = copy.deepcopy(value); promoted["counts"]["native_target_measurements"] = 7; mutations["target-overclaim"] = promoted
        distribution = copy.deepcopy(value); distribution["method"]["percentiles"] = "p95"; mutations["invented-percentile"] = distribution
        raw = copy.deepcopy(value); raw["raw_output"]["wmbench"] += "mutation"; mutations["raw-output-drift"] = raw
        caught = []
        for name, mutated in mutations.items():
            try:
                validate(mutated)
            except ValueError:
                caught.append(name)
            else:
                raise ValueError(f"legacy benchmark selftest mutation escaped: {name}")
        print("benchmark legacy-v1 selftest: caught " + ", ".join(caught))
        return
    mutations = {}
    hidden = copy.deepcopy(value)
    candidate = next(row for row in hidden["metrics"] if row["within_60hz_budget"])
    candidate["frame_us"] = FRAME_BUDGET_US + 1
    candidate["within_60hz_budget"] = False
    mutations["hidden-regressions"] = hidden
    promoted = copy.deepcopy(value)
    promoted["counts"]["native_target_measurements"] = 7
    mutations["target-overclaim"] = promoted
    distribution = copy.deepcopy(value)
    distribution["metrics"][0]["p95_us"] += 1
    mutations["distribution-summary-drift"] = distribution
    identity = copy.deepcopy(value)
    identity["executables"][0]["sha256"] = "short"
    mutations["missing-executable-identity"] = identity
    raw = copy.deepcopy(value)
    raw["raw_output"]["wmbench"][0] += "mutation"
    mutations["raw-output-drift"] = raw
    caught = []
    for name, mutated in mutations.items():
        try:
            validate(mutated)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"benchmark selftest mutation escaped: {name}")
    print("benchmark selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", dir=OUTPUT.parent, delete=False, encoding="utf-8") as handle:
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
            value = json.loads(OUTPUT.read_text())
            validate(value)
            if args.selftest:
                selftest(value)
        print(
            "benchmark: " + value["result"] + ": "
            f"{value['counts']['within_budget']}/{value['counts']['measurements']} host metrics within 60 Hz budget; "
            f"{value['counts']['native_target_measurements']} native-target measurements"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.TimeoutExpired) as error:
        print(f"benchmark: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
