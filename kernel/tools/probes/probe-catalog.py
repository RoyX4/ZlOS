#!/usr/bin/env python3
"""probe-catalog.py - are the register, menu, and catalogue reachable?

The 47 registry apps exist behind this window, so "can a person open it" is not
a detail. A screenshot cannot tell a working catalogue from a picture of one,
so this drives the real input paths and asks the compositor what happened.

  1 TERMINAL  the register's first row raises Terminal, not Menu
  2 MENU      a clean Super tap opens and closes Menu
  3 ALL APPS  the register's final fixed row opens "All Applications"
  4 MAZE      the formerly excluded Maze tile opens THAT app after scrolling

Every shell coordinate here comes from zlosboot.py's parser of kernel.zl. The
catalogue tile coordinates come from the compositor's reported client rectangle.
The probe follows the current input contract: pointer for the rail, keyboard for
Menu, and pointer again inside All Applications.
"""
import os
import re
import sys

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
HERE = KERNEL_ROOT
ORACLE_DIR = os.path.join(KERNEL_ROOT, "tests", "oracle")
sys.path.insert(0, ORACLE_DIR)

import importlib.util
_spec = importlib.util.spec_from_file_location(
    "zlosboot", os.path.join(ORACLE_DIR, "zlosboot.py"))
zb = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(zb)

SHOTS = os.path.join(HERE, "shots")
W, H = 1280, 800


def lifecycle(log, event, app):
    return re.findall(
        rf"wm:lifecycle v=1 event={event} slot=\d+ app={app} generation=\d+ live=\d+",
        log,
    )


def main():
    os.makedirs(SHOTS, exist_ok=True)
    fails = []
    with zb.Machine(W, H, how="native") as m:
        u = zb.guest_ui(m.w)
        g = zb.rail_geometry(m.w, m.h)
        settle = m.ser.drain
        print(f"ui {u}x  rail {g['rail_w']}px  row0 {g['slots'][0]}  "
              f"all-apps {g['catalogue']}")

        # ---- 1. the first register row is Terminal -------------------------
        start = len(m.ser.all)
        zb.click(m.qmp, g["slots"][0][0], g["slots"][0][1], m.w, m.h, settle)
        ok = not lifecycle(m.ser.all[start:], "open", 4)
        print(f"  1 register row 01 does NOT open Menu          "
              f"{'ok' if ok else 'FAIL'}")
        if not ok:
            fails.append("terminal-row-opens-menu")

        # ---- 2. Menu is a Super-key surface --------------------------------
        start = len(m.ser.all)
        zb.tap_key(m.qmp, "meta_l", settle)
        menu = zb.grab(m.qmp, m.tmp, "menu")
        opened = lifecycle(m.ser.all[start:], "open", 4)
        ready = lifecycle(m.ser.all[start:], "ready", 4)
        ok = len(opened) == 1 and len(ready) == 1
        print(f"  2 a Super tap opens and draws Menu             "
              f"{'ok' if ok else 'FAIL'}")
        if not ok:
            fails.append("menu")
        close_start = len(m.ser.all)
        zb.tap_key(m.qmp, "meta_l", settle)
        if len(lifecycle(m.ser.all[close_start:], "close", 4)) != 1:
            fails.append("menu-close")

        # ---- 3. the final fixed rail row opens the catalogue ----------------
        # THE COMPOSITOR'S OWN REPORT, not a pixel delta: a delta cannot tell
        # "the catalog opened" from "a tooltip appeared", and reg_open() prints
        # `wm: win N title ... client ...` through wm_report on every window it
        # actually opens. It stays silent on the raise-and-focus path, which is
        # precisely the failure this probe exists to catch.
        before = zb.win_count(m.ser.all)
        start = len(m.ser.all)
        zb.click(m.qmp, g["catalogue"][0], g["catalogue"][1], m.w, m.h, settle)
        settle(2.0)
        cat = zb.grab(m.qmp, m.tmp, "catalog")
        rows = zb.WIN_REPORT.findall(m.ser.all[start:])
        ok = (zb.win_count(m.ser.all) > before
              and len(lifecycle(m.ser.all[start:], "ready", 70)) == 1)
        print(f"  3 register row 12 opens the catalogue       "
              f"{zb.win_count(m.ser.all) - before:5d} win  "
              f"{'ok' if ok else 'FAIL'}")
        if not ok:
            fails.append("all-apps-row")
            print("      reg_open(APP_CATALOG) reported no window. Either the "
                  "click missed the row, or the id it opens is already on "
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
