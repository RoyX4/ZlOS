#!/usr/bin/env bash
# verify-raw.sh - boot the kernel with OUR bootloader (no GRUB) and drive the
# shell. Proves raw_boot.asm loads the kernel, enters protected mode and hands
# over correctly, independent of GRUB and multiboot entirely.
set -uo pipefail
cd "$(dirname "$0")"

command -v qemu-system-i386 >/dev/null || { echo "skip: no qemu"; exit 0; }
command -v nasm >/dev/null || { echo "skip: no nasm"; exit 0; }

./mkdisk.sh >/dev/null 2>&1 || { echo "FAIL: disk image did not build"; exit 1; }

OUT=$(mktemp); trap 'rm -f "$OUT"' EXIT
# .=throwaway, h=help, 20f=fib(20), q=halt
printf '.h20fq' | timeout 30 qemu-system-i386 \
    -drive file=zlOS.img,format=raw -serial stdio -display none -no-reboot \
    >"$OUT" 2>/dev/null
tr -d '\r' < "$OUT" > "$OUT.c" && mv "$OUT.c" "$OUT"

fail=0
grep -q "our bootloader (raw_boot), no GRUB" "$OUT" || { echo "  FAIL  did not boot via our loader"; fail=1; }
grep -q "ready\." "$OUT"  || { echo "  FAIL  never reached the prompt"; fail=1; }
grep -q "^6765$" "$OUT"   || { echo "  FAIL  fib(20) wrong or shell unresponsive"; fail=1; }

if [ "$fail" -eq 0 ]; then
    echo "ok    kernel boots via our own bootloader (no GRUB), shell responds"
fi
exit $fail
