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
gcc -O2 -w -pthread -o fbbench fbbench.c \
    ../fb.c ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./fbbench       (run: ./fbbench)"

# The proportional text engine, asserted. fbbench times fb.c and browsershot
# photographs it; neither NOTICES when a style flag stops changing the pixels.
# Both regressions this gate exists for shipped green: italic silently rendered
# upright, and six heading sizes collapsed onto two, because a line count does
# not move when a heading that already fits on one line is set too small.
# Run against the pre-fix fb.c it reports 12 failures, which is the only reason
# to believe it would catch the next one.
gcc -O2 -g -Wall -Wextra -Wno-unused-parameter -D_GNU_SOURCE -o fbtext fbtext.c \
    ../fb.c ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./fbtext        (run: ./fbtext)"

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
gcc -O2 -w -o wmtest wmtest.c ../wm.c ../notify.c ../snap.c ../ui.c ../wmglue.c ../settings.c hoststubs.c ../fb.c ../input.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmtest        (run: ./wmtest)"
gcc -O2 -w -o wmtest_feel wmtest_feel.c ../wm.c ../notify.c ../snap.c ../ui.c ../wmglue.c ../settings.c hoststubs.c ../fb.c ../input.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmtest_feel"

# ...and one frame of it, as a picture. Assertions catch a click landing on the
# wrong window; eyes catch a title bar four pixels too tall, or a toggle that
# renders as a circle instead of a pill. Both were real, and only the second
# kind is found by looking.
gcc -O2 -w -o wmshot wmshot.c ../wm.c ../notify.c ../snap.c ../ui.c ../wmglue.c ../settings.c hoststubs.c ../fb.c ../input.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./wmshot        (run: ./wmshot out.ppm)"

# A FRAME, timed - the same trick fbbench plays for fb.c, one layer up. It
# exists because the in-guest frame timer needs QEMU to read, and this box has
# two sessions on four cores: the same drag measured 19,399 us at host load
# 2.25 and 16,000 us at load 7.43, which is not an A/B. Cycles counted here are
# perturbed by cache pressure, not by an order of magnitude, and it attributes
# the cost per app instead of reporting one number.
gcc -O2 -w -o wmbench wmbench.c ../wm.c ../notify.c ../snap.c ../ui.c ../wmglue.c ../settings.c hoststubs.c ../fb.c ../input.c \
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
#
# Guarded on BOTH the headers and the source. It used to check only
# /usr/include/GL/glx.h, and gpu_fillrate.c has never been committed to any
# branch - so on a machine with the GL headers installed (this one) every run
# of this script ended:
#
#     cc1: fatal error: gpu_fillrate.c: No such file or directory
#
# after the seven harnesses above had built and passed. A fatal error at the
# END of a successful build reads as noise, which is how it survived. Same
# class as the four source lists this repo just consolidated: a build script
# naming a file that is not there.
if [ -f /usr/include/GL/glx.h ] && [ -f gpu_fillrate.c ]; then
  gcc -O2 -w -o gpu_fillrate gpu_fillrate.c -lGL -lX11
  echo "built ./gpu_fillrate  (run: ./gpu_fillrate)"
elif [ -f /usr/include/GL/glx.h ]; then
  echo "skip  ./gpu_fillrate  (gpu_fillrate.c is not in the tree)"
else
  echo "skip  ./gpu_fillrate  (no GL headers - apt install libgl1-mesa-dev)"
fi

# The browser's parser and box model, asserted. html.c and layout.c reach for
# exactly one thing outside themselves - a function that measures a string - so
# injecting a synthetic one turns both into ordinary programs. Malformed markup
# recovering rather than faulting is unprovable by looking at a rendered page,
# and reflow is a claim about numbers before it is a claim about pixels.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o htmltest htmltest.c ../html.c ../css.c ../layout.c
echo "built ./htmltest      (run: ./htmltest)"

