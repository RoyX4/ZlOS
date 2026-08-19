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

# The wallpaper cache against the arena it shares with the blur slots. This is
# the ONLY way 2560x1440 - the ThinkPad's panel - gets exercised before somebody
# flashes a USB stick: GRUB falls back to 800x600 on the emulated card and
# kernel.zl's set_res() ladder stops at 1920x1200, so no boot gate can reach it.
gcc -O2 -w -o walltest walltest.c \
    ../fb.c ../font8x16.c ../font_aa.c ../font_sub.c ../icons.c
echo "built ./walltest      (run: ./walltest)"

# The shell's long lines against the shell's window - DECISIONS.md item G.
# term.c ALONE, against recording stubs, because the defect is that characters
# are LOST and the scissor guarantees no ink escapes the client rect either way:
# a pixel test is green before and after. This asserts on what term_draw asks to
# be drawn instead.
gcc -O2 -w -o termwrap termwrap.c ../term.c
echo "built ./termwrap      (run: ./termwrap)"

# ONE palette across the three files that carry it - DECISIONS.md item E.
# Parses the reference HTML and kernel.zl rather than restating their values,
# because a test that hardcoded the numbers would be a FOURTH copy of the
# palette. ui.c is linked for real. Run from this directory: it opens
# ../kernel.zl, ../settings.c and ../../docs/design/.
gcc -O2 -w -o palette palette.c ../ui.c
echo "built ./palette       (run: ./palette)"

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

# The crypto primitives, against published test vectors. Both of these
# #include ../crypto.c rather than linking it: the file has no header of its
# own for the host path, and compiling it once per harness keeps each one a
# single translation unit with no link order to get wrong.
#
# EVERY expected value in both is from a published standard - FIPS 180/197,
# RFC 2202/4231/6070/4493/5869/7748, NIST SP 800-38D, IEEE 802.11i - and not
# from a previous run. That is what let all of this be written and finished
# with no server to talk to and no hardware present.
gcc -O2 -g -Wall -Wextra -D_GNU_SOURCE -o cryptotest cryptotest.c
echo "built ./cryptotest    (run: ./cryptotest)"
gcc -O2 -g -Wall -Wextra -D_GNU_SOURCE -o tlscryptotest tlscryptotest.c
echo "built ./tlscryptotest (run: ./tlscryptotest)"

# The TLS 1.3 handshake, against OpenSSL. Every primitive underneath is already
# checked against published constants by tlscryptotest; what that cannot check
# is the hundred small ways a handshake goes wrong - a length written
# little-endian, an extension out of order, a transcript hash taken one message
# too late, a nonce that does not advance, the record header left out of the
# additional data. None of those produce a wrong constant anywhere; all of them
# produce a handshake that fails. So this talks to `openssl s_server` and
# asserts interoperability, which is the only property a client actually needs.
# Skips rather than fails when openssl is absent.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o tlstest tlstest.c ../tls.c ../crypto.c ../x509.c ../ecdsa.c ../rsa.c ../roots.c
echo "built ./tlstest       (run: ./tlstest)"

# The bounded JavaScript interpreter. The SCOPE CLAIM in js.h is under test as
# much as the code: section 7 asserts that what it cannot do fails cleanly with
# a message, because a parser that accepts garbage and evaluates it to 0 is
# worse than one that refuses.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o jstest jstest.c ../js.c
echo "built ./jstest        (run: ./jstest)"

# ECDSA verification, P-256 and P-384. A VERIFIER IS TESTED BY WHAT IT REJECTS:
# an implementation that returns 1 unconditionally passes every "valid signature
# verifies" test ever written, and one that returns 0 unconditionally passes
# every rejection test. Both halves are here, and the second half caught a real
# bug - two incompatible Jacobian addition formulas, which rejected everything.
gcc -O2 -g -Wall -Wextra -D_GNU_SOURCE -o ecdsatest ecdsatest.c ../ecdsa.c
echo "built ./ecdsatest     (run: ./ecdsatest)"

# Certificate parsing and chain validation - the half of TLS that decides
# whether "encrypted" means "encrypted TO THEM". Written the way ecdsatest is:
# a validator that returns 1 unconditionally passes every accept-the-real-chain
# test ever written, so the accept case is checked once and every way of being
# wrong is checked individually. The certificates are en.wikipedia.org's REAL
# chain, captured off the wire - a parser that only meets certificates made by
# its own author agrees with its author, not with a certificate authority.
gcc -O2 -g -Wall -Wextra -D_GNU_SOURCE -o x509test x509test.c ../x509.c ../ecdsa.c ../rsa.c ../crypto.c
echo "built ./x509test      (run: ./x509test)"

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

# NOT A GATE - a measuring instrument, and the only one that answers "does a
# real page fit". Every cap number in browser-render-run.md §11 and §12 came
# from a throwaway program that was thrown away, so the next person to raise
# HTML_MAX_NODES or CSS_MAX_SELS had to either trust a document or rebuild the
# measurement from scratch. It builds here so that it exists; it asserts
# nothing, because its right answer depends on which page you fed it.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o parsestat parsestat.c \
    ../html.c ../css.c ../layout.c
