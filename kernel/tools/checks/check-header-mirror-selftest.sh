#!/bin/sh
# Does check-header-mirror.py actually FAIL when a mirror is wrong?
#
# It passed the first time it was ever run. That is not evidence - a checker
# whose regex silently matches nothing also passes, and this repo has shipped
# exactly that: the EFI -Werror guard was inert and hid 34 truncation sites.
# So each defect class is planted, one at a time, against a real copy of the
# tree, and the check must reject it.
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

mkdir -p "$TMP/tools/checks" "$TMP/src" "$TMP/apps"
cp "$HERE/check-header-mirror.py" "$TMP/tools/checks/"
cp -r "$ROOT/src/." "$TMP/src/"
# apps/ too, since the checker reads every .zl and not only kernel.zl. Without
# this the selftest would exercise one of the eleven files it now guards.
cp "$ROOT"/apps/*.zl "$TMP/apps/" 2>/dev/null || true
ZL="$TMP/src/kernel.zl"
CHECK="$TMP/tools/checks/check-header-mirror.py"

fails=0
expect_fail() {
    if python3 "$CHECK" >/dev/null 2>&1; then
        echo "  $1: NOT CAUGHT"
        fails=$((fails + 1))
    else
        echo "  $1: caught"
    fi
}
restore() { cp "$ROOT/src/kernel.zl" "$ZL"; cp "$ROOT"/apps/*.zl "$TMP/apps/" 2>/dev/null || true; }

# baseline - the unmodified tree must pass, or every result below is noise
if python3 "$CHECK" >/dev/null 2>&1; then
    echo "  baseline: passes"
else
    echo "  baseline: FAILS ON A CLEAN TREE - every case below is meaningless"
    python3 "$CHECK" || true
    exit 1
fi

# A: a value that disagrees with the header. This is the bug that shipped:
#    desk_key held 0x13B where keycodes.h says 0x120.
sed -i 's/^KEY_ESC_K  = 0x101/KEY_ESC_K  = 0x13B/' "$ZL"
expect_fail "A wrong value"
restore

# B: cites a symbol the header does not have (a rename on the C side)
sed -i 's/# keycodes.h KEY_ESC /# keycodes.h KEY_ESCAPE /' "$ZL"
expect_fail "B symbol not in header"
restore

# C: cites a header that does not exist
sed -i 's/# keycodes.h KEY_ESC /# keycodez.h KEY_ESC /' "$ZL"
expect_fail "C header not found"
restore

# D: every citation removed. A checker that finds nothing must not pass -
#    silence is how an inert guard looks from the outside.
# .h AND .c both - the checker reads both, and a case D that only stripped .h
# stopped catching anything the day fs.c's FS_WHY_* mirrors were added. The
# selftest found that itself, which is the entire reason it plants this case.
# EVERY .zl, not just kernel.zl - the checker reads them all now, so stripping
# one file's citations leaves the others' and the run still finds work to do.
# This case has now caught a coverage change three times: when .c citations were
# added, and again when apps/*.zl came into scope. That is what it is for.
sed -i -E 's/#( *)([A-Za-z0-9_]*\.[hc]) ([A-Z_][A-Z0-9_]*)/# \3/' "$ZL" "$TMP"/apps/*.zl
expect_fail "D no citations at all"
restore

# E: a decimal mirror. design.h's tokens are decimal; a hex-only reader would
#    skip every one of them and still report OK.
sed -i 's/^SPARK_MAX  = 64/SPARK_MAX  = 65/' "$ZL"
expect_fail "E wrong decimal value"
restore

if [ "$fails" -ne 0 ]; then
    echo "check-header-mirror-selftest: FAIL - $fails case(s) not caught"
    exit 1
fi
echo "check-header-mirror-selftest: OK - all 5 planted defects rejected"
