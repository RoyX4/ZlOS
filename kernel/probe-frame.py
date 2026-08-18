#!/usr/bin/env python3
"""probe-frame.py - is the frame time on screen a real measurement?

Item 9's gate is deliberately not "a number appears". It is that the number
AGREES with what fbbench says the same work costs - because a frame timer that
is plausible and wrong is worse than no frame timer, and this one has two ways
to be exactly that:

  the clock      cpu_tsc used to return cpu_tsc_lo(), a u32 that wraps every
                 1.8 s at 2.4 GHz. A timer on it reads correctly most of the
                 time and goes wildly negative a few times a minute.
  the conversion cycles -> microseconds needs the TSC rate, and a wrong rate
                 gives a number that looks completely reasonable.

So this asserts, on the serial log rather than off a photograph:

  IDLE      with nothing animating, a frame is cheap - almost all of it is
            input_poll over an empty queue and a repaint with no damage
  BUSY      with the cube and the starfield running, every frame repaints two
            windows, so the figure must RISE - and stay inside the envelope
            fbbench measured for that much work
  THE RATE  cpu_khz() has to be a real CPU speed, because everything above is
            derived from it

The busy/idle comparison is the part that makes it a measurement. A constant
is plausible; a number that tracks the work is measured.
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from exercise import Serial, Qmp, qemu_argv, build  # noqa: E402

SHOTS = os.path.join(HERE, "shots")
QCODE = {" ": "spc", "\n": "ret"}
for _c in "abcdefghijklmnopqrstuvwxyz":
    QCODE[_c] = _c


def qtype(qmp, text, settle=0.07):
    for ch in text:
        qmp.sendkey(QCODE[ch])
        time.sleep(settle)


def read_frame(ser, qmp, log_ref):
    """Type `frame` and return (microseconds, tsc_khz) off the serial log."""
    qtype(qmp, "frame\n")
    ok, got = ser.wait("frame: ", 30)
    if not ok:
        return None, None
    ser.drain(0.5)
    got += ser.buf; ser.buf = ""
    log_ref.append(got)
    m = re.search(r"frame:\s*(\d+)\s*us avg over \d+ painted frames\s+tsc\s*(\d+)\s*kHz", got)
    if not m:
        return None, None
    return int(m.group(1)), int(m.group(2))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--keep-shots", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()

    if not args.no_build:
        build(False)

    tmp = tempfile.mkdtemp(prefix="probeframe-")
    ser_path = os.path.join(tmp, "ser.sock")
    qmp_path = os.path.join(tmp, "qmp.sock")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path, tablet=False),
                            cwd=HERE, stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    failures = []

    def check(label, ok, detail=""):
        print(f"  {'ok  ' if ok else 'FAIL'}  {label}{('   ' + detail) if detail else ''}")
        if not ok:
            failures.append(label)

    try:
        ser = Serial(ser_path)
        qmp = Qmp(qmp_path)
        ok, log = ser.wait("compositor:", args.boot_timeout)
        if not ok:
            print("never reached the compositor:\n" + log[-2000:]); return 1
        ser.drain(1.5)
        ser.buf = ""

        sink = []
        idle_us, khz = read_frame(ser, qmp, sink)
        if idle_us is None:
            print("the frame command printed nothing parseable"); return 1
        print(f"  note  idle {idle_us} us,  tsc {khz} kHz")

        # A TSC rate that is not a CPU speed makes every figure above fiction.
        check("the TSC rate is a real CPU speed",
              khz is not None and 300000 <= khz <= 6000000, f"{khz} kHz")

        # Now give it real work: the cube and the starfield both damage their
        # windows every frame, so every frame repaints.
        qtype(qmp, "cube\n")
        qtype(qmp, "anim\n")
        time.sleep(2.0)
        ser.drain(1.0); ser.buf = ""
        busy_us, _ = read_frame(ser, qmp, sink)
        if busy_us is None:
            print("no busy reading"); return 1
        print(f"  note  busy {busy_us} us with the cube and starfield running")

        # THE ASSERTION THAT MAKES IT A MEASUREMENT. A hardcoded constant would
        # pass "is it plausible"; only a real timer moves when the work does.
        check("the frame time RISES under load", busy_us > idle_us,
              f"{idle_us} -> {busy_us} us")

        # ...and stays inside the envelope. fbbench puts a whole-desktop redraw
        # at 3-8 ms and ONE WINDOW at ~1.2 ms on this class of machine. Two
        # animating windows per frame has to land between "did nothing" and
        # "redrew everything", generously bounded because the host is shared.
        check("...into fbbench's envelope for that work",
              200 <= busy_us <= 40000,
              f"{busy_us} us, against ~1200 us for one window and "
              f"~6000 us for the whole desktop")

        if args.keep_shots:
            os.makedirs(SHOTS, exist_ok=True)
            ppm = os.path.join(tmp, "frame.ppm")
            if qmp.screendump(ppm):
                subprocess.run(["convert", ppm, os.path.join(SHOTS, "frame-monitor.png")],
                               capture_output=True)

        print()
        if failures:
            print(f"frame gate FAILED: {len(failures)}")
            print("\n--- serial ---\n" + "".join(sink)[-1200:])
            return 1
        print("frame gate green: it is a measurement, not a number")
        return 0
    finally:
        proc.kill()
        proc.wait()
        subprocess.run(["rm", "-rf", tmp])


if __name__ == "__main__":
    sys.exit(main())
