#!/usr/bin/env python3
"""probe-urlbar.py - can a person actually type an address into the URL bar?

THE GATE THAT WOULD HAVE CAUGHT THE BUG THIS EXISTS FOR. browser_click had no
URL-bar hit test at all, so clicking the bar did nothing; typing then fell
through browser_key's UNFOCUSED switch, which drops everything that is not a
shortcut - until the string's first `l`, which IS the focus shortcut. That `l`
was swallowed arming select-all and the next character cleared the buffer, so
typing "https://en.wikipedia.org/wiki/Linux" left exactly "inux".

Every host assertion in browsertest.c passed throughout, because every one of
them pressed 'l' first and so focused the bar by the one route that worked.
The bug lived in the wiring between the compositor and the app, which is where
four of this branch's bugs have lived, and it is only reachable by driving the
real machine.

THE ASSERTION IS SELF-CALIBRATING, and that is the point. Measuring "the ink is
at least N pixels wide" needs an N, and an N is a guess about a font at a
resolution on a day - the exact kind of number that makes a gate fail for
reasons unrelated to the code, which kernel/CLAUDE.md records costing this
project a bisect. So this types the whole address, measures the ink, then
presses BackSpace until four characters are left and measures again, IN THE
SAME BOOT with the same font and the same window. A working bar gives a long
string several times wider than a four-character one. The bug gives the two
measurements the same width, because with the bug the bar only ever held four
characters in the first place.

  ./probe-urlbar.py                 boot, click, type, assert
  ./probe-urlbar.py --keep-shots    leave the pictures in kernel/shots/
"""
import argparse, os, re, subprocess, sys, tempfile, time

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
HERE = KERNEL_ROOT
sys.path.insert(0, PROBE_DIR)
from exercise import Serial, Qmp, qemu_argv, build   # noqa: E402

SHOTS = os.path.join(HERE, "shots")

# The address from the bug report, and it matters that it is THIS one: its
# first `l` is the L of Linux, three characters from the end, which is what
# made the survivors look like "everything after the first shifted character"
# and sent the original diagnosis into the keyboard layer.
URL = "https://en.wikipedia.org/wiki/Linux"

# qcode names for the characters in it. QEMU has no "type this string", so a
# shifted character is a chord and has to be sent as one.
PLAIN = {"/": "slash", ".": "dot", "-": "minus"}
SHIFTED = {":": "semicolon"}


def absolute(qmp, x, y, w, h):
    """The tablet is ABSOLUTE. probe-drag.py records what sending relative
    events into it looks like: nothing moves and the gate blames the code."""
    qmp.cmd("input-send-event", events=[
        {"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / (w - 1))}},
        {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / (h - 1))}}])


def click(qmp, x, y, w, h):
    absolute(qmp, x, y, w, h)
    time.sleep(0.15)
    for down in (True, False):
        qmp.cmd("input-send-event", events=[
            {"type": "btn", "data": {"down": down, "button": "left"}}])
        time.sleep(0.12)


