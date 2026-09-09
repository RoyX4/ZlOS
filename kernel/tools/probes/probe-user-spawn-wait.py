#!/usr/bin/env python3
"""Disk-loaded parent/child spawn, wait, isolation and generation-reuse proof.

--fixtures-only proves assembly and exact multi-file zlfs readback, not boot.
The ordinary mode requires a fresh native-UEFI image and the real desktop route.
"""
import argparse
import hashlib
import importlib.util
import json
import os
import re
from pathlib import Path
import subprocess
import sys
import tempfile
import time

HERE = Path(__file__).resolve().parent
KERNEL = HERE.parent.parent
ROOT = KERNEL.parent
sys.path.insert(0, str(HERE))
from exercise import Qmp, Serial, build, qemu_argv, qtype, validate_process_boot

spec = importlib.util.spec_from_file_location("existing_exit_probe", HERE / "probe-user-process-exit.py")
exit_probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(exit_probe)
PARENT = "/system/user.bin"
CHILD = "/system/child.bin"
SOURCES = [
    "kernel/src/kernel.zl", "freestanding/runtime_kernel.c",
    "kernel/src/arch/x86/usermode.c", "kernel/src/arch/x86/user_image64.c",
    "kernel/src/arch/x86/user_image64.h", "kernel/src/arch/x86/user_process_abi.h",
    "kernel/src/arch/x86/user_syscalls.json", "kernel/src/arch/x86/user_syscalls_generated.h",
    "kernel/src/core/process_lifecycle.c", "kernel/src/core/process_lifecycle.h",
    "kernel/src/core/user_process_service.c", "kernel/src/core/scheduler_policy.c",
    "kernel/src/core/process_memory.c", "kernel/src/core/anon_memory.c",
    "kernel/src/fs/fs.c", "kernel/tests/host/zlfsseed.c",
    "kernel/tests/fixtures/user-spawn-parent.asm", "kernel/tests/fixtures/user-spawn-child.asm",
    "kernel/tools/probes/probe-user-spawn-wait.py",
    "kernel/tools/probes/exercise.py", "kernel/tools/probes/probe-user-process-exit.py",
    "kernel/tools/checks/write-user-process-receipt.py",
    "kernel/tools/checks/write-scheduler-receipt.py",
    "kernel/tools/generators/gen-user-syscalls.py",
]

def digest(path):
    h = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def write_json(path, value):
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2) + "\n")
    temporary.replace(path)

