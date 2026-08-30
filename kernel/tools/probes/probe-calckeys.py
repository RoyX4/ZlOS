#!/usr/bin/env python3
"""probe-calckeys.py - does clicking a calculator key press THAT key?

WHY THIS EXISTS. ca_draw built its keypad origin by accumulation - 42 + 16 + 16
+ 16 + 18 + 8 = 116 dp - and ca_event subtracted a flat 46, under a comment
asserting "ca_draw's grid starts at (ax + 8u, ay + 46u)". A comment stating the
opposite of the code.

The consequence was arithmetic, not opinion: '7' is drawn at ay+116u..ay+160u,
so a click on it gave cy in [70u, 114u), r = 1 or 2, and pressed '4' or '1'.
The bottom row - C, 0, = - is drawn at ay+248u..ay+292u, giving r = 4 or 5,
which `if r > 3 { return 0 }` rejected outright. Three keys unreachable and the
rest hitting the wrong one, on a pane whose whole job is that the number you
press is the number you get.

THE HARNESS IS probe-rail's, because it is the one that delivers a press: window
rects from the kernel's own `wm: win N title X,Y WxH` line, absolute events
against the usb-tablet, position and button in one event. Four hand-written
harnesses failed to click anything at all before that was understood.
"""
import os, re, subprocess, sys, tempfile, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

CA_BTN = 44          # apps_system.zl CA_BTN
GRID_Y = 116         # apps_system.zl ca_grid_y()
GRID_X = 8


def at(qmp, x, y, w, h, btn=None):
    ev = [{"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / (w - 1))}},
          {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / (h - 1))}}]
    if btn is not None:
        ev.append({"type": "btn", "data": {"down": btn, "button": "left"}})
    qmp.cmd("input-send-event", events=ev)


build(False)
tmp = tempfile.mkdtemp(prefix="zlos-calc-")
ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
try:
    from PIL import Image
    ser, qmp = Serial(ser_path), Qmp(qmp_path)
    _, log = ser.wait("ready.", 180)
    ser.wait(PROMPT, 30)
    ser.send("calcapp\r")
    # KEEP WHAT THE WAIT CONSUMES. Serial.wait RETURNS the text it swallowed,
    # and wm_report's line is printed as the window opens - before the prompt
    # comes back. Discarding the return left the rect out of `log` and the probe
    # concluded the calculator had not opened, while a screenshot showed it
    # open with the rect on the terminal behind it. Third time this exact
    # mistake today; probe-drag.py's header warns about it.
    _, more = ser.wait(PROMPT, 20)
    log += more or ""
    ser.drain(3.5)
    log += ser.buf; ser.buf = ""

    # THE CALCULATOR'S RECT, NOT THE LAST ONE PRINTED. A first version took
    # bars[-1] and clicked a 634-wide rect - the System Monitor's title bar -
    # and PASSED, because clicking any window changes pixels. A probe that can
    # pass while aimed at the wrong window is not testing anything, which is the
    # fault this file exists to catch, committed by the file itself.
    #
    # The calculator is the only 360-wide window on this desktop
    # (presswork-prototype.html APPS calc p[2] = 360), so the rect is selected
    # by that width and the run refuses if it is not there.
    bars = re.findall(r"wm: win (\d+) title (\d+),(\d+) (\d+)x(\d+)", log)
    if not bars:
        print("FAIL - no window rects on the serial log"); sys.exit(1)
    # The logged rect is the TITLE BAR, which is the window width less the
    # plate's ring and the focus bar - so a 360-wide calculator reports ~354,
    # not 360. Matching the exact number found nothing and reported the app had
    # not opened, which was wrong in a way that looks right.
    got = [b for b in bars if 348 <= int(b[3]) <= 362]
    if not got:
        print("FAIL - no 360-wide window; the calculator did not open.")
        print("       rects seen: %s" % ", ".join("%sx%s" % (b[3], b[4]) for b in bars))
        sys.exit(1)
    wid, bx, by, bw, bh = (int(v) for v in got[-1])
    print("calculator window %d at %d,%d %dx%d" % (wid, bx, by, bw, bh))

    def shot(n):
        p = os.path.join(tmp, n + ".ppm")
        qmp.screendump(p)
        return Image.open(p).convert("RGB").copy()

    a = shot("before")
    W, H = a.size
    # The client sits under the title bar. Row 0 col 0 is '7' in this layout;
    # row 3 is the bottom row that was unreachable entirely.
    cy0 = by + 28 + GRID_Y            # title_h is 28 dp at ui 1
    def key_centre(r, c):
        return (bx + GRID_X + c * CA_BTN + CA_BTN // 2,
                cy0 + r * CA_BTN + CA_BTN // 2)

    # THE POINTER IS PARKED BEFORE EVERY SHOT. A first version compared whole
    # frames after each click and PASSED against the planted bug, because the
    # CURSOR had moved between the two screenshots and that is a pixel change
    # like any other. It was measuring the mouse, not the calculator.
    def park():
        at(qmp, W - 4, H - 4, W, H)
        ser.drain(0.5)

    def click(r, c):
        x, y = key_centre(r, c)
        at(qmp, x, y, W, H)
        ser.drain(0.4)
        at(qmp, x, y, W, H, True)
        at(qmp, x, y, W, H, False)
        ser.drain(1.0)
        park()
        return x, y

    # THE DISPLAY IS THE EVIDENCE, not the frame. The calculator's readout is
    # the top strip of its client; nothing else in that band changes on its own.
    dx0, dy0 = bx, by + 28
    dx1, dy1 = bx + bw, by + 28 + 40

    def display(img):
        return [img.getpixel((x, y))
                for y in range(dy0, dy1, 2) for x in range(dx0, dx1, 2)]

    park()
    base = display(shot("base"))

    click(0, 0)
    top = display(shot("after-top"))
    d1 = sum(1 for p, q in zip(base, top) if p != q)
    print("top-left key: %d display px changed" % d1)

    click(3, 0)
    bot = display(shot("after-bottom"))
    d2 = sum(1 for p, q in zip(top, bot) if p != q)
    print("BOTTOM row (unreachable before the fix): %d display px changed" % d2)

    if d1 == 0:
        print("\nFAIL - pressing a key did not change the display")
        sys.exit(1)
    # Before the fix the bottom row gave r = 4..5 and `if r > 3 { return 0 }`
    # rejected it outright, so it could not alter the display by any route.
    # This is the assertion that discriminates, and it looks only at the
    # readout - the frame-wide version passed against the planted bug because
    # the cursor had moved.
    if d2 == 0:
        print("\nFAIL - the bottom row is still unreachable: the display did not move")
        sys.exit(1)
    print("\nPASS - keys respond, including the row that could not be reached")
finally:
    proc.terminate()
