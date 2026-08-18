#!/usr/bin/env python3
"""probe-shot.py - boot zlOS, press some keys, and save a PNG of the screen.

The visual gates in kernel/docs/desktop-TODO.md all read "screendump it and
look" - the icon edges, the sparkline, the logo, the window stack. Doing that
by hand means ./try.sh, a GTK window, and a human. This does it headless, so it
can run unattended and leave a file behind.

It is the same harness exercise.py uses: boot the ISO (GRUB supplies the
multiboot framebuffer tag that QEMU's own -kernel loader never does), talk to
the serial socket, and take the picture over QMP.

  ./probe-shot.py                        the desktop as it boots
  ./probe-shot.py -k windows             after running the `windows` command
  ./probe-shot.py -k help -k nonsense    two commands, in order
  ./probe-shot.py -k anim --settle 3     give an animation time to run
  ./probe-shot.py -k windows --via keyboard    ...typed on the emulated keyboard
  ./probe-shot.py --crop 200,600,700,200 --zoom 3 -o dock
                                         a magnified crop, for judging edges

-k TAKES A COMMAND, NOT A KEYPRESS, and it used to take a keypress. That is the
whole of the bug this note exists for. `-k w` sent one 'w' down the wire and
photographed the screen 0.4 s later; under the compositor the shell is a window
whose input is a LINE, so the 'w' landed in term.c's line buffer and sat there
unsubmitted for ever. The picture came back looking exactly like a desktop that
had ignored the key, WITHOUT AN ERROR, so anything gating on it was asserting
against a frame no command had run in.

MEASURED, 2026-08-19, three boots of this script on this branch:

    no -k        vs  -k w                 1225 px differ (0.05%), all of it
                                          inside the shell's prompt line: the
                                          'w' is ON SCREEN, in the line buffer
    no -k        vs  -k $'windows\\r'      113015 px differ (4.9%), the whole
                                          desktop - the command actually ran

So the key was arriving all along; nothing was submitting it. The serial byte
reaches the compositor because input.c feeds COM1 into the one event queue
alongside PS/2 and USB ("SERIAL, the third source") - exactly so that every
gate in this repo kept working when the desktop became the boot state. A
single character produces NO serial output because term.c echoes the line only
on Enter, and "no output" was read as "the key never arrived".

Each -k is now typed WITH ITS Enter, and this then waits for term.c to echo the
line back - which happens on Enter and at no other time, on whichever wire the
keys came in on. That echo is the proof the line was taken; without it this
exits non-zero and says so, rather than photographing a stale frame.

Output lands in kernel/shots/ as PNG. Nothing here waits a fixed wall-clock
time for a BOOT or for a command to be taken - it polls the serial log, per the
project rule that a gate must never be timing-sensitive. --settle is different:
it is how long to let an animation run before photographing it, which is a
deliberate duration, not a guess at how slow the host is.
"""
import argparse, os, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, qtype, PROMPT

HERE = os.path.dirname(os.path.abspath(__file__))
SHOTS = os.path.join(HERE, "shots")


def to_png(ppm, png, crop=None, zoom=1):
    from PIL import Image
    img = Image.open(ppm)
    if crop:
        x, y, w, h = crop
        img = img.crop((x, y, x + w, y + h))
    if zoom > 1:
        # NEAREST on purpose: this is for judging whether an EDGE is smooth,
        # and any smoothing filter here would invent the very thing being
        # looked for.
        img = img.resize((img.width * zoom, img.height * zoom), Image.NEAREST)
    img.save(png)
    return img.size


def type_command(ser, qmp, cmd, via, ceiling):
    """Type one shell command, submit it, and prove the shell took the line.

    term.c echoes the submitted text to the serial log inside term_key's Enter
    branch and nowhere else - the prefix goes to the scrollback only, the typed
    characters to both. So `cmd + "\\n"` appearing on serial is the one exact
    marker for "this line was submitted", it arrives the instant Enter is
    handled, and it is identical whether the keys came over COM1 or off the
    emulated keyboard. A demo that then blocks does not affect it, which is why
    this waits for the echo rather than for the prompt to come back.
    """
    if via == "keyboard":
        qtype(qmp, cmd + "\n")
    else:
        ser.send(cmd + "\r")
    ok, _ = ser.wait(cmd + "\n", ceiling)
    if not ok:
        print(f"the shell never echoed {cmd!r} - the line was not submitted, so "
              f"the picture would be of a frame no command ran in", file=sys.stderr)
        sys.exit(1)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-k", "--keys", action="append", default=[], metavar="CMD",
                    help="a shell command to type and submit before the shot, "
                         "e.g. -k windows. Repeatable, typed in order. This is "
                         "a COMMAND NAME, not a keypress - see the module "
                         "docstring for why that distinction cost a gate.")
    ap.add_argument("--via", choices=("serial", "keyboard"), default="serial",
                    help="which wire to type on. serial is the default and is "
                         "measured to reach the compositor; keyboard drives the "
                         "emulated PS/2 or USB HID keyboard through QMP, which "
                         "is a different assertion - use it when the input "
                         "stack rather than the shell is what is under test.")
    ap.add_argument("--cmd-timeout", type=float, default=30,
                    help="ceiling on the shell echoing a typed line back")
    ap.add_argument("-o", "--out", default="shot", help="output name (no extension)")
    ap.add_argument("--crop", help="x,y,w,h in screen pixels")
    ap.add_argument("--zoom", type=int, default=1, help="nearest-neighbour magnify")
    ap.add_argument("--settle", type=float, default=1.0,
                    help="seconds to let the screen settle after the keys")
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--gfxmode",
                    help="force GRUB's gfxmode, e.g. 2560x1440,auto. The kernel "
                         "only re-modesets itself below 1900 wide, so this is "
                         "the way to hand it a bigger panel than it would pick.")
    ap.add_argument("--src",
                    help="build from this .zl instead of kernel.zl - for booting "
                         "a variant without editing the tracked source")
    args = ap.parse_args()

    crop = tuple(int(v) for v in args.crop.split(",")) if args.crop else None
    os.makedirs(SHOTS, exist_ok=True)

    if args.gfxmode:
        os.environ["ZLOS_GFXMODE"] = args.gfxmode
    if args.src:
        os.environ["ZLOS_SRC"] = os.path.abspath(args.src)
    build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-shot-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        ok, log = ser.wait("ready.", args.boot_timeout)
        if not ok:
            print("never booted. serial so far:\n" + log[-2000:]); sys.exit(1)
        print(log.strip())
        ok, _ = ser.wait(PROMPT, 60)
        if not ok:
            print("booted but no prompt"); sys.exit(1)
        for cmd in args.keys:
            type_command(ser, qmp, cmd, args.via, args.cmd_timeout)
        ser.drain(args.settle)

        ppm = os.path.join(tmp, "shot.ppm")
        qmp.screendump(ppm)
        png = os.path.join(SHOTS, args.out + ".png")
        size = to_png(ppm, png, crop, args.zoom)
        print(f"wrote {png}  {size[0]}x{size[1]}")
    finally:
        proc.kill()
        proc.wait()


if __name__ == "__main__":
    main()
