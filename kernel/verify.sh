#!/usr/bin/env bash
# kernel/verify.sh - the second gate, deliberately separate.
#
# design_kernel.md §10 asks for exactly this: build the image, boot it
# headless, capture serial, diff against a golden transcript, always time
# out. run_tests.sh is the language gate; this is the kernel gate, and a
# kernel that stops booting must fail loudly rather than quietly.
set -uo pipefail
cd "$(dirname "$0")"

GOLDEN=golden.txt
OUT=$(mktemp)
trap 'rm -f "$OUT"' EXIT

./build.sh >/dev/null 2>&1 || { echo "FAIL: kernel did not build"; exit 1; }

# A kernel that hangs must not hang the test suite - always time out.
timeout 30 qemu-system-i386 \
    -kernel kernel.elf \
    -serial "file:$OUT" \
    -display none \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
    -no-reboot >/dev/null 2>&1

# QEMU's isa-debug-exit reports (value<<1)|1, so kernel_done's 0 becomes 1.
# Any other code means it never reached the end.
rc=$?
if [ "$rc" -eq 124 ]; then
    echo "FAIL: kernel timed out - it hung before halting"
    exit 1
fi

if [ ! -s "$OUT" ]; then
    echo "FAIL: no serial output at all - it did not boot"
    exit 1
fi

# strip CR: the UART emits \r\n, the golden file is plain \n
tr -d '\r' < "$OUT" > "$OUT.clean" && mv "$OUT.clean" "$OUT"

if [ ! -f "$GOLDEN" ]; then
    cp "$OUT" "$GOLDEN"
    echo "wrote $GOLDEN (first run - review it, then commit it)"
    exit 0
fi

if diff -q "$GOLDEN" "$OUT" >/dev/null; then
    echo "ok    kernel boots and its serial transcript matches $GOLDEN"
    exit 0
fi

echo "FAIL: serial transcript changed"
diff "$GOLDEN" "$OUT" | head -20
exit 1
