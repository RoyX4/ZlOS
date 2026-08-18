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
# shellcheck disable=SC2086
gcc $CFLAGS -c support.c -o _support.o
# shellcheck disable=SC2086
gcc $CFLAGS -c vga.c      -o _vga.o
# shellcheck disable=SC2086
gcc $CFLAGS -c fb.c       -o _fb.o
# shellcheck disable=SC2086
gcc $CFLAGS -c fb3d.c     -o _fb3d.o
gcc $CFLAGS -c font8x16.c -o _font.o
gcc $CFLAGS -c font_aa.c  -o _fontaa.o
gcc $CFLAGS -c font_sub.c -o _fontsub.o
gcc $CFLAGS -c icons.c    -o _icons.o
gcc $CFLAGS -c pci.c      -o _pci.o
gcc $CFLAGS -c bga.c      -o _bga.o
gcc $CFLAGS -c intel.c    -o _intel.o
gcc $CFLAGS -c xhci.c     -o _xhci.o
# shellcheck disable=SC2086
gcc $CFLAGS -c console.c  -o _console.o
# shellcheck disable=SC2086
gcc $CFLAGS -c divmod.c   -o _divmod.o
gcc $CFLAGS -c gdt.c      -o _gdt.o
# interrupt handlers must not touch SSE - -mgeneral-regs-only enforces it
gcc $CFLAGS -mgeneral-regs-only -c idt.c -o _idt.o
gcc $CFLAGS -mgeneral-regs-only -c apic.c -o _apic.o
# virtio-gpu: the one GPU driver we can prove on every build.
gcc $CFLAGS -c virtio_gpu.c -o _vgpu.o
# reading the processor itself: CPUID, topology, caches, the TSC
gcc $CFLAGS -c cpu.c -o _cpu.o
# NVMe: real storage, so something survives a power cycle
gcc $CFLAGS -c nvme.c -o _nvme.o
# the scheduler: more than one thing at a time
gcc $CFLAGS -c sched.c -o _sched.o
# SMP: waking the other cores
gcc $CFLAGS -c smp.c -o _smp.o
# I2C-HID: the touchpad
gcc $CFLAGS -c i2c_hid.c -o _i2c.o
# the input stack: events, modifiers, repeat
gcc $CFLAGS -c input.c -o _input.o
# the terminal app: scrollback + the typed-command matcher
gcc $CFLAGS -c term.c -o _term.o
# the compositor. runtime_kernel.c calls into it for the wm_* builtins, so
# every build that links the runtime needs these three - not just build.sh.
gcc $CFLAGS -c wm.c -o _wm.o
gcc $CFLAGS -c ui.c -o _ui.o
gcc $CFLAGS -c wmglue.c -o _wmglue.o
gcc $CFLAGS -c settings.c -o _settings.o
gcc $CFLAGS -c smp_trampoline.S -o _smptr.o
gcc -m32 -c raw_entry.S -o _rawentry.o

ld -m elf_i386 -T link-raw.ld -o kernel_raw.elf \
   _rawentry.o _gen.o _rt.o _support.o _vga.o _fb.o _fb3d.o _font.o _fontaa.o _fontsub.o _icons.o _pci.o _bga.o _intel.o _xhci.o _console.o _divmod.o _gdt.o _idt.o _apic.o _vgpu.o _cpu.o _nvme.o _sched.o _smp.o _smptr.o _i2c.o _input.o _term.o _wm.o _ui.o _wmglue.o _settings.o
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
