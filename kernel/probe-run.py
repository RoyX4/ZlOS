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
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from exercise import Serial, Qmp, qemu_argv, build, PROMPT  # noqa: E402
sys.path.insert(0, os.path.join(HERE, "oracle"))
import zlosboot as zb  # noqa: E402

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

_lc_spec = _ilu.spec_from_file_location(
    "app_lifecycle", os.path.join(HERE, "probe-app-lifecycle.py"))
_lc = _ilu.module_from_spec(_lc_spec)
_lc_spec.loader.exec_module(_lc)

SHOTS = os.path.join(HERE, "shots")
COMPOSITOR = "compositor:"
DEFAULT_RECEIPT = os.path.join(HERE, "oracle", "out", "run-qemu.json")


def stream(transcript):
    transcript.ser.pump()
    return transcript.log + transcript.ser.buf


def await_lifecycle(transcript, start, event, app, ceiling):
    deadline = time.monotonic() + ceiling
    while time.monotonic() < deadline:
        current = stream(transcript)
        found = [item for item in _lc.events(current[start:])
                 if item["event"] == event and item["app"] == app]
        if found:
            return found[-1]
        time.sleep(0.01)
    raise RuntimeError(f"timed out waiting for lifecycle {event} app {app}")


def qcommand(transcript, qmp, command, ceiling, prompt_ready=False):
    """Submit one emulated-keyboard command at an exact prompt boundary."""
    if not prompt_ready and not transcript.expect(PROMPT, ceiling):
        return False
    qtype(qmp, command + "\n")
    # term_key writes the prompt before inviting a line and only the submitted
    # characters here. Looking for `zl> command` after the prompt was already
    # consumed made the very first command fail and the second inherit its
    # delayed echo.
    return transcript.expect(command + "\n", ceiling)


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def ppm_size(path):
    with open(path, "rb") as handle:
        tokens = []
        while len(tokens) < 4:
            line = handle.readline()
            if not line:
                raise RuntimeError("truncated PPM header")
            line = line.split(b"#", 1)[0]
            tokens.extend(line.split())
    if tokens[0] != b"P6":
        raise RuntimeError(f"unexpected screendump format {tokens[0]!r}")
    return int(tokens[1]), int(tokens[2])