echo "built ./parsestat     (run: ./parsestat page.html sheet.css [viewport])"

# ...and the same document at three widths, as a picture. Same argument as
# wmtest/wmshot: assertions catch a run escaping the content box, eyes catch
# inline <code> set at the wrong size or a list marker sitting in its own text.
gcc -O2 -w -o browsershot browsershot.c ../browser.c ../html.c ../css.c ../layout.c ../png.c \
    ../tls.c ../crypto.c ../x509.c ../ecdsa.c ../rsa.c ../roots.c ../entropy.c ../js.c hostmachine.c \
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
#
# THIS LINE WAS STALE AND THE GATE COULD NOT BUILD AT ALL. http.c gained the
# TLS transport when https landed, and this link line was never updated - so
# `httptest` had 18 undefined references and produced no binary, while
# browser-status.md went on citing "91 checks, 0 failed" from the last time it
# ran. A gate that cannot build is indistinguishable from a gate that passes
# if nobody looks at the build output, and this script prints a lot of it.
# Verified by rebuilding HEAD's http.c against HEAD's httptest.c: same 18.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o httptest httptest.c ../http.c ../tcp.c ../net.c \
    ../tls.c ../crypto.c ../x509.c ../ecdsa.c ../rsa.c ../roots.c ../entropy.c hostmachine.c
echo "built ./httptest      (run: ./httptest)"

# The browser app's LOGIC - URL parsing, history, the URL bar's key machine -
# with the drawing stubbed rather than linked. browsershot renders browser.c
# and asserts nothing about it; everything the app does that is not drawing was
# untested, including the one place it takes whatever a person typed. The
# network below it is real: net.c, tcp.c and http.c are all linked, so "did it
# parse the port" is answered by looking at the SYN that went out.
gcc -O1 -g -Wall -Wextra -D_GNU_SOURCE -o browsertest browsertest.c ../browser.c \
    ../html.c ../css.c ../layout.c ../png.c ../http.c ../tcp.c ../net.c ../dns.c \
    ../tls.c ../crypto.c ../x509.c ../ecdsa.c ../rsa.c ../roots.c ../entropy.c ../js.c hostmachine.c
echo "built ./browsertest   (run: ./browsertest)"

# ...AND THE SAME HARNESS WITH THE SANITIZERS ON, which is not redundant: this
# is the gap that let a signed-overflow bug through. `fuzz` below is built with
# ASan and UBSan and the PNG decoder was fuzzed under them, but browser.c sits
# BETWEEN the two and was covered by neither - so b64_decode shifted a signed
# int past 31 bits on the home page's own inline image, every single time, and
# 103 green checks said nothing. A clean run without the sanitizers proves
# almost nothing; that sentence was already written in this file, about a
# different harness.
gcc -O1 -g -w -D_GNU_SOURCE -fsanitize=address,undefined -fno-sanitize-recover=all \
    -o browsertest_san browsertest.c ../browser.c \
    ../html.c ../css.c ../layout.c ../png.c ../http.c ../tcp.c ../net.c ../dns.c \
    ../tls.c ../crypto.c ../x509.c ../ecdsa.c ../rsa.c ../roots.c ../entropy.c ../js.c hostmachine.c
echo "built ./browsertest_san (run: ./browsertest_san)"

# Every layer that takes bytes from somewhere else, fed garbage. The harnesses
# above check the code does the right thing with inputs someone thought of;
# this checks it does nothing catastrophic with inputs nobody thought of. Build
# it WITH the sanitizers - a clean run without them proves almost nothing.
#   ./fuzz [iterations] [seed]
#
# THIS LINE WAS STALE TOO, and this is the gate that found four real layout
# defects nobody would have typed. Same cause as httptest above: http.c gained
# the TLS transport and neither link line followed it, so the fuzzer has not
# built - and therefore has not run - since https landed. Verified against a
# clean `git archive HEAD` tree, not against the working copy: the first A/B I
# ran compiled HEAD's layout.c against the NEW css.h, which failed at COMPILE
# and never reached the link, so "0 undefined references" was true for entirely
# the wrong reason. A measurement that can be right by accident is not one.
gcc -O1 -g -w -D_GNU_SOURCE -fsanitize=address,undefined -o fuzz fuzz.c \
    ../html.c ../css.c ../layout.c ../png.c ../net.c ../tcp.c ../http.c \
    ../tls.c ../crypto.c ../x509.c ../ecdsa.c ../rsa.c ../roots.c ../entropy.c \
    hostmachine.c
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

