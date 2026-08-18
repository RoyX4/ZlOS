#!/usr/bin/env bash
# mkdisk.sh - build zlOS.img, a bootable disk with OUR bootloader. No GRUB.
#
#   kernel.zl -> ../compile -> out.c -> gcc -m32 -ffreestanding
#             +  raw_entry.S (our entry, no multiboot)
#             +  the same console/font/runtime objects
#             -> link-raw.ld (linked at 0x10000) -> objcopy -> kernel_raw.bin
#   raw_boot.asm -> nasm -> raw_boot.bin (512-byte boot sector)
#   disk = boot sector ++ kernel, padded
#
#   qemu-system-i386 -drive file=zlOS.img,format=raw     (boots with no GRUB)
#
# This is the BIOS/legacy path. It does not boot a UEFI-only machine - that
# needs an EFI loader, which is a separate, larger job (design_kernel.md §3).
set -euo pipefail
cd "$(dirname "$0")"

SRC=${1:-kernel.zl}
[ -x ../compile ] || { echo "build the toolchain first: ../build.sh"; exit 1; }

../compile "$SRC" >/dev/null
cp out.c _gen.c

CFLAGS="-m32 -O2 -ffreestanding -nostdlib -fno-stack-protector -fno-pic
        -fno-builtin -Wall -Wextra -Wno-unused-parameter -I.."

# the kernel objects (same as build.sh, but our raw entry instead of boot.S)
# shellcheck disable=SC2086
gcc $CFLAGS -DZL_KERNEL_SERIAL -c ../freestanding/runtime_kernel.c -o _rt.o
# shellcheck disable=SC2086
gcc $CFLAGS -c _gen.c    -o _gen.o
gcc $CFLAGS -c gdt.c     -o _gdt.o

# THE SHARED SOURCE LIST. See ./SOURCES - one file, read by all four build
# scripts. This script is the one that used to be forgotten: verify.sh builds
# only build.sh, so a .c added there and nowhere else broke the raw disk image
# with nothing to catch it.
OBJS=""
while read -r f; do
    case "$f" in ''|\#*) continue ;; esac
    o="_$(basename "$f" .c).o"
    EXTRA=""
    case "$f" in idt.c|apic.c) EXTRA="-mgeneral-regs-only" ;; esac
    # shellcheck disable=SC2086
    gcc $CFLAGS $EXTRA -c "$f" -o "$o"
    OBJS="$OBJS $o"
done < SOURCES

gcc $CFLAGS -c smp_trampoline.S -o _smptr.o
gcc -m32 -c raw_entry.S -o _rawentry.o

# shellcheck disable=SC2086
ld -m elf_i386 -T link-raw.ld -o kernel_raw.elf \
   _rawentry.o _gen.o _rt.o _gdt.o _smptr.o $OBJS
objcopy -O binary kernel_raw.elf kernel_raw.bin

nasm -f bin raw_boot.asm -o raw_boot.bin

# assemble the disk: boot sector first, then the kernel, padded to a round size
cat raw_boot.bin kernel_raw.bin > zlOS.img
# pad to at least what the loader reads (12 * 32 KiB) so no read runs off the end
truncate -s 2M zlOS.img

echo "built zlOS.img - OUR bootloader, no GRUB"
echo "  boot sector: $(stat -c%s raw_boot.bin) bytes  (must be 512)"
echo "  kernel:      $(stat -c%s kernel_raw.bin) bytes"
echo "  disk image:  $(stat -c%s zlOS.img) bytes"
echo
echo "boot it:  qemu-system-i386 -drive file=zlOS.img,format=raw -serial stdio"
