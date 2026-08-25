#!/usr/bin/env python3
"""probe-apps.py - do all the converted apps really run in windows, at once?

Item 6's gate. Each of snake / paint / 3D / anim / pointer used to be a
full-screen loop that owned the machine and ended with "press any key". They
are app_draw + app_event + app_tick now, and the claim is not "each one still
works" - it is that ALL OF THEM ARE RUNNING SIMULTANEOUSLY, which is only
possible once none of them owns the frame.

So: open all five, let them run, and assert

  every window opened            the kernel reports a rect for each
  the animated ones ANIMATE      two screendumps a beat apart differ inside
                                 each of their own client rectangles, with
                                 nothing typed in between
  the static one does not        the pointer readout only repaints when the
                                 pointer moves, so it must be STILL - which is
                                 the control that proves the others' motion is
                                 the apps ticking and not the screen shimmering

That last one matters. A test that only asks "did pixels change" passes on a
flickering screen; requiring one window to hold still while the others move is
what makes the result mean something.

    ./probe-apps.py                 the gate
    ./probe-apps.py --keep-shots    ...and leave the picture in shots/
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile
import time

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
HERE = KERNEL_ROOT
sys.path.insert(0, PROBE_DIR)

from exercise import Serial, Qmp, qemu_argv, build  # noqa: E402

SHOTS = os.path.join(HERE, "shots")

QCODE = {" ": "spc", "\n": "ret"}
for _c in "abcdefghijklmnopqrstuvwxyz":
    QCODE[_c] = _c


def qtype(qmp, text, settle=0.07):
    for ch in text:
        qmp.sendkey(QCODE[ch])
        time.sleep(settle)


def crop(path, box, step=2):
    with open(path, "rb") as fh:
        blob = fh.read()
    if not blob.startswith(b"P6"):
        return None
    fields, i = [], 2
    while len(fields) < 3:
        while i < len(blob) and blob[i:i + 1].isspace():
            i += 1
        if blob[i:i + 1] == b"#":
            while i < len(blob) and blob[i] != 0x0A:
                i += 1
            continue
        j = i
        while j < len(blob) and not blob[j:j + 1].isspace():
            j += 1
        fields.append(int(blob[i:j])); i = j
    i += 1
    w, h, _ = fields
    px = blob[i:]
    x0, y0, bw, bh = box
    x1, y1 = min(x0 + bw, w), min(y0 + bh, h)
    out = bytearray()
    for y in range(max(0, y0), max(0, y1), step):
        base = y * w * 3
        for x in range(max(0, x0), max(0, x1), step):
            p = base + x * 3
            out += px[p:p + 3]
    return bytes(out)


def differs(a, b):
    if a is None or b is None or len(a) != len(b) or not a:
        return None
    n = sum(1 for i in range(0, len(a), 3) if a[i:i + 3] != b[i:i + 3])
    return n / (len(a) / 3)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--keep-shots", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()

    if not args.no_build:
        build(False)

    tmp = tempfile.mkdtemp(prefix="probeapps-")
    ser_path = os.path.join(tmp, "ser.sock")
    qmp_path = os.path.join(tmp, "qmp.sock")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path, tablet=False),
                            cwd=HERE, stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    failures = []

    def check(label, ok, detail=""):
        print(f"  {'ok  ' if ok else 'FAIL'}  {label}{('   ' + detail) if detail else ''}")
        if not ok:
            failures.append(label)

    def shot(name, box):
        ppm = os.path.join(tmp, name + ".ppm")
        if not qmp.screendump(ppm):
            return None
        px = crop(ppm, box)
        if args.keep_shots:
            os.makedirs(SHOTS, exist_ok=True)
            subprocess.run(["convert", ppm, os.path.join(SHOTS, "apps-" + name + ".png")],
                           capture_output=True)
        return px

    # command typed -> the title the kernel gives that window, and whether it
    # is expected to animate on its own with no input at all.
    # SNAKE IS OPENED LAST, deliberately. It is the one app that can legitimately
    # STOP - it hits a wall and the game ends - and on its tile that takes about
    # 1.7 s. Opened first, it had already finished before the samples were taken
    # and this gate read 0.00% for an app that was working perfectly. Opening it
    # last means the samples straddle a game still in progress.
    APPS = [
        ("cube",  "3D",      True),
        ("anim",  "zlOS",    True),
        ("paint", "Paint",   False),
        ("mouse", "Pointer", False),
        ("snake", "Snake",   True),
    ]

    try:
        ser = Serial(ser_path)
        qmp = Qmp(qmp_path)
        ok, log = ser.wait("compositor:", args.boot_timeout)
        if not ok:
            print("never reached the compositor:\n" + log[-2000:]); return 1
        ser.drain(1.2)
        log += ser.buf; ser.buf = ""
        before_n = len(re.findall(r"wm: win (\d+) title", log))

        for cmd, _title, _anim in APPS:
            qtype(qmp, cmd + "\n")
            time.sleep(0.15)
        ser.drain(0.4)
        log += ser.buf; ser.buf = ""

        wins = re.findall(
            r"wm: win (\d+) title (\d+),(\d+) (\d+)x(\d+) client (\d+),(\d+) (\d+)x(\d+)", log)
        opened = wins[before_n:]
        check(f"all {len(APPS)} apps opened a window",
              len(opened) == len(APPS), f"{len(opened)} of {len(APPS)}")
        if len(opened) != len(APPS):
            print("\n--- serial tail ---\n" + log[-2500:])
            return 1

        rects = {}
        for (cmd, _t, _a), w in zip(APPS, opened):
            rects[cmd] = tuple(int(v) for v in w[5:9])
            print(f"  note  {cmd:6s} win {w[0]} client {rects[cmd][0]},{rects[cmd][1]} "
                  f"{rects[cmd][2]}x{rects[cmd][3]}")

        # Nothing is typed between these two. Anything that changes, changed
        # because its own app_tick asked for a repaint.
        # Sampled newest-first: whichever app opened last has had the least
        # time to run, so photograph it before the four that have been going
        # for seconds already.
        order = [c for c, _t, _a in reversed(APPS)]
        first = {c: shot("t0-" + c, rects[c]) for c in order}
        time.sleep(1.0)
        second = {c: shot("t1-" + c, rects[c]) for c in order}
        shot("all", (0, 0, 4096, 4096))

        for cmd, _title, animates in APPS:
            d = differs(first[cmd], second[cmd])
            if d is None:
                check(f"{cmd}: measurable", False, "no screendump")
                continue
            if animates:
                check(f"{cmd} keeps running with nothing typed",
                      d > 0.002, f"{d * 100:.2f}% of its client area changed")
            else:
                # The control. paint has no ticker and the pointer readout only
                # repaints when the pointer moves - and it has not. If these
                # move too, the "animation" above is the whole screen shimmering
                # and none of the assertions mean anything.
                check(f"{cmd} correctly holds still (the control)",
                      d < 0.002, f"{d * 100:.2f}% changed")

        print()
        if failures:
            print(f"apps gate FAILED: {len(failures)}")
            return 1
        print("apps gate green: five apps, five windows, all live at once")
        return 0
    finally:
        proc.kill()
        proc.wait()
        subprocess.run(["rm", "-rf", tmp])


if __name__ == "__main__":
    sys.exit(main())
