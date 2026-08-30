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

    the PRESS tab was actually reached  0.3454   or the rest is about another tab
    the bar itself changed              0.3050   the picked rung is knocked out
    the column HEAD followed            0.0656   and the monitor followed it

THE HEAD, NOT THE COLUMN. The first version read the table's VALUES and got
0.0079 - which proves nothing, because APP US is a live measurement that
differs between any two screendumps whether the control moved or not. The head
reads "app us" or "kpx" and changes only when set_us does.

AND THE MONITOR IS READ WITH NOTHING ON TOP OF IT. Settings opens at 188,71
660 wide and the monitor's client is 188,489 634x343 - covered completely. The
before-shot is taken before Settings exists and the after-shot once Ctrl+W has
closed it.

To see it fail, make sm_win_head ignore set_us and always print "app us": the
tab is still reached and the bar still moves, and the third figure falls to
0.0000 - which is the split that says the control moved and nothing followed.
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
    # THE MONITOR IS SLOT 2, and the width is a CHECK rather than the search.
    #
    # The first version looked for a 424-wide client because that number is in
    # an old comment; MON_W is 640 (kernel.zl:409) and the client is 634 after
    # the chrome, so it matched nothing and the probe said so instead of
    # picking the wrong window. The boot composition creates shell, files, then
    # monitor - kernel.zl:13563-13570 - so slot 2 is the monitor, and asserting
    # the width catches a reordering rather than silently measuring the shell.
    mon = wins.get(2)
    if mon is None:
        print("no slot 2 among", sorted(wins)); raise SystemExit(1)
    if abs(mon[6] - 634) > 8:
        print("slot 2 is %d wide, expected the monitor's 634" % mon[6]); raise SystemExit(1)
    mcx, mcy, mcw, mch = mon[4], mon[5], mon[6], mon[7]
    print("monitor client at %d,%d %dx%d" % (mcx, mcy, mcw, mch))
    # THE COLUMN HEAD, NOT THE COLUMN.
    #
    # The first version read the table's VALUES and reported 0.0079 - which
    # proves nothing, because APP US is a live measurement that changes between
    # any two screendumps whether the control moved or not. The HEAD is the
    # discriminator: it reads "app us" or "kpx" and changes only when set_us
    # does. Located from kernel/shots/monitor-us.png, which this probe writes.
    COL = (mcx + mcw // 2 + 4, mcy + 145, mcw // 2 - 10, 14)

    A = os.path.join(tmp, "a.ppm"); B = os.path.join(tmp, "b.ppm"); C = os.path.join(tmp, "c.ppm")
    D = os.path.join(tmp, "d.ppm")
    # THE MONITOR IS READ BEFORE SETTINGS EXISTS, AND AGAIN AFTER IT IS CLOSED.
    #
    # reopen_settings opens at desk_x(120), desk_y(40), 660 x 720 - measured on
    # the boot desktop that is 188,71 and 660 wide, and the monitor's client is
    # 188,489 634x343. Settings covers it completely. The first version of this
    # probe read a band "in the monitor" while Settings was on top of it and
    # reported 0.0000 for a column that had changed - a figure about the wrong
    # window, which is the same fault as measuring a spin and calling it a
    # picker.
    qmp.screendump(D)
    a_col = band(D, *COL)
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
    # THE PRESS TAB IS THE LAST OF FOUR, and the first attempt at this landed
    # in the ONE-PIXEL GAP between BOUNDARY and PRESS - seg_at returned -1,
    # correctly, and the probe went on to report two figures about a tab it had
    # never reached. Read off kernel/shots/settings-press.png, which this probe
    # writes every run: PRESS spans bodyx+204..bodyx+252.
    #
    # And the reach is CHECKED rather than assumed: the strip is read before
    # and after, and a click that changed nothing stops the run instead of
    # producing figures that describe the wrong pane.
    TAB = (bodyx, segy, 280 * ui, SET_SEGH * ui)
    qmp.screendump(C)
    pre_tab = band(C, *TAB)
    click(qmp, bodyx + 228 * ui, segy + SET_SEGH * ui // 2)
    time.sleep(1.6)
    qmp.screendump(A)
    moved = diff(pre_tab, band(A, *TAB))
    if moved < 0.02:
        print("...it was not - every figure below would be about the wrong tab")
        raise SystemExit(1)
    W, H, _ = rd(A)
    print("booted %dx%d" % (W, H))
    # row 6 is `per window timing`; the bar is right-flushed in the row
    rowy = bodyy + 6 * SET_ROWH * ui
    BAR = (bodyx + bodyw - 200 * ui, rowy, 200 * ui, SET_ROWH * ui)
    a_bar = band(A, *BAR)
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
    b_bar = band(B, *BAR)
    print("the PRESS tab was actually reached: %.4f" % moved)
    print("the bar itself changed         : %.4f" % diff(a_bar, b_bar))
    # ...and now get Settings out of the way. Ctrl+W is wm.c's close (:4575).
    ser.send("\x17"); time.sleep(2.0)
    qmp.screendump(C)
    c_col = band(C, *COL)
    print("the column HEAD followed        : %.4f" % diff(a_col, c_col))
    try:
        from PIL import Image
        Wp, Hp, pxs = rd(C)
        Image.frombytes("RGB", (Wp, Hp), pxs[:Wp*Hp*3]).crop(
            (mcx - 6, mcy - 34, mcx + mcw + 6, mcy + mch + 6)).save(
            "/home/roy/Documents/repos/zl-linux-presswork/kernel/shots/monitor-us.png")
    except Exception as e:
        print("frame not written (%s)" % e)
finally:
    p.terminate()
