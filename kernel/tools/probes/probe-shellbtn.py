#!/usr/bin/env python3
"""probe-shellbtn.py - does the terminal's DMESG button actually launch anything?

WHY THIS EXISTS. The button row shipped with a commit message saying "the
buttons are clickable, which is the difference between a control and a picture
of one". They were not. An adversarial review of that very diff found them dead
for TWO independent reasons:

  - sh_btn_hit computed `wm_ch(win) - ...`, a client HEIGHT, and compared it
    against ey, a SCREEN coordinate. Drawn in one space, hit-tested in another.
  - both dispatched through open_app(), which handles 77, 97, 100, 103, 105,
    118 and 120 - neither 210 nor 216 - so each returned 0 and did nothing.

WHY THIS FILE IS probe-rail WITH THE TARGET SWAPPED. After fixing both, a
hand-written harness of my own reported the button still dead, six runs
running - through a batched button event, a separate button event, a PS/2
walk, and a twelve-second settle. An instrumented kernel showed route_mouse
never saw a non-zero button mask at all, while probe-rail clicking a rail row
passed against the SAME binary. The variable was the harness, not the kernel.
So this reuses the harness that works and changes only what it aims at, which
is the whole reason it now passes.

THE VERDICT IS THE KERNEL'S OWN LIFECYCLE LOG, not a pixel count. `wm:lifecycle
event=open app=40` is the machine saying which app it opened; a frame delta is
a second, weaker opinion about the same question - and it was the frame delta
that kept reporting a working button as dead.
"""
import os, subprocess, sys, tempfile, re, time

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
sys.path.insert(0, PROBE_DIR)
from exercise import Serial, Qmp, qemu_argv, build, PROMPT   # noqa: E402

# THE SCREEN SIZE IS READ FROM THE SCREENSHOT, never declared. It was
# `W, H = 1280, 800` while the framebuffer is 1920x1200, so at() scaled every
# pointer coordinate by 1920/1280 = 1.5 and the click meant for row 05 landed on
# row 08. The probe then reported "row 05 opened app 31" as a KERNEL bug - and
# app 31 is Clocks & Timers, which is exactly what sits at slot 8. A probe that
# states the geometry instead of measuring it can manufacture a finding.
W, H = 0, 0


def at(qmp, x, y, btn=None):
    ev = [{"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / W)}},
          {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / H)}}]
    assert W and H, "screen size must be read from a screenshot before clicking"
    if btn is not None:
        ev.append({"type": "btn", "data": {"down": btn, "button": "left"}})
    qmp.cmd("input-send-event", events=ev)


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


