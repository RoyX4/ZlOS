#!/usr/bin/env bash
# kernel/tools/checks/check-memmap.sh - assert the hand-placed buffers do not overlap.
#
# Several zl-language buffers still live at fixed addresses below the heap and
# DMA arenas. Nothing checks these low buffers at runtime: two that overlap
# simply corrupt each other, and the symptom shows up far from the cause. The
# retired numbered RAM filesystem once overlapped LINE_BUF/HIST_BUF here; keep
# that incident as the reason this discovery gate still exists.
#
# The sizes are DERIVED from the constants in kernel.zl rather than repeated
# here, so bumping FS_SLOT or HIST_N re-runs the arithmetic instead of
# silently invalidating it.
#
# Static: it parses source and does arithmetic. No build, no QEMU, no timing,
# so it cannot fail because the host is busy.
set -uo pipefail
SELF=$(realpath "$0")
cd "$(dirname "$0")/../.." || exit

if [ "${1:-}" = "--selftest" ]; then
    fixture=$(mktemp)
    log=$(mktemp)
    trap 'rm -f "$fixture" "$log"' EXIT
    cp src/kernel.zl "$fixture"
    printf '\nCODEX_DUPLICATE = 0x02030000\n' >> "$fixture"
    if "$SELF" "$fixture" >"$log" 2>&1; then
        echo "FAIL: duplicate-address mutation escaped"
        exit 1
    fi
    grep -q "two fixed constants share an address" "$log" || {
        echo "FAIL: mutation failed for the wrong reason"
        tail -20 "$log"
        exit 1
    }
    echo "check-memmap selftest: caught duplicate-address mutation"
    exit 0
fi

SRC=${1:-src/kernel.zl}
[ -f "$SRC" ] || { echo "FAIL: no $SRC"; exit 1; }
ARENA_SRC=src/core/arena.c
MEMMAP=src/arch/x86/memmap.h
CLIP_SRC=src/graphics/windowing/clip.c
FS_SRC=src/fs/fs.c

# Pull `NAME = value` out of the source. A missing constant aborts: parsed in
# the main shell, deliberately NOT via a helper called as `X=$(konst ...)`,
# because `exit` inside a command substitution only leaves the subshell - the
# script would carry on with an empty value, size everything at 0 and report
# a clean map. A check that can pass vacuously is worse than no check.
# ---- DISCOVERY SWEEP: every fixed address, not a hand-written list -------
#
# The list below is hand-maintained and that is exactly how LINE_BUF and
# DISK_SCRATCH both sat on 0x02030000 through an entire eleven-branch
# integration. DISK_SCRATCH arrived on desktop/system-track; this file was
# written on another branch and never learned the name, so the check ran, passed
# and proved nothing. A detector that cannot see a new constant is not a
# detector - it is a green light with a hardcoded allowlist.
#
# So before anything else: pull EVERY `NAME = 0xADDR` out of the source and fail
# on any address claimed twice. No list to keep in step, and a constant added
# tomorrow is covered the moment it is written.
dupes=$(grep -oP '^[A-Z_]+\s*=\s*\K0x0[0-9A-Fa-f]{5,}' "$SRC" | sort | uniq -d)
if [ -n "$dupes" ]; then
    echo "FAIL: two fixed constants share an address:"
    for a in $dupes; do
        printf '  %s  <- ' "$a"
        grep -oP "^\K[A-Z_]+(?=\s*=\s*$a\b)" "$SRC" | tr '\n' ' '
        echo
    done
    exit 1
fi

# ...and say which constants the sized checks below do NOT cover, so the gap is
# visible rather than silent. Not a failure: a new address is not automatically
# wrong, it is automatically unexamined.
known=" SNAKE_X SNAKE_Y LINE_BUF LINE_MAX HIST_BUF HIST_N DISK_SCRATCH PAINT_BUF PAINT_MAX FILES_NAME_BUF EDIT_BUF EDIT_MAX "
unsized=""
for n in $(grep -oP '^\K[A-Z_]+(?=\s*=\s*0x0[0-9A-Fa-f]{5,})' "$SRC" | sort -u); do
    case "$known" in *" $n "*) ;; *) unsized="$unsized $n";; esac
done
[ -n "$unsized" ] && echo "note: fixed addresses with no size check here:$unsized"

declare -A K
for name in SNAKE_X SNAKE_Y \
            LINE_BUF LINE_MAX HIST_BUF HIST_N DISK_SCRATCH \
            PAINT_BUF PAINT_MAX FILES_NAME_BUF \
            EDIT_BUF EDIT_MAX; do
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
LINE_BUF=${K[LINE_BUF]}; LINE_MAX=${K[LINE_MAX]}
HIST_BUF=${K[HIST_BUF]}; HIST_N=${K[HIST_N]}
DISK_SCRATCH=${K[DISK_SCRATCH]}
PAINT_BUF=${K[PAINT_BUF]}; PAINT_MAX=${K[PAINT_MAX]}
FILES_NAME_BUF=${K[FILES_NAME_BUF]}
EDIT_BUF=${K[EDIT_BUF]}; EDIT_MAX=${K[EDIT_MAX]}

