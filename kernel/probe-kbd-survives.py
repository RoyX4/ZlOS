#!/usr/bin/env python3
"""Does the USB keyboard survive the input-events demo?

The open bug is that '=' followed by '3i' leaves the editor unable to receive
its ESC. Two very different explanations remain and they need separating:

  A. the keyboard stops delivering entirely after '=' runs
  B. the keyboard is fine, and something specific to the editor's read path
     (edit_key -> usb_key, the only caller that bypasses input_poll and evq)
     cannot see it

So: run '=' TWICE. The demo reports every key it receives. If the second run
reports a keystroke, the keyboard survived and the fault is B. If it reports
nothing, the keyboard is dead and the fault is A.

During each run only the FIRST injected key has ever been seen, so this also
checks whether keys 2 and 3 arrive at all.
"""
import os, subprocess, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

build(False)
tmp = tempfile.mkdtemp(prefix="zlos-kbdsurv-")
ser_p, qmp_p = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
argv = qemu_argv(tmp, False, ser_p, qmp_p)
proc = subprocess.Popen(argv, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def run_input_demo(ser, qmp, label, keys):
    ser.buf = ""
    ser.send("=")
    ser.wait("ESC ends.", 40)
    for k in keys:
        qmp.sendkey(k)
        ser.drain(0.6)
    ok, tail = ser.wait(PROMPT, 90)          # the demo self-ends after 30 s
    seen = [ln.strip() for ln in tail.splitlines()
            if "char " in ln or "down " in ln or "up  " in ln]
    print(f"  {label}: reached prompt={ok}, events reported={len(seen)}")
    for s in seen:
        print(f"      {s}")
    if not seen:
        print("      (nothing - no key reached the guest)")
    return len(seen)


try:
    ser, qmp = Serial(ser_p), Qmp(qmp_p)
    ser.wait("ready.", 180)
    ser.wait(PROMPT, 30)
    print("tablet attached; injecting a, b, esc into each run\n")

    first  = run_input_demo(ser, qmp, "run 1", ("a", "b", "esc"))
    second = run_input_demo(ser, qmp, "run 2", ("c", "d", "esc"))

    print()
    if second > 0:
        print("  VERDICT: the keyboard SURVIVES '=' - so the fault is specific")
        print("           to the editor's read path (edit_key -> usb_key),")
        print("           not the keyboard dying.")
    elif first > 0:
        print("  VERDICT: the keyboard delivered during run 1 and NOTHING during")
        print("           run 2 - it dies as a result of the demo. Fault is in")
        print("           the HID endpoint, not the editor.")
    else:
        print("  VERDICT: nothing arrived even in run 1 - the injection route is")
        print("           wrong, and neither conclusion can be drawn.")
finally:
    proc.terminate()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        proc.kill()