# The CSS engine, against hand-written stylesheets. css.c takes UNTRUSTED TEXT
# from a machine we did not choose and turns it into numbers that move pixels -
# the same trust boundary dns.c sits on - so most of this harness is malformed
# or hostile rather than merely unusual. It needs no html.c and no layout.c:
# matching takes an explicit ancestor path, so the engine is testable with
# nothing else linked at all.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o csstest csstest.c ../css.c
echo "built ./csstest       (run: ./csstest)"

# ...and the same document at three widths, as a picture. Same argument as
# wmtest/wmshot: assertions catch a run escaping the content box, eyes catch
# inline <code> set at the wrong size or a list marker sitting in its own text.
gcc -O2 -w -o browsershot browsershot.c ../browser.c ../html.c ../css.c ../layout.c \
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
    ../html.c ../css.c ../layout.c ../http.c ../tcp.c ../net.c ../dns.c
echo "built ./browsertest   (run: ./browsertest)"

# Every layer that takes bytes from somewhere else, fed garbage. The harnesses
# above check the code does the right thing with inputs someone thought of;
# this checks it does nothing catastrophic with inputs nobody thought of. Build
# it WITH the sanitizers - a clean run without them proves almost nothing.
#   ./fuzz [iterations] [seed]
gcc -O1 -g -w -D_GNU_SOURCE -fsanitize=address,undefined -o fuzz fuzz.c \
    ../html.c ../css.c ../layout.c ../net.c ../tcp.c ../http.c
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

# The program arena, asserted. This one is the cheapest gate in the project -
# no QEMU, no GPU, no sudo - because arena.c is just arithmetic against memory,
# and the arithmetic is the entire point: a ceiling test written as an addition
# instead of a subtraction hands a script the whole machine. Built at the same
# -O2 the kernel uses; the harness mmaps the address arena.c hardcodes so the
# shipping source compiles unmodified.
# The error boundary: twenty lines of assembly the whole kill path rests on.
# Built for BOTH architectures, because a setjmp that forgets a callee-saved
# register does not crash - it hands back a stale value after an unwind, and
# the symptom surfaces in whatever loop was using that register, in another
# file, later. The i386 build is the one the kernel actually ships.
gcc -O2 -w -o jmptest   jmptest.c ../ksetjmp.S
gcc -m32 -O2 -w -o jmptest32 jmptest.c ../ksetjmp.S
echo "built ./jmptest       (run: ./jmptest && ./jmptest32)"

# The kernel's libc replacements, against the real libc. Fourteen functions,
# each a dozen lines, each with a well-known way to be SUBTLY wrong - signed
# chars in strcmp, strncpy forgetting to pad, memmove copying forwards through
# an overlap. None of those crash; they return a plausible wrong answer inside
# an interpreter running somebody's script. So they are checked against glibc's
# own on tens of thousands of generated inputs rather than against what I
# thought to test. Links the REAL arena.c, so the allocator is exercised for
# real rather than stubbed.
gcc -O2 -w -o libctest libctest.c ../interp_kernel.c ../arena.c -lm
echo "built ./libctest      (run: ./libctest)"

gcc -O2 -w -o arenatest arenatest.c ../arena.c
echo "built ./arenatest     (run: ./arenatest)"

# `run`, and every way it declines. TWO binaries from one source, which is the
# point of exec.c's weak fs_* references: with a filesystem linked it reaches
# not-found / empty / too-big / loaded; with nothing defining fs_* the weak
# symbols are NULL and it says "no fs driver" instead. Since fs.c was merged the
# FIRST is what the kernel ships - and exec.c did not change by a character to
# make that happen. The second still earns its place: it is the only proof that
# the NULL-weak branch is reached rather than merely written, and it is the
# branch any build without fs.c still lands on.
gcc -O2 -w -o exectest exectest.c ../exec.c
echo "built ./exectest      (run: ./exectest)"

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
gcc -O2 -w -o toasttest toasttest.c ../wm.c ../ui.c ../wmglue.c ../settings.c hoststubs.c ../fb.c \
    ../input.c ../notify.c ../snap.c \
    ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./toasttest     (run: ./toasttest)"
