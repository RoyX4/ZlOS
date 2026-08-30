#!/usr/bin/env python3
"""Validate native-UEFI scheduler observations and write a build-bound receipt."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
IDENTITY = KERNEL_ROOT / "metadata/build-identity.json"
ARTIFACT = KERNEL_ROOT / "zlOS-usb.img"
HARNESS = KERNEL_ROOT / "tools/checks/verify-efi.sh"
USERMODE = KERNEL_ROOT / "src/arch/x86/usermode.c"
IDT = KERNEL_ROOT / "src/arch/x86/idt.c"
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
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


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
        "result": "PASS_BOUNDED_TWO_PROCESS_GATE",
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
        "implementation": [
            {"path": "kernel/src/arch/x86/usermode.c", "sha256": digest(USERMODE)},
            {"path": "kernel/src/arch/x86/idt.c", "sha256": digest(IDT)},
        ],
        "assertions": [dict(assertion) for assertion in ASSERTIONS],
        "known_gaps": [
            "the preemptive Ring-3 gate has exactly two fixed process slots",
            "there is no general priority, fairness or deadline contract",
            "there is no per-CPU run-queue ownership or process migration",
            "spawn, reap, cancellation and resource reclamation are not persistent services",
            "there is no current physical-hardware scheduler receipt",
        ],
        "evidence_ceiling": (
            "exact current native-UEFI64 QEMU artifact and three bounded two-process "
            "observations; not a general scheduler or physical-hardware qualification"
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
            or value.get("result") != "PASS_BOUNDED_TWO_PROCESS_GATE" \
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
    expected_implementation = [
        {"path": "kernel/src/arch/x86/usermode.c", "sha256": digest(USERMODE)},
        {"path": "kernel/src/arch/x86/idt.c", "sha256": digest(IDT)},
    ]
    if value.get("implementation") != expected_implementation:
        raise ValueError("scheduler receipt implementation identity drifted")
    if value.get("assertions") != [dict(assertion) for assertion in ASSERTIONS]:
        raise ValueError("scheduler receipt assertion set drifted")
    if len(value.get("boot_log_sha256", "")) != 64 \
            or len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("scheduler receipt evidence identity is missing")
    gaps = value.get("known_gaps", [])
    if len(gaps) != 5 or not any("physical" in gap for gap in gaps):
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
