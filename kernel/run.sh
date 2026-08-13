#!/usr/bin/env bash
# run.sh - boot zlOS.
#
#   ./run.sh            a QEMU window shows the screen, you type here
#   ./run.sh --term     no window; everything in this terminal
#
# There is no PS/2 keyboard driver yet, so input arrives over the serial
# line - which means you type into this terminal, not into the QEMU window.
# That is the one thing a keyboard driver would fix.
set -euo pipefail
cd "$(dirname "$0")"

./build.sh >/dev/null

COMMON=(-kernel kernel.elf
        -device isa-debug-exit,iobase=0xf4,iosize=0x04
        -no-reboot)

if [ "${1:-}" = "--term" ]; then
    shift
    echo "zlOS - terminal mode. Type here. 'q' halts, Ctrl-A X force-quits QEMU."
    echo
    exec qemu-system-i386 "${COMMON[@]}" -nographic "$@"
fi

cat <<'EOT'
zlOS booting.

  A QEMU window is opening - that is the screen.
  TYPE HERE in this terminal (input goes over the serial line).

  h    help          q    halt
  20f  fib(20)       10s  sum of squares
  m    poke/peek video memory
  c    clear

  Ctrl-A X force-quits QEMU.

EOT
exec qemu-system-i386 "${COMMON[@]}" -serial stdio "$@"
