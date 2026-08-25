#!/usr/bin/env python3
"""Prove post-ExitBootServices diagnostics survive when ZLLOG is refused."""

from __future__ import annotations

import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path


KERNEL = Path(__file__).resolve().parents[2]
REPO = KERNEL.parent
sys.path.insert(0, str(REPO / "tools"))
import zlbootdiag  # noqa: E402


def main() -> int:
    qemu = shutil.which("qemu-system-x86_64")
    sgdisk = shutil.which("sgdisk")
    code = Path("/usr/share/OVMF/OVMF_CODE_4M.fd")
    vars_template = Path("/usr/share/OVMF/OVMF_VARS_4M.fd")
    image = KERNEL / "zlOS-usb.img"
    if not qemu or not sgdisk or not code.exists() or not vars_template.exists():
        print("  skip  EFI runtime diagnostic gate needs qemu, sgdisk and OVMF")
        return 0
    if not image.exists():
        raise AssertionError("zlOS-usb.img is missing; run mkusb.sh first")

    with tempfile.TemporaryDirectory(prefix="zlos-runtime-diag-") as directory:
        root = Path(directory)
        bad_image = root / "bad-label.img"
        vars_file = root / "vars.fd"
        serial = root / "serial.log"
        qemu_stderr = root / "qemu.stderr"
        shutil.copyfile(image, bad_image)
        shutil.copyfile(vars_template, vars_file)
        subprocess.run(
            [sgdisk, "--change-name=2:NOT-ZLLOG", str(bad_image)],
            check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
        )

        accel = ["-cpu", "host", "-accel", "kvm"] if Path("/dev/kvm").exists() else [
            "-cpu", "max", "-accel", "tcg"
        ]
        command = [
            qemu, "-m", "1G", "-smp", "2", *accel,
            "-drive", f"if=pflash,format=raw,unit=0,readonly=on,file={code}",
            "-drive", f"if=pflash,format=raw,unit=1,file={vars_file}",
            "-device", "qemu-xhci,id=xhci",
            "-drive", f"format=raw,file={bad_image},if=none,id=boot",
            "-device", "usb-storage,bus=xhci.0,drive=boot",
            "-device", "usb-kbd,bus=xhci.0",
            "-device", "usb-mouse,bus=xhci.0",
            "-vga", "std", "-display", "none", "-no-reboot",
            "-serial", f"file:{serial}",
        ]
        with qemu_stderr.open("wb") as error_log:
            process = subprocess.Popen(
                command, stdout=subprocess.DEVNULL, stderr=error_log
            )
            try:
                deadline = time.monotonic() + 120
                while time.monotonic() < deadline:
                    if serial.exists() and b"ready." in serial.read_bytes():
                        break
                    if process.poll() is not None:
                        break
                    time.sleep(0.1)
                else:
                    raise AssertionError("runtime diagnostic QEMU boot timed out")
            finally:
                if process.poll() is None:
                    process.terminate()
                process.wait(timeout=10)

        if not serial.exists():
            detail = qemu_stderr.read_text(errors="replace").strip()
            raise AssertionError(
                f"QEMU exited {process.returncode} before opening serial: {detail}"
            )

        transcript = serial.read_text(errors="replace")
        assert "ready." in transcript, "bad-label image never reached the prompt"
        store = vars_file.read_bytes()
        offset = store.rfind(zlbootdiag.MAGIC)
        assert offset >= 0, "OVMF variable store has no ZlBootDiag payload"
        size = zlbootdiag.payload_size(store, offset)
        report = zlbootdiag.parse(
            store[offset : offset + size], efivarfs=False
        )
        assert report["version"] == 6, report
        assert report["state_name"] == "storage-refused", report
        assert report["reason_name"] == "gpt", report
        assert report["msc_stage_name"] == "ready", report
        assert report["msc_result"] == 0, report
        assert any(port["msc_candidate"] for port in report["ports"]), report
        assert len(report["ports"]) >= 3, report
        assert any(
            port.get("ep0_trace", {}).get("valid") for port in report["ports"]
        ), report
        probe = report["first_device_probe"]
        assert probe["valid"], report
        assert probe["first"]["setup_bytes"] == probe["second"]["setup_bytes"], report
        assert probe["first"]["value"] == 0x0100, report
        assert probe["first"]["event_completion_code"] == 1, report
        assert probe["second"]["event_completion_code"] == 1, report
        assert probe["first_device_address"] == probe["second_device_address"], report
        assert probe["first_slot_state"] == probe["second_slot_state"], report
        config = report["first_config_probe"]
        assert config["valid"], report
        assert config["trace"]["value"] == 0x0200, report
        assert config["trace"]["event_completion_code"] == 1, report

    print("  ok    post-exit firmware fallback retained a forced ZLLOG refusal")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, subprocess.SubprocessError) as exc:
        print(f"  FAIL  EFI runtime diagnostic gate: {exc}", file=sys.stderr)
        raise SystemExit(1)
