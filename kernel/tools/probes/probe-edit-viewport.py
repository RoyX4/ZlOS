#!/usr/bin/env python3
"""probe-edit-viewport.py - the editor follows the caret past the bottom of the well.

THE DEFECT THIS EXISTS FOR. ed_text_body drew from byte 0 every frame and
stopped when it ran out of well. There was no scroll offset in the editor's
state at all. So past about fifteen rows the pane went blind: keystrokes kept
appending to EDIT_BUF, nothing drew them, and the caret sat frozen off the
bottom edge. You could type into a file you could not see.

WHY THE ASSERTION IS "THE TOP OF THE WELL CHANGED" AND NOT "INK APPEARED".

Ink appears either way - the blind editor still draws its first fifteen rows,
and a probe that only counts ink passes against the bug. What the bug cannot do
is MOVE. Under the old code line 1 is drawn at the top of the well on every
frame forever, so the top band is byte-identical between four lines typed and
twenty-eight lines typed. Under a working viewport the view is tail-anchored,
so by twenty-eight lines the top band is showing something else entirely.

That is the whole test: same band, two moments, and the buggy build physically
cannot differ between them.

THE BAND IS SAMPLED BELOW THE HEAD AND ABOVE THE FOOT so that neither the file
name, the "modified" state nor the LN/COL row can carry the assertion. Those
change on their own for reasons that have nothing to do with scrolling, and a
band that included them would pass on a build with no viewport whatsoever.

VERIFIED IN BOTH DIRECTIONS, which is the only reason to believe a green run.
Measured on this branch, same window (client 516x309), two builds:

    fixed                        control 0.3397   test 0.8236   PASS
    ed_top() forced to return 0  control 0.3397   test 0.3397   FAIL

The planted number is not merely low, it is EXACTLY the control - with the view
pinned to line 1 the band stops changing the moment the well is full, so the
late picture and the short picture are the same picture. That equality is the
signature of a stuck view, and it is why this probe can tell the two builds
apart when the earlier area-based version could not.

Re-plant it before trusting a green run again: the fix is one line to add and
one to remove, and a probe nobody has seen fail is not evidence.
"""
import argparse, os, re, subprocess, sys, tempfile, time

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
sys.path.insert(0, PROBE_DIR)
from exercise import Serial, Qmp, qemu_argv, build  # noqa: E402

SHOTS = os.path.join(KERNEL_ROOT, "shots")
QCODE = {" ": "spc", "\n": "ret", "\x1b": "esc"}
for _c in "abcdefghijklmnopqrstuvwxyz":
    QCODE[_c] = _c


def qtype(qmp, text, settle=0.06):
    for ch in text:
        qmp.sendkey(QCODE[ch])
        time.sleep(settle)


def read_ppm(path):
    """(w, h, pixels) from a binary P6. Returns None if the dump never landed."""
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
        if blob[i:i + 1] == b"#":
            while i < len(blob) and blob[i:i + 1] != b"\n":
                i += 1
            continue
        j = i
        while j < len(blob) and not blob[j:j + 1].isspace():
            j += 1
        fields.append(int(blob[i:j]))
        i = j
    return fields[0], fields[1], blob[i + 1:]


def band(path, box):
    """The pixels of one horizontal strip, as a flat bytes object."""
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


