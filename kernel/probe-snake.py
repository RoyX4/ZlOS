#!/usr/bin/env python3
"""probe-snake.py - does snake keep playing while another window is dragged?

That is the assertion, and it is deliberately not "snake runs". Snake ran
before: it ran in a `while alive == 1` loop that owned the screen, owned the
keyboard, and ended with "press any key". Any test that only asks whether the
board appears would have passed against that, which is exactly why the brief
words the gate the way it does.

What it actually proves, in one shot:

  app_tick advances the game       state changes with nothing typed
  app_tick does not draw           the change reaches the screen only via the
                                   repaint wm.c schedules from the damage it
                                   returned
  the loop is gone                 a second window can be dragged AT THE SAME
                                   TIME, which is impossible if snake owns the
                                   frame

Method. Open Snake, then press and HOLD the left button on another window's
title bar and move it. While the button is still down - i.e. while wm.c has a
pointer grab and is repainting a dragged window every frame - screendump the
snake window's client area twice, a beat apart, and require that it changed.
Then release and confirm the dragged window really moved, so a "pass" cannot
come from a drag that never happened.

Both rectangles come from the kernel's own `wm: win N ... client X,Y WxH` lines
rather than from arithmetic repeated here. probe-drag.py carried hardcoded
coordinates for one resolution and reported "dragging is a no-op" for a drag
that worked perfectly; a gate that lies costs more than no gate.

    ./probe-snake.py                 the gate
    ./probe-snake.py --keep-shots    ...and leave the PNGs in shots/
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
for _c in "0123456789":
    QCODE[_c] = _c


def qtype(qmp, text, settle=0.12):
    for ch in text:
        code = QCODE.get(ch)
        if code is None:
            raise RuntimeError(f"no qcode for {ch!r}")
        qmp.sendkey(code)
        time.sleep(settle)


def rel(qmp, dx, dy):
    ev = []
    if dx:
        ev.append({"type": "rel", "data": {"axis": "x", "value": dx}})
    if dy:
        ev.append({"type": "rel", "data": {"axis": "y", "value": dy}})
    if ev:
        qmp.cmd("input-send-event", events=ev)


def btn(qmp, down):
    qmp.cmd("input-send-event",
            events=[{"type": "btn", "data": {"down": down, "button": "left"}}])


def crop(path, box, step=2):
    """Every step'th pixel inside box=(x,y,w,h) as one bytes object."""
    with open(path, "rb") as fh:
        blob = fh.read()
    if not blob.startswith(b"P6"):
        return None
    fields, i = [], 2
    while len(fields) < 3:
        while i < len(blob) and blob[i:i + 1].isspace():
            i += 1
        if blob[i:i + 1] == b"#":
            while i < len(blob) and blob[i] != 0x0A:
                i += 1
            continue
        j = i
        while j < len(blob) and not blob[j:j + 1].isspace():
            j += 1
        fields.append(int(blob[i:j])); i = j
    i += 1
    w, h, _ = fields
    px = blob[i:]
    x0, y0, bw, bh = box
    x1, y1 = min(x0 + bw, w), min(y0 + bh, h)
    out = bytearray()
    for y in range(max(0, y0), max(0, y1), step):
        base = y * w * 3
        for x in range(max(0, x0), max(0, x1), step):
            p = base + x * 3
            out += px[p:p + 3]
    return bytes(out)


