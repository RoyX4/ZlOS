#!/usr/bin/env bash
# killtest.sh - can a zl program be stopped?
#
# EXEC-PROMPT.md Item 2 says three properties are not negotiable, and that the
# second and third matter MORE than the first: a runaway program must be
# killable, and a crashing program must not take the kernel with it. It also
# says to decide the mechanism before writing the loop rather than after.
#
# This gate is the adversarial half of that. Every script below is written to
# WIN - to run forever, to exhaust the stack, to divide by zero, to index off
# the end of a list - and the assertion is that each one is stopped, that the
# interpreter says which one it was, and that the process is still there
# afterwards to be asked.
#
# WHY THIS RUNS ON THE HOST, WITH NO KERNEL IN SIGHT
#
# The mechanism is arithmetic - a counter in eval() and exec() and a longjmp
# out - and none of it is kernel-specific. Proving it here costs a second per
# run instead of a QEMU boot, so the adversarial scripts can be plentiful and
# cheap, and every one of them exercises THE SAME LINES the kernel will run:
# zi_limit / zi_step / runtime_error are compiled once, in interp.c, with the
# limits off by default and armed by whoever is running the program.
#
# What this does NOT prove, and the kernel gate must: that the desktop stays
# responsive while it happens. That is Item 2's own gate, and it needs a
# machine.
#
#   ./killtest.sh
set -uo pipefail
cd "$(dirname "$0")"
ROOT=$(cd ../.. && pwd)
INTERP="$ROOT/interp"
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT

[ -x "$INTERP" ] || { echo "FAIL: no $INTERP - run ../../build.sh first"; exit 1; }

checks=0; fails=0

# run <name> <expected-exit> <expected-message-fragment> <steps> <depth> <<'ZL'
run_case() {
    local name="$1" want_rc="$2" want_msg="$3" steps="$4" depth="$5"
    local src="$TMP/case.zl" out="$TMP/out" err="$TMP/err"
    cat > "$src"
    checks=$((checks+1))

    # A HARD WALL-CLOCK LIMIT, and it is the one place a timeout is right: it
    # is not measuring the program, it is catching the case where the budget
    # DID NOT WORK. Without it a failure of the thing under test hangs the gate
    # forever, which is the one failure mode a gate must never have.
    timeout 20 "$INTERP" --steps "$steps" --depth "$depth" "$src" >"$out" 2>"$err"
    local rc=$?

    if [ "$rc" = 124 ]; then
        echo "  FAIL  $name - THE BUDGET DID NOT STOP IT (killed by timeout)"
        fails=$((fails+1)); return
    fi
    if [ "$rc" != "$want_rc" ]; then
        echo "  FAIL  $name - exit $rc, wanted $want_rc"
        sed 's/^/          /' "$err" | head -3
        fails=$((fails+1)); return
    fi
    if ! grep -qF "$want_msg" "$err"; then
        echo "  FAIL  $name - never said '$want_msg'"
        sed 's/^/          /' "$err" | head -3
        fails=$((fails+1)); return
    fi
    echo "  ok    $name"
}

echo "killtest - every one of these is trying to wedge the machine"
echo

# ---- 1. the plain infinite loop -------------------------------------------
run_case "while true {} is stopped" 2 "step budget exhausted" 100000 0 <<'ZL'
x = 0
while 1 == 1 {
    x = x + 1
}
ZL

# ---- 2. an infinite loop that prints nothing and touches nothing ----------
# The empty body is the interesting one: there is no allocation, no builtin
# call and no output, so anything watching for progress by watching for SIDE
# EFFECTS sees a program that looks identical to a halted machine.
run_case "an empty infinite loop is stopped" 2 "step budget exhausted" 50000 0 <<'ZL'
while 1 == 1 { }
ZL

# ---- 3. infinite recursion --------------------------------------------------
# The budget alone would eventually stop this, but only after the C stack has
# already been walked off the bottom of - which in the kernel is 256 KiB and
# somebody else's memory. The depth cap has to be what catches it.
run_case "infinite recursion hits the DEPTH cap, not the budget" 2 "too deeply nested" 100000000 200 <<'ZL'
fn down(n) {
    return down(n + 1)
}
down(0)
ZL

# ---- 4. mutual recursion, which a naive self-call check would miss ---------
run_case "mutual recursion is stopped too" 2 "too deeply nested" 100000000 200 <<'ZL'
fn ping(n) { return pong(n + 1) }
fn pong(n) { return ping(n + 1) }
ping(0)
ZL

# ---- 5. a loop nested inside a loop ---------------------------------------
run_case "nested infinite loops are stopped" 2 "step budget exhausted" 60000 0 <<'ZL'
while 1 == 1 {
    while 1 == 1 { }
}
ZL

# ---- 6. an illegal operation UNWINDS rather than killing the process -------
# Exit 1, not 2: this is the program being wrong, not the program being
# stopped, and the two must not be reported as the same thing.
run_case "looping over a non-list unwinds as an ERROR, not a kill" 1 "can only loop over a list" 1000000 500 <<'ZL'
for x in 7 {
    print(x)
}
ZL

run_case "a name that does not exist unwinds" 1 "doesn't exist yet" 1000000 500 <<'ZL'
print(never_defined_anywhere)
ZL

run_case "indexing off the end of a list unwinds" 1 "range" 1000000 500 <<'ZL'
xs = [1, 2, 3]
print(xs[99])
ZL

