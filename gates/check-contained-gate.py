#!/usr/bin/env python3
"""Mutation-check the host-containment contract for the complete zlOS gate."""

from __future__ import annotations

import argparse
from pathlib import Path


HERE = Path(__file__).resolve().parent
LAUNCHER = HERE / "run-land-gate-contained.sh"
HOSTED_LAUNCHER = HERE / "run-land-gate-hosted.sh"
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
    "tools/generators/gen-build-identity.py\" --write --selftest",
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
    'if [ "${ZLOS_CONTAINED_GATE:-}" = "1" ]',
    '[[ "$cgroup_path" == *"/zlos-master-land-gate.service" ]]',
    '[ "${ZLOS_HOSTED_GATE:-}" = "1" ]',
    '[ "${RUNNER_ENVIRONMENT:-}" = "github-hosted" ]',
    '[ "${ZLOS_HOSTED_GATE_RUN_ID:-}" = "$GITHUB_RUN_ID" ]',
    '[ "${ZLOS_HOSTED_GATE_WORKSPACE:-}" = "$WT" ]',
    "refusing unrestricted execution",
    "use the contained local or GitHub-hosted launcher",
    'run "contained gate launcher contract"',
)

HOSTED_LAUNCHER_REQUIREMENTS = (
    '[ "${GITHUB_ACTIONS:-}" = "true" ]',
    '[ "${CI:-}" = "true" ]',
    '[ "${RUNNER_ENVIRONMENT:-}" = "github-hosted" ]',
    '[ "${RUNNER_OS:-}" = "Linux" ]',
    '[[ "${GITHUB_RUN_ID:-}" =~ ^[0-9]+$ ]]',
    '[[ "${GITHUB_RUN_ATTEMPT:-}" =~ ^[0-9]+$ ]]',
    '[ "$workspace" = "$WT" ]',
    "foreign_process_matches '[l]and-gate\\.sh'",
    "foreign_process_matches '[q]emu-system'",
    "pgrep -x gcc",
    "pgrep -x clang",
    "-name '*.o.tmp'",
    'MEM_MIN_MIB=4000',
    'DISK_MIN_MIB=6000',
    'GATE_TIMEOUT=5h',
    "verify-sources.sh\" --selftest-recovery",
    "verify-sources.sh\" --recover-only",
    '[ ! -x "$WT/compile" ]',
    '(cd "$WT" && ./build.sh)',
    "tools/generators/gen-build-identity.py\" --write --selftest",
    "tools/generators/gen-dependency-lock.py\" --write --selftest",
    '--hydrate --cache-root "$CACHE_BASE" --selftest',
    '--check --cache-root "$CACHE_BASE" --selftest',
    'ulimit -c 0',
    'ulimit -f 8388608',
    'ulimit -u 384',
    'timeout --foreground --kill-after=30s "$GATE_TIMEOUT"',
    'ZLOS_HOSTED_GATE=1',
    'ZLOS_HOSTED_GATE_RUN_ID="$GITHUB_RUN_ID"',
    'ZLOS_HOSTED_GATE_WORKSPACE="$WT"',
    'ZLOS_DEPENDENCY_CACHE="$CACHE_BASE"',
)


def failures(launcher: str, hosted_launcher: str, land_gate: str) -> list[str]:
    errors = [
        f"launcher is missing: {item}"
        for item in LAUNCHER_REQUIREMENTS
        if item not in launcher
    ]
    errors.extend(
        f"hosted launcher is missing: {item}"
        for item in HOSTED_LAUNCHER_REQUIREMENTS
        if item not in hosted_launcher
    )
    errors.extend(
        f"raw gate is missing: {item}"
        for item in LAND_GATE_REQUIREMENTS
        if item not in land_gate
    )
    if "nohup bash gates/land-gate.sh" in land_gate:
        errors.append("raw gate still recommends an unrestricted background run")
    return errors


def selftest(launcher: str, hosted_launcher: str, land_gate: str) -> None:
    mutations = (
        (
            launcher.replace("--property=CPUQuota=100%", "", 1),
            hosted_launcher,
            land_gate,
            "cpu-cap",
        ),
        (
            launcher.replace("foreign_process_matches '[q]emu-system'", "true", 1),
            hosted_launcher,
            land_gate,
            "qemu-refusal",
        ),
        (
            launcher,
            hosted_launcher,
            land_gate.replace("/zlos-master-land-gate.service", "/anywhere", 1),
            "cgroup-binding",
        ),
        (
            launcher,
            hosted_launcher.replace(
                '[ "${RUNNER_ENVIRONMENT:-}" = "github-hosted" ]', "true", 1
            ),
            land_gate,
            "self-hosted-refusal",
        ),
        (
            launcher,
            hosted_launcher.replace("ulimit -u 384", "", 1),
            land_gate,
            "hosted-process-cap",
        ),
        (
            launcher,
            hosted_launcher,
            land_gate.replace("refusing unrestricted execution", "", 1),
            "raw-refusal",
        ),
    )
    for changed_launcher, changed_hosted, changed_gate, label in mutations:
        if not failures(changed_launcher, changed_hosted, changed_gate):
            raise AssertionError(f"mutation escaped: {label}")
    print(
        "contained-gate selftest: caught cpu-cap, qemu-refusal, cgroup-binding, "
        "self-hosted-refusal, hosted-process-cap and raw-refusal mutations"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    launcher = LAUNCHER.read_text()
    hosted_launcher = HOSTED_LAUNCHER.read_text()
    land_gate = LAND_GATE.read_text()
    if args.selftest:
        selftest(launcher, hosted_launcher, land_gate)
    errors = failures(launcher, hosted_launcher, land_gate)
    if errors:
        print("contained-gate contract: FAIL")
        for error in errors:
            print(f"  {error}")
        return 1
    total = (
        len(LAUNCHER_REQUIREMENTS)
        + len(HOSTED_LAUNCHER_REQUIREMENTS)
        + len(LAND_GATE_REQUIREMENTS)
    )
    print(f"contained-gate contract: PASS: {total} required controls")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
