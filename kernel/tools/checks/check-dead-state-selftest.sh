#!/usr/bin/env bash
# Prove check-dead-state.py can fail, and does not fail on the real tree.
#
# The guard exists because a one-off search found fifteen globals and three
# constants that nothing read, and that search was a command somebody typed
# once. A guard that replaces a typed command and then never fires is worse
# than the command was: the command at least got run that once.
#
# THREE DIRECTIONS, and the third is the one that would otherwise rot:
#
#   1  the real tree passes
#   2  a name that dies AFTER the baseline fails
#   3  a baseline entry that comes back to life fails
#
# Direction 3 matters because the baseline is a list of 28 names somebody has
# to keep true. Without this, wiring one of them up would leave a file claiming
# it is dead, and the next reader would believe the file.
#
# Runs against synthetic copies in a temp dir and never touches the checkout,
# so it is safe to run alongside a boot gate. Two seconds, no QEMU.
set -u
cd "$(dirname "$0")/../.." || exit 1
GUARD=tools/checks/check-dead-state.py
BASE=tools/checks/dead-state-baseline.txt
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
fail=0

# A SYNTHETIC KERNEL IS THE WHOLE TREE, HARD-LINKED, with the two files a case
# edits replaced by real copies.
#
# The first version copied only kernel.zl, apps/*.zl and the guard - and every
# case failed, because the guard counts mentions in the C, header, Python and
# shell sources too. Without them, every zl name that is read from C looked
# dead and the "real tree passes" case reported 200-odd orphans. A synthetic
# world that is missing what the guard reads does not test the guard, it tests
# the amputation.
#
# cp -al hard-links 11 MB in well under a second and uses no extra disk. The
# two files a case appends to are then UNLINKED and re-copied, so an append can
# never reach back into the checkout through the link.
build_world() {           # $1 = dir, $2 = extra zl line or "", $3 = extra baseline line or ""
    local w="$1"
    mkdir -p "$w"
    cp -al src tools apps "$w/" 2>/dev/null || cp -a src tools apps "$w/"
    rm -f "$w/src/kernel.zl" "$w/tools/checks/dead-state-baseline.txt"
    cp src/kernel.zl "$w/src/kernel.zl"
    cp "$BASE" "$w/tools/checks/dead-state-baseline.txt"
    [ -n "$2" ] && printf '%s\n' "$2" >> "$w/src/kernel.zl"
    [ -n "$3" ] && printf '%s\n' "$3" >> "$w/tools/checks/dead-state-baseline.txt"
    return 0
}

run() {                   # $1 = dir -> prints exit code, output in $tmp/out
    ( cd "$1" && python3 tools/checks/check-dead-state.py ) > "$tmp/out" 2>&1
    echo $?
}

check() {                 # $1 label, $2 dir, $3 wanted-exit, $4 wanted-substring
    local got
    got=$(run "$2")
    if [ "$got" != "$3" ]; then
        printf '  %-52s FAIL  (exit %s, wanted %s)\n' "$1" "$got" "$3"
        sed 's/^/      /' "$tmp/out" | tail -4
        fail=$((fail + 1))
        return
    fi
    if [ -n "$4" ] && ! grep -qF "$4" "$tmp/out"; then
        printf '  %-52s FAIL  (exit %s, but did not say %s)\n' "$1" "$got" "$4"
        sed 's/^/      /' "$tmp/out" | tail -4
        fail=$((fail + 1))
        return
    fi
    printf '  %-52s ok\n' "$1"
}

echo "check-dead-state selftest - three directions"
echo

# ---- 1. the real tree ------------------------------------------------------
build_world "$tmp/clean" "" ""
check "the real tree passes" "$tmp/clean" 0 "0 new"

# ---- 2. a name that dies after the baseline --------------------------------
# THE PLANTED NAME IS SPLIT ACROSS A QUOTE SO THIS FILE NEVER CONTAINS IT.
#
# It did, at first, and the case passed when it should have failed: this script
# lives at tools/checks/ and the guard counts mentions in every .sh under
# kernel/, so the selftest's own source was a second reader and the orphan was
# not orphaned. That is the guard's self-reference hazard reappearing one level
# out - the guard excludes ITSELF and its baseline by exact path, and adding a
# third exclusion would have hidden this instead of fixing it.
#
# `"ZZ_ORPHAN""_SELFTEST"` is one word to the shell and two identifiers to any
# tokeniser reading this file, which is precisely the property needed.
ORPHAN="ZZ_ORPHAN""_SELFTEST"
build_world "$tmp/orphan" "$ORPHAN = 1" ""
check "a newly dead name fails" "$tmp/orphan" 1 "$ORPHAN"

# ---- 3. a baseline entry that is alive again -------------------------------
# The name is declared AND read, so it is not dead - but the baseline claims it
# is. That is a file describing a tree that no longer exists, and it is fatal
# for the same reason a stale golden.txt would be.
# Split for the same reason, though this case would pass either way: the name
# is READ by the function below it, so it is alive whatever this file says.
# Consistency here is worth more than the one case that does not need it.
ALIVE="ZZ_ALIVE""_SELFTEST"
build_world "$tmp/stale" \
    "$ALIVE = 1
fn zz_alive_reader() { return $ALIVE }" \
    "$ALIVE"
check "a stale baseline entry fails" "$tmp/stale" 1 "alive again"

echo
if [ "$fail" -eq 0 ]; then
    echo "dead-state selftest: all good (0 failures)"
    exit 0
fi
echo "dead-state selftest: FAILED ($fail)"
exit 1
