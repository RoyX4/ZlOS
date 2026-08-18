#!/usr/bin/env bash
# Compile zlOS's Intel driver as a Linux program, against the real GPU.
# The driver source is used unmodified - that is the whole point.
set -e
cd "$(dirname "$0")"
gcc -O1 -g -Wall -DZL_64 -Wno-unused-function -o intel_probe \
    intel_probe.c ../intel.c
echo "built ./intel_probe   (run: sudo ./intel_probe [--unsafe])"

# The modeset harness holds its own mmap and reads raw offsets, because the
# whole point of stage 1 is to settle which offsets are right - it must not go
# through the driver's opinion of where things live.
gcc -O2 -g -Wall -Wextra -o modeset_test modeset_test.c
echo "built ./modeset_test  (run: sudo ./modeset_test --survey)"

# The renderer, timed. Same idea as intel_probe: fb.c is just C against memory,
# so it runs here at native speed with a cycle counter instead of a reboot and
# a stopwatch. Built at the SAME -O2 the kernel uses, or the numbers are fiction.
# No sudo - it maps its own anonymous memory at the addresses fb.c hardcodes.
gcc -O2 -w -o fbbench fbbench.c \
    ../fb.c ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./fbbench       (run: ./fbbench)"

# The event stack, asserted. input.c talks to four functions outside itself, so
# stubbing those turns it into an ordinary program. Every failure mode of
# "push EV_MOUSE from the same pump" is invisible in a screenshot - a phantom
# event at boot, a flood of duplicates, a coalesce that swallows a button - so
# it gets assertions rather than a photograph.
gcc -O2 -w -o inputtest inputtest.c ../input.c
echo "built ./inputtest     (run: ./inputtest)"

# The compositor, asserted. A compositor's bugs are not crashes - they are a
# sliver of an old window left on the wallpaper, a click landing on the window
# underneath, a drag that stops when the pointer outruns the frame. None of
# those show in a screenshot taken a frame later.
gcc -O2 -w -o wmtest wmtest.c ../wm.c ../ui.c ../wmglue.c ../fb.c ../input.c ../notify.c ../snap.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmtest        (run: ./wmtest)"

# ...and one frame of it, as a picture. Assertions catch a click landing on the
# wrong window; eyes catch a title bar four pixels too tall, or a toggle that
# renders as a circle instead of a pill. Both were real, and only the second
# kind is found by looking.
gcc -O2 -w -o wmshot wmshot.c ../wm.c ../ui.c ../wmglue.c ../fb.c ../input.c ../notify.c ../snap.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmshot        (run: ./wmshot out.ppm)"

# The tiled rasterizer against the scanline one it does NOT replace. Two ways
# to fill a polygon are only worth having if they draw the same pixels, and a
# rasterizer one pixel fat along an edge looks fine until two triangles share
# that edge and the mesh cracks.
gcc -O2 -w -o tritest tritest.c ../fb3d.c ../fb.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./tritest       (run: ./tritest)"

# The filesystem, against a RAM disk that can be told to fail a write. fs.c
# talks to storage through three functions, so replacing them with an array is
# the whole of the fake hardware - and it buys the sequences that matter: a
# torn write, a deleted file's blocks being reused under a live neighbour, a
# superblock with one byte flipped in its tail. Built -Wall -Wextra, unlike the
# harnesses above, because this one can lose data.
gcc -O2 -g -Wall -Wextra -Wno-unused-parameter -DFS_HOSTTEST \
    -o fstest fstest.c ../fs.c
echo "built ./fstest        (run: ./fstest)"

# The clipboard, window snapping and notifications. All three are integer logic
# with no framebuffer, and all three have bugs that a screenshot cannot show: a
# clipboard that truncates silently, a snap that overwrites its restore
# rectangle on the SECOND snap, a toast that eats the next keystroke.
# Deliberately not folded into wmtest.c - that harness is being edited in
# another worktree right now, and colliding with it would help nobody.
gcc -O2 -g -Wall -Wextra -Wno-unused-parameter \
    -o systest systest.c ../clip.c ../snap.c ../notify.c
echo "built ./systest       (run: ./systest)"

# The clock. Two port instructions and a pile of decoding, and the bug it
# exists to avoid - a read torn across the second boundary, giving a time an
# hour wrong - lasts a few hundred microseconds a second and cannot be
# reproduced on demand any other way. So the CMOS chip is faked: it can hold
# UIP high, hand out a different time on the second sweep, claim any of the
# three encodings, or not be there at all.
gcc -O2 -g -Wall -Wextra -Wno-unused-parameter -DRTC_HOSTTEST \
    -o rtctest rtctest.c ../rtc.c
echo "built ./rtctest       (run: ./rtctest)"

# The toast INSIDE the compositor. systest asserts notify.c's own queue and
# expiry; this asserts the part that only exists once wm.c is involved - that
# it paints ON TOP of a window, that it leaves no ghost when it retires, and
# that focus never moves, because a toast is not a window and cannot be one.
gcc -O2 -w -o toasttest toasttest.c ../wm.c ../ui.c ../wmglue.c ../fb.c \
    ../input.c ../notify.c ../snap.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./toasttest     (run: ./toasttest)"

# The comparison number: what the REAL GPU on this same laptop does with a
# blended full-screen layer. Offscreen pixmap, so it never touches the desktop.
# Needs libGL - skipped silently if the dev headers are not installed.
if [ -f /usr/include/GL/glx.h ] && [ -f gpu_fillrate.c ]; then
  gcc -O2 -w -o gpu_fillrate gpu_fillrate.c -lGL -lX11
  echo "built ./gpu_fillrate  (run: ./gpu_fillrate)"
fi
