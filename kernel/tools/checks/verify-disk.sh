#!/usr/bin/env bash
# verify-disk.sh - the only gate in this tree that POWER-CYCLES the machine.
#
# Every other gate boots once. That is enough to prove a driver talks to a
# device, and it proves exactly nothing about persistence: a filesystem backed
# entirely by RAM passes a single-boot test perfectly. So this one boots twice
# against the SAME disk image and checks that a number written by the first
# boot is read back, incremented, by the second.
#
#   boot 1 on a fresh image  ->  formats, creates "boots", prints BOOTCOUNT=1
#   boot 2 on the same image ->  mounts, finds "boots",   prints BOOTCOUNT=2
#
# BOOTCOUNT=2 is the assertion. It cannot come from a cache, a buffer, a lucky
# read or a hopeful driver - the process that wrote the 1 no longer exists, and
# the only thing shared between the two boots is the file on the host's disk.
#
# A third boot is run to catch the failure mode where the counter is really a
# "have I formatted" flag: a filesystem that reformats on every mount would
# print 1, 1, 1, and one that never persists past the first write would print
# 1, 2, 2.
#
# NEVER a fixed wall-clock wait. Under TCG the boot time depends on host load,
# and a gate that fails because the machine is busy costs a bisect every time
# it happens (CLAUDE.md, and verify-raw.sh learned this the hard way). This
# polls for its marker with a generous ceiling.
set -uo pipefail
cd "$(dirname "$0")/../.." || exit
. tools/checks/qemu-crash.sh

IMG=${ZLOS_DISK_IMG:-/tmp/zlos-diskgate.img}
CEILING=${CEILING:-240}
OUT=$(mktemp); trap 'rm -f "$OUT" "$OUT".*' EXIT

# ONE QEMU across the whole box, ever - four sessions share four cores.
# pgrep WITHOUT -f, so it matches the executable's name and not every command
# line containing the string "qemu-system" - including this script's own
# invocation, which made it refuse to run against itself.
if pgrep '^qemu-system' >/dev/null 2>&1; then
    echo "SKIP: another qemu-system is already running on this box."
    echo "      §1.2 allows exactly one. Re-run when it is done."
    exit 2
fi
LOAD=$(cut -d' ' -f1 /proc/loadavg)
if awk "BEGIN{exit !($LOAD > 4)}"; then
    echo "SKIP: load average is $LOAD. Above ~4 a TCG boot slows enough to make"
    echo "      this gate report a regression that does not exist."
    exit 2
fi

./build.sh >/dev/null 2>&1 || { echo "FAIL: kernel did not build"; exit 1; }

# The arena lives at 208 MiB, so 128 MiB (qemu's default) puts every NVMe
# queue outside guest RAM. That is this project's recurring bug class and it
# reads as a protocol failure every time, so the memory size is not optional.
boot_once () {
    local tag=$1 keys=$2 log=$3
    printf '%s' "$keys" | timeout "$CEILING" qemu-system-i386 \
        -kernel kernel.elf -m 1G \
        -drive "file=$IMG,if=none,id=nvm,format=raw" \
        -device nvme,serial=zlosgate,drive=nvm \
        -serial stdio -display none -no-reboot >"$log" 2>/dev/null &
    local qpid=$!
    for _ in $(seq $((CEILING * 2))); do
        grep -q "halting" "$log" 2>/dev/null && break
        kill -0 "$qpid" 2>/dev/null || break
        sleep 0.5
    done
    kill "$qpid" 2>/dev/null; wait "$qpid" 2>/dev/null; local qstatus=$?
    qemu_crashed "$qstatus" || true
    grep -q "halting" "$log" 2>/dev/null || {
        echo "FAIL: boot $tag never halted - it hung"; return 1; }
    tr -d '\r' < "$log" > "$log.c" && mv "$log.c" "$log"
    return 0
}

count_of () { sed -n 's/.*BOOTCOUNT=\([0-9]*\).*/\1/p' "$1" | tail -1; }

# a FRESH image every run, or the count is whatever the last run left
rm -f "$IMG"
qemu-img create -f raw "$IMG" 64M >/dev/null 2>&1 || {
    echo "FAIL: could not create $IMG"; exit 1; }

# '.' runs the disk command, ',' lists by name, 'q' halts. The leading '.'
# throwaway is verify.sh's trick: QEMU can hand the guest the first serial
# byte before it starts executing, so that byte is lost no matter what.
for n in 1 2 3; do
    boot_once "$n" '..,q' "$OUT.$n" || exit 1
done

C1=$(count_of "$OUT.1"); C2=$(count_of "$OUT.2"); C3=$(count_of "$OUT.3")

echo "  boot 1: BOOTCOUNT=${C1:-<none>}"
echo "  boot 2: BOOTCOUNT=${C2:-<none>}"
echo "  boot 3: BOOTCOUNT=${C3:-<none>}"

[ -n "$C1" ] && [ -n "$C2" ] && [ -n "$C3" ] || {
    echo "FAIL: the disk command did not report a count on every boot"
    echo "--- last boot transcript ---"; tail -25 "$OUT.3"; exit 1; }

if [ "$C1" = 1 ] && [ "$C2" = 2 ] && [ "$C3" = 3 ]; then
    echo "ok    a named file survived two power cycles: 1 -> 2 -> 3"
    # and the listing must still show it by name on the last boot
    grep -q "boots" "$OUT.3" || {
        echo "FAIL: 'boots' was not in the on-disk listing"; exit 1; }
    echo "ok    and 'boots' is still there by NAME after the reboots"
    exit 0
fi

echo "FAIL: expected 1 -> 2 -> 3"
[ "$C1$C2$C3" = "111" ] && echo "      1,1,1 means the volume is REFORMATTED on every mount"
[ "$C1$C2$C3" = "122" ] && echo "      1,2,2 means the second write never reached the platter"
echo "--- boot 2 transcript ---"; tail -25 "$OUT.2"
exit 1
