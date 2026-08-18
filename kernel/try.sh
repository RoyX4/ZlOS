#!/usr/bin/env bash
# try.sh - run zlOS in a window, right now, with everything plugged in.
#
#   ./try.sh            the full machine: 4 cores, GPU, USB keyboard and
#                       mouse, an NVMe disk and a USB stick
#   ./try.sh uefi       the same, but booted the way the real laptop boots -
#                       pure UEFI from the USB image, no BIOS, no GRUB
#   ./try.sh serial     no window; drive it over the terminal instead
#
# Everything here is emulated hardware that zlOS drives with its OWN drivers:
# the display through virtio-gpu, the keyboard and mouse through our xHCI and
# HID stack, the disk through our NVMe driver. Nothing is using a BIOS call.
set -euo pipefail
cd "$(dirname "$0")"

MODE="${1:-gui}"
DISK=/tmp/zlos-nvme.img
STICK=/tmp/zlos-stick.img

[ -f "$DISK" ]  || qemu-img create -f raw "$DISK" 64M >/dev/null
[ -f "$STICK" ] || qemu-img create -f raw "$STICK" 32M >/dev/null

COMMON=(
  -m 1G -smp 4 -cpu host -accel kvm
  -drive "file=$DISK,if=none,id=nvm,format=raw"  -device nvme,serial=zlos001,drive=nvm
  -drive "file=$STICK,if=none,id=stick,format=raw"
  -device qemu-xhci,id=xhci
  -device usb-storage,bus=xhci.0,drive=stick
  -device usb-kbd,bus=xhci.0
  -device usb-mouse,bus=xhci.0
  # THE NETWORK CARD. Without these two flags the browser opens and cannot
  # fetch anything, and `N` reports "no virtio-net device on the PCI bus" -
  # which reads as a broken driver rather than an unplugged machine.
  # virtio_net.c matches PCI 1af4:1041 and 1af4:1000 only, so QEMU's default
  # e1000 is not enough; these are the exact flags net_gate() prints when it
  # cannot find a card.
  -netdev user,id=n0
  -device virtio-net-pci,netdev=n0
  -no-reboot
)

case "$MODE" in
uefi)
  ./mkusb.sh >/dev/null
  VARS=$(mktemp /tmp/zlos-vars-XXXX.fd)
  cp /usr/share/OVMF/OVMF_VARS_4M.fd "$VARS"
  echo "booting the UEFI image - the same path the real laptop takes"
  exec qemu-system-x86_64 "${COMMON[@]}" \
    -drive if=pflash,format=raw,unit=0,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
    -drive if=pflash,format=raw,unit=1,file="$VARS" \
    -drive format=raw,file=zlOS-usb.img,if=none,id=boot \
    -device usb-storage,bus=xhci.0,drive=boot \
    -vga none -device virtio-gpu-pci,xres=1280,yres=800 \
    -display gtk -serial mon:stdio
  ;;
serial)
  ./build.sh >/dev/null
  echo "serial only - type here, 'h' for help, 'q' to halt"
  exec qemu-system-i386 -kernel kernel.elf "${COMMON[@]}" \
    -display none -serial mon:stdio
  ;;
*)
  ./mkiso.sh >/dev/null
  echo "zlOS - press h for help. Things worth trying:"
  echo "    h  help          w  draggable windows    v  spinning 3D cube"
  echo "    k  PCI + GPU     u  USB bus              o  NVMe disk"
  echo "    y  virtio-gpu    *  wake all 4 cores     +  multitasking"
  echo "    =  input events  z  the CPU              g  snake"
  echo
  # Boot the ISO, not -kernel. QEMU's own multiboot loader never fills in the
  # framebuffer tag - it says so out loud: "multiboot knows VBE. we don't" - so
  # console_init() never reaches fb_setup(), and every graphical demo listed
  # above answers "needs the framebuffer console" and refuses. GRUB, inside the
  # ISO, does supply that tag. Keep the standard VGA it draws through, and keep
  # virtio-gpu-pci alongside it so 'y' still has a device of its own to drive.
  exec qemu-system-i386 -cdrom zlOS.iso "${COMMON[@]}" \
    -device virtio-gpu-pci,xres=1280,yres=800 \
    -display gtk -serial mon:stdio
  ;;
esac