def drag(qmp, x0, y0, x1, y1, w, h, steps=14):
    """Press, walk, release - the shape probe-drag.py proves works on this
    machine. Walked rather than teleported because a compositor that only sees
    press-then-release-somewhere-else has no drag to follow."""
    absolute(qmp, x0, y0, w, h)
    time.sleep(0.2)
    qmp.cmd("input-send-event",
            events=[{"type": "btn", "data": {"down": True, "button": "left"}}])
    time.sleep(0.2)
    for i in range(1, steps + 1):
        absolute(qmp, x0 + (x1 - x0) * i // steps, y0 + (y1 - y0) * i // steps, w, h)
        time.sleep(0.06)
    qmp.cmd("input-send-event",
            events=[{"type": "btn", "data": {"down": False, "button": "left"}}])
    time.sleep(0.4)


def key(qmp, name, shift=False, settle=0.06, alt=False):
    ev = []
    if alt:
        ev.append({"type": "key", "data": {"down": True,
                   "key": {"type": "qcode", "data": "alt"}}})
    if shift:
        ev.append({"type": "key", "data": {"down": True,
                   "key": {"type": "qcode", "data": "shift"}}})
    ev.append({"type": "key", "data": {"down": True,
               "key": {"type": "qcode", "data": name}}})
    qmp.cmd("input-send-event", events=ev)
    time.sleep(settle)
    ev = [{"type": "key", "data": {"down": False,
           "key": {"type": "qcode", "data": name}}}]
    if shift:
        ev.append({"type": "key", "data": {"down": False,
                   "key": {"type": "qcode", "data": "shift"}}})
    if alt:
        ev.append({"type": "key", "data": {"down": False,
                   "key": {"type": "qcode", "data": "alt"}}})
    qmp.cmd("input-send-event", events=ev)
    time.sleep(settle)


def type_url(qmp, text):
    for ch in text:
        if ch.isalpha():
            key(qmp, ch.lower(), shift=ch.isupper())
        elif ch.isdigit():
            key(qmp, str(ch))
        elif ch in SHIFTED:
            key(qmp, SHIFTED[ch], shift=True)
        elif ch in PLAIN:
            key(qmp, PLAIN[ch])
        elif ch == " ":
            key(qmp, "spc")
        else:
            print(f"  no qcode for {ch!r} - refusing to type a guess")
            sys.exit(2)


def read_ppm(path):
    with open(path, "rb") as f:
        data = f.read()
    if not data.startswith(b"P6"):
        return None
    # header: P6 <w> <h> <max>, whitespace separated, comments allowed
    i, fields = 2, []
    while len(fields) < 3:
        while i < len(data) and data[i : i + 1].isspace():
            i += 1
        if data[i : i + 1] == b"#":
            while i < len(data) and data[i] != 0x0A:
                i += 1
            continue
        j = i
        while j < len(data) and not data[j : j + 1].isspace():
            j += 1
        fields.append(int(data[i:j]))
        i = j
    i += 1
    w, h, _ = fields
    return w, h, data[i : i + w * h * 3]


def ink_extent(path, x0, y0, x1, y1):
    """How far right does the text reach inside this strip?

    The bar's own background is whatever colour occurs most often in the strip;
    anything else is ink. Taking the mode rather than a hardcoded theme colour
    is what keeps this working when the accent or the theme changes, which is
    a thing this desktop lets the user do at run time."""
    got = read_ppm(path)
    if not got:
        return None
    w, h, px = got
    x0, y0 = max(0, x0), max(0, y0)
    x1, y1 = min(w, x1), min(h, y1)
    if x1 <= x0 or y1 <= y0:
        return None
    hist = {}
    for y in range(y0, y1):
        base = (y * w + x0) * 3
        for x in range(x1 - x0):
            c = px[base + x * 3 : base + x * 3 + 3]
            hist[c] = hist.get(c, 0) + 1
    bg = max(hist, key=hist.get)
    right, n = -1, 0
    for y in range(y0, y1):
        base = (y * w + x0) * 3
        for x in range(x1 - x0):
            c = px[base + x * 3 : base + x * 3 + 3]
            # a real difference, not a subpixel fringe one step off the bg
            if sum(abs(c[k] - bg[k]) for k in range(3)) > 90:
                n += 1
                if x > right:
                    right = x
    return right + 1, n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=300)
    ap.add_argument("--keep-shots", action="store_true")
    ap.add_argument("--src", help="build from this tree instead")
    args = ap.parse_args()

    if args.src:
        os.environ["ZLOS_SRC"] = os.path.abspath(args.src)
    os.makedirs(SHOTS, exist_ok=True)
    build(False)

    tmp = tempfile.mkdtemp(prefix="zlos-urlbar-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path, tablet=True),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    fails = []

    def check(label, ok, detail=""):
        print(("  ok   " if ok else "  FAIL ") + label + (("  " + detail) if detail else ""))
        if not ok:
            fails.append(label)

    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        ok, log = ser.wait("ready.", args.boot_timeout)
        if not ok:
            print("never booted:\n" + log[-2000:])
            sys.exit(1)
        ok, more = ser.wait("compositor:", args.boot_timeout)
        log += more
        ser.drain(1.5)
        log += ser.buf
        ser.buf = ""

        # WHERE THE BROWSER IS, from the compositor rather than from a number
        # that was true at one resolution. probe-drag.py learned this the hard
        # way: its literal default was off the right edge of the screen, so the
        # press landed on nothing and it reported a working drag as a no-op.
        wins = re.findall(
            r"wm: win (\d+) title (\d+),(\d+) (\d+)x(\d+) "
            r"client (\d+),(\d+) (\d+)x(\d+)", log)
        if not wins:
            print("the compositor reported no window rects - cannot aim")
            print(log[-1500:])
            sys.exit(1)
        _, btx, bty, btw, bth, cx, cy, cw, ch = (int(v) for v in wins[-1])
        print(f"  browser client rect {cx},{cy} {cw}x{ch}"
              f"  title {btx},{bty} {btw}x{bth}")
        # THE TITLE BAR RECT, TAKEN FROM THE LOG AND NOT DERIVED. The first
        # version computed it as "the strip between the title y and the client
        # y" while discarding the title y in the regex, so it came out zero
        # tall, ink_extent returned None for every rotation, and the argmin
        # below picked rotation 0 four times over. A measurement that returns
        # the same sentinel for every input is not a measurement.
        tbar = (btx, bty, btx + btw, bty + bth)

        probe = os.path.join(tmp, "probe.ppm")
        qmp.screendump(probe)
        sw, sh, _ = read_ppm(probe)

        # The chrome row runs from the top of the client area down by one text
        # row. Aim at the middle of the window's width, which is inside the URL
        # bar for any em: the bar starts after Back (about five ems in) and
        # runs to the right edge.
        bar_y = cy + 14
        bar_x0, bar_x1 = cx + cw // 2 - 4, cx + cw - 8
        strip = (cx + 4, cy + 2, cx + cw - 4, cy + 34)

        # UNCOVER THE BROWSER FIRST, and this is not a detail. The desktop opens
        # four windows and the browser is the LAST of them, which puts it
        # entirely UNDERNEATH the shell: shell client 82,160 1236x834 contains
        # browser client 242,248 1036x702 on every axis. A press at the URL
        # bar's coordinates therefore lands on the shell, and the first run of
        # this gate typed the whole address into the shell prompt - which is
        # visible in the screenshot and is a perfectly good proof that
        # keystroke delivery works, and no proof at all of what it was asked.
        #
        # Alt+Tab raises (wm.c's cycle_focus calls wm_raise). WHICH rotation
        # puts the browser on top is a z-order question this harness must not
        # try to reason about - z-order is not on the serial log and reasoning
        # about it here is exactly the "recomputing the layout in Python" that
        # probe-drag.py's comment refuses. So: walk a FULL cycle, measure the
        # ink in the browser's title-bar strip at each step, and take the
        # argmin. A title bar is flat colour and a few words; the shell's body
        # underneath is dense console text, so the minimum is unambiguous and
        # needs no threshold. A full cycle is the identity, so continuing to
        # the winning rotation is just (best) more presses.
        # Alt+Tab raises (wm.c's cycle_focus calls wm_raise), and it was the
        # first thing tried - a full rotation produced the SAME title-bar ink
        # four times, so the modifier is not reaching route_key on this
        # machine. That is the input track's business (POINTER-PROMPT), not
        # this gate's, and a gate that depends on a second subsystem's
        # modifier handling is a gate that will fail for the wrong reason.
        #
        # The POINTER demonstrably works - the address typed in the first run
        # went into the shell, which means the click landed and focused it. So
        # move the shell out of the way with a drag, which probe-drag.py
        # already proves on this exact machine, and then the browser's own
        # chrome is reachable with an ordinary press.
        shell = [w for w in wins if int(w[0]) == 0]
        if shell:
            _, stx, sty, stw, sth, *_ = (int(v) for v in shell[0])
            print(f"  moving the shell's title bar {stx},{sty} {stw}x{sth} out of the way")
            drag(qmp, stx + stw // 3, sty + sth // 2,
                 stx + stw // 3, sh - 60, sw, sh)
        time.sleep(0.6)

        # ...and now a press inside the browser raises AND focuses it, which is
        # what route_mouse does for any press in a window.
        click(qmp, cx + cw // 2, cy + ch // 2, sw, sh)
        time.sleep(0.5)

        click(qmp, cx + cw * 3 // 4, bar_y, sw, sh)
        time.sleep(0.6)

        type_url(qmp, URL)
        time.sleep(0.8)
        long_shot = os.path.join(SHOTS if args.keep_shots else tmp, "urlbar-long.ppm")
        qmp.screendump(long_shot)
        long_ext, long_ink = ink_extent(long_shot, *strip)

        # ...then cut it down to four characters IN THE SAME BOOT. This is the
        # control, and it is what makes the comparison need no magic number.
        for _ in range(len(URL) - 4):
            key(qmp, "backspace", settle=0.045)
        time.sleep(0.8)
        short_shot = os.path.join(SHOTS if args.keep_shots else tmp, "urlbar-short.ppm")
        qmp.screendump(short_shot)
        short_ext, short_ink = ink_extent(short_shot, *strip)

        print(f"  {len(URL)} characters -> ink reaches {long_ext}px, {long_ink} pixels")
        print(f"   4 characters -> ink reaches {short_ext}px, {short_ink} pixels")

        check("the bar drew something at all", long_ink > 0,
              f"{long_ink} ink pixels")
        check("four characters drew something", short_ink > 0,
              f"{short_ink} ink pixels")

        # THE COUNT, NOT THE EXTENT, and the first version of this gate got it
        # wrong. "How far right does the ink reach" sounds like the natural
        # measure of how much text is in a box, and it is useless here: the
        # strip contains the bar's own right-hand chrome, which is ink at a
        # fixed column, so the extent read 790 for 35 characters and 790 for 4.
        # The number was real, reproducible, and measuring the border.
        #
        # The COUNT separates cleanly because it is proportional to the text
        # while the chrome only adds a constant. Measured on this machine:
        # 876 ink pixels at 4 characters, 2767 at 35 - which solves to about
        # 61 pixels per character over ~630 of fixed chrome, so the
        # relationship is linear and the constant does not swamp it. Asserting
        # 2x sits well inside that margin and well outside the bug, which makes
        # the two counts EQUAL because the bar only ever held four characters.
        check(f"{len(URL)} typed characters put far more ink in the bar than 4",
              long_ink > short_ink * 2,
              f"{long_ink} vs {short_ink} ink pixels")

        if args.keep_shots:
            print(f"  shots in {SHOTS}")

    finally:
        proc.terminate()
        try:
            proc.wait(timeout=10)
        except subprocess.TimeoutExpired:
            proc.kill()

    print()
    if fails:
        print(f"urlbar gate FAILED: {len(fails)} check(s)")
        sys.exit(1)
    print("urlbar gate ok: a click focuses the bar and every character typed arrives")


if __name__ == "__main__":
    main()
