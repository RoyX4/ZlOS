#!/usr/bin/env python3
"""probe-mouse.py - does the pointer actually move?

zlOS reads the pointer from idt_mouse_x/y/btn only - the PS/2 mouse on IRQ12.
xhci.c contains no mouse code at all, so a usb-mouse attached to the guest is
not a second option, it is a thief: QEMU routes every pointer event to it and
the PS/2 mouse zlOS IS driving never sees one. The cursor then sits still and
the whole pointer UI looks dead.

This drives the mouse demo ('x'), which erases the cursor at its old spot and
redraws it at the new one each frame. With no movement two frames are pixel
identical, so a frame delta is exactly the question "did the cursor move".

  ./probe-mouse.py             PS/2 mouse, nothing stealing from it
  ./probe-mouse.py --usb       re-attach usb-mouse, to show the failure
  ./probe-mouse.py --tablet    re-attach usb-tablet, which steals it too (T-5)

The pointer must land WHERE IT WAS TOLD, not merely somewhere else. An earlier
version of this script asserted only "did it move", and passed while the
pointer was being driven to 0,0 - both axes pinned at the clamp - by a
usb-tablet swallowing every event. "It moved" is not a test a compositor can
build hit-testing on.
"""
import os, subprocess, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, ppm_sample, frame_delta, qemu_argv, build, PROMPT

use_usb = "--usb" in sys.argv
# try.sh attaches a usb-tablet and that is a THIEF, measured in
# probe-mouse-sync.py: with it present QEMU routes the RELATIVE events to it
# too, and the PS/2 mouse zlOS drives never sees one. So the default here is a
# machine with nothing competing for the pointer; --tablet reproduces try.sh.
use_tablet = "--tablet" in sys.argv
build(False)
tmp = tempfile.mkdtemp(prefix="zlos-mouse-")
ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
argv = qemu_argv(tmp, False, ser_path, qmp_path, tablet=use_tablet)
if use_usb:
    argv += ["-device", "usb-mouse,bus=xhci.0"]
print("pointer device:", "usb-mouse (zlOS cannot read it)" if use_usb
      else "PS/2 mouse (zlOS drives this)"
           + (" + usb-tablet, which steals from it" if use_tablet else ""))

proc = subprocess.Popen(argv, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
try:
    ser, qmp = Serial(ser_path), Qmp(qmp_path)
    ser.wait("ready.", 180)
    ser.wait(PROMPT, 30)

    ser.send("x")                      # the mouse demo
    ser.drain(2.5)
    a = os.path.join(tmp, "a.ppm"); qmp.screendump(a); before = ppm_sample(a)

    # shove the pointer a long way, in steps, the way a hand would
    for _ in range(12):
        qmp.cmd("input-send-event", events=[
            {"type": "rel", "data": {"axis": "x", "value": 40}},
            {"type": "rel", "data": {"axis": "y", "value": 25}}])
        ser.drain(0.15)
    ser.drain(1.5)
    b = os.path.join(tmp, "b.ppm"); qmp.screendump(b); after = ppm_sample(b)

    moved = frame_delta(before, after)
    print(f"  frame changed after moving the mouse: {moved*100:.2f}%")

    # and the buttons, which colour the cursor
    qmp.cmd("input-send-event", events=[
        {"type": "btn", "data": {"down": True, "button": "left"}}])
    ser.drain(1.0)
    c = os.path.join(tmp, "c.ppm"); qmp.screendump(c); clicked = ppm_sample(c)
    qmp.cmd("input-send-event", events=[
        {"type": "btn", "data": {"down": False, "button": "left"}}])
    click_delta = frame_delta(after, clicked)
    print(f"  frame changed on left button down:    {click_delta*100:.2f}%")

    # The screen delta is NOT the test. The cursor is 12x18 px on a 1920x1200
    # screen, so moving it repaints 0.02% of the pixels - below any sane
    # threshold. Ask the guest what it actually received instead.
    ser.send(" ")                       # exit the demo; it reports IRQ12 count
    ok, tail = ser.wait("zl> ", 30)
    irqs, pos = None, None
    for line in tail.splitlines():
        if "mouse:" in line:
            print("  guest says:" + line.split("mouse:", 1)[1])
            body = line.split("mouse:", 1)[1]
            irqs = int(body.strip().split()[0])
            pos = body.rsplit("at", 1)[1].strip()
    # idt.c starts the pointer here, and 12 events of (+40, +25) land it
    # exactly there. QEMU's y-down convention and the ISR's inversion cancel,
    # measured in probe-mouse-sync.py: ten (0, +10) moves y from 300 to 400.
    START = "400,300"
    WANT = f"{400 + 12 * 40},{300 + 12 * 25}"
    if irqs is None:
        print("\n  VERDICT: no report from the guest")
    elif irqs == 0:
        print("\n  VERDICT: DEAD - IRQ12 never fired. Nothing is reaching the"
              " PS/2 mouse;\n           something else owns the pointer.")
    elif pos == START:
        print("\n  VERDICT: DEAD - interrupts arrive but the position never"
              " moved: decode bug.")
    elif pos != WANT:
        print(f"\n  VERDICT: WRONG - it moved, but to {pos}, not {WANT}."
              "\n           Moving somewhere is not the test. Either something"
              " is eating\n           events (see T-5) or the decode is off.")
    else:
        print(f"\n  VERDICT: WORKS - {irqs} packets, pointer went"
              f" {START} -> {pos}, exactly where it was sent")
finally:
    proc.terminate()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        proc.kill()
