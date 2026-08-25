#!/usr/bin/env python3
"""probe-tray.py - does a SHRINKING number in the dock tray erase its own tail?

The tray reads `frame N us peak N up N`. Zac photographed it reading
`frame 0  us peak 0  )08  up 1` - the `)08` is not a value, it is the head of a
previous, wider peak that was never erased. docs/archive/prompts/POINTER-PROMPT.md 1b diagnosed it as
"the status numbers are drawn at fixed x-offsets with no background clear".

THAT DIAGNOSIS IS WRONG, and the fix that follows from it would have been wrong
too. desk_draw DOES clear: it blits the wallpaper cache, which has the whole
dock baked into it, over the damage rectangle before draw_status draws a thing.
The defect is that the damage rectangle did not cover the row.

  kernel.zl  draw_status()   lays the row out from tray_x() = px_w() - 300 * ui()
  kernel.zl  app_tick()      invalidated  px_w() - 130 * ui(), 120 * ui() wide

Two origins for one row - the exact defect tray_x() was extracted to kill. The
invalidated strip began at tray + 170 * ui() while the row begins at tray + 0,
so both LIVE numbers sat outside their own damage rectangle:

  wm_us()    at tray +  44 * ui()   never invalidated at all
  wm_peak()  at tray + 152 * ui()   invalidated from +170 on, so its first
                                    18 * ui() columns were never cleared

Only `up` looked right, and only because `up` is the one field that happened to
fall inside the rectangle.

HOW THIS TESTS IT, without asserting on a number whose value it cannot predict.

A frame time is not reproducible under emulation, so "the tray equals this
reference PNG" is not a gate, it is a flake. What IS reproducible is that the
tray must not depend on its own history:

  1. boot - the boot peak is five digits, the most expensive frame there is
  2. `peakreset` - the value shrinks by four digits
  3. wait for the once-a-second tick repaint, photograph the peak field   -> X
  4. hover a dock tile. desk_click damages the WHOLE dock strip
     (kernel.zl:3288), so the tray is repainted from the wallpaper cache with
     no history at all. photograph the same field                        -> Y
  5. X must equal Y, pixel for pixel

Y is ground truth by construction. If X differs from Y then the tick path left
something on screen that a full repaint does not, which is the bug and nothing
else. The peak VALUE is read over serial either side and the comparison is only
made when it did not move - and because the debris persists until a full
repaint, waiting longer is always allowed. Nothing here waits a fixed time for a
boot; --settle is a deliberate duration for a once-a-second tick, not a guess at
how fast the host is.

The geometry is read from the kernel's own boot line rather than hardcoded:

  fb: 1280x800x32 cell 8x16 ui 1x, back ON (...)
"""
import os, re, subprocess, sys, tempfile, time

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
HERE = KERNEL_ROOT
sys.path.insert(0, PROBE_DIR)
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

SHOTS = os.path.join(HERE, "shots")

# straight out of kernel.zl's tray table, and the only numbers this file knows
DOCK_X0, DOCK_PITCH = 118, 56       # first tile, tile-to-tile spacing
TRAY_W = 330                        # tray_x() = px_w() - TRAY_W * ui()
PEAK_OFF, LATE_OFF = 124, 172       # TRAY_PEAK_N .. TRAY_LATE_L
NUM_W = 48                          # TRAY_NUM_W - what a number cell needs
DOCK_BACK = 64                      # dock_y() = px_h() - 64 * ui()

# GRUB's default list tops out at 1280x720 and in practice lands on 800x600,
# where the nine dock tiles leave 162 px and draw_status drops every field past
# `frame` because it will not draw off-screen. The field under test would then
# be blank in BOTH shots and this gate would pass by comparing nothing at all.
# Ask for the mode the reference shots were taken at.
GFXMODE = "1920x1200,1280x800,auto"


def tray_geometry(w, h, ui):
    """the peak field, in screen pixels - mirroring kernel.zl's own arithmetic"""
    tray = w - TRAY_W * ui
    tdend = DOCK_X0 * ui + 9 * DOCK_PITCH * ui + 16 * ui
    if tray < tdend:
        tray = tdend
    dy = h - DOCK_BACK * ui
    # x from where the peak number starts to where the `late` label starts:
    # this box holds the peak number and nothing else that can change
    return (tray + PEAK_OFF * ui, dy + 12 * ui,
            (LATE_OFF - PEAK_OFF) * ui, 30 * ui), tray, dy


def grab(qmp, tmp, n):
    p = os.path.join(tmp, f"shot{n}.ppm")
    if not qmp.screendump(p):
        return None
    from PIL import Image
    return Image.open(p).convert("RGB").copy()


def peak(ser):
    """Wait for FIN, the LAST marker the `peak` command prints, and parse both
    of its lines out of the one chunk. Waiting for END instead would return
    halfway through the answer, and Serial.wait leaves the remainder in the
    buffer for the next call to trip over - which is exactly what happened."""
    ser.send("peak\r")
    ok, out = ser.wait("FIN", 30)
    m = re.search(r"FRAMEUS\s+(\d+)\s+(\d+)\s+END", out)
    n = re.search(r"FRAMEMISS\s+late\s+(\d+)\s+lost\s+(\d+)\s+of\s+(\d+)", out)
    return ((int(m.group(1)), int(m.group(2))) if m else (None, None),
            (int(n.group(1)), int(n.group(2)), int(n.group(3))) if n else None)


def at(qmp, x, y, w, h):
    qmp.cmd("input-send-event", events=[
        {"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / w)}},
        {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / h)}}])


