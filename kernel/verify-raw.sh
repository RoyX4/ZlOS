#!/usr/bin/env bash
# verify-raw.sh - boot the kernel with OUR bootloader (no GRUB), TWICE.
#
# Proves raw_boot.asm loads the kernel, enters protected mode and hands over
# correctly, independent of GRUB and multiboot entirely.
#
# WHY TWO BOOTS. The compositor is the boot state now: kernel.zl ends in
# `if wm_avail() == 0 { ...text shell... } else { wm_session() }`, and
# raw_boot.asm asks the card for a linear framebuffer through VBE itself. So
# the raw path HAS a framebuffer and takes the compositor branch - and the
# compositor cannot be driven over serial, because wm_frame's only input is
# input_poll() -> PS/2 and USB HID. Nothing in it looks at COM1.
#
# That is correct behaviour and it broke this gate, which drove the shell by
# typing `.h20fq` down the serial line and grepping for 6765. Rather than
# weaken the assertion to "it booted", each half is now checked where it can
# actually be checked:
#
#   TEXT leg (-vga none)   no card, so VBE fails, the console falls back to VGA
#                          text and the OLD assertions run UNCHANGED: the
#                          bootloader banner, the prompt, and fib(20) = 6765
#                          computed by the zl runtime and echoed over serial.
#
#   GRAPHICAL leg          the default card, i.e. what real hardware looks
#                          like. Asserts the framebuffer console came up and
#                          the compositor opened its windows - which is
#                          strictly MORE than the text leg proves, since it
#                          also covers VBE modesetting, fb_setup, the zl->C
#                          bind in wmglue.c, and three wm_open calls.
#
# Neither leg waits a fixed wall-clock time. That used to be `timeout 30`, and
# under TCG the boot plus the help text is a couple of KiB over a simulated
# 115200 line, taking anywhere from 12 s to well past 30 s on the same
# unchanged kernel depending on host load - so the gate reported failures that
# were load, not behaviour, which is worse than no gate: it costs a bisect
# every time. Poll for the marker with a generous ceiling instead.
set -uo pipefail
cd "$(dirname "$0")"

command -v qemu-system-i386 >/dev/null || { echo "skip: no qemu"; exit 0; }
command -v nasm >/dev/null || { echo "skip: no nasm"; exit 0; }

./mkdisk.sh >/dev/null 2>&1 || { echo "FAIL: disk image did not build"; exit 1; }

CEILING=180
# THE KEYS CHANGED WITH THE INVERSION, and the reason is worth stating: this
# path boots through raw_boot.asm, which asks the card for a linear framebuffer
# itself - so px_w() is non-zero, the COMPOSITOR is the boot state here, and
# the shell is a window inside it. Single keypresses are not commands any more.
# `.h20fq` therefore typed five characters into a line buffer and pressed
# nothing, and the gate correctly reported an unresponsive shell.
#
# The leading '.' is still the throwaway QEMU may eat before the guest runs;
# the \r after it flushes whatever survived as one empty or unknown line.
printf '.\rhelp\rfib 20\rquit\r' | timeout "$CEILING" qemu-system-i386 \
    -drive file=zlOS.img,format=raw -serial stdio -display none -no-reboot \
    >"$OUT" 2>/dev/null &
QPID=$!
for _ in $(seq $((CEILING * 2))); do
    grep -q "6765" "$OUT" 2>/dev/null && break
    kill -0 "$QPID" 2>/dev/null || break      # qemu exited on its own
    sleep 0.5
done
kill "$QPID" 2>/dev/null; wait "$QPID" 2>/dev/null
tr -d '\r' < "$OUT" > "$OUT.c" && mv "$OUT.c" "$OUT"

fail=0

# boot_until <logfile> <marker> <extra qemu args...>
#   Feeds the shell keys on stdin, backgrounds QEMU, and polls the log for the
#   marker. Breaks early if QEMU exits on its own.
boot_until() {
    local log=$1 marker=$2; shift 2
    # .=throwaway first byte (QEMU can hand the guest a byte before it starts
    # executing), h=help, 20f=fib(20), q=halt. Harmless on the graphical leg,
    # where nothing reads them.
    printf '.h20fq' | timeout "$CEILING" qemu-system-i386 \
        -drive file=zlOS.img,format=raw -serial stdio -display none -no-reboot \
        "$@" >"$log" 2>/dev/null &
    local pid=$!
    local i
    for i in $(seq $((CEILING * 2))); do
        grep -q "$marker" "$log" 2>/dev/null && break
        kill -0 "$pid" 2>/dev/null || break
        sleep 0.5
    done
    kill "$pid" 2>/dev/null; wait "$pid" 2>/dev/null
    tr -d '\r' < "$log" > "$log.c" && mv "$log.c" "$log"
}

want() {           # want <label> <logfile> <string> <message>
    grep -q "$3" "$2" || { echo "  FAIL  $1 - $4"; fail=1; }
}

echo "== raw: text console (-vga none), driven over serial =="
TLOG=$(mktemp)
boot_until "$TLOG" "6765" -vga none
want "text" "$TLOG" "our bootloader (raw_boot), no GRUB" "did not boot via our loader"
want "text" "$TLOG" "VGA text console"                   "did not fall back to text - is -vga none still honoured?"
want "text" "$TLOG" "ready\."                            "never reached the prompt"
want "text" "$TLOG" "6765"                               "fib(20) wrong or shell unresponsive"
grep -q "6765" "$TLOG" && echo "  ok    text - bootloader hands over, shell responds, fib(20) = 6765"
rm -f "$TLOG"

echo "== raw: framebuffer, straight into the compositor =="
GLOG=$(mktemp)
boot_until "$GLOG" "compositor:"
want "gfx" "$GLOG" "our bootloader (raw_boot), no GRUB" "did not boot via our loader"
want "gfx" "$GLOG" "framebuffer console"                "no framebuffer - VBE modesetting failed"
want "gfx" "$GLOG" "compositor: [1-9]"                  "the compositor never opened a window"
grep -q "compositor: [1-9]" "$GLOG" && \
    echo "  ok    gfx  - $(grep -o 'compositor: .*' "$GLOG" | head -1)"
rm -f "$GLOG"

[ "$fail" -eq 0 ] && echo "raw gate green" || echo "raw gate FAILED"
exit $fail
