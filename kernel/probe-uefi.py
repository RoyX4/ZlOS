#!/usr/bin/env python3
"""probe-uefi.py - does zlOS get a framebuffer from GOP, and on which card?

Booting zlOS-usb.img under OVMF, the boot log said:

    [  OK  ] our bootloader (raw_boot), no GRUB - then 64-bit long mode
    [  OK  ] VGA text console, 80x25

Both lines are wrong for that path and they have the same cause. efi.c sets
loader()=2 and a framebuffer console through console_init_efi(); it only falls
back to console_init(0) - VGA text, loader 0 - when fb_addr is zero, i.e. when
LocateProtocol did not hand back a Graphics Output Protocol.

That is fatal on the laptop rather than cosmetic: UEFI has no VGA text mode, so
"fall back to text" means nothing reaches the panel at all, while the serial log
keeps looking perfectly healthy.

This boots the same image against several display devices to find which ones
OVMF publishes a GOP for.
"""
import os, shutil, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from exercise import Serial, build, OVMF_CODE, OVMF_VARS, PROMPT

CARDS = [
    ("virtio-gpu-pci", ["-vga", "none", "-device", "virtio-gpu-pci,xres=1280,yres=800"]),
    ("std VGA",        ["-vga", "std"]),
    ("VMware svga",    ["-vga", "vmware"]),
    ("virtio-vga",     ["-vga", "none", "-device", "virtio-vga"]),
]

subprocess.run(["./mkusb.sh"], cwd=HERE, check=True, capture_output=True)
print("built zlOS-usb.img\n")
print(f"{'display device':<18} {'loader line zlOS printed':<58} console")
print("-" * 100)

for label, disp in CARDS:
    tmp = tempfile.mkdtemp(prefix="zlos-uefi-")
    ser_path = os.path.join(tmp, "ser")
    varsf = os.path.join(tmp, "vars.fd")
    shutil.copy(OVMF_VARS, varsf)
    argv = [
        "qemu-system-x86_64", "-m", "1G", "-smp", "2", "-cpu", "host", "-accel", "kvm",
        "-drive", f"if=pflash,format=raw,unit=0,readonly=on,file={OVMF_CODE}",
        "-drive", f"if=pflash,format=raw,unit=1,file={varsf}",
        "-device", "qemu-xhci,id=xhci",
        "-drive", f"format=raw,file={HERE}/zlOS-usb.img,if=none,id=boot",
        "-device", "usb-storage,bus=xhci.0,drive=boot",
        "-no-reboot", "-display", "none",
        "-chardev", f"socket,id=ser0,path={ser_path},server=on,wait=off",
        "-serial", "chardev:ser0",
    ] + disp
    proc = subprocess.Popen(argv, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    loader_line, console_line = "(never booted)", "-"
    try:
        ser = Serial(ser_path)
        ok, out = ser.wait("ready.", 120)
        for line in out.splitlines():
            s = line.strip()
            if "bootloader" in s or "UEFI application" in s or "multiboot handoff" in s:
                loader_line = s.replace("[  OK  ] ", "")
            if "console," in s:
                console_line = s.split("]", 1)[-1].strip()
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=8)
        except subprocess.TimeoutExpired:
            proc.kill()
        shutil.rmtree(tmp, ignore_errors=True)
    print(f"{label:<18} {loader_line:<58} {console_line}")
