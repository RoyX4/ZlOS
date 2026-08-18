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
# zlfs: the filesystem. Superblock, a directory of names, contiguous runs.
# Tested without booting anything - hosttest/fstest.c compiles THIS file
# against a RAM disk that can be told to fail a write.
gcc $CFLAGS -c fs.c      -o _fs.o
# the system track's three: the clipboard that makes this one machine rather
# than several programs sharing a screen, window snapping (wm_resize finally
# has a caller), and a toast that goes away by itself.
gcc $CFLAGS -c clip.c    -o _clip.o
gcc $CFLAGS -c snap.c    -o _snap.o
gcc $CFLAGS -c notify.c  -o _notify.o
# the clock. CMOS at 0x70/0x71 - the header drew uptime and called it a time.
gcc $CFLAGS -c rtc.c     -o _rtc.o
# the scheduler: more than one thing at a time
gcc $CFLAGS -c sched.c -o _sched.o
# the program arena: where a program the kernel was NOT built with is
# allowed to put its memory, and the ceiling it cannot cross
gcc $CFLAGS -c arena.c -o _arena.o
# `run`: the command, the window, and every way it declines. Built BEFORE
# anything can execute - the failure modes are the only modes it has.
gcc $CFLAGS -c exec.c -o _exec.o
# SMP: waking the other cores
gcc $CFLAGS -c smp.c -o _smp.o
# I2C-HID: the touchpad
gcc $CFLAGS -c i2c_hid.c -o _i2c.o
# the input stack: events, modifiers, repeat
gcc $CFLAGS -c input.c -o _input.o
# the terminal app: scrollback + the typed-command matcher
gcc $CFLAGS -c term.c -o _term.o
# the compositor and the toolkit. Compiled and linked, but nothing CALLS them
# yet: kernel.zl still ends in the shell's while-loop. That is deliberate -
# the compositor must be optional, because verify.sh boots -kernel -display
# none where px_w() == 0 and there is no framebuffer at all.
gcc $CFLAGS -c wm.c -o _wm.o
gcc $CFLAGS -c ui.c -o _ui.o
# the seam between the compositor and the apps. Its references to kernel.zl's
# app_* functions are WEAK, so this links today - in a kernel where kernel.zl
# has not grown them - and starts working the day it does, with no change here.
gcc $CFLAGS -c wmglue.c -o _wmglue.o
gcc $CFLAGS -c smp_trampoline.S -o _smptr.o
gcc -m32 -c boot.S -o _boot.o

# No -lgcc. __divdi3/__moddi3 (64-bit division on a 32-bit target) are the
# only things the kernel took from libgcc, and divmod.c now supplies them.
# Nothing GNU is linked into the kernel any more - only gcc-the-tool that
# compiled the C, which nativegen is on track to replace.
ld -m elf_i386 -T link.ld -o kernel.elf _boot.o _gen.o _rt.o _support.o _vga.o _fb.o _fb3d.o _font.o _fontaa.o _fontsub.o _icons.o _pci.o _bga.o _intel.o _xhci.o _console.o _divmod.o _gdt.o _idt.o _apic.o _vgpu.o _cpu.o _nvme.o _fs.o _clip.o _snap.o _notify.o _rtc.o _sched.o _arena.o _exec.o _smp.o _smptr.o _i2c.o _input.o _term.o _wm.o _ui.o _wmglue.o

echo "built kernel.elf"
echo "  undefined symbols: $(nm -u kernel.elf 2>/dev/null | wc -l)   (0 = no libc, no OS)"
echo "  size:              $(stat -c%s kernel.elf) bytes"
echo "  multiboot header:  $(grep -c . < <(od -A d -t x4 -N 8192 kernel.elf | grep -c '1badb002' || true)) found"
