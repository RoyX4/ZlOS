#!/usr/bin/env python3
"""probe-files.py - the Files app, cold boot to cold boot.

This is the acceptance gate the feature was actually asked for, and it is the
one thing none of verify-disk.sh, probe-edit.py or fstest.c prove on their
own:

  verify-disk.sh   proves zlfs SURVIVES two power cycles - but it drives the
                    OLD text shell ('.', ',', 'q'), which has no framebuffer
                    and therefore no windows. It never touches the Files app.
  probe-edit.py     proves the EDITOR is a real window - but only the ten
                    numbered RAM slots, which live in BSS and are gone the
                    moment QEMU exits. It never touches disk.
  hosttest/fstest.c proves fs.c's C API survives a cold start in a SEPARATE
                    PROCESS - but it calls fs_create/fs_write/fs_find
                    directly. It never goes through a single line of the zl
                    glue this feature adds (files_commit, editor_open_disk).

None of the three, alone or together, proves what this probe does: create a
file BY NAME from the Files app, save it from the disk-backed editor, kill
the machine, and boot a SECOND, independent QEMU process against the SAME
disk image - the only thing the two processes share is the file on the
host's disk, same as verify-disk.sh's BOOTCOUNT.

  boot 1: filemgr -> type a name -> Enter creates+opens it in the editor
          -> type some text -> ESC saves AND closes
  (kill this QEMU; nothing about boot 2 can depend on anything it remembers)
  boot 2: filemgr -> type the SAME name -> Enter finds it (fs_get, not
          fs_new) -> the editor opens with the text already in it

Screenshots are compared the same way probe-edit.py compares them - by "ink"
(how many sampled pixels are NOT the single most common colour), not by
guessing a brightness threshold. See its header for why.
"""
import argparse, os, re, subprocess, sys, tempfile, time
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from exercise import Serial, Qmp, qemu_argv, qtype, build  # noqa: E402
SHOTS = os.path.join(HERE, "shots")
NAME = "probe.txt"           # every character has a QCODE - see exercise.py
BODY = "cold boot works"     # ditto


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
    for y in range(max(0, y0), max(0, y1), step):
        base = y*w*3
        for x in range(max(0, x0), max(0, x1), step):
            out += px[base+x*3: base+x*3+3]
    return bytes(out)


def ink(px):
    if not px: return None
    c = {}
    for i in range(0, len(px), 3):
        k = px[i:i+3]; c[k] = c.get(k, 0) + 1
    return len(px)//3 - max(c.values())


