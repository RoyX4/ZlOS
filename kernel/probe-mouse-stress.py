#!/usr/bin/env python3
"""probe-mouse-stress.py - make the keyboard and the mouse collide on purpose.

The symptom was "sometimes it works, then I redo it and it works": intermittent,
which no clamp or config bug explains. The 8042 is ONE controller with ONE
output buffer at port 0x60 shared by keyboard and mouse, and status bit 5 at
0x64 is the only thing saying which device a byte came from. IRQ1 read 0x60
without checking, so a key arriving mid-packet swallowed one of the mouse's
three bytes and desynced the stream.

Two things this test has to get right, both of which the first version got
wrong and which silently made it measure nothing:

  1. Do NOT type inside the mouse demo. It loops `while key_get() == 0`, so the
     first keystroke exits it - the run then reports one packet's worth of
     movement and looks like a catastrophic desync that never happened.
     Type at the SHELL PROMPT instead, using digits: is_digit() just
     accumulates them into `pending` and executes nothing.
  2. Do NOT leave usb-kbd attached. QEMU routes injected keys to it, so they
     arrive over USB HID and never touch port 0x60 - no collision is possible
     and the test passes for the wrong reason.

idt.c keeps mouse_x/mouse_y across commands, so the position can be racked up
at the prompt and read back afterwards by entering and leaving the demo.

  ./probe-mouse-stress.py           PS/2 keys racing PS/2 mouse packets
  ./probe-mouse-stress.py --quiet   mouse only, as a control
"""
import os, subprocess, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

quiet = "--quiet" in sys.argv
STEPS, DX, DY = 20, 30, 20

build(False)
tmp = tempfile.mkdtemp(prefix="zlos-stress-")
ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
argv = qemu_argv(tmp, False, ser_path, qmp_path)
i = argv.index("usb-kbd,bus=xhci.0")          # drop "-device usb-kbd,..."
del argv[i - 1:i + 1]                          # so keys land on the PS/2 port
print("keys go to the PS/2 keyboard, sharing port 0x60 with the mouse")
print("mouse only (control)" if quiet else "typing WHILE moving - the collision")

proc = subprocess.Popen(argv, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
try:
    ser, qmp = Serial(ser_path), Qmp(qmp_path)
    ser.wait("ready.", 180)
    ser.wait(PROMPT, 30)

    # At the prompt: digits accumulate into `pending` and run nothing, so the
    # shell keeps looping and the pointer keeps tracking while keys stream in.
    for _ in range(STEPS):
        qmp.cmd("input-send-event", events=[
            {"type": "rel", "data": {"axis": "x", "value": DX}},
            {"type": "rel", "data": {"axis": "y", "value": DY}}])
        if not quiet:
            for down in (True, False):         # a real PS/2 scancode, racing it
                qmp.cmd("input-send-event", events=[
                    {"type": "key",
                     "data": {"down": down, "key": {"type": "qcode", "data": "1"}}}])
        ser.drain(0.12)
    ser.drain(1.0)

    ser.send("\x1b")            # clear any accumulated digits
    ser.drain(0.4)
    ser.send("x")               # enter the demo purely to read the position out
    ser.drain(1.5)
    ser.send(" ")
    ok, tail = ser.wait(PROMPT, 30)
    pos, irqs = None, None
    for line in tail.splitlines():
        if "mouse:" in line:
            body = line.split("mouse:", 1)[1]
            print("  guest says:" + body)
            irqs = int(body.strip().split()[0])
            pos = body.rsplit("at", 1)[1].strip()

    want = f"{min(400 + STEPS * DX, 1919)},{min(300 + STEPS * DY, 1199)}"
    print(f"  arithmetic says {want}")
    if pos == want:
        print("\n  VERDICT: exact - every packet survived the keyboard traffic")
    else:
        print(f"\n  VERDICT: LOST BYTES - landed at {pos}, expected {want}"
              f" ({irqs} packets seen)")
finally:
    proc.terminate()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        proc.kill()
