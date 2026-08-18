#!/bin/sh
# buildefi.sh - zlOS as a native UEFI application. No GRUB, no bootloader.
#
# The firmware loads EFI/BOOT/BOOTX64.EFI directly, so the kernel IS the boot
# image. Built with clang targeting PE32+ and linked by lld-link as an EFI
# application - no gnu-efi, no edk2, no external dependency at all.
set -e
cd "$(dirname "$0")"

../compile kernel.zl >/dev/null
cp out.c _genefi.c

# -fshort-wchar: UEFI strings are UTF-16
# -mno-red-zone: mandatory - interrupts would clobber it
# -fno-stack-protector / -ffreestanding: no runtime under us
# -w silences everything, which is how FIVE pointer truncations sat in the boot
# path unseen. This target is LLP64: `unsigned long` is 4 bytes, so casting a
# pointer through it silently drops the top half, and shifting such a value by
# 32 is undefined - clang compiled one such shift to a bare `ret`, leaving an
# IDT gate's high half to whatever was in eax. The three warnings below are
# exactly the ones that catch that class, so they are re-enabled after -w (clang
# applies flags left to right) and made fatal. Do not reorder them before -w.
CF="-target x86_64-unknown-windows -ffreestanding -fno-stack-protector \
    -fshort-wchar -mno-red-zone -O2 -DZL_64 -DZL_EFI -w -I.. \
    -Werror=shift-count-overflow -Werror=void-pointer-to-int-cast \
    -Werror=pointer-to-int-cast -Werror=int-to-pointer-cast"

OBJS=""
for f in efi.c _genefi.c ../freestanding/runtime_kernel.c support.c vga.c fb.c fb3d.c font8x16.c font_aa.c \
         font_sub.c icons.c pci.c bga.c intel.c xhci.c console.c divmod.c gdt64.c idt.c apic.c virtio_gpu.c cpu.c nvme.c sched.c smp.c i2c_hid.c input.c wm.c ui.c wmglue.c settings.c term.c; do
    o="_efi_$(basename "$f" .c).o"
    # idt.c and apic.c hold the interrupt handlers, and they must be built
    # -mgeneral-regs-only so a handler can never touch SSE. build.sh and
    # build64.sh have always done this; THIS script did not, and the omission
    # was invisible for as long as the handlers happened to be small enough
    # that the compiler had no reason to reach for XMM. Grow one and it does -
    # and an ISR that clobbers XMM without saving it lands on the zl
    # interpreter, because every zl number is a double. The symptom is the
    # 64-bit boot dying inside setup_idt() with no diagnostic at all, while
    # the 32-bit build is perfectly happy.
    EXTRA=""
    case "$f" in idt.c|apic.c) EXTRA="-mgeneral-regs-only" ;; esac
    # shellcheck disable=SC2086
    clang $CF $EXTRA -DZL_KERNEL_SERIAL -c "$f" -o "$o"
    OBJS="$OBJS $o"
done

# The AP trampoline is assembly, not C, so it is not in the loop above.
clang $CF -c smp_trampoline64.S -o _efi_smptr.o
OBJS="$OBJS _efi_smptr.o"

lld-link -subsystem:efi_application -nodefaultlib -dll \
         -entry:efi_main -out:BOOTX64.EFI $OBJS

echo "built BOOTX64.EFI ($(stat -c%s BOOTX64.EFI) bytes)"
