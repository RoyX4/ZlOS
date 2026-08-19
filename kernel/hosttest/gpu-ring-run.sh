#!/usr/bin/env bash
# gpu-ring-run.sh - give the blitter to gpu_ring, and ALWAYS give the display back.
#
#   sudo ./gpu-ring-run.sh --survey    read-only; proves recovery works. DO THIS FIRST.
#   sudo ./gpu-ring-run.sh --dry       unbinds and rebinds, writes NOTHING.
#                                      the recovery rehearsal - do this second.
#   sudo ./gpu-ring-run.sh --ring      the real experiment: takes the ring.
#
# WHY THIS IS NOT modeset-run.sh
# ------------------------------
# Same hazard, one extra problem. modeset-run.sh says "run it from a TTY, not
# from a terminal inside the desktop - stopping the display manager kills that
# terminal along with the session." That is correct, and it means the recovery
# trap dies with the terminal that was going to run it. A laptop with no display
# and no way back is a reboot.
#
# So this script DETACHES ITSELF FIRST. It re-execs under setsid with its output
# on a log file, and only then stops the display manager. The recovery trap then
# belongs to a process that has no controlling terminal and no session to lose,
# so it survives the thing it is recovering from.
#
# That matters here specifically because the machine runs a Wayland session with
# a lot of agent sessions inside it (`loginctl` reports Type=wayland), and every
# one of them dies with the display manager. The trap must not be one of them.
#
# WHAT IS AND IS NOT TOUCHED
# --------------------------
# gpu_ring writes the blitter's ring registers and GGTT entries. It writes NO
# display register: no pipe, no DPLL, and no panel power. intel.c's hazard list
# (T12's 500 ms power-cycle delay, AUX into an unpowered panel) is about
# hardware it never addresses. The screen is dark during a run because i915 is
# unbound, not because anything here programmed the display.
set -uo pipefail
cd "$(dirname "$0")" || exit 1

DEV=0000:00:02.0
SYS=/sys/bus/pci/devices/$DEV
MODE="${1:---survey}"
BL=/sys/class/backlight/intel_backlight
TIMEOUT=120
LOG="$PWD/gpu-ring-$(date +%Y%m%d-%H%M%S).log"

[ "$(id -u)" -eq 0 ] || { echo "run me with sudo"; exit 1; }

# ---- re-exec detached ------------------------------------------------------
# Everything after this point must outlive the graphical session.
if [ "${GPU_RING_DETACHED:-0}" != "1" ]; then
    echo "gpu-ring-run: detaching so recovery survives the display manager stopping."
    echo "  log:    $LOG"
    echo "  follow: tail -f $LOG"
    echo
    GPU_RING_DETACHED=1 setsid nohup "$0" "$MODE" >"$LOG" 2>&1 < /dev/null &
    disown 2>/dev/null || true
    echo "started (pid $!). It will rebind i915 and restart the desktop when done."
    exit 0
fi

say() { echo "$@"; }

DM=""
for c in gdm3 gdm lightdm sddm; do
    systemctl is-active --quiet "$c" && { DM="$c"; break; }
done

BL_SAVED=""
[ -r "$BL/brightness" ] && BL_SAVED=$(cat "$BL/brightness" 2>/dev/null)

restore() {
    local rc=$?
    say
    say "== restoring =="
    if [ ! -e "$SYS/driver" ]; then
        echo "$DEV" > /sys/bus/pci/drivers/i915/bind 2>/dev/null \
            && say "  i915 rebound" || say "  i915 bind returned nonzero"
    else
        say "  i915 already bound"
    fi
    sleep 1
    # Brightness comes back as whatever the hardware held, which is 0 after
    # i915 powered the panel down on unbind. Same fix, same reason, as
    # modeset-run.sh: captured before, written after, at the Linux level.
    if [ -n "$BL_SAVED" ] && [ -w "$BL/brightness" ]; then
        echo "$BL_SAVED" > "$BL/brightness" 2>/dev/null && say "  brightness restored to $BL_SAVED"
    fi
    [ -n "$DM" ] && { systemctl start "$DM" && say "  $DM restarted"; }
    say "== done (exit $rc) =="
}
trap restore EXIT INT TERM

say "gpu-ring-run $(date -Is)   mode: $MODE"
say "display manager: ${DM:-none active}"
say "brightness before: ${BL_SAVED:-unknown}"
say

# Build ONLY gpu_ring, not ./build.sh's twenty-odd harnesses.
#
# Two reasons, both measured. It is slow - the full script rebuilds every
# harness in this directory, and on a loaded box that is minutes with i915
# already unbound and the screen dark. And it RACES: other sessions work in
# this same checkout and run ./build.sh too, so a full build here can collide
# with theirs over the same object files. Neither is acceptable in the window
# where the display is gone.
gcc -O2 -g -Wall -Wextra -o gpu_ring gpu_ring.c 2>&1 | head -5
[ -x ./gpu_ring ] || { say "gpu_ring did not build"; exit 1; }

# --survey is read-only and works with i915 loaded, so run it BEFORE taking
# anything - it is the witness that the plumbing is good, and its real job is
# to prove this script's recovery works while the cost of being wrong is zero.
say "== survey, i915 still bound =="
timeout "$TIMEOUT" ./gpu_ring --survey
say

# --survey NEVER unbinds. The first version of this script printed "stopping
# here without unbinding" and then fell straight through into the unbind below,
# because the message was not followed by an exit - so the safe mode was not
# safe. Caught before anyone ran it; kept as a comment because a mode that
# claims to be read-only and is not is exactly the failure this whole tree keeps
# writing docs about.
if [ "$MODE" = "--survey" ]; then
    say "== survey mode: i915 was never unbound, nothing was written =="
    say "   Use --dry next: it unbinds and rebinds WITHOUT writing a ring"
    say "   register, which is how you prove the recovery works before it has"
    say "   to. Then --ring."
    exit 0
fi

say "== taking the GPU =="
[ -n "$DM" ] && { systemctl stop "$DM"; say "  $DM stopped"; sleep 2; }
if [ -e "$SYS/driver" ]; then
    echo "$DEV" > "$SYS/driver/unbind" && say "  i915 unbound - the screen is dark now"
else
    say "  already unbound"
fi
sleep 1

say
if [ "$MODE" = "--dry" ]; then
    # THE RECOVERY REHEARSAL. i915 is unbound and the screen is dark, and we
    # deliberately write nothing - the only thing under test is whether the
    # trap below brings the display back. If this does not restore the desktop,
    # nothing else in this file matters and --ring must not be run.
    say "== dry: i915 is unbound and NOTHING will be written =="
    say "   The only thing being tested is the recovery below."
    timeout 20 ./gpu_ring --survey
    rc=$?
else
    say "== running: gpu_ring $MODE =="
    timeout "$TIMEOUT" ./gpu_ring "$MODE"
    rc=$?
fi
[ $rc -eq 124 ] && say "  TIMED OUT after ${TIMEOUT}s - recovering"
say "  exit code: $rc"

# restore() runs from the trap, deliberately not called here, so a crash on the
# line above recovers exactly the same way a clean finish does.
exit $rc
