#!/usr/bin/env python3
"""probe-drag.py - can a window actually be dragged, at this resolution?

The gate for desktop-TODO 0a is not "the desktop draws at 2560x1440", it is
"dragging still works there". Those are different questions, because dragging
goes through a SECOND pair of fixed buffers - bg_buf and sp_buf - each of which
had its own compile-time pixel ceiling, and bg_buf's was 1920x1200. Above it
bg_ok went to 0 and every drag silently became a no-op.

zlOS reads the pointer from the PS/2 mouse on IRQ12 only (see probe-mouse.py),
so this sends RELATIVE events. Absolute ones go to the usb-tablet, which zlOS
has no driver for.

  ./probe-drag.py                                  drag at whatever mode boots
  ./probe-drag.py --src /tmp/kernel-2560.zl        ...at 2560x1440

It homes the pointer into the top-left corner first, because there is no way to
ask the guest where the pointer currently is - the kernel clamps to the screen,
so shoving it hard enough at the corner puts it somewhere known.
"""
import argparse, os, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, ppm_sample, frame_delta, qemu_argv, build, PROMPT

HERE = os.path.dirname(os.path.abspath(__file__))
SHOTS = os.path.join(HERE, "shots")


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
    ap.add_argument("--grab", default="2110,120", help="x,y to press at")
    ap.add_argument("--drop", default="1100,760", help="x,y to release at")
    ap.add_argument("--boot-timeout", type=float, default=240)
    args = ap.parse_args()

    gx, gy = (int(v) for v in args.grab.split(","))
    dx, dy = (int(v) for v in args.drop.split(","))
    os.makedirs(SHOTS, exist_ok=True)

    if args.gfxmode:
        os.environ["ZLOS_GFXMODE"] = args.gfxmode
    if args.src:
        os.environ["ZLOS_SRC"] = os.path.abspath(args.src)
    build(False)

    tmp = tempfile.mkdtemp(prefix="zlos-drag-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    # tablet=False is not optional here. With the usb-tablet attached QEMU
    # keeps every pointer event for itself and zlOS's PS/2 mouse never sees
    # one - measured in probe-mouse-sync.py. A drag test against a dead
    # pointer would report "dragging is broken" for the wrong reason.
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path, tablet=False),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        ok, log = ser.wait("ready.", args.boot_timeout)
        if not ok:
            print("never booted:\n" + log[-2000:]); sys.exit(1)
        for line in log.splitlines():
            if line.strip().startswith("fb:") or "lost:" in line:
                print(line.strip())
        ser.wait(PROMPT, 60)
        ser.drain(1.0)

        # home: the kernel clamps the pointer to the screen, so overshooting
        # the top-left corner leaves it at a position we know
        for _ in range(50):
            rel(qmp, -200, -200)
        ser.drain(0.6)

        step = 80
        for _ in range(max(1, gx // step)):
            rel(qmp, step, 0)
        for _ in range(max(1, gy // step)):
            rel(qmp, 0, step)
        rel(qmp, gx % step, gy % step)
        ser.drain(1.2)

        before = os.path.join(tmp, "before.ppm")
        qmp.screendump(before)

        btn(qmp, True)
        ser.drain(0.4)
        # walk there, so the kernel's drag loop sees intermediate positions the
        # way it would from a hand
        steps = 24
        for i in range(steps):
            rel(qmp, (dx - gx) // steps, (dy - gy) // steps)
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
