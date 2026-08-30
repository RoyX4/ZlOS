#!/usr/bin/env python3
"""probe-image.py - the Image Viewer shows a picture, and its figures are of it.

THE DEFECT THIS EXISTS FOR. The pane drew three file tabs - "gradient.ppm",
"plasma.ppm", "mandelbrot.pgm" - and four zoom pills over a flat fill.
s3iv_sel and s3iv_zoom each had THREE occurrences in the whole tree: the
declaration, the setter, and the test that lit the pill. Nothing else read
either. The pane contradicted itself on screen: clicking a tab lit it while the
stat strip twelve pixels below still read "IMAGE  none loaded", because that
string was passed unconditionally.

The three names were not this machine's - grep found them only in the
superseded ds-reference.html - and there is no image decoder of any kind in
kernel/src. There did not need to be one: a gradient, a plasma and a Mandelbrot
set are COMPUTED. imgen.c makes all three from integers, and the tabs lost
their extensions with the files they were claiming.

FOUR MEASUREMENTS, in one boot:

    the canvas carries a picture     coverage; 0.0000 is the flat fill it was
    the tab changed the picture      plasma is not gradient
    the zoom changed the picture     1x is not 3x
    the strip followed both          the figures are of what is on the canvas

The last is the one that stops this being "draw something and print whatever
you like beside it": SOURCE PIXELS is img_w * img_h, the same divide imgen.c
does to size its own grid, so a strip that disagrees with the canvas fails here.

To see it fail, drop the img_draw call from s3iv_draw: the first figure goes to
0.0000 and takes the next two with it, while the strip keeps printing figures -
which is precisely the split the pane used to ship.
"""
import os, sys, subprocess, tempfile, time
sys.path.insert(0, "/home/roy/Documents/repos/zl-linux-presswork/kernel/tools/probes")
from exercise import Serial, Qmp, qemu_argv, build
build(False)
tmp = tempfile.mkdtemp(prefix="image-")
sp, qp = os.path.join(tmp, "s"), os.path.join(tmp, "q")
p = subprocess.Popen(qemu_argv(tmp, False, sp, qp),
                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def rd(path):
    b = open(path, 'rb').read()
    f, i = [], 2
    while len(f) < 3:
        while b[i:i+1].isspace(): i += 1
        j = i
        while not b[j:j+1].isspace(): j += 1
        f.append(int(b[i:j])); i = j
    return f[0], f[1], b[i+1:]

def band(path, x, y, w, h):
    W, H, px = rd(path); out = bytearray()
    for r in range(y, y + h):
        o = (r * W + x) * 3; out += px[o:o+w*3]
    return bytes(out)

def diff(a, b):
    return sum(1 for i in range(0, len(a), 3) if a[i:i+3] != b[i:i+3]) / (len(a) / 3)

def covered(a):
    px = [a[i:i+3] for i in range(0, len(a), 3)]
    counts, best = {}, 0
    for q in px:
        c = counts.get(q, 0) + 1
        counts[q] = c
        if c > best: best = c
    return 1.0 - best / len(px)

def click(qmp, x, y):
    qmp.cmd("input-send-event", events=[
        {"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / 1919)}},
        {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / 1199)}}])
    time.sleep(0.25)
    qmp.cmd("input-send-event", events=[{"type": "btn", "data": {"down": True, "button": "left"}}])
    time.sleep(0.15)
    qmp.cmd("input-send-event", events=[{"type": "btn", "data": {"down": False, "button": "left"}}])
    time.sleep(1.1)

try:
    ser, qmp = Serial(sp), Qmp(qp)
    ser.wait("compositor:", 240); ser.drain(1.5)
    A = os.path.join(tmp, "a.ppm"); B = os.path.join(tmp, "b.ppm"); C = os.path.join(tmp, "c.ppm")
    ser.send("image\r"); time.sleep(2.6)
    qmp.screendump(A)
    W, H, _ = rd(A)
    # sys3_new opens the Image Viewer at sys3_ox() + sys3_x(APP_IMG) by
    # sys3_y(APP_IMG), 520 x 410 design units. Computed from the source rather
    # than eyeballed, so the bands move with the layout.
    ox = max(0, (W - 1280) // 2)
    # s3_x(APP_IMG) = 280, s3_y(APP_IMG) = 120, sys3_w = 520, sys3_h = 410
    # (apps_sys3.zl:1868, :1875, :149, :157), with s3_ox() = (W - 1280)/2.
    fx, fy = ox + 280, 120
    wx, wy = fx + 4, fy + 28            # client: 1 + ZD_FOCUS_BAR left, ZD_TITLE_H top
    TB = 34
    CAN = (wx + 4, wy + TB + 4, 500, 300)
    STRIP = (wx + 4, fy + 410 - 60, 500, 34)
    try:
        from PIL import Image
        Wp, Hp, pxs = rd(A)
        Image.frombytes("RGB", (Wp, Hp), pxs[:Wp*Hp*3]).crop(
            (wx - 6, wy - 34, wx + 520, wy + 384)).save(
            "/home/roy/Documents/repos/zl-linux-presswork/kernel/shots/imageviewer.png")
    except Exception as e:
        print("frame not written (%s)" % e)
    a_can, a_strip = band(A, *CAN), band(A, *STRIP)
    # The three tabs start at 9dp with 5dp gaps; "plasma" is the middle one.
    # These two offsets are read off kernel/shots/imageviewer.png, which this
    # probe writes on every run - a click coordinate that was never looked at
    # is how the Renderer's first version measured a spin and called it a
    # picker.
    click(qmp, wx + 135, wy + 17)      # the plasma tab
    qmp.screendump(B)
    b_can, b_strip = band(B, *CAN), band(B, *STRIP)
    click(qmp, wx + 371, wy + 17)      # the 1x zoom pill
    qmp.screendump(C)
    c_can = band(C, *CAN)
    print("the canvas carries a picture   : %.4f" % covered(a_can))
    print("the tab changed the picture    : %.4f" % diff(a_can, b_can))
    print("the zoom changed the picture   : %.4f" % diff(b_can, c_can))
    print("the strip followed the tab     : %.4f" % diff(a_strip, b_strip))
finally:
    p.terminate()
