#!/usr/bin/env bash
# preflight.sh - run locally what CI will run remotely.
#
#   tools/preflight.sh          fast gates only  (~1 min, no emulator)
#   tools/preflight.sh --boot   also the four boot paths (~4 min, QEMU)
#   tools/preflight.sh --all    everything, including the desktop render
#
# The point is to find out here rather than in a red PR, and to keep one list
# of what "green" means instead of two that drift.
#
# Every gate is run to completion even when an earlier one fails - a run that
# stops at the first failure hides the other three, and the whole reason this
# repo has a boot matrix is that partial information about which paths work is
# what let a dead 64-bit build sit behind three green gates.

set -uo pipefail
cd "$(dirname "$0")/.." || exit

MODE="${1:-}"
declare -a NAMES=() RESULTS=()
t0=$SECONDS

run() {   # run <name> <command...>
    local name="$1"; shift
    printf '\n\033[1m══ %s ══\033[0m\n' "$name"
    local s0=$SECONDS
    if "$@"; then
        NAMES+=("$name"); RESULTS+=("pass  $((SECONDS-s0))s")
    else
        NAMES+=("$name"); RESULTS+=("FAIL  $((SECONDS-s0))s")
    fi
}

# A gate that prints "skip:" and exits 0 is a green check for a test that never
# ran. CI fails on that; so does this.
run_noskip() {
    local name="$1"; shift
    printf '\n\033[1m══ %s ══\033[0m\n' "$name"
    local s0=$SECONDS log; log=$(mktemp)
    if "$@" 2>&1 | tee "$log"; then
        if grep -qiE '^[[:space:]]*skip:' "$log"; then
            echo "  -> reported skip: it did not actually run"
            NAMES+=("$name"); RESULTS+=("SKIP! $((SECONDS-s0))s")
        else
            NAMES+=("$name"); RESULTS+=("pass  $((SECONDS-s0))s")
        fi
    else
        NAMES+=("$name"); RESULTS+=("FAIL  $((SECONDS-s0))s")
    fi
    rm -f "$log"
}

[ -x ./interp ] || { echo "building the toolchain first..."; ./build.sh >/dev/null || exit 2; }

run "build"          ./build.sh
run "language"       ./run_tests.sh
run "engine parity"  tools/engine-parity.sh
run "formatter"      ./verify_fmt.sh
run "hazards"        tools/hazard-scan.sh

if [ "$MODE" = "--boot" ] || [ "$MODE" = "--all" ]; then
    run_noskip "boot: bios32" kernel/verify.sh
    run_noskip "boot: raw"    kernel/verify-raw.sh
    run_noskip "boot: iso"    kernel/verify-iso.sh
    run_noskip "boot: efi"    kernel/verify-efi.sh
fi

if [ "$MODE" = "--all" ]; then
    run "desktop render" bash .github/scripts/render-desktop.sh preflight
fi

printf '\n\033[1m════ preflight (%ds total) ════\033[0m\n' "$((SECONDS-t0))"
bad=0
for i in "${!NAMES[@]}"; do
    printf '  %-8s %s\n' "${RESULTS[$i]%% *}" "${NAMES[$i]}"
    case "${RESULTS[$i]}" in FAIL*|SKIP!*) bad=1 ;; esac
done

if [ "$MODE" != "--boot" ] && [ "$MODE" != "--all" ]; then
    echo
    echo "  (fast gates only - add --boot for the four boot paths, --all for everything)"
fi

echo
[ "$bad" -ne 0 ] && { echo "preflight: NOT clean"; exit 1; }
echo "preflight: clean"
