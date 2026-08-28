#!/usr/bin/env python3
"""probe-escape.py - does ESC actually dismiss an overlay, by both roads?

WHY THIS EXISTS. desk_key has always handled Escape, and Escape never reached
it. Two independent bugs, each hiding the other:

  1. hook_desk_key had ONE call site, gated `code == KEY_SUPER`. Escape and F1
     were never routed to the desktop at all.
  2. desk_key compared Escape against 27 and F1 against 0x13B. keycodes.h says
     KEY_ESC is 0x101 and KEY_F1 is 0x120. 0x13B is 0x100|0x3B, the LINUX
     keycode, from a scheme nothing in this tree emits.

Note which of the three bindings was right: KEY_SUPER, 0x11A - and Super was the
only one ever routed. A key nobody can press cannot be observed to hold the
wrong number, so each bug kept the other invisible while the overlay painted
ESC TO DISMISS on screen.

BOTH ROADS, because Escape arrives as two different events. A real keyboard
sends EV_KEY_DOWN/KEY_ESC. COM1 sends the raw byte as EV_CHAR 27 and no key
event - input.c chose that deliberately ("a serial ESC would start arriving as
KEY_ESC where the editor has always seen 27"). Every probe here drives the
machine over that wire, so a fix covering only the keycode would be one no probe
could reach.

TWO MEASUREMENT TRAPS THIS AVOIDS, both hit while writing it:

  - SHOOTING BEFORE THE DESKTOP PAINTS. The first version's opening frame was
    mid-boot, so the first case read "100% of pixels differ" and every ratio
    after it was measured against a picture of nothing. settle() waits for two
    consecutive frames to agree before anything is called a baseline.
  - CASES CONTAMINATING EACH OTHER. When case 1 failed to dismiss, case 2's
    baseline already had case 1's overlay in it, so case 2 reported "the overlay
    never appeared" - a second failure invented by the first. Every case now
    returns to the verified baseline before the next one starts, and if it
    cannot, the run STOPS rather than printing more verdicts it cannot support.
"""
import os, subprocess, sys, tempfile, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, ppm_sample, frame_delta, PROMPT

MOVED = 0.005      # an overlay covers far more than this; idle noise is far less
SETTLED = 0.001    # two frames this close apart are the same picture

build(False)
tmp = tempfile.mkdtemp(prefix="zlos-esc-")
ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
fails, n = [], [0]
try:
    ser, qmp = Serial(ser_path), Qmp(qmp_path)
    ser.wait("ready.", 180)
    ser.wait(PROMPT, 30)

    def shot(tag):
        n[0] += 1
        p = os.path.join(tmp, "%03d-%s.ppm" % (n[0], tag.replace("/", "-")))
        if not qmp.screendump(p):
            return None
        return ppm_sample(p)

    def settle(tag, tries=12):
        """A frame that has stopped changing. Returns None if it never does."""
        prev = shot(tag)
        for _ in range(tries):
            time.sleep(0.4)
            cur = shot(tag)
            if prev and cur:
                d = frame_delta(prev, cur)
                if d is not None and d <= SETTLED:
                    return cur
            prev = cur
        return prev

    base = settle("boot")
    if base is None:
        print("FAIL - never got a stable frame after boot")
        sys.exit(1)

    def at_base(f):
        d = frame_delta(base, f)
        return d is not None and d < MOVED

    def case(name, opener, dismiss):
        up = None
        ser.send(opener + "\r")
        ser.wait(PROMPT, 20)
        up = settle(name + "-up")
        d_open = frame_delta(base, up) if up else None
        if d_open is None or d_open < MOVED:
            fails.append("%s: opener '%s' changed nothing (%s) - cannot test dismissal"
                         % (name, opener, d_open))
            print("  %-22s OPENER FAILED" % name)
            return False
        dismiss()
        gone = settle(name + "-gone")
        d_back = frame_delta(base, gone) if gone else None
        ok = d_back is not None and d_back < MOVED
        print("  %-22s open %.4f   after-esc-vs-desktop %.4f   %s"
              % (name, d_open, d_back if d_back is not None else -1,
                 "OK" if ok else "FAIL - still up"))
        if not ok:
            fails.append("%s: ESC did not restore the desktop (residue %s)" % (name, d_back))
            # try to recover so the NEXT case is not judged on this one's mess
            ser.send("\x1b"); time.sleep(0.4); qmp.sendkey("esc")
            if not at_base(settle(name + "-recover")):
                fails.append("STOPPED: could not return to the baseline after %s" % name)
                return False
        return True

    print("ESC dismisses, by both roads:")
    for nm, op, dis in (("palette/keyboard",    "palette",    lambda: qmp.sendkey("esc")),
                        ("palette/serial",      "palette",    lambda: ser.send("\x1b")),
                        ("activities/keyboard", "activities", lambda: qmp.sendkey("esc")),
                        ("activities/serial",   "activities", lambda: ser.send("\x1b")),
                        ("lock/serial",         "lock",       lambda: ser.send("\x1b"))):
        if not case(nm, op, dis):
            break

    if at_base(settle("pre-f1")):
        print("F1 opens activities (keycode 0x120, which never matched before):")
        qmp.sendkey("f1")
        up = settle("f1-up")
        d = frame_delta(base, up) if up else None
        ok = d is not None and d >= MOVED
        print("  %-22s delta %.4f   %s" % ("f1", d if d is not None else -1,
                                           "OK" if ok else "FAIL"))
        if not ok:
            fails.append("f1: activities did not open (delta %s)" % d)
        else:
            qmp.sendkey("esc")
    else:
        fails.append("f1: skipped - not at baseline, an earlier case left something up")
finally:
    proc.terminate()

if fails:
    print("\nFAIL")
    for f in fails:
        print("  " + f)
    sys.exit(1)
print("\nPASS - every overlay answers the key it prints, from keyboard and from serial")