# ---- 7. an error raised from INSIDE deep recursion -------------------------
# The unwind has to cross a few hundred live frames. If the longjmp target or
# the depth bookkeeping were wrong, this is where it shows.
run_case "an error deep inside recursion still unwinds" 1 "doesn't exist yet" 100000000 5000 <<'ZL'
fn deep(n) {
    if n > 300 {
        return no_such_name_anywhere
    }
    return deep(n + 1)
}
deep(0)
ZL

# ---- 7b. THE DIVERGENCE, asserted so it cannot quietly change ------------
# `1 / 0` is NOT an error in this interpreter - it produces inf and exits 0.
# The kernel's OTHER runtime disagrees: runtime_kernel.c:579 calls
# kfatal("divide by zero"), which halts the machine.
#
# So the same three characters are a silent inf on one path and a dead machine
# on the other, and which one a zl program gets depends on whether it was
# INTERPRETED or COMPILED IN. That is worth a gate on its own: whichever way it
# is eventually settled, it should be settled deliberately and not discovered
# by someone whose script divided by zero.
checks=$((checks+1))
echo 'print(1 / 0)' > "$TMP/dz.zl"
got=$(timeout 20 "$INTERP" "$TMP/dz.zl" 2>&1); rc=$?
if [ "$rc" = 0 ] && [ "$got" = "inf" ]; then
    echo "  ok    1/0 is inf in the interpreter (runtime_kernel.c:579 kfatals - see T-EXEC-8)"
else
    echo "  FAIL  1/0 changed behaviour: rc=$rc out='$got' - update T-EXEC-8"
    fails=$((fails+1))
fi

# ---- 8. a loop that allocates on every iteration ---------------------------
# Under the kernel's arena there is no free(), so this is the shape that
# exhausts memory rather than time. It must still be the BUDGET that stops it
# here, because on the host there is a real heap and it would otherwise run
# until the OOM killer arrived.
run_case "an allocating loop is stopped by the budget" 2 "step budget exhausted" 40000 0 <<'ZL'
s = ""
while 1 == 1 {
    s = s + "x"
}
ZL

# ---- 8b. UNBOUNDED WORK IN ONE STEP - the hole an adversarial reader found -
# The budget originally counted visits to eval()/exec(), which bounds how many
# NODES run and says nothing about how much work each one does. One builtin call
# is one step no matter how big it is:
#
#     ./interp --steps 100 --depth 50 <<< 'xs = range(50000000)'
#     50000000          <- exit 0. A hundred steps of budget, fifty million
#                          allocations, and the program won.
#
# Capping range() would have been whack-a-mole; every container-building builtin
# has the same shape and the next one added walks straight past. The budget is
# charged PER BYTE ALLOCATED instead, at the single allocation seam, so a new
# builtin cannot escape it without avoiding memory altogether.
run_case "one huge builtin call cannot outrun the budget" 2 "step budget exhausted" 100 50 <<'ZL'
xs = range(50000000)
print(len(xs))
ZL

run_case "...nor a huge one nested inside another" 2 "step budget exhausted" 1000 50 <<'ZL'
xs = reverse(range(200000))
print(len(xs))
ZL

run_case "...nor one built by repeated concatenation" 2 "step budget exhausted" 2000 50 <<'ZL'
s = repeat("x", 10000000)
print(len(s))
ZL

# ---- 9. THE CONTROL CASE, and it is the one that catches a lazy fix --------
# A budget that stops everything is not a budget, it is a broken interpreter.
# These have to RUN, produce the right answer, and exit 0.
checks=$((checks+1))
cat > "$TMP/ok.zl" <<'ZL'
fn fib(n) {
    if n < 2 { return n }
    return fib(n - 1) + fib(n - 2)
}
print(fib(20))
ZL
got=$(timeout 20 "$INTERP" --steps 10000000 --depth 500 "$TMP/ok.zl" 2>&1)
if [ "$got" = "6765" ]; then
    echo "  ok    a program that terminates still runs (fib 20 = 6765)"
else
    echo "  FAIL  a legitimate program was broken by the limits: $got"
    fails=$((fails+1))
fi

# ---- 10. the limits OFF are the old behaviour, exactly --------------------
checks=$((checks+1))
got=$(timeout 20 "$INTERP" "$TMP/ok.zl" 2>&1)
if [ "$got" = "6765" ]; then
    echo "  ok    with no limits set, nothing changed for the hosted interpreter"
else
    echo "  FAIL  unlimited mode is broken: $got"
    fails=$((fails+1))
fi

# ---- 11. the budget is EXACT, not approximate -----------------------------
# A budget that is off by a variable amount cannot be reasoned about, and the
# gate above would still pass. Same program, same budget, same step count -
# twice - is what makes this reproducible rather than merely bounded, and
# reproducible is the property a non-timing-sensitive gate needs.
checks=$((checks+1))
a=$(timeout 20 "$INTERP" --steps 5000 "$TMP/ok.zl" 2>&1 | grep -o "steps used [0-9]*")
b=$(timeout 20 "$INTERP" --steps 5000 "$TMP/ok.zl" 2>&1 | grep -o "steps used [0-9]*")
if [ -n "$a" ] && [ "$a" = "$b" ]; then
    echo "  ok    the budget is deterministic - two runs agree ($a)"
else
    echo "  FAIL  the budget is not deterministic: '$a' vs '$b'"
    fails=$((fails+1))
fi

echo
echo "$checks cases, $fails failures"
if [ "$fails" = 0 ]; then
    echo "ok    every program written to wedge the machine was stopped, and said why"
    exit 0
fi
exit 1
