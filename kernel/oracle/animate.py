#!/usr/bin/env python3
"""animate.py - did anything on screen actually MOVE?

  ./animate.py --do drag                    drag a window, watch the pixels
  ./animate.py --do menu                    open the start menu
  ./animate.py --do hover-dock              sweep the pointer along the dock
  ./animate.py --do type                    type at the shell
  ./animate.py --do app --app anim          open an app and let it animate
  ./animate.py --do idle                    the control: nothing is driven
  ./animate.py --do drag --frames 12 --json

Output: kernel/oracle/out/anim-<do>-NN.png (with --save) and a report.


WHY THIS EXISTS
---------------
A previous run of this desktop had 9 of 14 event handlers updating their state
and never calling wm_dmg(win). The code compiled clean. Every gate was green.
Nothing repainted. From outside, "the model changed and the screen did not" and
"nothing happened" are the same picture, and a build that is green proves
neither one.

So this is deliberately NOT an assertion about correctness. It is a difference
engine: it drives one interaction, takes N frames, and reports for each
consecutive pair how many pixels changed, where their bounding box is, and
which named regions they fall in. A handler that updates state and forgets its
damage call produces the one signature nothing else in this repo looks for -
ZERO changed pixels during an interaction that plainly should move something.

The verdict is about MOTION, not about beauty. It is red only when nothing
moved at all, or when everything moved (a full-screen repaint per frame, which
is the other failure: a compositor that has given up on damage tracking and is
redrawing the world 60 times a second).


WHAT COUNTS AS A FRAME
----------------------
A QMP screendump of the guest's real framebuffer, N of them in a row, with the
interaction driven between them. The interval is a deliberate duration and
decides nothing about pass or fail - it is how long to let an animation run,
not a guess at how slow the host is. Booting and command submission still poll
for markers the guest printed.
"""
import argparse, json, os, sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from zlosboot import (Machine, OUT, at, click, catalog_apps,   # noqa: E402
                      grab, guest_ui, open_app, type_line, DOCK_H)

HERE = os.path.dirname(os.path.abspath(__file__))
GRID_W, GRID_H = 32, 20        # the ASCII change map: 40x40 px cells at 1280x800


# ---- the interactions -------------------------------------------------------
# Each is a generator: it yields once per frame boundary, having driven a
# little more of the interaction. That shape keeps "what to do" and "when to
# photograph" in one place instead of two lists that can drift apart.
def do_idle(m, frames):
    """The control. Nothing is driven, so anything that changes here is the
    guest moving on its own - a clock, a caret, an animation left running. Read
    the other interactions' numbers against this one, not against zero."""
    for _ in range(frames):
        m.ser.drain(0.35)
        yield


