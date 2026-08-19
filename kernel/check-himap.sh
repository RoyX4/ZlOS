#!/usr/bin/env bash
# check-himap.sh - is any C file squatting inside a declared high-RAM region?
#
# WHY THIS EXISTS, AND WHY check-memmap.sh COULD NOT DO IT
# -------------------------------------------------------
# check-memmap.sh's discovery sweep is the right idea and covers the wrong half.
# It reads `kernel.zl` and NOTHING ELSE, and matches only `UPPERCASE = 0x...` at
# the start of a line. So the entire C-side map - memmap.h's HI_* regions, and
# any raw hex literal in a .c - is outside its vision.
#
# That is not hypothetical. `intel.c` had
#
#     static uptr edid_buf = 0x0C980000u;
#
# which is 9.5 MiB into fb.c's 16 MiB cached-blur arena. fb.c hands out that
# space and knew nothing of intel.c; intel.c did not include memmap.h at all.
# check-memmap.sh printed a clean map and said OK for as long as both existed.
# The same shape put i2c_hid.c's buffers at 0x0C900000 before that.
#
# THE RULE, and it needs no allowlist to rot
# ------------------------------------------
# A literal that EQUALS a region base is fine - that is a deliberate
# cross-check, and fb.c and i2c_hid.c both do it on purpose with a
# _Static_assert beside it. A literal that lands STRICTLY INSIDE a region
# without being its base is the defect: it is someone picking an address inside
# a neighbour's allocation, which is exactly what nothing else here can see.
#
# COMMENTS ARE STRIPPED FIRST. memmap.h and i2c_hid.c both quote the OLD bad
# addresses in prose explaining the fix, and intel.c now quotes 0x0C980000 in
# the comment recording what it used to be. A checker that flagged those would
# be unusable within a day and would be turned off.
#
# Static: parses source, no build, no QEMU, no timing. Cannot fail because the
# host is busy.
set -uo pipefail
cd "$(dirname "$0")"

MAP=memmap.h
[ -f "$MAP" ] || { echo "FAIL: no $MAP"; exit 1; }

# --- the declared regions, in ascending order -------------------------------
names=(); bases=()
while read -r n v; do
    names+=("$n"); bases+=("$((v))")
done < <(grep -oP '^#define\s+\K(HI_[A-Z]+)\s+(0x[0-9A-Fa-f]+)' "$MAP" | awk '{print $1, $2}')

[ "${#names[@]}" -ge 2 ] || { echo "FAIL: found ${#names[@]} HI_ regions in $MAP"; exit 1; }

echo "  high-RAM regions declared in $MAP:"
for i in "${!names[@]}"; do
    printf '    0x%08X  %s\n' "${bases[$i]}" "${names[$i]}"
done

lo=${bases[0]}
hi=${bases[$(( ${#bases[@]} - 1 ))]}

# --- every hex literal in C, with comments removed --------------------------
# `gcc -fpreprocessed -dD -E` is the reliable comment stripper; a regex for /* */
# gets block comments spanning lines wrong, and that is precisely where the old
# addresses are quoted.
fail=0
found=0
for f in *.c *.h; do
    [ -f "$f" ] || continue
    [ "$f" = "$MAP" ] && continue
    stripped=$(gcc -fpreprocessed -dD -E -P "$f" 2>/dev/null) || continue

    while IFS= read -r hexlit; do
        v=$((hexlit))
        # only care about the span the map governs
        [ "$v" -ge "$lo" ] && [ "$v" -lt "$hi" ] || continue

        # PAGE-ALIGNED ONLY, and this is the rule that makes the check usable
        # rather than an allowlist that rots. A fixed buffer address in this map
        # is a page base - edid_buf was 0x0C980000, i2c_hid's were 0x0C900000 and
        # 0x0C900100 - while a 32-bit CONSTANT that happens to land in the span
        # is not. crypto.c's sha256_k[] contains 0x0FC19DC6, a SHA-256 round
        # constant that sits inside HI_VGPU by pure coincidence and was the very
        # first thing this script flagged. One misaligned false positive is all
        # it takes for a checker to get switched off.
        [ $((v & 0xFFF)) -eq 0 ] || continue
        found=$((found+1))

        # equal to a base? deliberate cross-check, allowed.
        isbase=0
        for b in "${bases[@]}"; do [ "$v" -eq "$b" ] && { isbase=1; break; }; done
        [ "$isbase" = 1 ] && continue

        # strictly inside which region? owner = the highest base below it,
        # ceiling = the next base above. Report the REGION's span, not the
        # literal's own value - the first version printed $v as the start,
        # which made every message say the region began exactly where the
        # offending address was.
        owner=""; obase=0; ceil=0
        for i in "${!bases[@]}"; do
            if [ "$v" -gt "${bases[$i]}" ]; then owner="${names[$i]}"; obase=${bases[$i]}; fi
        done
        for i in "${!bases[@]}"; do
            if [ "${bases[$i]}" -gt "$v" ]; then ceil=${bases[$i]}; break; fi
        done
        printf 'FAIL: %s uses %s - inside %s (0x%08X..0x%08X), which it does not own\n' \
               "$f" "$hexlit" "$owner" "$obase" "$ceil"
        echo "      A raw address inside a declared region is the edid_buf defect."
        echo "      Either give it its own region in $MAP, or - better - find out"
        echo "      whether it needs a fixed physical address at all."
        fail=1
    done < <(grep -oP '0x0[0-9A-Fa-f]{7}(?![0-9A-Fa-f])' <<<"$stripped" | sort -u)
done

if [ "$fail" = 0 ]; then
    echo "  OK: $found in-range literals, every one of them a declared region base"
    exit 0
fi
exit 1
