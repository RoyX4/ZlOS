#!/usr/bin/env python3
"""Mutation-check the host-containment contract for the complete zlOS gate."""

from __future__ import annotations

import argparse
from pathlib import Path


HERE = Path(__file__).resolve().parent
LAUNCHER = HERE / "run-land-gate-contained.sh"
LAND_GATE = HERE / "land-gate.sh"

LAUNCHER_REQUIREMENTS = (
    "foreign_process_matches '[l]and-gate\\.sh'",
    "foreign_process_matches '[q]emu-system'",
    "pgrep -x gcc",
    "pgrep -x clang",
    "-name '*.o.tmp'",
    "LOAD_MAX=\"1.50\"",
    "MEM_MIN_MIB=8192",
    "verify-sources.sh\" --selftest-recovery",
    "verify-sources.sh\" --recover-only",
    "tools/generators/gen-build-identity.py\" --check --selftest",
    "--setenv=ZLOS_CONTAINED_GATE=1",
    "--property=CPUQuota=100%",
    "--property=CPUWeight=10",
    "--property=IOWeight=10",
    "--property=Nice=10",
    "--property=MemoryHigh=4G",
    "--property=MemoryMax=6G",
    "--property=MemorySwapMax=2G",
    "--property=TasksMax=128",
    "--property=OOMPolicy=stop",
    "--property=KillMode=control-group",
    "--property=TimeoutStopSec=30s",
    "--property=StandardOutput=journal",
    "--property=StandardError=journal",
)

LAND_GATE_REQUIREMENTS = (
    'if [ "${ZLOS_CONTAINED_GATE:-}" != "1" ]',
    '[[ "$cgroup_path" != *"/zlos-master-land-gate.service" ]]',
    "refusing unrestricted execution",
    "run-land-gate-contained.sh start",
    'run "contained gate launcher contract"',
)


def failures(launcher: str, land_gate: str) -> list[str]:
    errors = [
        f"launcher is missing: {item}"
        for item in LAUNCHER_REQUIREMENTS
        if item not in launcher
    ]
    errors.extend(
        f"raw gate is missing: {item}"
        for item in LAND_GATE_REQUIREMENTS
        if item not in land_gate
    )
    if "nohup bash gates/land-gate.sh" in land_gate:
        errors.append("raw gate still recommends an unrestricted background run")
    return errors


def selftest(launcher: str, land_gate: str) -> None:
    mutations = (
        (launcher.replace("--property=CPUQuota=100%", "", 1), land_gate, "cpu-cap"),
        (
            launcher.replace("foreign_process_matches '[q]emu-system'", "true", 1),
            land_gate,
            "qemu-refusal",
        ),
        (
            launcher,
            land_gate.replace("/zlos-master-land-gate.service", "/anywhere", 1),
            "cgroup-binding",
        ),
        (launcher, land_gate.replace("refusing unrestricted execution", "", 1), "raw-refusal"),
    )
    for changed_launcher, changed_gate, label in mutations:
        if not failures(changed_launcher, changed_gate):
            raise AssertionError(f"mutation escaped: {label}")
    print(
        "contained-gate selftest: caught cpu-cap, qemu-refusal, cgroup-binding "
        "and raw-refusal mutations"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    launcher = LAUNCHER.read_text()
    land_gate = LAND_GATE.read_text()
    if args.selftest:
        selftest(launcher, land_gate)
    errors = failures(launcher, land_gate)
    if errors:
        print("contained-gate contract: FAIL")
        for error in errors:
            print(f"  {error}")
        return 1
    total = len(LAUNCHER_REQUIREMENTS) + len(LAND_GATE_REQUIREMENTS)
    print(f"contained-gate contract: PASS: {total} required controls")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
