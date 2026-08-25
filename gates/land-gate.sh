#!/bin/bash
# Build gate for a zlOS landing. Runs sequentially, never in parallel, and
# NEVER masks an exit code — the previous version piped every build through
# `tail`, so every step reported the exit status of `tail` (always 0) and a
# tree that did not link gated green.
#
# Do not invoke this file directly. `run-land-gate-contained.sh start` is the
# only supported entry point on the four-core development machine. It gives
# the gate its own resource-bounded cgroup and preserves the desktop.

set -u
cgroup_path=$(awk -F: '$1 == "0" { print $3 }' /proc/$$/cgroup)
if [ "${ZLOS_CONTAINED_GATE:-}" != "1" ] || \
   [[ "$cgroup_path" != *"/zlos-master-land-gate.service" ]]; then
  echo "land-gate: refusing unrestricted execution" >&2
  echo "use: gates/run-land-gate-contained.sh start" >&2
  exit 2
fi
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
  # The 2026-08-24 desktop freeze had no surviving OOM or panic record. Guard
  # both load and memory without pretending either was the sole cause.
  if awk "BEGIN{exit !($la > 4.0)}"; then echo "load $la > 4.0 — waiting"; return 1; fi
  if [ "$mem" -lt 3000 ]; then echo "available memory ${mem}MB < 3000 — waiting"; return 1; fi
  if pgrep '^qemu-system' >/dev/null; then echo "a qemu is already running — waiting"; return 1; fi
  return 0
}

echo "gate: $WT @ $(git rev-parse --short HEAD)"
echo "load: $(cut -d' ' -f1-3 /proc/loadavg)   avail: $(awk '/MemAvailable/{print int($2/1024)}' /proc/meminfo)MB"

# Individual boot scripts remain useful on reduced developer machines and may
# report a skip when QEMU or firmware is unavailable. The complete landing gate
# is not optional: prove all of its prerequisites up front so a skip can never
# be promoted to a green landing.
run "mandatory boot prerequisites" "$WT/kernel" \
    python3 tools/checks/check-boot-prereqs.py --selftest
run "contained gate launcher contract" "$WT/gates" \
    python3 check-contained-gate.py --selftest
run "landing authority closure" "$WT/kernel" \
    python3 tools/checks/check-land-gate.py --selftest
run "wrapper inventory write" "$WT/kernel" \
    python3 tools/generators/gen-wrapper-registry.py --write --selftest
run "wrapper inventory check" "$WT/kernel" \
    python3 tools/generators/gen-wrapper-registry.py --check --selftest
run "warning-strict build contract" "$WT/kernel" \
    python3 tools/checks/check-build-contract.py --selftest
run "host dependency lock" "$WT/kernel" \
    python3 tools/generators/gen-dependency-lock.py --check --selftest
run "license/provenance truth" "$WT/kernel" \
    python3 tools/generators/gen-license-registry.py --check --selftest

# --- toolchain and compile-only steps (cheap, no QEMU)
run "zl toolchain"     "$WT"               ./build.sh
run "build input identity" "$WT/kernel" python3 tools/generators/gen-build-identity.py --check --selftest
run "toolchain manifest write" "$WT/kernel" python3 tools/generators/gen-toolchain-manifest.py --write --selftest
run "toolchain manifest check" "$WT/kernel" python3 tools/generators/gen-toolchain-manifest.py --check --selftest
run "build graph write" "$WT/kernel" python3 tools/generators/gen-build-graph.py --write --selftest
run "build graph check" "$WT/kernel" python3 tools/generators/gen-build-graph.py --check --selftest
run "source snapshot write" "$WT/kernel" python3 tools/generators/gen-source-snapshot.py --write --selftest
run "source snapshot check" "$WT/kernel" python3 tools/generators/gen-source-snapshot.py --check --selftest
run "kernel 32-bit"    "$WT/kernel"        ./build.sh
run "kernel 64-bit"    "$WT/kernel"        ./build64.sh
run "kernel EFI"       "$WT/kernel"        ./buildefi.sh
run "kernel ELF permissions" "$WT/kernel" python3 tools/checks/check-elf-permissions.py --selftest
run "SOURCES recovery selftest" "$WT/kernel" ./tools/checks/verify-sources.sh --selftest-recovery
run "SOURCES coverage" "$WT/kernel" ./tools/checks/verify-sources.sh
run "hosttest build"   "$WT/kernel/tests/host" ./build.sh

