#!/usr/bin/env bash
# verify-net.sh - prove virtio_net fetches http://example.com/ under QEMU user-net.
#
# Wait for the answer, never for a fixed wall-clock. HTTPS stays refused.
set -uo pipefail
KERNEL_ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$KERNEL_ROOT"

OUT=$(mktemp); trap 'rm -f "$OUT"' EXIT

./build.sh >/dev/null 2>&1 || { echo "FAIL: kernel did not build"; exit 1; }

# N brings the card and IP stack up; E fetches example.com by name.
# Leading '.' is the same throwaway verify.sh uses (first serial byte can drop).
KEYS='.NEq'
CEILING=240

printf '%s' "$KEYS" | timeout "$CEILING" qemu-system-i386 \
    -kernel kernel.elf -m 256 \
    -netdev user,id=n0 -device virtio-net-pci,netdev=n0 \
    -serial stdio -display none -no-reboot \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04 >"$OUT" 2>/dev/null &
QPID=$!
for _ in $(seq $((CEILING * 2))); do
    grep -q "fetched and rendered" "$OUT" 2>/dev/null && break
    grep -q "halting" "$OUT" 2>/dev/null && break
    kill -0 "$QPID" 2>/dev/null || break
    sleep 0.5
done
kill "$QPID" 2>/dev/null; wait "$QPID" 2>/dev/null

if grep -q "fetched and rendered" "$OUT" 2>/dev/null; then
    echo "ok    virtio_net fetched http://example.com/ and the browser laid it out"
    exit 0
fi
echo "FAIL: no fetch"
grep -E "network|website|failed|virtio|HTTP|halt" "$OUT" | head -20
exit 1
