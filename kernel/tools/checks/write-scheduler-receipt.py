#!/usr/bin/env python3
"""Validate native-UEFI scheduler observations and write a build-bound receipt."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
IDENTITY = KERNEL_ROOT / "metadata/build-identity.json"
ARTIFACT = KERNEL_ROOT / "zlOS-usb.img"
HARNESS = KERNEL_ROOT / "tools/checks/verify-efi.sh"
USERMODE = KERNEL_ROOT / "src/arch/x86/usermode.c"
IDT = KERNEL_ROOT / "src/arch/x86/idt.c"
PROCESS_LIFECYCLE_HEADER = KERNEL_ROOT / "src/core/process_lifecycle.h"
PROCESS_LIFECYCLE = KERNEL_ROOT / "src/core/process_lifecycle.c"
PROCESS_LIFECYCLE_TEST = KERNEL_ROOT / "tests/host/processlifecycletest.c"
SCHEDULER_POLICY_HEADER = KERNEL_ROOT / "src/core/scheduler_policy.h"
SCHEDULER_POLICY = KERNEL_ROOT / "src/core/scheduler_policy.c"
SCHEDULER_POLICY_TEST = KERNEL_ROOT / "tests/host/schedulerpolicytest.c"
PROCESS_SERVICE_HEADER = KERNEL_ROOT / "src/core/user_process_service.h"
PROCESS_SERVICE = KERNEL_ROOT / "src/core/user_process_service.c"
PROCESS_SERVICE_TEST = KERNEL_ROOT / "tests/host/userprocessservicetest.c"
HOST_RECEIPT = KERNEL_ROOT / "tests/host/test-run-receipt.json"
DEFAULT_OUTPUT = KERNEL_ROOT / "docs/receipts/scheduler-native-uefi64-qemu-2026-08-29.json"

ASSERTIONS = (
    {
        "id": "cooperative-context-resume",
        "marker": "<- two PML4 processes yielded/resumed AB12 and exited independently",
        "processes": 2,
        "trace": "AB12",
        "separate_address_spaces": True,
        "separate_kernel_stacks": True,
    },
    {
        "id": "timer-preemption",
        "marker": "<- PIT preempted two non-yielding Ring-3 loops PQ",
        "processes": 2,
        "trace": "PQ",
        "non_yielding": True,
        "minimum_preemptions": 2,
    },
    {
        "id": "faulted-process-sibling-survival",
        "marker": "<- one process GP-faulted; its sibling ran and exited",
        "fault_vector": 13,
        "sibling_trace": "K",
        "kernel_survived": True,
    },
    {
        "id": "persistent-process-service",
        "marker": "<- persistent service scheduled cooperative ST12 across four kernel turns; exact exit custody reaped",
        "processes": 2,
        "kernel_turns": 4,
        "trace": "ST12",
        "selection": "round-robin exact lifecycle handle",
        "turn_boundary": "cooperative yield or exit; timer preemption is proved by the independent non-yielding-process assertion",
        "exit_statuses": [11, 22],
        "terminal_custody_observed": True,
        "scheduler_detached_before_identity_reap": True,
        "physical_frame_baseline_restored": True,
    },
    {
        "id": "persistent-process-sleep",
        "marker": "<- persistent sleep LSW: injected deadline held, sibling exited, wake and frames reclaimed",
        "trace": "LSW",
        "clock": "injected scheduler ticks; no wall-clock latency claim",
        "syscall": 25,
        "zero_delay_refused": True,
        "no_early_dispatch": True,
        "sibling_exit_status": 44,
        "wake_exit_status": 33,
        "physical_frame_baseline_restored": True,
    },
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def expected_implementation() -> list[dict]:
    files = (
        ("kernel/src/arch/x86/usermode.c", USERMODE),
        ("kernel/src/arch/x86/idt.c", IDT),
        ("kernel/src/core/process_lifecycle.h", PROCESS_LIFECYCLE_HEADER),
        ("kernel/src/core/process_lifecycle.c", PROCESS_LIFECYCLE),
        ("kernel/tests/host/processlifecycletest.c", PROCESS_LIFECYCLE_TEST),
        ("kernel/src/core/scheduler_policy.h", SCHEDULER_POLICY_HEADER),
        ("kernel/src/core/scheduler_policy.c", SCHEDULER_POLICY),
        ("kernel/tests/host/schedulerpolicytest.c", SCHEDULER_POLICY_TEST),
        ("kernel/src/core/user_process_service.h", PROCESS_SERVICE_HEADER),
        ("kernel/src/core/user_process_service.c", PROCESS_SERVICE),
        ("kernel/tests/host/userprocessservicetest.c", PROCESS_SERVICE_TEST),
    )
    return [{"path": name, "sha256": digest(path)} for name, path in files]


def host_observation() -> dict:
    receipt = json.loads(HOST_RECEIPT.read_text())
    identity = json.loads(IDENTITY.read_text())["identity_sha256"]
    if receipt.get("build_identity") != identity:
        raise ValueError("scheduler host receipt is from a foreign build")
    observed = {}
    for target, minimum in (("processlifecycletest", 80),
                            ("schedulerpolicytest", 100),
                            ("userprocessservicetest", 100)):
        rows = [row for row in receipt.get("results", [])
                if row.get("name") == target]
        if len(rows) != 1 or rows[0].get("status") != "passed":
            raise ValueError(f"current host receipt has no passing {target}")
        commands = rows[0].get("commands", [])
        match = re.search(rf"{target}: ([1-9][0-9]*) checks, 0 failures",
                          commands[0].get("output_tail", "") if len(commands) == 1 else "")
        if not match or int(match.group(1)) < minimum:
            raise ValueError(f"{target} lost its bounded observations")
        observed[target] = {"checks": int(match.group(1))}
    return {
        "path": "kernel/tests/host/test-run-receipt.json",
        "sha256": digest(HOST_RECEIPT),
        "targets": observed,
    }


def validate_log(log: str) -> None:
    for assertion in ASSERTIONS:
        marker = assertion["marker"]
        if log.count(marker) != 1:
            raise ValueError(
                f"scheduler marker count for {assertion['id']} is {log.count(marker)}, expected 1"
            )
    rejected = (
        "multi-process yield/resume FAILED",
        "timer process preemption FAILED",
        "sibling fault isolation FAILED",
        "persistent user-process service FAILED",
        "persistent sleep deadline FAILED",
    )
    present = [marker for marker in rejected if marker in log]
    if present:
        raise ValueError("scheduler failure marker present: " + ", ".join(present))


def build(log_path: Path) -> dict:
    log = log_path.read_text(encoding="latin-1").replace("\r", "")
    validate_log(log)
    identity = json.loads(IDENTITY.read_text())
    value = {
        "schema": "zlos.scheduler-native-uefi64-qemu-receipt.v1",
        "result": "PASS_BOUNDED_PERSISTENT_PROCESS_GATE",
        "build_identity": identity["identity_sha256"],
        "route": "native-uefi64",
        "artifact": {
            "path": "kernel/zlOS-usb.img",
            "sha256": digest(ARTIFACT),
        },
        "boot_log_sha256": digest(log_path),
        "harness": {
            "path": "kernel/tools/checks/verify-efi.sh",
            "sha256": digest(HARNESS),
        },
        "implementation": expected_implementation(),
        "host_receipt": host_observation(),
        "assertions": [dict(assertion) for assertion in ASSERTIONS],
        "known_gaps": [
            "the persistent Ring-3 service has exactly two fixed process slots",
            "there is no general priority or deadline contract beyond bounded round robin",
            "there is no per-CPU run-queue ownership or process migration",
            "spawn and reap are kernel commands; there is no userspace process-management or cancellation API",
            "the separate eight-slot kernel task demo is cooperative and lacks FPU/SSE state",
            "there is no current physical-hardware scheduler receipt",
        ],
        "evidence_ceiling": (
            "exact current host policy/service tests and native-UEFI64 QEMU artifact "
            "with five bounded two-process observations; not a general per-CPU "
            "scheduler, userspace process API or physical-hardware qualification"
        ),
        "generator": {
            "path": "kernel/tools/checks/write-scheduler-receipt.py",
            "sha256": digest(Path(__file__).resolve()),
        },
    }
    validate(value)
    return value


def validate(value: dict) -> None:
    identity = json.loads(IDENTITY.read_text())["identity_sha256"]
    if value.get("schema") != "zlos.scheduler-native-uefi64-qemu-receipt.v1" \
            or value.get("result") != "PASS_BOUNDED_PERSISTENT_PROCESS_GATE" \
            or value.get("route") != "native-uefi64":
        raise ValueError("wrong scheduler receipt schema/result/route")
    if value.get("build_identity") != identity:
        raise ValueError("scheduler receipt is not bound to the current build identity")
    if value.get("artifact") != {
            "path": "kernel/zlOS-usb.img", "sha256": digest(ARTIFACT)}:
        raise ValueError("scheduler receipt does not bind the exact current USB artifact")
    if value.get("harness") != {
            "path": "kernel/tools/checks/verify-efi.sh", "sha256": digest(HARNESS)}:
        raise ValueError("scheduler receipt does not bind the current verifier")
    if value.get("implementation") != expected_implementation():
        raise ValueError("scheduler receipt implementation identity drifted")
    if value.get("host_receipt") != host_observation():
        raise ValueError("scheduler host observation drifted")
    if value.get("assertions") != [dict(assertion) for assertion in ASSERTIONS]:
        raise ValueError("scheduler receipt assertion set drifted")
    if len(value.get("boot_log_sha256", "")) != 64 \
            or len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("scheduler receipt evidence identity is missing")
    gaps = value.get("known_gaps", [])
    if len(gaps) != 6 or not any("physical" in gap for gap in gaps):
        raise ValueError("scheduler receipt hides its bounded or physical gaps")


def selftest(value: dict) -> None:
    mutations = {}
    identity = copy.deepcopy(value)
    identity["build_identity"] = "0" * 64
    mutations["foreign-build"] = identity
    artifact = copy.deepcopy(value)
    artifact["artifact"]["sha256"] = "0" * 64
    mutations["foreign-artifact"] = artifact
    missing = copy.deepcopy(value)
    missing["assertions"].pop()
    mutations["missing-assertion"] = missing
    yielding = copy.deepcopy(value)
    yielding["assertions"][1]["non_yielding"] = False
    mutations["invented-cooperative-preemption"] = yielding
    gaps = copy.deepcopy(value)
    gaps["known_gaps"] = []
    mutations["hidden-gaps"] = gaps
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except (OSError, ValueError, TypeError, json.JSONDecodeError):
            caught.append(name)
        else:
            raise ValueError(f"scheduler receipt mutation escaped: {name}")
    print("scheduler receipt selftest: caught " + ", ".join(caught))


def write_atomic(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", dir=path.parent, delete=False,
                                     encoding="utf-8") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--log", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    try:
        value = build(args.log)
        if args.selftest:
            selftest(value)
        output = args.output.resolve()
        write_atomic(output, value)
        print(f"scheduler QEMU receipt: PASS -> {output.relative_to(KERNEL_ROOT)}")
        return 0
    except (OSError, ValueError, TypeError, json.JSONDecodeError) as error:
        print(f"scheduler QEMU receipt: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
