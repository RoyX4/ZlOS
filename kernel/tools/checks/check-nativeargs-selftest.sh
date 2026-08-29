#!/usr/bin/env bash
# check-nativeargs-selftest.sh - can the arity guard actually fail?
#
# This tree has a file about guards that reported green while checking nothing
# (docs/GUARDS-THAT-DID-NOT-GUARD.md), and a guard was deleted from it this week
# for enforcing a bug class that did not exist. So: plant defects, plant things
# that LOOK like defects and are not, and require exactly the right set.
#
# Case B's first plant is the REAL br_click bug, verbatim - the one that made a
# browser click depend on stack garbage. A self-test whose plants are invented
# proves the guard catches invented things.
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
GUARD="$HERE/check-nativeargs.py"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/kernel/src" "$TMP/kernel/apps" "$TMP/kernel/tools/checks" "$TMP/freestanding"
cp "$GUARD" "$TMP/kernel/tools/checks/"

# A miniature runtime with four natives of known arity.
cat > "$TMP/freestanding/runtime_kernel.c" <<'C'
Value zl_call(const char *name, int n, ...)
{
    Value a[8] = { 0 };
    if (streq(name, "br_click"))  return zl_num((double)browser_click((int)a[0].num,(int)a[1].num,(int)a[2].num));
    if (streq(name, "two_arg"))   return zl_num((double)f2((int)a[0].num,(int)a[1].num));
    if (streq(name, "one_arg"))   return zl_num((double)f1((int)a[0].num));
    if (streq(name, "no_arg"))    return zl_num((double)f0());
    return zl_nil();
}
C
: > "$TMP/kernel/apps/apps_x.zl"

fails=0
say() { if [ "$1" = 0 ]; then echo "  ok    $2"; else echo "  FAIL  $2"; fails=$((fails+1)); fi }
run_guard() { python3 "$TMP/kernel/tools/checks/check-nativeargs.py" 2>&1; }

# ------------------------------------------------------------------ A: clean
cat > "$TMP/kernel/src/kernel.zl" <<'ZL'
fn clean(x, y, b) {
    br_click(x, y, b)
    two_arg(x, y)
    one_arg("a string is one argument")
    no_arg()
    two_arg(idiv(x - 1, y), b)
    return 0
}
ZL
out="$(run_guard)"
case "$out" in *"OK - every call"*) say 0 "a clean tree passes";; *) say 1 "a clean tree passes -- got: $out";; esac
# The string case and the nested-call case are the two that broke the counter on
# its first run, so they are asserted individually, not just via the OK above.
case "$out" in *one_arg*) say 1 "a lone string counts as an argument";; *) say 0 "a lone string counts as an argument";; esac
case "$out" in *two_arg*) say 1 "a nested call counts as ONE argument";; *) say 0 "a nested call counts as ONE argument";; esac

# ------------------------------------------------------- B: the real defects
cat > "$TMP/kernel/src/kernel.zl" <<'ZL'
fn planted(x, y, b) {
    br_click(x, y)
    two_arg(x)
    one_arg()
    br_click(x)
    no_arg()
    return 0
}
ZL
out="$(run_guard)"
case "$out" in *"br_click takes 3, given 2"*) say 0 "the real br_click defect is caught";; *) say 1 "the real br_click defect MISSED";; esac
case "$out" in *"two_arg takes 2, given 1"*) say 0 "planted caught: two_arg given 1";; *) say 1 "planted MISSED: two_arg given 1";; esac
case "$out" in *"one_arg takes 1, given 0"*) say 0 "planted caught: one_arg given 0";; *) say 1 "planted MISSED: one_arg given 0";; esac
case "$out" in *"br_click takes 3, given 1"*) say 0 "planted caught: br_click given 1";; *) say 1 "planted MISSED: br_click given 1";; esac
case "$out" in *no_arg*) say 1 "a zero-arity native is not flagged";; *) say 0 "a zero-arity native is not flagged";; esac
case "$out" in *FAIL*) say 0 "the guard reports failure when defects exist";; *) say 1 "the guard reported success with defects present";; esac

# ------------------------------------------ C: passing MORE is not a finding
cat > "$TMP/kernel/src/kernel.zl" <<'ZL'
fn extra(x, y, b, c) {
    br_click(x, y, b, c)
    return 0
}
ZL
out="$(run_guard)"
case "$out" in *"OK - every call"*) say 0 "passing MORE than the native reads is not flagged";; *) say 1 "an over-supplied call was flagged -- got: $out";; esac

# --------------------------------------- D: a definition is not a call site
cat > "$TMP/kernel/src/kernel.zl" <<'ZL'
fn two_arg() {
    return 0
}
ZL
out="$(run_guard)"
case "$out" in *"OK - every call"*) say 0 "a fn DEFINITION is not mistaken for a call";; *) say 1 "a definition was flagged -- got: $out";; esac

# ----------------------------------------- E: a commented-out call is not one
cat > "$TMP/kernel/src/kernel.zl" <<'ZL'
fn commented(x) {
    # br_click(x) - this is prose about a call, not a call
    return 0
}
ZL
out="$(run_guard)"
case "$out" in *"OK - every call"*) say 0 "a call inside a comment is not flagged";; *) say 1 "a commented call was flagged -- got: $out";; esac

echo
if [ "$fails" -eq 0 ]; then
    echo "check-nativeargs-selftest: PASS - catches the real defect and three"
    echo "                            planted ones, and none of the five decoys"
    exit 0
fi
echo "check-nativeargs-selftest: FAILED - $fails case(s)"
exit 1
