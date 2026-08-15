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
gcc $CFLAGS -c fb3d.c     -o _fb3d.o
gcc $CFLAGS -c font8x16.c -o _font.o
gcc $CFLAGS -c font_aa.c  -o _fontaa.o
gcc $CFLAGS -c font_sub.c -o _fontsub.o
gcc $CFLAGS -c icons.c    -o _icons.o
gcc $CFLAGS -c pci.c      -o _pci.o
gcc $CFLAGS -c bga.c      -o _bga.o
gcc $CFLAGS -c intel.c    -o _intel.o
gcc $CFLAGS -c xhci.c     -o _xhci.o
gcc $CFLAGS -c console.c  -o _console.o
gcc $CFLAGS -c divmod.c   -o _divmod.o
gcc $CFLAGS -c gdt.c      -o _gdt.o
# interrupt handlers must not touch SSE - -mgeneral-regs-only enforces it
gcc $CFLAGS -mgeneral-regs-only -c idt.c -o _idt.o
# The APIC driver replaces the 1981 PIC on machines that no longer wire it.
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
gcc $CFLAGS -c smp_trampoline.S -o _smptr.o
gcc -m32 -c boot.S -o _boot.o

# No -lgcc. __divdi3/__moddi3 (64-bit division on a 32-bit target) are the
# only things the kernel took from libgcc, and divmod.c now supplies them.
# Nothing GNU is linked into the kernel any more - only gcc-the-tool that
# compiled the C, which nativegen is on track to replace.
ld -m elf_i386 -T link.ld -o kernel.elf _boot.o _gen.o _rt.o _support.o _vga.o _fb.o _fb3d.o _font.o _fontaa.o _fontsub.o _icons.o _pci.o _bga.o _intel.o _xhci.o _console.o _divmod.o _gdt.o _idt.o _apic.o _vgpu.o _cpu.o _nvme.o _sched.o _smp.o _smptr.o _i2c.o _input.o

echo "built kernel.elf"
echo "  undefined symbols: $(nm -u kernel.elf 2>/dev/null | wc -l)   (0 = no libc, no OS)"
echo "  size:              $(stat -c%s kernel.elf) bytes"
echo "  multiboot header:  $(grep -c . < <(od -A d -t x4 -N 8192 kernel.elf | grep -c '1badb002' || true)) found"
