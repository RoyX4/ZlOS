#!/usr/bin/env bash
# kernel/verify.sh - the second gate (design_kernel.md §10).
#
# Build the image, boot it headless, drive the shell over serial, capture
# the transcript, diff against golden.txt, always time out.
set -uo pipefail
cd "$(dirname "$0")" || exit

GOLDEN=tests/fixtures/golden.txt
OUT=$(mktemp)
NORMALIZED=$(mktemp)
trap 'rm -f "$OUT" "$NORMALIZED"' EXIT

if ! BUILD_OUTPUT=$(./build.sh 2>&1); then
    echo "FAIL: kernel did not build"
    printf '%s\n' "$BUILD_OUTPUT" | tail -80
    exit 1
fi

# The leading '.' is a deliberate throwaway: QEMU can hand the guest the
# very first serial byte before it starts executing, so that byte is lost
# no matter what the kernel does. A human typing never hits this.
# Then: h=help, 20f=fib(20), 10s=sum_squares(10), m=poke/peek proof, q=halt.
KEYS='.h20f10smq'

# Wait for the answer, not a fixed wall clock. Under TCG the boot time depends
# on host load, and a fixed timeout turns a busy machine into a failed gate on
# an unchanged kernel. 'halting' is the last thing the kernel prints for 'q'.
CEILING=180
# -m 1G is HI_TOP (memmap.h), not a round number. This gate passed no -m for its
# whole life, so it booted qemu-system-i386's default 128 MiB and every address
# in the high-RAM map above 128 MiB was UNBACKED - the map was half-imaginary on
# the gate that is supposed to prove it. check-ram.sh now fails if this number
# and HI_TOP ever disagree.
printf '%s' "$KEYS" | timeout "$CEILING" qemu-system-i386 \
    -kernel kernel.elf -m 1G -serial stdio -display none -no-reboot \
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

MANIFEST_SHA=$(sha256sum metadata/app-manifest.json | awk '{print $1}')
# Derived, like the three identity fields below it. This was `entries=62` in two
# places in this file while the kernel printed 64 - the gate and the thing it
# gates disagreeing about a number both of them could have just looked up.
MANIFEST_N=$(python3 -c 'import json; print(len(json.load(open("metadata/app-manifest.json"))["entries"]))')
BUILD_ID=$(python3 -c 'import json; print(json.load(open("metadata/build-identity.json"))["identity_sha256"])')
BUILD_HEAD=$(python3 -c 'import json; print(json.load(open("metadata/build-identity.json"))["git"]["head"])')
BUILD_DIRTY=$(python3 -c 'import json; print(1 if json.load(open("metadata/build-identity.json"))["git"]["dirty"] else 0)')
for marker in \
    "app-manifest: schema=1 entries=$MANIFEST_N sha256=$MANIFEST_SHA" \
    "build-identity: schema=1 id=$BUILD_ID" \
    "build-source: head=$BUILD_HEAD dirty=$BUILD_DIRTY"; do
    [ "$(grep -Fc "$marker" "$OUT")" -eq 1 ] || {
        echo "FAIL: serial transcript has missing or duplicate current receipt: $marker"
        exit 1
    }
done

sed -E \
    -e "s/(app-manifest: schema=1 entries=$MANIFEST_N sha256=)[0-9a-f]{64}/\1<CURRENT>/" \
    -e 's/(build-identity: schema=1 id=)[0-9a-f]{64}/\1<CURRENT>/' \
    -e 's/(build-source: head=)[0-9a-f]{40} dirty=[01]/\1<CURRENT>/' \
    "$OUT" > "$NORMALIZED"

if diff -q "$GOLDEN" "$NORMALIZED" >/dev/null; then
    echo "ok    kernel boots, shell responds, transcript matches golden.txt"
    exit 0
fi
echo "FAIL: serial transcript changed"; diff "$GOLDEN" "$NORMALIZED" | head -20; exit 1
