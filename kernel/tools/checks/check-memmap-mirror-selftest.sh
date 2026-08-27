#!/usr/bin/env bash
# Prove check-memmap-mirror.py can fail, and does not fail on the real tree.
#
# It guards a hazard that was WRITTEN DOWN and left unenforced for weeks, so the
# one thing it must never become is another written-down intention. Runs the real
# script against synthetic copies; never touches the checkout, so it is safe
# alongside a boot gate.
set -u
cd "$(dirname "$0")/../.." || exit 1
GUARD=tools/checks/check-memmap-mirror.py
ZL=src/kernel.zl
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
fail=0

run() {  # $1 = kernel.zl content file -> prints exit code
    mkdir -p "$tmp/w/src/arch/x86" "$tmp/w/tools/checks"
    cp "$1" "$tmp/w/src/kernel.zl"
    cp src/arch/x86/memmap.h "$tmp/w/src/arch/x86/memmap.h"
    cp "$GUARD" "$tmp/w/tools/checks/"
    python3 "$tmp/w/tools/checks/check-memmap-mirror.py" >"$tmp/out" 2>&1
    echo $?
}

check() { # $1 label, $2 file, $3 wanted-exit
    local got; got=$(run "$2")
    if [ "$got" = "$3" ]; then printf "  ok   %s (exit %s)\n" "$1" "$got"
    else printf "  FAIL %s: exit %s, wanted %s\n" "$1" "$got" "$3"; fail=$((fail+1)); fi
}

cp "$ZL" "$tmp/real.zl"
check "the real tree passes" "$tmp/real.zl" 0

# a drifted mirror - the exact hazard, RULER_DMA no longer equal to HI_IMG
sed 's/^RULER_DMA     = 0x03000000/RULER_DMA     = 0x03000004/' "$ZL" > "$tmp/drift.zl"
check "a DRIFTED mirror is caught" "$tmp/drift.zl" 1

# a mirror citing a symbol that does not exist
sed 's/# memmap.h HI_IMG /# memmap.h HI_NOPE /' "$ZL" > "$tmp/ghost.zl"
check "a citation to a MISSING symbol is caught" "$tmp/ghost.zl" 1

echo
if [ "$fail" -ne 0 ]; then echo "check-memmap-mirror-selftest: FAILED ($fail)"; exit 1; fi
echo "check-memmap-mirror-selftest: PASS - catches drift and dead citations"
