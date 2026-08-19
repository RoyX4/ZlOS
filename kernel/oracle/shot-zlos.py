#!/usr/bin/env python3
"""shot-zlos.py - photograph the REAL zlOS desktop at the reference size.

This is the zlOS half of the fidelity oracle. The other half renders
kernel/refrender/out/reference-1280x800.png from the mockup; diff-regions.py
scores this against that, per region.

  ./shot-zlos.py                             the boot desktop
  ./shot-zlos.py --app files --shot files       ...with Files opened
  ./shot-zlos.py --app Calculator --shot calc   ...an app only the catalog has
  ./shot-zlos.py --list-apps                 what --app accepts, and how each opens
  ./shot-zlos.py --width 1920 --height 1200  a different target size

Output: kernel/oracle/out/zlos-<name>.png


WHY QEMU AND NOT hosttest/wmshot
--------------------------------
wmshot renders the compositor from Linux userspace in milliseconds and needs no
boot, which makes it the obvious choice right up until you read it. Its apps
are FOUR HAND-WRITTEN C FUNCTIONS in wmshot.c (APP_SHELL, APP_MONITOR,
APP_ABOUT, APP_FILES), and its wallpaper, header and dock are its own
desk_draw() - about forty lines of C that exist in that file and nowhere else.
The real desktop and its twenty-odd apps are in kernel/kernel.zl. So wmshot
renders A desktop, not THE desktop: scoring it against the mockup would measure
a test fixture nobody ships and publish a fidelity number for pixels no user
will ever see.

It stays the right tool for compositor geometry (wmtest asserts, wmshot shows).
It is the wrong tool for fidelity, and the difference is not one of speed.


RESOLUTION - MEASURED, NOT ASSUMED
----------------------------------
zlOS does not boot at 1280x800 and cannot be made to by asking the bootloader.
See zlosboot.reach_size() for the whole chain; the short version is that
kernel.zl re-modesets anything under 1900 wide up to 1920x1200 with its own
driver, and the shell's `mode` command is the only runtime lever back down.
This script measures the screen and toggles until it is at the requested size
or can prove it cannot get there. --width/--height are parameters, so the day
that ladder learns a new mode nothing here needs an edit.


WAITING
-------
Nothing here sleeps for a boot or for a command; every wait polls the serial
log for a marker the guest itself printed. The one timed thing is --settle,
which decides nothing: it lets a frame finish before the picture is taken.
"""
import argparse, os, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from zlosboot import (Machine, OUT, WORD_APPS, catalog_apps,  # noqa: E402
                      open_app)


def main():
    cat = catalog_apps()
    ap = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="--app takes a shell word (" + ", ".join(sorted(WORD_APPS))
               + ") or a catalog app name (see --list-apps)")
    ap.add_argument("--app", help="open this app before the shot")
    ap.add_argument("--shot", default="desktop", help="output name (no extension)")
    ap.add_argument("--width", type=int, default=1280)
    ap.add_argument("--height", type=int, default=800)
    ap.add_argument("--settle", type=float, default=2.0,
                    help="seconds to let the frame finish before the picture. "
                         "This decides nothing; it is not a boot timeout.")
    ap.add_argument("--boot-timeout", type=float, default=300.0)
    ap.add_argument("--cmd-timeout", type=float, default=60.0)
    ap.add_argument("--how", choices=("src", "toggle"), default="src",
                    help="how to reach the target size. src (default) boots a "
                         "variant kernel.zl whose modeset ladder asks for it "
                         "directly; toggle boots normally and types `mode`, "
                         "which changes the framebuffer but leaves the desktop "
                         "laid out for 1920x1200 - see zlosboot.variant_source")
    ap.add_argument("--keep-ppm", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--list-apps", action="store_true")
    args = ap.parse_args()

    if args.list_apps:
        print("shell words (typed at the prompt, no pointer involved):")
        for w in sorted(WORD_APPS):
            print(f"  {w:<22} open_app code {WORD_APPS[w]}")
        print("\ncatalog apps (Start -> All Applications -> tile), from "
              "apps_registry.zl:")
        for n in sorted(cat, key=lambda k: cat[k]):
            print(f"  {n:<22} tile {cat[n]}")
        return 0

    if args.app and args.app not in WORD_APPS and args.app not in cat:
        near = [k for k in list(WORD_APPS) + list(cat)
                if args.app.lower() in k.lower()]
        raise SystemExit(f"unknown --app {args.app!r}"
                         + (f" - did you mean {near}?" if near else "")
                         + "\nrun --list-apps")

    os.makedirs(OUT, exist_ok=True)
    with Machine(args.width, args.height, do_build=not args.no_build,
                 boot_timeout=args.boot_timeout,
                 cmd_timeout=args.cmd_timeout, how=args.how) as m:
        if args.app:
            open_app(m.ser, m.qmp, args.app, m.w, m.h, args.cmd_timeout, cat)

        m.ser.drain(args.settle)
        ppm = os.path.join(m.tmp, "shot.ppm")
        if not m.qmp.screendump(ppm):
            print("screendump failed", file=sys.stderr)
            return 1
        from PIL import Image
        png = os.path.join(OUT, f"zlos-{args.shot}.png")
        with Image.open(ppm) as im:
            im = im.convert("RGB")
            if im.size != (args.width, args.height):
                print(f"FAIL: the frame is {im.size[0]}x{im.size[1]}, not "
                      f"{args.width}x{args.height} - refusing to write a "
                      f"picture the region map cannot be applied to",
                      file=sys.stderr)
                return 1
            im.save(png)
            if args.keep_ppm:
                import shutil
                shutil.copy(ppm, os.path.join(OUT, f"zlos-{args.shot}.ppm"))
            colours = len(im.getcolors(maxcolors=1 << 20) or [])
        print(f"wrote {png}  {args.width}x{args.height}  {colours} distinct colours")
        return 0


if __name__ == "__main__":
    sys.exit(main())
