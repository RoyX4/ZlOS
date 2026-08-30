#!/usr/bin/env python3
"""probe-gridtoggle.py - does CTRL G actually change the desk?

WHY THIS EXISTS. The field menu has always printed "toggle desk grid  CTRL G".
I wired the key and the state this morning, shipped it, and it did nothing: the
grid is drawn into the wallpaper CACHE, which is baked once at boot, and every
later frame blits that bitmap. The flag flipped, a toast fired saying the desk
had changed, and the desk did not change. A control that reports success and
does nothing is worse than one that visibly fails - and I shipped it in the same
hour I was fixing five other controls for exactly that fault.

THE TOAST IS NOT THE EVIDENCE, WHICH IS THE WHOLE POINT. This samples a patch of
the FIELD, far from where the toast lands, so a toast alone cannot make it pass.
The first version of a probe I wrote today passed against a deliberately broken
kernel because it measured the wrong thing; this one measures the thing the
control claims to change.
"""
import os, re, subprocess, sys, tempfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

build(False)
tmp = tempfile.mkdtemp(prefix="zlos-grid-")
ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
try:
    from PIL import Image
    ser, qmp = Serial(ser_path), Qmp(qmp_path)
    ser.wait("ready.", 180)
    ser.wait(PROMPT, 30)
    ser.drain(2.0)

    def shot(n):
        p = os.path.join(tmp, n + ".ppm")
        qmp.screendump(p)
        return Image.open(p).convert("RGB").copy()

    a = shot("before")
    W, H = a.size
    # A PATCH OF BARE FIELD. Low and right of the boot windows, well above the
    # foot, and far from the toast's bottom-right corner - so only the desk
    # itself is being measured.
    x0, y0, x1, y1 = int(W * 0.55), int(H * 0.55), int(W * 0.90), int(H * 0.80)
    print("sampling the bare field at %d,%d..%d,%d on a %dx%d screen"
          % (x0, y0, x1, y1, W, H))

    def diff(p, q):
        n = t = 0
        for y in range(y0, y1, 3):
            for x in range(x0, x1, 3):
                t += 1
                if p.getpixel((x, y)) != q.getpixel((x, y)):
                    n += 1
        return n / t if t else 0

    # THE RAIL IS SAMPLED TOO, and it is the half this probe could not see.
    #
    # Re-baking the wallpaper cache means repeating what boot did:
    # draw_wallpaper, then draw_rail_static, THEN wall_save - because
    # fb_wall_save copies what is on screen and draw_wallpaper's first act is to
    # fill the whole screen, rail included. The first version of this toggle
    # skipped draw_rail_static, so every press erased the rail's static half
    # from the cache permanently - and this probe passed it, because it looked
    # only at the field. A probe that samples one region cannot see a fault in
    # another, however carefully it measures the one it has.
    rx0, ry0, rx1, ry1 = 4, int(H * 0.05), 160, int(H * 0.75)

    def rail(img):
        return [img.getpixel((x, y))
                for y in range(ry0, ry1, 4) for x in range(rx0, rx1, 4)]

    rail_a = rail(a)

    ser.send("\x07")          # CTRL G
    ser.drain(2.5)
    b = shot("after")
    b.save("/tmp/gridtoggle.png")
    d1 = diff(a, b)
    print("field changed by %.4f after the first CTRL G" % d1)

    ser.send("\x07")          # and back
    ser.drain(2.5)
    c = shot("back")
    d2 = diff(b, c)
    d3 = diff(a, c)
    print("field changed by %.4f after the second, and differs from the start by %.4f"
          % (d2, d3))

    # The grid is thin rules on a large field, so the changed fraction is small
    # by nature - but it is not zero, and an inert toggle gives exactly zero.
    if d1 < 0.002:
        print("\nFAIL - CTRL G did not change the desk. The toggle is inert.")
        sys.exit(1)
    if d2 < 0.002:
        print("\nFAIL - the desk did not come back on the second CTRL G")
        sys.exit(1)
    # And it must RETURN to where it started, or it is not a toggle.
    if d3 > d1 / 2:
        print("\nFAIL - the second CTRL G did not restore the first state")
        sys.exit(1)

    # AND THE RAIL MUST SURVIVE BOTH PRESSES. It carries live readouts, so it is
    # never pixel-identical; what it must not do is go blank. Comparing it
    # against its own starting state catches an erase, which is total.
    rail_c = rail(c)
    same = sum(1 for p, q in zip(rail_a, rail_c) if p == q)
    frac = same / len(rail_a) if rail_a else 0
    print("rail still matches its starting state in %.3f of sampled px" % frac)
    if frac < 0.90:
        print("\nFAIL - the rail was erased by the re-bake")
        sys.exit(1)
    print("\nPASS - CTRL G changes the desk, changes it back, and leaves the rail intact")
finally:
    proc.terminate()
