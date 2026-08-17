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
BL=/sys/class/backlight/intel_backlight
BL_SAVED=""

[ "$(id -u)" -eq 0 ] || { echo "run me with sudo"; exit 1; }

# Find the display manager rather than assuming lightdm - Kali ships lightdm,
# but a gdm3 box would silently not be stopped and would keep hold of the GPU.
for c in lightdm gdm3 gdm sddm; do
    systemctl is-active --quiet "$c" && { DM="$c"; break; }
done

restore() {
    local rc=$?
    # Put the user's brightness back.
    #
    # The driver restores the PWM duty register, but that is not where the
    # brightness the USER sees comes from once i915 is back: rebinding creates
    # a fresh /sys/class/backlight device whose level is read from whatever the
    # hardware happens to hold, and i915 powers the panel down on unbind with
    # the duty at zero. So every run ended with the panel lit at nothing and
    # the brightness keys as the only way out.
    #
    # Fixing it here rather than in the driver is deliberate. This is a Linux
    # level value, it is captured before anything is unbound - so it is the
    # brightness the user actually had, not whatever i915 left behind on its
    # way out - and it is written after the rebind, when the device exists
    # again. The driver has no business knowing about sysfs.
    echo | tee -a "$LOG"
    echo "== restoring ==" | tee -a "$LOG"
    echo "$DEV" > /sys/bus/pci/drivers/i915/bind 2>/dev/null \
        && echo "  i915 rebound" | tee -a "$LOG" \
        || echo "  i915 bind returned nonzero (may already be bound)" | tee -a "$LOG"
    if [ -n "$DM" ]; then
        systemctl start "$DM" && echo "  $DM restarted" | tee -a "$LOG"
    fi
    if [ -n "$BL_SAVED" ]; then
        # the backlight device reappears with i915, which takes a moment
        for _ in 1 2 3 4 5 6 7 8 9 10; do
            [ -w "$BL/brightness" ] && break
            sleep 0.5
        done
        if [ -w "$BL/brightness" ]; then
            echo "$BL_SAVED" > "$BL/brightness" 2>/dev/null \
                && echo "  brightness restored to $BL_SAVED" | tee -a "$LOG" \
                || echo "  brightness restore FAILED - set it with the keys" | tee -a "$LOG"
        else
            echo "  backlight device did not reappear" | tee -a "$LOG"
        fi
    fi

    # If the console is still dark, a VT switch forces fbcon to repaint.
    chvt 1 2>/dev/null; chvt 7 2>/dev/null
    echo "  log: $(pwd)/$LOG" | tee -a "$LOG"
    exit $rc
}
trap restore EXIT INT TERM

# Capture brightness BEFORE anything is touched. After the unbind this reads 0.
[ -r "$BL/brightness" ] && BL_SAVED=$(cat "$BL/brightness" 2>/dev/null)
echo "brightness before: ${BL_SAVED:-unknown}" | tee -a "$LOG"

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
