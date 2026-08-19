#!/usr/bin/env python3
"""probe-resize.py - can a window actually be RESIZED, and did the fade land?

wm_resize() existed from the day wm.c was written and had no caller at all, so
"a window table with resize" and "a desktop you can resize a window in" were
different claims and only the first was ever true. wmtest asserts the geometry;
this asserts that a person dragging the corner of a real window on a real boot
changes what is on the screen, which is a different question and the one that
was wrong.

Absolute pointer events, because zlOS prefers the usb-tablet when it is there.
"""
import os, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

HERE = os.path.dirname(os.path.abspath(__file__))
SHOTS = os.path.join(HERE, "shots")


def at(qmp, x, y, w, h, btn=None):
    ev = [{"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / w)}},
          {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / h)}}]
    if btn is not None:
        ev.append({"type": "btn", "data": {"down": btn, "button": "left"}})
    qmp.cmd("input-send-event", events=ev)


def shot(qmp, tmp, name):
    from PIL import Image
    p = os.path.join(tmp, name + ".ppm")
    qmp.screendump(p)
    return Image.open(p).convert("RGB").copy()


def diff(a, b, box):
    x0, y0, x1, y1 = box
    n = 0
    for y in range(y0, y1, 2):
        for x in range(x0, x1, 2):
            if a.getpixel((x, y)) != b.getpixel((x, y)):
                n += 1
    return n


def main():
    build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-resize-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    fails = []
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        if not ser.wait("ready.", 240)[0]:
            print("never booted"); return 1
        ser.wait(PROMPT, 60)
        ser.drain(1.5)
        base = shot(qmp, tmp, "base")
        W, H = base.size
        # fb.c keeps common 1080/1200p modes at 1x and rounds the q8 scale to
        # 2x only from 2880px wide. Keep the probe on the same rule.
        ui = 2 if W >= 2880 else 1
        print(f"booted {W}x{H}, ui {ui}x")

        # The boot workspace is a centred 1280-unit canvas. Do not guess from
        # the right edge: changing the composition should break this probe at
        # the same coordinate the visible window actually moved to.
        ox = max(0, (W - 1280 * ui) // 2)
        mx, my = ox + 800 * ui, 160 * ui
        mw, mh = 424 * ui, 376 * ui
        # its bottom-right corner, one pixel inside
        cx, cy = mx + mw - 3, my + mh - 3

        # SHRINK, not grow, and for two reasons. The System Monitor sits
        # against the right edge - 1908 of 1920 - so growing it would drag the
        # pointer off the screen, where an absolute event has nowhere to land.
        # And shrinking is the harder direction anyway: the band the window
        # vacates has to be repainted as wallpaper, and under-damaging it
        # leaves a ghost of the old frame that no later repaint ever cleans up.
        SHRINK = 160
        vacated = (mx + mw - SHRINK + 4, my + mh - SHRINK + 4, mx + mw, my + mh)

        at(qmp, cx, cy, W, H)
        ser.drain(0.5)
        at(qmp, cx, cy, W, H, btn=True)
        ser.drain(0.4)
        at(qmp, cx - SHRINK, cy - SHRINK, W, H, btn=True)
        ser.drain(1.2)
        at(qmp, cx - SHRINK, cy - SHRINK, W, H, btn=False)
        ser.drain(1.5)
        smaller = shot(qmp, tmp, "smaller")
        n = diff(base, smaller, vacated)
        print(f"  dragging the corner RESIZES it         {n:5d} px  "
              f"{'ok' if n > 500 else 'FAIL'}")
        if n <= 500: fails.append("resize")
        smaller.save(os.path.join(SHOTS, "resize-after.png"))

        bigger = smaller

        # ...and the title bar still MOVES it rather than resizing
        at(qmp, mx + 60, my + 10 * ui, W, H, btn=True)
        ser.drain(0.4)
        at(qmp, mx - 240, my + 200, W, H, btn=True)
        ser.drain(1.2)
        at(qmp, mx - 240, my + 200, W, H, btn=False)
        ser.drain(1.2)
        moved = shot(qmp, tmp, "moved")
        n = diff(bigger, moved, (int(W * 0.3), 0, W, int(H * 0.7)))
        print(f"  the title bar still MOVES it           {n:5d} px  "
              f"{'ok' if n > 2000 else 'FAIL'}")
        if n <= 2000: fails.append("move")
        moved.save(os.path.join(SHOTS, "resize-moved.png"))

        # ...AND WHAT THE SHRINK VACATED WAS WALLPAPER, not a ghost of the old
        # frame. A resize that under-damages leaves the old border and shadow
        # behind and nothing ever repaints them, because nothing knows they are
        # there - the same failure as the 12 px drag smear C4 deleted.
        #
        # The reference has to be a REAL picture of that region with no window
        # in it, which is what `moved` is now that the window has been dragged
        # away. An earlier version compared against wallpaper 40 px to the left
        # and failed on a gradient: the v10 wallpaper has three radial glows
        # and two conic sweeps in it, so it is not constant along a row and
        # never was. A wrong reference is a wrong test, however red it goes.
        ghost = diff(bigger, moved, vacated)
        print(f"  ...and what it vacated was wallpaper   {ghost:5d} px  "
              f"{'ok' if ghost < 300 else 'FAIL'}")
        if ghost >= 300: fails.append("ghost")

        print()
        print("all good" if not fails else "FAILED: " + ", ".join(fails))
        return 1 if fails else 0
    finally:
        proc.kill(); proc.wait()


if __name__ == "__main__":
    sys.exit(main())
