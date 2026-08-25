#!/usr/bin/env bash
# test-real-xhci.sh - run zlOS against a REAL Intel xHCI controller, with no
# USB stick and no reboot.
#
# This machine has two xHCI controllers:
#
#     00:14.0   Intel 400-series PCH   <- the one the host is using
#     2d:00.0   Thunderbolt 3 (JHL6540) <- its own IOMMU group, nothing on it
#
# The Thunderbolt one sits alone in IOMMU group 16 with only its own bridge,
# and both of its root hubs are empty. That makes it safe to hand to a VM: the
# host loses two USB-C ports for the duration and nothing else.
#
# Why this is worth doing: QEMU's emulated xHCI asks for no scratchpad buffers,
# has no BIOS handoff to perform, and gets a BAR below 4 GiB. Real Intel
# silicon does all three differently, and those are exactly the paths that
# cannot be exercised any other way short of booting the machine.
#
#   sudo ./test-real-xhci.sh              # run it
#   sudo ./test-real-xhci.sh --restore    # give the controller back
#
# Plug a USB keyboard into a USB-C port first, or there is nothing to find.
set -euo pipefail
cd "$(dirname "$0")/../.."

DEV=0000:2d:00.0
VID_DID="8086 15d4"

need_root() { [ "$(id -u)" -eq 0 ] || { echo "run me with sudo"; exit 1; }; }

restore() {
    echo "returning $DEV to the host..."
    if [ -e "/sys/bus/pci/devices/$DEV/driver" ]; then
        echo "$DEV" > "/sys/bus/pci/devices/$DEV/driver/unbind" 2>/dev/null || true
    fi
    echo "$DEV" > /sys/bus/pci/drivers/xhci_hcd/bind 2>/dev/null || true
    echo "done - the USB-C ports are back."
}

if [ "${1:-}" = "--restore" ]; then need_root; restore; exit 0; fi
need_root

[ -f zlOS-usb.img ] || { echo "build it first: ./tools/images/mkusb.sh"; exit 1; }

modprobe vfio-pci

# Hand the controller to vfio-pci. Reversible: --restore puts it back, and so
# does a reboot.
if [ -e "/sys/bus/pci/devices/$DEV/driver" ]; then
    echo "$DEV" > "/sys/bus/pci/devices/$DEV/driver/unbind"
fi
echo "$VID_DID" > /sys/bus/pci/drivers/vfio-pci/new_id 2>/dev/null || true
echo "$DEV" > /sys/bus/pci/drivers/vfio-pci/bind 2>/dev/null || true

trap restore EXIT

VARS=$(mktemp /tmp/zlos-vars-XXXX.fd)
cp /usr/share/OVMF/OVMF_VARS_4M.fd "$VARS"

echo
echo "booting zlOS with the real Thunderbolt xHCI passed through..."
echo "watch for: 'USB: HID keyboard live on slot N'"
echo

qemu-system-x86_64 -m 1G -machine q35,accel=kvm -cpu host \
    -drive if=pflash,format=raw,unit=0,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
    -drive if=pflash,format=raw,unit=1,file="$VARS" \
    -drive format=raw,file=zlOS-usb.img,if=none,id=zl \
    -device qemu-xhci,id=boot -device usb-storage,bus=boot.0,drive=zl \
    -device vfio-pci,host=$DEV \
    -serial mon:stdio -display none -no-reboot

rm -f "$VARS"
