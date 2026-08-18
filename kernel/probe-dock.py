#!/usr/bin/env python3
"""probe-dock.py - is the DOCK a control, or a picture of one?

Everything the dock does was invisible to every existing gate, because every
existing gate types. The dock is not a window, so wm_at() finds nothing under
the pointer and route_mouse dropped the click - which means "the dock is
drawn" and "the dock works" were completely different claims and only the
first one was ever checked. A screenshot cannot tell them apart: a dock that
does nothing looks exactly like a dock that does something.

Four questions, each answered by a pixel comparison rather than by eye:

  1 HOVER  moving the pointer onto a tile must CHANGE that tile
  2 PRESS  holding the button down on it must change it AGAIN, differently
  3 CLICK  releasing must open a window - the screen above the dock changes
  4 MENU   the start button must open the menu, and a click outside dismiss it

Absolute pointer events, because zlOS prefers the usb-tablet when it is there
and sending relative events into an absolute device is how probe-drag.py once
reported a working drag as a no-op.
"""
import os, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

HERE = os.path.dirname(os.path.abspath(__file__))
SHOTS = os.path.join(HERE, "shots")


def at(qmp, x, y, w, h, btn=None):
    """put the pointer AT x,y; the HID logical range is 0..32767 across the
    whole screen, so this is a proportion and not a pixel count"""
    ev = [{"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / w)}},
          {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / h)}}]
    if btn is not None:
        ev.append({"type": "btn", "data": {"down": btn, "button": "left"}})
    qmp.cmd("input-send-event", events=ev)


def shot(qmp, tmp, name):
    p = os.path.join(tmp, name + ".ppm")
    qmp.screendump(p)
    from PIL import Image
    return Image.open(p).convert("RGB").copy()


def differs(a, b, box):
    """how many pixels changed inside box - the only honest way to ask "did
    that do anything" about a control whose whole job is to look different"""
    x0, y0, x1, y1 = box
    n = 0
    for y in range(y0, y1, 2):
        for x in range(x0, x1, 2):
            if a.getpixel((x, y)) != b.getpixel((x, y)):
                n += 1
    return n


def main():
    build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-dock-")
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
        print(f"booted {W}x{H}")
        # the dock geometry, from kernel.zl: dock_y = h - 64*ui, tiles start at
        # 118*ui with a 56*ui pitch and are 48*ui wide. ui is 2 at this width.
        ui = 2 if W >= 1400 else 1
        dy = H - 64 * ui
        tile = lambda n: (118 * ui + n * 56 * ui + 24 * ui, dy + 32 * ui)
        tbox = lambda n: (118 * ui + n * 56 * ui, dy + 4 * ui,
                          118 * ui + n * 56 * ui + 48 * ui, dy + 60 * ui)

        # 1. HOVER
        tx, ty = tile(4)                      # the snake tile
        at(qmp, tx, ty, W, H)
        ser.drain(0.6)
        hov = shot(qmp, tmp, "hover")
        n = differs(base, hov, tbox(4))
        print(f"  hover changes the tile            {n:5d} px  "
              f"{'ok' if n > 20 else 'FAIL'}")
        if n <= 20: fails.append("hover")

        # 2. PRESS
        at(qmp, tx, ty, W, H, btn=True)
        ser.drain(0.6)
        prs = shot(qmp, tmp, "press")
        n = differs(hov, prs, tbox(4))
        print(f"  press changes it again            {n:5d} px  "
              f"{'ok' if n > 20 else 'FAIL'}")
        if n <= 20: fails.append("press")

        # 3. CLICK opens a window
        at(qmp, tx, ty, W, H, btn=False)
        ser.drain(2.0)
        opened = shot(qmp, tmp, "opened")
        mid = (int(W * 0.25), int(H * 0.15), int(W * 0.75), int(H * 0.6))
        n = differs(base, opened, mid)
        print(f"  clicking it OPENS a window        {n:5d} px  "
              f"{'ok' if n > 2000 else 'FAIL'}")
        if n <= 2000: fails.append("launch")
        opened.save(os.path.join(SHOTS, "dock-launched.png"))

        # 4. the start menu
        at(qmp, 40 * ui, dy + 32 * ui, W, H, btn=True)
        ser.drain(0.3)
        at(qmp, 40 * ui, dy + 32 * ui, W, H, btn=False)
        ser.drain(1.5)
        menu = shot(qmp, tmp, "menu")
        mbox = (10 * ui, dy - 260 * ui, 220 * ui, dy - 10 * ui)
        n = differs(opened, menu, mbox)
        print(f"  the start button opens the menu   {n:5d} px  "
              f"{'ok' if n > 2000 else 'FAIL'}")
        if n <= 2000: fails.append("menu")
        menu.save(os.path.join(SHOTS, "dock-menu.png"))

        # ...and a click outside dismisses it, which is WF_MODAL doing its job
        at(qmp, int(W * 0.8), int(H * 0.5), W, H, btn=True)
        ser.drain(0.3)
        at(qmp, int(W * 0.8), int(H * 0.5), W, H, btn=False)
        ser.drain(1.5)
        gone = shot(qmp, tmp, "gone")
        n = differs(menu, gone, mbox)
        print(f"  a click outside dismisses it      {n:5d} px  "
              f"{'ok' if n > 2000 else 'FAIL'}")
        if n <= 2000: fails.append("dismiss")

        print()
        print("all good" if not fails else "FAILED: " + ", ".join(fails))
        return 1 if fails else 0
    finally:
        proc.kill(); proc.wait()


if __name__ == "__main__":
    sys.exit(main())
