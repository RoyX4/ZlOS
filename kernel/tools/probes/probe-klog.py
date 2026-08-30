#!/usr/bin/env python3
"""probe-klog.py - the log's level column is read, and its filter is applied.

TWO DEFECTS THIS EXISTS FOR, and the second one was mine.

  1  The pane drew TWO columns - subsys and message - where the authority
     declares FOUR (proto R.log: time 13%, level 14%, subsys 12%, message 61%).
     The six constants naming those percentages were still in the file, read by
     nothing, which is what led to them.

  2  Following those constants found a CLOSED ISLAND of nine dead functions -
     kl_time, kl_sub, kl_level, kl_msg, kl_kind, kl_kindcol, kl_shown,
     kl_count, kl_warns - where every caller of each was itself dead. kl_filter
     was read by kl_shown, kl_shown was called by kl_count, and kl_count was
     called by nothing. I wired the command palette's `dmesg --level warn` row
     to that variable and shipped a control that changed no pixel.

WHY THE EMPTY CASE IS THE HARD ONE. This kernel's console had exactly two level
emitters, so nothing could be tagged WARN; warn_line is a third, and the three
sites that carry it are degradations a healthy machine does not reach. A clean
QEMU boot emits ten OK, one INFO and ZERO WARN - so the filter's whole visible
effect here is to empty the pane, and a blank pane is what a BROKEN filter
looks like too.

That is why the pane states the empty result instead of showing blank, and why
this probe asserts on the STATEMENT rather than on the absence of rows.

FOUR MEASUREMENTS, in one boot:

    the level column has ink          the level is drawn, not blank
    the filter emptied the rows       it is applied in the draw loop
    ...and said so                    the empty state is on screen
    toggling back restored them       and it is a filter, not a wipe

To see it fail, drop the kl_line_shown test from the row loop - which is
exactly where the predecessor's filter was NOT: the second and fourth figures
fall to 0.0000 while the first is unchanged, and that split says the setting
moved and the rows did not.
"""
import os, sys, subprocess, tempfile, time
sys.path.insert(0, "/home/roy/Documents/repos/zl-linux-presswork/kernel/tools/probes")
from exercise import Serial, Qmp, qemu_argv, build
build(False)
tmp = tempfile.mkdtemp(prefix="klog-")
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

try:
    ser, qmp = Serial(sp), Qmp(qp)
    ser.wait("compositor:", 240)
    ser.wait("zl>", 60)
    ser.drain(1.5); ser.buf = ""

    ser.send("klog\r"); time.sleep(2.6)
    ser.drain(1.0)
    import re
    m = re.search(r"wm: win (\d+) title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", ser.buf)
    if not m:
        print("the kernel log did not report a rectangle:", repr(ser.buf[-200:]))
        raise SystemExit(1)
    cx, cy, cw, ch = (int(m.group(i)) for i in (2, 3, 4, 5))
    print("kernel log client at %d,%d %dx%d" % (cx, cy, cw, ch))

    A = os.path.join(tmp, "a.ppm"); B = os.path.join(tmp, "b.ppm"); C = os.path.join(tmp, "c.ppm")
    # the table body: below the 30dp head and the column head, above the well
    ROWS = (cx + 4, cy + 56, cw - 8, ch - 130)
    # the level column is the first of three in ui_grid("70|110|*")
    LEVEL = (cx + 4, cy + 56, 70, ch - 130)

    qmp.screendump(A)
    a_rows, a_level = band(A, *ROWS), band(A, *LEVEL)
    try:
        from PIL import Image
        Wp, Hp, pxs = rd(A)
        Image.frombytes("RGB", (Wp, Hp), pxs[:Wp*Hp*3]).crop(
            (cx - 6, cy - 34, cx + cw + 6, cy + ch + 6)).save(
            "/home/roy/Documents/repos/zl-linux-presswork/kernel/shots/klog.png")
    except Exception as e:
        print("frame not written (%s)" % e)

    # THE REAL PALETTE ROUTE, not a bespoke shell word and not pixel geometry.
    # Ctrl+K opens the palette, the typed query filters its rows, and Enter runs
    # the selected one - ov_pal_run maps the VISIBLE index back through
    # ov_pal_nth, so a filtered list still runs the right absolute row. Typing
    # "dmesg" leaves exactly the `dmesg --level warn` row.
    ser.send("\x0b"); time.sleep(1.0)
    for chx in "dmesg": ser.send(chx); time.sleep(0.10)
    time.sleep(0.7)
    ser.send("\r"); time.sleep(1.8)
    qmp.screendump(B)
    b_rows = band(B, *ROWS)

    # ...and back, which is what separates a filter from a wipe.
    ser.send("\x0b"); time.sleep(1.0)
    for chx in "dmesg": ser.send(chx); time.sleep(0.10)
    time.sleep(0.7)
    ser.send("\r"); time.sleep(1.8)
    qmp.screendump(C)
    c_rows = band(C, *ROWS)

    print("the level column has ink       : %.4f" % covered(a_level))
    print("the filter emptied the rows    : %.4f" % diff(a_rows, b_rows))
    print("...and said so                 : %.4f" % covered(b_rows))
    print("toggling back restored them    : %.4f" % diff(a_rows, c_rows))
finally:
    p.terminate()
