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
CF="-target x86_64-unknown-windows -ffreestanding -fno-stack-protector \
    -fshort-wchar -mno-red-zone -O2 -DZL_64 -DZL_EFI -w -I.."

OBJS=""
for f in efi.c _genefi.c ../freestanding/runtime_kernel.c support.c vga.c fb.c fb3d.c font8x16.c font_aa.c \
         font_sub.c icons.c pci.c bga.c intel.c xhci.c console.c divmod.c gdt64.c idt.c apic.c virtio_gpu.c cpu.c nvme.c sched.c; do
    o="_efi_$(basename "$f" .c).o"
    # shellcheck disable=SC2086
    clang $CF -DZL_KERNEL_SERIAL -c "$f" -o "$o"
    OBJS="$OBJS $o"
done

lld-link -subsystem:efi_application -nodefaultlib -dll \
         -entry:efi_main -out:BOOTX64.EFI $OBJS

echo "built BOOTX64.EFI ($(stat -c%s BOOTX64.EFI) bytes)"
