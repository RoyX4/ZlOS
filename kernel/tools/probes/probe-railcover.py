#!/usr/bin/env python3
"""probe-railcover.py - can a window be dragged over the register rail?

The prototype states this as RULE 1 - "THE RAIL MUST NEVER BE COVERED" - and
enforces it in layout() with an explicit clamp. zlOS enforced it in exactly one
place, inside snap_to_rect, so a SNAPPED window sat beside the rail correctly
while a DRAGGED one could cover it completely: wm_move assigned x and y
verbatim and route_mouse's GRAB_MOVE handed it a raw pointer delta. The rail is
stated to be the only route to every system function, and the title bar you
would grab to drag a window back off it is the first part to go under.

TWO ASSERTIONS, AND THE FIRST ONE IS THE IMPORTANT ONE. A first version of this
probe checked only that the rail was unchanged, and it PASSED against a kernel
with the clamp deliberately disabled - because the drag never happened, so of
course nothing reached the rail. A test that cannot fail is not evidence. This
one asserts the window MOVED before it asserts where it stopped, and it reads
the window's rect from the kernel's own `wm: win N title X,Y WxH` line rather
than hunting for a title bar in pixels.

THE PRIMITIVES ARE probe-drag's, DELIBERATELY. Its header records the trap:
"it made an earlier run of this script report 'dragging is a no-op' when the
drag was fine and the harness was sending relative events into an absolute
device." A hand-written harness hit that same wall several ways before this was
read. zlOS prefers the usb-tablet, so: absolute events, scaled across (w - 1),
button as its own event, walked in steps so the drag loop sees intermediate
positions the way it would from a hand.
"""
import os, re, subprocess, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, ppm_sample, frame_delta, PROMPT

RAIL_DP = 170          # ZD_RAIL_W


def absolute(qmp, x, y, w, h):
    qmp.cmd("input-send-event", events=[
        {"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / (w - 1))}},
        {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / (h - 1))}}])


def btn(qmp, down):
    qmp.cmd("input-send-event",
            events=[{"type": "btn", "data": {"down": down, "button": "left"}}])


def img(qmp, tmp, name):
    from PIL import Image
    p = os.path.join(tmp, name + ".ppm")
    qmp.screendump(p)
    return Image.open(p).convert("RGB").copy(), p


build(False)
tmp = tempfile.mkdtemp(prefix="zlos-rail-")
ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
try:
    ser, qmp = Serial(ser_path), Qmp(qmp_path)
    # KEEP WHAT THE WAIT CONSUMES. Serial.wait RETURNS the text it swallowed,
    # and the `wm: win N title ...` lines are printed BEFORE "ready." - so
    # waiting for the prompt and then reading the buffer finds an empty one and
    # the probe refuses to run, blaming the compositor for saying nothing.
    # probe-drag carries this exact warning in a comment: "the first wait
    # already consumed it, so waiting for it a second time burns the whole
    # timeout and then falsely reports that no rectangles were printed."
    _, log = ser.wait("ready.", 180)
    ser.drain(1.5)
    log += ser.buf; ser.buf = ""

    # THE RECTS COME FROM THE KERNEL, not from a pixel hunt. The last window
    # reported is the one on top, and its title bar is the one a press reaches.
    bars = re.findall(r"wm: win (\d+) title (\d+),(\d+) (\d+)x(\d+)", log)
    if not bars:
        print("FAIL - the compositor reported no window rects; refusing to press at a guess")
        sys.exit(1)
    wid, bx, by, bw, bh = (int(v) for v in bars[-1])
    gx, gy = bx + bw // 3, by + bh // 2
    print("grabbing window %d's title bar at %d,%d (rect %d,%d %dx%d)"
          % (wid, gx, gy, bx, by, bw, bh))

    base, base_p = img(qmp, tmp, "base")
    W, H = base.size
    print("screen %dx%d, the rail is the first %d px" % (W, H, RAIL_DP))

    absolute(qmp, gx, gy, W, H)
    ser.drain(1.0)
    btn(qmp, True)
    ser.drain(0.4)
    # Aim well past the left edge AND well above the top: if nothing clamps,
    # this lands the window over the rail and under the raster strip.
    dx, dy = -400, -400
    steps = 24
    for i in range(1, steps + 1):
        absolute(qmp, max(0, gx + (dx - gx) * i // steps),
                      max(0, gy + (dy - gy) * i // steps), W, H)
        ser.drain(0.08)
    ser.drain(0.6)
    btn(qmp, False)
    ser.drain(1.5)

    after, after_p = img(qmp, tmp, "after")
    after.save("/tmp/railcover.png")

    # ASSERTION 1: something actually moved. Without this the rail check below
    # passes on a kernel that ignores the drag entirely - which is exactly how
    # the first version of this probe passed against a disabled clamp.
    delta = frame_delta(ppm_sample(base_p), ppm_sample(after_p))
    print("the screen changed by %.2f%% during the drag" % (delta * 100))
    if delta < 0.01:
        print("\nFAIL - nothing moved, so this run proves nothing about the rail")
        sys.exit(1)

    # ASSERTION 2: and it did not land on the rail.
    changed = total = 0
    for y in range(40, H - 60, 3):
        for x in range(4, RAIL_DP - 4, 3):
            total += 1
            if base.getpixel((x, y)) != after.getpixel((x, y)):
                changed += 1
    frac = changed / total if total else 0
    print("the rail column changed by %.4f" % frac)
    # THE THRESHOLD IS MEASURED FROM BOTH STATES, NOT GUESSED. Run against a
    # kernel with clamp_to_field disabled the rail column changes 0.0450; with
    # it, 0.0033 - the rail's own live readouts, which repaint either way. A
    # first version guessed 0.05 and so PASSED the disabled-clamp build by five
    # thousandths, reporting a broken kernel as fixed. 0.02 sits an order of
    # magnitude clear of the quiet case and less than half the broken one.
    if frac > 0.02:
        print("\nFAIL - the rail was covered; rule 1 is not enforced")
        sys.exit(1)
    print("\nPASS - the window moved and stopped at the field's edge; the rail is intact")
finally:
    proc.terminate()
