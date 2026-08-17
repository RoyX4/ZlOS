#!/usr/bin/env python3
"""probe-mouse-sync.py - is the PS/2 packet decode actually in sync?

probe-mouse.py says the pointer WORKS, on the grounds that it moved at all.
That is a weaker claim than it looks. It sent 12 events of (+40, +25) from the
start position 400,300 and the guest reported the pointer at 0,0 - both axes
pinned at the minimum, when x should have gone UP by 480. "It moved" and "it
moved where it was told" are different tests, and only the second one is worth
anything to a compositor that has to hit-test a title bar.

So: send a known number of known deltas and check the arithmetic. Three
experiments in ONE boot, because a boot is a minute:

  1. baseline    - enter and leave the demo without touching the mouse
  2. one event   - a single (+10, 0); the byte count says how many bytes a
                   packet really cost, which is how a leading stray byte shows
  3. ten events  - 10 x (+10, 0); the position says whether the decode is
                   accumulating what was sent, or garbage

The demo prints "mouse: N IRQ12 packets, last at X,Y" when it exits, and N is
really an INTERRUPT count - one per byte - so a clean 3-byte packet stream
gives multiples of 3.
"""
import os, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT


def report(ser):
    """leave the demo and read back what the guest saw"""
    ser.send(" ")
    ok, tail = ser.wait("zl> ", 40)
    for line in tail.splitlines():
        if "mouse:" in line:
            body = line.split("mouse:", 1)[1].strip()
            n = int(body.split()[0])
            pos = body.rsplit("at", 1)[1].strip()
            return n, pos
    return None, None


def main():
    # --no-tablet drops the usb-tablet from the machine. try.sh attaches one so
    # the cursor cannot drift, but a tablet is an ABSOLUTE device and zlOS has
    # no driver for it, so it is worth knowing whether QEMU is handing it the
    # relative events too - in which case the PS/2 mouse zlOS does drive never
    # sees them, exactly the way probe-mouse.py says a usb-MOUSE steals them.
    no_tablet = "--no-tablet" in sys.argv
    build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-msync-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    argv = qemu_argv(tmp, False, ser_path, qmp_path, tablet=not no_tablet)
    print("pointer devices:", "PS/2 only" if no_tablet else "PS/2 + usb-tablet")
    proc = subprocess.Popen(argv,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        ok, log = ser.wait("ready.", 240)
        if not ok:
            print("never booted:\n" + log[-1500:]); sys.exit(1)
        ser.wait(PROMPT, 60)

        results = []
        for label, count, dx, dy in (("baseline, no events", 0, 0, 0),
                                     ("one  (+10, 0)",       1, 10, 0),
                                     ("ten  (+10, 0)",      10, 10, 0),
                                     ("ten  (0, +10)",      10, 0, 10)):
            ser.send("x")
            ser.drain(1.5)
            for _ in range(count):
                ev = []
                if dx: ev.append({"type": "rel", "data": {"axis": "x", "value": dx}})
                if dy: ev.append({"type": "rel", "data": {"axis": "y", "value": dy}})
                if ev: qmp.cmd("input-send-event", events=ev)
                ser.drain(0.12)
            ser.drain(1.0)
            n, pos = report(ser)
            results.append((label, count, n, pos))
            ser.drain(0.5)

        print("\n  experiment              events   IRQ12 bytes   pointer")
        prev_n = 0
        for label, count, n, pos in results:
            d = (n - prev_n) if n is not None else 0
            print(f"  {label:<22} {count:>6}   {n:>5} (+{d:<3})   {pos}")
            prev_n = n if n is not None else prev_n

        # A clean 3-byte-per-packet stream makes every delta a multiple of 3,
        # and 10 events of +10 starting from wherever it was must move x by
        # exactly 100.
        print("\n  A packet is 3 bytes, so every '+' above should be 3 x events.")
        print("  And ten (+10, 0) must move x by exactly 100.")
    finally:
        proc.kill()
        proc.wait()


if __name__ == "__main__":
    main()
