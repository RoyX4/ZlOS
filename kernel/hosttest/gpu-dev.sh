#!/usr/bin/env bash
# gpu-dev.sh - a development loop for the Intel driver that never reboots.
#
# The problem this solves: intel.c could only be executed by writing a USB
# stick, rebooting, and reading a screen. Minutes per iteration, no printf, no
# diff. Developing a DPLL sequence that way is not realistic.
#
# The GPU's registers are memory. Linux maps the PCI BAR into a process, so the
# same driver source runs here at native speed with the whole toolchain
# available.
#
#   ./gpu-dev.sh probe            read everything (safe, i915 keeps running)
#   ./gpu-dev.sh dump [file]      snapshot every display register
#   ./gpu-dev.sh diff a b         what changed between two snapshots
#   ./gpu-dev.sh watch            snapshot, wait for you to change something,
#                                 snapshot again, and print the difference
#   ./gpu-dev.sh detach           unbind i915  -- BLANKS THE SCREEN
#   ./gpu-dev.sh attach           give the GPU back to i915
#
# `detach` is what makes WRITES safe: with i915 gone nothing else is
# programming the display engine, so our driver is the only thing touching it.
# It also blanks the display, so run it from SSH or expect a dark screen until
# `attach`. This is a test machine; that is the trade being made deliberately.
set -euo pipefail
cd "$(dirname "$0")"

DEV=0000:00:02.0
SYS=/sys/bus/pci/devices/$DEV

need_root() { [ "$(id -u)" -eq 0 ] || { echo "run me with sudo"; exit 1; }; }

case "${1:-probe}" in

probe)
    ./build.sh >/dev/null
    exec sudo ./intel_probe "${@:2}"
    ;;

dump)
    ./build.sh >/dev/null
    out="${2:-regs-$(date +%H%M%S).txt}"
    sudo ./intel_probe --dump > "$out"
    echo "wrote $out ($(wc -l < "$out") registers)"
    ;;

diff)
    [ $# -ge 3 ] || { echo "usage: $0 diff FILE_A FILE_B"; exit 1; }
    # Only the lines that actually changed, which is the whole point: a
    # modeset touches a few dozen registers out of hundreds.
    diff <(sort "$2") <(sort "$3") | grep -E '^[<>]' | sort -k2 || true
    echo
    echo "( < was $2 ,  > is $3 )"
    ;;

watch)
    ./build.sh >/dev/null
    a=$(mktemp); b=$(mktemp)
    sudo ./intel_probe --dump > "$a"
    echo "snapshot taken. change something (plug a monitor, change resolution,"
    echo "dim the backlight...) then press enter."
    read -r _
    sudo ./intel_probe --dump > "$b"
    echo
    echo "registers that changed:"
    diff <(sort "$a") <(sort "$b") | grep -E '^[<>]' | sort -k2 || echo "  (none)"
    rm -f "$a" "$b"
    ;;

detach)
    need_root
    echo "unbinding i915 - the screen will go dark until 'attach'"
    if [ -e "$SYS/driver" ]; then
        echo "$DEV" > "$SYS/driver/unbind"
        echo "done. the GPU is ours; writes are now safe."
    else
        echo "already unbound."
    fi
    ;;

attach)
    need_root
    echo "$DEV" > /sys/bus/pci/drivers/i915/bind 2>/dev/null || true
    echo "i915 rebound. if the console stays dark, switch VT (chvt 1) or restart"
    echo "the display manager."
    ;;

*)
    sed -n '2,26p' "$0" | sed 's/^# \{0,1\}//'
    ;;
esac