def do_drag(m, frames):
    """Grab the topmost window's title bar and walk it across the desktop.

    The grab point is the compositor's OWN reported title rect, never a
    constant: the literal 2110,120 default in probe-drag.py was off the right
    edge of a 1920-wide screen, so the press landed on nothing and the gate
    reported "dragging is a no-op" for a drag that worked perfectly.
    """
    import re
    m.ser.drain(1.0)
    log = m.ser.buf
    bars = re.findall(r"wm: win (\d+) title (\d+),(\d+) (\d+)x(\d+)", log)
    if not bars:
        raise SystemExit("the compositor reported no window title rects, so "
                         "there is nothing to aim at - refusing to press at a "
                         "guess")
    _, bx, by, bw, bh = (int(v) for v in bars[-1])
    gx, gy = bx + bw // 3, by + bh // 2
    dx, dy = min(m.w - 60, gx + 260), min(m.h - 80, gy + 180)
    print(f"  dragging window {bars[-1][0]} from {gx},{gy} to {dx},{dy}")

    at(m.qmp, gx, gy, m.w, m.h)
    m.ser.drain(0.5)
    at(m.qmp, gx, gy, m.w, m.h, btn=True)
    m.ser.drain(0.3)
    yield
    for i in range(1, frames):
        at(m.qmp, gx + (dx - gx) * i // (frames - 1),
                  gy + (dy - gy) * i // (frames - 1), m.w, m.h)
        m.ser.drain(0.35)
        yield
    at(m.qmp, dx, dy, m.w, m.h, btn=False)


def do_menu(m, frames):
    """Click the start button, then move the pointer down the open menu.

    Two motions in one: the menu appearing (a large change) and the hover row
    following the pointer (a small one). A build where the menu opens but the
    hover never repaints shows up as frame 1 large, frames 2..N zero.
    """
    u = guest_ui(m.w)
    dock_y = m.h - DOCK_H * u
    click(m.qmp, 40 * u, dock_y + 32 * u, m.w, m.h, m.ser.drain)
    m.ser.drain(0.6)
    yield
    for i in range(1, frames):
        at(m.qmp, 110 * u, dock_y - 40 * u - i * 26 * u, m.w, m.h)
        m.ser.drain(0.35)
        yield


def do_hover_dock(m, frames):
    """Sweep the pointer along the dock. Every tile should light as it passes.

    This is the exact shape of the bug in the docstring: dock_hover is state,
    and a handler that sets it without a wm_dmg call changes nothing on screen
    while every unit test still passes.
    """
    u = guest_ui(m.w)
    y = m.h - DOCK_H * u // 2
    for i in range(frames):
        at(m.qmp, 40 * u + i * 56 * u, y, m.w, m.h)
        m.ser.drain(0.35)
        yield


def do_type(m, frames):
    """Type at the shell. The caret and the echoed characters should move."""
    for i in range(frames):
        m.ser.send("abcdefghij"[i % 10])
        m.ser.drain(0.35)
        yield
    m.ser.send("\x15\r")          # ^U clears the line, Enter submits nothing


def do_app(m, frames, app):
    """Open an app and watch it. `--do app --app anim` is the positive control:
    that demo animates by itself, so a run where NOTHING changes afterwards is
    unambiguous."""
    open_app(m.ser, m.qmp, app, m.w, m.h, 60.0)
    for _ in range(frames):
        m.ser.drain(0.35)
        yield


DOERS = {"idle": do_idle, "drag": do_drag, "menu": do_menu,
         "hover-dock": do_hover_dock, "type": do_type, "app": do_app}


# ---- measuring --------------------------------------------------------------
def changed(a, b):
    """(count, bbox, per-cell counts) of pixels that differ between two frames.

    Exact inequality, not a threshold. These are two dumps of the same
    framebuffer, not two photographs: there is no sensor noise to filter, and a
    threshold here would hide precisely the one-shade hover highlight this tool
    exists to find.
    """
    d = np.any(a != b, axis=2)
    n = int(d.sum())
    if n == 0:
        return 0, None, np.zeros((GRID_H, GRID_W), dtype=np.int64)
    ys, xs = np.nonzero(d)
    bbox = (int(xs.min()), int(ys.min()),
            int(xs.max() - xs.min() + 1), int(ys.max() - ys.min() + 1))
    h, w = d.shape
    cell = np.zeros((GRID_H, GRID_W), dtype=np.int64)
    yb = np.linspace(0, h, GRID_H + 1).astype(int)
    xb = np.linspace(0, w, GRID_W + 1).astype(int)
    for j in range(GRID_H):
        for i in range(GRID_W):
            cell[j, i] = int(d[yb[j]:yb[j + 1], xb[i]:xb[i + 1]].sum())
    return n, bbox, cell


def ascii_map(cell):
    """The change map, as text, because a picture of a diff needs a viewer and
    this has to be readable in a CI log."""
    ramp = " .:-=+*#%@"
    top = cell.max()
    if top == 0:
        return ["(nothing changed anywhere)"]
    out = []
    for row in cell:
        out.append("".join(ramp[min(len(ramp) - 1,
                                    int(v * (len(ramp) - 1) / top))] for v in row))
    return out


def region_hits(regions, a, b):
    """Which named regions changed, and by how much of their own area."""
    hits = []
    for r in regions:
        x, y, w, h = r["rect"]
        ca, cb = a[y:y + h, x:x + w], b[y:y + h, x:x + w]
        if ca.size == 0 or ca.shape != cb.shape:
            continue
        n = int(np.any(ca != cb, axis=2).sum())
        if n:
            hits.append((r["id"], n, n / float(w * h)))
    hits.sort(key=lambda t: -t[1])
    return hits


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--do", default="drag", choices=sorted(DOERS))
    ap.add_argument("--app", default="anim",
                    help="which app, for --do app")
    ap.add_argument("--frames", type=int, default=8)
    ap.add_argument("--width", type=int, default=1280)
    ap.add_argument("--height", type=int, default=800)
    ap.add_argument("--how", choices=("src", "toggle"), default="src")
    ap.add_argument("--regions", default=os.path.join(HERE, "regions.json"))
    ap.add_argument("--save", action="store_true", help="write every frame as a PNG")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--no-build", action="store_true")
    ap.add_argument("--boot-timeout", type=float, default=300.0)
    args = ap.parse_args()

    if args.frames < 2:
        raise SystemExit("--frames must be at least 2 - one pair is the minimum "
                         "unit of 'did anything change'")
    regions = []
    if os.path.exists(args.regions):
        doc = json.load(open(args.regions))
        if (doc["screen"]["w"], doc["screen"]["h"]) == (args.width, args.height):
            regions = doc["regions"]

    os.makedirs(OUT, exist_ok=True)
    with Machine(args.width, args.height, do_build=not args.no_build,
                 boot_timeout=args.boot_timeout, how=args.how) as m:
        if args.do == "app":
            gen = DOERS[args.do](m, args.frames, args.app)
        else:
            gen = DOERS[args.do](m, args.frames)

        frames = []
        for i, _ in enumerate(gen):
            f = grab(m.qmp, m.tmp, f"f{i}")
            frames.append(f)
            if args.save:
                from PIL import Image
                Image.fromarray(f).save(
                    os.path.join(OUT, f"anim-{args.do}-{i:02d}.png"))
        if len(frames) < 2:
            raise SystemExit("fewer than two frames were captured")

    total = args.width * args.height
    steps = []
    for i in range(1, len(frames)):
        n, bbox, cell = changed(frames[i - 1], frames[i])
        steps.append(dict(frame=i, changed=n, pct=round(100.0 * n / total, 4),
                          bbox=list(bbox) if bbox else None,
                          regions=[dict(id=rid, px=px, frac=round(fr, 4))
                                   for rid, px, fr in
                                   region_hits(regions, frames[i - 1], frames[i])[:6]],
                          _cell=cell))

    moved = sum(s["changed"] for s in steps)
    dead = [s["frame"] for s in steps if s["changed"] == 0]
    flooding = [s["frame"] for s in steps if s["pct"] > 90.0]

    if args.json:
        json.dump(dict(interaction=args.do, app=args.app if args.do == "app" else None,
                       screen=dict(w=args.width, h=args.height),
                       frames=len(frames), total_changed=moved,
                       dead_frames=dead, flooding_frames=flooding,
                       verdict=("NOTHING MOVED" if moved == 0 else
                                "FULL REPAINT EVERY FRAME" if len(flooding) == len(steps)
                                else "moved"),
                       steps=[{k: v for k, v in s.items() if k != "_cell"}
                              for s in steps]),
                  sys.stdout, indent=1)
        print()
    else:
        print(f"\ninteraction: {args.do}"
              + (f" ({args.app})" if args.do == "app" else "")
              + f"   {len(frames)} frames at {args.width}x{args.height}\n")
        print(f"{'pair':<6}{'changed px':>12}{'% of screen':>13}  "
              f"{'bbox (x,y,w,h)':<24} regions")
        print("-" * 96)
        for s in steps:
            regs = ", ".join(f"{r['id']}({r['frac'] * 100:.0f}%)"
                             for r in s["regions"][:3]) or "-"
            print(f"{s['frame'] - 1}->{s['frame']:<3}{s['changed']:>12}"
                  f"{s['pct']:>12.3f}%  {str(s['bbox']):<24} {regs}")
        print("-" * 96)
        busiest = max(steps, key=lambda s: s["changed"])
        print(f"\nwhere the change was, on the busiest pair "
              f"({busiest['frame'] - 1}->{busiest['frame']}, "
              f"{busiest['changed']} px):")
        for line in ascii_map(busiest["_cell"]):
            print("   " + line)
        print()
        if moved == 0:
            print("VERDICT: NOTHING MOVED. Every frame is byte-identical to the")
            print("  one before it. If this interaction is supposed to change the")
            print("  screen, the handler is updating state without asking for a")
            print("  repaint - the wm_dmg() failure this tool exists for. A green")
            print("  build says nothing about it.")
        elif len(flooding) == len(steps):
            print("VERDICT: FULL REPAINT EVERY FRAME. Over 90% of the screen")
            print("  changed on every pair, which means damage tracking is not")
            print("  narrowing anything - correct output, wrong cost.")
        else:
            print(f"VERDICT: moved. {moved} pixels changed across "
                  f"{len(steps)} frame pairs"
                  + (f", but {len(dead)} pair(s) were identical: {dead}"
                     if dead else "."))
        if args.save:
            print(f"\nframes: {OUT}/anim-{args.do}-NN.png")

    return 1 if moved == 0 else 0


if __name__ == "__main__":
    sys.exit(main())