# The xHCI EVENT RING, with this harness playing the controller. The layer
# inputtest.c cannot reach: it stubs xhci_ptr_poll() as `return 0`, so the ring
# that the pointer bug actually lived in is not in its picture at all. Here the
# real xhci.c is compiled unmodified (included, not linked, to reach the static
# state an enumeration would have set) and driven by a fake controller that
# walks the transfer ring, honours the cycle bit and the Link TRB, and fills the
# buffer each TRB names.
#
# It exists because every probe-*.py in this repo attaches a usb-TABLET while
# try.sh attaches a usb-MOUSE, so the relative path a person actually uses had
# no coverage of any kind. -DZL_64 only widens xhci.c's `uptr`; every DMA
# address it puts in a ring stays 32-bit, exactly as in the kernel.
gcc -O1 -g -w -D_GNU_SOURCE -DZL_64 -o xhcitest xhcitest.c ../input.c
echo "built ./xhcitest      (run: ./xhcitest)"
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

# The general allocator, and this one is built with the WARNINGS ON rather than
# -w. arena.c is a bump pointer and an addition; heap.c has boundary tags, two
# levels of size class and three bitmaps, so it is the file in this tree where
# an unused variable or a signed/unsigned comparison is most likely to be an
# actual bug rather than noise. It found nothing on the way in, which is worth
# rather more than -w finding nothing.
#
# Links the REAL heap.c, unmodified. The harness mmaps HEAP_BASE and supplies
# putc, exactly as arenatest does for the arena.
gcc -O2 -g -Wall -Wextra -Wno-unused-parameter -o heaptest heaptest.c ../heap.c
echo "built ./heaptest      (run: ./heaptest)"

# The virtual-memory arithmetic. NOT the mapping - installing a PDPT entry needs
# CR3 and ring 0, and the only proof of that is verify-efi.sh booting green. But
# vmm_phys()/vmm_virt() are called from dma_addr() on every keystroke, mouse
# report, disk block and network frame, and they are the twelve lines where an
# off-by-one hands a device an address one page out.
gcc -O2 -g -Wall -Wextra -o pagingtest pagingtest.c ../paging.c
echo "built ./pagingtest    (run: ./pagingtest)"

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

# THE BLITTER. The first thing in this project that asks a GPU to draw.
#
# Raw ioctl on /dev/dri/renderD128 - no libdrm, no Mesa, no -l flags at all,
# because the point is to learn the command level zlOS will have to speak and
# zlOS cannot link a library. It runs ALONGSIDE i915 and does not detach it:
# the blitter is a DMA engine, not the display, so unlike modeset-run.sh this
# cannot blank the screen of whoever is using the laptop.
#
# Guarded on the header AND on hardware being present, so a box with no Intel
# GPU skips instead of failing. `--negative` is the one that matters in CI: it
# proves the verification can still reject a blit that writes nothing.
if [ -f /usr/include/drm/i915_drm.h ]; then
  gcc -O2 -g -Wall -Wextra -o gpu_blt gpu_blt.c
  if [ -e /dev/dri/renderD128 ]; then
    echo "built ./gpu_blt       (run: ./gpu_blt --blit --negative, or --sweep)"
  else
    echo "built ./gpu_blt       (no /dev/dri/renderD128 here - it will skip at run time)"
  fi
else
  echo "skip  ./gpu_blt       (no drm headers - apt install libdrm-dev)"
fi

# The blitter command stream, asserted against the dwords that really drew.
# gpu_blt proves the encoding on the GPU; this proves it on any machine in
# milliseconds, and covers what hardware cannot reach cheaply - the refusals,
# and a batch-buffer overflow, which in the kernel means a DMA engine parsing
# whatever followed the batch in memory. Both mutations were watched failing
# it before it was committed.
gcc -O2 -g -Wall -Wextra -o gputest gputest.c
echo "built ./gputest       (run: ./gputest)"

# The ring experiment. Builds anywhere; needs root AND i915 unbound to do
# anything, so it is not part of any gate - gpu-ring-run.sh drives it and always
# gives the display back. --survey is read-only and safe with i915 loaded.
gcc -O2 -g -Wall -Wextra -o gpu_ring gpu_ring.c
echo "built ./gpu_ring      (run: sudo ./gpu-ring-run.sh --survey)"

# The plane registers, read rather than assumed. intel.c's method is "verified
# against what firmware programmed"; the timing registers had modeset_test and
# the PLANE registers had no witness at all. Read-only, 77 without root or
# without an Intel GPU.
gcc -O2 -g -Wall -Wextra -o gpu_planes gpu_planes.c
echo "built ./gpu_planes    (run: sudo ./gpu_planes)"

# GPU-visible memory through the aperture (GMADR/BAR2). This is how anything
# confirms the GPU wrote something WITHOUT trusting the GPU - /dev/mem is refused
# for normal RAM on this kernel (STRICT_DEVMEM=y) and a GEM buffer only works
# while i915 is driving. Read-only, 77 without root or without an Intel BAR2.
gcc -O2 -g -Wall -Wextra -o gpu_aperture gpu_aperture.c
echo "built ./gpu_aperture  (run: sudo ./gpu_aperture)"
