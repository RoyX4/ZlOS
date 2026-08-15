#!/bin/sh
# build64.sh - the 64-bit kernel. Long mode, entered from multiboot.
#
# Everything except the boot path and the GDT is shared with the 32-bit build:
# the zl-generated code, the runtime and every driver compile unchanged for
# x86-64. -mno-red-zone is mandatory in a kernel (an interrupt would otherwise
# clobber the 128 bytes below RSP that the ABI lets leaf functions use), and
# -mcmodel=large keeps addresses absolute rather than RIP-relative-with-2GB.
set -e
cd "$(dirname "$0")"

../compile kernel.zl >/dev/null
cp out.c _gen64.c

CFLAGS="-m64 -O2 -ffreestanding -nostdlib -fno-stack-protector -fno-pic -fno-builtin -mno-red-zone -mcmodel=large -DZL_64 -Wall -Wextra -Wno-unused-parameter -I.."

# shellcheck disable=SC2086
gcc $CFLAGS -DZL_KERNEL_SERIAL -c ../freestanding/runtime_kernel.c -o _rt64.o
# shellcheck disable=SC2086
gcc $CFLAGS -c _gen64.c   -o _gen64.o
gcc $CFLAGS -c support.c  -o _support64.o
gcc $CFLAGS -c vga.c      -o _vga64.o
gcc $CFLAGS -c fb.c       -o _fb64.o
gcc $CFLAGS -c fb3d.c     -o _fb3d64.o
gcc $CFLAGS -c font8x16.c -o _font64.o
gcc $CFLAGS -c font_aa.c  -o _fontaa64.o
gcc $CFLAGS -c font_sub.c -o _fontsub64.o
gcc $CFLAGS -c icons.c    -o _icons64.o
gcc $CFLAGS -c pci.c      -o _pci64.o
gcc $CFLAGS -c bga.c      -o _bga64.o
gcc $CFLAGS -c intel.c    -o _intel64.o
gcc $CFLAGS -c xhci.c     -o _xhci64.o
gcc $CFLAGS -c console.c  -o _console64.o
gcc $CFLAGS -c divmod.c   -o _divmod64.o
gcc $CFLAGS -c gdt64.c    -o _gdt64.o
gcc $CFLAGS -mgeneral-regs-only -c idt.c -o _idt64.o
# The APIC driver replaces the 1981 PIC on machines that no longer wire it.
gcc $CFLAGS -mgeneral-regs-only -c apic.c -o _apic64.o
# virtio-gpu: the one GPU driver we can prove on every build.
gcc $CFLAGS -c virtio_gpu.c -o _vgpu64.o
# reading the processor itself: CPUID, topology, caches, the TSC
gcc $CFLAGS -c cpu.c -o _cpu64.o
# NVMe: real storage, so something survives a power cycle
gcc $CFLAGS -c nvme.c -o _nvme64.o
# the scheduler: more than one thing at a time
gcc $CFLAGS -c sched.c -o _sched64.o
# SMP: waking the other cores
gcc $CFLAGS -c smp.c -o _smp64.o
# I2C-HID: the touchpad
gcc $CFLAGS -c i2c_hid.c -o _i2c64.o
# the input stack: events, modifiers, repeat
gcc $CFLAGS -c input.c -o _input64.o
gcc $CFLAGS -c smp_trampoline64.S -o _smptr64.o
gcc -m64 -c boot64.S -o _boot64.o

ld -m elf_x86_64 -T link64.ld -o kernel64.elf \
   _boot64.o _gen64.o _rt64.o _support64.o _vga64.o _fb64.o _fb3d64.o \
   _font64.o _fontaa64.o _fontsub64.o _icons64.o _pci64.o _bga64.o _intel64.o _xhci64.o \
   _console64.o _divmod64.o _gdt64.o _idt64.o _apic64.o _vgpu64.o _cpu64.o _nvme64.o _sched64.o _smp64.o _smptr64.o _i2c64.o _input64.o

echo "built kernel64.elf"
echo "  undefined symbols: $(nm -u kernel64.elf 2>/dev/null | wc -l)   (0 = no libc, no OS)"
echo "  size:              $(stat -c%s kernel64.elf) bytes"
echo "  arch:              $(readelf -h kernel64.elf | grep -m1 Class | awk '{print $2}')"
