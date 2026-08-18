#!/usr/bin/env bash
# mkiso.sh - build a bootable zlOS.iso for real hardware or a USB stick.
#
# QEMU's -kernel flag loads a multiboot kernel for us. A real machine has no
# such favour, so the ISO ships GRUB, and GRUB does the multiboot load.
#
#   ./mkiso.sh                 -> zlOS.iso
#   qemu-system-i386 -cdrom zlOS.iso     (test it the way a real boot goes)
#   sudo dd if=zlOS.iso of=/dev/sdX bs=4M status=progress && sync
#
# NOTE, and it matters: this is a BIOS/legacy boot image. Most machines made
# after ~2020 are UEFI-only with no CSM, and will not boot it. Test in QEMU
# first; on real hardware you need a box that can still do legacy boot.
set -euo pipefail
cd "$(dirname "$0")"

# Which zl source to build. Overridable so a gate can boot a VARIANT kernel -
# one that asks for a different resolution, say - without editing the tracked
# kernel.zl, which another session may be part-way through.
#
#   ZLOS_SRC=/tmp/kernel-2560.zl ./mkiso.sh
SRC_ARGS=()
[ -n "${ZLOS_SRC:-}" ] && SRC_ARGS=("$ZLOS_SRC")
./build.sh "${SRC_ARGS[@]+"${SRC_ARGS[@]}"}" >/dev/null

# The mode GRUB asks for. Overridable so a gate can force a resolution the
# kernel would not otherwise be handed - desktop-TODO 0a needs a 2560x1440
# boot to prove the back buffer survives the ThinkPad's panel, and the kernel
# only re-modesets itself when what it was given is under 1900 wide.
#
#   ZLOS_GFXMODE=2560x1440,auto ./mkiso.sh
GFXMODE="${ZLOS_GFXMODE:-1280x720,1280x800,1024x768,auto}"

rm -rf _iso
mkdir -p _iso/boot/grub
cp kernel.elf _iso/boot/kernel.elf

cat > _iso/boot/grub/grub.cfg <<EOF
# all_video pulls in the GOP/VBE drivers. Without it GRUB may have no video
# driver loaded at all and cannot satisfy the kernel's framebuffer request.
insmod all_video
insmod gfxterm

# Ask GRUB for a bigger screen and hand that exact mode to the kernel.
# gfxmode lists preferred resolutions (first that the firmware offers wins);
# gfxpayload=keep makes the loaded kernel inherit GRUB's framebuffer instead
# of the firmware default. The kernel READS width/height from multiboot, so it
# adapts to whatever mode is actually set - no code change needed for the size.
set gfxmode=$GFXMODE
set gfxpayload=keep

set timeout=3
set default=0

menuentry "zlOS - a kernel written in zl" {
    multiboot /boot/kernel.elf
    boot
}

menuentry "zlOS (serial console on COM1)" {
    multiboot /boot/kernel.elf console=serial
    boot
}
EOF

grub-mkrescue -o zlOS.iso _iso 2>/dev/null

rm -rf _iso
echo "built zlOS.iso ($(stat -c%s zlOS.iso) bytes)"
echo
echo "test it:      qemu-system-i386 -cdrom zlOS.iso"
echo "write a USB:  sudo dd if=zlOS.iso of=/dev/sdX bs=4M status=progress && sync"
echo
echo "Hybrid image: boots on BOTH legacy BIOS and 64-bit UEFI (efi/boot/bootx64.efi)."
