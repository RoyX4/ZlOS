#!/usr/bin/env python3
"""probe-drag.py - can a window actually be dragged, at this resolution?

The gate for desktop-TODO 0a is not "the desktop draws at 2560x1440", it is
"dragging still works there". Those are different questions, because dragging
goes through a SECOND pair of fixed buffers - bg_buf and sp_buf - each of which
had its own compile-time pixel ceiling, and bg_buf's was 1920x1200. Above it
bg_ok went to 0 and every drag silently became a no-op.

zlOS reads TWO pointers and prefers the USB one: xhci.c drives the usb-tablet
(absolute) and idt.c drives the PS/2 mouse (relative), and the mouse_x builtin
takes the tablet when it is there. So the event type has to match the machine,
and this defaults to the machine try.sh actually gives - tablet attached,
ABSOLUTE events. Getting that backwards is not a theoretical hazard: it made an
earlier run of this script report "dragging is a no-op" when the drag was fine
and the harness was sending relative events into an absolute device.

  ./probe-drag.py                                  drag at whatever mode boots
  ./probe-drag.py --src /tmp/kernel-2560.zl        ...at 2560x1440
  ./probe-drag.py --no-tablet                      the PS/2 path (the laptop's
                                                   TrackPoint is PS/2, so this
                                                   is not a synthetic case)

Absolute mode needs no homing: the position IS the message. Relative mode homes
into the top-left corner first, because there is no way to ask the guest where
the pointer is and the kernel clamps to the screen, so overshooting the corner
puts it somewhere known.
"""
import argparse, os, re, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, ppm_sample, frame_delta, qemu_argv, build, PROMPT

HERE = os.path.dirname(os.path.abspath(__file__))
SHOTS = os.path.join(HERE, "shots")


def absolute(qmp, x, y, w, h):
    """put the pointer AT x,y. The HID logical range is 0..32767 across the
    whole screen, so this is a proportion, not a pixel count."""
    qmp.cmd("input-send-event", events=[
        {"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / (w - 1))}},
        {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / (h - 1))}}])


def rel(qmp, dx, dy):
    ev = []
    if dx:
        ev.append({"type": "rel", "data": {"axis": "x", "value": dx}})
    if dy:
        ev.append({"type": "rel", "data": {"axis": "y", "value": dy}})
    if ev:
        qmp.cmd("input-send-event", events=ev)


def btn(qmp, down):
    qmp.cmd("input-send-event",
            events=[{"type": "btn", "data": {"down": down, "button": "left"}}])