def prepare(directory, normal_exit, orphan_order=None):
    exit_probe.ensure_seed_tool()
    disk = directory / "nvme.img"
    if not disk.exists():
        with disk.open("wb") as stream:
            stream.truncate(64 * 1024 * 1024)
    fixtures = []
    for role, name in (("parent", PARENT), ("child", CHILD)):
        source = KERNEL / "tests/fixtures" / f"user-spawn-{role}.asm"
        binary = directory / f"{role}.bin"
        subprocess.run(["nasm", "-f", "bin", f"-DEXPECT_FAULT={0 if normal_exit else 1}",
                        f"-DORPHAN_ORDER={ {None: 0, 'parent-first': 1, 'child-first': 2}[orphan_order]}",
                        str(source), "-o", str(binary)], check=True)
        size = binary.stat().st_size
        if not 1 <= size <= 4096:
            raise RuntimeError("raw fixture exceeds the admitted executable profile")
        cmd = [exit_probe.SEED_BINARY]
        if role == "child":
            cmd.append("--append")
        subprocess.run([*cmd, str(disk), name, str(binary)], check=True)
        fixtures.append({"name": name, "bytes": size, "sha256": digest(binary),
                         "source": str(source.relative_to(ROOT)), "source_sha256": digest(source)})
    # Fresh read-only opens verify that adding the child preserved the parent.
    for role, name in (("parent", PARENT), ("child", CHILD)):
        subprocess.run([exit_probe.SEED_BINARY, "--verify", str(disk), name,
                        str(directory / f"{role}.bin")], check=True)
    return fixtures

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--normal-exit", action="store_true")
    parser.add_argument("--fixtures-only", action="store_true")
    parser.add_argument("--orphan-order", choices=("parent-first", "child-first"))
    parser.add_argument("--boot-timeout", type=float, default=240)
    parser.add_argument("--step-timeout", type=float, default=90)
    parser.add_argument("--receipt", type=Path, required=True)
    args = parser.parse_args()
    args.receipt.parent.mkdir(parents=True, exist_ok=True)
    # A failed preflight or run must not leave an earlier PASS at this path.
    write_json(args.receipt, {"schema": "zlos.user-spawn-wait-proof.v1",
                             "result": "INCOMPLETE_ATTEMPT"})
    if not args.fixtures_only:
        if os.getloadavg()[0] > 4:
            raise RuntimeError("host load exceeds 4; defer the build/QEMU run")
        if not args.no_build:
            build(True)
        subprocess.run([sys.executable, str(KERNEL / "tools/generators/gen-build-identity.py"),
                        "--check"], check=True)
    transcript = ""
    assertions = []
    with tempfile.TemporaryDirectory(prefix="zlos-spawn-wait-") as temporary:
        directory = Path(temporary)
        serial_path, qmp_path = directory / "serial.sock", directory / "qmp.sock"
        argv = None
        if not args.fixtures_only:
            argv = qemu_argv(str(directory), True, str(serial_path), str(qmp_path), boot_snapshot=True)
            # This ABI is explicitly owned by one CPU. Match that bounded
            # profile instead of spending four vCPUs on unrelated AP loops.
            argv[argv.index("-smp") + 1] = "1"
        fixtures = prepare(directory, args.normal_exit, args.orphan_order)
        disk_sha = digest(directory / "nvme.img")
        if not args.fixtures_only:
            identity = json.loads((KERNEL / "metadata/build-identity.json").read_text())["identity_sha256"]
            stderr_path = args.receipt.with_suffix(".qemu-stderr.txt")
            stderr_stream = stderr_path.open("w")
            process = subprocess.Popen(argv, cwd=KERNEL, stdout=subprocess.DEVNULL, stderr=stderr_stream)
            serial = qmp = None
            def expect(serial, marker, timeout):
                nonlocal transcript
                ok, got = serial.wait(marker, timeout)
                transcript += got
                if not ok:
                    raise RuntimeError("missing runtime marker: " + marker + "\n" + transcript[-3500:])
                assertions.append(marker)
                print("PASS: " + marker, flush=True)
            def account():
                qtype(qmp, "userps\n", settle=settle)
                expect(serial, "persistent Ring-3 processes", args.step_timeout)
                expect(serial, "empty", args.step_timeout)
                begin = len(transcript)
                expect(serial, "physical frames in use: ", args.step_timeout)
                expect(serial, "\n", args.step_timeout)
                rows = re.findall(r"physical frames in use: ([0-9]+); allocator faults: ([0-9]+)\n",
                                  transcript[begin:])
                if len(rows) != 1 or int(rows[0][1]) != 0:
                    raise RuntimeError("missing physical-frame count or failed allocator invariant")
                return int(rows[0][0])
            try:
                serial, qmp = Serial(str(serial_path)), Qmp(str(qmp_path))
                expect(serial, "ready.", args.boot_timeout)
                if "build-identity: schema=1 id=" + identity not in transcript:
                    raise RuntimeError("booted identity does not match the current source-bound image")
                validate_process_boot(transcript)
                settle = 0.12 if os.access("/dev/kvm", os.R_OK | os.W_OK) else 0.8
                qtype(qmp, ".\n", settle=settle)
                expect(serial, "mounted:", args.step_timeout)
                frames_before = account()
                qtype(qmp, "userexec\n", settle=settle)
                expect(serial, "started /system/user.bin as pid 1000", args.step_timeout)
                if args.orphan_order:
                    if args.orphan_order == "parent-first":
                        expect(serial, "ORPHAN-ALIVE", args.step_timeout)
                    # Reissue the read-only status command until both terminal
                    # records are observable; timing is not a guessed delay.
                    deadline = time.monotonic() + args.step_timeout
                    parent = "slot 1: pid 1000 exited with status -19; ready to reap"
                    child = ("slot 2: pid 1001 exited with status -37; ready to reap" if args.normal_exit
                             else "slot 2: pid 1001 faulted on vector 14; ready to reap")
                    while time.monotonic() < deadline:
                        qtype(qmp, "userps\n", settle=settle)
                        ok, got = serial.wait(child, min(2, max(0, deadline-time.monotonic())))
                        transcript += got
                        if ok and parent in transcript:
                            assertions.extend((parent, child))
                            break
                    else:
                        raise RuntimeError("both orphan termination records were not retained")
                else:
                    expect(serial, "SPAWN-WAIT-OK", args.step_timeout)
                    qtype(qmp, "userps\n", settle=settle)
                    expect(serial, "slot 1: pid 1000 exited with status 37; ready to reap", args.step_timeout)
                if args.orphan_order:
                    # Reap the child while the terminal parent is still owned.
                    # This requires immediate service-step adoption; parent
                    # cleanup's later fallback cannot make this pass.
                    qtype(qmp, "userreap 2\n", settle=settle)
                    expect(serial, "released slot 2", args.step_timeout)
                    qtype(qmp, "userps\n", settle=settle)
                    expect(serial, parent, args.step_timeout)
                qtype(qmp, "userreap 1\n", settle=settle)
                expect(serial, "released slot 1", args.step_timeout)
                frames_after = account()
                if frames_after != frames_before:
                    raise RuntimeError(f"physical-frame leak: {frames_before} before, {frames_after} after")
                assertions.append("physical-frame baseline restored and allocator invariant passed")
            finally:
                if process.poll() is None:
                    process.kill()
                process.wait()
                if serial is not None:
                    transcript += serial.buf
                    serial.sock.close()
                if qmp is not None:
                    qmp.f.close()
                stderr_stream.close()
                # Preserve diagnostic output even when a runtime assertion fails.
                args.receipt.with_suffix(".serial.txt").write_text(transcript)
    if "scheduler fail-stop status:" in transcript:
        raise RuntimeError("persistent scheduler failed during the scenario")
    value = {
        "schema": "zlos.user-spawn-wait-proof.v1",
        "result": "PASS_FIXTURES_ONLY" if args.fixtures_only else "PASS_NATIVE_UEFI64_QEMU",
        "scenario": "signed-exit" if args.normal_exit else "parent-private-page-fault",
        "orphan_order": args.orphan_order,
        "fixtures": fixtures, "preboot_disk_sha256": disk_sha,
        "implementation": [{"path": name, "sha256": digest(ROOT / name)} for name in SOURCES],
        "assertions": assertions,
        "known_gaps": ["two fixed slots and 1..4096-byte raw images", "no SMP process locking",
                       "physical hardware untested", "allocation failure injection remains host-tested separately"],
    }
    if not args.fixtures_only:
        value.update(build_identity=identity,
                     virtual_cpus=1,
                     physical_frames={"before": frames_before, "after": frames_after, "allocator_faults": 0},
                     image_sha256=digest(KERNEL / "zlOS-usb.img"),
                     transcript_sha256=hashlib.sha256(transcript.encode("latin-1")).hexdigest())
        args.receipt.with_suffix(".serial.txt").write_text(transcript)
    write_json(args.receipt, value)
    print(value["result"] + ": " + str(args.receipt))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
