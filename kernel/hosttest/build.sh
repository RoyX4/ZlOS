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
gcc -O2 -w -o wmtest wmtest.c ../wm.c ../ui.c ../wmglue.c ../fb.c ../input.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmtest        (run: ./wmtest)"

# ...and one frame of it, as a picture. Assertions catch a click landing on the
# wrong window; eyes catch a title bar four pixels too tall, or a toggle that
# renders as a circle instead of a pill. Both were real, and only the second
# kind is found by looking.
gcc -O2 -w -o wmshot wmshot.c ../wm.c ../ui.c ../wmglue.c ../fb.c ../input.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmshot        (run: ./wmshot out.ppm)"

# A FRAME, timed - the same trick fbbench plays for fb.c, one layer up. It
# exists because the in-guest frame timer needs QEMU to read, and this box has
# two sessions on four cores: the same drag measured 19,399 us at host load
# 2.25 and 16,000 us at load 7.43, which is not an A/B. Cycles counted here are
# perturbed by cache pressure, not by an order of magnitude, and it attributes
# the cost per app instead of reporting one number.
gcc -O2 -w -o wmbench wmbench.c ../wm.c ../ui.c ../wmglue.c ../fb.c ../input.c \
    ../term.c ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmbench       (run: ./wmbench)"

# The tiled rasterizer against the scanline one it does NOT replace. Two ways
# to fill a polygon are only worth having if they draw the same pixels, and a
# rasterizer one pixel fat along an edge looks fine until two triangles share
# that edge and the mesh cracks.
gcc -O2 -w -o tritest tritest.c ../fb3d.c ../fb.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./tritest       (run: ./tritest)"

# The comparison number: what the REAL GPU on this same laptop does with a
# blended full-screen layer. Offscreen pixmap, so it never touches the desktop.
# Needs libGL - skipped silently if the dev headers are not installed.
if [ -f /usr/include/GL/glx.h ]; then
  gcc -O2 -w -o gpu_fillrate gpu_fillrate.c -lGL -lX11
  echo "built ./gpu_fillrate  (run: ./gpu_fillrate)"
fi
