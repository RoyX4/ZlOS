#!/usr/bin/env bash
# verify-efi.sh - boot BOOTX64.EFI, the NATIVE UEFI application.
#
# THIS SCRIPT DID NOT EXIST, and three documents told you to run it.
# PLATFORM-PROMPT.md §1.4 lists it as a required gate, item 8's gate is
# literally "verify-efi.sh green", and desktop-wiring.md tells you to run it
# after wiring the compositor in. What existed was:
#
#   verify.sh       qemu-system-i386 -kernel kernel.elf     32-bit BIOS
#   verify-raw.sh   our own 512-byte bootloader
#   verify-iso.sh   mkiso.sh, then BIOS *and* UEFI - but both through GRUB
#
# and buildefi.sh, which builds BOOTX64.EFI and which NOTHING BOOTED.
#
# That gap is not cosmetic. verify-iso.sh's UEFI leg boots the *multiboot*
# kernel that build64.sh produces, loaded by GRUB under OVMF. efi.c is not in
# that binary. So the file containing the T-11 truncation - and the whole
# firmware-handoff path the ThinkPad actually takes, with no bootloader of any
# kind in between - was compiled by the build system and executed by nothing.
#
# It is also the only gate where fb.c's SIMD is live on the path that ships:
# clang, LLP64, __SSE2__ baseline.
#
# No root and no loop devices: mtools writes the FAT image, the same way
# mkusb.sh does.
set -uo pipefail
cd "$(dirname "$0")"

OVMF_CODE=/usr/share/OVMF/OVMF_CODE_4M.fd
OVMF_VARS=/usr/share/OVMF/OVMF_VARS_4M.fd

command -v qemu-system-x86_64 >/dev/null || { echo "skip: no qemu-system-x86_64"; exit 0; }
command -v mformat >/dev/null || { echo "skip: no mtools (apt install mtools)"; exit 0; }
[ -f "$OVMF_CODE" ] || { echo "skip: no OVMF firmware (apt install ovmf)"; exit 0; }

./buildefi.sh >/dev/null 2>&1 || { echo "FAIL: BOOTX64.EFI did not build"; exit 1; }
[ -s BOOTX64.EFI ] || { echo "FAIL: BOOTX64.EFI is empty"; exit 1; }

TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
ESP="$TMP/esp.img"
LOG="$TMP/serial.log"
VARS="$TMP/vars.fd"
cp "$OVMF_VARS" "$VARS"

# The firmware looks for EFI/BOOT/BOOTX64.EFI on a FAT filesystem and executes
# it. That is the whole boot protocol - zlOS IS the boot image.
truncate -s 64M "$ESP"
mformat -i "$ESP" -F -v ZLOS ::
mmd   -i "$ESP" ::/EFI ::/EFI/BOOT
mcopy -i "$ESP" BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI

# Wait for the marker, never a fixed wall-clock time. Under OVMF the firmware
# alone takes a couple of seconds before our first byte, and how long depends
# on host load - a timeout here would make the gate a function of what else is
# running, which is what verify-raw.sh was rewritten to stop doing.
CEILING=180
timeout "$CEILING" qemu-system-x86_64 \
    -drive if=pflash,format=raw,unit=0,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,unit=1,file="$VARS" \
    -drive file="$ESP",format=raw,if=ide \
    -m 512 -display none -no-reboot \
    -serial "file:$LOG" >/dev/null 2>&1 &
QPID=$!
for _ in $(seq $((CEILING * 2))); do
    grep -q "compositor:" "$LOG" 2>/dev/null && break
    kill -0 "$QPID" 2>/dev/null || break
    sleep 0.5
done
kill "$QPID" 2>/dev/null; wait "$QPID" 2>/dev/null
tr -d '\r' < "$LOG" > "$LOG.c" 2>/dev/null && mv "$LOG.c" "$LOG"

fail=0
want() {           # want <string> <message>
    grep -q "$1" "$LOG" 2>/dev/null || { echo "  FAIL  $2"; fail=1; }
}

want "zlOS starting"        "the kernel never started - firmware did not run BOOTX64.EFI"
# loader() == 2 is the UEFI-application path specifically, as opposed to a
# multiboot handoff. If this line says anything else we booted the wrong thing
# and the whole point of this gate is lost.
want "UEFI application"     "booted, but not as a UEFI application - wrong binary?"
want "framebuffer console"  "no framebuffer - GOP handoff failed"
want "ready\."              "started but never reached the prompt"
want "compositor: [1-9]"    "reached the prompt but the compositor opened no window"

# THE T-11 CHECK. fb_setup prints where the back buffer lives, and it computes
# that from the address the GOP handed over. A truncated address does not just
# move the picture - it takes the whole map with it.
if grep -q "REFUSED the mode" "$LOG" 2>/dev/null; then
    echo "  FAIL  fb_setup refused the mode:"
    grep "REFUSED the mode" "$LOG" | sed 's/^/          /'
    fail=1
fi

if [ "$fail" -eq 0 ]; then
    echo "  ok    $(grep -oE 'framebuffer console, [0-9]+x[0-9]+' "$LOG" | head -1), $(grep -oE 'compositor: [0-9]+ windows' "$LOG" | head -1)"
    echo "EFI application gate green"
else
    echo "EFI application gate FAILED"
    echo "--- serial tail ---"
    tail -25 "$LOG" 2>/dev/null | sed 's/^/  /'
fi
exit $fail
