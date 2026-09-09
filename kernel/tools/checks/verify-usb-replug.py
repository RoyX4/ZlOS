#!/usr/bin/env python3
"""Prove the driver re-attaches a USB keyboard that was unplugged and plugged
back, WITHOUT the Terminal `usb` command.

Until 2026-09-08 xhci_poll() consumed Port Status Change Events and did
nothing, and the only thing that ever re-ran enumeration was `usb_up()` from
the `usb` command. So the witness has to be read-only: `usbstat` prints
kbd_ready / kbd_slot / replug (the count of automatic re-attachments) and
calls nothing that enumerates. The `usb` command would pass this probe by
doing the work itself, which is exactly what it must not be allowed to prove.

QEMU's hot-plug is the real thing: `device_del kbd0` clears CCS on the port
and posts the change event; `device_add` plugs a fresh usb-kbd. Between the
two `usbstat` lines the guest was never typed at.

  python3 tools/checks/verify-usb-replug.py --run [--route native-uefi64|bios32] [--no-build]
"""
from __future__ import annotations
import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
METADATA = KERNEL_ROOT / "metadata"
sys.path.insert(0, str(KERNEL_ROOT / "tools/probes"))
from exercise import Qmp, Serial, build, qemu_argv  # noqa: E402

ROUTES = {
    "native-uefi64": {"uefi": True,  "artifact": "zlOS-usb.img", "kernel": "ZLOS.EFI"},
    "bios32":        {"uefi": False, "artifact": "zlOS.iso",     "kernel": "kernel.elf"},
}
STAT = re.compile(r"usbstat kbd_ready=(\d+) kbd_slot=(\d+) replug=(\d+)")


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def usbstat(serial: Serial) -> tuple[int, int, int, str]:
    serial.send("usbstat\r")
    ok, text = serial.wait("replug=", 30)
    if not ok:
        raise ValueError("usbstat produced no status line: " + text[-800:])
    ok, rest = serial.wait("\n", 5)
    text += rest
    match = STAT.search(text)
    if not match:
        raise ValueError("usbstat line is not the expected shape: " + text[-300:])
    line = text[match.start():].splitlines()[0].strip()   # the WHOLE line, diagnostics included
    return int(match[1]), int(match[2]), int(match[3]), line


