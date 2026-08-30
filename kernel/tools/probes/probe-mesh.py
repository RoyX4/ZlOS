#!/usr/bin/env python3
"""probe-mesh.py - the Renderer draws a solid, and its counts come off it.

THE DEFECT THIS EXISTS FOR. The Renderer's toolbar carried a four-way segmented
control - cube | pyramid | octa | cylinder - whose only reader was a caption
that printed TRIANGLE AND VERTEX COUNTS for each. There were no meshes. The
canvas under the caption was one flat fill, and the four figures (12/8, 6/5,
8/6, 44/24) were correct arithmetic about solids this kernel could not draw -
written down, never read, and sitting in the numeric slot beside two readings
that ARE real probes.

fb3d.c now builds all four from vertex and face tables and fills them with the
cube's own rasteriser; mesh_tris and mesh_verts walk those same tables, so the
caption is a reading of the thing on screen rather than a memory of it.

THE SPIN HAS TO BE STOPPED FIRST, AND THAT IS NOT A CONVENIENCE. rd_yaw
advances every tick, so two screendumps of a SPINNING mesh differ by a third of
the canvas whatever the picker does. The first version of this probe measured
0.3138 for "the picker changed the canvas" while its click was landing 25 px
below the toolbar and hitting nothing - a green-looking figure produced entirely
by the animation. Halting the raster first is what makes the later figure mean
what it says, and it doubles as the assertion that Stop works.

FOUR MEASUREMENTS, in one boot:

    the canvas is not a flat fill    coverage; 0.0000 means nothing was drawn
    stopped means stopped            0.0000 - the picture is still
    the picker changed the canvas    picking octa redraws it as a different one
    the caption changed with it      and the numbers moved with the picture

The third is the one the segmented control existed to promise. The fourth is
what stops the fix being "draw a cube and print whatever you like beside it":
the caption band is read separately from the canvas band, so a picture that
changes while the figures do not fails here, and so does the reverse.

To see it fail, point rd_ui's mesh3d call at a constant kind: the canvas figure
falls to the order of 0.0000 while the caption figure stays where it is - and
the split tells you which half broke.
"""
import os, sys, subprocess, tempfile, time
sys.path.insert(0, "/home/roy/Documents/repos/zl-linux-presswork/kernel/tools/probes")
from exercise import Serial, Qmp, qemu_argv, build
build(False)
tmp = tempfile.mkdtemp(prefix="mesh-")
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
    """what FRACTION of the band is not its own background.

    This counted distinct colours instead, and that reading depends on where
    the spin was stopped: the cube halts at an angle whose visible faces shade
    almost identically and the band came back with TWO inks - true, and one
    step from the flat fill it was supposed to be distinguishing. Coverage does
    not care about the angle. A flat canvas is 0.0000; anything drawn on it is
    a fraction of the band."""
    px = [a[i:i+3] for i in range(0, len(a), 3)]
    best = 0
    counts = {}
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
    time.sleep(0.9)

try:
    ser, qmp = Serial(sp), Qmp(qp)
    ser.wait("compositor:", 240); ser.drain(1.5)
    A = os.path.join(tmp, "a.ppm"); B = os.path.join(tmp, "b.ppm")
    ser.send("render\r"); time.sleep(2.2)
    qmp.screendump(A)
    W, H, _ = rd(A)
    # The Renderer opens centred at 352x326 design units. Read two bands out of
    # its client: the caption strip at the top and the canvas under it.
    # sys2_new opens this one at sys2_ox() + 800 by 340, 352 x 326 design units
    # (apps_sys2.zl:1243, :1252, :1190, :1199), with sys2_ox() = (W - 1280)/2.
    # Computed rather than eyeballed so the bands move with the layout.
    ox = max(0, (W - 1280) // 2)
    fx, fy = ox + 800, 340
    wx, wy = fx + 4, fy + 28          # client: 1 + ZD_FOCUS_BAR left, ZD_TITLE_H top
    CAP = (wx + 8, wy + 6, 320, 16)     # the mesh/tris/verts line
    CAN = (wx + 8, wy + 80, 320, 150)   # the canvas the solid is drawn on
    # HALT THE RASTER, then read twice a second apart. A moving mesh makes every
    # later figure meaningless, and this proves the Stop pill does what it says.
    click(qmp, wx + 37, wy + 255)
    time.sleep(1.0)
    qmp.screendump(A)
    a_cap, a_can = band(A, *CAP), band(A, *CAN)
    time.sleep(1.4)
    S1 = os.path.join(tmp, "s1.ppm"); qmp.screendump(S1)
    still = diff(a_can, band(S1, *CAN))
    # THE WINDOW ITSELF, KEPT. A pixel figure over a band nobody has looked at
    # is a number about a guess; this writes the frame out so the bands can be
    # checked against the picture they claim to describe.
    try:
        from PIL import Image
        Wp, Hp, pxs = rd(A)
        Image.frombytes("RGB", (Wp, Hp), pxs[:Wp*Hp*3]).crop(
            (wx - 6, wy - 34, wx + 352, wy + 300)).save(
            "/home/roy/Documents/repos/zl-linux-presswork/kernel/shots/renderer.png")
    except Exception as e:
        print("frame not written (%s)" % e)
    # the octa pill - third of four in the segmented control on the toolbar
    # The octa pill - third of four in the segmented control. Read off
    # kernel/shots/renderer.png, which this probe writes on every run: the
    # toolbar row sits 43 px above the frame's bottom edge and the four pills
    # start after the Stop button.
    click(qmp, wx + 186, wy + 255)
    time.sleep(1.2)
    qmp.screendump(B)
    b_cap, b_can = band(B, *CAP), band(B, *CAN)
    print("the canvas is not a flat fill  : %.4f" % covered(a_can))
    print("stopped means stopped          : %.4f" % still)
    print("the picker changed the canvas  : %.4f" % diff(a_can, b_can))
    print("the caption changed with it    : %.4f" % diff(a_cap, b_cap))
finally:
    p.terminate()
