#!/usr/bin/env python3
"""probe-field.py - the strip and the foot are BORDER-BOX, measured in pixels.

THE DEFECT THIS EXISTS FOR. The prototype sets `* { box-sizing: border-box }`
(proto:325), so `#raster { height: var(--zd-strip-h); border-bottom: 2px }`
(proto:486-487) is 30dp TOTAL - 28 of ZD_CUT with the 2dp ZD_LIT rule INSIDE
it - and `#foot` (proto:560-562) is 46dp total the same way.

zlOS added the rule ON TOP of each declared height. draw_strip filled the full
30 and then drew the rule below it; draw_foot drew the rule above a full 46.
field_y() and field_h() encoded the same reading. Measured at ui 1 the strip
occupied 32dp against 30 and the foot 48 against 46, and the field was `h - 80`
against the authority's `h - 76`.

IT WAS NOT ONLY COSMETIC. wm.c reserves the border-box figure -
`#define RESERVE_TOP(t) ((t)->strip_h)` at :338 - so the compositor's
clamp_to_field and kernel.zl's field disagreed by RULE_H at both ends.

WHY A PIXEL PROBE AND NOT A HOST TEST. This is two pixels at each end of the
screen. It cannot be seen and it cannot be reasoned about from the source
without redoing the arithmetic that was wrong in the first place - so the
screen is read directly, in a column chosen to be empty of content: just right
of the rail, inside the strip's own left pad, where the raster plot does not
reach.

FOUR MEASUREMENTS, in one boot:

    the strip's ground ends at      28 * ui
    its rule ends at               30 * ui   == ZD_STRIP_H, border-box
    the foot's rule starts at    H - 46 * ui == H - ZD_FOOT_H
    and is 2 * ui tall

To see it fail, put the rule back outside either band: the corresponding
boundary moves by exactly RULE_H and the figure beside it says by how much.
"""
import os, sys, subprocess, tempfile, time
sys.path.insert(0, "/home/roy/Documents/repos/zl-linux-presswork/kernel/tools/probes")
from exercise import Serial, Qmp, qemu_argv, build

ZD_CUT = (0x0A, 0x03, 0x00)
ZD_LIT = (0x6F, 0x68, 0x64)
ZD_STRIP_H = 30      # design.h:451
ZD_FOOT_H  = 46      # design.h:452
ZD_RULE_H  = 2       # design.h:457

build(False)
tmp = tempfile.mkdtemp(prefix="field-")
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

fails = 0
def check(what, got, want):
    global fails
    ok = got == want
    print("  %-42s %s  (%s, want %s)" % (what, "ok  " if ok else "FAIL", got, want))
    if not ok: fails += 1

try:
    ser, qmp = Serial(sp), Qmp(qp)
    ser.wait("compositor:", 240); ser.drain(2.0)
    A = os.path.join(tmp, "a.ppm")
    qmp.screendump(A)
    W, H, px = rd(A)
    def at(x, y):
        o = (y * W + x) * 3
        return (px[o], px[o+1], px[o+2])
    # ui is whatever the machine chose; derive it from the rule rather than
    # assuming, so this probe does not repeat probe-snap's mistake.
    ui = 2 if W >= 2560 else 1
    # a column just right of the rail: inside the strip's left pad, and the
    # raster plot starts well to the right of it
    RAIL = 170 * ui
    x = RAIL + 4 * ui
    print("booted %dx%d, reading column x=%d" % (W, H, x))

    y = 0
    while y < H and at(x, y) == ZD_CUT: y += 1
    check("the strip's ZD_CUT ground ends at", y, (ZD_STRIP_H - ZD_RULE_H) * ui)
    y0 = y
    while y < H and at(x, y) == ZD_LIT: y += 1
    check("its ZD_LIT rule ends at", y, ZD_STRIP_H * ui)
    check("...which makes the rule this tall", y - y0, ZD_RULE_H * ui)

    y = H - 1
    while y > 0 and at(x, y) != ZD_LIT: y -= 1
    while y > 0 and at(x, y) == ZD_LIT: y -= 1
    check("the foot's ZD_LIT rule starts at", y + 1, H - ZD_FOOT_H * ui)
finally:
    p.terminate()

print("\n%s: %d failure(s)" % ("FAILED" if fails else "all good", fails))
sys.exit(1 if fails else 0)
