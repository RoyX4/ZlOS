#!/usr/bin/env bash
# The only supported launcher for the complete zlOS landing gate on this host.
# It refuses a busy machine and runs the sequential gate in a resource-bounded
# user service so a compiler or QEMU fault cannot take the desktop with it.

set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
WT=$(cd "$HERE/.." && pwd)
UNIT_BASENAME="zlos-master-land-gate"
UNIT="${UNIT_BASENAME}.service"
LOAD_MAX="1.50"
MEM_MIN_MIB=8192

usage() {
  echo "usage: $0 {doctor|start|status|logs|limits|stop}"
}

load_now() {
  cut -d' ' -f1 /proc/loadavg
}

mem_available_mib() {
  awk '/MemAvailable/{print int($2/1024)}' /proc/meminfo
}

unit_active() {
  systemctl --user is-active --quiet "$UNIT"
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

preflight() {
  local load mem stale
  load=$(load_now)
  mem=$(mem_available_mib)

  if unit_active; then
    echo "contained gate is already active: $UNIT" >&2
    return 1
  fi
  if foreign_process_matches '[l]and-gate\.sh'; then
    echo "another landing gate is already running" >&2
    return 1
  fi
  if foreign_process_matches '[q]emu-system'; then
    echo "QEMU is active; leave it to its owning task and retry later" >&2
    return 1
  fi
  if pgrep -x gcc >/dev/null || pgrep -x clang >/dev/null || \
     pgrep -x cc1 >/dev/null || pgrep -x cc1plus >/dev/null || \
     pgrep -x ld >/dev/null || pgrep -x ld.lld >/dev/null || \
     pgrep -x lld-link >/dev/null; then
    echo "a compiler is active; retry after the other build finishes" >&2
    return 1
  fi
  if awk "BEGIN{exit !($load > $LOAD_MAX)}"; then
    echo "load $load exceeds the start ceiling $LOAD_MAX" >&2
    return 1
  fi
  if [ "$mem" -lt "$MEM_MIN_MIB" ]; then
    echo "available memory ${mem}MiB is below ${MEM_MIN_MIB}MiB" >&2
    return 1
  fi

  stale=$(find "$WT/kernel" -maxdepth 1 -type f -name '*.o.tmp' -print -quit)
  if [ -n "$stale" ]; then
    echo "interrupted compiler temporary exists: $stale" >&2
    echo "inspect and remove it before starting a proof run" >&2
    return 1
  fi

  "$WT/kernel/tools/checks/verify-sources.sh" --selftest-recovery
  "$WT/kernel/tools/checks/verify-sources.sh" --recover-only
  python3 "$WT/kernel/tools/generators/gen-build-identity.py" --write --selftest
  echo "contained-gate preflight: PASS (load=$load, available=${mem}MiB)"
}

start_gate() {
  preflight
  systemctl --user reset-failed "$UNIT" 2>/dev/null || true
  systemd-run --user --unit="$UNIT_BASENAME" --collect --no-block \
    --description="Resource-contained zlOS complete landing gate" \
    --working-directory="$WT" \
    --setenv=ZLOS_CONTAINED_GATE=1 \
    --property=CPUQuota=100% \
    --property=CPUWeight=10 \
    --property=IOWeight=10 \
    --property=Nice=10 \
    --property=MemoryHigh=4G \
    --property=MemoryMax=6G \
    --property=MemorySwapMax=2G \
    --property=TasksMax=128 \
    --property=OOMPolicy=stop \
    --property=KillMode=control-group \
    --property=TimeoutStopSec=30s \
    --property=StandardOutput=journal \
    --property=StandardError=journal \
    /usr/bin/bash "$WT/gates/land-gate.sh" "$WT"
  echo "started $UNIT"
  echo "follow with: $0 logs"
}

case "${1:-}" in
  doctor)
    preflight
    ;;
  start)
    start_gate
    ;;
  status)
    systemctl --user status "$UNIT" --no-pager || true
    ;;
  logs)
    journalctl --user-unit="$UNIT" -f
    ;;
  limits)
    systemctl --user show "$UNIT" \
      -p ActiveState -p SubState -p CPUQuotaPerSecUSec -p CPUWeight \
      -p IOWeight -p Nice -p MemoryHigh -p MemoryMax -p MemorySwapMax \
      -p TasksMax -p OOMPolicy -p KillMode -p TimeoutStopUSec
    ;;
  stop)
    systemctl --user stop "$UNIT"
    ;;
  *)
    usage >&2
    exit 2
    ;;
esac
