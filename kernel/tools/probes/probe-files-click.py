#!/usr/bin/env python3
"""probe-files-click.py - clicking a row in 02 FILES selects THAT row.

WHY THIS EXISTS, AND IT IS AN APOLOGY AS MUCH AS A TEST.

A commit this week claimed "the Files list ignored nineteen clicks in twenty",
attributed it to float division in the row index, and converted the line to
idiv. The attribution was wrong - `/` is integer division in the compiled kernel
(see docs/design/zl-division-semantics.md) - and the failure itself was never
observed. It was inferred from an operator and written up as a measurement.

That left a question genuinely open: IS the list clickable? The report was
closed against a mechanism that was not it, so nothing had actually been
established either way. This establishes it.

WHAT IS ASSERTED, AND WHY IT IS THE DETAIL BLOCK.

"The row highlighted" is a weak assertion - a hover, a repaint or a scroll can
all move pixels in a list. The detail block under the table names the SELECTED
file: its name, its size, its first block and its run length. If clicking row B
after row A leaves that block unchanged, the click did not select. If it
changes, it did.

The control is a click on the same row twice: the block must NOT change. Without
it the assertion would also be satisfied by a pane that redrew its detail block
on any click at all.

VERIFIED IN BOTH DIRECTIONS. Measured, screen 1920x1200, Files client 534x343:

    as shipped                       same row 0.0000   other row 0.5855   PASS
    files_mouse_slot forced to -1    same row 0.0000   other row 0.0000   FAIL

THE ANSWER IS THAT THE LIST WAS NEVER BROKEN. Clicking a row selects it, and
selects THAT row. The commit that said otherwise was wrong about the mechanism
and wrong about the effect - it described a failure nobody had seen, in a
sentence shaped like a measurement.

Re-plant before trusting a green run: one line, `return 0 - 1` at the top of
files_mouse_slot.
"""
import argparse
import os
import re
import subprocess
import sys
import tempfile
import time

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, PROBE_DIR)
from exercise import Serial, Qmp, qemu_argv, build  # noqa: E402


def moveto(qmp, x, y, w, h):
    qmp.cmd("input-send-event", events=[
        {"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / (w - 1))}},
        {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / (h - 1))}}])


def click(qmp, x, y, w, h):
    moveto(qmp, x, y, w, h)
    time.sleep(0.25)
    qmp.cmd("input-send-event", events=[{"type": "btn", "data": {"down": True, "button": "left"}}])
    time.sleep(0.15)
    qmp.cmd("input-send-event", events=[{"type": "btn", "data": {"down": False, "button": "left"}}])
    time.sleep(0.8)


def read_ppm(path):
    try:
        blob = open(path, "rb").read()
    except OSError:
        return None
    if not blob.startswith(b"P6"):
        return None
    fields, i = [], 2
    while len(fields) < 3:
        while i < len(blob) and blob[i:i + 1].isspace():
            i += 1
        j = i
        while j < len(blob) and not blob[j:j + 1].isspace():
            j += 1
        fields.append(int(blob[i:j]))
        i = j
    return fields[0], fields[1], blob[i + 1:]


def band(path, box):
    got = read_ppm(path)
    if got is None:
        return None
    w, h, px = got
    x, y, bw, bh = box
    if x < 0 or y < 0 or x + bw > w or y + bh > h:
        return None
    out = bytearray()
    for row in range(y, y + bh):
        off = (row * w + x) * 3
        out += px[off:off + bw * 3]
    return bytes(out)


