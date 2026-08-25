#!/usr/bin/env python3
"""Compile and execute EV-018's host core, preserving an exact bounded receipt."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
METADATA = KERNEL_ROOT / "metadata"
RECEIPT = KERNEL_ROOT / "docs/receipts/event-trace-host-2026-08-24.json"
SOURCES = ("tests/host/trace_event.h", "tests/host/trace_event.c",
           "tests/host/eventtracetest.c")
EXPECTED_CHECKS = 37
FORBIDDEN_ENVIRONMENT = (
    "CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "LIBRARY_PATH",
    "GCC_EXEC_PREFIX", "COMPILER_PATH", "LD_PRELOAD",
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def run(argv: list[str], cwd: Path = KERNEL_ROOT) -> subprocess.CompletedProcess[str]:
    return subprocess.run(argv, cwd=cwd, text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, check=False,
                          env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:halt_on_error=1",
                               "UBSAN_OPTIONS": "halt_on_error=1"})


def tool_version(command: str) -> dict:
    resolved = shutil.which(command)
    if not resolved:
        raise ValueError(f"required compiler is missing: {command}")
    completed = run([command, "--version"])
    if completed.returncode != 0:
        raise ValueError(f"cannot identify {command}")
    return {"invoked": command, "resolved": str(Path(resolved).resolve()),
            "binary_sha256": sha256(Path(resolved).resolve()),
            "version_first_line": completed.stdout.splitlines()[0]}


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.event-trace-host-receipt.v1":
        raise ValueError("wrong event-trace receipt schema")
    if value.get("result") != "PASS_HOST_CORE_TARGET_UNINTEGRATED":
        raise ValueError("event trace was failed or overpromoted")
    if value.get("build_identity") != json.loads(
            (METADATA / "build-identity.json").read_text())["identity_sha256"]:
        raise ValueError("event-trace receipt has stale build context")
    identities = value.get("source_identities", {})
    expected = {name: sha256(KERNEL_ROOT / name) for name in SOURCES}
    if identities != expected:
        raise ValueError("event-trace source identity drift")
    proof = value.get("proof", {})
    if proof.get("host_execution", {}).get("exit_code") != 0:
        raise ValueError("event-trace host execution did not pass")
    if proof.get("host_execution", {}).get("checks") != EXPECTED_CHECKS or \
            proof.get("host_execution", {}).get("failures") != 0:
        raise ValueError("event-trace assertion count drift")
    lanes = proof.get("compile_lanes", [])
    expected_lanes = ["host-asan-ubsan", "bios-raw-ilp32", "uefi-llp64"]
    if [row.get("id") for row in lanes] != expected_lanes or \
            any(row.get("exit_code") != 0 for row in lanes):
        raise ValueError("event-trace compile-lane proof is incomplete")
    if value.get("target_integration") != {
            "compiled_into_shipped_kernel": False,
            "target_emitters": 0,
            "qemu_routes": 0,
            "native_hardware_routes": 0,
    }:
        raise ValueError("event trace invented target integration")
    if value.get("open_gaps") != [
            "single-owner-only",
            "not-compiled-into-shipped-kernel",
            "no-target-emitter",
            "no-QEMU-receipt",
            "no-native-hardware-receipt",
            "no-durable-anchor",
            "no-service-export-policy",
    ]:
        raise ValueError("event-trace gaps were hidden")


def selftest(value: dict) -> None:
    mutations = {}
    failed = copy.deepcopy(value)
    failed["proof"]["host_execution"]["exit_code"] = 1
    mutations["hidden-host-failure"] = failed
    missing = copy.deepcopy(value)
    missing["source_identities"].pop("hosttest/trace_event.c")
    mutations["missing-source"] = missing
    lane = copy.deepcopy(value)
    lane["proof"]["compile_lanes"].pop()
    mutations["missing-abi-lane"] = lane
    promoted = copy.deepcopy(value)
    promoted["target_integration"]["compiled_into_shipped_kernel"] = True
    mutations["invented-target-integration"] = promoted
    gaps = copy.deepcopy(value)
    gaps["open_gaps"].remove("single-owner-only")
    mutations["hidden-concurrency-gap"] = gaps
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"event-trace receipt mutation escaped: {name}")
    print("event-trace receipt selftest: caught " + ", ".join(caught))


def build_receipt() -> dict:
    injected = [name for name in FORBIDDEN_ENVIRONMENT if os.environ.get(name)]
    if injected:
        raise ValueError("ambient compiler environment is forbidden: " +
                         ", ".join(injected))
    gcc = tool_version("gcc")
    clang = tool_version("clang")
    common = ["-std=gnu11", "-Wall", "-Wextra", "-Werror",
              "-DZLOS_TRACE_CAPACITY=4"]
    with tempfile.TemporaryDirectory(prefix="zlos-event-trace-") as temporary:
        temp = Path(temporary)
        host = temp / "eventtracetest"
        host_command = ["gcc", "-O1", "-g", *common,
                        "-fsanitize=address,undefined",
                        "-fno-sanitize-recover=all", "-DZLOS_TRACE_HOSTTEST",
                        "-o", str(host), "hosttest/eventtracetest.c",
                        "hosttest/trace_event.c"]
        host_compile = run(host_command)
        if host_compile.returncode != 0:
            raise ValueError("host event-trace compile failed:\n" + host_compile.stdout)
        execution = run([str(host)])
        match = re.search(r"(\d+) checks, (\d+) failed", execution.stdout)
        if execution.returncode != 0 or not match:
            raise ValueError("host event-trace execution failed:\n" + execution.stdout)

        ilp32 = temp / "trace-event-32.o"
        ilp32_command = ["gcc", "-m32", "-ffreestanding", "-fno-builtin",
                         *common, "-c", "hosttest/trace_event.c", "-o", str(ilp32)]
        ilp32_compile = run(ilp32_command)
        if ilp32_compile.returncode != 0:
            raise ValueError("ILP32 event-trace compile failed:\n" + ilp32_compile.stdout)

        uefi = temp / "trace-event-uefi.obj"
        uefi_command = ["clang", "--target=x86_64-pc-win32-coff",
                        "-ffreestanding", "-fshort-wchar", "-mno-red-zone",
                        *common, "-c", "hosttest/trace_event.c", "-o", str(uefi)]
        uefi_compile = run(uefi_command)
        if uefi_compile.returncode != 0:
            raise ValueError("LLP64 event-trace compile failed:\n" + uefi_compile.stdout)

        output = execution.stdout.encode()
        value = {
            "schema": "zlos.event-trace-host-receipt.v1",
            "result": "PASS_HOST_CORE_TARGET_UNINTEGRATED",
            "build_identity": json.loads(
                (METADATA / "build-identity.json").read_text())["identity_sha256"],
            "source_identities": {name: sha256(KERNEL_ROOT / name) for name in SOURCES},
            "tools": {"gcc": gcc, "clang": clang},
            "proof": {
                "host_execution": {
                    "exit_code": execution.returncode,
                    "checks": int(match.group(1)),
                    "failures": int(match.group(2)),
                    "output_bytes": len(output),
                    "output_sha256": hashlib.sha256(output).hexdigest(),
                },
                "compile_lanes": [
                    {"id": "host-asan-ubsan", "exit_code": host_compile.returncode,
                     "artifact_sha256": sha256(host)},
                    {"id": "bios-raw-ilp32", "exit_code": ilp32_compile.returncode,
                     "artifact_sha256": sha256(ilp32)},
                    {"id": "uefi-llp64", "exit_code": uefi_compile.returncode,
                     "artifact_sha256": sha256(uefi)},
                ],
            },
            "target_integration": {
                "compiled_into_shipped_kernel": False,
                "target_emitters": 0,
                "qemu_routes": 0,
                "native_hardware_routes": 0,
            },
            "open_gaps": [
                "single-owner-only",
                "not-compiled-into-shipped-kernel",
                "no-target-emitter",
                "no-QEMU-receipt",
                "no-native-hardware-receipt",
                "no-durable-anchor",
                "no-service-export-policy",
            ],
            "evidence_ceiling": "real core host execution plus ILP32/LLP64 compile; no shipped target route",
        }
    validate(value)
    return value


def write_atomic(value: dict) -> None:
    RECEIPT.parent.mkdir(parents=True, exist_ok=True)
    temporary = RECEIPT.with_suffix(RECEIPT.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2) + "\n")
    os.replace(temporary, RECEIPT)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if not args.write and not args.check:
        args.check = True
    try:
        if args.write:
            value = build_receipt()
            write_atomic(value)
        else:
            if not RECEIPT.is_file():
                raise ValueError("event-trace receipt is missing")
            value = json.loads(RECEIPT.read_text())
            validate(value)
        if args.selftest:
            selftest(value)
        print(f"event-trace receipt: {value['result']}: "
              f"{value['proof']['host_execution']['checks']} checks, "
              f"{len(value['proof']['compile_lanes'])} compile lanes")
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"event-trace receipt: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
