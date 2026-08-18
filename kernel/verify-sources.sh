#!/usr/bin/env bash
# verify-sources.sh - does adding one source file really reach ALL FOUR builds?
#
# There used to be four copies of the source list, and adding a .c to the build
# you were testing with silently missed the other three. It broke the UEFI and
# the raw builds TWICE on 2026-08-18, and verify.sh cannot see it because
# verify.sh only ever builds the first one.
#
# ./SOURCES is now the single list. This proves that rather than asserting it,
# the way the task asked for: drop a throwaway .c in, build all four, and look
# for it INSIDE each of the four outputs. Not "the build succeeded" - a build
# that ignored the file succeeds too. The marker string has to actually be in
# the binary.
#
# Cleanup runs from an EXIT trap, so it fires on success, failure, and Ctrl-C
# alike - the same reason modeset-run.sh does it that way. Leaving a probe file
# in SOURCES would poison every later build on this machine.
set -uo pipefail
cd "$(dirname "$0")"

PROBE=_srcprobe.c
MARKER=ZL_SOURCES_PROBE_9c3f1a
SOURCES_BAK=$(mktemp)
cp SOURCES "$SOURCES_BAK"

cleanup() {
    cp "$SOURCES_BAK" SOURCES
    rm -f "$SOURCES_BAK" "$PROBE" _srcprobe.o _srcprobe64.o _efi__srcprobe.o
}
trap cleanup EXIT

cat > "$PROBE" <<EOF
/* Written and deleted by verify-sources.sh. If you are reading this in a
 * working tree, that script died between writing it and its EXIT trap - it is
 * safe to delete, and \`git status\` should not be showing it at all. */
__attribute__((used))
const char zl_sources_probe[] = "$MARKER";
EOF

echo "$PROBE" >> SOURCES

fail=0
build_and_look() {          # build_and_look <label> <script> <output file>
    local label=$1 script=$2 out=$3
    if ! "./$script" >/dev/null 2>&1; then
        echo "  FAIL  $label - $script did not build with the probe in SOURCES"
        fail=1; return
    fi
    if [ ! -f "$out" ]; then
        echo "  FAIL  $label - $script produced no $out"; fail=1; return
    fi
    # grep the BINARY. A build that quietly ignored SOURCES still exits 0.
    if grep -qa "$MARKER" "$out"; then
        echo "  ok    $label - the new source reached $out"
    else
        echo "  FAIL  $label - $out does not contain the probe: $script is not reading SOURCES"
        fail=1
    fi
}

echo "== added $PROBE to SOURCES, building all four =="
build_and_look "32-bit BIOS " build.sh    kernel.elf
build_and_look "64-bit      " build64.sh  kernel64.elf
build_and_look "UEFI app    " buildefi.sh BOOTX64.EFI
build_and_look "raw disk    " mkdisk.sh   kernel_raw.bin

# ...and the other half of the claim: with the probe gone, it is gone from all
# four. Otherwise a stale object left in the tree would keep the marker alive
# and this whole gate would pass on a build system that reads nothing.
cleanup
trap - EXIT
echo "== removed it again =="
for pair in "build.sh kernel.elf" "build64.sh kernel64.elf" \
            "buildefi.sh BOOTX64.EFI" "mkdisk.sh kernel_raw.bin"; do
    set -- $pair
    "./$1" >/dev/null 2>&1
    if grep -qa "$MARKER" "$2"; then
        echo "  FAIL  $2 still contains the probe after removal - stale object?"
        fail=1
    fi
done
[ "$fail" -eq 0 ] && echo "  ok    all four are clean again"

[ "$fail" -eq 0 ] && echo "sources gate green: one list, four builds" \
                  || echo "sources gate FAILED"
exit $fail
