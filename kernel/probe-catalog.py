#!/usr/bin/env python3
"""probe-catalog.py - is "All Applications" reachable with the POINTER?

47 of zlOS's 53 apps exist only behind this window, so "can a person open it"
is not a detail - it is whether five sixths of the desktop is software or
decoration. A screenshot cannot tell a working catalog from a picture of one,
so this drives the real pointer through the real chrome and asks the COMPOSITOR
what happened, not the pixels alone.

  1 TERMINAL  the dock's first tile is a launcher, not the start button
  2 MENU      the topbar's Activities corner still opens the menu
  3 GRID      the dock's grid button opens "All Applications"
  4 MAZE      the formerly excluded Maze tile opens THAT app after scrolling

WHY 1 IS FIRST, and why it is a test at all. desk_click() carried a guard from
the full-width bar - `if cx >= dock_start_x() { if cx < dock_start_x() + 42u {
open_menu() } }` - and dock_start_x() returns dock_x0(), the left edge of TILE
0. 42 design units is wider than a tile plus its gap, so the Terminal tile and
the first four units of the Browser opened the start menu and never reached
dock_launch(). Nothing caught it because the menu opening looks like something
working.

EVERY COORDINATE HERE IS PARSED OUT OF kernel.zl, not transcribed. The previous
version of this file hardcoded `DOCK_H = 64` against a 52-unit dock and
`ui = 2 if W >= 1400 else 1` against fb.c's real formula, and clicked a start
button that had been replaced by a floating island - three independent ways to
land on the wallpaper and report a broken feature that worked.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "oracle"))

import importlib.util
_spec = importlib.util.spec_from_file_location(
    "zlosboot", os.path.join(HERE, "oracle", "zlosboot.py"))
zb = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(zb)

SHOTS = os.path.join(HERE, "shots")
W, H = 1280, 800


def differs(a, b, box):
    x0, y0, x1, y1 = box
    return int((a[y0:y1:2, x0:x1:2] != b[y0:y1:2, x0:x1:2]).any(axis=2).sum())


def main():
    os.makedirs(SHOTS, exist_ok=True)
    fails = []
    with zb.Machine(W, H, how="native") as m:
        u = zb.guest_ui(m.w)
        g = zb.dock_geometry(m.w, m.h)
        g["tile0"] = g["slots"][0]
        settle = m.ser.drain
        print(f"ui {u}x  dock top {g['dock_y']}  tile0 {g['tile0']}  "
              f"grid {g['grid']}")

        # The start menu is a window at (10u, dock_y - mh - 8u), 210u wide.
        # Its exact height needs ui_metric(), which lives in C - so the box is
        # the whole strip it can occupy. Anything opening there is the menu.
        menu_box = (10 * u, max(0, g["dock_y"] - 420 * u),
                    min(m.w, 220 * u), g["dock_y"] - 8 * u)

        base = zb.grab(m.qmp, m.tmp, "base")

        # ---- 1. the dock's first tile is a LAUNCHER ------------------------
        zb.click(m.qmp, g["tile0"][0], g["tile0"][1], m.w, m.h, settle)
        settle(1.2)
        after = zb.grab(m.qmp, m.tmp, "tile0")
        n = differs(base, after, menu_box)
        ok = n < 500
        print(f"  1 the Terminal tile does NOT open the menu  {n:6d} px  "
              f"{'ok' if ok else 'FAIL'}")
        if not ok:
            fails.append("terminal-tile-opens-menu")

        # ---- 2. ...and the menu is still reachable -------------------------
        zb.click(m.qmp, g["topbar_corner"][0], g["topbar_corner"][1],
                 m.w, m.h, settle)
        settle(1.2)
        menu = zb.grab(m.qmp, m.tmp, "menu")
        n = differs(after, menu, menu_box)
        ok = n > 2000
        print(f"  2 the topbar corner opens the menu         {n:6d} px  "
              f"{'ok' if ok else 'FAIL'}")
        if not ok:
            fails.append("menu")
        # open_menu() toggles, so this closes it again and leaves a clean
        # desktop for the catalog test rather than a modal over it.
        zb.click(m.qmp, g["topbar_corner"][0], g["topbar_corner"][1],
                 m.w, m.h, settle)
        settle(1.2)

        # ---- 3. the grid button opens the catalog --------------------------
        # THE COMPOSITOR'S OWN REPORT, not a pixel delta: a delta cannot tell
        # "the catalog opened" from "a tooltip appeared", and reg_open() prints
        # `wm: win N title ... client ...` through wm_report on every window it
        # actually opens. It stays silent on the raise-and-focus path, which is
        # precisely the failure this probe exists to catch.
        before = zb.win_count(m.ser.all)
        zb.click(m.qmp, g["grid"][0], g["grid"][1], m.w, m.h, settle)
        settle(2.0)
        cat = zb.grab(m.qmp, m.tmp, "catalog")
        rows = zb.WIN_REPORT.findall(m.ser.all)
        ok = zb.win_count(m.ser.all) > before
        print(f"  3 the dock's grid button opens the catalog  "
              f"{zb.win_count(m.ser.all) - before:5d} win  "
              f"{'ok' if ok else 'FAIL'}")
        if not ok:
            fails.append("grid-button")
            print("      reg_open(APP_CATALOG) reported no window. Either the "
                  "click missed the button, or the id it opens is already on "
                  "screen under another app - which is a raise, not an open.")

        # ---- 4. the formerly excluded Maze tile launches Maze --------------
        # This is deliberately not tile zero. The old static checker printed
        # `Maze exists NO` but exited zero because a blank ID 14 kept the dense
        # count at 47. Parsing the source-owned dense index, scrolling to that
        # exact tile and requiring a new compositor window closes the runtime
        # half of that false green.
        if ok:
            _, cx, cy, cw, ch = (int(v) for v in rows[-1])
            cols = max(1, cw // (zb.CAT_TILE_W * u))
            vis = max(1, (ch - zb.CAT_HEADER * u) // (zb.CAT_TILE_H * u))
            idx = zb.catalog_apps()["Maze"]
            col, row = idx % cols, idx // cols
            scroll = max(0, row - vis + 1)
            if scroll:
                zb.at(m.qmp, cx + cw // 2, cy + ch // 2, m.w, m.h)
                settle(0.3)
                zb.wheel(m.qmp, -scroll)
                settle(1.0)
            tx = cx + col * zb.CAT_TILE_W * u + zb.CAT_TILE_W * u // 2
            ty = (cy + zb.CAT_HEADER * u
                  + (row - scroll) * zb.CAT_TILE_H * u
                  + zb.CAT_TILE_H * u // 2)
            before = zb.win_count(m.ser.all)
            zb.click(m.qmp, tx, ty, m.w, m.h, settle)
            settle(2.0)
            shot = zb.grab(m.qmp, m.tmp, "launched")
            got = zb.win_count(m.ser.all) - before
            print(f"  4 Maze tile {idx} ({cols} cols, scroll {scroll}) opens "
                  f"{got:5d} win  {'ok' if got > 0 else 'FAIL'}")
            if got <= 0:
                fails.append("maze-tile-launch")
            from PIL import Image
            Image.fromarray(shot).save(os.path.join(SHOTS, "catalog-launched.png"))
        from PIL import Image
        Image.fromarray(cat).save(os.path.join(SHOTS, "catalog-open.png"))
        Image.fromarray(menu).save(os.path.join(SHOTS, "catalog-menu.png"))

    print()
    print("all good" if not fails else "FAILED: " + ", ".join(fails))
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
