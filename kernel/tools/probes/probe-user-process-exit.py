#!/usr/bin/env python3
"""Prove a valid external zlfs program reaches a retained normal exit."""

import argparse
import hashlib
import json
import os
import subprocess
import sys
import tempfile

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
REPO_ROOT = os.path.dirname(KERNEL_ROOT)
HOST_TESTS = os.path.join(KERNEL_ROOT, "tests", "host")
SEED_SOURCE = os.path.join(HOST_TESTS, "zlfsseed.c")
SEED_BINARY = os.path.join(HOST_TESTS, "zlfsseed")
FS_SOURCE = os.path.join(KERNEL_ROOT, "src", "fs", "fs.c")
NAME = "/system/user.bin"
EXIT_STATUS = 37
OUTPUT_MARKER = "R3!"
PROGRAM = bytes.fromhex(
    "bb52000000b801000000cd80"
    "bb33000000b801000000cd80"
    "bb21000000b801000000cd80"
    "bb25000000b803000000cd80"
    "0f0b"
)
# Read guest ticks, sleep five ticks, then require an unsigned elapsed value
# of at least five before running the original output/exit program. No polling
# loop exists in the fixture: an early successful wake executes UD2.
SLEEP_TICKS = 5
SLEEP_PROGRAM = bytes.fromhex(
    "b805000000cd8089c6"          # time -> ESI
    "bb0500000031c931d2b819000000cd80"  # sleep(5, 0, 0)
    "4885c074020f0b"              # unsuccessful sleep -> UD2
    "b805000000cd8029f083f80573020f0b"  # time - ESI >= 5, else UD2
) + PROGRAM
DEFAULT_RECEIPT = os.path.join(
    KERNEL_ROOT, "docs", "receipts",
    "user-process-exit-native-uefi64-qemu-2026-09-03.json",
)
SOURCE_FILES = (
    "freestanding/runtime_kernel.c",
    "kernel/src/kernel.zl",
    "kernel/src/arch/x86/usermode.c",
    "kernel/src/core/process_lifecycle.c",
    "kernel/src/core/scheduler_policy.c",
    "kernel/src/core/user_process_service.c",
    "kernel/src/fs/fs.c",
    "kernel/tests/host/zlfsseed.c",
    "kernel/tools/probes/probe-user-process-exit.py",
)

sys.path.insert(0, PROBE_DIR)
from exercise import Qmp, Serial, build, qemu_argv, qtype  # noqa: E402


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def ensure_seed_tool():
    newest_source = max(os.path.getmtime(SEED_SOURCE), os.path.getmtime(FS_SOURCE))
    if os.path.exists(SEED_BINARY) and os.path.getmtime(SEED_BINARY) >= newest_source:
        return
    include_args = []
    for root, directories, _files in os.walk(os.path.join(KERNEL_ROOT, "src")):
        directories.sort()
        include_args.extend(("-I", root))
    command = [
        "gcc", *include_args, "-O2", "-g", "-Wall", "-Wextra", "-Werror",
        "-DFS_HOSTTEST", "-o", SEED_BINARY, SEED_SOURCE, FS_SOURCE,
    ]
    subprocess.run(command, cwd=HOST_TESTS, check=True)


