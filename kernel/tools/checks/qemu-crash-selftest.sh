#!/usr/bin/env bash
# Prove qemu_crashed() fires on a crash and stays quiet on a normal exit.
#
# It is a four-line `case`, which is exactly the kind of thing nobody tests and
# which is therefore free to be wrong forever. The status values are the whole
# content of the check: get one wrong and the gate either cries wolf on every
# clean run or goes back to blaming the kernel for the emulator's segfault.
#
# 143 is the important negative. That is SIGTERM, which is what these gates send
# themselves once they have seen the marker they were waiting for - the NORMAL
# path. A version of this that treated "died by signal" as a crash would fail
# every single healthy boot.
set -u
cd "$(dirname "$0")/../.." || exit 1
. tools/checks/qemu-crash.sh

fail=0
check() {  # $1 status, $2 = "crash" | "fine", $3 why
    local out rc
    out=$(qemu_crashed "$1"); rc=$?
    local want=1; [ "$2" = "crash" ] && want=0
    if [ "$rc" -eq "$want" ]; then
        printf "  ok   %-3s -> %-5s  %s\n" "$1" "$2" "$3"
    else
        printf "  FAIL %-3s -> got rc=%s, wanted %s  (%s)\n" "$1" "$rc" "$want" "$3"
        fail=$((fail + 1))
    fi
    # a crash must SAY something; a clean exit must say nothing at all
    if [ "$2" = "crash" ] && [ -z "$out" ]; then
        echo "  FAIL $1 detected but printed nothing"; fail=$((fail + 1))
    fi
    if [ "$2" = "fine" ] && [ -n "$out" ]; then
        echo "  FAIL $1 is a normal exit but printed: $out"; fail=$((fail + 1))
    fi
}

echo "qemu-crash-selftest"
check 139 crash "SIGSEGV - the one actually observed, 2026-08-27"
check 134 crash "SIGABRT"
check 135 crash "SIGBUS"
check 143 fine  "SIGTERM - OUR OWN kill, the normal path"
check 0   fine  "clean exit"
check 1   fine  "qemu exited non-zero but did not crash"
check 124 fine  "timeout(1) gave up - a hang, not a crash"

echo
if [ "$fail" -ne 0 ]; then
    echo "qemu-crash-selftest: FAILED ($fail)"; exit 1
fi
echo "qemu-crash-selftest: PASS - fires on a crash, silent on every normal exit"