def png(ppm, path, scale=None):
    from PIL import Image
    img = Image.open(ppm)
    if scale:
        img = img.resize((scale, int(scale * img.height / img.width)), Image.LANCZOS)
    img.save(path)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", help="build from this .zl instead of kernel.zl")
    ap.add_argument("--gfxmode", help="force GRUB's gfxmode")
    ap.add_argument("-o", "--out", default="drag")
    # where to grab, and where to drop. Defaults are the System Monitor's title
    # bar at 2560x1440: mon_x = 2560 - (MON_W + 16) * ui() = 1960, and
    # mon_y = (DESK_TOP + 8) * ui() = 96, with a 56px title bar above it.
    ap.add_argument("--grab", default="", help="x,y to press at "
                    "(default: the title bar the kernel reports)")
    ap.add_argument("--drop", default="1100,760", help="x,y to release at")
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--no-tablet", action="store_true",
                    help="drop the usb-tablet and drive the PS/2 mouse with "
                         "relative events instead")
    args = ap.parse_args()

    # 0,0 means "not given" - filled in from the kernel's own report below.
    gx, gy = (int(v) for v in args.grab.split(",")) if args.grab else (0, 0)
    dx, dy = (int(v) for v in args.drop.split(","))
    os.makedirs(SHOTS, exist_ok=True)

    if args.gfxmode:
        os.environ["ZLOS_GFXMODE"] = args.gfxmode
    if args.src:
        os.environ["ZLOS_SRC"] = os.path.abspath(args.src)
    build(False)

    tmp = tempfile.mkdtemp(prefix="zlos-drag-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    # Default to the machine try.sh gives - tablet attached - and send the
    # ABSOLUTE events that machine expects. An earlier version forced the
    # tablet off because relative events were going nowhere, which passed for
    # the wrong reason and hid that zlOS drives the tablet perfectly well.
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path,
                                      tablet=not args.no_tablet),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        ok, log = ser.wait("ready.", args.boot_timeout)
        if not ok:
            print("never booted:\n" + log[-2000:]); sys.exit(1)
        for line in log.splitlines():
            if line.strip().startswith("fb:") or "lost:" in line:
                print(line.strip())
        # WHERE TO PRESS. This used to be the literal default 2110,120, which
        # is off the right edge of a 1920-wide screen - so the press landed on
        # nothing and the gate reported "NOTHING MOVED - dragging is a no-op"
        # for a drag that worked perfectly. The compositor prints every
        # window's title bar on the serial log; aim at one of those instead of
        # at a number that was true for one resolution on one day.
        # `compositor:` is normally printed before `ready.`. The first wait
        # already consumed it, so waiting for it a second time burns the whole
        # timeout and then falsely reports that no rectangles were printed.
        ok = "compositor:" in log
        more = ""
        if not ok:
            ok, more = ser.wait("compositor:", args.boot_timeout)
            log += more
        if ok:
            ser.drain(1.0)
            log += ser.buf; ser.buf = ""
        bars = re.findall(r"wm: win (\d+) title (\d+),(\d+) (\d+)x(\d+)", log)
        if not args.grab:
            if not bars:
                print("the compositor reported no window rects, and --grab was "
                      "not given - refusing to press at a guess"); sys.exit(1)
            # The LAST window opened is on top, so its title bar is the one a
            # press actually reaches. Aim at the middle of it, left of the
            # close box.
            _, bx, by, bw, bh = (int(v) for v in bars[-1])
            gx, gy = bx + bw // 3, by + bh // 2
            print(f"  grabbing the title bar of window {bars[-1][0]} at {gx},{gy}")
        ser.drain(1.0)

        # one screendump first, purely to learn the screen size - the absolute
        # range is a proportion of it and guessing would put the press in the
        # wrong place at any mode but the one the defaults were written for
        probe = os.path.join(tmp, "probe.ppm")
        qmp.screendump(probe)
        sw, sh, _ = ppm_sample(probe)

        if args.no_tablet:
            # relative: home into the corner, then walk out to the grab point
            for _ in range(50):
                rel(qmp, -200, -200)
            ser.drain(0.6)
            step = 80
            for _ in range(max(1, gx // step)):
                rel(qmp, step, 0)
            for _ in range(max(1, gy // step)):
                rel(qmp, 0, step)
            rel(qmp, gx % step, gy % step)
        else:
            absolute(qmp, gx, gy, sw, sh)
        ser.drain(1.2)

        before = os.path.join(tmp, "before.ppm")
        qmp.screendump(before)

        btn(qmp, True)
        ser.drain(0.4)
        # walk there, so the kernel's drag loop sees intermediate positions the
        # way it would from a hand
        steps = 24
        for i in range(1, steps + 1):
            if args.no_tablet:
                rel(qmp, (dx - gx) // steps, (dy - gy) // steps)
            else:
                absolute(qmp, gx + (dx - gx) * i // steps,
                              gy + (dy - gy) * i // steps, sw, sh)
            ser.drain(0.08)
        ser.drain(0.6)
        btn(qmp, False)
        ser.drain(1.2)

        after = os.path.join(tmp, "after.ppm")
        qmp.screendump(after)

        b, a = ppm_sample(before), ppm_sample(after)
        delta = frame_delta(b, a)
        png(before, os.path.join(SHOTS, args.out + "-before.png"), 1024)
        png(after, os.path.join(SHOTS, args.out + "-after.png"), 1024)
        print(f"  screen {b[0]}x{b[1]}   changed by the drag: {delta*100:.2f}%")
        print("  VERDICT:", "the window moved" if delta > 0.01 else
              "NOTHING MOVED - dragging is a no-op at this mode")
        print(f"  shots/{args.out}-before.png  shots/{args.out}-after.png")
        sys.exit(0 if delta > 0.01 else 2)
    finally:
        proc.kill()
        proc.wait()


if __name__ == "__main__":
    main()