def run(route: str, no_build: bool, ceiling: float) -> dict:
    spec = ROUTES[route]
    if not no_build:
        build(spec["uefi"])
    artifact = KERNEL_ROOT / spec["artifact"]
    kernel = KERNEL_ROOT / spec["kernel"]
    for path in (artifact, kernel, METADATA / "build-identity.json"):
        if not path.is_file():
            raise ValueError(f"required current artifact is missing: {path.name}")
    temp = Path(tempfile.mkdtemp(prefix="zlos-replug-"))
    ser_path, qmp_path = temp / "serial.sock", temp / "qmp.sock"
    argv = qemu_argv(str(temp), spec["uefi"], str(ser_path), str(qmp_path),
                     boot_snapshot=spec["uefi"])
    proc = subprocess.Popen(argv, cwd=KERNEL_ROOT, stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    steps = []
    try:
        serial = Serial(str(ser_path))
        qmp = Qmp(str(qmp_path))
        ok, boot = serial.wait("ready.", ceiling)
        if not ok:
            raise ValueError("zlOS never reached ready: " + boot[-1000:])
        serial.drain(1.0)
        ready0, slot0, replug0, line0 = usbstat(serial)
        steps.append({"step": "boot", "line": line0})
        if ready0 != 1 or replug0 != 0:
            raise ValueError("keyboard was not up at boot, or replug count not zero: " + line0)

        # Where QEMU has the keyboard NOW. Without a port on device_add QEMU
        # picks any free slot, and on the UEFI route (five USB devices on four
        # USB2 root ports) that is a port on the hub QEMU auto-inserted - and
        # this driver speaks no hubs, so the plug is invisible to it. Plugging
        # back into the same root port is what a hand does with a cable.
        usb0 = qmp.cmd("human-monitor-command", **{"command-line": "info usb"})
        view0 = str(usb0.get("return", usb0))
        found = re.search(r"Port ([0-9.]+),[^\n]*ID: kbd0", view0)
        if not found or "." in found.group(1):
            raise ValueError("kbd0 is not on a root port before the unplug: " + view0.replace("\n", " | "))
        root_port = found.group(1)
        steps.append({"step": "qemu-boot", "line": f"kbd0 on root port {root_port}"})
        reply = qmp.cmd("device_del", id="kbd0")
        if reply is None or "error" in reply:
            raise ValueError(f"device_del kbd0 refused: {reply}")
        serial.drain(1.5)
        ready1, slot1, replug1, line1 = usbstat(serial)
        steps.append({"step": "unplugged", "line": line1})
        if ready1 != 0 or replug1 != 0:
            raise ValueError("the driver did not notice the unplug: " + line1)

        reply = qmp.cmd("device_add", driver="usb-kbd", id="kbd0", bus="xhci.0", port=root_port)
        if reply is None or "error" in reply:
            raise ValueError(f"device_add usb-kbd refused: {reply}")
        # the driver debounces 120 ms and enumerates from the frame poll; give
        # it a generous margin, then read - never type anything else first
        serial.drain(3.0)
        ready2, slot2, replug2, line2 = usbstat(serial)
        steps.append({"step": "replugged", "line": line2})
        usb = qmp.cmd("human-monitor-command", **{"command-line": "info usb"})
        steps.append({"step": "qemu", "line": str(usb.get("return", usb)).replace("\r", "").replace("\n", " | ")})
        if ready2 != 1:
            raise ValueError("the keyboard did not come back on its own: " + line2)
        if replug2 != 1:
            raise ValueError("re-attachment was not counted as automatic: " + line2)
        # The slot id may legitimately be the same number: the driver disabled
        # the old slot and Enable Slot handed the id back. What must not
        # happen is reuse WITHOUT that disable, which xhcitest holds; the
        # controller's numbering is not evidence either way.
    except ValueError as error:
        # every line read so far is evidence; a failure must not hide them
        raise ValueError(str(error) + "".join(f"\n    {st['step']:<10} {st['line']}" for st in steps)) from None
    finally:
        proc.kill()
        proc.wait()
        shutil.rmtree(temp)
    identity = json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"]
    return {
        "schema": "zlos.usb-replug-qemu-receipt.v1",
        "result": "PASS",
        "route": route,
        "build_identity": identity,
        "artifact": {"path": "kernel/" + spec["artifact"], "sha256": sha256(artifact)},
        "kernel": {"path": "kernel/" + spec["kernel"], "sha256": sha256(kernel)},
        "steps": steps,
        "evidence": ("QEMU device_del/device_add of the usb-kbd with no guest input in "
                     "between; the read-only usbstat word shows kbd_ready 1 -> 0 -> 1, "
                     "a new slot, and replug=1 counted by the driver's own poll path"),
        "weakest_link": ("QEMU's xHCI model, and the same root port on purpose: the driver "
                         "speaks no hubs, so a keyboard that comes back behind one (which is "
                         "where QEMU puts a fifth device on four USB2 root ports) is not seen; "
                         "a slow-to-settle physical keyboard is not exercised either"),
    }


def receipt_path(route: str) -> Path:
    return KERNEL_ROOT / "docs/receipts" / f"usb-replug-{route}-qemu-2026-09-08.json"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", action="store_true")
    parser.add_argument("--route", choices=tuple(ROUTES), default="native-uefi64")
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--ceiling", type=float, default=120.0)
    args = parser.parse_args()
    try:
        if args.run:
            value = run(args.route, args.no_build, args.ceiling)
            path = receipt_path(args.route)
            path.write_text(json.dumps(value, indent=2) + "\n")
            print(f"usb-replug: PASS ({args.route}) -> {path.relative_to(KERNEL_ROOT)}")
            for step in value["steps"]:
                print(f"  {step['step']:<10} {step['line']}")
            return 0
        path = receipt_path(args.route)
        value = json.loads(path.read_text())
        identity = json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"]
        if value.get("result") != "PASS" or value.get("build_identity") != identity:
            raise ValueError("usb-replug receipt is missing, failed, or from another build")
        print(f"usb-replug: receipt current ({args.route})")
        return 0
    except ValueError as error:
        print(f"usb-replug: FAIL: {error}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
