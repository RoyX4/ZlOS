#!/usr/bin/env python3
"""probe-catalog.py - does "All Applications" actually launch an app?

Same discipline as probe-dock.py: a screenshot cannot tell a working catalog
from a picture of one, so this drives the real pointer through the real path
- start button, the new "All Applications" menu row, a tile in the catalog -
and asks whether the SCREEN changed each time, not whether the code compiled.

  1 MENU     the start button opens the 10-row menu (same as probe-dock.py)
  2 CATALOG  clicking "All Applications" (the new last-but-one row) opens it
  3 LAUNCH   clicking the first tile in the catalog opens THAT app's window
"""
import os, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

HERE = os.path.dirname(os.path.abspath(__file__))
SHOTS = os.path.join(HERE, "shots")

# design-unit constants, copied from kernel.zl/apps_registry.zl rather than
# re-derived, so a layout change there is what breaks this probe rather than
# independent arithmetic drifting from it.
TITLE_H = 28
MENU_ROWS = 10
DOCK_H = 64
CAT_HEADER = 26
CAT_TILE_W = 130
CAT_TILE_H = 108


def at(qmp, x, y, w, h, btn=None):
    ev = [{"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / w)}},
          {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / h)}}]
    if btn is not None:
        ev.append({"type": "btn", "data": {"down": btn, "button": "left"}})
    qmp.cmd("input-send-event", events=ev)


def click(qmp, x, y, w, h, settle):
    at(qmp, x, y, w, h, btn=True)
    settle(0.3)
    at(qmp, x, y, w, h, btn=False)
    settle(0.3)


def shot(qmp, tmp, name):
    p = os.path.join(tmp, name + ".ppm")
    qmp.screendump(p)
    from PIL import Image
    return Image.open(p).convert("RGB").copy()


def differs(a, b, box):
    x0, y0, x1, y1 = box
    n = 0
    for y in range(y0, y1, 2):
        for x in range(x0, x1, 2):
            if a.getpixel((x, y)) != b.getpixel((x, y)):
                n += 1
    return n


def main():
    build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-catalog-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    fails = []
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        if not ser.wait("ready.", 240)[0]:
            print("never booted"); return 1
        ser.wait(PROMPT, 60)
        ser.drain(1.5)

        base = shot(qmp, tmp, "base")
        W, H = base.size
        print(f"booted {W}x{H}")
        ui = 2 if W >= 1400 else 1
        settle = ser.drain

        # ---- 1. open the start menu - identical click to probe-dock.py's
        dock_y = H - DOCK_H * ui
        click(qmp, 40 * ui, dock_y + 32 * ui, W, H, settle)
        settle(1.0)
        menu = shot(qmp, tmp, "menu")
        mbox = (10 * ui, dock_y - 320 * ui, 220 * ui, dock_y - 10 * ui)
        n = differs(base, menu, mbox)
        print(f"  start button opens the menu       {n:5d} px  "
              f"{'ok' if n > 2000 else 'FAIL'}")
        if n <= 2000: fails.append("menu")
        menu.save(os.path.join(SHOTS, "catalog-menu.png"))

        # ---- 2. click "All Applications" - row 8 of 10 (0-indexed), the
        # same row_y formula menu_draw uses: ay + 4u + row*26u, where ay is
        # the menu WINDOW's client top (its y + TITLE_H*u).
        mh = MENU_ROWS * 26 * ui + TITLE_H * ui + 12 * ui
        my = dock_y - mh - 8 * ui
        menu_client_y = my + TITLE_H * ui
        row8_y = menu_client_y + 4 * ui + 8 * 26 * ui + 13 * ui
        click(qmp, 10 * ui + 100 * ui, row8_y, W, H, settle)
        settle(1.5)
        catalog = shot(qmp, tmp, "catalog")
        cbox = (int(W * 0.05), int(H * 0.05), int(W * 0.65), int(H * 0.65))
        n = differs(menu, catalog, cbox)
        print(f"  'All Applications' opens the catalog {n:5d} px  "
              f"{'ok' if n > 3000 else 'FAIL'}")
        if n <= 3000: fails.append("catalog-open")
        catalog.save(os.path.join(SHOTS, "catalog-open.png"))

        # ---- 3. click the first tile in the catalog. reg_new() opens it at
        # (120u, 40u); client top = 40u + TITLE_H*u; the grid starts
        # CAT_HEADER*u below that; tile 0 is the first column, first row.
        cat_x = 120 * ui
        cat_y = 40 * ui + TITLE_H * ui
        tile0_x = cat_x + CAT_TILE_W * ui // 2
        tile0_y = cat_y + CAT_HEADER * ui + CAT_TILE_H * ui // 2
        click(qmp, tile0_x, tile0_y, W, H, settle)
        settle(1.5)
        launched = shot(qmp, tmp, "launched")
        lbox = (0, 0, W, H)
        n = differs(catalog, launched, lbox)
        print(f"  clicking a tile launches an app      {n:5d} px  "
              f"{'ok' if n > 3000 else 'FAIL'}")
        if n <= 3000: fails.append("launch")
        launched.save(os.path.join(SHOTS, "catalog-launched.png"))

        # ---- 4. a GAME specifically, not just a read-only utility: open
        # Tic-Tac-Toe (grid index 11 - row 2, col 3) and place a mark, which
        # exercises ac_cell_idx and the win-check path most of the board
        # games in apps_games1.zl share.
        tt_col, tt_row = 11 % 4, 11 // 4
        ttile_x = cat_x + tt_col * CAT_TILE_W * ui + CAT_TILE_W * ui // 2
        ttile_y = cat_y + CAT_HEADER * ui + tt_row * CAT_TILE_H * ui + CAT_TILE_H * ui // 2
        click(qmp, ttile_x, ttile_y, W, H, settle)
        settle(1.5)
        tt_open = shot(qmp, tmp, "tt-open")
        n = differs(launched, tt_open, (0, 0, W, H))
        print(f"  Tic-Tac-Toe tile opens the game      {n:5d} px  "
              f"{'ok' if n > 3000 else 'FAIL'}")
        if n <= 3000: fails.append("game-open")

        # Tic-Tac-Toe opens at (160u, 80u), 300u x 220u; tt_draw centres a
        # 3x44u board at ax + (aw - 132u)/2, ay + 4u.
        tt_ax, tt_ay = 160 * ui, 80 * ui + TITLE_H * ui
        cell = 44 * ui
        gx = tt_ax + (300 * ui - 3 * cell) // 2
        gy = tt_ay + 4 * ui
        click(qmp, gx + cell // 2, gy + cell // 2, W, H, settle)
        settle(1.0)
        tt_marked = shot(qmp, tmp, "tt-marked")
        gbox = (gx, gy, gx + 3 * cell, gy + 3 * cell)
        n = differs(tt_open, tt_marked, gbox)
        print(f"  clicking a cell places a mark        {n:5d} px  "
              f"{'ok' if n > 20 else 'FAIL'}")
        if n <= 20: fails.append("game-click")
        tt_marked.save(os.path.join(SHOTS, "catalog-tt-marked.png"))

        print()
        print("all good" if not fails else "FAILED: " + ", ".join(fails))
        return 1 if fails else 0
    finally:
        proc.kill(); proc.wait()


if __name__ == "__main__":
    sys.exit(main())
