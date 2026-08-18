#!/usr/bin/env python3
"""probe-anim.py - does an animated demo actually reach the screen?

exercise.py reported 0% of the screen changing while the 3D cube, the
animation, snake, paint and the editor were running. That is either a real
defect or an artefact of photographing one frame. A spinning cube cannot
produce four identical frames, so: start the demo, take four pictures spaced
out over several seconds, and compare them to each other.

  ./probe-anim.py v      the 3D cube
  ./probe-anim.py a      the bouncing logo
"""
import os, subprocess, sys, tempfile, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, ppm_sample, frame_delta, qemu_argv, build, PROMPT

key = sys.argv[1] if len(sys.argv) > 1 else "v"
build(False)
tmp = tempfile.mkdtemp(prefix="zlos-probe-")
ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
proc = subprocess.Popen(qemu_argv(tmp, False, ser_path, qmp_path),
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
try:
    ser, qmp = Serial(ser_path), Qmp(qmp_path)
    ser.wait("ready.", 180)
    ser.wait(PROMPT, 30)
    print(f"booted; starting demo {key!r}")

    shots = []
    p0 = os.path.join(tmp, "f0.ppm")
    qmp.screendump(p0); shots.append(("before", ppm_sample(p0)))

    ser.send(key)
    for i in range(4):
        ser.drain(1.5)
        p = os.path.join(tmp, f"f{i+1}.ppm")
        qmp.screendump(p)
        shots.append((f"t+{(i+1)*1.5:.1f}s", ppm_sample(p)))

    base = shots[0][1]
    print("\n  frame      vs before   vs previous")
    prev = base
    for label, s in shots[1:]:
        print(f"  {label:<9}  {frame_delta(base, s)*100:6.2f}%     "
              f"{frame_delta(prev, s)*100:6.2f}%")
        prev = s
    moving = any(frame_delta(shots[i][1], shots[i + 1][1]) > 0.001
                 for i in range(1, len(shots) - 1))
    print("\n  VERDICT:", "the demo is animating on screen" if moving
          else "NOTHING reaches the card - every frame is identical")
    ser.send(" ")
finally:
    proc.terminate()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        proc.kill()
