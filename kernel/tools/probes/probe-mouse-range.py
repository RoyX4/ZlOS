#!/usr/bin/env python3
"""probe-mouse-range.py - how far can the pointer actually get?

Reported symptom: the cursor moves, but will not go below about half the
screen. That has two completely different causes and guessing between them is
how you fix the wrong one:

  1. zlOS clamps the position too low - a kernel bug.
  2. QEMU is translating host pointer motion into RELATIVE PS/2 deltas, and
     when the host pointer reaches the edge of the QEMU window there is no
     more motion to translate. The guest cursor stops wherever it got to.
     Nothing is wrong with the kernel; the window just ran out.

Injecting events over QMP bypasses the window completely, so if the pointer
CAN reach the bottom this way, cause 2 is the answer and the fix is a mouse
grab, not a code change.

Sweeps to each extreme and reports where the guest says it ended up.
"""
import os, subprocess, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

build(False)
tmp = tempfile.mkdtemp(prefix="zlos-range-")
ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def sweep(qmp, ser, dx, dy, steps=60):
    for _ in range(steps):
        ev = []
        if dx: ev.append({"type": "rel", "data": {"axis": "x", "value": dx}})
        if dy: ev.append({"type": "rel", "data": {"axis": "y", "value": dy}})
        qmp.cmd("input-send-event", events=ev)
        ser.drain(0.03)
    ser.drain(0.8)


def read_pos(ser, qmp):
    """Enter the mouse demo, leave it, and read the position it reports."""
    ser.send("x")
    ser.drain(1.2)
    ser.send(" ")
    ok, tail = ser.wait(PROMPT, 30)
    for line in tail.splitlines():
        if "mouse:" in line:
            return line.split("at", 1)[1].strip()
    return "?"


try:
    ser, qmp = Serial(ser_path), Qmp(qmp_path)
    ser.wait("ready.", 180)
    ser.wait(PROMPT, 30)
    print("screen is 1920x1200; idt.c starts the pointer at 400,300")
    print("idt.c clamps to 0..2000 x 0..1500; the demo clamps y to 48..1182\n")

    for label, dx, dy in [("hard DOWN", 0, 60), ("hard RIGHT", 60, 0),
                          ("hard UP", 0, -60), ("hard LEFT", -60, 0),
                          ("DOWN again", 0, 60)]:
        sweep(qmp, ser, dx, dy)
        print(f"  after {label:<11} -> {read_pos(ser, qmp)}")

    print("\n  bottom of a 1200-tall screen is y=1182 after the demo's clamp.")
    print("  If y reaches ~1182 here, the kernel is fine and the limit you hit")
    print("  is QEMU's window running out of relative motion - grab with")
    print("  Ctrl+Alt+G, or the pointer can never cross the whole screen.")
finally:
    proc.terminate()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        proc.kill()
