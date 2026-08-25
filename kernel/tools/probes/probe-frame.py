#!/usr/bin/env python3
"""probe-frame.py - what does a frame actually COST, on a real boot?

desktop-TODO 0h: "add a tsc() builtin and put frame time on screen. DO THIS
BEFORE ANY PERFORMANCE WORK. Optimising without measurement is guessing." The
v10 run did the performance work first and this never. So every claim about the
compositor's speed up to now was arithmetic from fbbench's per-primitive
numbers, which is not the same thing as a frame.

wm_frame() times itself with the TSC now, in MICROseconds - a cheap frame is
well under 1 ms and integer milliseconds would report every one of them as
zero - and only frames that actually repaint are counted, or a desktop at rest
would average out as infinitely fast.

This drives real interaction and reads the retained 256-frame sample ring by
OCR of nothing. It reports frame and causal input-to-CPU-present p50/p95/p99;
the raw samples remain in the serial transcript.
"""
import argparse, os, subprocess, sys, tempfile, re

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
HERE = KERNEL_ROOT
sys.path.insert(0, PROBE_DIR)
from exercise import Serial, Qmp, qemu_argv, build, PROMPT, ppm_sample


def at(qmp, x, y, w, h, btn=None):
    ev = [{"type": "abs", "data": {"axis": "x", "value": int(x * 32767 / w)}},
          {"type": "abs", "data": {"axis": "y", "value": int(y * 32767 / h)}}]
    if btn is not None:
        ev.append({"type": "btn", "data": {"down": btn, "button": "left"}})
    qmp.cmd("input-send-event", events=ev)


def peak(ser):
    """Read the frame timer. The shell prints ONE line with a marker at each
    end, and this waits for the END marker - waiting for a substring that
    appears twice in the answer returns halfway through it."""
    ser.send("peak\r")
    ok, out = ser.wait("END", 30)
    m = re.search(r"FRAMEUS\s+(\d+)\s+(\d+)\s+END", out)
    return (int(m.group(1)), int(m.group(2))) if m else (None, None)


def percentile(values, pct):
    if not values:
        return None
    ordered = sorted(values)
    return ordered[min(len(ordered) - 1,
                       max(0, (len(ordered) * pct + 99) // 100 - 1))]


def perf(ser, label):
    ser.send("perf\r")
    ok, out = ser.wait("PERFEND", 30)
    if not ok:
        print(f"  {label:<28} no PERFEND marker")
        return False
    start = out.rfind("  PERF ")
    block = out[start:] if start >= 0 else out
    pairs = [(int(a), int(b)) for a, b in
             re.findall(r"(?m)^  P \d+ (\d+) (\d+)\s*$", block)]
    frames = [a for a, _ in pairs]
    inputs = [b for _, b in pairs if b > 0]
    fp = [percentile(frames, p) for p in (50, 95, 99)]
    ip = [percentile(inputs, p) for p in (50, 95, 99)]
    print(f"  {label:<28} n={len(frames):3d}  frame p50/p95/p99 "
          f"{fp[0]}/{fp[1]}/{fp[2]} us  input {ip[0]}/{ip[1]}/{ip[2]} us")
    return bool(frames)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()
    if not args.no_build:
        build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-frame-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        if not ser.wait("ready.", 240)[0]:
            print("never booted"); return 1
        ser.wait(PROMPT, 60)
        ser.drain(2.0)
        shot = os.path.join(tmp, "frame-mode.ppm")
        if not qmp.screendump(shot):
            print("could not read the live framebuffer geometry"); return 1
        W, H, _ = ppm_sample(shot)
        ui = max(1, min(3, ((W * 256 + 960) // 1920 + 128) // 256))
        print(f"LIVE MODE {W}x{H}, integer UI scale {ui}")
        dock_bar_w = 395
        dock_x0 = (W - dock_bar_w * ui) // 2 + 7 * ui
        dock_pitch = 38 * ui
        dock_y = H - (43 + 9) * ui

        print("BOOT (the full-screen repaint plus baking the wallpaper)")
        last, pk = peak(ser)
        print(f"  after boot                 last {last} us   peak {pk} us")

        ser.send("peakreset\r"); ser.drain(0.6)

        # sweep the pointer across the dock: hover changes damage the dock strip
        print("\nHOVERING ACROSS THE DOCK (the strip that got expensive)")
        dy = dock_y
        for i in range(9):
            at(qmp, dock_x0 + i * dock_pitch + 16 * ui,
               dy + 21 * ui, W, H)
            ser.drain(0.25)
        last, pk = peak(ser)
        print(f"  nine tiles hovered         last {last} us   peak {pk} us")
        perf(ser, "dock hover")

        ser.send("peakreset\r"); ser.drain(0.6)

        # drag a window right across the screen - the most damage a frame gets
        print("\nDRAGGING A WINDOW ACROSS THE SCREEN")
        desk_ox = max(0, (W - 1280 * ui) // 2)
        mon_x, mon_y = desk_ox + 758 * ui, 76 * ui
        at(qmp, mon_x + 60 * ui, mon_y + 10 * ui, W, H, btn=True)
        for k in range(12):
            at(qmp, mon_x + 60 * ui - k * 45 * ui,
               mon_y + 10 * ui + k * 20 * ui, W, H, btn=True)
            ser.drain(0.2)
        at(qmp, mon_x + 60 * ui - 11 * 45 * ui,
           mon_y + 10 * ui + 11 * 20 * ui, W, H, btn=False)
        ser.drain(0.8)
        # CLICK THE SHELL BACK FIRST. Dragging a window focuses it, and keys go
        # to the focused window - so `peak` was being typed into the System
        # Monitor, which has no app_event, and the probe reported None. That is
        # the routing working exactly as designed, and the probe not knowing it.
        # Use the Terminal's dock tile, not a guessed client coordinate. The
        # dragged monitor can end over any fixed point in the workspace; the
        # dock is outside the window stack and its tile deterministically
        # raises/focuses the shell.
        shell_dx, shell_dy = dock_x0 + 16 * ui, dock_y + 21 * ui
        at(qmp, shell_dx, shell_dy, W, H, btn=True)
        at(qmp, shell_dx, shell_dy, W, H, btn=False)
        ser.drain(0.8)
        last, pk = peak(ser)
        print(f"  twelve drag steps          last {last} us   peak {pk} us")
        good = perf(ser, "stacked-window drag")
        if pk:
            print(f"\n  budget is 16667 us (60 fps). peak is "
                  f"{pk / 16667.0:.2f}x that -> {1e6 / pk:.1f} fps worst frame")
        return 0 if good else 1
    finally:
        proc.kill(); proc.wait()


if __name__ == "__main__":
    sys.exit(main())
