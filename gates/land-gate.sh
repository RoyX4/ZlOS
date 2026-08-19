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

# --- RUN them. Building a test proves it compiles; it proves nothing else.
# This gate built ~26 harnesses and executed none of them for its whole life -
# roughly 276 assertions, including a 2.7-million-check fuzz, sitting as
# decoration while the gate reported green. They were run by hand instead, which
# is exactly the habit a gate exists to remove.
#
# EXIT 77 MEANS SKIPPED, NOT FAILED, and this loop has to know that or the
# convention is decoration. gpu_blt.c:624 returns 77 when there is no
# /dev/dri/renderD128 - it is a harness for the Intel blitter and there is
# nothing for it to talk to on a machine without an Intel GPU. Its comment says
# "77 = skip, not fail"; this loop treated every non-zero as FAIL, so that
# contract was honoured by nobody and the gate would have gone red on every
# box without the hardware. Same class as everything in
# docs/GUARDS-THAT-DID-NOT-GUARD.md: a stated guarantee whose only consumer
# never implemented it.
#
# A skip is COUNTED AND PRINTED rather than folded into the pass count. "27
# passed" when three of them did nothing is the false green this gate exists to
# stop, and a hardware harness that silently stops running is exactly how GPU
# work would rot.
echo; echo "=== hosttest run ==="
hf=0; hp=0; hs=0
for t in "$WT"/kernel/hosttest/*; do
  [ -x "$t" ] && [ ! -d "$t" ] || continue
  case "$(basename "$t")" in
    *.*|intel_probe|modeset_test|dpll_test|gpu_fillrate|gpu-dev.sh|modeset-run.sh|jmptest32) continue;;
  esac
  ( cd "$WT/kernel/hosttest" && timeout 180 "./$(basename "$t")" >/dev/null 2>&1 )
  case $? in
    0)  hp=$((hp+1));;
    77) echo "SKIP: $(basename "$t") (77 - hardware or device not present here)"
        hs=$((hs+1));;
    *)  echo "FAIL: $(basename "$t")"; hf=$((hf+1));;
  esac
done
if [ $hf -gt 0 ]; then FAIL=$((FAIL+1)); echo ">>> FAIL (hosttest run: $hf of $((hp+hf)))"
else echo ">>> ok (hosttest run: $hp passed, $hs skipped)"; fi

# --- the two static checkers. Neither builds anything or boots anything, so
# there is no excuse for them not being in the gate: check-zl-calls proves every
# kernel.zl call site resolves (zl has no compile-time check for that at all),
# and check-memmap proves no two fixed addresses overlap - which is how
# LINE_BUF and DISK_SCRATCH sat on 0x02030000 through a whole integration.
[ -x "$WT/kernel/check-zl-calls.sh" ] && run "zl call sites" "$WT/kernel" ./check-zl-calls.sh
[ -x "$WT/kernel/check-memmap.sh" ]   && run "memory map"    "$WT/kernel" ./check-memmap.sh

# --- the reverse SOURCES check: a .c present but not listed is silently not compiled
if [ -f "$WT/kernel/SOURCES" ]; then
  echo; echo "=== reverse SOURCES sweep ==="
  # SOURCES proves every listed file is compiled. This proves the reverse: that
  # a .c sitting in kernel/ is not silently absent from the build. Three
  # outcomes, and only one of them is a failure.
  miss=0; hostonly=0; dead=0
  for f in "$WT"/kernel/*.c; do
    b=$(basename "$f")
    # compiled outside the SOURCES loop by every target, deliberately
    case "$b" in _gen*.c|gdt.c|gdt64.c|efi.c|out.c) continue;; esac
    grep -qx "$b" "$WT/kernel/SOURCES" && continue
    if grep -q "$b" "$WT/kernel/hosttest/build.sh" 2>/dev/null; then
      # host-only: a harness compiles it, the kernel does not. Correct.
      echo "host-only (not in the kernel): $b"; hostonly=$((hostonly+1))
    elif grep -lsr -- "${b%.c}" "$WT"/kernel/*.c "$WT"/kernel/*.h 2>/dev/null \
         | grep -qv "/$b\$"; then
      # something references it but SOURCES does not list it - this is the
      # silent-drop this whole check exists for
      echo "NOT IN SOURCES: $b"; miss=$((miss+1))
    else
      # referenced by nothing at all. Not a build failure; dead weight.
      echo "dead (referenced by nothing): $b"; dead=$((dead+1))
    fi
  done
  if [ $miss -gt 0 ]; then
    FAIL=$((FAIL+1)); echo ">>> FAIL (reverse SOURCES: $miss silently uncompiled)"
  else
    echo ">>> ok (reverse SOURCES; $hostonly host-only, $dead dead)"
  fi
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
