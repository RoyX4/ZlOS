#!/usr/bin/env bash
# zladdrs.sh - every fixed address in EVERY .zl file, not just kernel.zl.
#
# WHY THIS EXISTS, AND WHAT IT CATCHES THAT check-memmap.sh CANNOT.
#
# ../check-memmap.sh opens ONE file:
#
#     SRC=${1:-kernel.zl}
#
# and its own comment already explains why a detector that cannot see a new
# constant "is not a detector - it is a green light with a hardcoded
# allowlist". That comment was written about a hand-maintained LIST of names;
# the same sentence is now true of the hand-maintained list of FILES, because
# the app suite is being written as id-disjoint slices and a slice that owns
# its own state owns its own addresses. Today, measured:
#
#     kernel/apps_games3.zl   6 fixed addresses, 0x02240000..0x02245000
#     kernel/apps_games4.zl   4 fixed addresses, 0x02280000..0x02283000
#
# Ten addresses, in two files, written by two agents who could not see each
# other's work, and check-memmap.sh reads neither file. They do not collide -
# that is luck plus a deliberate moat, and luck is not a gate.
#
# Static: it greps source and does arithmetic. No build, no QEMU, no timing,
# so it cannot fail because the host is busy - which matters here, where the
# same box runs several agents and a gate that tracks load costs a bisect
# (see CLAUDE.md, "Gates must never be timing-sensitive").
#
#   ./zladdrs.sh            check the tree
#   ./zladdrs.sh --selftest prove the check can still go red
set -uo pipefail
cd "$(dirname "$0")/.." || exit 2

# The app-suite block. kernel.zl states the rule for it out loud - "One page
# (0x1000 = 4096 bytes) PER CONSUMER, deliberately generous rather than
# packed" - so above this line a page of separation is a REQUIREMENT and not a
# style. Below it live older, deliberately tighter pairs (SNAKE_X/SNAKE_Y are
# 512 bytes apart), which is why the spacing rule starts here rather than
# applying to the whole map.
SUITE_FLOOR=$((0x02200000))
PAGE=$((0x1000))
# virtio_net.c: NET_FLOOR. Nothing in this block may reach it.
NET_FLOOR=$((0x03000000))

selftest=0
[ "${1:-}" = "--selftest" ] && selftest=1

scan() {
    # NAME:VALUE:FILE for every `NAME = 0xADDR` at the top level of any .zl
    grep -HnoP '^[A-Z_][A-Z_0-9]*\s*=\s*0x0[0-9A-Fa-f]{5,}' -- *.zl 2>/dev/null \
      | sed -E 's/^([^:]+):[0-9]+:([A-Z_0-9]+)[[:space:]]*=[[:space:]]*(0x[0-9A-Fa-f]+)$/\2:\3:\1/'
    if [ "$selftest" = 1 ]; then
        # a planted collision: an address kernel.zl already claims, in a file
        # check-memmap.sh does not open. This is the exact shape of the bug.
        echo "PLANTED_DUPE:0x02200000:apps_games4.zl"
        # ...and a neighbour a quarter of a page away, inside the suite block
        echo "PLANTED_TIGHT:0x02280400:apps_games4.zl"
    fi
}

rows=$(scan | sort -t: -k2,2)
[ -n "$rows" ] || { echo "FAIL: no fixed addresses found at all - the pattern stopped matching"; exit 1; }

fail=0
printf '  fixed addresses across every .zl (%s):\n' "$(printf '%s\n' "$rows" | wc -l)"
prev_name=""; prev_val=0; prev_file=""
while IFS=: read -r name val file; do
    v=$((val))
    printf '    %-13s %-16s %s\n' "$val" "$name" "$file"
    if [ -n "$prev_name" ]; then
        if [ "$v" -eq "$prev_val" ]; then
            printf 'FAIL: %s (%s) and %s (%s) are both at %s\n' \
                   "$prev_name" "$prev_file" "$name" "$file" "$val"
            fail=1
        elif [ "$v" -ge "$SUITE_FLOOR" ] && [ "$prev_val" -ge "$SUITE_FLOOR" ] \
             && [ $((v - prev_val)) -lt "$PAGE" ]; then
            printf 'FAIL: %s (%s) is %d bytes above %s (%s) - the app-suite block is one PAGE per consumer\n' \
                   "$name" "$file" $((v - prev_val)) "$prev_name" "$prev_file"
            fail=1
        fi
    fi
    if [ "$v" -ge "$NET_FLOOR" ]; then
        printf 'FAIL: %s (%s) at %s is at or above virtio_net.c NET_FLOOR 0x03000000\n' \
               "$name" "$file" "$val"
        fail=1
    fi
    prev_name=$name; prev_val=$v; prev_file=$file
done <<EOF
$rows
EOF

if [ "$selftest" = 1 ]; then
    if [ "$fail" = 1 ]; then echo "  OK: --selftest went red, so the check can still fail"; exit 0; fi
    echo "FAIL: --selftest planted a duplicate AND a tight neighbour and the check stayed green"
    exit 1
fi

[ "$fail" = 0 ] || exit 1
echo "  OK: no two fixed addresses collide, and the app-suite block is page-spaced"