def main():
    settle = float(os.environ.get("ZLOS_TRAY_SETTLE", "2.5"))
    os.environ.setdefault("ZLOS_GFXMODE", GFXMODE)
    build(False)
    os.makedirs(SHOTS, exist_ok=True)
    tmp = tempfile.mkdtemp(prefix="zlos-tray-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        ok, boot = ser.wait("ready.", 240)
        if not ok:
            print("FAIL: never booted")
            return 1
        # The kernel says what mode it actually got - do not assume one, and take
        # the LAST report, not the first. fb_setup prints this line every time it
        # runs, and it runs at least twice: once at console init with whatever
        # GRUB handed over, and again from kernel.zl's own set_res() ladder,
        # which re-modesets to 1920x1200 whenever the bootloader's mode was under
        # 1900 wide. Reading the first one reports 800x600 for a desktop that is
        # running at 1920x1200, which is how this gate first refused to run.
        modes = re.findall(r"fb:\s+(\d+)x(\d+)x\d+\s+cell\s+\d+x\d+\s+ui\s+(\d+)x", boot)
        if not modes:
            print("FAIL: no 'fb: WxHxB cell CWxCH ui Nx' line in the boot log")
            return 1
        if len(modes) > 1:
            print("  modes reported, in order: " +
                  " -> ".join(f"{w}x{h} ui{u}" for w, h, u in modes))
        W, H, UI = (int(v) for v in modes[-1])
        box, tray, dy = tray_geometry(W, H, UI)
        print(f"  screen {W}x{H} ui {UI}x, tray_x {tray}, dock_y {dy}")
        print(f"  peak field {box}")

        # draw_status drops any field that would run off the right edge, so on a
        # narrow screen the box below is blank and comparing it proves nothing.
        # Refuse rather than pass: a gate that cannot fail is worse than none.
        avail = W - tray
        if avail < (PEAK_OFF + NUM_W) * UI:
            print(f"FAIL: the tray row does not fit at {W}x{H} - {avail} px available, "
                  f"{(PEAK_OFF + NUM_W) * UI} needed, so the peak field is not drawn "
                  f"and there is nothing here to compare")
            return 1

        ser.wait(PROMPT, 60)
        ser.drain(2.0)

        (last, boot_peak), miss = peak(ser)
        print(f"  boot peak {boot_peak} us  ({len(str(boot_peak))} digits on screen)")
        print(f"  miss counters late/lost/painted {miss}")
        if boot_peak is None:
            print("FAIL: could not read the frame timer")
            return 1
        if boot_peak < 1000:
            print(f"FAIL: boot peak {boot_peak} us is under four digits - this test "
                  "needs a wide value to shrink FROM, and has nothing to prove")
            return 1

        # the shrink. everything after this is about what is left behind.
        ser.send("peakreset\r")
        ser.drain(0.6)

        # a dock tile to hover, well clear of the tray: tile 0 sits at DOCK_X0
        tile_x, tile_y = (DOCK_X0 + 24) * UI, dy + 32 * UI

        x_img = y_img = None
        for attempt in range(1, 4):
            time.sleep(settle)          # let the once-a-second tick repaint fire
            (_, v1), _ = peak(ser)
            x_img = grab(qmp, tmp, f"x{attempt}")

            at(qmp, tile_x, tile_y, W, H)   # -> desk_click damages the whole dock
            time.sleep(0.8)
            (_, v2), _ = peak(ser)
            y_img = grab(qmp, tmp, f"y{attempt}")
            at(qmp, W // 2, H // 3, W, H)   # pointer back off the dock

            if x_img is None or y_img is None:
                print("FAIL: screendump failed")
                return 1
            if v1 == v2:
                print(f"  peak stable at {v1} us across both shots - comparing")
                break
            print(f"  peak moved {v1} -> {v2} between shots, retrying "
                  f"(the debris persists until a full repaint, so this is safe)")
        else:
            print("INCONCLUSIVE: the peak never held still across a pair of shots")
            return 1

        xc, yc = x_img.crop((box[0], box[1], box[0] + box[2], box[1] + box[3])), \
                 y_img.crop((box[0], box[1], box[0] + box[2], box[1] + box[3]))
        xc.resize((xc.width * 3, xc.height * 3), 0).save(
            os.path.join(SHOTS, "tray-peak-tick.png"))
        yc.resize((yc.width * 3, yc.height * 3), 0).save(
            os.path.join(SHOTS, "tray-peak-full.png"))

        xd, yd = list(xc.getdata()), list(yc.getdata())
        diff = sum(1 for a, b in zip(xd, yd) if a != b)
        total = xc.width * xc.height
        print(f"  peak field: {diff} of {total} px differ between the tick "
              f"repaint and a full repaint")
        print("  shots/tray-peak-tick.png  shots/tray-peak-full.png  (3x)")

        # TWO BLANK CROPS ARE EQUAL. If the field ever stops being drawn - the
        # layout table moves, the fit rule tightens, the crop drifts off the
        # digits - this comparison starts passing for the wrong reason and keeps
        # passing for ever. docs/archive/prompts/POINTER-PROMPT.md 2.4: a gate that has never gone red
        # has not been tested, and one that CANNOT go red is decoration.
        ink = len(set(yd))
        if ink < 3:
            print(f"FAIL: the ground-truth crop has {ink} distinct colour(s) - "
                  f"there is no number in the field under test, so an equal "
                  f"comparison would prove nothing")
            return 1
        if diff:
            print("FAIL: the tick repaint left pixels a full repaint does not - "
                  "the tray's damage rectangle does not cover the row it draws")
            return 1
        print("PASS: a shrunk peak looks the same however the tray got repainted")
        return 0
    finally:
        proc.terminate()
        try:
            proc.wait(10)
        except subprocess.TimeoutExpired:
            proc.kill()


if __name__ == "__main__":
    sys.exit(main())