def write_receipt(path, transcript, fixture_path, sleep=False):
    program = SLEEP_PROGRAM if sleep else PROGRAM
    with open(os.path.join(KERNEL_ROOT, "metadata", "build-identity.json"),
              encoding="utf-8") as stream:
        identity = json.load(stream)["identity_sha256"]
    assertions = [
        "external /system/user.bin exists as exact raw x86-64 bytes",
        "userexec admitted pid 1000",
        "the external program emitted R3! through three Ring-3 syscalls",
        "userps retained normal exit status 37",
        "userreap released slot 1",
        "userps reported an empty table after reap",
    ]
    if sleep:
        assertions.insert(2, "guest sleep returned successfully after at least five PIT ticks without userspace polling")
    value = {
        "schema": ("zlos.user-process-sleep-native-uefi64-qemu-receipt.v1" if sleep
                   else "zlos.user-process-exit-native-uefi64-qemu-receipt.v1"),
        "result": ("PASS_EXTERNAL_FILE_SLEEP_EXIT_OBSERVE_REAP" if sleep
                   else "PASS_EXTERNAL_FILE_SPAWN_EXIT_OBSERVE_REAP"),
        "build_identity": identity,
        "route": "native-uefi64",
        "artifact": {
            "path": "kernel/zlOS-usb.img",
            "sha256": sha256(os.path.join(KERNEL_ROOT, "zlOS-usb.img")),
        },
        "probe": {
            "path": "kernel/tools/probes/probe-user-process-exit.py",
            "sha256": sha256(__file__),
        },
        "implementation": [
            {"path": item, "sha256": sha256(os.path.join(REPO_ROOT, item))}
            for item in SOURCE_FILES[:-1]
        ],
        "fixture": {
            "path": NAME,
            "bytes": len(program),
            "sha256": sha256(fixture_path),
            "content_hex": program.hex(),
            "created_through": "host instrument linked to the shipping zlfs implementation",
            "expected_output": OUTPUT_MARKER,
            "expected_exit_status": EXIT_STATUS,
        },
        "assertions": assertions,
        "serial_transcript_sha256": hashlib.sha256(
            transcript.encode("latin-1")
        ).hexdigest(),
        "known_gaps": [
            "the service has exactly two fixed process slots",
            "the command route has no userspace spawn/wait or process-handle syscall",
            "the raw image contract has no relocations or shared libraries",
            "there is no current physical-hardware command-route receipt",
        ],
        "evidence_ceiling": (
            "one current native-UEFI64 QEMU route proves that an external zlfs "
            "image can run useful syscalls, exit normally with exact status, be "
            "observed and be reaped; it does not prove a general process API, "
            "application format, or physical hardware"
        ),
    }
    if sleep:
        value["fixture"]["minimum_guest_sleep_ticks"] = SLEEP_TICKS
        value["known_gaps"].append("no measured maximum wake latency, suspend behavior or physical timer receipt")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    temporary = path + ".tmp"
    with open(temporary, "w", encoding="utf-8") as stream:
        json.dump(value, stream, indent=2)
        stream.write("\n")
    os.replace(temporary, path)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--boot-timeout", type=float, default=240)
    parser.add_argument("--step-timeout", type=float, default=60)
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--sleep", action="store_true",
                        help="prove a real PIT-timed sleep before output and exit")
    parser.add_argument("--receipt", default=DEFAULT_RECEIPT)
    args = parser.parse_args()
    program = SLEEP_PROGRAM if args.sleep else PROGRAM
    if args.sleep and args.receipt == DEFAULT_RECEIPT:
        args.receipt = DEFAULT_RECEIPT.replace("user-process-exit-", "user-process-sleep-")

    if not args.no_build:
        build(True)
    ensure_seed_tool()
    key_settle = 0.12 if os.access("/dev/kvm", os.R_OK | os.W_OK) else 0.8

    temporary = tempfile.TemporaryDirectory(prefix="probe-user-process-exit-")
    tmp = temporary.name
    serial_path = os.path.join(tmp, "serial.sock")
    qmp_path = os.path.join(tmp, "qmp.sock")
    fixture_path = os.path.join(tmp, "user.bin")
    with open(fixture_path, "wb") as stream:
        stream.write(program)

    argv = qemu_argv(tmp, True, serial_path, qmp_path, boot_snapshot=True)
    subprocess.run(
        [SEED_BINARY, os.path.join(tmp, "nvme.img"), NAME, fixture_path],
        cwd=KERNEL_ROOT, check=True,
    )
    process = subprocess.Popen(
        argv, cwd=KERNEL_ROOT, stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    failures = []
    transcript = ""

    def expect(serial, label, marker):
        nonlocal transcript
        passed, got = serial.wait(marker, args.step_timeout)
        transcript += got
        print(f"  {'ok  ' if passed else 'FAIL'}  {label}", flush=True)
        if not passed:
            failures.append(label)
        return passed

    try:
        serial = Serial(serial_path)
        qmp = Qmp(qmp_path)
        passed, got = serial.wait("ready.", args.boot_timeout)
        transcript += got
        print(f"  {'ok  ' if passed else 'FAIL'}  native UEFI desktop reached ready",
              flush=True)
        if not passed:
            print(transcript[-2500:])
            return 1
        if "persistent user-process service FAILED" in transcript or \
                "persistent sleep deadline FAILED" in transcript:
            print("  FAIL  boot-time persistent process service self-check", flush=True)
            print(transcript[-2500:])
            return 1
        print("  ok    boot-time persistent process service self-check", flush=True)

        qtype(qmp, ".\n", settle=key_settle)
        expect(serial, "preseeded zlfs volume mounted", "mounted:")

        qtype(qmp, "ls\n", settle=key_settle)
        expect(serial, "external executable is present", NAME)
        expect(serial, "external executable has the exact byte count",
               f"{len(program)} bytes")

        qtype(qmp, "userexec\n", settle=key_settle)
        expect(serial, "userexec admitted the external image",
               "started /system/user.bin as pid 1000")
        expect(serial, "external Ring-3 syscalls emitted their marker", OUTPUT_MARKER)

        qtype(qmp, "userps\n", settle=key_settle)
        expect(serial, "normal exit status remained observable",
               "slot 1: pid 1000 exited with status 37; ready to reap")

        qtype(qmp, "userreap 1\n", settle=key_settle)
        expect(serial, "userreap released the terminal process", "released slot 1")

        qtype(qmp, "userps\n", settle=key_settle)
        expect(serial, "process table is empty after reap", "persistent Ring-3 processes")
        expect(serial, "empty table marker followed the heading", "empty")
    finally:
        process.kill()
        process.wait()

    if failures:
        print(f"\nnormal-exit user-process gate FAILED: {len(failures)}")
        print("\n--- serial transcript tail ---\n" + transcript[-4000:])
        return 1
    write_receipt(os.path.abspath(args.receipt), transcript, fixture_path, args.sleep)
    if args.sleep:
        print("  ok    external process slept at least five real guest PIT ticks without polling")
    print(f"  note  receipt: {os.path.abspath(args.receipt)}")
    print("normal-exit user-process gate green: external file -> syscalls -> "
          "exit status -> observe -> reap")
    return 0


if __name__ == "__main__":
    sys.exit(main())
