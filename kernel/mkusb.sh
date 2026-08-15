#!/usr/bin/env bash
# mkusb.sh - build zlOS-usb.img, a bootable UEFI USB image.
#
# There is no GRUB and no bootloader here at all. UEFI firmware looks for
# EFI/BOOT/BOOTX64.EFI on a FAT filesystem and executes it, so zlOS IS the boot
# image: buildefi.sh produces a PE32+ EFI application and this wraps it in a
# GPT-partitioned disk with an EFI System Partition.
#
#   ./mkusb.sh                  -> zlOS-usb.img
#   ./mkusb.sh /dev/sdX         -> ...and write it to that device (asks first)
#
# Everything below runs unprivileged: mtools writes into the FAT image without
# mounting it, so no sudo is needed to BUILD. Only writing to a real device is.
set -euo pipefail
cd "$(dirname "$0")"

IMG=zlOS-usb.img
SIZE_MB=64

./buildefi.sh >/dev/null
[ -s BOOTX64.EFI ] || { echo "FAIL: BOOTX64.EFI was not built"; exit 1; }

rm -f "$IMG"
truncate -s "${SIZE_MB}M" "$IMG"

# GPT with a single EFI System Partition. sgdisk is scriptable; parted is not
# reliably so across versions.
sgdisk --clear \
       --new=1:2048:0 \
       --typecode=1:ef00 \
       --change-name=1:"zlOS EFI" "$IMG" >/dev/null

# The partition starts at LBA 2048 = 1 MiB. Build the FAT filesystem separately
# and dd it into place, which avoids needing loop devices or root.
PART=_esp.img
rm -f "$PART"
truncate -s "$((SIZE_MB - 2))M" "$PART"
mkfs.vfat -F 32 -n ZLOS "$PART" >/dev/null

mmd   -i "$PART" ::/EFI ::/EFI/BOOT
mcopy -i "$PART" BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI

dd if="$PART" of="$IMG" bs=1M seek=1 conv=notrunc status=none
rm -f "$PART"

echo "built $IMG ($(stat -c%s "$IMG") bytes)"
mdir -i "$IMG@@1M" ::/EFI/BOOT | sed 's/^/  /'

# ---- optionally write it to a real USB stick --------------------------------
if [ $# -ge 1 ]; then
    DEV="$1"
    [ -b "$DEV" ] || { echo "FAIL: $DEV is not a block device"; exit 1; }
    echo
    echo "TARGET: $DEV"
    lsblk -o NAME,SIZE,TYPE,TRAN,MODEL,MOUNTPOINT "$DEV"
    echo
    echo "This ERASES everything on $DEV."
    printf 'Type the device name again to confirm: '
    read -r confirm
    [ "$confirm" = "$DEV" ] || { echo "aborted"; exit 1; }
    sudo dd if="$IMG" of="$DEV" bs=4M oflag=direct status=progress
    sync
    echo "written. eject and boot it."
fi
