#!/usr/bin/env python3
"""probe-pointer-usb.py - does the USB mouse move the compositor pointer?

POINTER-PROMPT forbids treating a green xhcitest as done. This boots the ISO
the way ./try.sh does (qemu-xhci + usb-mouse, relative, no tablet), sends a
handful of relative QMP events, and asks whether the framebuffer changed.

A gtk window a human stares at is still the confirmation that prompt named.
This is the closest a headless box can get: a live VM, the real xHCI stack,
the real compositor pointer, not a stub.
"""
import os, subprocess, sys, tempfile, time

HERE = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
from exercise import Serial, Qmp, ppm_sample, frame_delta

def main():
    iso = os.path.join(KERNEL_ROOT, "zlOS.iso")
    r = subprocess.run(
        ["./tools/images/mkiso.sh"], cwd=KERNEL_ROOT, capture_output=True, text=True)
    if r.returncode != 0:
        print("mkiso.sh failed:\n", r.stderr[-2000:] or r.stdout[-2000:])
        return 1
    tmp = tempfile.mkdtemp(prefix="zlos-ptr-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    accel = ["-accel", "kvm", "-cpu", "host"] if os.access("/dev/kvm", os.W_OK) \
        else ["-accel", "tcg", "-cpu", "max"]
    argv = [
        "qemu-system-i386", "-cdrom", iso, "-m", "1G", "-smp", "2",
        *accel,
        "-device", "qemu-xhci,id=xhci",
        "-device", "usb-kbd,bus=xhci.0",
        "-device", "usb-mouse,bus=xhci.0",
        "-device", "virtio-gpu-pci,xres=1280,yres=800",
        "-display", "none", "-no-reboot",
        "-chardev", f"socket,id=ser0,path={ser_path},server=on,wait=off",
        "-serial", "chardev:ser0",
        "-qmp", f"unix:{qmp_path},server=on,wait=off",
    ]
    proc = subprocess.Popen(argv, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        ok, _ = ser.wait("ready.", 180)
        if not ok:
            print("FAIL: guest never said ready.")
            return 1
        ser.drain(2.0)
        a = os.path.join(tmp, "a.ppm")
        b = os.path.join(tmp, "b.ppm")
        if not qmp.screendump(a):
            print("FAIL: first screendump")
            return 1
        before = ppm_sample(a)
        for _ in range(16):
            qmp.cmd("input-send-event", events=[
                {"type": "rel", "data": {"axis": "x", "value": 40}},
                {"type": "rel", "data": {"axis": "y", "value": 25}}])
            time.sleep(0.05)
        ser.drain(1.0)
        if not qmp.screendump(b):
            print("FAIL: second screendump")
            return 1
        after = ppm_sample(b)
        moved = frame_delta(before, after)
        if moved is None:
            print("FAIL: could not compare frames")
            return 1
        print(f"  usb-mouse relative events changed {moved*100:.3f}% of sampled pixels")
        if moved <= 0:
            print("FAIL: pointer did not move the framebuffer")
            return 1
        print("ok    live VM: usb-mouse moved the compositor pointer")
        return 0
    finally:
        proc.kill()
        proc.wait()

if __name__ == "__main__":
    sys.exit(main())
