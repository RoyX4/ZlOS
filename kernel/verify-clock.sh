#!/usr/bin/env bash
# verify-clock.sh - the RTC, against a clock whose value we chose.
#
# The obvious gate is "boot, print the time, compare with the host". It is the
# wrong one on THIS machine: CLAUDE.md records that the hardware clock here runs
# behind and NTP corrects it mid-session, so a comparison against the host is a
# comparison against a moving target, and the gate would fail for reasons that
# have nothing to do with the driver.
#
# So the guest's clock is SET, with `-rtc base=`, and the expected output is
# known exactly. That makes this a test of the decoder - BCD, the century
# register, the update-in-progress flag - rather than a test of whether two
# clocks agree. Which is the part that can actually be wrong.
#
# Three bases, chosen for what each one breaks:
#   2026-08-18 14:37:05  ordinary
#   2026-12-31 23:59:07  every field near its maximum; 0x59 must decode as 59
#                        and not 89, which is the whole point of BCD
#   2000-02-29 12:00:00  a leap day in a century year - the one date a naive
#                        "divisible by 4" calendar gets wrong
set -uo pipefail
cd "$(dirname "$0")" || exit

CEILING=${CEILING:-240}
OUT=$(mktemp); trap 'rm -f "$OUT"' EXIT

if pgrep '^qemu-system' >/dev/null 2>&1; then
    echo "SKIP: another qemu-system is running; §1.2 allows one."; exit 2
fi

./build.sh >/dev/null 2>&1 || { echo "FAIL: kernel did not build"; exit 1; }

fails=0
check () {
    local base=$1 want_rtc=$2 want_epoch=$3
    : > "$OUT"
    # '.' throwaway (QEMU can eat the first serial byte), ';' the clock, 'q' halt
    printf '.;q' | timeout "$CEILING" qemu-system-i386 \
        -kernel kernel.elf -m 1G -rtc "base=$base" \
        -serial stdio -display none -no-reboot >"$OUT" 2>/dev/null &
    local qpid=$!
    for _ in $(seq $((CEILING * 2))); do
        grep -q "halting" "$OUT" 2>/dev/null && break
        kill -0 "$qpid" 2>/dev/null || break
        sleep 0.5
    done
    kill "$qpid" 2>/dev/null; wait "$qpid" 2>/dev/null
    tr -d '\r' < "$OUT" > "$OUT.c" && mv "$OUT.c" "$OUT"

    local got_rtc got_epoch
    got_rtc=$(sed -n 's/.*RTC=\([0-9:-]* [0-9:]*\).*/\1/p' "$OUT" | tail -1)
    got_epoch=$(sed -n 's/.*epoch=\([0-9]*\).*/\1/p' "$OUT" | tail -1)

    if [ "$got_rtc" = "$want_rtc" ] && [ "$got_epoch" = "$want_epoch" ]; then
        echo "ok    $base -> $got_rtc  epoch $got_epoch"
    else
        echo "FAIL  $base"
        echo "        want RTC=$want_rtc epoch=$want_epoch"
        echo "        got  RTC=${got_rtc:-<none>} epoch=${got_epoch:-<none>}"
        fails=$((fails + 1))
    fi
}

# expected epochs come from `date -u -d ... +%s`, never from arithmetic by hand
check 2026-08-18T14:37:05 "2026-08-18 14:37:05" 1787063825
check 2026-12-31T23:59:07 "2026-12-31 23:59:07" 1798761547
check 2000-02-29T12:00:00 "2000-02-29 12:00:00" 951825600

[ "$fails" = 0 ] && { echo "ok    the clock decodes every base correctly"; exit 0; }
echo "FAIL: $fails of 3"; exit 1
