#!/usr/bin/env python3
"""Create, save, power off, then reopen a file through the graphical apps."""
import argparse, os, re, subprocess, sys, tempfile, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from exercise import Serial, Qmp, build, qemu_argv, qtype  # noqa: E402


def crop(path, box, step=2):
    blob = open(path, "rb").read(); fields, i = [], 2
    while len(fields) < 3:
        while blob[i:i + 1].isspace(): i += 1
        if blob[i:i + 1] == b"#":
            while blob[i] != 10: i += 1
            continue
        j = i
        while not blob[j:j + 1].isspace(): j += 1
        fields.append(int(blob[i:j])); i = j
    i += 1
    width, height, _ = fields; pixels = blob[i:]
    x0, y0, w, h = box; out = bytearray()
    for y in range(max(0, y0), min(height, y0 + h), step):
        for x in range(max(0, x0), min(width, x0 + w), step):
            p = (y * width + x) * 3; out += pixels[p:p + 3]
    return bytes(out)


def last_box(log):
    rows = re.findall(r"wm: win \d+ title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", log)
    return tuple(int(v) for v in rows[-1]) if rows else None


def shot(qmp, tmp, name, box):
    path = os.path.join(tmp, name + ".ppm")
    return crop(path, box) if qmp.screendump(path) else None


def difference(a, b):
    if not a or not b or len(a) != len(b): return 1.0
    changed = sum(a[i:i + 3] != b[i:i + 3] for i in range(0, len(a), 3))
    return changed / (len(a) // 3)


def launch(argv, serial_path, qmp_path):
    for path in (serial_path, qmp_path):
        try: os.unlink(path)
        except FileNotFoundError: pass
    proc = subprocess.Popen(argv, cwd=HERE, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return proc, Serial(serial_path), Qmp(qmp_path)


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--boot-timeout", type=float, default=240); args = ap.parse_args()
    if not args.no_build: build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-files-")
    sp, qp = os.path.join(tmp, "serial.sock"), os.path.join(tmp, "qmp.sock")
    argv = qemu_argv(tmp, False, sp, qp, tablet=False); failures = []
    first = second = None

    def check(label, ok, detail=""):
        print(f"  {'ok  ' if ok else 'FAIL'}  {label}{('   ' + detail) if detail else ''}")
        if not ok: failures.append(label)

    try:
        first, serial, qmp = launch(argv, sp, qp)
        ok, log = serial.wait("zl> ", args.boot_timeout); check("first machine reached the desktop", ok)
        if not ok: return 1
        base_windows = len(re.findall(r"wm: win \d+ title", log))
        qtype(qmp, ".\n"); ok, more = serial.wait("BOOTCOUNT=1", 90); log += more
        check("fresh NVMe volume formatted and mounted", ok)
        qtype(qmp, "files\n"); time.sleep(1); serial.drain(.5); log += serial.buf; serial.buf = ""
        file_windows = len(re.findall(r"wm: win \d+ title", log))
        check("Files opened as a window", file_windows > base_windows,
              f"{file_windows - base_windows} new window(s)")
        qtype(qmp, "nnotes.txt\n"); time.sleep(1); serial.drain(.5); log += serial.buf; serial.buf = ""
        box = last_box(log); check("creating notes.txt opened the disk editor", box is not None)
        if box is None: return 1
        qtype(qmp, "persistent text"); time.sleep(.8); before = shot(qmp, tmp, "before", box)
        qmp.sendkey("esc"); time.sleep(1); first.kill(); first.wait(); first = None

        second, serial2, qmp2 = launch(argv, sp, qp)
        ok, log2 = serial2.wait("zl> ", args.boot_timeout)
        check("second machine cold-booted from the same disk", ok)
        if not ok: return 1
        qtype(qmp2, "files\n"); time.sleep(1); qmp2.sendkey("down"); qmp2.sendkey("ret")
        time.sleep(1); serial2.drain(.5); log2 += serial2.buf
        box2 = last_box(log2); check("notes.txt opened after the power cycle", box2 is not None)
        if box2 is not None:
            after = shot(qmp2, tmp, "after", box2); delta = difference(before, after)
            check("saved text came back from disk", delta < .08,
                  f"editor pixels differ by {delta * 100:.2f}%")
        print()
        if failures: print(f"Files gate FAILED: {len(failures)}"); return 1
        print("Files gate green: create -> edit -> save -> power off -> reopen"); return 0
    finally:
        for proc in (first, second):
            if proc is not None and proc.poll() is None: proc.kill(); proc.wait()


if __name__ == "__main__": sys.exit(main())
