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
gcc -O2 -w -o inputtest_hid inputtest_hid.c ../input.c
echo "built ./inputtest_hid (run: ./inputtest_hid)"

# The compositor, asserted. A compositor's bugs are not crashes - they are a
# sliver of an old window left on the wallpaper, a click landing on the window
# underneath, a drag that stops when the pointer outruns the frame. None of
# those show in a screenshot taken a frame later.
gcc -O2 -w -o wmtest wmtest.c ../wm.c ../ui.c ../wmglue.c ../settings.c hoststubs.c ../fb.c ../input.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmtest        (run: ./wmtest)"
gcc -O2 -w -o wmtest_feel wmtest_feel.c ../wm.c ../ui.c ../wmglue.c ../settings.c hoststubs.c ../fb.c ../input.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmtest_feel"

# ...and one frame of it, as a picture. Assertions catch a click landing on the
# wrong window; eyes catch a title bar four pixels too tall, or a toggle that
# renders as a circle instead of a pill. Both were real, and only the second
# kind is found by looking.
gcc -O2 -w -o wmshot wmshot.c ../wm.c ../ui.c ../wmglue.c ../settings.c hoststubs.c ../fb.c ../input.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmshot        (run: ./wmshot out.ppm)"

# A FRAME, timed - the same trick fbbench plays for fb.c, one layer up. It
# exists because the in-guest frame timer needs QEMU to read, and this box has
# two sessions on four cores: the same drag measured 19,399 us at host load
# 2.25 and 16,000 us at load 7.43, which is not an A/B. Cycles counted here are
# perturbed by cache pressure, not by an order of magnitude, and it attributes
# the cost per app instead of reporting one number.
gcc -O2 -w -o wmbench wmbench.c ../wm.c ../ui.c ../wmglue.c ../settings.c hoststubs.c ../fb.c ../input.c \
    ../term.c ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmbench       (run: ./wmbench)"
# The settings block, against a fake disk. This is the first code in the project
# that WRITES to a disk, and its stated gate needs a booting kernel - so the
# record gets a fake NVMe instead, and every single-bit flip of it is walked.
gcc -O2 -w -o settingstest settingstest.c ../settings.c ../ui.c
echo "built ./settingstest  (run: ./settingstest)"

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

# The browser's parser and box model, asserted. html.c and layout.c reach for
# exactly one thing outside themselves - a function that measures a string - so
# injecting a synthetic one turns both into ordinary programs. Malformed markup
# recovering rather than faulting is unprovable by looking at a rendered page,
# and reflow is a claim about numbers before it is a claim about pixels.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o htmltest htmltest.c ../html.c ../layout.c
echo "built ./htmltest      (run: ./htmltest)"

# ...and the same document at three widths, as a picture. Same argument as
# wmtest/wmshot: assertions catch a run escaping the content box, eyes catch
# inline <code> set at the wrong size or a list marker sitting in its own text.
gcc -O2 -w -o browsershot browsershot.c ../browser.c ../html.c ../layout.c \
    ../ui.c ../fb.c ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c \
    ../http.c ../tcp.c ../net.c ../dns.c
echo "built ./browsershot   (run: ./browsershot out.ppm)"

# ARP, IPv4 and ICMP against scripted packets. net.c holds no link driver - the
# link is two function pointers - so this harness IS the machine on the other
# end of the wire: it answers ARP and ICMP, and it can be told to lose one
# packet in four or to alternate its delay. That makes loss and jitter numbers
# with a known right answer rather than whatever the network did that
# afternoon, and the clock is virtual so the jitter assertion cannot be flaky.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o nettest nettest.c ../net.c
echo "built ./nettest       (run: ./nettest)"

# The TCP state machine against scripted packet sequences. Every case here is
# one that either cannot be produced from a real peer on demand (a segment
# arriving twice, a FIN mid-transfer, a RST with the wrong sequence number) or
# takes minutes of wall clock (five SYN retransmits with exponential backoff).
# The harness IS the peer and the clock is a variable, so all of it is instant.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o tcptest tcptest.c ../tcp.c ../net.c
echo "built ./tcptest       (run: ./tcptest)"

# HTTP/1.0 over the REAL tcp.c, driven by scripted TCP segments. http.c was
# gated end to end against a python http.server, which proves the happy path
# and nothing else - and the happy path is the one case a server will reliably
# give you. Everything that makes a parser wrong is what happens when the
# response is not what you expected: headers split across segments, bare LF
# line endings, a body with no Content-Length, a Content-Length that lies, a
# 3xx with no Location, a type that is not a page. None of those can be asked
# for from a real server; all of them are two lines here.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o httptest httptest.c ../http.c ../tcp.c ../net.c
echo "built ./httptest      (run: ./httptest)"

# The browser app's LOGIC - URL parsing, history, the URL bar's key machine -
# with the drawing stubbed rather than linked. browsershot renders browser.c
# and asserts nothing about it; everything the app does that is not drawing was
# untested, including the one place it takes whatever a person typed. The
# network below it is real: net.c, tcp.c and http.c are all linked, so "did it
# parse the port" is answered by looking at the SYN that went out.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o browsertest browsertest.c ../browser.c \
    ../html.c ../layout.c ../http.c ../tcp.c ../net.c ../dns.c
echo "built ./browsertest   (run: ./browsertest)"

# Every layer that takes bytes from somewhere else, fed garbage. The harnesses
# above check the code does the right thing with inputs someone thought of;
# this checks it does nothing catastrophic with inputs nobody thought of. Build
# it WITH the sanitizers - a clean run without them proves almost nothing.
#   ./fuzz [iterations] [seed]
gcc -O1 -g -w -D_GNU_SOURCE -fsanitize=address,undefined -o fuzz fuzz.c \
    ../html.c ../layout.c ../net.c ../tcp.c ../http.c
echo "built ./fuzz          (run: ./fuzz 3000 1)"

# The resolver, mostly fed answers it should refuse. A DNS response is
# unauthenticated data from a machine we did not choose, parsed by a walk over
# length-prefixed labels with BACKWARD POINTERS in them - nothing in the format
# stops a pointer aimed at itself. Most of this harness is malicious rather
# than merely malformed, and none of it can be asked for from a real server.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o dnstest dnstest.c ../dns.c ../net.c
echo "built ./dnstest       (run: ./dnstest)"
# The input stack, against fake hardware. No GPU, no root, no QEMU - so unlike
# everything else here it can run on any box, in milliseconds.
gcc -O1 -g -Wall -Wextra -Wno-unused-function -o inputtest \
    inputtest.c ../input.c
echo "built ./inputtest     (run: ./inputtest)"
gcc -O1 -g -Wall -Wextra -Wno-unused-function -o inputtest_feel \
    inputtest_feel.c ../input.c
echo "built ./inputtest_feel"
