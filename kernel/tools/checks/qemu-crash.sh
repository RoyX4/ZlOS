# Did the EMULATOR crash, as opposed to the kernel failing?
#
# Source this and call `qemu_crashed "$status"` with whatever `wait` returned.
#
# WHY IT EXISTS. On 2026-08-27 qemu-system-x86_64 segfaulted under load - four
# cores, load average 4.56 - and verify-efi.sh reported:
#
#     FAIL  the kernel never started
#
# True, and pointing at entirely the wrong thing: the kernel never started
# because the emulator died before it could. It cost a full push cycle to work
# out, and none of the five boot gates could have told the difference. Every one
# of them killed QEMU, waited, threw the status away, and judged on log contents
# alone. A check that cannot tell its own tooling failing from the thing it
# tests failing is the same defect as a test that restates the constant it is
# testing - see docs/evidence/one-fact-many-copies-2026-08-27.md.
#
# IT REPORTS; IT DOES NOT DECIDE. The first version of this returned a verdict,
# and that was wrong in a way the very first real crash exposed: QEMU segfaulted
# during TEARDOWN, after the kernel had already written every marker the gate
# wanted, and the gate went red on a boot that had demonstrably succeeded. A
# crash mid-boot truncates the log and the marker checks fail on their own -
# and then this line is the EXPLANATION for that failure, not a second one.
# So callers print this for context and let the log decide.
#
# THE DISTINCTION IS ONE LINE. Our own `kill` sends SIGTERM, which bash reports
# as 143, and a clean exit is 0. A segfault is 139, an abort 134, a bus error
# 135, and nothing these gates do can produce those.
#
# ONE FILE, FIVE CALLERS. The first version of this fix pasted the same `case`
# into all five gates, which is precisely the mistake this whole day was spent
# undoing. tools/checks/qemu-crash-selftest.sh tests this copy, and it is the
# only copy.

qemu_crashed() {   # $1 = the status `wait` returned. 0 = yes it crashed.
    case "$1" in
        139|134|135)
            echo "  CRASH QEMU ITSELF crashed (status $1) - the EMULATOR, not the kernel"
            echo "        load average: $(cut -d' ' -f1-3 /proc/loadavg)"
            echo "        this box has 4 cores. Re-run on a quiet one before"
            echo "        believing there is a kernel regression."
            return 0 ;;
    esac
    return 1
}
