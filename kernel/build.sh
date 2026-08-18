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
# the browser: the tokenizer, the box model and the app. html.c and layout.c
# hold no pixels and no theme, which is what lets hosttest/htmltest.c assert
# the whole box model with no kernel and no boot.
# virtio-net: the network card. Two virtqueues, the same shape as
# virtio_gpu.c's one. Its DMA arena is at 64 MiB - BELOW fb.c's high-RAM map,
# which is full - and virtio_net.c _Static_asserts that against both
# neighbours, which is the check nvme.c does not have.
gcc $CFLAGS -c virtio_net.c -o _vnet.o
# ARP, IPv4 and ICMP. No link driver in it - the link is two function
# pointers, which is what lets hosttest/nettest.c drive the whole stack
# with scripted packets and no machine.
gcc $CFLAGS -c net.c -o _net.o
# DNS, and UDP under it. The rung that reaches the real internet: everything
# below is addressed by number and a person types a name. A response is
# unauthenticated data with backward pointers in it, so the parser is written
# defensively - see the header of dns.c.
gcc $CFLAGS -c dns.c -o _dns.o
# TCP: one connection, client only. The state enum first, the transitions
# second - hosttest/tcptest.c drives all nine states against scripted packet
# sequences, including a SYN-ACK that never comes and a FIN mid-transfer.
gcc $CFLAGS -c tcp.c -o _tcp.o
# HTTP/1.0. Not 1.1: no chunked encoding, no keep-alive, no pipelining.
# A body ends when the connection closes, which the TCP state machine
# already handles because a FIN mid-transfer is one of its gated cases.
gcc $CFLAGS -c http.c -o _http.o
gcc $CFLAGS -c html.c    -o _html.o
gcc $CFLAGS -c layout.c  -o _layout.o
gcc $CFLAGS -c browser.c -o _browser.o
gcc $CFLAGS -c settings.c -o _settings.o
gcc $CFLAGS -c smp_trampoline.S -o _smptr.o
gcc -m32 -c boot.S -o _boot.o

# No -lgcc. __divdi3/__moddi3 (64-bit division on a 32-bit target) are the
# only things the kernel took from libgcc, and divmod.c now supplies them.
# Nothing GNU is linked into the kernel any more - only gcc-the-tool that
# compiled the C, which nativegen is on track to replace.
ld -m elf_i386 -T link.ld -o kernel.elf _boot.o _gen.o _rt.o _support.o _vga.o _fb.o _fb3d.o _font.o _fontaa.o _fontsub.o _icons.o _pci.o _bga.o _intel.o _xhci.o _console.o _divmod.o _gdt.o _idt.o _apic.o _vgpu.o _cpu.o _nvme.o _sched.o _smp.o _smptr.o _i2c.o _input.o _term.o _wm.o _ui.o _wmglue.o _vnet.o _net.o _dns.o _tcp.o _http.o _html.o _layout.o _browser.o _settings.o
echo "built kernel.elf"
echo "  undefined symbols: $(nm -u kernel.elf 2>/dev/null | wc -l)   (0 = no libc, no OS)"
echo "  size:              $(stat -c%s kernel.elf) bytes"
echo "  multiboot header:  $(grep -c . < <(od -A d -t x4 -N 8192 kernel.elf | grep -c '1badb002' || true)) found"