def differs(a, b):
    """Fraction of sampled bytes that differ."""
    if a is None or b is None or len(a) != len(b):
        return None
    n = sum(1 for i in range(0, len(a), 3) if a[i:i + 3] != b[i:i + 3])
    return n / (len(a) / 3)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--settle", type=float, default=1.2)
    ap.add_argument("--keep-shots", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()

    if not args.no_build:
        build(False)

    tmp = tempfile.mkdtemp(prefix="probesnake-")
    ser_path = os.path.join(tmp, "ser.sock")
    qmp_path = os.path.join(tmp, "qmp.sock")
    # tablet=False: zlOS reads the usb-tablet through xhci_ptr_*, which is the
    # half of b19207d that was never committed, so the PS/2 mouse is the only
    # pointer that works today. Relative events, therefore.
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path, tablet=False),
                            cwd=HERE, stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    failures = []

    def check(label, ok, detail=""):
        print(f"  {'ok  ' if ok else 'FAIL'}  {label}{('   ' + detail) if detail else ''}")
        if not ok:
            failures.append(label)

    def shot(name, box):
        ppm = os.path.join(tmp, name + ".ppm")
        if not qmp.screendump(ppm):
            return None
        px = crop(ppm, box)
        if args.keep_shots:
            os.makedirs(SHOTS, exist_ok=True)
            subprocess.run(["convert", ppm, os.path.join(SHOTS, "snake-" + name + ".png")],
                           capture_output=True)
        return px

    try:
        ser = Serial(ser_path)
        qmp = Qmp(qmp_path)

        ok, log = ser.wait("compositor:", args.boot_timeout)
        if not ok:
            print("never reached the compositor:\n" + log[-2000:]); return 1
        ser.drain(1.5)
        log += ser.buf; ser.buf = ""

        # HOME THE POINTER FIRST. Snake starts in the middle of a 46-cell
        # board moving right, so it reaches the wall and stops after 23 steps -
        # 1.6 s at SNAKE_RATE. Homing the PS/2 pointer takes longer than that,
        # so doing it after opening Snake meant every sample was taken of a
        # finished game, and the gate measured the dragged window's edge
        # crossing the sampled rectangle instead. It passed at 3.72%. Dying is
        # correct behaviour for a snake that hits a wall; sampling after it is
        # the test's bug.
        boot_wins = re.findall(
            r"wm: win (\d+) title (\d+),(\d+) (\d+)x(\d+) client (\d+),(\d+) (\d+)x(\d+)", log)
        if len(boot_wins) < 2:
            print("the compositor reported too few windows:\n" + log[-1500:]); return 1
        drag = boot_wins[1]
        tx, ty, tw, th = (int(v) for v in drag[1:5])
        gx, gy = tx + tw // 3, ty + th // 2
        for _ in range(60):
            rel(qmp, -200, -200)
        time.sleep(0.4)
        step = 80
        for _ in range(max(1, gx // step)):
            rel(qmp, step, 0)
        for _ in range(max(1, gy // step)):
            rel(qmp, 0, step)
        rel(qmp, gx % step, gy % step)
        time.sleep(0.4)

        # ...and only NOW open Snake, so the clock starts here.
        qtype(qmp, "snake\n", settle=0.06)
        ser.drain(0.35)
        log += ser.buf; ser.buf = ""

        wins = re.findall(
            r"wm: win (\d+) title (\d+),(\d+) (\d+)x(\d+) client (\d+),(\d+) (\d+)x(\d+)", log)
        # Snake reports itself when it opens (wm_report), so its rect is the
        # LAST wm: line rather than something computed here.
        if len(wins) <= len(boot_wins):
            print("Snake never opened a window:\n" + log[-1500:]); return 1
        snake = wins[-1]
        sx, sy, sw, sh = (int(v) for v in snake[5:9])
        print(f"  note  snake client {sx},{sy} {sw}x{sh}; dragging window {drag[0]} "
              f"title {tx},{ty} {tw}x{th}")

        # THE DRAGGED WINDOW MUST NEVER TOUCH THE SAMPLED RECTANGLE, and that
        # is checked here rather than assumed. A dragged window crossing the
        # snake would change those pixels by itself and the gate would pass
        # while snake sat frozen - measured on the first version of this
        # script, which dragged LEFT, ended at x 942 against a snake spanning
        # 242..974, and reported a comfortable 3.72% that was entirely the
        # monitor's own edge. That is the "predicate weaker than the property"
        # failure desktop-overnight-run.md records for probe-mouse.py.
        #
        # The drag only ever moves right and down, so the dragged window's
        # LEFT edge never goes below its starting x. One comparison settles it.
        if tx <= sx + sw:
            print(f"  FAIL  the dragged window starts at x={tx}, inside the snake's "
                  f"{sx}..{sx + sw} - the sample would be measuring the wrong thing")
            return 1
        print(f"  ok    the dragged window (x>={tx}) can never enter the snake "
              f"({sx}..{sx + sw}) - it only moves right and down")

        before_all = shot("before", (0, 0, 4096, 4096))

        # ---- press and HOLD, and drag ------------------------------------
        btn(qmp, True)
        time.sleep(0.1)
        for _ in range(6):
            rel(qmp, 12, 34)          # right and DOWN - away from snake
            time.sleep(0.03)

        # While the button is STILL DOWN, sample snake twice.
        a = shot("mid-a", (sx, sy, sw, sh))
        for _ in range(6):                 # keep the drag genuinely in flight
            rel(qmp, 8, 20)
            time.sleep(0.06)
        b = shot("mid-b", (sx, sy, sw, sh))

        btn(qmp, False)
        time.sleep(args.settle)
        after_all = shot("after", (0, 0, 4096, 4096))

        d = differs(a, b)
        if d is None:
            check("snake keeps playing while a window is dragged", False, "no screendump")
        else:
            # A snake at 70 ms/step moves several cells in 0.8 s, so this is a
            # clear signal rather than a threshold to tune. It is a FRACTION of
            # the snake window only, which is why the dragged window passing
            # over it cannot be what moved these pixels... so the drag check
            # below has to pass too, or the result means nothing.
            check("snake keeps playing while a window is dragged",
                  d > 0.002, f"{d * 100:.2f}% of its client area changed mid-drag")

        dd = differs(before_all, after_all)
        if dd is None:
            check("...and the drag really happened", False, "no screendump")
        else:
            check("...and the drag really happened", dd > 0.02,
                  f"{dd * 100:.2f}% of the screen changed")

        print()
        if failures:
            print(f"snake gate FAILED: {len(failures)}")
            return 1
        print("snake gate green: it plays while another window is dragged over it")
        return 0
    finally:
        proc.kill()
        proc.wait()
        subprocess.run(["rm", "-rf", tmp])


if __name__ == "__main__":
    sys.exit(main())