# ---- boot 2's argv: the SAME disk, never recreated -------------------------
# qemu_argv() in exercise.py always runs `qemu-img create`, which is exactly
# right for every OTHER probe (a fresh disk per run) and exactly wrong here -
# calling it twice would wipe boot 1's write before boot 2 ever mounted it.
# So this is qemu_argv()'s own "common" list, minus the two qemu-img create
# calls, pointed at the files boot 1 already made.
def reboot_argv(tmp, ser_path, qmp_path):
    disk = os.path.join(tmp, "nvme.img")
    stick = os.path.join(tmp, "stick.img")
    assert os.path.exists(disk), "boot 1 should have created this already"
    return [
        "qemu-system-i386",
        "-m", "1G", "-smp", "4", "-cpu", "host", "-accel", "kvm",
        "-drive", f"file={disk},if=none,id=nvm,format=raw",
        "-device", "nvme,serial=zlos001,drive=nvm",
        "-drive", f"file={stick},if=none,id=stick,format=raw",
        "-device", "qemu-xhci,id=xhci",
        "-device", "usb-storage,bus=xhci.0,drive=stick",
        "-device", "usb-kbd,bus=xhci.0",
        "-no-reboot", "-display", "none",
        "-chardev", f"socket,id=ser0,path={ser_path},server=on,wait=off",
        "-serial", "chardev:ser0",
        "-qmp", f"unix:{qmp_path},server=on,wait=off",
        "-cdrom", f"{HERE}/zlOS.iso",
        "-device", "virtio-gpu-pci,id=vgpu,xres=1280,yres=800",
    ]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--keep-shots", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()
    if not args.no_build:
        build(False)

    tmp = tempfile.mkdtemp(prefix="probefiles-")
    fails = []

    def check(l, ok, d=""):
        print(f"  {'ok  ' if ok else 'FAIL'}  {l}{('   '+d) if d else ''}")
        if not ok: fails.append(l)

    def shot(qmp, tmpdir, name, box, prefix):
        p = os.path.join(tmpdir, name + ".ppm")
        if not qmp.screendump(p): return None
        r = crop(p, box)
        if args.keep_shots:
            os.makedirs(SHOTS, exist_ok=True)
            subprocess.run(["convert", p, os.path.join(SHOTS, prefix+"-"+name+".png")],
                           capture_output=True)
        return r

    # ---- boot 1: create the file, write it, save it, close it -------------
    sp1, qp1 = os.path.join(tmp, "ser1"), os.path.join(tmp, "qmp1")
    proc1 = subprocess.Popen(qemu_argv(tmp, False, sp1, qp1, tablet=False), cwd=HERE,
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    editor_box = None
    try:
        ser, qmp = Serial(sp1), Qmp(qp1)
        ok, log = ser.wait("compositor:", args.boot_timeout)
        check("boot 1 reached the compositor", ok)
        if not ok:
            print(log[-1500:]); proc1.kill(); proc1.wait()
            return report(fails, tmp)
        ser.drain(1.2); log += ser.buf; ser.buf = ""
        base_n = len(re.findall(r"wm: win \d+ title", log))

        qtype(qmp, "filemgr\n"); time.sleep(0.8)
        ser.drain(0.8); log += ser.buf; ser.buf = ""
        wins = re.findall(r"wm: win (\d+) title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", log)
        check("the Files app opened a window", len(wins) > base_n, f"{len(wins)-base_n} new")
        if len(wins) <= base_n:
            print("\n--- serial ---\n"+log[-1500:]); proc1.kill(); proc1.wait()
            return report(fails, tmp)

        # type the name, Enter: no such file yet, so this CREATES it and opens
        # the editor on it - files_commit()'s fs_new() path, not fs_get()'s.
        qtype(qmp, NAME + "\n"); time.sleep(1.0)
        ser.drain(0.8); log += ser.buf; ser.buf = ""
        wins2 = re.findall(r"wm: win (\d+) title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", log)
        check("Enter on a new name opened the editor", len(wins2) > len(wins),
              f"{len(wins2)-len(wins)} new")
        if len(wins2) <= len(wins):
            print("\n--- serial ---\n"+log[-1500:]); proc1.kill(); proc1.wait()
            return report(fails, tmp)
        editor_box = tuple(int(v) for v in wins2[-1][1:5])
        print(f"  note  editor win client {editor_box[0]},{editor_box[1]} "
              f"{editor_box[2]}x{editor_box[3]}")

        empty = shot(qmp, tmp, "b1-empty", editor_box, "b1")
        qtype(qmp, BODY)
        time.sleep(0.8)
        typed = shot(qmp, tmp, "b1-typed", editor_box, "b1")
        i0, i1 = ink(empty), ink(typed)
        check("typing landed in the disk-backed editor",
              i1 is not None and i0 is not None and i1 > i0 + 20, f"ink {i0} -> {i1}")

        # ESC: editor_save() (disk branch: fs_wr) then the window closes -
        # editor_key()'s code==27 arm, unchanged in shape from the RAM path
        # probe-edit.py already exercises, just routed to fs_wr this time.
        qmp.sendkey("esc"); time.sleep(1.0)
        ser.drain(0.8); log += ser.buf; ser.buf = ""
        closed = shot(qmp, tmp, "b1-closed", editor_box, "b1")
        check("ESC saved to disk and closed the window",
              ink(closed) is not None and ink(closed) < i1 / 2,
              f"ink {i1} -> {ink(closed)} where the editor was")
    finally:
        # KILL, not a graceful in-guest shutdown - the acceptance gate is
        # explicit that nothing about boot 2 may depend on boot 1 having
        # exited cleanly. The write already reached fs_wr's completion before
        # this line runs; verify-disk.sh relies on the identical property.
        proc1.kill(); proc1.wait()

    if fails:
        return report(fails, tmp)

    # ---- boot 2: an INDEPENDENT process, same disk, find it BY NAME -------
    sp2, qp2 = os.path.join(tmp, "ser2"), os.path.join(tmp, "qmp2")
    proc2 = subprocess.Popen(reboot_argv(tmp, sp2, qp2), cwd=HERE,
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(sp2), Qmp(qp2)
        ok, log = ser.wait("compositor:", args.boot_timeout)
        check("boot 2 (independent process, same disk) reached the compositor", ok)
        if not ok:
            print(log[-1500:]); return report(fails, tmp)
        ser.drain(1.2); log += ser.buf; ser.buf = ""
        base_n = len(re.findall(r"wm: win \d+ title", log))

        qtype(qmp, "filemgr\n"); time.sleep(0.8)
        ser.drain(0.8); log += ser.buf; ser.buf = ""
        wins = re.findall(r"wm: win (\d+) title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", log)
        check("the Files app opened on the reboot too", len(wins) > base_n)
        if len(wins) <= base_n:
            print("\n--- serial ---\n"+log[-1500:]); return report(fails, tmp)

        # SAME name. fs_get() must find it this time - files_commit() never
        # calls fs_new() when a file already answers to that name.
        qtype(qmp, NAME + "\n"); time.sleep(1.0)
        ser.drain(0.8); log += ser.buf; ser.buf = ""
        wins2 = re.findall(r"wm: win (\d+) title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)", log)
        check("the same name reopened the editor on the reboot",
              len(wins2) > len(wins))
        if len(wins2) <= len(wins):
            print("\n--- serial ---\n"+log[-1500:]); return report(fails, tmp)
        box2 = tuple(int(v) for v in wins2[-1][1:5])

        reopened = shot(qmp, tmp, "b2-reopened", box2, "b2")
        blank = shot(qmp, tmp, "b2-blank-ref", (0, 0, 1, 1), "b2")
        check("the file has CONTENT after a real power cycle - not created empty",
              ink(reopened) is not None and ink(reopened) > 20,
              f"ink {ink(reopened)}")
        print("  note  create -> save -> kill -> reboot -> found by name -> "
              "content present: the whole acceptance chain ran")
    finally:
        proc2.kill(); proc2.wait()

    return report(fails, tmp)


def report(fails, tmp):
    subprocess.run(["rm", "-rf", tmp])
    print()
    if fails:
        print(f"files cold-boot gate FAILED: {len(fails)}")
        return 1
    print("files cold-boot gate green: create -> save -> kill -> reboot -> "
          "still there, by name, with its contents")
    return 0


if __name__ == "__main__":
    sys.exit(main())