def differs(a, b):
    """How much of the INK moved, 0.0 to 1.0.

    NOT the fraction of the band's area that changed - that was the first
    version and it could not tell a working viewport from a broken one. Text is
    sparse: three characters on a line, in a band five hundred pixels wide, is
    under three per cent of the pixels even when EVERY line has been replaced.
    The buggy build scored 0.0101 and the fixed build 0.0291, both far below
    any threshold worth writing down, and the honest reading of that pair is
    "this instrument cannot see the thing it was pointed at".

    So the denominator is the ink, not the area: of all the positions that
    carry ink in either picture, how many differ. A view that scrolled replaces
    essentially all of them and scores near 1.0; a view that cannot move scores
    near 0.0. The measurement is now the same size as the effect.

    The background is taken as the most common colour in the band rather than
    read from the theme, so a re-pointed palette cannot silently turn every
    pixel into ink and make this pass on anything at all.
    """
    if a is None or b is None or len(a) != len(b) or not a:
        return None
    px_a = [a[i:i + 3] for i in range(0, len(a), 3)]
    px_b = [b[i:i + 3] for i in range(0, len(b), 3)]
    counts = {}
    for p in px_a:
        counts[p] = counts.get(p, 0) + 1
    bg = max(counts, key=counts.get)
    inked = [i for i in range(len(px_a)) if px_a[i] != bg or px_b[i] != bg]
    if not inked:
        return None
    changed = sum(1 for i in inked if px_a[i] != px_b[i])
    return changed / len(inked)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--keep-shots", action="store_true")
    # Half the ink. A scrolled view replaces nearly all of it and a stuck view
    # replaces almost none, so anything in the middle of that range separates
    # them; the exact number is not load-bearing and is not tuned. Both ends
    # are measured and printed on every run, so a future change that narrows
    # the gap shows up as a narrowed gap rather than as a threshold nudge.
    ap.add_argument("--min-change", type=float, default=0.50)
    args = ap.parse_args()

    build(False)
    fails = []

    def check(label, ok, detail=""):
        print(("  ok    " if ok else "  FAIL  ") + label + (f"   [{detail}]" if detail else ""))
        if not ok:
            fails.append(label)

    tmp = tempfile.mkdtemp(prefix="editview-")
    sp = os.path.join(tmp, "ser.sock")
    qp = os.path.join(tmp, "qmp.sock")
    proc = subprocess.Popen(qemu_argv(tmp, False, sp, qp), stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(sp), Qmp(qp)
        ok, log = ser.wait("compositor:", args.boot_timeout)
        if not ok:
            print("no compositor:\n" + log[-1500:])
            return 1
        ser.drain(1.2)
        log += ser.buf
        ser.buf = ""
        base_n = len(re.findall(r"wm: win \d+ title", log))

        # THE VOLUME HAS TO EXIST BEFORE THE EDITOR CAN OPEN A FILE ON IT.
        # editor_open_default goes through files_ensure_mounted, and a fresh
        # rd0 is blank, so without this `edit` prints "zlfs: no filesystem
        # here - format it first" and no window opens at all. The first run of
        # this probe asserted against exactly that: a shell that had refused.
        qtype(qmp, "format\n")
        time.sleep(1.2)
        ser.drain(1.0)
        log += ser.buf
        ser.buf = ""

        qtype(qmp, "edit\n")
        time.sleep(0.9)
        ser.drain(0.9)
        log += ser.buf
        ser.buf = ""
        wins = re.findall(
            r"wm: win (\d+) title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", log)
        check("the editor opened a window", len(wins) > base_n, f"{len(wins) - base_n} new")
        if len(wins) <= base_n:
            print("\n--- serial ---\n" + log[-1500:])
            return 1
        cx, cy, cw, ch = (int(v) for v in wins[-1][1:5])
        print(f"  note  editor client {cx},{cy} {cw}x{ch}")

        # The band: a strip across the top of the WELL. Starting a quarter of
        # the way down clears the head (buffer name, state, encoding); ending
        # before 45% keeps it far above the LN/COL foot. Nothing in this strip
        # moves for any reason except the text scrolling.
        by = cy + ch // 4
        bh = max(4, ch // 8)
        bx = cx + 8
        bw = cw - 16
        box = (bx, by, bw, bh)

        def shot(name):
            p = os.path.join(tmp, name + ".ppm")
            if not qmp.screendump(p):
                return None
            if args.keep_shots:
                os.makedirs(SHOTS, exist_ok=True)
                subprocess.run(["convert", p, os.path.join(SHOTS, "editview-" + name + ".png")],
                               capture_output=True)
            return band(p, box)

        # Four short lines: comfortably inside one screenful either way.
        qtype(qmp, "".join(c * 12 + "\n" for c in "abcd"), settle=0.03)
        time.sleep(0.7)
        early = shot("early")
        check("the band was readable after four lines", early is not None)

        # A control: typing MORE text that still fits must not move the top.
        # Without this the assertion below could be satisfied by any repaint at
        # all, and the probe would pass on a build that simply flickers.
        qtype(qmp, "".join(c * 12 + "\n" for c in "ef"), settle=0.03)
        time.sleep(0.7)
        still_short = shot("short")
        d_short = differs(early, still_short)
        check("the top does not move while the text still fits",
              d_short is not None and d_short < args.min_change,
              f"{d_short:.4f}" if d_short is not None else "unreadable")

        # Now past the well's capacity. Twenty-two more lines takes any window
        # this OS opens well beyond the fifteen rows the old code could show.
        qtype(qmp, "".join(c * 12 + "\n" for c in "ghijklmnopqrstuvwxyzab"), settle=0.03)
        time.sleep(1.0)
        late = shot("late")
        d_late = differs(early, late)
        check("the view followed the caret past the bottom of the well",
              d_late is not None and d_late >= args.min_change,
              f"{d_late:.4f}" if d_late is not None else "unreadable")

        print()
        if d_short is not None and d_late is not None:
            print(f"top band: {d_short:.4f} changed while it fit, "
                  f"{d_late:.4f} changed once it did not")
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
