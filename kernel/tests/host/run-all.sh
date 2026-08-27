#!/usr/bin/env bash
# Build every host test FROM SCRATCH and run the ones that self-check.
#
# WHY THIS EXISTS. On 2026-08-27 a change to uikit.c added calls to
# fb_text_rich / fb_text_rich_w and settingstest's stub set did not follow, so
# it stopped LINKING - which means it stopped RUNNING, silently. Two things
# then conspired to hide it:
#
#   1. build.sh has `set -e`, so it ABORTED at that link failure. Every test
#      declared after it was never rebuilt - uitest and runtest among them.
#   2. The old binaries were still sitting on disk from the previous run, so
#      running them by name after a failed build reported "all passed" from
#      code that predated the change under test.
#
# Green from a stale binary is the worst result a gate can produce, because it
# is indistinguishable from a real pass. So: DELETE FIRST, then build, then
# check build.sh's exit status, then report NOT BUILT for anything missing
# rather than skipping it quietly.
#
# THE TEST LIST IS READ OUT OF build.sh, not kept here. A second hand-maintained
# list is a second thing to forget - kernel/.gitignore's hosttest allowlist is
# already the example of how that ends. Anything build.sh announces with
# `built ./name (run: ./name)` is picked up automatically the day it is added.
set -u
cd "$(dirname "$0")"

# Announced with `sudo` - real hardware, not a self-check. Never run here.
# Everything else is excluded BY NAME with its reason, so the list stays honest.
declare -A SKIP=(
  [fbbench]="benchmark - prints timings, has no pass/fail"
  [wmbench]="benchmark - prints timings, has no pass/fail"
  [wmshot]="writes a .ppm render, needs an output path"
  [browsershot]="writes a .ppm render, needs an output path"
  [parsestat]="measuring instrument - needs a page and a sheet"
  [fuzz]="needs an iteration count and a seed"
  [intel_probe]="real GPU - needs root and the live device"
  [modeset_test]="real GPU - needs root and the live device"
  # These three exit NON-ZERO with "needs root" rather than reporting a skip, so
  # without naming them here they read as four real failures on any unprivileged
  # run. They map BAR0; there is nothing for them to do without it.
  [gpu_aperture]="real GPU - needs root, maps BAR0"
  [gpu_planes]="real GPU - needs root, maps BAR0"
  [gpu_ring]="real GPU - needs root, maps BAR0"
  [gpu_blt]="real GPU - needs root, maps BAR0"
  [gpu_fillrate]="real GPU - needs root, maps BAR0"
)

echo "== deleting every binary first =="
# Strip the trailing quote: two lines announce as `echo "built ./wmtest_feel"`
# with no `(run: ...)` half, so a bare \S+ captures the closing quote and the
# name never matches a file. That produced two false NOT BUILTs on the first
# run of this script - a guard reporting a healthy test as missing, which is
# the same class of lie as reporting a missing one as healthy.
mapfile -t TESTS < <(grep -oP '(?<=built \./)[A-Za-z0-9_.-]+' build.sh | sort -u)
for t in "${TESTS[@]}"; do rm -f "./$t"; done
echo "   ${#TESTS[@]} binaries removed"

echo "== build.sh =="
if ./build.sh > /tmp/host-build.log 2>&1; then
    echo "   build.sh exit 0"
else
    echo "   BUILD FAILED (exit $?) - it stops at the first error, so everything"
    echo "   declared after this point was never built:"
    tail -8 /tmp/host-build.log | sed 's/^/     /'
fi

echo "== running =="
pass=0; fail=0; missing=0; skipped=0
for t in "${TESTS[@]}"; do
    if [[ -v SKIP[$t] ]]; then
        printf "  %-22s skipped   (%s)\n" "$t" "${SKIP[$t]}"
        skipped=$((skipped + 1)); continue
    fi
    if [ ! -x "./$t" ]; then
        printf "  %-22s NOT BUILT\n" "$t"
        missing=$((missing + 1)); continue
    fi
    out=$(timeout 120 "./$t" 2>&1); rc=$?
    last=$(printf '%s' "$out" | tail -1)
    if [ $rc -eq 0 ]; then
        printf "  %-22s ok        %s\n" "$t" "$last"
        pass=$((pass + 1))
    else
        printf "  %-22s FAILED    %s\n" "$t" "$last"
        fail=$((fail + 1))
    fi
done

echo
echo "  $pass passed, $fail failed, $missing NOT BUILT, $skipped skipped"
# A test that did not build is a FAILURE, not an absence. That is the entire
# point of this script.
[ $fail -eq 0 ] && [ $missing -eq 0 ]
