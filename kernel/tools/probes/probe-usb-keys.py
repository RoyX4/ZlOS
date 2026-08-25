#!/usr/bin/env python3
"""Do non-character keys reach the guest over USB?

Two things were broken and they need different evidence.

  ESC.  The '=' demo exits on EV_KEY_DOWN with KEY_ESC. The USB path only ever
        pushed EV_CHAR, so ESC did nothing and the demo ended by running out
        its 30-second timer. From outside, "exited on ESC" and "timed out" look
        identical - both end at the prompt - which is exactly why the sweep
        scored this step "ok" for four rounds while it was broken. The evidence
        that separates them is TIME: pressing ESC ends it in about a second,
        the timeout takes thirty.

  Arrows. The demo prints "try caps lock, ctrl, shift, arrows". hid_to_ascii()
        returned 0 for the arrow usages, which is the same value as "no key",
        so they were dropped in the driver. They should now arrive as
        EV_KEY_DOWN with codes 0x110-0x113.

Run against a build BEFORE the fix and this reports esc_seconds ~30 and no
arrow lines; after, ~1 second and four arrows.
"""
import os, subprocess, sys, tempfile, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, PROMPT

ARROWS = {0x110: "right", 0x111: "left", 0x112: "down", 0x113: "up"}
TIMEOUT_S = 30          # the demo's own self-end, from kernel.zl
ESC_FAST_S = 10         # anything under this could not have been the timeout

build(False)
tmp = tempfile.mkdtemp(prefix="zlos-usbkeys-")
ser_p, qmp_p = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
proc = subprocess.Popen(qemu_argv(tmp, False, ser_p, qmp_p),
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
rc = 1
try:
    ser, qmp = Serial(ser_p), Qmp(qmp_p)
    ser.wait("ready.", 180)
    ser.wait(PROMPT, 30)

    ser.buf = ""
    ser.send("=")
    ser.wait("ESC ends.", 40)

    print("injecting the four arrows, then ESC\n")
    for k in ("up", "down", "left", "right"):
        qmp.sendkey(k)
        ser.drain(0.6)

    t0 = time.monotonic()
    qmp.sendkey("esc")
    reached, tail = ser.wait(PROMPT, 90)
    esc_s = time.monotonic() - t0

    lines = [ln.strip() for ln in tail.splitlines()
             if "down " in ln or "char " in ln or "up   " in ln]
    for ln in lines:
        print(f"    {ln}")

    codes = set()
    for ln in lines:
        if "down " in ln and "code 0x" in ln:
            try:
                codes.add(int(ln.split("code 0x")[1].split()[0], 16))
            except ValueError:
                pass
    found = sorted(c for c in codes if c in ARROWS)

    print()
    print(f"  reached prompt      : {reached}")
    print(f"  seconds to exit     : {esc_s:.1f}   (timeout is {TIMEOUT_S})")
    print(f"  arrow keys arrived  : {len(found)}/4 "
          f"{[ARROWS[c] for c in found]}")
    print()

    esc_ok = reached and esc_s < ESC_FAST_S
    if not reached:
        print("  FAIL  never returned to the prompt at all")
    elif not esc_ok:
        print(f"  FAIL  took {esc_s:.1f}s - that is the demo timing out, not ESC")
        print("        ESC is still not reaching the guest as a key event")
    elif len(found) != 4:
        print(f"  FAIL  ESC works, but {4 - len(found)} arrow(s) never arrived")
    else:
        print("  PASS  ESC exits the demo and all four arrows arrive as keys")
        rc = 0
finally:
    proc.terminate()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        proc.kill()
sys.exit(rc)
