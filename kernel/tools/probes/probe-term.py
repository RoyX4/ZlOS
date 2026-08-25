#!/usr/bin/env python3
"""probe-term.py - does the terminal in the compositor actually WORK?

term.c was written, wired, linked and committed, and nobody had ever typed a
command into it. Everything in the platform queue sits on top of that
assumption, so it gets a gate rather than a hand test.

It types ten lines and asserts on each:

    help        the app list reaches the shell
    uptime      a live figure, so the command really ran
    fib 20      -> 6765, which proves the ARGUMENT parser, not just dispatch
    nonsense    -> "unknown command: nonsense"     <- THE IMPORTANT ONE
    i2c         bounded status; never stalls the compositor to probe hardware
    dig<Left>a  -> diag                              cursor insertion
    dixag<Home><Right><Right><Delete><End> -> diag   cursor deletion
    <Up>        -> diag                              command history
    x<Up><Down> -> x                                 draft restoration
    clear       the scrollback empties

The unknown-command case matters most. A shell that silently ignores what you
typed is worse than one with no commands at all, and it is the only one of the
five whose failure looks exactly like success from a distance.

TWO THINGS THIS HAD TO WORK AROUND, both measured rather than guessed:

1. THE KEYS GO THROUGH QMP, WHICH TYPES ON THE EMULATED KEYBOARD.

   **The reason originally given here was wrong and is corrected below**, since
   a false "that is not supported" is the expensive kind of stale: nobody
   re-tests what they have been told is absent. What it said was that serial
   keystrokes cannot reach the compositor, because wm_frame()'s only input is
   PS/2 and USB HID and nothing in that path looks at COM1.

   Serial reaches the compositor perfectly well. input.c drains COM1 into the
   same event queue as PS/2 and USB - the block commented `SERIAL, the third
   source` - precisely so every gate in this repo kept working when the desktop
   became the boot state. Measured 2026-08-19: `exercise.py` types `help`,
   `fib 20` and `windows` as words over serial and scores 4/4.

   What misled the original note is that a single character produces NO SERIAL
   OUTPUT AT ALL. term.c buffers printable characters and echoes only in its
   Enter branch, so a delivered keystroke and a dropped one are the same
   silence. Sending 'w' and seeing nothing is not evidence it did not arrive -
   photograph the prompt line and the 'w' is sitting in it.

   Using the keyboard here is still right: this gate's subject includes the
   input stack, and QMP exercises the PS/2 and USB HID decoders, which are the
   only input a laptop has. It is a better assertion, not a workaround.
   `docs/typing-into-the-compositor.md` has the full measurement.

2. THE TERMINAL'S OWN OUTPUT DID NOT REACH THE SERIAL LOG. term_putc writes the
   scrollback ring and nothing else, so the unknown-command message and the
   echo of the typed line were invisible to any headless gate. term.c now emits
   those through term_say(), which writes the scrollback AND the serial port
   but deliberately not the console. Without that change the assertion this
   gate exists for could not be made at all.

`clear` prints nothing on any sink by design, so it is the one assertion made
on PIXELS: an ink count over a QMP screendump, before and after. Both numbers
are printed, because "fewer" is a measurement and "it looked empty" is not.

Nothing here waits a fixed wall-clock time for anything the guest decides -
every wait polls for a marker, per the project rule that a gate must never be
timing-sensitive. The only clocked waits are the settles that let a frame
finish rendering before it is photographed, which decide nothing.

    ./probe-term.py                 # the gate
    ./probe-term.py --keep-shots    # ...and leave the PNGs in shots/
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
HERE = KERNEL_ROOT
sys.path.insert(0, PROBE_DIR)

from exercise import Serial, Qmp, qemu_argv, build, qtype, PROMPT  # noqa: E402

SHOTS = os.path.join(HERE, "shots")

# The compositor announces itself on serial even though its console pixels are
# muted, so "it is up and listening" is distinguishable from "the w never
# arrived". Those are the same silence otherwise, and telling them apart by
# waiting longer is exactly the timing-sensitive gate this project banned.
COMPOSITOR = "compositor:"

# QCODE and qtype() are imported from exercise.py, which is where the one copy
# lives. They used to be duplicated here and in probe-net.py, on the reasoning
# that a module name with a hyphen in it is not importable - true, but both of
# these already import exercise.py, so that was never the obstacle.


class Transcript:
    """Serial.wait() DRAINS the buffer when it fails to match, which silently
    throws away output a later assertion needs. This keeps a running record and
    puts back anything a failed match consumed."""

    def __init__(self, ser):
        self.ser = ser
        self.log = ""

    def expect(self, marker, ceiling=60):
        ok, got = self.ser.wait(marker, ceiling)
        if ok:
            self.log += got
        else:
            self.ser.buf = got + self.ser.buf      # unconsumed - hand it back
        return ok

    def seen(self, marker, ceiling=60):
        """Consume up to and including `marker`, returning what came with it."""
        ok, got = self.ser.wait(marker, ceiling)
        if ok:
            self.log += got
            return got
        self.ser.buf = got + self.ser.buf
        return None


def ppm_crop(path, box, step=2):
    """Every step'th pixel inside `box` = (x, y, w, h), as a list of 3-byte
    colours. A whole-frame sample is the wrong instrument here: the wallpaper
    is a full-screen GRADIENT, so almost every pixel on the desktop is a colour
    that occurs a handful of times, and any "how many unusual colours" measure
    over the whole frame is dominated by the wallpaper and blind to the text.
    Measured: emptying an entire screen of scrollback moved a whole-frame ink
    count by 11%, which is indistinguishable from noise. Cropped to the window,
    the same change is total."""
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
    out = []
    for y in range(max(0, y0), max(0, y1), step):
        base = y * w * 3
        for x in range(max(0, x0), max(0, x1), step):
            p = base + x * 3
            out.append(bytes(px[p:p + 3]))
    return out


def ink(px):
    """How much TEXT is in a crop, as a count of pixels.

    A glyph is drawn in a text colour over a FLAT panel, and anti-aliasing puts
    a spread of intermediate shades along every edge. Inside a window the
    commonest colour is the panel itself, so "everything that is not the
    commonest colour" is the text, its anti-aliased edges, and nothing else. No
    theme colour is named here, which matters: the look track is changing all
    of them in another worktree.
    """
    if not px:
        return None
    counts = {}
    for p in px:
        counts[p] = counts.get(p, 0) + 1
    return len(px) - max(counts.values())


def shot(qmp, tmp, name, box, keep):
    ppm = os.path.join(tmp, name + ".ppm")
    if not qmp.screendump(ppm):
        return None
    px = ppm_crop(ppm, box)
    if keep:
        os.makedirs(SHOTS, exist_ok=True)
        png = os.path.join(SHOTS, "term-" + name + ".png")
        subprocess.run(["convert", ppm, png], capture_output=True)
        if not os.path.exists(png):          # no ImageMagick - keep the PPM
            subprocess.run(["cp", ppm, os.path.join(SHOTS, "term-" + name + ".ppm")])
    return px


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--step-timeout", type=float, default=60)
    ap.add_argument("--settle", type=float, default=1.5,
                    help="seconds to let a frame render before photographing it")
    ap.add_argument("--keep-shots", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--uefi", action="store_true",
                    help="boot the native UEFI/GOP image used by the ThinkPad")
    ap.add_argument("--ps2-only", action="store_true",
                    help="drop the USB keyboard - the ThinkPad's own keyboard "
                         "is PS/2, and Enter/Backspace/ESC arrive there as "
                         "navigation codes with no character attached")
    args = ap.parse_args()

    import time

    if not args.no_build:
        build(args.uefi)

    tmp = tempfile.mkdtemp(prefix="probeterm-")
    ser_path = os.path.join(tmp, "ser.sock")
    qmp_path = os.path.join(tmp, "qmp.sock")
    argv = qemu_argv(tmp, args.uefi, ser_path, qmp_path)
    if args.ps2_only:
        # Every probe in this tree boots with -device usb-kbd, which is why
        # nothing could see that the PS/2 path never produces a character for
        # Enter. Strip it and the emulated i8042 is the only way in.
        #
        # Indexed, not `argv.index(a)` - that returns the FIRST "-device" every
        # time, so the first version of this tested the wrong device, removed
        # nothing, and the gate passed against a USB keyboard while claiming to
        # be PS/2-only. It is asserted below rather than assumed.
        out, i = [], 0
        while i < len(argv):
            if argv[i] == "-device" and i + 1 < len(argv) and "usb-kbd" in argv[i + 1]:
                i += 2
                continue
            out.append(argv[i]); i += 1
        if any("usb-kbd" in a for a in out):
            print("  FAIL  --ps2-only did not remove the USB keyboard"); return 1
        print("  note  PS/2 only - the USB keyboard is not attached")
        argv = out
    proc = subprocess.Popen(argv,
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

        # THE COMPOSITOR MAY ALREADY BE THE BOOT STATE. Queue item 2 replaces
        # the shell loop with wm_session(), at which point there is no text
        # prompt to send 'w' to and sending one would type a stray character
        # into the terminal. Ask which world we are in rather than assuming,
        # so this gate keeps working across that change unaltered.
        #
        # LOOK IN THE WHOLE TRANSCRIPT, not only at what arrives next. The
        # compositor announces itself BEFORE "ready.", so the wait above has
        # already consumed that line and waiting for it a second time finds
        # nothing. Measured 2026-08-19 on merged main: this gate took the else
        # branch on a machine that had booted straight into the desktop, typed
        # a stray 'w' into the terminal, and then failed with "the compositor
        # never started" while its own printed transcript contained
        # "compositor: 4 windows, shell client 82,160 1236x834".
        booted_into_wm = COMPOSITOR in t.log or t.expect(COMPOSITOR, 8)
        if booted_into_wm:
            print("  note  the compositor is the boot state - no 'w' needed")
        else:
            if not t.expect(PROMPT, args.step_timeout):
                print("booted but no shell prompt:\n" + t.log[-2000:])
                return 1
            ser.send("w")                     # the LAST thing sent over serial
            if not t.expect(COMPOSITOR, args.step_timeout):
                print("the compositor never started:\n" + t.log[-2000:])
                return 1
            print("  ok    'w' started the compositor")

        # The rest of that line carries the shell's client rectangle. Crop to
        # what the kernel says rather than recomputing the layout here.
        #
        # Two places to look, for the same reason as above: when the desktop
        # was the boot state the whole line is already in t.log, and when 'w'
        # started it the tail of the line is still unread on the wire.
        m = re.search(r"shell client (\d+),(\d+) (\d+)x(\d+)", t.log)
        if not m:
            rest = t.seen("\n", args.step_timeout) or ""
            m = re.search(r"shell client (\d+),(\d+) (\d+)x(\d+)", rest)
            if not m:
                print("the compositor did not report the shell rect: " + repr(rest))
                return 1
        box = tuple(int(g) for g in m.groups())
        print(f"  ok    shell client rect {box[0]},{box[1]} {box[2]}x{box[3]}")

        time.sleep(args.settle)

        # ---- the five ordinary commands ---------------------------------
        # Each is asserted twice: the ECHO proves the keystrokes arrived at
        # all, and the RESULT proves the command ran. Without the echo a
        # command that produces no output (clear) is indistinguishable from a
        # key that never landed.
        for cmd, marker, what in (
            # "help              this help", not "h        this help". The
            # compositor calls help_typed(), which lists the WORDS term.c
            # accepts; the single-letter table is the old text shell's help and
            # is only reached with no framebuffer. Same staleness as `-k w`:
            # written when a command was one character, never revisited when
            # the desktop made it a word. exercise.py asserts the new string.
            ("help",     "help              this help", "help lists the apps"),
            ("uptime",   "ticks at 100 Hz",            "uptime reports a live figure"),
            ("fib 20",   "6765",                       "fib 20 = 6765 - the ARGUMENT parser works"),
            ("nonsense", "unknown command: nonsense",  "an unknown command SAYS SO"),
            # QEMU has no LPSS I2C. That makes it the exact bounded check for
            # the status path: it must answer rather than re-probe the bus or
            # monopolise the compositor for the old five-second loop.
            ("i2c",      "automatic startup has not found", "i2c status returns without freezing the desktop"),
        ):
            qtype(qmp, cmd + "\n")
            if not t.expect("zl> " + cmd, args.step_timeout):
                check(what, False, "the keystrokes never arrived")
                continue
            check(what, t.expect(marker, args.step_timeout))

        # ---- the physical 2026-08-24 failure pattern --------------------
        # The ThinkPad trace showed Right/Down/Delete reaching app_event and
        # being silently discarded before term.c.  These are keyboard events,
        # not serial bytes, and deliberately edit malformed commands so a
        # matcher that merely accepts `diag` cannot make this pass.
        qtype(qmp, "dig")
        qmp.sendkey("left")
        qtype(qmp, "a\n")
        corrected = t.expect("zl> diag", args.step_timeout)
        check("Left + insertion corrects `dig` to `diag`", corrected and
              t.expect("flight recorder", args.step_timeout))

        qtype(qmp, "dixag")
        for key in ("home", "right", "right", "delete", "end", "ret"):
            qmp.sendkey(key)
            time.sleep(0.12)
        corrected = t.expect("zl> diag", args.step_timeout)
        check("Home/Right/Delete/End correct `dixag` to `diag`", corrected and
              t.expect("flight recorder", args.step_timeout))

        qmp.sendkey("up"); qmp.sendkey("ret")
        recalled = t.expect("zl> diag", args.step_timeout)
        check("Up recalls the newest command", recalled and
              t.expect("flight recorder", args.step_timeout))

        qtype(qmp, "x")
        qmp.sendkey("up"); qmp.sendkey("down"); qmp.sendkey("ret")
        restored = t.expect("zl> x", args.step_timeout)
        check("Down returns from history to the in-progress draft", restored and
              t.expect("unknown command: x", args.step_timeout))

        # ---- clear, which prints nothing anywhere by design --------------
        before = shot(qmp, tmp, "before-clear", box, args.keep_shots)
        qtype(qmp, "clear\n")
        echoed = t.expect("zl> clear", args.step_timeout)
        check("clear was typed and echoed", echoed)
        time.sleep(args.settle)
        after = shot(qmp, tmp, "after-clear", box, args.keep_shots)

        i0, i1 = ink(before), ink(after)
        if i0 is None or i1 is None:
            check("clear empties the scrollback", False, "no screendump")
        else:
            # Inside the client area nothing survives clear except the one-line
            # prompt, so this is a near-total collapse rather than a nudge.
            # 0.2 leaves room for the prompt, the caret and the window's inner
            # border without naming a single theme colour.
            check("clear empties the scrollback", i1 < i0 * 0.2,
                  f"ink {i0} -> {i1} inside the shell")

        # ---- the double prompt -------------------------------------------
        # PLATFORM-PROMPT item 1 names two "zl>" prompts - one captured into
        # the scrollback from the text shell before the compositor starts, one
        # live - and predicts item 2 removes it. It does, and this is the
        # measurement rather than the assumption: with the compositor as the
        # BOOT STATE no text shell ever runs, so the only prompts on the wire
        # are term.c's own echo of each line typed. Ten commands, ten prompts.
        # Under 'w' it is eleven, and the extra one IS the bug.
        typed = 10
        prompts = t.log.count(PROMPT)
        if booted_into_wm:
            check("no captured prompt - one per line typed", prompts == typed,
                  f"{prompts} on the wire, {typed} typed")
        else:
            print(f"  note  '{PROMPT.strip()}' x{prompts} for {typed} typed - "
                  f"the extra one is the text shell's, captured (item 2 removes it)")

        print()
        if failures:
            print(f"terminal gate FAILED: {len(failures)} - " + ", ".join(failures))
            print("\n--- serial transcript (tail) ---\n" + t.log[-3000:])
            return 1
        print("terminal gate green: ten lines typed, cursor/history/results asserted")
        return 0
    finally:
        proc.kill()
        proc.wait()
        subprocess.run(["rm", "-rf", tmp])


if __name__ == "__main__":
    sys.exit(main())
