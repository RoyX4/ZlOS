#!/usr/bin/env bash
# install-esp.sh - boot zlOS on this machine with NO USB stick.
#
# The EFI System Partition is already mounted at /boot/efi with ~975 MB free.
# Dropping our EFI application there under its own name lets the firmware boot
# menu (F12 on a ThinkPad) offer zlOS directly. Nothing about GRUB, the default
# boot order, or any existing file is touched, so removing the one file we add
# undoes it completely.
#
#   sudo ./install-esp.sh            # install
#   sudo ./install-esp.sh --remove   # take it back off
set -euo pipefail
cd "$(dirname "$0")/../.."

ESP=/boot/efi
DEST="$ESP/EFI/zlOS"

[ "$(id -u)" -eq 0 ] || { echo "run me with sudo"; exit 1; }
mountpoint -q "$ESP" || { echo "FAIL: $ESP is not mounted"; exit 1; }

if [ "${1:-}" = "--remove" ]; then
    rm -rf "$DEST"
    echo "removed $DEST - nothing else was ever changed."
    exit 0
fi

./buildefi.sh >/dev/null
mkdir -p "$DEST"
cp BOOTX64.EFI "$DEST/zlOS.efi"
sync

echo "installed $DEST/zlOS.efi ($(stat -c%s "$DEST/zlOS.efi") bytes)"
echo
echo "to boot it:  reboot, hold F12, pick the zlOS entry."
echo "if the firmware will not list it, add a boot entry with:"
echo "  sudo efibootmgr --create --disk /dev/nvme0n1 --part 5 \\"
echo "       --loader '\\EFI\\zlOS\\zlOS.efi' --label 'zlOS'"
echo
echo "to undo:     sudo ./install-esp.sh --remove"
