#!/usr/bin/env python3
"""Cold-boot acceptance gate for Files and zlEDIT.

Boot 1 formats a blank NVMe volume from Files, creates and deletes one named
file, creates notes.txt, exercises editor copy/paste and Ctrl+S, then kills
QEMU. Boot 2 starts from the same image, opens notes.txt through Files, and
compares the editor pixels with boot 1. Only the disk image crosses the power
cycle.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

from PIL import Image, ImageChops

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from exercise import Qmp, Serial, build, qemu_argv, qtype  # noqa: E402

WIN_RE = re.compile(
    r"wm: win \d+ title \d+,\d+ \d+x\d+ client (\d+),(\d+) (\d+)x(\d+)"
)


def key_event(qmp, key, down):
    reply = qmp.cmd("input-send-event", events=[
        {"type": "key",
         "data": {"down": down, "key": {"type": "qcode", "data": key}}}
    ])
    if reply is None or "error" in reply:
        raise RuntimeError(f"QMP rejected key {key!r}: {reply!r}")


def chord(qmp, modifier, key):
    key_event(qmp, modifier, True)
    time.sleep(0.08)
    qmp.sendkey(key)
    time.sleep(0.08)
    key_event(qmp, modifier, False)
    time.sleep(0.08)


def press(qmp, key):
    key_event(qmp, key, True)
    key_event(qmp, key, False)


def wait_window(ser, ceiling):
    ok, first = ser.wait("wm: win ", ceiling)
    if not ok:
        return None, first
    ok, rest = ser.wait("\n", ceiling)
    blob = first + rest
    matches = WIN_RE.findall(blob)
    if not ok or not matches:
        return None, blob
    return tuple(int(v) for v in matches[-1]), blob


def stable_crop(qmp, tmp, stem, box, ceiling=20, different_from=None,
                min_change=0):
    last = None
    same = 0
    deadline = time.monotonic() + ceiling
    serial = 0
    while time.monotonic() < deadline:
        path = os.path.join(tmp, f"{stem}-{serial}.ppm")
        serial += 1
        if not qmp.screendump(path):
            continue
        image = Image.open(path).convert("RGB").crop(
            (box[0], box[1], box[0] + box[2], box[1] + box[3])
        ).copy()
        if (different_from is not None and
                changed_pixels(different_from, image) <= min_change):
            time.sleep(0.15)
            continue
        pixels = image.tobytes()
        if pixels == last:
            same += 1
            if same >= 2:
                return image
        else:
            last = pixels
            same = 0
        time.sleep(0.15)
    return None


def changed_pixels(a, b):
    if a is None or b is None or a.size != b.size:
        return 0
    diff = ImageChops.difference(a, b)
    raw = diff.tobytes()
    return sum(1 for i in range(0, len(raw), 3)
               if raw[i] or raw[i + 1] or raw[i + 2])


def replace_sockets(argv, old_ser, old_qmp, new_ser, new_qmp):
    return [arg.replace(old_ser, new_ser).replace(old_qmp, new_qmp)
            for arg in argv]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--boot-timeout", type=float, default=240)
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--keep-shots", action="store_true")
    args = parser.parse_args()

    if not args.no_build:
        build(False)

    tmp = tempfile.mkdtemp(prefix="zlos-files-")
    ser1_path = os.path.join(tmp, "ser-1")
    qmp1_path = os.path.join(tmp, "qmp-1")
    argv1 = qemu_argv(tmp, False, ser1_path, qmp1_path)
    failures = []
    saved_editor = None
    saved_list = None

    def check(label, condition, detail=""):
        print(f"  {'ok  ' if condition else 'FAIL'}  {label}"
              f"{('   ' + detail) if detail else ''}")
        if not condition:
            failures.append(label)

    proc = None
    try:
        # Boot 1: the Files app owns formatting and all named-file operations.
        proc = subprocess.Popen(argv1, cwd=HERE, stdout=subprocess.DEVNULL,
                                stderr=subprocess.DEVNULL)
        ser1, qmp1 = Serial(ser1_path), Qmp(qmp1_path)
        ok, log = ser1.wait("compositor:", args.boot_timeout)
        if not ok:
            print("boot 1 never reached the compositor:\n" + log[-1500:])
            return 1
        ser1.drain(1.0)
        # Startup reports the shell/monitor/about windows after "compositor:".
        # They are not candidates for the window opened by the next command.
        # Leaving them buffered made wait_window() return an old client rect,
        # so every screenshot watched the wrong app and the gate confidently
        # reported that Files ignored all keys.
        ser1.buf = ""

        qtype(qmp1, "files\n")
        files_box, log = wait_window(ser1, 60)
        check("Files opened as a window", files_box is not None)
        if files_box is None:
            return 1
        print(f"  note  Files client {files_box[0]},{files_box[1]} "
              f"{files_box[2]}x{files_box[3]}")
        ser1.wait("zl> ", 60)  # consume the prompt emitted after `files`

        blank = stable_crop(qmp1, tmp, "blank", files_box)
        press(qmp1, "f")
        format_ok, format_log = ser1.wait("files: formatted", 30)
        check("Files routed the format shortcut to zlfs", format_ok,
              repr(format_log[-160:]))
        if not format_ok:
            return 1
        formatted = stable_crop(qmp1, tmp, "formatted", files_box,
                                different_from=blank, min_change=100)
        check("Shift+F formatted the blank NVMe volume",
              changed_pixels(blank, formatted) > 100)

        press(qmp1, "n")
        new_ok, new_log = ser1.wait("files: new name", 10)
        check("Files entered named-create mode", new_ok, repr(new_log[-120:]))
        if not new_ok:
            return 1
        qtype(qmp1, "trash.tmp\n")
        create_ok, create_log = ser1.wait("files: created", 30)
        check("Files committed trash.tmp to zlfs", create_ok,
              repr(create_log[-120:]))
        if not create_ok:
            return 1
        trash = stable_crop(qmp1, tmp, "trash", files_box,
                            different_from=formatted, min_change=100)
        check("N created trash.tmp by name",
              changed_pixels(formatted, trash) > 100)

        press(qmp1, "d")
        delete_ok, delete_log = ser1.wait("files: deleted", 30)
        check("Files committed the delete to zlfs", delete_ok,
              repr(delete_log[-120:]))
        if not delete_ok:
            return 1
        deleted = stable_crop(qmp1, tmp, "deleted", files_box,
                              different_from=trash, min_change=100)
        check("D deleted the selected file",
              changed_pixels(trash, deleted) > 100)

        press(qmp1, "n")
        new_ok, new_log = ser1.wait("files: new name", 10)
        if not new_ok:
            check("Files re-entered named-create mode", False,
                  repr(new_log[-120:]))
            return 1
        qtype(qmp1, "notes.txt\n")
        create_ok, create_log = ser1.wait("files: created", 30)
        check("Files committed notes.txt to zlfs", create_ok,
              repr(create_log[-120:]))
        if not create_ok:
            return 1
        notes = stable_crop(qmp1, tmp, "notes", files_box,
                            different_from=deleted, min_change=100)
        check("N created notes.txt by name",
              changed_pixels(deleted, notes) > 100)
        if notes is None:
            return 1
        qmp1.sendkey("ret")
        editor_box, log = wait_window(ser1, 60)
        check("Enter opened the selected file in zlEDIT", editor_box is not None)
        if editor_box is None:
            ser1.drain(0.5)
            print("  note  serial after ignored shortcuts: "
                  + repr(ser1.buf[-500:]))
            return 1

        qtype(qmp1, "persist ")
        before_paste = stable_crop(qmp1, tmp, "before-paste", editor_box)
        chord(qmp1, "ctrl", "c")
        chord(qmp1, "ctrl", "v")
        after_paste = stable_crop(qmp1, tmp, "after-paste", editor_box,
                                  different_from=before_paste, min_change=40)
        check("Ctrl+C and Ctrl+V copied and pasted the document",
              changed_pixels(before_paste, after_paste) > 40)
        qtype(qmp1, "cold boot")
        chord(qmp1, "ctrl", "s")
        saved_editor = stable_crop(qmp1, tmp, "saved", editor_box)
        check("Ctrl+S left a stable saved editor frame", saved_editor is not None)

        # ESC saves once more and returns focus to the shell. Listing the file
        # before power-off proves the write completed, not merely the redraw.
        qmp1.sendkey("esc")
        closed1 = stable_crop(qmp1, tmp, "closed-1", editor_box,
                              different_from=saved_editor, min_change=100)
        check("ESC saved and closed the disk editor",
              changed_pixels(saved_editor, closed1) > 100)
        saved_files = stable_crop(qmp1, tmp, "saved-list", files_box)
        if saved_files is None:
            check("Files redrew the saved file size", False)
            return 1
        saved_list = saved_files.crop(
            (0, 40, saved_files.width, max(41, saved_files.height - 100))
        )

        proc.kill()
        proc.wait()
        proc = None

        # Boot 2: same argv and same NVMe image; only socket names change.
        ser2_path = os.path.join(tmp, "ser-2")
        qmp2_path = os.path.join(tmp, "qmp-2")
        argv2 = replace_sockets(argv1, ser1_path, qmp1_path,
                                ser2_path, qmp2_path)
        proc = subprocess.Popen(argv2, cwd=HERE, stdout=subprocess.DEVNULL,
                                stderr=subprocess.DEVNULL)
        ser2, qmp2 = Serial(ser2_path), Qmp(qmp2_path)
        ok, log = ser2.wait("compositor:", args.boot_timeout)
        if not ok:
            print("boot 2 never reached the compositor:\n" + log[-1500:])
            return 1
        ser2.drain(1.0)
        ser2.buf = ""

        qtype(qmp2, "files\n")
        files_box2, log = wait_window(ser2, 60)
        check("Files mounted zlfs on demand after the cold boot",
              files_box2 is not None)
        if files_box2 is None:
            return 1
        ser2.wait("zl> ", 60)  # consume the prompt emitted after `files`
        list2 = stable_crop(qmp2, tmp, "cold-list", files_box2)
        if list2 is None:
            check("the cold-boot Files window rendered", False)
            return 1
        list2_rows = list2.crop((0, 40, list2.width,
                                 max(41, list2.height - 100)))
        check("the named-file rows are unchanged after power-off",
              ImageChops.difference(saved_list, list2_rows).getbbox() is None)

        qmp2.sendkey("ret")
        editor_box2, log = wait_window(ser2, 60)
        check("notes.txt opens from Files after the cold boot",
              editor_box2 is not None)
        if editor_box2 is None:
            return 1
        cold_editor = stable_crop(qmp2, tmp, "cold-editor", editor_box2)
        exact = (saved_editor is not None and cold_editor is not None and
                 saved_editor.size == cold_editor.size and
                 ImageChops.difference(saved_editor, cold_editor).getbbox() is None)
        check("the exact editor contents survived the QEMU power cycle", exact)

        if args.keep_shots:
            shots = os.path.join(HERE, "shots")
            os.makedirs(shots, exist_ok=True)
            if saved_editor is not None:
                saved_editor.save(os.path.join(shots, "files-saved.png"))
            if cold_editor is not None:
                cold_editor.save(os.path.join(shots, "files-cold-boot.png"))
            print(f"  note  screenshots in {shots}")
    finally:
        if proc is not None:
            proc.kill()
            proc.wait()
        if not args.keep_shots:
            shutil.rmtree(tmp)
        else:
            print(f"  note  diagnostic files kept in {tmp}")

    print()
    if failures:
        print(f"Files gate FAILED: {len(failures)} assertion(s)")
        return 1
    print("Files gate green: named create/open/delete and exact contents survived power-off")
    return 0


if __name__ == "__main__":
    sys.exit(main())
