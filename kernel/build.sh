#!/usr/bin/env bash
# kernel/build.sh - compile src/kernel.zl into a bootable multiboot kernel.
#
#   src/kernel.zl -> ../compile (C backend) -> out.c
#             -> gcc -m32 -ffreestanding -nostdlib
#             +  runtime_kernel.c (-DZL_KERNEL_SERIAL: print goes to COM1)
#             +  boot/boot.S (multiboot header, stack, entry)
#             +  src/arch/x86/support.c (outb/inb, UART bring-up)
#             -> kernel.elf, bootable by QEMU -kernel
set -euo pipefail
cd "$(dirname "$0")"

SRC=${1:-src/kernel.zl}
[ -x ../compile ] || { echo "build the toolchain first: ../build.sh"; exit 1; }

ZL_STDLIB="$PWD/apps" ../compile "$SRC" >/dev/null
cp out.c _gen.c            # compile writes out.c into the CWD, not ..

INCLUDES=$(find src boot -type d -printf ' -I%s' | sort)
CFLAGS="-m32 -O2 -ffreestanding -nostdlib -fno-stack-protector -fno-pic
        -fno-builtin -Wall -Wextra -Wno-unused-parameter -I.. -I../src/runtime $INCLUDES"

# shellcheck disable=SC2086
gcc $CFLAGS -DZL_KERNEL_SERIAL -c ../freestanding/runtime_kernel.c -o _rt.o
# shellcheck disable=SC2086
gcc $CFLAGS -c _gen.c    -o _gen.o
# shellcheck disable=SC2086
gcc $CFLAGS -c boot/gdt.c -o _gdt.o

# THE SHARED SOURCE LIST. See ./SOURCES - one file, read by all four build
# scripts, so a new driver cannot be added to the build you happen to be
# testing with and silently missed by the other three.
OBJS=""
while read -r f; do
    case "$f" in ''|\#*) continue ;; esac
    o="_$(basename "$f" .c).o"
    # Interrupt handlers must not touch SSE - an ISR that clobbers XMM without
    # saving it lands on the zl interpreter, where every number is a double.
    EXTRA=""
    case "$f" in */idt.c|*/apic.c) EXTRA="-mgeneral-regs-only" ;; esac
    # shellcheck disable=SC2086
    gcc $CFLAGS $EXTRA -c "$f" -o "$o"
    OBJS="$OBJS $o"
done < SOURCES

gcc $CFLAGS -c boot/smp_trampoline.S -o _smptr.o
gcc -m32 -c boot/boot.S -o _boot.o

# No -lgcc. __divdi3/__moddi3 (64-bit division on a 32-bit target) are the
# only things the kernel took from libgcc, and divmod.c now supplies them.
# Nothing GNU is linked into the kernel any more - only gcc-the-tool that
# compiled the C, which nativegen is on track to replace.
# shellcheck disable=SC2086
ld -m elf_i386 -T boot/link.ld -o kernel.elf _boot.o _gen.o _rt.o _gdt.o _smptr.o $OBJS

echo "built kernel.elf"
echo "  undefined symbols: $(nm -u kernel.elf 2>/dev/null | wc -l)   (0 = no libc, no OS)"
echo "  size:              $(stat -c%s kernel.elf) bytes"
echo "  multiboot header:  $(grep -c . < <(od -A d -t x4 -N 8192 kernel.elf | grep -c '1badb002' || true)) found"
