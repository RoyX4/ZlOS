#!/usr/bin/env bash
# freestanding/build.sh - compile a .zl program to a binary with NO libc.
#
#   zl -> compile (C backend) -> out.c -> gcc -ffreestanding -nostdlib
#                                       + runtime_kernel.c
#                                       + start.c (or efi_main, in a kernel)
#
# This is the proof that design_ffi_syscalls.md §6.4's "separate, smaller
# epic" is real: the C backend CAN produce freestanding code, so the two
# W5 blockers it names (freestanding output, privileged instructions) are
# supplied by gcc rather than having to be built into zl itself.
#
# Usage: freestanding/build.sh program.zl [output]
set -euo pipefail
cd "$(dirname "$0")/.."

SRC=${1:?usage: freestanding/build.sh program.zl [output]}
OUT=${2:-freestanding/a.out}

[ -x ./compile ] || { echo "build the toolchain first: ./build.sh"; exit 1; }

./compile "$SRC" >/dev/null
cp out.c freestanding/_gen.c

# -ffreestanding : no libc is assumed to exist
# -nostdlib      : do not link the CRT or libc
# -fno-pic/-no-pie: fixed addresses; a kernel image is not relocated by ld.so
#                  (note design_kernel.md K1 wants RIP-relative for UEFI -
#                   that is a kernel-backend concern, not this harness's)
gcc -O2 -ffreestanding -nostdlib -fno-stack-protector -fno-pic -no-pie \
    -I. -Isrc/runtime -o "$OUT" \
    freestanding/start.c freestanding/_gen.c freestanding/runtime_kernel.c

echo "built $OUT"
echo "  undefined symbols: $(nm -u "$OUT" 2>/dev/null | wc -l)   (0 = genuinely libc-free)"
echo "  size:              $(stat -c%s "$OUT") bytes"
