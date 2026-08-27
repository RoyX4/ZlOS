#!/usr/bin/env bash
# Prove the shared classifier catches unexpected signals without treating the
# SIGTERM sent by healthy boot gates as an emulator crash.
set -u
cd "$(dirname "$0")/../.." || exit 1
. tools/checks/qemu-crash.sh

fail=0
check() {
    local status=$1 want=$2 out rc
    out=$(qemu_crashed "$status"); rc=$?
    if [ "$rc" -ne "$want" ]; then
        echo "  FAIL  status $status returned $rc, expected $want"
        fail=$((fail + 1))
    elif [ "$want" -eq 0 ] && [ -z "$out" ]; then
        echo "  FAIL  status $status was detected silently"
        fail=$((fail + 1))
    elif [ "$want" -ne 0 ] && [ -n "$out" ]; then
        echo "  FAIL  status $status was reported as a crash: $out"
        fail=$((fail + 1))
    else
        echo "  ok    status $status"
    fi
}

for status in 128 132 134 135 136 137 139 142; do check "$status" 0; done
for status in 0 1 33 124 143; do check "$status" 1; done

[ "$fail" -eq 0 ] || { echo "qemu-crash-selftest: FAILED ($fail)"; exit 1; }
echo "qemu-crash-selftest: PASS - unexpected signals fail; normal gate exits stay quiet"