def write_receipt(path, value):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    temp = path + ".tmp"
    with open(temp, "w", encoding="utf-8") as handle:
        json.dump(value, handle, indent=2, sort_keys=True)
        handle.write("\n")
    os.replace(temp, path)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--step-timeout", type=float, default=60)
    ap.add_argument("--settle", type=float, default=1.5)
    ap.add_argument("--keep-shots", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--receipt", default=DEFAULT_RECEIPT)
    args = ap.parse_args()

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
        arena = re.search(
            r"arena: 16 MiB at (\d+) MiB, ends at (\d+) MiB, ceiling (\d+) MiB",
            t.log)
        check("the arena is up and reports a bounded RAM range",
              arena is not None and int(arena.group(2)) <= int(arena.group(3)),
              arena.group(0) if arena else "missing arena receipt")
        check("the arena did NOT report missing RAM",
              "NOT BACKED BY RAM" not in t.log)

        if COMPOSITOR in t.log or t.expect(COMPOSITOR, 8):
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

        # Synchronize at the actual interactive prompt before injecting the
        # first command. Consuming one arbitrary newline here used to leave
        # boot prose in front of the prompt; the first command then timed out,
        # and its delayed echo was falsely accepted as the second command.
        if not t.expect(PROMPT, args.step_timeout):
            print("the compositor started but the shell never became interactive")
            return 1
        m = re.search(r"shell client (\d+),(\d+) (\d+)x(\d+)", t.log)
        if not m:
            print("the compositor did not report the shell rect")
            return 1
        box = tuple(int(g) for g in m.groups())
        print(f"  ok    shell client rect {box[0]},{box[1]} {box[2]}x{box[3]}")

        time.sleep(args.settle)
        lifecycle_before_run = _lc.events(stream(t))
        if not lifecycle_before_run:
            print("the compositor emitted no lifecycle telemetry")
            return 1
        lifecycle_baseline = lifecycle_before_run[-1]["live"]
        run_open_start = len(stream(t))

        # ---- `run` with no filename --------------------------------------
        first_response_start = len(stream(t))
        if not qcommand(t, qmp, "run", args.step_timeout, prompt_ready=True):
            check("`run` was typed", False, "the keystrokes never arrived")
        else:
            check("`run` was typed and echoed", True)
            has_no_filename = t.expect("no filename", args.step_timeout)
            has_usage = t.expect("run hello.zl", args.step_timeout)
            got = stream(t)[first_response_start:]
            check("`run` alone says it wants a filename",
                  has_no_filename, repr(got.strip()[:70]))
            check("...and shows how to use it", has_usage)
            check("it did NOT claim to have run anything",
                  "found" not in got.lower())

        # ---- `run nothing.zl` --------------------------------------------
        # The first typed argument in this shell's history that is neither a
        # bare word nor a number. If match_cmd's digit scan had been left to
        # decide where the text starts, this is the case that exposes it.
        disk_response_start = len(stream(t))
        if not qcommand(t, qmp, "run nothing.zl", args.step_timeout):
            check("`run nothing.zl` was typed", False, "keystrokes never arrived")
        else:
            check("`run nothing.zl` was typed and echoed", True)
            has_no_filesystem = t.expect("no filesystem on the disk",
                                         args.step_timeout)
            blob = stream(t)[disk_response_start:]
            # THE DRIVER IS PRESENT NOW, and the message has to change with it.
            # Before fs.c was merged from desktop/system-track, exec.c's weak
            # fs_* symbols were NULL and this said "no fs driver". They now bind
            # to the real filesystem, so the honest answer is about the DISK -
            # there is a driver, nothing is mounted on it. That the sentence
            # changed by itself, with no edit to exec.c, is the weak-symbol seam
            # working exactly as it was designed to.
            check("it says there is no filesystem",
                  has_no_filesystem, repr(blob.strip()[:70]))
            check("it blames the DISK, not the driver - the driver is linked now",
                  "on the disk" in blob and "no fs driver" not in blob)
            check("the two refusals are DIFFERENT sentences",
                  "no filename" not in blob)

        run_opened = await_lifecycle(t, run_open_start, "open", 7,
                                     args.step_timeout)
        run_ready = await_lifecycle(t, run_open_start, "ready", 7,
                                    args.step_timeout)
        check("the Run surface opened as APP_RUN id 7",
              run_opened["app"] == 7)
        check("the Run surface reached its first compositor draw",
              run_ready["slot"] == run_opened["slot"] and
              run_ready["generation"] == run_opened["generation"])

        # ---- and once a filesystem EXISTS, a third distinct answer ---------
        # `.` mounts zlfs, formatting the NVMe disk if it is blank. That moves
        # `run` from "there is nowhere to look" to "I looked and it is not
        # there" - which is the whole error ladder, and the assertion that the
        # weak symbols really did bind rather than merely link.
        if qcommand(t, qmp, ".", args.step_timeout):
            has_mount = t.expect("mounted:", args.step_timeout)
            check("`.` mounted a filesystem", has_mount)

            if qcommand(t, qmp, "run nothing.zl", args.step_timeout):
                missing_start = len(stream(t))
                has_missing_file = t.expect("no such file", args.step_timeout)
                has_missing_name = t.expect("nothing.zl", args.step_timeout)
                blob2 = stream(t)[missing_start:]
                check("with a filesystem mounted, run says NO SUCH FILE",
                      has_missing_file, repr(blob2.strip()[:70]))
                check("...and names it", has_missing_name)
                check("...and no longer claims there is no filesystem",
                      "no filesystem" not in blob2)
        else:
            check("`.` mounted a filesystem", False, "keystroke never arrived")

        # ---- the window --------------------------------------------------
        # A window opened by `run` must not have replaced the terminal or
        # wedged the frame loop: type something ordinary afterwards and it has
        # to still work. This is the assertion that catches a compositor that
        # died the moment a fourth window appeared.
        time.sleep(args.settle)
        if qcommand(t, qmp, "fib 20", args.step_timeout):
            check("the terminal still works after run opened a window",
                  t.expect("6765", args.step_timeout))
        else:
            check("the terminal still works after run opened a window", False,
                  "no echo - the compositor may have stopped")

        # ---- run twice must not open a second window ----------------------
        opens_before_duplicate = len([
            event for event in _lc.events(stream(t))
            if event["event"] == "open" and event["app"] == 7
        ])
        duplicate_typed = qcommand(t, qmp, "run nothing.zl", args.step_timeout)
        duplicate_answered = (t.expect("no such file", args.step_timeout)
                              if duplicate_typed else False)
        time.sleep(args.settle)
        opens_after_duplicate = len([
            event for event in _lc.events(stream(t))
            if event["event"] == "open" and event["app"] == 7
        ])
        check("a second `run` reused the existing Run surface",
              duplicate_typed and duplicate_answered and
              opens_after_duplicate == opens_before_duplicate,
              f"open events {opens_before_duplicate}->{opens_after_duplicate}")
        if qcommand(t, qmp, "fib 20", args.step_timeout):
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

        # Focus the Run title bar through the real pointer route, then close it
        # through the unified Ctrl+W route. The Run surface deliberately hands
        # focus back to Terminal after opening, so closing without the click
        # would prove teardown of the wrong window.
        t.ser.drain(0.1)
        reports = re.findall(
            r"wm: win (\d+) title (\d+),(\d+) (\d+)x(\d+)", stream(t))
        if not reports:
            check("Run emitted a pointer-addressable window rectangle", False)
            run_closed = None
        else:
            win, tx, ty, tw, th = (int(value) for value in reports[-1])
            frame = os.path.join(tmp, "run-close.ppm")
            if not qmp.screendump(frame):
                check("Run teardown frame could be captured", False)
                run_closed = None
            else:
                width, height = ppm_size(frame)
                close_start = len(stream(t))
                zb.click(qmp, tx + max(8, tw // 3), ty + max(4, th // 2),
                         width, height, t.ser.drain)
                t.ser.send("\x17")
                run_closed = await_lifecycle(t, close_start, "close", 7,
                                             args.step_timeout)
                lifecycle_errors = _lc.validate_cycle(
                    run_opened, run_ready, run_closed, 7, lifecycle_baseline)
                check("Run completed open-ready-close without a live-window leak",
                      not lifecycle_errors, "; ".join(lifecycle_errors))

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
    receipt = {
        "schema": "zlos.run-route-qemu-receipt.v1",
        "evidence": "QEMU keyboard, filesystem error ladder, window lifecycle; not successful executable loading or physical proof",
        "source_head": subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=os.path.dirname(HERE), text=True).strip(),
        "source_files_sha256": {
            name: sha256(os.path.join(HERE, name))
            for name in ("kernel.zl", "wm.c", "exec.c", "term.c",
                         "app-manifest.json", "build-identity.json",
                         "build_identity_embed.zl", "probe-run.py",
                         "probe-app-lifecycle.py", "probe-term.py",
                         "exercise.py", "oracle/zlosboot.py")
        },
        "artifact": {
            "path": "kernel/zlOS.iso",
            "sha256": sha256(os.path.join(HERE, "zlOS.iso")),
        },
        "shipped_manifest": _lc.shipped_manifest(stream(t)),
        "shipped_build_identity": _lc.shipped_build_identity(stream(t)),
        "result": {
            "app": {"id": 7, "name": "Run"},
            "lifecycle": {
                "open": run_opened,
                "ready": run_ready,
                "close": run_closed,
            },
            "duplicate_open_events": {
                "before": opens_before_duplicate,
                "after": opens_after_duplicate,
            },
            "failure_ladder": ["no filename", "no filesystem on the disk",
                               "no such file"],
            "assertions_failed": 0,
        },
        "weakest_link": "the loader's successful execution path is outside this receipt; physical keyboard and display are unproven here",
    }
    write_receipt(args.receipt, receipt)
    print("ok    `run` declines clearly, and differently for different reasons")
    print(f"ok    Run identity/lifecycle receipt -> {args.receipt}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
