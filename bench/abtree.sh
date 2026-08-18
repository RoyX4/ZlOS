#!/usr/bin/env bash
# abtree.sh - interleaved A/B of two whole CHECKOUTS.
#
# ab.sh links one out.c against two runtime.c revisions. That is enough when
# only runtime.c changed, and useless the moment runtime.h changes too: the
# old runtime.c will not compile against the new header. A Value layout change
# is exactly that case.
#
# So this one takes two repo roots and uses each tree's OWN compile, runtime.c,
# runtime.h and os_linux.c. Same interleaving as ab.sh, same reason - see its
# header for the phantom 12.8% that motivated it.
#
# Usage:  ./abtree.sh <rootA> <rootB> [runs]
#         ./abtree.sh ~/Documents/repos/zl-linux ~/Documents/repos/zl-value16 7
set -uo pipefail
cd "$(dirname "$0")"
BENCH=$PWD

A=$(cd "${1:?usage: abtree.sh <rootA> <rootB> [runs]}" && pwd)
B=$(cd "${2:?usage: abtree.sh <rootA> <rootB> [runs]}" && pwd)
RUNS=${3:-7}

for R in "$A" "$B"; do
    [ -x "$R/compile" ] || { echo "no ./compile in $R - run its build.sh" >&2; exit 1; }
done

tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT

echo "A = $A"
echo "B = $B"
echo "interleaved, best of $RUNS; load at start: $(cut -d' ' -f1-3 /proc/loadavg)"
echo
printf '%-12s %10s %10s %9s   %s\n' benchmark "A ms" "B ms" "B/A" answer
printf '%-12s %10s %10s %9s   %s\n' ------------ ---------- ---------- --------- ------

fail=0
for src in "$BENCH"/b*.zl; do
    name=$(basename "${src%.zl}")

    # Each tree compiles the source with ITS OWN compile and links ITS OWN
    # runtime, so the comparison covers codegen and layout together - which is
    # what a Value change actually alters.
    ok=1
    for arm in A B; do
        R=$A; [ $arm = B ] && R=$B
        d=$tmp/$arm; mkdir -p "$d"
        ( cd "$d" && "$R/compile" "$src" >/dev/null 2>&1 ) || { ok=0; break; }
        gcc -O2 -D_strdup=strdup -I"$R" -o "$tmp/$name.$arm" \
            "$d/out.c" "$R/runtime.c" "$R/os_linux.c" -lm 2>"$tmp/$name.$arm.err" || { ok=0; break; }
    done
    if [ $ok -eq 0 ]; then
        printf '%-12s %10s   %s\n' "$name" BUILD-FAIL "$(head -2 "$tmp/$name."?.err 2>/dev/null | tr '\n' ' ')"
        fail=1; continue
    fi

    bestA=999999999; bestB=999999999; ansA=""; ansB=""
    for _ in $(seq "$RUNS"); do
        for arm in A B; do
            t0=$(date +%s%N); out=$("$tmp/$name.$arm" 2>&1); t1=$(date +%s%N)
            ms=$(( (t1 - t0) / 1000000 ))
            if [ $arm = A ]; then ansA=$out; [ "$ms" -lt "$bestA" ] && bestA=$ms
            else                  ansB=$out; [ "$ms" -lt "$bestB" ] && bestB=$ms; fi
        done
    done

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
