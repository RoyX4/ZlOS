#!/usr/bin/env python3
"""probe-frame.py - what does a frame actually COST, on a real boot?

desktop-TODO 0h: "add a tsc() builtin and put frame time on screen. DO THIS
BEFORE ANY PERFORMANCE WORK. Optimising without measurement is guessing." The
v10 run did the performance work first and this never. So every claim about the
compositor's speed up to now was arithmetic from fbbench's per-primitive
numbers, which is not the same thing as a frame.

wm_frame() times itself with the TSC now, in MICROseconds - a cheap frame is
well under 1 ms and integer milliseconds would report every one of them as
zero - and only frames that actually repaint are counted, or a desktop at rest
would average out as infinitely fast.

This drives real interaction and reads the peak back off the tray, by OCR of
nothing: it types `peak` into the shell, which prints the two numbers.
"""
import os, subprocess, sys, tempfile, re

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

HERE = os.path.dirname(os.path.abspath(__file__))


def at(qmp, x, y, w, h, btn=None):
    ev = [{"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / w)}},
          {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / h)}}]
    if btn is not None:
        ev.append({"type": "btn", "data": {"down": btn, "button": "left"}})
    qmp.cmd("input-send-event", events=ev)


def peak(ser):
    """Read the frame timer. The shell prints ONE line with a marker at each
    end, and this waits for the END marker - waiting for a substring that
    appears twice in the answer returns halfway through it."""
    ser.send("peak\r")
    ok, out = ser.wait("END", 30)
    m = re.search(r"FRAMEUS\s+(\d+)\s+(\d+)\s+END", out)
    return (int(m.group(1)), int(m.group(2))) if m else (None, None)


def main():
    build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-frame-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        if not ser.wait("ready.", 240)[0]:
            print("never booted"); return 1
        ser.wait(PROMPT, 60)
        ser.drain(2.0)
        W, H = 1920, 1200
        ui = 2

        print("BOOT (the full-screen repaint plus baking the wallpaper)")
        last, pk = peak(ser)
        print(f"  after boot                 last {last} us   peak {pk} us")

        ser.send("peakreset\r"); ser.drain(0.6)

        # sweep the pointer across the dock: hover changes damage the dock strip
        print("\nHOVERING ACROSS THE DOCK (the strip that got expensive)")
        dy = H - 64 * ui
        for i in range(9):
            at(qmp, 118 * ui + i * 56 * ui + 24 * ui, dy + 32 * ui, W, H)
            ser.drain(0.25)
        last, pk = peak(ser)
        print(f"  nine tiles hovered         last {last} us   peak {pk} us")

        ser.send("peakreset\r"); ser.drain(0.6)

        # drag a window right across the screen - the most damage a frame gets
        print("\nDRAGGING A WINDOW ACROSS THE SCREEN")
        at(qmp, W - 290 * ui + 60, 52 * ui + 10 * ui, W, H, btn=True)
        for k in range(12):
            at(qmp, W - 290 * ui + 60 - k * 90, 52 * ui + 10 * ui + k * 40, W, H, btn=True)
            ser.drain(0.2)
        at(qmp, W - 290 * ui + 60 - 11 * 90, 52 * ui + 10 * ui + 11 * 40, W, H, btn=False)
        ser.drain(0.8)
        # CLICK THE SHELL BACK FIRST. Dragging a window focuses it, and keys go
        # to the focused window - so `peak` was being typed into the System
        # Monitor, which has no app_event, and the probe reported None. That is
        # the routing working exactly as designed, and the probe not knowing it.
        at(qmp, 300, 400, W, H, btn=True)
        at(qmp, 300, 400, W, H, btn=False)
        ser.drain(0.8)
        last, pk = peak(ser)
        print(f"  twelve drag steps          last {last} us   peak {pk} us")
        if pk:
            print(f"\n  budget is 16667 us (60 fps). peak is "
                  f"{pk / 16667.0:.2f}x that -> {1e6 / pk:.1f} fps worst frame")
        return 0
    finally:
        proc.kill(); proc.wait()


if __name__ == "__main__":
    sys.exit(main())