# --- RUN every declared gate through the generated inventory. The old loop
# guessed semantics from filenames. It therefore ran parsestat even though its
# own build comment says "NOT A GATE", skipped jmptest32, and skipped every
# executable shell test because `*.*` was its blanket exclusion. Instruments,
# manual hardware actions, optional builds, real gates and exit-77 hardware
# absences are now distinct machine-checked states. No non-run is counted as a
# pass and every compiled output plus executable script must be classified.
run "host test inventory" "$WT/kernel" python3 tools/generators/gen-test-inventory.py --check --selftest
run "host tests execute" "$WT/kernel" python3 tools/run/run-host-tests.py --run --selftest
until guard; do sleep 30; done
run "host benchmark receipt" "$WT/kernel" python3 tools/run/run-benchmarks.py --run --selftest

# --- the two static checkers. Neither builds anything or boots anything, so
# there is no excuse for them not being in the gate: check-zl-calls proves every
# kernel.zl call site resolves (zl has no compile-time check for that at all),
# and check-memmap proves no two fixed addresses overlap - which is how
# LINE_BUF and DISK_SCRATCH sat on 0x02030000 through a whole integration.
run "zl call sites" "$WT/kernel" ./tools/checks/check-zl-calls.sh
run "memory map" "$WT/kernel" ./tools/checks/check-memmap.sh
run "memmap guards" "$WT/kernel/tests/host" ./memmap-guard-test.sh
run "unique app ids" "$WT/kernel" ./tools/checks/check-appids.py --selftest
run "app registry coverage" "$WT" python3 kernel/tests/host/apps53.py --selftest
run "61-app manifest" "$WT/kernel" python3 tools/generators/gen-app-manifest.py --check --selftest
run "app lifecycle verifier" "$WT/kernel" python3 tools/probes/probe-app-lifecycle.py --selftest
run "reproducible artifact verifier" "$WT/kernel" python3 tools/checks/check-reproducible-build.py --selftest
# check-memmap.sh reads kernel.zl and no C at all, which is why intel.c's
# edid_buf sat inside fb.c's blur arena while it printed a clean map. This is
# the other half: every page-aligned hex literal in a .c or .h that lands
# strictly inside a declared HI_* region without being its base.
run "high-RAM map" "$WT/kernel" ./tools/checks/check-himap.sh

# --- the reverse SOURCES check: a .c present but not listed is silently not compiled
if [ -f "$WT/kernel/SOURCES" ]; then
  echo; echo "=== reverse SOURCES sweep ==="
  # SOURCES proves every listed file is compiled. This proves the reverse: that
  # a .c sitting in kernel/src is not silently absent from the build. Three
  # outcomes, and only one of them is a failure.
  miss=0; hostonly=0; dead=0
  while IFS= read -r f; do
    b=$(basename "$f")
    rel=${f#"$WT/kernel/"}
    # compiled outside the SOURCES loop by every target, deliberately
    case "$rel" in src/runtime/interp_kernel.c) continue;; esac
    grep -qx "$rel" "$WT/kernel/SOURCES" && continue
    if grep -q "$rel\|$b" "$WT/kernel/tests/host/build.sh" 2>/dev/null; then
      # host-only: a harness compiles it, the kernel does not. Correct.
      echo "host-only (not in the kernel): $b"; hostonly=$((hostonly+1))
    elif grep -lsr -- "${b%.c}" "$WT"/kernel/src "$WT"/kernel/boot 2>/dev/null \
         | grep -qv "/$b\$"; then
      # something references it but SOURCES does not list it - this is the
      # silent-drop this whole check exists for
      echo "NOT IN SOURCES: $b"; miss=$((miss+1))
    else
      # referenced by nothing at all. Not a build failure; dead weight.
      echo "dead (referenced by nothing): $b"; dead=$((dead+1))
    fi
  done < <(find "$WT/kernel/src" -type f -name '*.c' | sort)
  if [ $miss -gt 0 ]; then
    FAIL=$((FAIL+1)); echo ">>> FAIL (reverse SOURCES: $miss silently uncompiled)"
  else
    echo ">>> ok (reverse SOURCES; $hostonly host-only, $dead dead)"
  fi
else
  FAIL=$((FAIL+1)); echo ">>> FAIL (reverse SOURCES: kernel/SOURCES is missing)"
fi

# --- boot gates: QEMU under TCG, one at a time, guarded
run "reproducible kernel and ISO" "$WT/kernel" python3 tools/checks/check-reproducible-build.py --check
for g in tools/images/mkiso.sh verify.sh tools/checks/verify-iso.sh \
         tools/checks/verify-64.sh tools/checks/verify-efi.sh \
         tools/checks/verify-raw.sh tools/checks/verify-disk.sh \
         tools/checks/verify-clock.sh tools/checks/verify-net.sh; do
  until guard; do sleep 30; done
  run "boot: $g" "$WT/kernel" "./$g"
done

