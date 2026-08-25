#!/usr/bin/env python3
"""Prove the exact current zlOS image captures a real CPU exception in QEMU."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
METADATA = KERNEL_ROOT / "metadata"
sys.path.insert(0, str(KERNEL_ROOT / "tools/probes"))
from exercise import Serial, build, qemu_argv  # noqa: E402

RECEIPT = KERNEL_ROOT / "docs/receipts/cpu-fault-invalid-opcode-qemu-2026-08-23.json"
SOURCES = ("src/core/crash.c", "src/core/crash.h", "src/arch/x86/idt.c",
           "src/graphics/windowing/term.c", "src/kernel.zl",
           "../freestanding/runtime_kernel.c", "tools/checks/verify-crash.py")
LINE = re.compile(
    r"^ZLCRASH v=(?P<version>\d+) bytes=(?P<bytes>\d+) "
    r"bits=(?P<bits>\d+) vec=(?P<vector>\d+) haserr=(?P<has_error>\d+) "
    r"err=(?P<error>0x[0-9a-f]{16}) ip=(?P<ip>0x[0-9a-f]{16}) "
    r"cs=(?P<cs>0x[0-9a-f]{16}) flags=(?P<flags>0x[0-9a-f]{16}) "
    r"sp=(?P<sp>0x[0-9a-f]{16}) ss=(?P<ss>0x[0-9a-f]{16}) "
    r"cr2=(?P<cr2>0x[0-9a-f]{16}) checksum=(?P<checksum>0x[0-9a-f]{16}) END\n$"
)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def symbol_address() -> int:
    output = subprocess.check_output(["nm", "-n", "kernel.elf"], cwd=KERNEL_ROOT,
                                     text=True)
    match = re.search(r"^([0-9a-fA-F]+)\s+[Tt]\s+crash_test_ud2$", output, re.M)
    if not match:
        raise ValueError("kernel.elf does not expose crash_test_ud2")
    return int(match.group(1), 16)


def mix64(value: int, item: int) -> int:
    for shift in range(0, 64, 8):
        value ^= (item >> shift) & 0xFF
        value = (value * 1099511628211) & ((1 << 64) - 1)
    return value


def checksum(record: dict) -> int:
    value = 1469598103934665603
    for key in ("version", "bytes", "vector", "has_error", "bits", "error",
                "ip", "cs", "flags", "sp", "ss", "cr2"):
        value = mix64(value, record[key])
    return value


def parse_line(line: str) -> dict:
    match = LINE.fullmatch(line)
    if not match:
        raise ValueError("crash line is not the exact bounded schema")
    decimal = {"version", "bytes", "bits", "vector", "has_error"}
    return {key: int(value, 10 if key in decimal else 16)
            for key, value in match.groupdict().items()}


def current_sources() -> dict:
    return {"kernel/" + value if not value.startswith("../") else value[3:]:
            sha256((KERNEL_ROOT / value).resolve()) for value in SOURCES}


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.cpu-fault-qemu-receipt.v1":
        raise ValueError("wrong crash receipt schema")
    if value.get("result") != "PASS":
        raise ValueError("crash receipt does not pass")
    if value.get("route") != "grub-bios32-qemu":
        raise ValueError("crash receipt route drift")
    if value.get("trigger") != "UD2 through typed crashtest diagnostic":
        raise ValueError("crash trigger identity drift")
    identity = json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"]
    if value.get("build_identity") != identity:
        raise ValueError("crash receipt build identity is stale")
    if value.get("artifact", {}).get("path") != "kernel/zlOS.iso" or \
            value["artifact"].get("sha256") != sha256(KERNEL_ROOT / "zlOS.iso"):
        raise ValueError("crash receipt ISO identity is stale")
    if value.get("kernel", {}).get("path") != "kernel/kernel.elf" or \
            value["kernel"].get("sha256") != sha256(KERNEL_ROOT / "kernel.elf"):
        raise ValueError("crash receipt kernel identity is stale")
    if value.get("source_sha256") != current_sources():
        raise ValueError("crash receipt source identity is stale")
    record = value.get("record", {})
    expected = {"version": 1, "bytes": 88, "bits": 32, "vector": 6,
                "has_error": 0, "error": 0, "cr2": 0}
    for key, wanted in expected.items():
        if record.get(key) != wanted:
            raise ValueError(f"crash record {key} is not {wanted}")
    if not all(isinstance(record.get(key), int) for key in
               ("ip", "cs", "flags", "sp", "ss", "checksum")):
        raise ValueError("crash record fields are incomplete")
    if record["ip"] != symbol_address() or \
            value.get("kernel_symbol_ip") != record["ip"]:
        raise ValueError("fault IP is not the UD2 instruction symbol")
    if record["checksum"] != checksum(record):
        raise ValueError("crash checksum does not independently verify")
    line = value.get("crash_line")
    if not isinstance(line, str) or parse_line(line) != record:
        raise ValueError("crash line and parsed record disagree")
    if value.get("crash_line_sha256") != hashlib.sha256(line.encode()).hexdigest():
        raise ValueError("crash line identity drift")
    if value.get("guest_halted_after_record") is not True:
        raise ValueError("guest did not remain halted after recording")


def selftest(value: dict) -> None:
    mutations = []
    for name, mutate in (
        ("wrong-vector", lambda x: x["record"].__setitem__("vector", 13)),
        ("wrong-checksum", lambda x: x["record"].__setitem__("checksum", 0)),
        ("wrong-symbol", lambda x: x.__setitem__("kernel_symbol_ip", 0)),
        ("stale-artifact", lambda x: x["artifact"].__setitem__("sha256", "0" * 64)),
        ("not-halted", lambda x: x.__setitem__("guest_halted_after_record", False)),
    ):
        mutant = copy.deepcopy(value)
        mutate(mutant)
        try:
            validate(mutant)
        except ValueError:
            mutations.append(name)
        else:
            raise ValueError(f"crash receipt selftest mutation escaped: {name}")
    print("crash receipt selftest: caught " + ", ".join(mutations))


def run(no_build: bool, ceiling: float) -> dict:
    if not no_build:
        build(False)
    for path in (KERNEL_ROOT / "kernel.elf", KERNEL_ROOT / "zlOS.iso",
                 METADATA / "build-identity.json"):
        if not path.is_file():
            raise ValueError(f"required current artifact is missing: {path.name}")

    temp = Path(tempfile.mkdtemp(prefix="zlos-crash-"))
    ser_path = temp / "serial.sock"
    qmp_path = temp / "qmp.sock"
    argv = qemu_argv(str(temp), False, str(ser_path), str(qmp_path))
    proc = subprocess.Popen(argv, cwd=KERNEL_ROOT, stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    try:
        serial = Serial(str(ser_path))
        ok, boot = serial.wait("ready.", ceiling)
        if not ok:
            raise ValueError("zlOS never reached ready before crash injection: " + boot[-1000:])
        serial.send("crashtest\r")
        ok, echo = serial.wait("zl> crashtest", 60)
        if not ok:
            raise ValueError("the guest did not accept the crashtest command: " + echo[-1000:])
        ok, prefix = serial.wait("ZLCRASH ", 60)
        if not ok:
            raise ValueError("the real invalid opcode produced no crash record: " + prefix[-1000:])
        ok, rest = serial.wait(" END\n", 30)
        if not ok:
            raise ValueError("the crash record has no completion marker: " + rest[-1000:])
        line = "ZLCRASH " + rest
        record = parse_line(line)
        # A halted guest stays alive in QEMU but produces no further prompt.
        ok_prompt, unexpected = serial.wait("zl> ", 1.0)
        guest_halted = proc.poll() is None and not ok_prompt
        if not guest_halted:
            raise ValueError("guest returned or exited after the fault: " + unexpected[-500:])
    finally:
        proc.kill()
        proc.wait()
        shutil.rmtree(temp)

    identity = json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"]
    value = {
        "schema": "zlos.cpu-fault-qemu-receipt.v1",
        "result": "PASS",
        "route": "grub-bios32-qemu",
        "trigger": "UD2 through typed crashtest diagnostic",
        "build_identity": identity,
        "artifact": {"path": "kernel/zlOS.iso", "sha256": sha256(KERNEL_ROOT / "zlOS.iso")},
        "kernel": {"path": "kernel/kernel.elf", "sha256": sha256(KERNEL_ROOT / "kernel.elf")},
        "kernel_symbol_ip": symbol_address(),
        "source_sha256": current_sources(),
        "record": record,
        "crash_line": line,
        "crash_line_sha256": hashlib.sha256(line.encode()).hexdigest(),
        "guest_halted_after_record": guest_halted,
        "evidence": "QEMU target execution of the exact current ISO/kernel; vector, UD2 IP, bounded schema and checksum independently verified; no durability or recovery claim",
        "weakest_link": "control frame only: general registers, stack symbols, persistent dump and recovery remain unimplemented",
    }
    validate(value)
    return value


def write_atomic(value: dict) -> None:
    RECEIPT.parent.mkdir(parents=True, exist_ok=True)
    temporary = RECEIPT.with_suffix(RECEIPT.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
    os.replace(temporary, RECEIPT)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--ceiling", type=float, default=300)
    args = parser.parse_args()
    if not args.run and not args.check:
        args.check = True
    try:
        if args.run:
            value = run(args.no_build, args.ceiling)
            write_atomic(value)
        else:
            value = json.loads(RECEIPT.read_text())
            validate(value)
        if args.selftest:
            selftest(value)
        print("crash-qemu: PASS: vector 6, exact UD2 IP, checksum, halt")
        return 0
    except (OSError, ValueError, json.JSONDecodeError,
            subprocess.CalledProcessError) as error:
        print(f"crash-qemu: FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
