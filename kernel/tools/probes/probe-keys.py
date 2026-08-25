#!/usr/bin/env python3
"""probe-keys.py - does an injected key reach zlOS, and by which road?

The input-events demo ('=') reads in_next(), which pumps BOTH the PS/2 IRQ
buffer and the USB HID queue itself (input.c:261) and never looks at serial.
So it prints a line for every key that actually arrives - and in the full
sweep it printed "timed out" instead, meaning nothing arrived at all.

That matters more than it looks: the laptop has no serial port, so if a key
cannot reach zlOS the machine is unusable there no matter how well it boots.

This isolates the road. QEMU is given two keyboards (PS/2 by default, plus
usb-kbd); qemu routes an injected key to ONE of them. Run it with each in
turn and see which - if either - zlOS hears.

  ./probe-keys.py            both keyboards, as the sweep runs it
  ./probe-keys.py --no-usb   PS/2 only
"""
import os, subprocess, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

no_usb = "--no-usb" in sys.argv
build(False)
tmp = tempfile.mkdtemp(prefix="zlos-keys-")
ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
argv = qemu_argv(tmp, False, ser_path, qmp_path)
if no_usb:
    i = argv.index("usb-kbd,bus=xhci.0")
    del argv[i - 1:i + 1]                    # drop "-device usb-kbd,..."
print("keyboards:", "PS/2 only" if no_usb else "PS/2 + usb-kbd")

proc = subprocess.Popen(argv, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
try:
    ser, qmp = Serial(ser_path), Qmp(qmp_path)
    ser.wait("ready.", 180)
    ser.wait(PROMPT, 30)

    print("input devices QEMU knows about:")
    r = qmp.cmd("query-input-devices") or {}
    for d in r.get("return", []) or []:
        print("   ", d)
    if "return" not in r:
        print("    (query-input-devices unsupported on this QEMU)")

    ser.send("=")
    ok, banner = ser.wait("ESC ends.", 40)
    print("demo started:", ok)

    for method in ("send-key", "input-send-event"):
        if method == "send-key":
            qmp.cmd("send-key", keys=[{"type": "qcode", "data": "a"}], hold_time=200)
        else:
            qmp.cmd("input-send-event", events=[
                {"type": "key", "data": {"down": True,
                                         "key": {"type": "qcode", "data": "b"}}}])
            qmp.cmd("input-send-event", events=[
                {"type": "key", "data": {"down": False,
                                         "key": {"type": "qcode", "data": "b"}}}])
        ser.drain(2.0)
        heard = "down " in ser.buf or "char " in ser.buf
        print(f"  {method:<18} -> zlOS reported an event: {heard}")

    ser.drain(3.0)
    print("\n--- what the demo printed ---")
    print(ser.buf[:1200] or "(nothing)")
finally:
    proc.terminate()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        proc.kill()
