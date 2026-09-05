#!/usr/bin/env python3
"""Exercise the persistent Ring-3 service through its real desktop commands.

The ordinary UEFI probes boot with a blank NVMe disk, while `userexec` loads an
external `/system/user.bin` from zlfs. This probe creates that file through the
Files app and disk-backed editor, then uses the terminal to spawn, observe and
reap it. The four bytes are ASCII `aaaa`; 0x61 is invalid in 64-bit mode, so the
expected result is a contained user fault rather than an invented successful
program.
"""

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
sys.path.insert(0, PROBE_DIR)

from exercise import Qmp, Serial, build, qemu_argv, qtype  # noqa: E402

NAME = "/system/user.bin"
BODY = "aaaa"
SHOTS = os.path.join(KERNEL_ROOT, "shots")
REPO_ROOT = os.path.dirname(KERNEL_ROOT)
DEFAULT_RECEIPT = os.path.join(
    KERNEL_ROOT, "docs", "receipts",
    "user-process-command-native-uefi64-qemu-2026-09-03.json",
)
SOURCE_FILES = (
    "freestanding/runtime_kernel.c",
    "kernel/src/kernel.zl",
    "kernel/src/graphics/windowing/term.c",
    "kernel/src/arch/x86/usermode.c",
    "kernel/src/core/process_lifecycle.c",
    "kernel/src/core/scheduler_policy.c",
    "kernel/src/core/user_process_service.c",
    "kernel/tools/probes/probe-user-process.py",
)


class Transcript:
    def __init__(self, serial):
        self.serial = serial
        self.log = ""

    def expect(self, marker, ceiling):
        ok, got = self.serial.wait(marker, ceiling)
        self.log += got
        return ok

    def discard_pending(self, settle=0.3):
        self.serial.drain(settle)
        self.log += self.serial.buf
        self.serial.buf = ""


def click(qmp, x, y, width, height):
    events = [
        {"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / width)}},
        {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / height)}},
    ]
    qmp.cmd("input-send-event", events=events)
    for down in (True, False):
        qmp.cmd("input-send-event", events=[
            {"type": "btn", "data": {"down": down, "button": "left"}}
        ])


