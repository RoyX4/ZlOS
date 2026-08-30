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
from exercise import Qmp, Serial, build, qemu_argv  # noqa: E402

CASES = {
    ("bios32", "ud2"): {
        "route": "grub-bios32-qemu",
        "artifact": "zlOS.iso",
        "kernel": "kernel.elf",
        "receipt": "cpu-fault-invalid-opcode-qemu-2026-08-23.json",
        "uefi": False,
        "command": "crashtest",
        "trigger": "UD2 through typed crashtest diagnostic",
        "vector": 6, "has_error": 0, "error": 0,
        "symbol_register": None,
    },
    ("native-uefi64", "ud2"): {
        "route": "native-uefi64-qemu",
        "artifact": "zlOS-usb.img",
        "kernel": "ZLOS.EFI",
        "receipt": "cpu-fault-native-uefi64-qemu-2026-08-29.json",
        "uefi": True,
        "command": "crashtest",
        "trigger": "UD2 through typed crashtest diagnostic",
        "vector": 6, "has_error": 0, "error": 0,
        "symbol_register": "r15",
    },
    ("native-uefi64", "gp"): {
        "route": "native-uefi64-qemu",
        "artifact": "zlOS-usb.img",
        "kernel": "ZLOS.EFI",
        "receipt": "cpu-fault-general-protection-native-uefi64-qemu-2026-08-29.json",
        "uefi": True,
        "command": "crashgptest",
        "trigger": "invalid GDT selector 0x38 through typed crashgptest diagnostic",
        "vector": 13, "has_error": 1, "error": 0x38,
        "symbol_register": "r15",
    },
    ("native-uefi64", "double-fault"): {
        "route": "native-uefi64-qemu",
        "artifact": "zlOS-usb.img",
        "kernel": "ZLOS.EFI",
        "receipt": "cpu-fault-double-fault-native-uefi64-qemu-2026-08-29.json",
        "uefi": True,
        "command": "crashdftest",
        "trigger": "page fault on unusable RSP through typed crashdftest diagnostic",
        "vector": 8, "has_error": 1, "error": 0,
        "symbol_register": None,
    },
}
SOURCES = ("src/core/crash.c", "src/core/crash.h", "src/arch/x86/idt.c",
           "boot/gdt64.c",
           "src/graphics/windowing/term.c", "src/kernel.zl",
           "../freestanding/runtime_kernel.c", "tools/checks/verify-crash.py")
