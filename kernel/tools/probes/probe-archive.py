#!/usr/bin/env python3
"""probe-archive.py - "Create archive" creates an archive.

THE DEFECT THIS EXISTS FOR. The button was primary-styled, labelled with a
verb, and its entire effect was `s3ar_open = 1` - a VIEW flag that switched the
pane from its empty state to a grid of the zlfs directory. No ustar header was
emitted, no block was written, no file appeared. Beside it the toolbar drew the
literal "/tmp/build.tar", right-flushed where a target belongs, for a path
nothing on the volume answered to - zlfs has no directories, so it could not
have. And "Extract to /tmp" had no hit test at all: the click was not even
consumed.

WHY THE ASSERTION IS THE SHELL AND NOT THE PANE. The pane's own toolbar now
prints whether zlfs.tar is on the volume, and a probe that read THAT would be
asking the pane to confirm its own click - the same window agreeing with
itself. `ls` walks the directory through a different code path and prints to
the console, so a name appearing there is a file on the disk.

tartest.c is the other half and the more important one: it hands the bytes to
the SHELL'S OWN tar. This probe only proves the button reaches that code.

THREE MEASUREMENTS, in one boot:

    ls does not list it, before      the volume starts without one
    ls lists it, after               the button wrote a file
    the toolbar target changed       and the pane says so too

To see it fail, put `s3ar_open = 1` back as the whole of the Create arm: the
second figure goes to 0 and the third goes with it.
"""
import os, sys, subprocess, tempfile, time
sys.path.insert(0, "/home/roy/Documents/repos/zl-linux-presswork/kernel/tools/probes")
from exercise import Serial, Qmp, qemu_argv, build
build(False)
tmp = tempfile.mkdtemp(prefix="arc-")
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
    time.sleep(1.4)

def ls_says(ser):
    ser.drain(0.4); ser.buf = ""
    ser.send("ls\r")
    time.sleep(1.4)
    ser.drain(0.8)
    return ser.buf

try:
    ser, qmp = Serial(sp), Qmp(qp)
    ser.wait("compositor:", 240); ser.drain(1.5)

    # THE VOLUME HAS TO EXIST FIRST, and this probe's first run is the reason
    # the line is here rather than assumed: qemu_argv hands the guest a fresh
    # raw nvme.img, which is UNFORMATTED, so zlfs does not mount and Create
    # archive correctly refused. The figures came back 0 / 0 / 0.0000 and read
    # exactly like a dead button. `format` then `edit`+Escape is the same
    # sequence probe-files-click.py uses: `touch` is not a command here, the
    # editor's save is what creates a file.
    ser.send("format\r"); time.sleep(1.8)
    ser.send("edit\r");   time.sleep(1.4)
    ser.send("\x1b");     time.sleep(1.4)
    ser.send("clip\r");   time.sleep(1.4)
    before = ls_says(ser)

    A = os.path.join(tmp, "a.ppm"); B = os.path.join(tmp, "b.ppm")
    ser.send("archive\r"); time.sleep(2.4)
    qmp.screendump(A)
    W, H, _ = rd(A)
    # s3_x(APP_ARCH) = 196, s3_y = 104, sys3_w = 590, sys3_h = 384
    # (apps_sys3.zl:1867, :1874, :148, :156), s3_ox() = (W - 1280)/2.
    ox = max(0, (W - 1280) // 2)
    fx, fy = ox + 196, 104
    wx, wy = fx + 4, fy + 28
    # the target line lives at the toolbar's right end
    TARGET = (wx + 300, wy + 4, 270, 20)
    a_t = band(A, *TARGET)
    click(qmp, wx + 70, wy + 17)        # "Create archive", the first button
    time.sleep(1.6)
    qmp.screendump(B)
    b_t = band(B, *TARGET)
    # THE SHELL HAS TO HAVE FOCUS BEFORE ls CAN BE TYPED AT IT. Serial bytes
    # arrive as EV_CHAR and wm.c routes them to the FOCUSED window, so after
    # the click the "ls" went into the Archive Manager, which ignores it - and
    # this probe's third run reported an empty console and a written archive
    # side by side. Ctrl+W is wm.c's close (:4575), and closing the pane hands
    # focus back to the plate underneath it.
    ser.send("\x17"); time.sleep(1.6)
    after = ls_says(ser)
    try:
        from PIL import Image
        Wp, Hp, pxs = rd(B)
        Image.frombytes("RGB", (Wp, Hp), pxs[:Wp*Hp*3]).crop(
            (wx - 6, wy - 34, wx + 590, wy + 358)).save(
            "/home/roy/Documents/repos/zl-linux-presswork/kernel/shots/archive.png")
    except Exception as e:
        print("frame not written (%s)" % e)
    # THE VOLUME MUST ACTUALLY HAVE SOMETHING IN IT, or "the archive was
    # written" and "there was nothing to archive" look the same from here.
    # WHAT ls ACTUALLY SAID, printed. The first two runs of this probe reported
    # 0 / 0 and read like a dead button while the screenshot showed the archive
    # on the volume - the assertion was wrong, not the code. A parse nobody has
    # seen the input for is a guess.
    print("ls before                      : %r" % before[-260:])
    print("ls after                       : %r" % after[-260:])
    print("ls does not list it, before    : %d" % before.count("zlfs.tar"))
    print("ls lists it, after             : %d" % after.count("zlfs.tar"))
    print("the toolbar target changed     : %.4f" % diff(a_t, b_t))
finally:
    p.terminate()