# --- Evidence joins must run AFTER the recipes and boot gates that create the
# artifacts and receipts they claim. The old order checked app-evidence.json
# before refreshing any receipt, then rebuilt the ISO several times and never
# checked the join again. Finish on one canonical ISO, exercise its graphical
# routes without rebuilding between probes, regenerate the join, and only then
# promote the exact artifact/route registry.
run "final canonical ISO" "$WT/kernel" ./tools/images/mkiso.sh
until guard; do sleep 30; done
run "CPU fault capture QEMU" "$WT/kernel" python3 tools/checks/verify-crash.py --run \
    --no-build --selftest
until guard; do sleep 30; done
run "app routes QEMU" "$WT/kernel" python3 tools/probes/probe-app-routes.py --no-build \
    --receipt docs/receipts/app-routes-qemu-2026-08-22.json
until guard; do sleep 30; done
run "47-app lifecycle QEMU" "$WT/kernel" python3 tools/probes/probe-app-lifecycle.py --no-build \
    --receipt docs/receipts/app-lifecycle-qemu-2026-08-22.json
until guard; do sleep 30; done
run "Run route QEMU" "$WT/kernel" python3 probe-run.py --no-build \
    --receipt docs/receipts/run-qemu-2026-08-22.json
run "62-surface evidence registry write" "$WT/kernel" \
    python3 tools/generators/gen-app-evidence.py --write --verify-artifact
run "62-surface evidence registry check" "$WT/kernel" \
    python3 tools/generators/gen-app-evidence.py --check --selftest --verify-artifact
run "artifact and boot-route registry write" "$WT/kernel" \
    python3 tools/generators/gen-artifact-registry.py --write --selftest
run "artifact and boot-route registry check" "$WT/kernel" \
    python3 tools/generators/gen-artifact-registry.py --check --selftest
run "initialization registry write" "$WT/kernel" \
    python3 tools/generators/gen-init-registry.py --write --selftest
run "initialization registry check" "$WT/kernel" \
    python3 tools/generators/gen-init-registry.py --check --selftest
run "adversarial registry write" "$WT/kernel" \
    python3 tools/generators/gen-adversarial-registry.py --write --selftest
run "adversarial registry check" "$WT/kernel" \
    python3 tools/generators/gen-adversarial-registry.py --check --selftest
run "host benchmark receipt check" "$WT/kernel" \
    python3 tools/run/run-benchmarks.py --check --selftest
run "visual evidence registry write" "$WT/kernel" \
    python3 tools/generators/gen-visual-registry.py --write --selftest
run "visual evidence registry check" "$WT/kernel" \
    python3 tools/generators/gen-visual-registry.py --check --selftest
run "accessibility proof registry write" "$WT/kernel" \
    python3 tools/generators/gen-accessibility-registry.py --write --selftest
run "accessibility proof registry check" "$WT/kernel" \
    python3 tools/generators/gen-accessibility-registry.py --check --selftest
run "security claim registry write" "$WT/kernel" \
    python3 tools/generators/gen-security-registry.py --write --selftest
run "security claim registry check" "$WT/kernel" \
    python3 tools/generators/gen-security-registry.py --check --selftest
run "decision ledger write" "$WT/kernel" \
    python3 tools/generators/gen-decision-ledger.py --write --selftest
run "decision ledger check" "$WT/kernel" \
    python3 tools/generators/gen-decision-ledger.py --check --selftest
run "event trace host receipt write" "$WT/kernel" \
    python3 tools/checks/verify-event-trace.py --write --selftest
run "event trace host receipt check" "$WT/kernel" \
    python3 tools/checks/verify-event-trace.py --check --selftest
run "event schema registry write" "$WT/kernel" \
    python3 tools/generators/gen-event-schema.py --write --selftest
run "event schema registry check" "$WT/kernel" \
    python3 tools/generators/gen-event-schema.py --check --selftest
run "observability registry write" "$WT/kernel" \
    python3 tools/generators/gen-observability-registry.py --write --selftest
run "observability registry check" "$WT/kernel" \
    python3 tools/generators/gen-observability-registry.py --check --selftest
run "release notes write" "$WT/kernel" \
    python3 tools/generators/gen-release-notes.py --write --selftest
run "release notes check" "$WT/kernel" \
    python3 tools/generators/gen-release-notes.py --check --selftest
run "provenance viewer write" "$WT/kernel" \
    python3 tools/generators/gen-provenance-viewer.py --write --selftest
run "provenance viewer check" "$WT/kernel" \
    python3 tools/generators/gen-provenance-viewer.py --check --selftest
run "joined evidence registry write" "$WT/kernel" \
    python3 tools/generators/gen-evidence-registry.py --write --selftest
run "joined evidence registry check" "$WT/kernel" \
    python3 tools/generators/gen-evidence-registry.py --check --selftest

echo
echo "================================"
if [ $FAIL -eq 0 ]; then echo "GATE GREEN"; else echo "GATE RED — $FAIL failing step(s)"; fi
echo "================================"
exit $FAIL
