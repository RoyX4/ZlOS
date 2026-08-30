#!/usr/bin/env python3
"""probe-overlay-click.py - the command palette can be driven with the mouse.

THE DEFECT THIS EXISTS FOR. wm.c registered exactly one overlay hook and it was
a DRAW hook. route_mouse contained no reference to the overlay layer and no hit
test for any overlay surface existed anywhere in the tree. So every row of the
command palette, the field menu and the window menu drew, highlighted, listed
its keyboard shortcut - and could not be clicked. Thirty-seven rows across
three surfaces. Worse than inert: the click fell THROUGH to whatever was
painted underneath, so clicking "close" on a window menu focused and raised the
very window it was drawn on top of.

WHAT IS ASSERTED, AND WHY IT IS THE WINDOW COUNT.

Clicking a palette row that opens an app has two visible consequences: a new
window exists, and the overlay is gone. The window count is the stronger of the
two because it comes off the SERIAL LOG - `wm: win N title ...` - rather than
off a picture, so it cannot be satisfied by a repaint, an animation, or the
overlay merely dismissing itself. A probe that only checked "the palette went
away" would pass on a build where the click dismissed and did nothing else,
which is one of the three failure modes here.

The control is a click on the palette's own header - inside the sheet, on no
row - which must NOT open anything. Without it the assertion would also be
satisfied by a build that opened an app on any click anywhere.

VERIFIED IN BOTH DIRECTIONS, both surfaces. Measured, screen 1920x1200:

    everything wired                  3 -> 4 -> 5   PASS
    route_mouse's overlay hook removed 3 -> 3        FAIL (palette row)
    desk_click's right-button arm removed             FAIL (menu never opens,
                                                            and its row with it)

Both control checks - the header click opening nothing, and the sheet still
being painted afterwards - PASS IN BOTH BUILDS. That is the point of them: they
are there to catch a probe that passes for the wrong reason, so they must not
move when the thing under test does.

Re-plant before trusting a green run. The plant is one line: wrap the
`hook_overlay_click` call in route_mouse with `if (0 && ...)`.
"""
import argparse, os, re, subprocess, sys, tempfile, time

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, PROBE_DIR)
from exercise import Serial, Qmp, qemu_argv, build  # noqa: E402


def moveto(qmp, x, y, w, h):
    qmp.cmd("input-send-event", events=[
        {"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / (w - 1))}},
        {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / (h - 1))}}])


def btn(qmp, down):
    qmp.cmd("input-send-event",
            events=[{"type": "btn", "data": {"down": down, "button": "left"}}])


def click(qmp, x, y, w, h):
    moveto(qmp, x, y, w, h)
    time.sleep(0.25)
    btn(qmp, True)
    time.sleep(0.15)
    btn(qmp, False)
    time.sleep(0.7)


