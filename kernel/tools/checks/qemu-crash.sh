# Shared QEMU process-status reporter for boot gates. It explains an emulator
# failure but does not decide the guest verdict: QEMU has crashed during
# teardown after every required marker was already written. Mid-boot crashes
# truncate the log, so the normal marker checks still fail them.

qemu_status_is_crash() {
    case "${1:-}" in
        # 128+signal, excluding SIGTERM (143), which every healthy gate sends
        # after observing its final marker. Timeouts report 124 separately.
        12[89]|13[0-9]|14[0-2])
            return 0
            ;;
    esac
    return 1
}

qemu_crashed() {
    if qemu_status_is_crash "$1"; then
        echo "  CRASH QEMU itself died from signal (status $1); the emulator, not the kernel"
        echo "        load average: $(cut -d' ' -f1-3 /proc/loadavg)"
        return 0
    fi
    return 1
}
