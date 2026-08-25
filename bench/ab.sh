#!/usr/bin/env bash
# ab.sh - interleaved A/B of two runtime.c revisions.
#
# WHY THIS EXISTS. run_bench.sh measures one build at a time, so an A run and a
# B run are minutes apart. On this box that is long enough for another session
# to start a QEMU gate, and the first attempt at measuring Stage 1 produced a
# b4_list "improvement" of 12.8% while its own interpreter control moved 12% -
# i.e. indistinguishable from host load. This project has already been bitten
# by exactly that once (kernel/CLAUDE.md, "Gates must never be timing-
# sensitive"), where an A/B against a clean worktree showed baseline and
# modified failing together, tracking load.
#
# The fix is to interleave: A, B, A, B, ... within the same seconds, so both
# arms see the same contention, and report best-of-N for each.
#
# Usage:  ./ab.sh <runtime_A.c> <runtime_B.c> [runs]
#         ./ab.sh /tmp/runtime-HEAD.c ../src/runtime/runtime.c 7
set -uo pipefail
cd "$(dirname "$0")" || exit
ROOT=$(cd .. && pwd)

A=${1:?usage: ab.sh <runtime_A.c> <runtime_B.c> [runs]}
B=${2:?usage: ab.sh <runtime_A.c> <runtime_B.c> [runs]}
RUNS=${3:-7}

tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT

echo "A = $A"
echo "B = $B"
echo "interleaved, best of $RUNS; load at start: $(cut -d' ' -f1-3 /proc/loadavg)"
echo

printf '%-12s %10s %10s %9s   %s\n' benchmark "A ms" "B ms" "B/A" answer
printf '%-12s %10s %10s %9s   %s\n' ------------ ---------- ---------- --------- ------

fail=0
for src in b*.zl; do
    name=${src%.zl}

    # One out.c, two link targets. The generated code is identical by
    # construction, so the ONLY difference between the two binaries is
    # which runtime.c they link - which is the thing being measured.
    ( cd "$tmp" && "$ROOT/compile" "$OLDPWD/$src" >/dev/null 2>&1 ) || {
        printf '%-12s %10s\n' "$name" COMPILE-FAIL; fail=1; continue; }
    for arm in A B; do
        rt=$A; [ $arm = B ] && rt=$B
        gcc -O2 -D_strdup=strdup -I"$ROOT/src/runtime" -o "$tmp/$name.$arm" \
            "$tmp/out.c" "$rt" "$ROOT/src/runtime/os_linux.c" -lm 2>"$tmp/$name.$arm.err" || {
            printf '%-12s %10s   %s\n' "$name" "BUILD-FAIL-$arm" "$(head -2 "$tmp/$name.$arm.err")"
            fail=1; continue 2; }
    done

    bestA=999999999; bestB=999999999; ansA=""; ansB=""
    for _ in $(seq "$RUNS"); do
        for arm in A B; do
            t0=$(date +%s%N); out=$("$tmp/$name.$arm" 2>&1); t1=$(date +%s%N)
            ms=$(( (t1 - t0) / 1000000 ))
            if [ $arm = A ]; then
                ansA=$out; [ "$ms" -lt "$bestA" ] && bestA=$ms
            else
                ansB=$out; [ "$ms" -lt "$bestB" ] && bestB=$ms
            fi
        done
    done

    # Both arms must compute the same answer. A speedup that changes the
    # result is not a speedup.
    if [ "$ansA" != "$ansB" ]; then
        printf '%-12s %10s %10s %9s   %s\n' "$name" "$bestA" "$bestB" WRONG "A=$ansA B=$ansB"
        fail=1; continue
    fi

    ratio=$(awk -v a="$bestA" -v b="$bestB" 'BEGIN{ if(b>0) printf "%.3f", b/a; else print "-" }')
    printf '%-12s %10s %10s %9s   %s\n' "$name" "$bestA" "$bestB" "$ratio" "$ansA"
done

echo
echo "B/A below 1.000 means B is faster. load at end: $(cut -d' ' -f1-3 /proc/loadavg)"
exit $fail
