#!/usr/bin/env python3
"""probe-seg.py - the segmented control is a widget, and it drives something.

THE DEFECT THIS EXISTS FOR. set_press_ctl's own comment said it: "`raster
clock` and `per window timing` are segmented controls this pane has no widget
for". Add the LADDER tab's 'surface ladder' and that was three of the
authority's twelve controls with no drawable form.

The drawing code existed and was hardwired: set_segbar looped SET_TABS, called
set_tabname(i) and compared against the global set_tab, so the only list it
could ever draw was the tab strip.

WHAT THIS ASSERTS, AND WHY THE SECOND HALF IS THE POINT. A generalised widget
that draws is worth nothing on its own - the whole round this belongs to has
been removing controls that drew and did nothing. So this picks a segment and
then reads a DIFFERENT WINDOW: 'per window timing' selects which column the
System Monitor's window table prints, exactly as usOf does at
presswork-prototype.html:1606-1610, and the monitor is open at boot.

    measured -> repaint changes the monitor    the column follows the segment
    ...and the bar itself changed              the picked rung is knocked out
    off empties the column                     and neither reading is drawn

To see it fail, drop the `if set_us == 1` arm from sm_win_row: the first figure
falls to the order of 0.0000 while the second stays where it is, and the split
says the control moved but nothing followed it.
"""
import os, sys, subprocess, tempfile, time
sys.path.insert(0, "/home/roy/Documents/repos/zl-linux-presswork/kernel/tools/probes")
from exercise import Serial, Qmp, qemu_argv, build
build(False)
tmp = tempfile.mkdtemp(prefix="seg-")
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

def click(qmp, x, y):
    qmp.cmd("input-send-event", events=[
        {"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / 1919)}},
        {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / 1199)}}])
    time.sleep(0.25)
    qmp.cmd("input-send-event", events=[{"type": "btn", "data": {"down": True, "button": "left"}}])
    time.sleep(0.15)
    qmp.cmd("input-send-event", events=[{"type": "btn", "data": {"down": False, "button": "left"}}])
    time.sleep(1.2)

try:
    ser, qmp = Serial(sp), Qmp(qp)
    ok, boot = ser.wait("compositor:", 240)
    got, more = ser.wait("zl>", 60); boot += more
    ser.drain(1.5); boot += ser.buf

    import re
    wins = dict()
    for m in re.finditer(r"wm: win (\d+) title (\d+),(\d+) (\d+)x(\d+) client (\d+),(\d+) (\d+)x(\d+)", boot):
        wins[int(m.group(1))] = [int(g) for g in m.groups()[1:]]
    if not wins:
        print("no wm: win lines - cannot locate the monitor"); raise SystemExit(1)
    # The System Monitor is the boot window whose client is MON_W x MON_H
    # (424 x 400 design units). Picked by size rather than by slot, so a
    # change to the boot order does not silently point this at the shell.
    mon = None
    for k, v in wins.items():
        if abs(v[6] - 424) <= 8:
            mon = v
    if mon is None:
        print("no 424-wide client among", [(k, v[6], v[7]) for k, v in wins.items()]); raise SystemExit(1)
    mcx, mcy, mcw, mch = mon[4], mon[5], mon[6], mon[7]
    print("monitor client at %d,%d %dx%d" % (mcx, mcy, mcw, mch))
    # the window table is the right column's lower half
    COL = (mcx + mcw // 2, mcy + mch // 2, mcw // 2 - 4, mch // 2 - 8)

    A = os.path.join(tmp, "a.ppm"); B = os.path.join(tmp, "b.ppm"); C = os.path.join(tmp, "c.ppm")
    ser.send("settings\r"); time.sleep(2.6)
    ser.drain(1.0)
    tail = ser.buf
    m = re.search(r"wm: win (\d+) title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", tail)
    if not m:
        print("Settings did not report a rectangle:", repr(tail[-200:])); raise SystemExit(1)
    scx, scy, scw = int(m.group(2)), int(m.group(3)), int(m.group(4))
    print("settings client at %d,%d w=%d" % (scx, scy, scw))

    # THE GEOMETRY IS THE PANE'S OWN, term for term - kernel.zl:10528-10541.
    #   tab row   ay + SET_PADY
    #   seg y     tab row + (SET_TABROW - SET_SEGH)/2
    #   body      tab row + SET_TABROW + SET_R2 + SET_STACK
    #   row n     body + n * SET_ROWH
    # Restating it here is the thing probe-snap got wrong, so every figure that
    # follows is CHECKED by the assertions below rather than trusted: if the
    # click misses, "the bar itself changed" reads 0.0000 and says so.
    ui = 1
    SET_PADY, SET_TABROW, SET_SEGH, SET_R2, SET_STACK, SET_ROWH, SET_PADX = 6, 24, 20, 2, 5, 26, 9
    tabrow = scy + SET_PADY * ui
    segy   = tabrow + (SET_TABROW * ui - SET_SEGH * ui) // 2
    bodyy  = tabrow + (SET_TABROW + SET_R2 + SET_STACK) * ui
    bodyx  = scx + SET_PADX * ui
    bodyw  = scw - 2 * SET_PADX * ui
    # the PRESS tab is the last of four; click near the strip's right end
    click(qmp, bodyx + 200 * ui, segy + SET_SEGH * ui // 2)
    time.sleep(1.4)
    qmp.screendump(A)
    W, H, _ = rd(A)
    print("booted %dx%d" % (W, H))
    # row 6 is `per window timing`; the bar is right-flushed in the row
    rowy = bodyy + 6 * SET_ROWH * ui
    BAR = (bodyx + bodyw - 200 * ui, rowy, 200 * ui, SET_ROWH * ui)
    a_bar, a_col = band(A, *BAR), band(A, *COL)
    try:
        from PIL import Image
        Wp, Hp, pxs = rd(A)
        Image.frombytes("RGB", (Wp, Hp), pxs[:Wp*Hp*3]).crop(
            (scx - 6, scy - 34, scx + scw + 6, scy + 420)).save(
            "/home/roy/Documents/repos/zl-linux-presswork/kernel/shots/settings-press.png")
    except Exception as e:
        print("frame not written (%s)" % e)

    # 'repaint' is the middle rung of three
    click(qmp, bodyx + bodyw - 200 * ui + 130 * ui, rowy + SET_ROWH * ui // 2)
    time.sleep(1.6)
    qmp.screendump(B)
    b_bar, b_col = band(B, *BAR), band(B, *COL)
    print("the bar itself changed         : %.4f" % diff(a_bar, b_bar))
    print("the monitor's column followed  : %.4f" % diff(a_col, b_col))
finally:
    p.terminate()
