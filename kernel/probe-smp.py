#!/usr/bin/env python3
"""probe-smp.py - band rendering IN THE KERNEL draws the same pixels.

hosttest/fbbench.c measures band rendering with a pthread pool, which is a
faithful model of the dispatch shape but is NOT the kernel: it never touches
smp.c, the trampolines, the AP stacks, or a core that was woken by an INIT/SIPI
pair. Everything that can actually go wrong with three real cores writing one
buffer lives on the other side of that line.

So this boots the real thing, wakes the cores, and asserts the compositor draws
IDENTICAL PIXELS afterwards.

Method, and the awkward part is making the comparison fair:

  clear + help      the shell scrollback is now exactly the help text
  screendump        crop the scrollback area - SERIAL rendering
  smp               wake the cores; fb.c starts handing out bands
  clear + help      the same content, drawn again
  screendump        crop the same rectangle - BANDED rendering
  assert equal      byte for byte

The crop stops short of the prompt row on purpose: the caret blinks off
ticks(), so including it would compare a cursor phase rather than a renderer.

A band that overlapped its neighbour, or left a gap, or raced the barrier so
fb_present blitted a row still being written, all show up here as a difference.
Nothing else in the tree can see them.

    ./probe-smp.py
    ./probe-smp.py --keep-shots
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from exercise import Serial, Qmp, qemu_argv, build  # noqa: E402

SHOTS = os.path.join(HERE, "shots")

QCODE = {" ": "spc", "\n": "ret"}
for _c in "abcdefghijklmnopqrstuvwxyz":
    QCODE[_c] = _c


def qtype(qmp, text, settle=0.07):
    for ch in text:
        qmp.sendkey(QCODE[ch])
        time.sleep(settle)


def crop(path, box, step=1):
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
        out += px[base + max(0, x0) * 3: base + max(0, x1) * 3]
    return bytes(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--keep-shots", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()

    if not args.no_build:
        build(False)

    tmp = tempfile.mkdtemp(prefix="probesmp-")
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
            subprocess.run(["convert", ppm, os.path.join(SHOTS, "smp-" + name + ".png")],
                           capture_output=True)
        return px

    try:
        ser = Serial(ser_path)
        qmp = Qmp(qmp_path)
        ok, log = ser.wait("compositor:", args.boot_timeout)
        if not ok:
            print("never reached the compositor:\n" + log[-2000:]); return 1
        ser.drain(1.2)
        log += ser.buf; ser.buf = ""

        m = re.search(r"wm: win 0 title \d+,\d+ \d+x(\d+) client (\d+),(\d+) (\d+)x(\d+)", log)
        if not m:
            print("no shell rect reported:\n" + log[-1500:]); return 1
        th, sx, sy, sw, sh = (int(g) for g in m.groups())
        # Stop short of the prompt row: the caret blinks off ticks(), and this
        # is a test of the renderer, not of a cursor phase.
        box = (sx, sy, sw, sh - th)

        qtype(qmp, "clear\nhelp\n")
        time.sleep(1.2)
        serial_px = shot("serial", box)

        qtype(qmp, "smp\n")
        okc, more = ser.wait("cores online now", 90)
        log += more
        ser.drain(1.0)
        log += ser.buf; ser.buf = ""
        line = ""
        for ln in log.splitlines():
            if "cores online now" in ln:
                line = ln.strip()
        # `"of" in line` passes for "cores online now: 1 of 4" - i.e. for a
        # total failure to wake anything. The number is what matters, so parse
        # it first and assert on THAT.
        mm = re.search(r"cores online now:\s*(\d+)\s*of\s*(\d+)", line)
        got = int(mm.group(1)) if mm else 0
        want = int(mm.group(2)) if mm else 0
        check("the other cores woke up", okc and mm is not None and got >= 2,
              f"{line}  ({got} of {want})")

        # Band rendering is only enabled once the cores are actually in the
        # spin loop, so a run on a single-core QEMU proves nothing about bands
        # - say so rather than passing quietly.
        cores = got
        if cores < 2:
            print(f"  note  only {cores} core(s) - band rendering stays serial, "
                  f"nothing to compare")
            return 1 if failures else 0

        qtype(qmp, "clear\nhelp\n")
        time.sleep(1.2)
        banded_px = shot("banded", box)

        if serial_px is None or banded_px is None:
            check("banded rendering is pixel-identical", False, "no screendump")
        elif len(serial_px) != len(banded_px):
            check("banded rendering is pixel-identical", False, "different sizes")
        else:
            diff = sum(1 for i in range(0, len(serial_px), 3)
                       if serial_px[i:i + 3] != banded_px[i:i + 3])
            total = len(serial_px) // 3
            check("banded rendering is pixel-identical",
                  diff == 0, f"{diff} of {total} pixels differ across {cores} cores")

        print()
        if failures:
            print(f"smp gate FAILED: {len(failures)}")
            return 1
        print(f"smp gate green: {cores} cores, same pixels")
        return 0
    finally:
        proc.kill()
        proc.wait()
        subprocess.run(["rm", "-rf", tmp])


if __name__ == "__main__":
    sys.exit(main())
