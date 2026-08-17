#!/usr/bin/env bash
# kernel/verify.sh - the second gate (design_kernel.md §10).
#
# Build the image, boot it headless, drive the shell over serial, capture
# the transcript, diff against golden.txt, always time out.
set -uo pipefail
cd "$(dirname "$0")"

GOLDEN=golden.txt
OUT=$(mktemp); trap 'rm -f "$OUT"' EXIT

./build.sh >/dev/null 2>&1 || { echo "FAIL: kernel did not build"; exit 1; }

# The leading '.' is a deliberate throwaway: QEMU can hand the guest the
# very first serial byte before it starts executing, so that byte is lost
# no matter what the kernel does. A human typing never hits this.
# Then: h=help, 20f=fib(20), 10s=sum_squares(10), m=poke/peek proof, q=halt.
KEYS='.h20f10smq'

# Wait for the answer, not a fixed wall clock. Under TCG the boot time depends
# on host load, and a fixed timeout turns a busy machine into a failed gate on
# an unchanged kernel. 'halting' is the last thing the kernel prints for 'q'.
CEILING=180
printf '%s' "$KEYS" | timeout "$CEILING" qemu-system-i386 \
    -kernel kernel.elf -serial stdio -display none -no-reboot \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04 >"$OUT" 2>/dev/null &
QPID=$!
for _ in $(seq $((CEILING * 2))); do
    grep -q "halting" "$OUT" 2>/dev/null && break
    kill -0 "$QPID" 2>/dev/null || break
    sleep 0.5
done
kill "$QPID" 2>/dev/null; wait "$QPID" 2>/dev/null
grep -q "halting" "$OUT" 2>/dev/null || { echo "FAIL: kernel never halted - it hung"; exit 1; }
[ -s "$OUT" ]     || { echo "FAIL: no serial output - it did not boot"; exit 1; }

tr -d '\r' < "$OUT" > "$OUT.c" && mv "$OUT.c" "$OUT"

if [ ! -f "$GOLDEN" ]; then
    cp "$OUT" "$GOLDEN"; echo "wrote $GOLDEN (first run - review, then commit)"; exit 0
fi

if diff -q "$GOLDEN" "$OUT" >/dev/null; then
    echo "ok    kernel boots, shell responds, transcript matches golden.txt"
    exit 0
fi
echo "FAIL: serial transcript changed"; diff "$GOLDEN" "$OUT" | head -20; exit 1