# The scratch buffer's largest caller is the clipboard, and the Files name
# buffer is owned by zlfs. Read both public bounds from their implementation
# owners instead of restating literals here or requiring duplicate zl globals.
DISK_SCRATCH_SIZE=$(grep -oP '^#define\s+CLIP_MAX\s+\K[0-9]+' "$CLIP_SRC")
FILES_NAME_SIZE=$(grep -oP '^#define\s+FS_NAME_MAX\s+\K[0-9]+' "$FS_SRC")
[ -n "$DISK_SCRATCH_SIZE" ] || { echo "FAIL: CLIP_MAX not found in $CLIP_SRC"; exit 1; }
[ -n "$FILES_NAME_SIZE" ] || { echo "FAIL: FS_NAME_MAX not found in $FS_SRC"; exit 1; }

# SNAKE_X/SNAKE_Y are one byte per body cell and the code bounds neither by a
# named constant; the gap between them is what each actually gets.
SNAKE_CELLS=$((SNAKE_Y - SNAKE_X))

# THE PROGRAM ARENA, WHICH WAS IN NEITHER CHECKER.
#
# check-memmap.sh sweeps kernel.zl's fixed buffers against each other from
# 32 MiB up. memmap.h's _Static_assert chain covers the high map from 128 MiB
# up. arena.c's 16 MiB arena sits between the two and was in NEITHER - a region
# larger than everything this script checks put together, invisible to it.
#
# It never mattered while the arena was at 8 MiB with 24 MiB of clearance. It
# started mattering the moment the arena MOVED (8 -> 14 MiB, when the raw
# loader's CHUNKS went to 192), because a move is when a gap gets spent. Read
# from arena.c rather than restated, so this cannot drift from the value the
# kernel actually uses.
AB=$(grep -oP '^#define\s+ARENA_BASE\s+\K0x[0-9A-Fa-f]+' "$ARENA_SRC")
AZ=$(grep -oP '^#define\s+ARENA_BYTES\s+\K0x[0-9A-Fa-f]+' "$ARENA_SRC")

# FOLLOW ONE LEVEL OF INDIRECTION. arena.c stopped restating literals and now
# DERIVES both from memmap.h - which is the entire point of memmap.h - and this
# gate could only read literals, so it failed with "not found" against correct
# code. A gate that cannot see the CORRECT form of what it checks is worse than
# no gate: it trains you to ignore it. It also earned its keep on the way past:
# the derivation exposed that memmap.h's LO_ARENA still said 8 MiB while the
# arena had moved to 14, which would have put a 16 MiB arena on top of the
# raw-boot stack at 12 MiB.
if [ -z "$AB" ]; then
    n=$(grep -oP '^#define\s+ARENA_BASE\s+\K[A-Z_][A-Z0-9_]*' "$ARENA_SRC")
    [ -n "$n" ] && AB=$(grep -oP "^#define\\s+${n}\\s+\\K0x[0-9A-Fa-f]+" "$MEMMAP")
fi
if [ -z "$AZ" ]; then
    e=$(grep -oP '^#define\s+LO_ARENA_END\s+\K0x[0-9A-Fa-f]+' "$MEMMAP")
    b=$(grep -oP '^#define\s+LO_ARENA\s+\K0x[0-9A-Fa-f]+' "$MEMMAP")
    [ -n "$e" ] && [ -n "$b" ] && AZ=$(printf '0x%X' $(( e - b )))
fi
[ -n "$AB" ] && [ -n "$AZ" ] || { echo "FAIL: ARENA_BASE/ARENA_BYTES not found in arena.c or memmap.h"; exit 1; }

# name:start:size - keep in sync with the map comment in kernel.zl
REGIONS=(
    "ARENA:$((AB)):$((AZ))"
    "SNAKE_X:$SNAKE_X:$SNAKE_CELLS"
    "SNAKE_Y:$SNAKE_Y:$SNAKE_CELLS"
    "LINE_BUF:$LINE_BUF:$LINE_MAX"
    "HIST_BUF:$HIST_BUF:$((HIST_N * HIST_STRIDE))"
    "DISK_SCRATCH:$DISK_SCRATCH:$DISK_SCRATCH_SIZE"
    "PAINT_BUF:$PAINT_BUF:$((PAINT_MAX * 4))"
    "FILES_NAME_BUF:$FILES_NAME_BUF:$FILES_NAME_SIZE"
    "EDIT_BUF:$EDIT_BUF:$EDIT_MAX"
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
