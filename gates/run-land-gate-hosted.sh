#!/usr/bin/env bash
# Run the complete zlOS landing gate only on a disposable GitHub-hosted runner.
# The local launcher protects Roy's desktop with systemd. This launcher instead
# binds execution to GitHub's non-overridable runner identity, rejects self-hosted
# machines, hydrates the exact Kali package/source closure, and bounds the job.

set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
WT=$(cd "$HERE/.." && pwd)
MEM_MIN_MIB=4000
DISK_MIN_MIB=6000
GATE_TIMEOUT=5h
CACHE_BASE="${ZLOS_DEPENDENCY_CACHE:-${RUNNER_TEMP:-}/zlos-dependency-cache}"

usage() {
  echo "usage: $0 {doctor|prepare|hydrate|gate|start}"
}

fail() {
  echo "hosted-gate: $*" >&2
  exit 2
}

foreign_process_matches() {
  local pattern="$1" pid parent ancestry=" "
  pid=$$
  while [ "$pid" -gt 1 ] 2>/dev/null; do
    ancestry+="$pid "
    parent=$(ps -o ppid= -p "$pid" | tr -d ' ')
    [ -n "$parent" ] || break
    pid=$parent
  done
  while read -r pid; do
    [ -n "$pid" ] || continue
    case "$ancestry" in
      *" $pid "*) continue ;;
    esac
    return 0
  done < <(pgrep -f "$pattern" || true)
  return 1
}

verify_runner() {
  local workspace temp mem disk stale
  [ "${GITHUB_ACTIONS:-}" = "true" ] || fail "GITHUB_ACTIONS is not true"
  [ "${CI:-}" = "true" ] || fail "CI is not true"
  [ "${RUNNER_ENVIRONMENT:-}" = "github-hosted" ] || \
    fail "runner is not GitHub-hosted (self-hosted runners are forbidden)"
  [ "${RUNNER_OS:-}" = "Linux" ] || fail "runner OS is not Linux"
  [[ "${GITHUB_RUN_ID:-}" =~ ^[0-9]+$ ]] || fail "GITHUB_RUN_ID is absent or invalid"
  [[ "${GITHUB_RUN_ATTEMPT:-}" =~ ^[0-9]+$ ]] || \
    fail "GITHUB_RUN_ATTEMPT is absent or invalid"
  [ -n "${GITHUB_WORKSPACE:-}" ] || fail "GITHUB_WORKSPACE is absent"
  [ -n "${RUNNER_TEMP:-}" ] || fail "RUNNER_TEMP is absent"
  workspace=$(cd "$GITHUB_WORKSPACE" 2>/dev/null && pwd) || \
    fail "GITHUB_WORKSPACE is unreadable"
  temp=$(cd "$RUNNER_TEMP" 2>/dev/null && pwd) || fail "RUNNER_TEMP is unreadable"
  [ "$workspace" = "$WT" ] || fail "launcher is outside GITHUB_WORKSPACE"
  [ -w "$temp" ] || fail "RUNNER_TEMP is not writable"

  mem=$(awk '/MemAvailable/{print int($2/1024)}' /proc/meminfo)
  [ "$mem" -ge "$MEM_MIN_MIB" ] || \
    fail "available memory ${mem}MiB is below ${MEM_MIN_MIB}MiB"
  disk=$(df -Pm "$temp" | awk 'NR == 2 {print $4}')
  [ "$disk" -ge "$DISK_MIN_MIB" ] || \
    fail "available temporary disk ${disk}MiB is below ${DISK_MIN_MIB}MiB"

  foreign_process_matches '[l]and-gate\.sh' && fail "another landing gate is active"
  foreign_process_matches '[q]emu-system' && fail "another QEMU process is active"
  if pgrep -x gcc >/dev/null || pgrep -x clang >/dev/null || \
     pgrep -x cc1 >/dev/null || pgrep -x cc1plus >/dev/null || \
     pgrep -x ld >/dev/null || pgrep -x ld.lld >/dev/null || \
     pgrep -x lld-link >/dev/null; then
    fail "another compiler or linker is active"
  fi

  stale=$(find "$WT/kernel" -maxdepth 1 -type f -name '*.o.tmp' -print -quit)
  [ -z "$stale" ] || fail "interrupted compiler temporary exists: $stale"
  echo "hosted-gate runner: PASS (run=$GITHUB_RUN_ID attempt=$GITHUB_RUN_ATTEMPT, memory=${mem}MiB, temp-disk=${disk}MiB)"
}

prepare() {
  verify_runner
  "$WT/kernel/tools/checks/verify-sources.sh" --selftest-recovery
  "$WT/kernel/tools/checks/verify-sources.sh" --recover-only
  python3 "$WT/kernel/tools/generators/gen-build-identity.py" --write --selftest
  python3 "$WT/kernel/tools/generators/gen-dependency-lock.py" --write --selftest
  local lock_sha
  lock_sha=$(sha256sum "$WT/kernel/metadata/dependency-lock.json" | cut -d' ' -f1)
  if [ -n "${GITHUB_OUTPUT:-}" ]; then
    echo "dependency-lock-sha256=$lock_sha" >> "$GITHUB_OUTPUT"
  fi
  echo "hosted-gate preparation: PASS (dependency-lock=$lock_sha)"
}

hydrate() {
  prepare
  mkdir -p "$CACHE_BASE"
  python3 "$WT/kernel/tools/checks/verify-dependency-archives.py" \
    --hydrate --cache-root "$CACHE_BASE" --selftest
}

gate() {
  prepare
  python3 "$WT/kernel/tools/checks/verify-dependency-archives.py" \
    --check --cache-root "$CACHE_BASE" --selftest
  (
    ulimit -c 0
    ulimit -f 8388608
    ulimit -n 4096
    ulimit -u 384
    exec timeout --foreground --kill-after=30s "$GATE_TIMEOUT" \
      env ZLOS_HOSTED_GATE=1 \
          ZLOS_HOSTED_GATE_RUN_ID="$GITHUB_RUN_ID" \
          ZLOS_HOSTED_GATE_WORKSPACE="$WT" \
          ZLOS_DEPENDENCY_CACHE="$CACHE_BASE" \
          bash "$WT/gates/land-gate.sh" "$WT"
  )
}

case "${1:-}" in
  doctor)
    verify_runner
    ;;
  prepare)
    prepare
    ;;
  hydrate)
    hydrate
    ;;
  gate)
    gate
    ;;
  start)
    hydrate
    gate
    ;;
  *)
    usage >&2
    exit 2
    ;;
esac