def ppm_size(path):
    with open(path, "rb") as stream:
        if stream.readline().strip() != b"P6":
            return None
        line = stream.readline()
        while line.startswith(b"#"):
            line = stream.readline()
        width, height = (int(value) for value in line.split())
        return width, height


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def write_receipt(path, transcript, screenshot):
    identity_path = os.path.join(KERNEL_ROOT, "metadata", "build-identity.json")
    with open(identity_path, encoding="utf-8") as stream:
        identity = json.load(stream)["identity_sha256"]
    assertions = [
        "external /system/user.bin exists as 4 zlfs bytes",
        "userexec admitted pid 1000",
        "invalid 0x61 user instruction became a contained Ring-3 fault",
        "userps retained exact terminal state",
        "userreap released slot 1",
        "userps reported an empty table after reap",
    ]
    value = {
        "schema": "zlos.user-process-command-native-uefi64-qemu-receipt.v1",
        "result": "PASS_EXTERNAL_FILE_SPAWN_FAULT_OBSERVE_REAP",
        "build_identity": identity,
        "route": "native-uefi64",
        "artifact": {
            "path": "kernel/zlOS-usb.img",
            "sha256": sha256(os.path.join(KERNEL_ROOT, "zlOS-usb.img")),
        },
        "probe": {
            "path": "kernel/tools/probes/probe-user-process.py",
            "sha256": sha256(__file__),
        },
        "implementation": [
            {"path": item, "sha256": sha256(os.path.join(REPO_ROOT, item))}
            for item in SOURCE_FILES[:-1]
        ],
        "fixture": {
            "path": NAME,
            "bytes": len(BODY),
            "content_hex": BODY.encode("ascii").hex(),
            "created_through": "Files app and disk-backed editor",
            "expected_execution": "invalid opcode fault in x86-64 mode",
        },
        "assertions": assertions,
        "serial_transcript_sha256": hashlib.sha256(
            transcript.encode("latin-1")
        ).hexdigest(),
        "screenshot": (
            {"path": os.path.relpath(screenshot, REPO_ROOT), "sha256": sha256(screenshot)}
            if screenshot and os.path.exists(screenshot) else None
        ),
        "known_gaps": [
            "the service has exactly two fixed process slots",
            "the command route has no userspace spawn/wait or process-handle syscall",
            "the fixture intentionally proves fault containment rather than normal exit",
            "there is no current physical-hardware command-route receipt",
        ],
        "evidence_ceiling": (
            "one current native-UEFI64 QEMU desktop route proves an external zlfs "
            "image can be spawned, fault-contained, observed and reaped; it does not "
            "prove a general process API, successful application workload or hardware"
        ),
    }
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
    parser.add_argument("--keep-shot", action="store_true")
    parser.add_argument("--receipt", default=DEFAULT_RECEIPT)
    args = parser.parse_args()

    accelerated = os.access("/dev/kvm", os.R_OK | os.W_OK)
    key_settle = 0.12 if accelerated else 0.8
    focus_settle = 0.5 if accelerated else 5.0

    if not args.no_build:
        build(True)

    tmp = tempfile.mkdtemp(prefix="probe-user-process-")
    serial_path = os.path.join(tmp, "serial.sock")
    qmp_path = os.path.join(tmp, "qmp.sock")
    process = subprocess.Popen(
        qemu_argv(tmp, True, serial_path, qmp_path, boot_snapshot=True),
        cwd=KERNEL_ROOT,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    failures = []
    screenshot = None

    def check(label, passed, detail=""):
        state = "ok  " if passed else "FAIL"
        print(f"  {state}  {label}{('   ' + detail) if detail else ''}",
              flush=True)
        if not passed:
            failures.append(label)

    try:
        serial = Serial(serial_path)
        qmp = Qmp(qmp_path)
        transcript = Transcript(serial)

        if not transcript.expect("ready.", args.boot_timeout):
            print("UEFI guest did not reach ready.\n" + transcript.log[-2500:])
            return 1

        match = re.search(
            r"compositor: \d+ windows, shell client (\d+),(\d+) (\d+)x(\d+)",
            transcript.log,
        )
        if not match:
            transcript.expect("compositor:", args.step_timeout)
            transcript.expect("\n", args.step_timeout)
            match = re.search(
                r"compositor: \d+ windows, shell client (\d+),(\d+) (\d+)x(\d+)",
                transcript.log,
            )
        if not match:
            print("guest did not report the terminal rectangle\n" + transcript.log[-2500:])
            return 1
        shell = tuple(int(value) for value in match.groups())
        check("native UEFI desktop reported the terminal rectangle", True)

        qtype(qmp, ".\n", settle=key_settle)
        check("mount command reached the graphical terminal",
              transcript.expect("zlfs on NVMe", args.step_timeout))
        check("blank NVMe was formatted and mounted as zlfs",
              transcript.expect("mounted:", args.step_timeout))

        qtype(qmp, "filemgr\n", settle=key_settle)
        transcript.discard_pending(focus_settle)

        qtype(qmp, "n", settle=key_settle)
        qtype(qmp, NAME + "\n", settle=key_settle)
        editor_open = transcript.expect("wm:lifecycle v=1 event=open", args.step_timeout)
        editor_is_app = transcript.expect(" app=12 ", args.step_timeout) if editor_open else False
        editor_ready = transcript.expect(
            "wm:lifecycle v=1 event=ready", args.step_timeout
        ) if editor_is_app else False
        editor_ready_app = transcript.expect(
            " app=12 ", args.step_timeout
        ) if editor_ready else False
        check("Files created the external image and opened the disk editor",
              editor_open and editor_is_app and editor_ready and editor_ready_app)

        qtype(qmp, BODY, settle=key_settle)
        transcript.discard_pending(focus_settle)
        qmp.sendkey("esc")
        editor_close = transcript.expect("wm:lifecycle v=1 event=close", args.step_timeout)
        editor_close_app = transcript.expect(" app=12 ", args.step_timeout) if editor_close else False
        check("editor saved four external bytes and closed", editor_close and editor_close_app)

        frame = os.path.join(tmp, "focus.ppm")
        if not qmp.screendump(frame) or not ppm_size(frame):
            print("could not capture a frame for terminal focus")
            return 1
        width, height = ppm_size(frame)
        sx, sy, sw, sh = shell
        click(qmp, sx + min(80, max(8, sw // 3)), sy + min(80, max(8, sh // 3)),
              width, height)
        transcript.discard_pending(focus_settle)

        qtype(qmp, "ls\n", settle=key_settle)
        check("external image is present in zlfs",
              transcript.expect(NAME, args.step_timeout))
        check("external image has the expected four bytes",
              transcript.expect("4 bytes", args.step_timeout))

        qtype(qmp, "userexec\n", settle=key_settle)
        check("userexec admitted an external file-backed process",
              transcript.expect("started /system/user.bin as pid 1000", args.step_timeout))

        qtype(qmp, "userps\n", settle=key_settle)
        check("invalid user bytes faulted only their Ring-3 process",
              transcript.expect("slot 1: pid 1000 faulted on vector 6; ready to reap",
                                args.step_timeout))

        qtype(qmp, "userreap 1\n", settle=key_settle)
        check("userreap released the exact terminal slot",
              transcript.expect("released slot 1", args.step_timeout))

        qtype(qmp, "userps\n", settle=key_settle)
        check("process table is empty after reap",
              transcript.expect("persistent Ring-3 processes", args.step_timeout) and
              transcript.expect("empty", args.step_timeout))

        if args.keep_shot:
            os.makedirs(SHOTS, exist_ok=True)
            ppm = os.path.join(tmp, "result.ppm")
            if qmp.screendump(ppm):
                png = os.path.join(SHOTS, "user-process-command-route.png")
                converted = subprocess.run(
                    ["convert", ppm, png], capture_output=True, check=False
                )
                if converted.returncode != 0:
                    shutil.copy(ppm, os.path.join(SHOTS, "user-process-command-route.ppm"))
                else:
                    screenshot = png
                    print(f"  note  screenshot: {png}")
    finally:
        process.kill()
        process.wait()
        shutil.rmtree(tmp)

    print()
    if failures:
        print(f"persistent user-process command gate FAILED: {len(failures)}")
        print("\n--- serial transcript tail ---\n" + transcript.log[-4000:])
        return 1
    write_receipt(os.path.abspath(args.receipt), transcript.log, screenshot)
    print(f"  note  receipt: {os.path.abspath(args.receipt)}")
    print("persistent user-process command gate green: external file -> spawn -> "
          "contained fault -> observe -> reap")
    return 0


if __name__ == "__main__":
    sys.exit(main())
