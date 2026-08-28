#!/usr/bin/env bash
# verify-clock.sh - the RTC, against a clock whose value we chose.
#
# The obvious gate is "boot, print the time, compare with the host". It is the
# wrong one on THIS machine: CLAUDE.md records that the hardware clock here runs
# behind and NTP corrects it mid-session, so a comparison against the host is a
# comparison against a moving target, and the gate would fail for reasons that
# have nothing to do with the driver.
#
# So the guest's clock is SET, with `-rtc base=`, and the expected starting
# epoch is known exactly. QEMU advances that clock while the guest boots, so
# the observed second may be a few seconds later; requiring the configured
# second exactly made machine load look like an RTC decoder failure. The gate
# instead requires a small nonnegative offset and requires the printed fields
# to convert back to the epoch the guest printed. That still tests BCD, the
# century register and leap-day decoding without pretending boot takes no time.
#
# Three bases, chosen for what each one breaks:
#   2026-08-18 14:37:05  ordinary
#   2026-12-31 23:59:07  every field near its maximum; 0x59 must decode as 59
#                        and not 89, which is the whole point of BCD
#   2000-02-29 12:00:00  a leap day in a century year - the one date a naive
#                        "divisible by 4" calendar gets wrong
set -uo pipefail
cd "$(dirname "$0")/../.." || exit
. tools/checks/qemu-crash.sh

CEILING=${CEILING:-240}
MAX_BOOT_SECONDS=${MAX_BOOT_SECONDS:-30}
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
    kill "$qpid" 2>/dev/null; wait "$qpid" 2>/dev/null; local qstatus=$?
    tr -d '\r' < "$OUT" > "$OUT.c" && mv "$OUT.c" "$OUT"

    qemu_crashed "$qstatus" || true

    local got_rtc got_epoch
    got_rtc=$(sed -n 's/.*RTC=\([0-9:-]* [0-9:]*\).*/\1/p' "$OUT" | tail -1)
    got_epoch=$(sed -n 's/.*epoch=\([0-9]*\).*/\1/p' "$OUT" | tail -1)

    local rtc_epoch offset
    rtc_epoch=$(date -u -d "$got_rtc" +%s 2>/dev/null || true)
    offset=0
    if [[ "$got_epoch" =~ ^[0-9]+$ ]]; then
        offset=$((got_epoch - want_epoch))
    else
        offset=$((MAX_BOOT_SECONDS + 1))
    fi

    if [ "$rtc_epoch" = "$got_epoch" ] && [ "$offset" -ge 0 ] && [ "$offset" -le "$MAX_BOOT_SECONDS" ]; then
        echo "ok    $base -> $got_rtc  epoch $got_epoch (+${offset}s boot)"
    else
        echo "FAIL  $base"
        echo "        want start RTC=$want_rtc epoch=$want_epoch, offset 0..${MAX_BOOT_SECONDS}s"
        echo "        got  RTC=${got_rtc:-<none>} epoch=${got_epoch:-<none>}"
        echo "        parsed epoch=${rtc_epoch:-<invalid>} offset=${offset}s"
        fails=$((fails + 1))
    fi
}

# expected epochs come from `date -u -d ... +%s`, never from arithmetic by hand
check 2026-08-18T14:37:05 "2026-08-18 14:37:05" 1787063825
check 2026-12-31T23:59:07 "2026-12-31 23:59:07" 1798761547
check 2000-02-29T12:00:00 "2000-02-29 12:00:00" 951825600

[ "$fails" = 0 ] && { echo "ok    the clock decodes every base correctly"; exit 0; }
echo "FAIL: $fails of 3"; exit 1