def sheet_pixels_differ(path, W, H, sx, sy, sw):
    """Is a plate painted where the palette sheet belongs?

    The sheet is ZD_PANEL over a scrimmed desktop, so its interior is close to
    uniform and markedly lighter than the field behind it. Sampling a row inside
    the sheet and a row well outside it and asking whether they differ is enough
    to say "a plate is there" without hard-coding a colour that a re-pointed
    palette would invalidate.
    """
    blob = open(path, "rb").read()
    if not blob.startswith(b"P6"):
        return False
    fields, i = [], 2
    while len(fields) < 3:
        while i < len(blob) and blob[i:i + 1].isspace():
            i += 1
        j = i
        while j < len(blob) and not blob[j:j + 1].isspace():
            j += 1
        fields.append(int(blob[i:j]))
        i = j
    px = blob[i + 1:]

    def at(x, y):
        o = (y * W + x) * 3
        return px[o:o + 3]

    inside = at(sx + sw // 2, sy + 8)
    outside = at(max(0, sx - 60), sy + 8)
    return inside != outside


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    args = ap.parse_args()
    build(False)
    fails = []

    def check(label, ok, detail=""):
        print(("  ok    " if ok else "  FAIL  ") + label + (f"   [{detail}]" if detail else ""))
        if not ok:
            fails.append(label)

    tmp = tempfile.mkdtemp(prefix="ovclick-")
    sp, qp = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, sp, qp),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(sp), Qmp(qp)
        ok, log = ser.wait("compositor:", args.boot_timeout)
        if not ok:
            print("no compositor:\n" + log[-1500:])
            return 1
        ser.drain(1.5)
        log += ser.buf
        ser.buf = ""

        # THE SCREEN IS MEASURED FROM A SCREENDUMP, not scraped out of the boot
        # text. The first version of this used a regex over the serial log and
        # matched an unrelated "800x600" printed earlier in the boot, so every
        # click was normalised against a screen a third of the real size and
        # landed nowhere near the palette. The probe reported a dead row and the
        # row was fine.
        #
        # A PPM header cannot be anything other than the frame that was just
        # captured, which is the property the log did not have.
        probe_ppm = os.path.join(tmp, "size.ppm")
        if not qmp.screendump(probe_ppm):
            print("no screendump")
            return 1
        hdr = open(probe_ppm, "rb").read(64).split()
        W, H = int(hdr[1]), int(hdr[2])
        print(f"  note  screen {W}x{H}")

        def wincount():
            ser.drain(0.6)
            nonlocal log
            log += ser.buf
            ser.buf = ""
            return len(re.findall(r"wm: win \d+ title", log))

        base = wincount()

        # CTRL+K OPENS THE COMMAND PALETTE, and it is sent down the SERIAL wire
        # as the bare control code. Two corrections are baked in here.
        #
        # The first version pressed Super, on the assumption that Super opens
        # the palette. It does not - desk_key routes Super to open_menu(), which
        # opens a WINDOW. The assertion "Super opened an overlay without opening
        # a window" was therefore testing the opposite of what it said, and it
        # passed only because the key never arrived at all.
        #
        # COM1 pushes EV_CHAR alone (input.c says so), so byte 11 IS Ctrl+K on
        # this wire, with no modifier dance to get wrong.
        ser.send("\x0b")
        time.sleep(1.0)
        after_open = wincount()
        check("Ctrl+K opened an overlay without opening a window",
              after_open == base, f"{base} -> {after_open}")

        # The palette sheet: 580 design units wide, centred, top third. These
        # match ov_sheet_x/ov_sheet_y. The header band is the top 26 units.
        u = 1
        sheet_w = 580 * u
        sheet_x = (W - sheet_w) // 2
        sheet_y = (H - 420 * u) // 3

        # CONTROL: a click on the header - inside the sheet, on no row. It must
        # open nothing AND must not dismiss, because only the scrim dismisses.
        #
        # The first version of this probe took the dismissal for granted and put
        # the control first, so the palette was already gone by the time the row
        # click happened and the probe reported a dead row against a working
        # build. It also found a real defect doing it: ov_click was treating
        # "not on a row" as "off the surface", so the palette's own header
        # closed it.
        click(qmp, sheet_x + sheet_w // 2, sheet_y + 12 * u, W, H)
        after_hdr = wincount()
        check("a click on the palette header opens nothing",
              after_hdr == after_open, f"{after_open} -> {after_hdr}")

        # A THIRD CHECK, from the same mistake: the sheet must still be up after
        # the header click. If it is not, the row assertion below is measuring a
        # desktop rather than a palette, and would read as a dead control.
        #
        # ROW 3, "open 05 kernel log", AND EACH SURFACE GETS ITS OWN APP.
        #
        # This was row 2, Settings - and the field-menu case further down clicks
        # ITS "open 11 settings" row. Settings was therefore already open by the
        # time the menu was tested, reg_open raised the existing window instead
        # of making a second, the count could not move, and the menu read as
        # dead. That is the SAME trap as row 0 being System Monitor, hit a
        # second time from a different direction: an assertion that cannot
        # succeed is indistinguishable from a feature that does not work.
        #
        # Two surfaces, two apps neither of which the boot composition opens.
        #
        # ROW 3, NOT ROW 0. Row 0 is System Monitor and row 1 is Files, and
        # BOTH ARE ALREADY OPEN AT BOOT - reg_open raises an open window rather
        # than making a second one, so the window count cannot move and the
        # probe fails against a perfectly working build. Row 2 is Settings,
        # which the boot composition does not open.
        #
        # This is the same trap as the screen size: an assertion that cannot
        # succeed reads exactly like a feature that does not work.
        sheet_probe = os.path.join(tmp, "sheet.ppm")
        qmp.screendump(sheet_probe)
        still_up = sheet_pixels_differ(sheet_probe, W, H, sheet_x, sheet_y, sheet_w)
        check("the header click did not dismiss the palette", still_up,
              "sheet still painted" if still_up else "the sheet is gone")

        click(qmp, sheet_x + sheet_w // 2, sheet_y + (62 + 3 * 22 + 11) * u, W, H)
        time.sleep(0.8)
        after_row = wincount()
        check("clicking a palette row opened a window",
              after_row > after_hdr, f"{after_hdr} -> {after_row}")

        # ---- THE FIELD MENU, WHICH RIGHT-CLICK COULD NOT OPEN AT ALL -------
        #
        # desk_click read bit 0 of the button mask and never bit 1, so a right
        # press on the desk produced down == 0 and fell straight out. The menu's
        # twelve rows had exactly one route: typing `ctxmenu` in the Terminal.
        #
        # Asserted the same way as the palette - a row that opens an app, judged
        # by the window count off the serial log - because "a menu appeared" can
        # be satisfied by a repaint and "a window opened" cannot.
        n_before_menu = wincount()
        fx, fy = W - 320, H - 420          # bare field: right of the windows,
                                           # above the foot, clear of the rail
        moveto(qmp, fx, fy, W, H)
        time.sleep(0.25)
        qmp.cmd("input-send-event",
                events=[{"type": "btn", "data": {"down": True, "button": "right"}}])
        time.sleep(0.2)
        qmp.cmd("input-send-event",
                events=[{"type": "btn", "data": {"down": False, "button": "right"}}])
        time.sleep(0.9)

        menu_ppm = os.path.join(tmp, "menu.ppm")
        qmp.screendump(menu_ppm)
        menu_up = sheet_pixels_differ(menu_ppm, W, H, fx, fy + 10, 200)
        check("right-clicking the field opened a menu", menu_up,
              "a plate is painted at the pointer" if menu_up else "nothing appeared")

        # Row 10 of the field menu is "open 11 settings". Rows are 20 units,
        # separators 7, under a 20-unit head. THREE separators precede row 10
        # (ov_ctx_sep flags 2, 6 and 9), not two - the first version of this
        # counted two and clicked 13 units low, landing on row 11 "lock session",
        # which opens no window and so read as a dead menu.
        #
        #   20 head + 7 rows x 20 + 3 seps x 7 = 181, middle 191.
        click(qmp, fx + 100, fy + 191, W, H)
        time.sleep(0.9)
        n_after_menu = wincount()
        check("a field-menu row opened its app",
              n_after_menu > n_before_menu, f"{n_before_menu} -> {n_after_menu}")

        print()
        print(f"windows: {base} at boot, {after_open} with the palette up, "
              f"{after_hdr} after the header click, {after_row} after the row click, "
              f"{n_after_menu} after the field-menu row")
        print(("FAIL: " + ", ".join(fails)) if fails else "PASS")
        return 1 if fails else 0
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()


if __name__ == "__main__":
    sys.exit(main())
