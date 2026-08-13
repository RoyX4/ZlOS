#!/usr/bin/env bash
# kernel/build.sh - compile kernel.zl into a bootable multiboot kernel.
#
#   kernel.zl -> ../compile (C backend) -> out.c
#             -> gcc -m32 -ffreestanding -nostdlib
#             +  runtime_kernel.c (-DZL_KERNEL_SERIAL: print goes to COM1)
#             +  boot.S (multiboot header, stack, entry)
#             +  support.c (outb/inb, UART bring-up)
#             -> kernel.elf, bootable by QEMU -kernel
set -euo pipefail
cd "$(dirname "$0")"

SRC=${1:-kernel.zl}
[ -x ../compile ] || { echo "build the toolchain first: ../build.sh"; exit 1; }

../compile "$SRC" >/dev/null
cp out.c _gen.c            # compile writes out.c into the CWD, not ..

CFLAGS="-m32 -O2 -ffreestanding -nostdlib -fno-stack-protector -fno-pic
        -fno-builtin -Wall -Wextra -Wno-unused-parameter -I.."

# shellcheck disable=SC2086
gcc $CFLAGS -DZL_KERNEL_SERIAL -c ../freestanding/runtime_kernel.c -o _rt.o
# shellcheck disable=SC2086
gcc $CFLAGS -c _gen.c    -o _gen.o
# shellcheck disable=SC2086
gcc $CFLAGS -c support.c -o _support.o
gcc $CFLAGS -c vga.c      -o _vga.o
gcc $CFLAGS -c fb.c       -o _fb.o
gcc $CFLAGS -c font8x16.c -o _font.o
gcc $CFLAGS -c console.c  -o _console.o
gcc $CFLAGS -c divmod.c   -o _divmod.o
gcc -m32 -c boot.S -o _boot.o

# No -lgcc. __divdi3/__moddi3 (64-bit division on a 32-bit target) are the
# only things the kernel took from libgcc, and divmod.c now supplies them.
# Nothing GNU is linked into the kernel any more - only gcc-the-tool that
# compiled the C, which nativegen is on track to replace.
ld -m elf_i386 -T link.ld -o kernel.elf _boot.o _gen.o _rt.o _support.o _vga.o _fb.o _font.o _console.o _divmod.o

echo "built kernel.elf"
echo "  undefined symbols: $(nm -u kernel.elf 2>/dev/null | wc -l)   (0 = no libc, no OS)"
echo "  size:              $(stat -c%s kernel.elf) bytes"
echo "  multiboot header:  $(grep -c . < <(od -A d -t x4 -N 8192 kernel.elf | grep -c '1badb002' || true)) found"