def rail_rows_from_render(img, u=1):
    """Find the register rows by LOOKING, not by restating the layout.

    The first version rebuilt the rail's geometry in Python from RAIL_IDENT_H
    and a guessed row pitch, and clicked four rows low - the probe reported "the
    row launched something" while opening Clocks & Timers from a row that says
    kernel log. row_h() is ui_metric(UM_ROW_H) anyway, which no amount of
    source-parsing resolves.

    THE SIGNAL IS THE CUT RULE. Every register row is closed by a 1 px ZD_CUT
    hairline, and on the rail's ZD_RAISE ground that rule is near-black - (10,
    3, 0) against (65, 58, 54). The second version looked for "darker than both
    neighbours", which also matches every icon edge in the column and found
    nothing usable. Matching on the absolute darkness is unambiguous.
    """
    x = 40 * u                       # inside the rail, in the icon column
    dark = [y for y in range(1, img.height - 1)
            if sum(img.getpixel((x, y))) < 40]
    # THE MODAL GAP, then the longest chain that keeps it. The previous version
    # walked forward accepting anything within 1 of the running pitch and
    # `break`ing only when the gap was LARGER - so a gap SMALLER than the pitch
    # fell through both branches, was skipped in silence, and the chain kept
    # growing across unrelated features. It reported 108 rows of pitch 11 on an
    # 800-tall screen, which is impossible and should have been caught by the
    # arithmetic rather than by a click landing in the identity block.
    if len(dark) < 4:
        sys.exit(f"probe-rail: only {len(dark)} dark rows in the rail column; "
                 "the register could not be located on screen")
    gaps = {}
    for i in range(len(dark) - 1):
        g = dark[i + 1] - dark[i]
        if 12 <= g <= 60:
            gaps[g] = gaps.get(g, 0) + 1
    if not gaps:
        sys.exit("probe-rail: no plausible row pitch among the rail's rules")
    pitch = max(gaps, key=lambda g: gaps[g])
    best = []
    for i in range(len(dark)):
        run = [dark[i]]
        for j in range(i + 1, len(dark)):
            d = dark[j] - run[-1]
            if abs(d - pitch) <= 1:
                run.append(dark[j])
            elif d > pitch:
                break                     # a real gap ends the register
        if len(run) > len(best):
            best = run
    if len(best) < 6:
        sys.exit(f"probe-rail: found only {len(best)} rows at pitch {pitch}; "
                 "the register could not be located on screen")
    centres = [(best[i] + best[i + 1]) // 2 for i in range(len(best) - 1)]
    return {"u": u, "row_h": pitch, "rows": [(85 * u, c) for c in centres]}


def rail_app_of_slot(slot):
    """What rail_app() says slot N launches - READ from kernel.zl, not copied.

    The probe must not restate the table it is checking, or it can only ever
    agree with itself. This resolves APP_* constants the same way the source
    does so a rename cannot silently pass.
    """
    root = os.path.join(KERNEL_ROOT, "src", "kernel.zl")
    src = open(root, encoding="utf-8").read()
    body = re.search(r"fn rail_app\(slot\) \{(.*?)\n\}", src, re.S)
    if not body:
        sys.exit("probe-rail: kernel.zl no longer defines rail_app")
    m = re.search(r"if slot == %d \{ return (\w+)" % slot, body.group(1))
    if not m:
        sys.exit(f"probe-rail: rail_app has no slot {slot}")
    tok = m.group(1)
    if tok.isdigit():
        return int(tok)
    c = re.search(r"^%s\s*=\s*(\d+)" % tok, src, re.M)
    if not c:
        sys.exit(f"probe-rail: cannot resolve {tok}")
    return int(c.group(1))


def main():
    build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-rail-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    fails = 0
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        ser.wait(PROMPT, 120)
        # ...AND THEN FOR THE DESKTOP. The prompt arrives on the serial line
        # before the compositor has painted, so the first screenshot was 1198 of
        # 1200 rows black. Every pixel comparison after that was measuring the
        # desktop APPEARING, which is why "clicking changed 177550 px" came back
        # byte-identical on runs that clicked different places.
        base = None
        for _ in range(60):
            cand = shot(qmp, tmp, "base")
            dark = sum(1 for y in range(1, cand.height - 1)
                       if sum(cand.getpixel((40, y))) < 40)
            if dark < cand.height // 2:
                base = cand
                break
            time.sleep(1)
        if base is None:
            print("  FAIL the desktop never painted"); return 1
        global W, H
        W, H = base.size
        print(f"  screen is {W}x{H}, read from the screenshot")
        base.save("/tmp/probe-rail-base.png")
        geo = rail_rows_from_render(base)
        print(f"  register located on screen: {len(geo['rows'])} rows, "
              f"pitch {geo['row_h']} px, first centre y={geo['rows'][0][1]}")

        # THE TERMINAL'S DMESG BUTTON, found by colour rather than computed.
        # This file is probe-rail with the target swapped, and that is
        # deliberate: a hand-written harness of my own failed to deliver a
        # press four times running while THIS one has always worked, so the
        # variable under test has to be the target and not the harness.
        VERM = (0xE8, 0x73, 0x4F)
        def near(px):
            return all(abs(px[i] - VERM[i]) <= 24 for i in range(3))
        best = None
        for yy in range(0, base.height - 4):
            xx = 0
            while xx < base.width:
                if near(base.getpixel((xx, yy))):
                    x0 = xx
                    while xx < base.width and near(base.getpixel((xx, yy))):
                        xx += 1
                    if best is None or (xx - x0) > best[2]:
                        best = (x0, yy, xx - x0)
                else:
                    xx += 1
        if best is None or best[2] < 40:
            print("  FAIL no button-sized overprint run found"); return 1
        tx, ty = best[0] + best[2] // 2, best[1] + 6
        print(f"  DMESG button found at x {best[0]}..{best[0]+best[2]} y {best[1]}"
              f" - clicking {tx},{ty}")
        at(qmp, tx, ty)
        hov = shot(qmp, tmp, "hover")
        row_box = (0, ty - geo["row_h"] // 2, 170 * geo["u"], ty + geo["row_h"] // 2)
        n = differs(base, hov, row_box)
        # OBSERVED, NOT ASSERTED. Whether a hover repaints is a design choice
        # and the compositor may coalesce it; the question this probe exists to
        # answer is which APP the row launches, and the lifecycle log answers
        # that outright. A pixel count kept as a verdict here would be a second,
        # weaker opinion competing with a definitive one.
        print(f"  note  hover on row 05 changed {n} px in the row")

        at(qmp, tx, ty, True)
        at(qmp, tx, ty, False)
        time.sleep(2)
        after = shot(qmp, tmp, "after")

        # the DESK, not the rail: a launch has to put a window on the field
        desk_box = (200 * geo["u"], 40, W - 20, H - 90)
        n = differs(base, after, desk_box)
        # Also observed rather than asserted, and for a sharper reason: this
        # number is what made the miscalibrated version of this probe look like
        # it was working. "177550 px changed" was the desktop finishing its
        # first paint, not a window opening, and it came back byte-identical on
        # runs that clicked completely different rows.
        print(f"  note  clicking row 05 changed {n} px on the desk")

        # ...AND IT IS THE RIGHT APP. "Something opened" is not the question:
        # the defect this probe exists for opened the EDITOR when the row said
        # kernel log, and that passes any pixel-count test. The kernel prints
        # `wm:lifecycle v=1 event=open slot=N app=N` for every window it opens,
        # so the app id is stated rather than inferred.
        want = rail_app_of_slot(4)
        # Serial accumulates into .buf, and pump() is what fills it. Reading the
        # attribute without pumping first is how this probe reported its own
        # bug as the kernel's on the previous run.
        for _ in range(40): ser.pump()
        opened = re.findall(r"wm:lifecycle v=1 event=open slot=\d+ app=(\d+)", ser.buf)
        print(f"  lifecycle open events: {opened}, row 05 declares app {want}")
        if str(want) not in opened:
            print(f"  FAIL row 05 opened {opened} - it names app {want}"); fails += 1
        else:
            print(f"  ok   it opened app {want}, which is what row 05 names")
    finally:
        proc.kill()
    print()
    if fails:
        print(f"probe-shellbtn2: FAILED ({fails})"); return 1
    print("probe-shellbtn2: PASS - the DMESG button launches the app it names, from the "
          "kernel's own lifecycle log")
    return 0


if __name__ == "__main__":
    sys.exit(main())