LINE = re.compile(
    r"^ZLCRASH v=(?P<version>\d+) bytes=(?P<bytes>\d+) "
    r"bits=(?P<bits>\d+) regmask=(?P<register_mask>0x[0-9a-f]{16}) "
    r"vec=(?P<vector>\d+) haserr=(?P<has_error>\d+) "
    r"err=(?P<error>0x[0-9a-f]{16}) ip=(?P<ip>0x[0-9a-f]{16}) "
    r"cs=(?P<cs>0x[0-9a-f]{16}) flags=(?P<flags>0x[0-9a-f]{16}) "
    r"sp=(?P<sp>0x[0-9a-f]{16}) ss=(?P<ss>0x[0-9a-f]{16}) "
    r"cr2=(?P<cr2>0x[0-9a-f]{16}) "
    r"handlersp=(?P<handler_sp>0x[0-9a-f]{16}) "
    r"stacklo=(?P<emergency_stack_low>0x[0-9a-f]{16}) "
    r"stackhi=(?P<emergency_stack_high>0x[0-9a-f]{16}) "
    r"ax=(?P<ax>0x[0-9a-f]{16}) bx=(?P<bx>0x[0-9a-f]{16}) "
    r"cx=(?P<cx>0x[0-9a-f]{16}) dx=(?P<dx>0x[0-9a-f]{16}) "
    r"si=(?P<si>0x[0-9a-f]{16}) di=(?P<di>0x[0-9a-f]{16}) "
    r"bp=(?P<bp>0x[0-9a-f]{16}) rsp=(?P<register_sp>0x[0-9a-f]{16}) "
    r"r8=(?P<r8>0x[0-9a-f]{16}) r9=(?P<r9>0x[0-9a-f]{16}) "
    r"r10=(?P<r10>0x[0-9a-f]{16}) r11=(?P<r11>0x[0-9a-f]{16}) "
    r"r12=(?P<r12>0x[0-9a-f]{16}) r13=(?P<r13>0x[0-9a-f]{16}) "
    r"r14=(?P<r14>0x[0-9a-f]{16}) r15=(?P<r15>0x[0-9a-f]{16}) "
    r"checksum=(?P<checksum>0x[0-9a-f]{16}) END\n$"
)
REGISTER_KEYS = ("ax", "bx", "cx", "dx", "si", "di", "bp", "register_sp",
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def symbol_address() -> int:
    output = subprocess.check_output(["nm", "-n", "kernel.elf"], cwd=KERNEL_ROOT,
                                     text=True)
    match = re.search(r"^([0-9a-fA-F]+)\s+[Tt]\s+crash_test_ud2_fault$", output, re.M)
    if not match:
        raise ValueError("kernel.elf does not expose crash_test_ud2_fault")
    return int(match.group(1), 16)


def mix64(value: int, item: int) -> int:
    for shift in range(0, 64, 8):
        value ^= (item >> shift) & 0xFF
        value = (value * 1099511628211) & ((1 << 64) - 1)
    return value


def checksum(record: dict) -> int:
    value = 1469598103934665603
    for key in ("version", "bytes", "vector", "has_error", "bits",
                "register_mask", "error", "ip", "cs", "flags", "sp", "ss",
                "cr2", "handler_sp", "emergency_stack_low",
                "emergency_stack_high", *REGISTER_KEYS):
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


def expected_registers(bits: int, fault_case: str) -> dict:
    if bits == 32:
        return {
            "ax": 0xA1B2C3D4, "bx": 0xB1C2D3E4,
            "cx": 0xC1D2E3F4, "dx": 0xD1E2F304,
            "si": 0x51627384, "di": 0x61728394,
            "bp": 0x718293A4,
            "r8": 0, "r9": 0, "r10": 0, "r11": 0,
            "r12": 0, "r13": 0, "r14": 0, "r15": 0,
        }
    return {
        "ax": (0x38 if fault_case == "gp" else 0x0102030405060708),
        "bx": 0x1112131415161718,
        "cx": 0x2122232425262728, "dx": 0x3132333435363738,
        "si": 0x4142434445464748, "di": 0x5152535455565758,
        "bp": 0x6162636465666768,
        "r8": 0x8182838485868788, "r9": 0x9192939495969798,
        "r10": 0xA1A2A3A4A5A6A7A8, "r11": 0xB1B2B3B4B5B6B7B8,
        "r12": 0xC1C2C3C4C5C6C7C8, "r13": 0xD1D2D3D4D5D6D7D8,
        "r14": 0xE1E2E3E4E5E6E7E8,
    }


def case_spec(value: dict) -> dict:
    matches = [spec for (route_name, fault_case), spec in CASES.items()
               if spec["route"] == value.get("route")
               and fault_case == value.get("fault_case")]
    if len(matches) != 1:
        raise ValueError("crash receipt route drift")
    return matches[0]


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.cpu-fault-qemu-receipt.v3":
        raise ValueError("wrong crash receipt schema")
    if value.get("result") != "PASS":
        raise ValueError("crash receipt does not pass")
    spec = case_spec(value)
    fault_case = value["fault_case"]
    if value.get("trigger") != spec["trigger"]:
        raise ValueError("crash trigger identity drift")
    identity = json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"]
    if value.get("build_identity") != identity:
        raise ValueError("crash receipt build identity is stale")
    artifact_path = KERNEL_ROOT / spec["artifact"]
    kernel_path = KERNEL_ROOT / spec["kernel"]
    if value.get("artifact", {}).get("path") != "kernel/" + spec["artifact"] or \
            value["artifact"].get("sha256") != sha256(artifact_path):
        raise ValueError("crash receipt artifact identity is stale")
    if value.get("kernel", {}).get("path") != "kernel/" + spec["kernel"] or \
            value["kernel"].get("sha256") != sha256(kernel_path):
        raise ValueError("crash receipt kernel identity is stale")
    if value.get("source_sha256") != current_sources():
        raise ValueError("crash receipt source identity is stale")
    record = value.get("record", {})
    bits = 64 if spec["uefi"] else 32
    expected = {"version": 3, "bytes": 240, "bits": bits,
                "register_mask": 0xFFFF if bits == 64 else 0xFF,
                "vector": spec["vector"], "has_error": spec["has_error"],
                "error": spec["error"]}
    if fault_case != "double-fault":
        expected.update({"cr2": 0, "emergency_stack_low": 0,
                         "emergency_stack_high": 0,
                         **expected_registers(bits, fault_case)})
    for key, wanted in expected.items():
        if record.get(key) != wanted:
            raise ValueError(
                f"crash record {key} is {record.get(key)!r}, expected {wanted!r}"
            )
    if not all(isinstance(record.get(key), int) for key in
               ("ip", "cs", "flags", "sp", "ss", "register_sp",
                "handler_sp", "emergency_stack_low", "emergency_stack_high",
                "checksum")):
        raise ValueError("crash record fields are incomplete")
    if record["register_sp"] != record["sp"]:
        raise ValueError("saved general-register SP does not match the control frame")
    if record["handler_sp"] == 0:
        raise ValueError("handler stack identity is absent")
    if fault_case == "double-fault":
        low, high = record["emergency_stack_low"], record["emergency_stack_high"]
        qmp_rsp = value.get("halted_cpu", {}).get("rsp")
        if not (low < high and low <= record["handler_sp"] < high):
            raise ValueError("double-fault handler did not enter the declared IST stack")
        if not isinstance(qmp_rsp, int) or not low <= qmp_rsp < high:
            raise ValueError("QEMU halted CPU RSP is outside the declared IST stack")
        if len(value.get("halted_cpu", {}).get("register_dump_sha256", "")) != 64:
            raise ValueError("QEMU halted register identity is missing")
        binding = {"mechanism": "architecturally undefined for #DF",
                   "register": None, "address": None}
    elif bits == 32:
        binding = {"mechanism": "ELF symbol table", "register": None,
                   "address": symbol_address()}
    else:
        register = spec["symbol_register"]
        binding = {"mechanism": "fault label address loaded before trigger",
                   "register": register, "address": record.get(register)}
    if value.get("runtime_symbol_binding") != binding:
        raise ValueError("runtime fault-symbol binding drift")
    if fault_case == "double-fault":
        if value.get("kernel_symbol_ip") is not None:
            raise ValueError("double-fault receipt invented a defined saved IP")
    elif record["ip"] != binding["address"] or \
            value.get("kernel_symbol_ip") != record["ip"]:
        raise ValueError("fault IP is not the triggering instruction symbol")
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
    cases = [
        ("wrong-vector", lambda x: x["record"].__setitem__(
            "vector", (x["record"]["vector"] + 1) % 32)),
        ("wrong-checksum", lambda x: x["record"].__setitem__("checksum", 0)),
        ("missing-registers", lambda x: x["record"].__setitem__("register_mask", 0)),
        ("stale-artifact", lambda x: x["artifact"].__setitem__("sha256", "0" * 64)),
        ("not-halted", lambda x: x.__setitem__("guest_halted_after_record", False)),
    ]
    if value.get("fault_case") == "double-fault":
        cases.extend([
            ("handler-outside-ist", lambda x: x["record"].__setitem__(
                "handler_sp", x["record"]["emergency_stack_low"] - 1)),
            ("qmp-rsp-outside-ist", lambda x: x["halted_cpu"].__setitem__(
                "rsp", x["record"]["emergency_stack_high"])),
            ("invented-defined-ip", lambda x: x.__setitem__("kernel_symbol_ip", 1)),
        ])
    else:
        cases.extend([
            ("wrong-symbol", lambda x: x.__setitem__("kernel_symbol_ip", 0)),
            ("wrong-register", lambda x: x["record"].__setitem__("bx", 0)),
            ("invented-upper-register", lambda x: x["record"].__setitem__("r15", 1)),
        ])
    for name, mutate in cases:
        mutant = copy.deepcopy(value)
        mutate(mutant)
        try:
            validate(mutant)
        except ValueError:
            mutations.append(name)
        else:
            raise ValueError(f"crash receipt selftest mutation escaped: {name}")
    print("crash receipt selftest: caught " + ", ".join(mutations))


def halted_registers(qmp: Qmp) -> tuple[str, int]:
    reply = qmp.cmd("human-monitor-command", **{"command-line": "info registers"})
    output = reply.get("return", "") if isinstance(reply, dict) else ""
    match = re.search(r"\b(?:RSP|ESP)=([0-9a-fA-F]{8,16})\b", output)
    if not match:
        raise ValueError("QEMU did not expose the halted CPU stack pointer: " +
                         output[:500].replace("\n", " "))
    return output, int(match.group(1), 16)


def run(no_build: bool, ceiling: float, route_name: str, fault_case: str) -> dict:
    try:
        spec = CASES[(route_name, fault_case)]
    except KeyError as error:
        raise ValueError(f"unsupported crash case: {route_name}/{fault_case}") from error
    if not no_build:
        build(spec["uefi"])
    for path in (KERNEL_ROOT / spec["kernel"], KERNEL_ROOT / spec["artifact"],
                 METADATA / "build-identity.json"):
        if not path.is_file():
            raise ValueError(f"required current artifact is missing: {path.name}")
    artifact_path = KERNEL_ROOT / spec["artifact"]
    artifact_before = sha256(artifact_path)

    temp = Path(tempfile.mkdtemp(prefix="zlos-crash-"))
    ser_path = temp / "serial.sock"
    qmp_path = temp / "qmp.sock"
    argv = qemu_argv(str(temp), spec["uefi"], str(ser_path), str(qmp_path),
                     boot_snapshot=spec["uefi"])
    proc = subprocess.Popen(argv, cwd=KERNEL_ROOT, stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    try:
        serial = Serial(str(ser_path))
        qmp = Qmp(str(qmp_path))
        ok, boot = serial.wait("ready.", ceiling)
        if not ok:
            raise ValueError("zlOS never reached ready before crash injection: " + boot[-1000:])
        serial.send(spec["command"] + "\r")
        ok, echo = serial.wait("zl> " + spec["command"], 60)
        if not ok:
            raise ValueError("the guest did not accept the crash command: " + echo[-1000:])
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
        register_dump, halted_rsp = halted_registers(qmp)
    finally:
        proc.kill()
        proc.wait()
        shutil.rmtree(temp)
    if sha256(artifact_path) != artifact_before:
        raise ValueError("QEMU crash observation changed its boot artifact")

    identity = json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"]
    if fault_case == "double-fault":
        binding = {"mechanism": "architecturally undefined for #DF",
                   "register": None, "address": None}
        kernel_symbol_ip = None
    elif not spec["uefi"]:
        binding = {"mechanism": "ELF symbol table", "register": None,
                   "address": symbol_address()}
        kernel_symbol_ip = record["ip"]
    else:
        register = spec["symbol_register"]
        binding = {"mechanism": "fault label address loaded before trigger",
                   "register": register, "address": record[register]}
        kernel_symbol_ip = record["ip"]
    value = {
        "schema": "zlos.cpu-fault-qemu-receipt.v3",
        "result": "PASS",
        "route": spec["route"],
        "fault_case": fault_case,
        "trigger": spec["trigger"],
        "build_identity": identity,
        "artifact": {"path": "kernel/" + spec["artifact"], "sha256": artifact_before},
        "kernel": {"path": "kernel/" + spec["kernel"],
                   "sha256": sha256(KERNEL_ROOT / spec["kernel"])},
        "kernel_symbol_ip": kernel_symbol_ip,
        "runtime_symbol_binding": binding,
        "halted_cpu": {
            "rsp": halted_rsp,
            "register_dump_sha256": hashlib.sha256(register_dump.encode()).hexdigest(),
        },
        "source_sha256": current_sources(),
        "record": record,
        "crash_line": line,
        "crash_line_sha256": hashlib.sha256(line.encode()).hexdigest(),
        "guest_halted_after_record": guest_halted,
        "evidence": ("QEMU target execution of the exact current artifact; architectural vector/error shape, pre-prologue registers, bounded schema, checksum and halted state independently verified; the double-fault case additionally binds serial handler SP and QEMU halted RSP to IST1"),
        "weakest_link": "the exercised matrix remains bounded; stack unwinding, durable dump storage, recovery policy and physical fault injection remain unimplemented",
    }
    validate(value)
    return value


def receipt_path(route_name: str, fault_case: str) -> Path:
    try:
        receipt = CASES[(route_name, fault_case)]["receipt"]
    except KeyError as error:
        raise ValueError(f"unsupported crash case: {route_name}/{fault_case}") from error
    return KERNEL_ROOT / "docs/receipts" / receipt


def write_atomic(value: dict, output: Path) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_suffix(output.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
    os.replace(temporary, output)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--ceiling", type=float, default=300)
    parser.add_argument("--route", choices=("bios32", "native-uefi64"), default="bios32")
    parser.add_argument("--fault", choices=("ud2", "gp", "double-fault"), default="ud2")
    args = parser.parse_args()
    if not args.run and not args.check:
        args.check = True
    try:
        if args.run:
            value = run(args.no_build, args.ceiling, args.route, args.fault)
            write_atomic(value, receipt_path(args.route, args.fault))
        else:
            value = json.loads(receipt_path(args.route, args.fault).read_text())
            validate(value)
        if args.selftest:
            selftest(value)
        print(f"crash-qemu: PASS: {value['route']}/{value['fault_case']}: "
              f"vector {value['record']['vector']}, error/frame/{value['record']['bits']}-bit "
              "registers, checksum, halt")
        return 0
    except (OSError, ValueError, json.JSONDecodeError,
            subprocess.CalledProcessError) as error:
        print(f"crash-qemu: FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
