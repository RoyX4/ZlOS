#!/usr/bin/env bash
# check-intdiv-selftest.sh - does the intdiv guard actually catch anything?
#
# A guard nobody has watched fail is a guard nobody should trust. This repo has
# a file about that (docs/GUARDS-THAT-DID-NOT-GUARD.md) listing five checks that
# reported green while checking nothing, including one whose entire mechanism
# was inert for its whole life.
#
# So: plant four defects of the shape the guard exists for, and one line that
# LOOKS like one and is not, and require the guard to catch exactly the four.
# The fifth case is the one that matters most - a guard that fires on everything
# gets switched off, which is the same outcome as not having it.
#
# Nothing here touches the real sources: everything happens in a temp copy.
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
KERNEL="$(cd "$HERE/../.." && pwd)"
GUARD="$HERE/check-intdiv.py"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

mkdir -p "$TMP/src" "$TMP/tools/checks" "$TMP/apps"
cp "$GUARD" "$TMP/tools/checks/"

fails=0
say() { if [ "$1" = 0 ]; then echo "  ok    $2"; else echo "  FAIL  $2"; fails=$((fails+1)); fi }

run_guard() { python3 "$TMP/tools/checks/check-intdiv.py" 2>&1; }

# ---------------------------------------------------------------- A: clean
cat > "$TMP/src/kernel.zl" <<'ZL'
fn clean(a, b) {
    row = idiv(a, b)
    half = (a - b) / 2
    return row + half
}
ZL
: > "$TMP/apps/apps_x.zl"
out="$(run_guard)"
case "$out" in *"OK - no float"*) say 0 "a clean tree passes";; *) say 1 "a clean tree passes -- got: $out";; esac

# `half` is deliberately NOT index-ish, and must not be flagged. This is the
# case that keeps the guard usable: over two hundred such divisions exist.
case "$out" in *half*) say 1 "a non-index name is not flagged";; *) say 0 "a non-index name is not flagged";; esac

# ------------------------------------------------------- B: the four defects
cat > "$TMP/src/kernel.zl" <<'ZL'
fn planted(ey, y0, rowh, cx, cw, i, cols, t) {
    row = (ey - y0) / rowh
    files_mcol = cx / cw
    kl_fit = (ey - y0) / rowh
    tsec = t / 100
    half = (cx - cw) / 2
    return row + files_mcol + kl_fit + tsec + half
}
ZL
out="$(run_guard)"
for name in row files_mcol kl_fit tsec; do
    case "$out" in *"$name"*) say 0 "planted defect caught: $name";; *) say 1 "planted defect MISSED: $name";; esac
done
case "$out" in *half*) say 1 "the legitimate float is still not flagged";; *) say 0 "the legitimate float is still not flagged";; esac
case "$out" in *FAIL*) say 0 "the guard reports failure when defects exist";; *) say 1 "the guard reported success with four defects present";; esac

# -------------------------------------------------- C: the escape hatch works
cat > "$TMP/src/kernel.zl" <<'ZL'
fn hatched(a, b) {
    row = a / b   # float ok - this is a ratio for a gradient, never an index
    return row
}
ZL
out="$(run_guard)"
case "$out" in *"OK - no float"*) say 0 "'# float ok' silences a line";; *) say 1 "'# float ok' did not silence -- got: $out";; esac

# ------------------------------------------- D: idiv on the line is not a hit
cat > "$TMP/src/kernel.zl" <<'ZL'
fn nested(a, b, c) {
    row = idiv(a - b / 2, c)
    return row
}
ZL
out="$(run_guard)"
case "$out" in *"OK - no float"*) say 0 "a divide inside idiv's arguments is not a finding";; *) say 1 "idiv with a nested divide was flagged -- got: $out";; esac

echo
if [ "$fails" -eq 0 ]; then
    echo "check-intdiv-selftest: PASS - the guard catches its defects, ignores"
    echo "                        legitimate floats, and honours its escape hatch"
    exit 0
fi
echo "check-intdiv-selftest: FAILED - $fails case(s)"
exit 1
