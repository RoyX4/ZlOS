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

python3 ./gen-app-manifest.py --check
python3 ./gen-build-identity.py --check

SRC=${1:-kernel.zl}
[ -x ../compile ] || { echo "build the toolchain first: ../build.sh"; exit 1; }

../compile "$SRC" >/dev/null
cp out.c _gen.c

CFLAGS="-m32 -O2 -ffreestanding -nostdlib -fno-stack-protector -fno-pic
        -fno-builtin -Wall -Wextra -Werror -Wno-unused-parameter -I.."

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
# THE KERNEL MUST FIT WHAT THE LOADER READS, and this check is the only thing
# that says so. raw_boot.asm loads CHUNKS x 32 KiB and then jumps; a kernel
# larger than that is silently TRUNCATED and jumped into, which is a hang with
# no build error and no gate - verify-raw.sh boots the truncated image and can
# only report that it did not come up.
#
# It was dropped by the apps-in-windows landing, which took that branch's
# SOURCES-driven mkdisk.sh wholesale and with it lost this guard. Restored, and
# worth stating plainly: the merge deleted a CHECK, which is worse than deleting
# code, because a check is what catches the next one.
CHUNKS=$(grep -oP 'CHUNKS\s+equ\s+\K[0-9]+' raw_boot.asm)
LIMIT=$((CHUNKS * 64 * 512))
KSIZE=$(stat -c%s kernel_raw.bin)
if [ "$KSIZE" -gt "$LIMIT" ]; then
    echo "FAIL: kernel is $KSIZE bytes; raw_boot.asm loads only $LIMIT" >&2
    echo "      (CHUNKS=$CHUNKS x 32 KiB). Raise CHUNKS and the truncate below." >&2
    exit 1
fi

cat raw_boot.bin kernel_raw.bin > zlOS.img

# PAD TO WHAT THE LOADER READS, DERIVED - never a literal.
#
# The loader reads CHUNKS chunks from LBA 1 unconditionally. If the image is
# shorter than 512 + CHUNKS*32 KiB the last reads run off the end of the file,
# INT 13h returns carry, and raw_boot.asm prints 'D' and halts - a dead machine
# with no build error, from two numbers in two files drifting apart.
#
# This was `truncate -s 2M` beside a comment claiming "12 * 32 KiB", against a
# CHUNKS of 60. All three disagreed: 2 MiB, 384 KiB and 1.875 MiB. It happened
# to work only because 2 MiB was the largest of them. Deriving it means raising
# CHUNKS is now a one-line change that cannot leave this behind.
IMG_MIN=$(( 512 + CHUNKS * 64 * 512 ))
truncate -s "$IMG_MIN" zlOS.img

echo "built zlOS.img - OUR bootloader, no GRUB"
echo "  boot sector: $(stat -c%s raw_boot.bin) bytes  (must be 512)"
echo "  kernel:      $KSIZE bytes  ($(( (LIMIT - KSIZE) / 1024 )) KiB of loader headroom left)"
echo "  disk image:  $(stat -c%s zlOS.img) bytes"
echo
echo "boot it:  qemu-system-i386 -drive file=zlOS.img,format=raw -serial stdio"
