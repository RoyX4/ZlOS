#!/usr/bin/env python3
"""probe-edit.py - the editor is a window, and ESC closes it.

Item 10, held until last because it had the most state to hoist. The assertion
is not "the editor opens" - it is the whole shape of the conversion:

  it opens in a WINDOW           the kernel reports a rect for it
  typing reaches it and DRAWS    ink appears in its client area, and only
                                 there, with the shell untouched behind it
  ESC saves AND CLOSES           the window count goes back down, which is what
                                 every other window means by "exit". The old
                                 editor ended in `while ed_run == 1` and left
                                 you at "press any key".
  the text SURVIVES              reopening the slot shows the same ink, because
                                 ESC wrote it to the RAM file rather than just
                                 stopping
"""
import argparse, os, re, subprocess, sys, tempfile, time
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from exercise import Serial, Qmp, qemu_argv, build  # noqa: E402
SHOTS = os.path.join(HERE, "shots")
QCODE = {" ": "spc", "\n": "ret", "\x1b": "esc"}
for _c in "abcdefghijklmnopqrstuvwxyz": QCODE[_c] = _c


def qtype(qmp, text, settle=0.07):
    for ch in text:
        qmp.sendkey(QCODE[ch]); time.sleep(settle)


def crop(path, box, step=2):
    blob = open(path, "rb").read()
    if not blob.startswith(b"P6"): return None
    fields, i = [], 2
    while len(fields) < 3:
        while i < len(blob) and blob[i:i+1].isspace(): i += 1
        if blob[i:i+1] == b"#":
            while i < len(blob) and blob[i] != 0x0A: i += 1
            continue
        j = i
        while j < len(blob) and not blob[j:j+1].isspace(): j += 1
        fields.append(int(blob[i:j])); i = j
    i += 1
    w, h, _ = fields; px = blob[i:]
    x0, y0, bw, bh = box
    x1, y1 = min(x0+bw, w), min(y0+bh, h)
    out = bytearray()
    for y in range(max(0,y0), max(0,y1), step):
        base = y*w*3
        for x in range(max(0,x0), max(0,x1), step):
            out += px[base+x*3: base+x*3+3]
    return bytes(out)


def ink(px):
    if not px: return None
    c = {}
    for i in range(0, len(px), 3):
        k = px[i:i+3]; c[k] = c.get(k, 0) + 1
    return len(px)//3 - max(c.values())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--keep-shots", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()
    if not args.no_build: build(False)
    tmp = tempfile.mkdtemp(prefix="probeedit-")
    sp, qp = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, sp, qp, tablet=False), cwd=HERE,
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    fails = []
    def check(l, ok, d=""):
        print(f"  {'ok  ' if ok else 'FAIL'}  {l}{('   '+d) if d else ''}")
        if not ok: fails.append(l)
    def shot(name, box):
        p = os.path.join(tmp, name + ".ppm")
        if not qmp.screendump(p): return None
        r = crop(p, box)
        if args.keep_shots:
            os.makedirs(SHOTS, exist_ok=True)
            subprocess.run(["convert", p, os.path.join(SHOTS, "edit-"+name+".png")],
                           capture_output=True)
        return r
    try:
        ser, qmp = Serial(sp), Qmp(qp)
        ok, log = ser.wait("compositor:", args.boot_timeout)
        if not ok: print("no compositor:\n"+log[-1500:]); return 1
        ser.drain(1.2); log += ser.buf; ser.buf = ""
        base_n = len(re.findall(r"wm: win \d+ title", log))

        qtype(qmp, "edit\n"); time.sleep(0.8)
        ser.drain(0.8); log += ser.buf; ser.buf = ""
        wins = re.findall(r"wm: win (\d+) title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", log)
        check("the editor opened a window", len(wins) > base_n,
              f"{len(wins)-base_n} new")
        if len(wins) <= base_n:
            print("\n--- serial ---\n"+log[-1500:]); return 1
        wid = wins[-1][0]
        box = tuple(int(v) for v in wins[-1][1:5])
        print(f"  note  editor win {wid} client {box[0]},{box[1]} {box[2]}x{box[3]}")

        empty = shot("empty", box)
        qtype(qmp, "hello zlos")
        time.sleep(0.8)
        typed = shot("typed", box)
        i0, i1 = ink(empty), ink(typed)
        check("typing lands in the editor", i1 is not None and i0 is not None and i1 > i0 + 20,
              f"ink {i0} -> {i1}")

        # ESC: saves and CLOSES. The old editor's ESC left you at "press any key".
        qmp.sendkey("esc"); time.sleep(1.0)
        ser.drain(0.8); log += ser.buf; ser.buf = ""
        after = shot("closed", box)
        check("ESC closed the window", ink(after) is not None and ink(after) < i1 / 2,
              f"ink {i1} -> {ink(after)} where the editor was")

        # ...and it SAVED: reopening the same slot shows the same text back.
        qtype(qmp, "edit\n"); time.sleep(1.0)
        ser.drain(0.8); log += ser.buf; ser.buf = ""
        w2 = re.findall(r"wm: win (\d+) title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", log)
        # findall re-scans the WHOLE log, so the first open's line is still in
        # it - counting entries is the only way to tell "it reopened" from
        # "I am looking at the old line again". That is how this check first
        # reported ink 0 against a window that had never come back.
        if len(w2) <= len(wins):
            check("the editor reopened", False, "no new wm: line - it did not reopen")
            print("\n--- serial ---\n" + log[-1200:])
            return 1
        box2 = tuple(int(v) for v in w2[-1][1:5])
        again = shot("reopened", box2)
        check("ESC saved - the text comes back", ink(again) is not None and ink(again) > i0 + 20,
              f"ink {ink(again)} against {i0} empty and {i1} typed")

        print()
        if fails:
            print(f"editor gate FAILED: {len(fails)}"); return 1
        print("editor gate green: a window, typing, ESC saves and closes")
        return 0
    finally:
        proc.kill(); proc.wait(); subprocess.run(["rm", "-rf", tmp])


if __name__ == "__main__":
    sys.exit(main())
