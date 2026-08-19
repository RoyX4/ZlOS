#!/usr/bin/env bash
# kernel/check-memmap.sh - assert the hand-placed buffers do not overlap.
#
# zlOS has no heap (pci.c says so out loud), so every buffer in the 32 MiB
# region is a fixed address picked by hand. Nothing checks them at runtime:
# two that overlap simply corrupt each other, and the symptom shows up far
# from the cause. That already happened once - LINE_BUF and HIST_BUF were
# placed inside FS_DATA's slots 7 and 8, so editing a RAM file overwrote the
# shell's input line and its history ring, and typing at the prompt overwrote
# the files.
#
# The sizes are DERIVED from the constants in kernel.zl rather than repeated
# here, so bumping FS_SLOT or HIST_N re-runs the arithmetic instead of
# silently invalidating it.
#
# Static: it parses source and does arithmetic. No build, no QEMU, no timing,
# so it cannot fail because the host is busy.
set -uo pipefail
cd "$(dirname "$0")"

SRC=${1:-kernel.zl}
[ -f "$SRC" ] || { echo "FAIL: no $SRC"; exit 1; }

# Pull `NAME = value` out of the source. A missing constant aborts: parsed in
# the main shell, deliberately NOT via a helper called as `X=$(konst ...)`,
# because `exit` inside a command substitution only leaves the subshell - the
# script would carry on with an empty value, size everything at 0 and report
# a clean map. A check that can pass vacuously is worse than no check.
declare -A K
for name in SNAKE_X SNAKE_Y FS_META FS_DATA FS_SLOT \
            LINE_BUF LINE_MAX HIST_BUF HIST_N; do
    v=$(grep -oP "^$name\s*=\s*\K(0x[0-9A-Fa-f]+|[0-9]+)" "$SRC" | head -1)
    [ -n "$v" ] || { echo "FAIL: constant $name not found in $SRC"; exit 1; }
    K[$name]=$((v))
done

# The per-entry history stride is a literal in hist_save/hist_load, not a
# named constant - read it back from the code so this cannot drift from it.
HIST_STRIDE=$(grep -oP 'HIST_BUF \+ hslot \* \K[0-9]+' "$SRC" | sort -u)
[ -n "$HIST_STRIDE" ] || { echo "FAIL: history stride not found in $SRC"; exit 1; }
[ "$(printf '%s\n' "$HIST_STRIDE" | wc -l)" = 1 ] || {
    echo "FAIL: hist_save/hist_load disagree on the history stride"; exit 1; }

SNAKE_X=${K[SNAKE_X]}; SNAKE_Y=${K[SNAKE_Y]}
FS_META=${K[FS_META]}; FS_DATA=${K[FS_DATA]}; FS_SLOT=${K[FS_SLOT]}
LINE_BUF=${K[LINE_BUF]}; LINE_MAX=${K[LINE_MAX]}
HIST_BUF=${K[HIST_BUF]}; HIST_N=${K[HIST_N]}

# SNAKE_X/SNAKE_Y are one byte per body cell and the code bounds neither by a
# named constant; the gap between them is what each actually gets.
SNAKE_CELLS=$((SNAKE_Y - SNAKE_X))

# name:start:size - keep in sync with the map comment in kernel.zl
REGIONS=(
    "SNAKE_X:$SNAKE_X:$SNAKE_CELLS"
    "SNAKE_Y:$SNAKE_Y:$SNAKE_CELLS"
    "FS_META:$FS_META:64"
    "FS_DATA:$FS_DATA:$((10 * FS_SLOT))"
    "LINE_BUF:$LINE_BUF:$LINE_MAX"
    "HIST_BUF:$HIST_BUF:$((HIST_N * HIST_STRIDE))"
)

mapfile -t SORTED < <(printf '%s\n' "${REGIONS[@]}" | sort -t: -k2 -n)

echo "  zlOS fixed-address map (kernel.zl's block ONLY - see the note below):"
for r in "${SORTED[@]}"; do
    IFS=: read -r n s z <<<"$r"
    printf '    0x%08X .. 0x%08X  %-9s %7d bytes\n' "$s" "$((s + z))" "$n" "$z"
done

# A full pairwise sweep, not just adjacent pairs: a region long enough to span
# a whole neighbour would slip past an adjacent-only comparison.
fail=0
for i in "${!SORTED[@]}"; do
    for j in "${!SORTED[@]}"; do
        [ "$i" -lt "$j" ] || continue
        IFS=: read -r an as az <<<"${SORTED[$i]}"
        IFS=: read -r bn bs bz <<<"${SORTED[$j]}"
        if [ $((as + az)) -gt "$bs" ] && [ $((bs + bz)) -gt "$as" ]; then
            printf 'FAIL: %s (0x%08X..0x%08X) overlaps %s (0x%08X..0x%08X)\n' \
                "$an" "$as" "$((as + az))" "$bn" "$bs" "$((bs + bz))"
            fail=1
        fi
    done
done

# The history slot must hold a full-length line plus its NUL terminator,
# or hist_save writes past the end of its slot into the next one.
if [ "$LINE_MAX" -ge "$HIST_STRIDE" ]; then
    echo "FAIL: LINE_MAX $LINE_MAX does not fit in history stride $HIST_STRIDE"
    fail=1
fi

[ "$fail" = 0 ] || exit 1
echo "  OK: no overlaps, $((HIST_STRIDE - LINE_MAX)) bytes spare in each history slot"

# THIS IS NOT THE WHOLE MAP, AND READING IT AS THOUGH IT WERE COST A P1.
# There are TWO fixed-address maps in this kernel:
#   this one   - kernel.zl's block at 32 MiB, derived from the source above
#   memmap.h   - the high-RAM regions (fb.c's back buffer, the DMA arenas,
#                png.c's decoded pictures), checked by _Static_assert
# A picture arena was placed at 0x02000000 because every assert in memmap.h
# passed - and it landed directly on SNAKE_X, FS_META, FS_DATA, LINE_BUF and
# HIST_BUF, all of which THIS script had just printed in the same session.
# The output was read and not joined up. memmap.h now declares
# ZL_LOW_BASE/ZL_LOW_END so the compiler checks across the boundary, but a
# human reading either map alone still sees half of one.
echo
echo "  NOTE: this covers kernel.zl only. The high-RAM regions (fb.c, the DMA"
echo "        arenas, png.c's pictures) live in memmap.h and are checked by"
echo "        _Static_assert there. Neither map is the whole map."