def ink_moved(a, b):
    """Fraction of the INKED positions that differ - see probe-edit-viewport
    for why the denominator is the ink and not the area."""
    if a is None or b is None or len(a) != len(b) or not a:
        return None
    pa = [a[i:i + 3] for i in range(0, len(a), 3)]
    pb = [b[i:i + 3] for i in range(0, len(b), 3)]
    counts = {}
    for p in pa:
        counts[p] = counts.get(p, 0) + 1
    bg = max(counts, key=counts.get)
    inked = [i for i in range(len(pa)) if pa[i] != bg or pb[i] != bg]
    if not inked:
        return None
    return sum(1 for i in inked if pa[i] != pb[i]) / len(inked)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    # Measured, not picked: selecting a different file moves more than half the
    # detail block's ink, and re-selecting the same one moves none of it.
    ap.add_argument("--min-change", type=float, default=0.20)
    args = ap.parse_args()
    build(False)
    fails = []

    def check(label, ok, detail=""):
        print(("  ok    " if ok else "  FAIL  ") + label + (f"   [{detail}]" if detail else ""))
        if not ok:
            fails.append(label)

    tmp = tempfile.mkdtemp(prefix="fclick-")
    sp, qp = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, sp, qp),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(sp), Qmp(qp)
        ok, log = ser.wait("compositor:", args.boot_timeout)
        if not ok:
            print("no compositor:\n" + log[-1500:])
            return 1
        ser.drain(1.5)
        log += ser.buf
        ser.buf = ""

        probe = os.path.join(tmp, "size.ppm")
        if not qmp.screendump(probe):
            print("no screendump")
            return 1
        hdr = open(probe, "rb").read(64).split()
        W, H = int(hdr[1]), int(hdr[2])

        # Two files, so there are two rows to tell apart. `touch` is not a
        # command here; the editor's save is what creates one, and `;` stamps
        # the clock so the mtime column has something in it.
        ser.send("format\r")
        time.sleep(1.6)
        ser.send("edit\r")
        time.sleep(1.3)
        ser.send("\x1b")
        time.sleep(1.3)
        ser.send("clip\r")
        time.sleep(1.3)
        ser.drain(1.0)
        log += ser.buf
        ser.buf = ""

        wins = re.findall(
            r"wm: win (\d+) title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", log)
        # FILES IS THE 534-WIDE CLIENT. The boot composition opens it at
        # FILES_W 540 design units, which is 534 of client after the chrome; the
        # shell and the monitor are 634 and the editor is 516, so that band
        # picks exactly one. Matching on a size range rather than on the window
        # INDEX because the index depends on open order and the size does not.
        files_win = None
        for w in wins:
            cw = int(w[3])
            if 525 < cw < 545:
                files_win = w
        if files_win is None:
            print("could not find the Files window among:", wins[-4:])
            return 1
        cx, cy, cw, ch = (int(v) for v in files_win[1:5])
        print(f"  note  Files client {cx},{cy} {cw}x{ch}")

        # The detail block: the bottom 116 design units of the client, which is
        # FILES_DETAIL_H. Its content is the selected file's name, size, lba and
        # run - so it changes if and only if the selection does.
        det = (cx + 8, cy + ch - 112, cw - 16, 70)

        def shot(name):
            p = os.path.join(tmp, name + ".ppm")
            if not qmp.screendump(p):
                return None
            return band(p, det)

        # The list starts 62dp + the column head below the client top; rows are
        # FILES_ROW (20) apart. Row 0's middle is about +92, row 1's about +112.
        click(qmp, cx + 120, cy + 92, W, H)
        a = shot("rowA")
        check("the detail block was readable after the first click", a is not None)

        # CONTROL: the same row again must change nothing.
        click(qmp, cx + 120, cy + 92, W, H)
        again = shot("rowA2")
        d_same = ink_moved(a, again)
        check("re-selecting the same row changes nothing",
              d_same is not None and d_same < args.min_change,
              f"{d_same:.4f}" if d_same is not None else "unreadable")

        # ...and a different row must change it.
        click(qmp, cx + 120, cy + 112, W, H)
        b = shot("rowB")
        d_diff = ink_moved(a, b)
        check("clicking a different row selects it",
              d_diff is not None and d_diff >= args.min_change,
              f"{d_diff:.4f}" if d_diff is not None else "unreadable")

        print()
        if d_same is not None and d_diff is not None:
            print(f"detail block: {d_same:.4f} on the same row, {d_diff:.4f} on another")
        print(("FAIL: " + ", ".join(fails)) if fails else "PASS")
        return 1 if fails else 0
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()


if __name__ == "__main__":
    sys.exit(main())
