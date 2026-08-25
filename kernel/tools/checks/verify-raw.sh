#!/usr/bin/env bash
# verify-raw.sh - boot the kernel with OUR bootloader (no GRUB) and drive the
# shell. Proves raw_boot.asm loads the kernel, enters protected mode and hands
# over correctly, independent of GRUB and multiboot entirely.
set -uo pipefail
cd "$(dirname "$0")/../.." || exit

command -v qemu-system-i386 >/dev/null || { echo "skip: no qemu"; exit 0; }
command -v nasm >/dev/null || { echo "skip: no nasm"; exit 0; }

if [ "${ZLOS_SKIP_BUILD:-0}" = 1 ]; then
    [ -s zlOS.img ] || { echo "FAIL: ZLOS_SKIP_BUILD=1 but zlOS.img is missing"; exit 1; }
else
    ./tools/images/mkdisk.sh >/dev/null 2>&1 || { echo "FAIL: disk image did not build"; exit 1; }
fi

OUT=$(mktemp); trap 'rm -f "$OUT"' EXIT

# Wait for the ANSWER, not for a fixed wall-clock time.
#
# This used to be `timeout 30`. Under TCG emulation the whole boot plus the help
# text is a couple of KiB over a simulated 115200 line, and how long that takes
# depends entirely on what else the host is doing - measured anywhere from 12 s
# to well past 30 s on the same unchanged kernel. So the gate reported failures
# that were host load, not kernel behaviour, which is worse than no gate at all:
# it costs a bisect every time. Poll for the marker instead and give it a
# generous ceiling; a healthy kernel now finishes as fast as the host allows,
# and only a genuinely stuck one waits out the ceiling.
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
# -m 1G is HI_TOP (memmap.h). This gate passed no -m for its whole life and got
# qemu's 128 MiB default, so the high-RAM map above 128 MiB was unbacked here.
printf '.\rhelp\rfib 20\rquit\r' | timeout "$CEILING" qemu-system-i386 \
    -drive file=zlOS.img,format=raw -m 1G -serial stdio -display none -no-reboot \
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
MANIFEST_SHA=$(sha256sum app-manifest.json | awk '{print $1}')
grep -q "our bootloader (raw_boot), no GRUB" "$OUT" || { echo "  FAIL  did not boot via our loader"; fail=1; }
grep -q "ready\." "$OUT"  || { echo "  FAIL  never reached the prompt"; fail=1; }
grep -q "6765" "$OUT"   || { echo "  FAIL  fib(20) wrong or shell unresponsive"; fail=1; }
grep -q "app-manifest: schema=1 entries=62 sha256=$MANIFEST_SHA" "$OUT" || {
    echo "  FAIL  running raw image did not report the current 62-app manifest"; fail=1;
}

if [ "$fail" -eq 0 ]; then
    python3 ./tools/generators/write-app-manifest-boot-receipt.py \
        --route raw-bios --artifact zlOS.img --log "$OUT" \
        --harness verify-raw.sh \
        --boot-origin "our bootloader (raw_boot), no GRUB" \
        --output docs/receipts/app-manifest-raw-bios-qemu-2026-08-22.json \
        || fail=1
    echo "ok    kernel boots via our own bootloader (no GRUB), shell responds"
fi
exit $fail
