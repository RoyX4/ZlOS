#!/usr/bin/env bash
# modeset-run.sh - give the display to zlOS, run one test, always give it back.
#
# Unbinding i915 blanks everything: the desktop AND the framebuffer console. So
# there is nobody to type "attach" if the test hangs, and a laptop with no
# display and no way to recover is a reboot. This script exists so that never
# happens: recovery runs from an EXIT trap, which fires on success, on failure,
# on a crash, and on Ctrl-C alike.
#
#   sudo ./modeset-run.sh --survey     read-only, but with i915 gone
#   sudo ./modeset-run.sh --dry        full driver survey + the dry sequence
#   sudo ./modeset-run.sh --modeset    the real thing - ARMS AND WRITES
#
# RUN --survey FIRST, and read the log. It is read-only, and its real job is to
# prove the recovery below actually works before anything is ever written. If
# the screen does not come back from a read-only run, nothing else matters.
#
# Run it from a TTY (Ctrl+Alt+F3), not from a terminal inside the desktop -
# stopping the display manager kills that terminal along with the session.
set -uo pipefail
cd "$(dirname "$0")" || exit 1

DEV=0000:00:02.0
SYS=/sys/bus/pci/devices/$DEV
ARGS=("${@:---survey}")
LOG="modeset-$(date +%Y%m%d-%H%M%S).log"
DM=""
TIMEOUT=120          # nothing here should take two minutes; if it does, recover

[ "$(id -u)" -eq 0 ] || { echo "run me with sudo"; exit 1; }

# Find the display manager rather than assuming lightdm - Kali ships lightdm,
# but a gdm3 box would silently not be stopped and would keep hold of the GPU.
for c in lightdm gdm3 gdm sddm; do
    systemctl is-active --quiet "$c" && { DM="$c"; break; }
done

restore() {
    local rc=$?
    echo | tee -a "$LOG"
    echo "== restoring ==" | tee -a "$LOG"
    echo "$DEV" > /sys/bus/pci/drivers/i915/bind 2>/dev/null \
        && echo "  i915 rebound" | tee -a "$LOG" \
        || echo "  i915 bind returned nonzero (may already be bound)" | tee -a "$LOG"
    if [ -n "$DM" ]; then
        systemctl start "$DM" && echo "  $DM restarted" | tee -a "$LOG"
    fi
    # If the console is still dark, a VT switch forces fbcon to repaint.
    chvt 1 2>/dev/null; chvt 7 2>/dev/null
    echo "  log: $(pwd)/$LOG" | tee -a "$LOG"
    exit $rc
}
trap restore EXIT INT TERM

{
    echo "zlOS modeset run - $(date -Is)"
    echo "args: ${ARGS[*]}"
    echo "display manager: ${DM:-none active}"
    echo
    echo "== before (i915 still holding the GPU) =="
} | tee -a "$LOG"

./build.sh >/dev/null 2>&1 || { echo "build failed" | tee -a "$LOG"; exit 1; }
./modeset_test --survey >> "$LOG" 2>&1 || true

{
    echo
    echo "== taking the GPU =="
} | tee -a "$LOG"

[ -n "$DM" ] && { systemctl stop "$DM"; echo "  $DM stopped" | tee -a "$LOG"; sleep 2; }

if [ -e "$SYS/driver" ]; then
    echo "$DEV" > "$SYS/driver/unbind" && echo "  i915 unbound - screen is dark now" | tee -a "$LOG"
else
    echo "  already unbound" | tee -a "$LOG"
fi
sleep 1

{
    echo
    echo "== running: modeset_test ${ARGS[*]} =="
} | tee -a "$LOG"

# --survey stays on modeset_test, which reads raw offsets and is deliberately
# NOT linked against the driver - that isolation is what makes it a witness.
# --modeset is the driver executing for real, so it goes through intel_probe,
# which is the binary intel.c is compiled into.
case "${ARGS[0]}" in
    --modeset) RUNNER=(./intel_probe --modeset) ;;
    --dry)     RUNNER=(./intel_probe) ;;
    *)         RUNNER=(./modeset_test "${ARGS[@]}") ;;
esac
echo "  runner: ${RUNNER[*]}" | tee -a "$LOG"
timeout "$TIMEOUT" "${RUNNER[@]}" >> "$LOG" 2>&1
rc=$?
[ $rc -eq 124 ] && echo "  TIMED OUT after ${TIMEOUT}s - recovering" | tee -a "$LOG"
echo "  exit code: $rc" | tee -a "$LOG"

# restore() runs from the trap - deliberately not called here, so that a crash
# above takes exactly the same path as a clean finish.
exit $rc
