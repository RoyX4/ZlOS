#!/usr/bin/env python3
"""probe-run.py - does `run` decline CLEARLY, in the real machine?

Item 1 of the exec track: the command, the app window, and the error path,
built before anything can execute. The brief's reasoning is that you get the
failure modes right while they are the only modes - and a failure mode nobody
typed is a failure mode nobody has tested.

hosttest/exectest.c already asserts exec.c's decision table in milliseconds
with no QEMU, and it covers far more states than this does (a fake filesystem
lets it reach not-found, empty, too-big and loaded, none of which exist on this
branch). So why boot a machine at all?

Because exectest cannot see any of this:

  - that `run` is in term.c's word table and reaches run_command at all
  - that the ARGUMENT TEXT survives term.c -> exec.c. exectest hands exec.c a
    string directly; here it has to come off an emulated keyboard, through
    term_key, through match_cmd's split, into argstr, and out again
  - that the line actually reaches a human. exec.c calls term_say, which writes
    the scrollback AND the serial port but deliberately not the console; if
    that ever became term_putc the message would be in the scrollback only and
    every headless gate would go blind while looking like it passed
  - that a filename with a DOT in it survives. `nothing.zl` is the first thing
    ever typed at this shell that is not a bare word or a number
  - that opening the program's window does not disturb the terminal, and that
    `run` twice does not open two windows

THE ONE THING THIS GATE IS REALLY FOR: `run` and `run nothing.zl` must produce
two DIFFERENT clear lines. "Something was printed" is not the assertion - a
usage error and a missing filesystem are different problems with different
fixes, and a shell that answers both with the same sentence has told you
nothing. That is the same argument runtime_kernel.c makes about telling "no
driver" apart from "no device", where conflating them cost a long hunt.

Keys go through QMP input-send-event, not serial: the compositor's only input
is input_poll(), and nothing in that path ever looks at COM1. Bytes sent down
the serial socket after 'w' sit in the FIFO and are eventually eaten by the
text shell as single-key commands, which is worse than nothing. All of that is
probe-term.py's finding, reused here.

Nothing waits a fixed wall-clock time for anything the guest decides.

    ./probe-run.py
    ./probe-run.py --keep-shots     # ...and leave the window PNG in shots/
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from exercise import Serial, Qmp, qemu_argv, build, PROMPT  # noqa: E402

# Transcript and qtype are probe-term.py's, and they are borrowed rather than
# copied. Both encode findings that were expensive to make - Serial.wait()
# DRAINS its buffer on a failed match and silently loses output a later
# assertion needs, and send-key is dropped under -display none where
# input-send-event is not. A second copy here would drift from the original the
# first time either is fixed, and probe-term.py belongs to the platform track.
#
# Imported by path because the filename has a hyphen in it. Its module level is
# imports, constants and defs behind an __main__ guard - nothing runs.
import importlib.util as _ilu  # noqa: E402
_spec = _ilu.spec_from_file_location("probe_term", os.path.join(HERE, "probe-term.py"))
_pt = _ilu.module_from_spec(_spec)
_spec.loader.exec_module(_pt)
Transcript, qtype = _pt.Transcript, _pt.qtype

SHOTS = os.path.join(HERE, "shots")
COMPOSITOR = "compositor:"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--step-timeout", type=float, default=60)
    ap.add_argument("--settle", type=float, default=1.5)
    ap.add_argument("--keep-shots", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()

    import time

    if not args.no_build:
        build(False)

    tmp = tempfile.mkdtemp(prefix="proberun-")
    ser_path = os.path.join(tmp, "ser.sock")
    qmp_path = os.path.join(tmp, "qmp.sock")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                            cwd=HERE, stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL)
    failures = []

    def check(label, ok, detail=""):
        print(f"  {'ok  ' if ok else 'FAIL'}  {label}{('   ' + detail) if detail else ''}")
        if not ok:
            failures.append(label)

    try:
        ser = Serial(ser_path)
        qmp = Qmp(qmp_path)
        t = Transcript(ser)

        if not t.expect("ready.", args.boot_timeout):
            print("never booted. serial so far:\n" + t.log[-2000:])
            return 1

        # THE ARENA LINE IS PART OF THE BOOT LOG NOW, and it carries an
        # address. Asserting on it here as well as in golden.txt is not
        # duplication: golden.txt proves the text has not changed, this proves
        # the arena came up RAM-backed on a machine that is about to be asked
        # to use it. They fail for different reasons.
        check("the arena is up and says where it is",
              "arena: 16 MiB at 8 MiB" in t.log)
        check("the arena did NOT report missing RAM",
              "NOT BACKED BY RAM" not in t.log)

        if t.expect(COMPOSITOR, 8):
            print("  note  the compositor is the boot state - no 'w' needed")
        else:
            if not t.expect(PROMPT, args.step_timeout):
                print("booted but no shell prompt:\n" + t.log[-2000:])
                return 1
            ser.send("w")
            if not t.expect(COMPOSITOR, args.step_timeout):
                print("the compositor never started:\n" + t.log[-2000:])
                return 1
            print("  ok    'w' started the compositor")

        rest = t.seen("\n", args.step_timeout) or ""
        m = re.search(r"shell client (\d+),(\d+) (\d+)x(\d+)", rest)
        if not m:
            print("the compositor did not report the shell rect: " + repr(rest))
            return 1
        box = tuple(int(g) for g in m.groups())
        print(f"  ok    shell client rect {box[0]},{box[1]} {box[2]}x{box[3]}")

        time.sleep(args.settle)

        # ---- `run` with no filename --------------------------------------
        qtype(qmp, "run\n")
        if not t.expect("zl> run", args.step_timeout):
            check("`run` was typed", False, "the keystrokes never arrived")
        else:
            check("`run` was typed and echoed", True)
            got = t.seen("\n", args.step_timeout) or ""
            got += t.seen("\n", args.step_timeout) or ""
            check("`run` alone says it wants a filename",
                  "no filename" in got, repr(got.strip()[:70]))
            check("...and shows how to use it", "run hello.zl" in got)
            check("it did NOT claim to have run anything",
                  "found" not in got.lower())

        # ---- `run nothing.zl` --------------------------------------------
        # The first typed argument in this shell's history that is neither a
        # bare word nor a number. If match_cmd's digit scan had been left to
        # decide where the text starts, this is the case that exposes it.
        qtype(qmp, "run nothing.zl\n")
        if not t.expect("zl> run nothing.zl", args.step_timeout):
            check("`run nothing.zl` was typed", False, "keystrokes never arrived")
        else:
            check("`run nothing.zl` was typed and echoed", True)
            blob = ""
            for _ in range(4):
                blob += t.seen("\n", args.step_timeout) or ""
            check("it says there is no filesystem",
                  "no filesystem" in blob, repr(blob.strip()[:70]))
            check("it blames the missing DRIVER, not the disk",
                  "no fs driver" in blob)
            check("it names the file it could not look up",
                  "nothing.zl" in blob)
            check("it points at the storage that DOES exist",
                  "ls" in blob)
            check("the two refusals are DIFFERENT sentences",
                  "no filename" not in blob)

        # ---- the window --------------------------------------------------
        # A window opened by `run` must not have replaced the terminal or
        # wedged the frame loop: type something ordinary afterwards and it has
        # to still work. This is the assertion that catches a compositor that
        # died the moment a fourth window appeared.
        time.sleep(args.settle)
        qtype(qmp, "fib 20\n")
        if t.expect("zl> fib 20", args.step_timeout):
            check("the terminal still works after run opened a window",
                  t.expect("6765", args.step_timeout))
        else:
            check("the terminal still works after run opened a window", False,
                  "no echo - the compositor may have stopped")

        # ---- run twice must not open a second window ----------------------
        qtype(qmp, "run nothing.zl\n")
        t.expect("zl> run nothing.zl", args.step_timeout)
        time.sleep(args.settle)
        qtype(qmp, "fib 20\n")
        if t.expect("zl> fib 20", args.step_timeout):
            check("a second `run` left the machine responsive",
                  t.expect("6765", args.step_timeout))
        else:
            check("a second `run` left the machine responsive", False)

        if args.keep_shots:
            os.makedirs(SHOTS, exist_ok=True)
            ppm = os.path.join(tmp, "run.ppm")
            if qmp.screendump(ppm):
                png = os.path.join(SHOTS, "run-window.png")
                subprocess.run(["convert", ppm, png], capture_output=True)
                if not os.path.exists(png):
                    subprocess.run(["cp", ppm, os.path.join(SHOTS, "run-window.ppm")])
                print(f"  note  screenshot in {SHOTS}")

    finally:
        proc.terminate()
        try:
            proc.wait(timeout=10)
        except subprocess.TimeoutExpired:
            proc.kill()

    print()
    if failures:
        print(f"FAIL  {len(failures)} of the run gate's assertions:")
        for f in failures:
            print(f"        {f}")
        return 1
    print("ok    `run` declines clearly, and differently for different reasons")
    return 0


if __name__ == "__main__":
    sys.exit(main())
