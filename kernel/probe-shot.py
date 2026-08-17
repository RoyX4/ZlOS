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
  ./probe-shot.py -k w                   after pressing 'w'
  ./probe-shot.py -k v --settle 3        give an animation time to run
  ./probe-shot.py --crop 200,600,700,200 --zoom 3 -o dock
                                         a magnified crop, for judging edges

Output lands in kernel/shots/ as PNG. Nothing here waits a fixed wall-clock
time for a BOOT - it polls the serial log for the prompt, per the project rule
that a gate must never be timing-sensitive. --settle is different: it is how
long to let an animation run before photographing it, which is a deliberate
duration, not a guess at how slow the host is.
"""
import argparse, os, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-k", "--keys", default="",
                    help="characters to send over serial before the shot")
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
        for ch in args.keys:
            ser.send(ch)
            ser.drain(0.4)
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
