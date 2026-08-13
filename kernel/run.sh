#!/usr/bin/env bash
# boot the kernel with serial on your terminal; Ctrl-A X to quit QEMU
set -euo pipefail
cd "$(dirname "$0")"
./build.sh >/dev/null
exec qemu-system-i386 -kernel kernel.elf -serial stdio \
     -device isa-debug-exit,iobase=0xf4,iosize=0x04 -no-reboot "$@"
