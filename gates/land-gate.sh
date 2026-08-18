#!/bin/bash
# Build gate for a zlOS landing. Runs sequentially, never in parallel, and
# NEVER masks an exit code — the previous version piped every build through
# `tail`, so every step reported the exit status of `tail` (always 0) and a
# tree that did not link gated green.
#
# usage: bash gates/land-gate.sh [worktree]   (default: this worktree)
# Run it backgrounded:  nohup bash gates/land-gate.sh > ~/gate.log 2>&1 &

set -u
WT="${1:-$(git rev-parse --show-toplevel)}"
cd "$WT" || exit 2

FAIL=0
run() {                       # run <label> <dir> <cmd...>
  local label="$1" dir="$2"; shift 2
  echo
  echo "=== $label ==="
  local out rc
  out=$( cd "$dir" && "$@" 2>&1 ); rc=$?
  echo "$out" | tail -25
  if [ $rc -ne 0 ]; then
    echo ">>> FAIL ($label) exit=$rc"
    FAIL=$((FAIL+1))
  else
    echo ">>> ok ($label)"
  fi
  return 0
}

guard() {
  local la mem
  la=$(cut -d' ' -f1 /proc/loadavg)
  mem=$(awk '/MemAvailable/{print int($2/1024)}' /proc/meminfo)
  # the documented prior kill on this box was an OOM, not CPU starvation,
  # so memory is guarded as well as load
  if awk "BEGIN{exit !($la > 4.0)}"; then echo "load $la > 4.0 — waiting"; return 1; fi
  if [ "$mem" -lt 3000 ]; then echo "available memory ${mem}MB < 3000 — waiting"; return 1; fi
  if pgrep '^qemu-system' >/dev/null; then echo "a qemu is already running — waiting"; return 1; fi
  return 0
}

echo "gate: $WT @ $(git rev-parse --short HEAD)"
echo "load: $(cut -d' ' -f1-3 /proc/loadavg)   avail: $(awk '/MemAvailable/{print int($2/1024)}' /proc/meminfo)MB"

# --- toolchain and compile-only steps (cheap, no QEMU)
run "zl toolchain"     "$WT"               ./build.sh
run "kernel 32-bit"    "$WT/kernel"        ./build.sh
run "kernel 64-bit"    "$WT/kernel"        ./build64.sh
run "kernel EFI"       "$WT/kernel"        ./buildefi.sh
[ -x "$WT/kernel/verify-sources.sh" ] && run "SOURCES coverage" "$WT/kernel" ./verify-sources.sh
run "hosttest build"   "$WT/kernel/hosttest" ./build.sh

# --- the reverse SOURCES check: a .c present but not listed is silently not compiled
if [ -f "$WT/kernel/SOURCES" ]; then
  echo; echo "=== reverse SOURCES sweep ==="
  miss=0
  for f in "$WT"/kernel/*.c; do
    b=$(basename "$f")
    case "$b" in _gen*.c|gdt.c|gdt64.c|efi.c|out.c) continue;; esac
    grep -qx "$b" "$WT/kernel/SOURCES" || { echo "NOT IN SOURCES: $b"; miss=$((miss+1)); }
  done
  [ $miss -gt 0 ] && FAIL=$((FAIL+1)) && echo ">>> FAIL (reverse SOURCES: $miss)" || echo ">>> ok (reverse SOURCES)"
fi

# --- boot gates: QEMU under TCG, one at a time, guarded
for g in mkiso.sh verify.sh verify-iso.sh verify-efi.sh verify-raw.sh verify-disk.sh verify-clock.sh; do
  [ -x "$WT/kernel/$g" ] || continue
  until guard; do sleep 30; done
  run "boot: $g" "$WT/kernel" "./$g"
done

echo
echo "================================"
if [ $FAIL -eq 0 ]; then echo "GATE GREEN"; else echo "GATE RED — $FAIL failing step(s)"; fi
echo "================================"
exit $FAIL
