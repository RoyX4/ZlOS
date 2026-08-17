#!/usr/bin/env bash
# verify-iso.sh - the slow gate: does the ISO boot BOTH ways?
#
# verify.sh boots kernel.elf directly with QEMU's -kernel, which is fast and
# exercises the VGA text path. This one builds the real ISO and boots it
# through GRUB twice - once on legacy BIOS, once on UEFI firmware - because
# those are the two paths a real machine actually takes, and the UEFI one
# has no VGA text mode at all.
#
# Kept out of run_tests.sh on purpose: it costs ~40s and needs OVMF.
set -uo pipefail
cd "$(dirname "$0")"

OVMF_CODE=/usr/share/OVMF/OVMF_CODE_4M.fd
OVMF_VARS=/usr/share/OVMF/OVMF_VARS_4M.fd
fail=0

command -v qemu-system-x86_64 >/dev/null || { echo "skip: no qemu-system-x86_64"; exit 0; }

./mkiso.sh >/dev/null 2>&1 || { echo "FAIL: ISO did not build"; exit 1; }

check() {
    local label=$1 log=$2
    if ! grep -q "zlOS starting" "$log" 2>/dev/null; then
        echo "  FAIL  $label - kernel never started"; fail=1; return
    fi
    if ! grep -q "ready\." "$log" 2>/dev/null; then
        echo "  FAIL  $label - booted but never reached the prompt"; fail=1; return
    fi
    echo "  ok    $label - $(grep -oE '(framebuffer|VGA text) console, [0-9]+x[0-9]+' "$log" | head -1)"
}

# Boot and wait for the console line, rather than for a fixed number of seconds.
# Two full QEMU boots run in this script and both were on fixed timeouts, which
# makes the gate a function of host load instead of of the kernel.
boot_until() {           # $1 = log file, rest = qemu argv
    local log="$1"; shift
    local ceiling=180
    timeout "$ceiling" "$@" >/dev/null 2>&1 &
    local pid=$!
    # Wait for the LAST thing check() requires, which is the prompt - not the
    # console line. The console line is printed early in boot, so waiting on it
    # kills QEMU before "ready." is ever emitted and the gate then reports
    # "booted but never reached the prompt" on a perfectly healthy kernel.
    for _ in $(seq $((ceiling * 2))); do
        grep -q "ready\." "$log" 2>/dev/null && break
        kill -0 "$pid" 2>/dev/null || break
        sleep 0.5
    done
    sleep 1        # let the line after the prompt land before we kill it
    kill "$pid" 2>/dev/null; wait "$pid" 2>/dev/null
}

echo "== ISO: legacy BIOS boot =="
BLOG=$(mktemp)
boot_until "$BLOG" qemu-system-i386 -cdrom zlOS.iso -display none \
    -serial "file:$BLOG" -no-reboot
tr -d '\r' < "$BLOG" > "$BLOG.c" && mv "$BLOG.c" "$BLOG"
check "BIOS" "$BLOG"

echo "== ISO: UEFI boot =="
if [ ! -f "$OVMF_CODE" ]; then
    echo "  skip  no OVMF firmware (apt install ovmf)"
else
    VARS=$(mktemp); cp "$OVMF_VARS" "$VARS"
    ULOG=$(mktemp)
    boot_until "$ULOG" qemu-system-x86_64 \
        -drive if=pflash,format=raw,unit=0,readonly=on,file="$OVMF_CODE" \
        -drive if=pflash,format=raw,unit=1,file="$VARS" \
        -cdrom zlOS.iso -display none \
        -serial "file:$ULOG" -no-reboot
    tr -d '\r' < "$ULOG" > "$ULOG.c" && mv "$ULOG.c" "$ULOG"
    check "UEFI" "$ULOG"
    # Under UEFI there is no VGA text mode, so a framebuffer is the ONLY way
    # anything reaches the screen. If it fell back to VGA the user sees black.
    if grep -q "VGA text console" "$ULOG" 2>/dev/null; then
        echo "  FAIL  UEFI fell back to VGA text - the screen would be blank"; fail=1
    fi
    rm -f "$VARS" "$ULOG"
fi
rm -f "$BLOG"

[ "$fail" -eq 0 ] && echo "ISO gate green" || echo "ISO gate FAILED"
exit $fail
