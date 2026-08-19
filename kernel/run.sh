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

# -m 1G is HI_TOP (memmap.h). This script passed no -m for its whole life, so a
# hand-run boot got qemu's 128 MiB default and behaved differently from every
# gate - the high-RAM map above 128 MiB was simply absent. check-ram.sh fails if
# this and HI_TOP ever disagree.
COMMON=(-kernel kernel.elf
        -m 1G
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
