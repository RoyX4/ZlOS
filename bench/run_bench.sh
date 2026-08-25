#!/usr/bin/env bash
# run_bench.sh - the Linux benchmark harness for the unboxing work.
#
# The Windows original (bench\run_bench.ps1, referenced throughout
# docs/design/PLAN_unboxing.md) does not exist in this repo. This is its
# replacement, and it differs from the plan's description in one way that
# matters - see CONTROL below.
#
# Usage:
#   ./run_bench.sh                 all benchmarks, 5 runs each
#   ./run_bench.sh -o b2           only b2_arith
#   ./run_bench.sh -r 9            9 runs
#
# BEST OF N, NEVER THE MEAN. This box has 4 cores and routinely runs QEMU for
# the kernel gates; a mean tracks host load and has already produced one
# phantom regression in this project (see kernel/CLAUDE.md, "Gates must never
# be timing-sensitive"). The minimum of N runs is the closest thing to the
# uncontended cost that a loaded box can report.
#
# CONTROL. PLAN_unboxing.md §4 says the interpreter column is a free control
# because interp does not link runtime.c. THAT IS A WINDOWS FACT AND IT IS
# FALSE HERE: build.sh:11 links runtime.c into interp. It is still a valid
# control, but for a different reason - interp.c contains ZERO calls to
# zl_binop (it has its own Value and its own eval), so a zl_binop change
# cannot move the interp column. Verified with: grep -c zl_binop interp.c
#
# If the interp column moves across a runtime.c-only change, the measurement
# is contaminated by host load and the run should be discarded.
set -uo pipefail
cd "$(dirname "$0")" || exit
ROOT=$(cd .. && pwd)

ONLY=""
RUNS=5
while getopts "o:r:" opt; do
    case $opt in
        o) ONLY=$OPTARG ;;
        r) RUNS=$OPTARG ;;
        *) echo "usage: $0 [-o name] [-r runs]" >&2; exit 2 ;;
    esac
done

[ -x "$ROOT/compile" ] || { echo "build the toolchain first: ./build.sh" >&2; exit 1; }
[ -x "$ROOT/interp" ]  || { echo "build the toolchain first: ./build.sh" >&2; exit 1; }

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

# best_ms CMD... - run it $RUNS times, echo the fastest wall-clock in ms.
# Correctness is checked too: every run must produce identical stdout, so a
# benchmark that starts returning a different answer fails loudly rather than
# quietly becoming a benchmark of the wrong thing.
#
# The program's answer goes to $tmp/.answer, NOT to a variable: callers use
# `cms=$(best_ms ...)`, which runs this in a subshell, so an assignment here
# would never reach the caller. Under `set -u` that failed loudly rather than
# silently, which is the right way round.
best_ms() {
    local best=999999999 first="" out t0 t1 ms
    for _ in $(seq "$RUNS"); do
        t0=$(date +%s%N)
        out=$("$@" 2>&1)
        t1=$(date +%s%N)
        ms=$(( (t1 - t0) / 1000000 ))
        if [ -z "$first" ]; then first=$out
        elif [ "$out" != "$first" ]; then
            echo "UNSTABLE OUTPUT: '$out' != '$first'" >&2
            return 1
        fi
        [ "$ms" -lt "$best" ] && best=$ms
    done
    printf '%s' "$first" > "$tmp/.answer"
    echo "$best"
}

printf '%-12s %12s %12s %10s   %s\n' benchmark "boxed-C ms" "interp ms" speedup answer
printf '%-12s %12s %12s %10s   %s\n' ------------ ------------ ------------ ---------- ------

fail=0
for src in b*.zl; do
    name=${src%.zl}
    [ -n "$ONLY" ] && [[ $name != $ONLY* ]] && continue

    # C backend: compile writes out.c into the CWD, not next to the source.
    ( cd "$tmp" && "$ROOT/compile" "$OLDPWD/$src" >/dev/null 2>&1 && \
      gcc -O2 -D_strdup=strdup -I"$ROOT/src/runtime" -o "$name.bin" out.c \
          "$ROOT/src/runtime/runtime.c" "$ROOT/src/runtime/os_linux.c" -lm 2>"$name.cc.err" )
    if [ ! -x "$tmp/$name.bin" ]; then
        printf '%-12s %12s %12s %10s   %s\n' "$name" BUILD-FAIL - - "see $tmp/$name.cc.err"
        fail=1
        continue
    fi

    cms=$(best_ms "$tmp/$name.bin")   || { fail=1; continue; }
    cans=$(cat "$tmp/.answer")
    ims=$(best_ms "$ROOT/interp" "$src") || { fail=1; continue; }
    ians=$(cat "$tmp/.answer")

    # The two engines must agree. This is the same cross-check run_tests.sh
    # does; a benchmark where they disagree is measuring a bug.
    if [ "$cans" != "$ians" ]; then
        printf '%-12s %12s %12s %10s   %s\n' "$name" "$cms" "$ims" MISMATCH "C=$cans interp=$ians"
        fail=1
        continue
    fi

    if [ "$cms" -gt 0 ]; then
        spd=$(awk -v i="$ims" -v c="$cms" 'BEGIN{printf "%.2fx", i/c}')
    else
        spd="-"
    fi
    printf '%-12s %12s %12s %10s   %s\n' "$name" "$cms" "$ims" "$spd" "$cans"
done

echo
echo "best of $RUNS runs; load average now: $(cut -d' ' -f1-3 /proc/loadavg)"
exit $fail
