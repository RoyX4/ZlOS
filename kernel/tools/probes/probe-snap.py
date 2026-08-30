#!/usr/bin/env python3
"""probe-snap.py - does dragging a window to an edge SHOW you where it lands?

visual-speed-northstar.md's five-part direction, item 3, lists "add the missing
snap preview" as outstanding. snap.c and the drop-time wiring already existed,
so dragging to an edge DID snap - there was just nothing on screen until you
let go.

A screenshot cannot tell a working preview from a picture of one, and the
compiler cannot see it at all: the preview is pure paint, so a green build
proves nothing here. This drives the real pointer through the real drag path
and asks whether the SCREEN changed, in the right half of the screen, at the
right moment.

  1 APPEAR   press a title bar, move to the LEFT edge -> left half lights up
  2 CLEAR    move back to the middle          -> it goes away again
  3 RIGHT    move to the RIGHT edge           -> the RIGHT half lights up

Step 3 is the one that catches a preview that merely draws something: a
constant rectangle, or one computed from the window instead of the pointer,
passes 1 and 2 and fails 3.
"""
import os, subprocess, sys, tempfile

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
HERE = KERNEL_ROOT
sys.path.insert(0, PROBE_DIR)
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

SHOTS = os.path.join(HERE, "shots")

SNAP_EDGE = 16      # snap.c:50 - copied, not re-derived, so a change there
                    # breaks this probe rather than silently diverging from it
TITLE_H   = 28      # design units, as in probe-catalog.py

# THE THRESHOLDS ARE CALIBRATED AGAINST A KNOWN CONFOUND, not tuned until the
# run went green. A drag moves the window with the pointer, so the 24px of
# pointer travel between each reference shot and its test shot also moves the
# window 24px - and a ~1236x834 window shifting 24px repaints on the order of
# 30,000 px all by itself. Measured on this desktop at 1920x1200:
#
#   preview present  ~985,000 px   (half the work area)
#   window travel     ~30,000 px   (the confound)
#
# 100,000 sits an order of magnitude above the confound and an order of
# magnitude below the signal, so neither a missing preview nor a stray one can
# hide inside the gap.
PRESENT = 100_000   # more than this in a half = the preview is there
ABSENT  = 100_000   # less than this = only the window moved


def at(qmp, x, y, w, h, btn=None):
    ev = [{"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / w)}},
          {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / h)}}]
    if btn is not None:
        ev.append({"type": "btn", "data": {"down": btn, "button": "left"}})
    qmp.cmd("input-send-event", events=ev)


def shot(qmp, tmp, name):
    from PIL import Image
    p = os.path.join(tmp, name + ".ppm")
    qmp.cmd("screendump", filename=p)
    return Image.open(p).convert("RGB")


def differs(a, b, box):
    """pixels that differ between two shots inside box=(x0,y0,x1,y1)"""
    ca, cb = a.crop(box), b.crop(box)
    return sum(1 for pa, pb in zip(ca.getdata(), cb.getdata()) if pa != pb)


def main():
    build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-snap-")
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
        ui = 2 if W >= 1400 else 1
        settle = ser.drain

        os.makedirs(SHOTS, exist_ok=True)

        # The shell window is opened at boot at UI_S6 from the top-left, so its
        # title bar is a few pixels in and a half-title-height down. Grabbing
        # anywhere on the title bar starts a GRAB_MOVE.
        grab_x, grab_y = 200 * ui, 40 * ui + TITLE_H * ui // 2

        # THE REFERENCE FRAME IS THE HARD PART. A drag moves the window, so
        # comparing "pointer at the edge" against "pointer in the middle"
        # measures the window travelling across the screen as much as it
        # measures the preview - the first version of this probe did exactly
        # that and reported a false failure, seeing the window VACATE the left
        # half as if it were a stray preview.
        #
        # So each zone is compared against a reference taken a few pixels the
        # OTHER side of the same threshold: same drag, window within ~12px of
        # where it was, and the only real difference on screen is whether the
        # zone is active. SNAP_EDGE + 8 is outside the zone; SNAP_EDGE // 2 is
        # inside it.
        at(qmp, grab_x, grab_y, W, H, btn=True)
        settle(0.4)
        at(qmp, W // 2, H // 2, W, H)
        settle(0.6)
        mid = shot(qmp, tmp, "mid")

        left_box  = (0, 0, W // 2, H)
        right_box = (W // 2, 0, W, H)

        # ---- 1. APPEAR - just outside the left zone vs just inside it
        at(qmp, SNAP_EDGE + 8, H // 2, W, H)
        settle(0.6)
        refl = shot(qmp, tmp, "refl")
        at(qmp, SNAP_EDGE // 2, H // 2, W, H)
        settle(0.6)
        onleft = shot(qmp, tmp, "left")
        n = differs(refl, onleft, left_box)
        ok = n > PRESENT
        print(f"  1 left edge shows a preview       {n:7d} px  {'ok' if ok else 'FAIL'}")
        if not ok: fails.append("appear")
        onleft.save(os.path.join(SHOTS, "snap-preview-left.png"))

        # ---- 2. CLEAR - back to the exact reference position. The window is
        # where it was, so anything still lit is a preview that did not clear.
        at(qmp, SNAP_EDGE + 8, H // 2, W, H)
        settle(0.6)
        back = shot(qmp, tmp, "back")
        n = differs(refl, back, left_box)
        ok = n < ABSENT
        print(f"  2 leaving the edge clears it      {n:7d} px  {'ok' if ok else 'FAIL'}")
        if not ok: fails.append("clear")

        # ---- 3. RIGHT - the preview must FOLLOW the pointer, not be a constant
        # rectangle. Same straddle, other edge: the RIGHT half must light up
        # and the LEFT half must not.
        at(qmp, W - 1 - SNAP_EDGE - 8, H // 2, W, H)
        settle(0.6)
        refr = shot(qmp, tmp, "refr")
        at(qmp, W - 1 - SNAP_EDGE // 2, H // 2, W, H)
        settle(0.6)
        onright = shot(qmp, tmp, "right")
        nr = differs(refr, onright, right_box)
        nl = differs(refr, onright, left_box)
        ok = nr > PRESENT and nl < ABSENT
        print(f"  3 right edge previews the RIGHT   {nr:7d} px right, "
              f"{nl:7d} px left  {'ok' if ok else 'FAIL'}")
        if not ok: fails.append("right")
        onright.save(os.path.join(SHOTS, "snap-preview-right.png"))

        # release, so the run leaves no button held down
        at(qmp, W - 1 - SNAP_EDGE // 2, H // 2, W, H, btn=False)
        settle(0.5)

        print("snap preview gate", "green" if not fails else f"RED {fails}")
        return 0 if not fails else 1
    finally:
        proc.kill()


if __name__ == "__main__":
    sys.exit(main())
