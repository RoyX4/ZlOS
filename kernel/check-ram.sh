#!/usr/bin/env bash
# check-ram.sh - does every QEMU in this tree boot with the RAM memmap.h claims?
#
# WHY THIS EXISTS
# ---------------
# memmap.h's HI_TOP is one promise: *the smallest guest zlOS claims to boot on.*
# Every DMA buffer is placed below it because below it is the only memory we
# have said exists. That promise is only worth something if the gates honour it,
# and for the whole life of this project they did not:
#
#     verify.sh  verify-raw.sh  verify-iso.sh  run.sh      passed NO -m at all
#     verify-disk.sh  verify-clock.sh                      passed -m 512
#     verify-efi.sh  exercise.py  try.sh                   passed -m 1G
#
# qemu-system-i386 with no -m gives the guest exactly 128 MiB (measured; the
# query-memory-size-summary transcript is in arena.c:38-52). HI_TOP was
# 256 MiB. So on four of those the top HALF of the declared map was unbacked
# RAM, on two it was fine, and on three it had four times the room the header
# was asserting against. Nothing anywhere compared the two numbers.
#
# That is the GUARDS-THAT-DID-NOT-GUARD shape exactly: a constant that reads as
# a constraint, in a file full of _Static_asserts, enforcing nothing at the one
# place it is actually decided - the qemu command line, which no compiler sees.
#
# WHAT IT CHECKS, IN TWO RULES
# ----------------------------
#   1. Every `-m` in a file that launches QEMU is >= HI_TOP. One that is smaller
#      is a gate booting a machine we do not claim to support.
#   2. Every literal `qemu-system-*` launch names `-m` itself, or references a
#      shared argument array (`"${COMMON[@]}"`, or python's `common` list). This
#      is the rule that catches a NEW gate written without -m, which is how all
#      four of the above got that way.
#
# THE HOLE, NAMED RATHER THAN HIDDEN
# ----------------------------------
# Rule 2 accepts an array reference without following it. A file with two arrays
# where only one carries -m would pass both rules. That case does not exist here
# (checked: run.sh and try.sh have one array each, exercise.py one `common`), and
# closing it means expanding shell arrays in bash, which is more machinery than
# the bug it would catch. If a second array ever appears, this comment is the
# reason the check did not fire.
#
# Static: greps source and does arithmetic. No build, no QEMU, no timing, so it
# cannot fail because the host is busy.
set -uo pipefail
cd "$(dirname "$0")"

MAP=memmap.h
[ -f "$MAP" ] || { echo "FAIL: no $MAP"; exit 1; }

# HI_TOP, in MiB, read from the header rather than repeated here.
HI_TOP=$(grep -oP '^#define\s+HI_TOP\s+\K0x[0-9A-Fa-f]+' "$MAP")
[ -n "$HI_TOP" ] || { echo "FAIL: HI_TOP not found in $MAP"; exit 1; }
WANT_MB=$(( HI_TOP / 1048576 ))
printf '  memmap.h HI_TOP = %s = %d MiB - every guest must have at least this\n' \
       "$HI_TOP" "$WANT_MB"

# `-m` takes 1G / 512M / 512. Normalise all three to MiB. An unparseable value
# is a FAILURE, not a skip: a check that shrugs at what it cannot read is the
# green light with a hardcoded allowlist this tree has already been bitten by.
to_mb () {
    local v=$1
    case "$v" in
        *[gG]) echo $(( ${v%[gG]} * 1024 ));;
        *[mM]) echo "${v%[mM]}";;
        *[0-9]) echo "$v";;
        *) echo "BAD";;
    esac
}

fail=0
launches=0
checked=0

for f in *.sh *.py; do
    [ -f "$f" ] || continue
    [ "$f" = "$(basename "$0")" ] && continue

    # LOGICAL lines, not physical ones. Every qemu launch in this tree is
    # backslash-continued and the `-m` is never on the first line, so a
    # per-physical-line rule 2 flags all nine of them. This is the check's own
    # version of the bug it is looking for: reading only the first line of the
    # thing you are checking.
    mapfile -t lines < <(awk '
        { line = $0
          cont = (line ~ /\\[[:space:]]*$/)
          sub(/\\[[:space:]]*$/, "", line)
          if (buf == "") start = NR
          buf = buf line " "
          if (!cont) { print start ":" buf; buf = "" } }
        END { if (buf != "") print start ":" buf }' "$f" \
        | grep -E 'qemu-system-[a-z0-9_]+' \
        | grep -vE ':[[:space:]]*#' \
        | grep -vE 'command -v|pgrep|echo |print\(|"qemu-system-[a-z0-9_]+"\]?[[:space:]]*$')
    [ "${#lines[@]}" -gt 0 ] || continue

    checked=$((checked + 1))

    # ---- rule 1: every -m in the file is big enough ------------------------
    # Both spellings: shell `-m 1G`, python `"-m", "1G"`.
    mvals=$( { grep -oP '(?<![\w-])-m\s+\K[0-9]+[GgMm]?(?![\w])' "$f";
               grep -oP '"-m",\s*"\K[0-9]+[GgMm]?(?=")' "$f"; } | sort -u )

    if [ -z "$mvals" ]; then
        printf 'FAIL: %s launches qemu and passes no -m at all\n' "$f"
        printf '      qemu-system-i386 defaults to 128 MiB, so everything in\n'
        printf '      memmap.h above 128 MiB is unbacked RAM under it.\n'
        fail=1
        continue
    fi

    for v in $mvals; do
        mb=$(to_mb "$v")
        if [ "$mb" = "BAD" ]; then
            printf 'FAIL: %s has -m %s and this script cannot read it\n' "$f" "$v"
            fail=1
            continue
        fi
        if [ "$mb" -lt "$WANT_MB" ]; then
            printf 'FAIL: %s boots -m %s (%d MiB) - below HI_TOP (%d MiB)\n' \
                   "$f" "$v" "$mb" "$WANT_MB"
            printf '      memmap.h places buffers up to %d MiB. This gate does\n' "$WANT_MB"
            printf '      not have that memory, so those buffers are not there.\n'
            fail=1
        fi
    done

    # ---- rule 2: every launch names -m, or a shared arg array --------------
    for l in "${lines[@]}"; do
        launches=$((launches + 1))
        body=${l#*:}
        case "$body" in
            *-m\ *|*'"-m"'*) continue;;                      # names it directly
            *'[@]}"'*|*'[@]}'*) continue;;                   # shell array
            *' + common'*|*'common +'*|*'+ common'*) continue;;  # exercise.py
        esac
        printf 'FAIL: %s:%s\n' "$f" "${l%%:*}"
        printf '      this qemu launch names neither -m nor a shared arg array.\n'
        printf '      %s\n' "$(echo "$body" | sed 's/^[[:space:]]*//')"
        fail=1
    done

    printf '    %-20s %s\n' "$f" "$(echo "$mvals" | tr '\n' ' ')"
done

[ "$checked" -ge 5 ] || {
    echo "FAIL: only $checked files looked like qemu launchers - the detector broke"
    exit 1
}

if [ "$fail" = 0 ]; then
    printf '  OK: %d files, %d launches, every one at or above %d MiB\n' \
           "$checked" "$launches" "$WANT_MB"
    exit 0
fi
exit 1
