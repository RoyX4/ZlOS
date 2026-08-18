#!/usr/bin/env bash
# verify-efi.sh - the gate that was missing: does zlOS boot as its OWN UEFI
# application?
#
# The other three gates all boot the 32-bit kernel:
#   verify.sh      -kernel kernel.elf, BIOS
#   verify-raw.sh  our 512-byte bootloader, BIOS
#   verify-iso.sh  zlOS.iso - and its "UEFI" case is GRUB's bootx64.efi
#                  multiboot-loading that same 32-bit kernel. Check for
#                  yourself: the EFI binary inside zlOS.iso has 451 GRUB
#                  strings in it and zero zlOS ones.
#
# So NOTHING covered kernel/efi.c, the 64-bit build, or the path the ThinkPad
# actually takes. That is not theoretical: a change to the interrupt handlers
# killed the 64-bit boot dead inside setup_idt(), and all three gates stayed
# green through it because none of them run that code. The regression was found
# by hand, hours later.
#
# This boots zlOS-usb.img - our BOOTX64.EFI, no GRUB anywhere - under OVMF,
# and checks it reaches the shell AS a UEFI application with a real
# framebuffer.
#
# -vga std, NOT -vga none: OVMF publishes no GOP this kernel can use for a bare
# virtio-gpu-pci, so efi.c finds no framebuffer and falls back to a VGA text
# console that does not exist under UEFI. The screen stays black while the
# serial log looks healthy. kernel/probe-uefi.py measures which devices work.
set -uo pipefail
cd "$(dirname "$0")"

OVMF_CODE=/usr/share/OVMF/OVMF_CODE_4M.fd
OVMF_VARS=/usr/share/OVMF/OVMF_VARS_4M.fd
fail=0

command -v qemu-system-x86_64 >/dev/null || { echo "skip: no qemu-system-x86_64"; exit 0; }
[ -f "$OVMF_CODE" ] || { echo "skip: no OVMF firmware (apt install ovmf)"; exit 0; }

./mkusb.sh >/dev/null 2>&1 || { echo "FAIL: the UEFI image did not build"; exit 1; }

VARS=$(mktemp); cp "$OVMF_VARS" "$VARS"
LOG=$(mktemp)

# Wait for the expected OUTPUT, never for a fixed number of seconds. A gate
# that fails because the host was busy costs a bisect every time it lies
# (kernel/CLAUDE.md).
CEILING=180

# KVM when the box has it, TCG when it does not. This gate is the one that most
# needs to run somewhere other than Roy's laptop - it is the only check on the
# path real hardware takes - and a CI runner has no /dev/kvm, where `-accel kvm`
# fails to start QEMU at all and reports as "the kernel never started".
#
# `-cpu host` is meaningless without KVM, so it moves with the accelerator.
# TCG is several times slower; that is safe here only because the loop below
# polls for the marker instead of racing a fixed clock.
if [ -r /dev/kvm ] && [ -w /dev/kvm ]; then
    ACCEL=(-cpu host -accel kvm)
else
    echo "  note  no /dev/kvm - falling back to TCG (slower, still correct)"
    ACCEL=(-cpu max -accel tcg)
fi

timeout "$CEILING" qemu-system-x86_64 \
    -m 1G -smp 2 "${ACCEL[@]}" \
    -drive if=pflash,format=raw,unit=0,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,unit=1,file="$VARS" \
    -device qemu-xhci,id=xhci \
    -drive format=raw,file=zlOS-usb.img,if=none,id=boot \
    -device usb-storage,bus=xhci.0,drive=boot \
    -vga std -display none -no-reboot \
    -serial "file:$LOG" >/dev/null 2>&1 &
QPID=$!
for _ in $(seq $((CEILING * 2))); do
    grep -q "ready\." "$LOG" 2>/dev/null && break
    kill -0 "$QPID" 2>/dev/null || break
    sleep 0.5
done
sleep 1                      # let the line after the prompt land
kill "$QPID" 2>/dev/null; wait "$QPID" 2>/dev/null
tr -d '\r' < "$LOG" > "$LOG.c" && mv "$LOG.c" "$LOG"

if ! grep -q "zlOS starting" "$LOG"; then
    echo "  FAIL  the kernel never started"; fail=1
elif ! grep -q "ready\." "$LOG"; then
    echo "  FAIL  it started but never reached the prompt"
    echo "        last thing it managed to say:"
    grep "OK  \]" "$LOG" | tail -1 | sed 's/^/          /'
    fail=1
else
    # It has to be OUR EFI application, not GRUB pretending. loader()==2 prints
    # this exact line; a multiboot handoff or our raw bootloader print others,
    # and seeing one of those here means the image is not what we think it is.
    if grep -q "UEFI application - no GRUB, no bootloader" "$LOG"; then
        echo "  ok    booted as a UEFI application, 64-bit from the start"
    else
        echo "  FAIL  reached the shell but NOT as a UEFI application:"
        grep -E "OK  \] (UEFI|multiboot|our bootloader)" "$LOG" | head -1 | sed 's/^/          /'
        fail=1
    fi
    # UEFI has no VGA text mode. Falling back to one means a black screen on
    # real hardware while the serial log still looks perfectly healthy - the
    # most misleading failure this kernel can produce.
    if grep -q "VGA text console" "$LOG"; then
        echo "  FAIL  fell back to VGA text - the panel would stay black"; fail=1
    else
        echo "  ok    $(grep -oE 'framebuffer console, [0-9]+x[0-9]+' "$LOG" | head -1)"
    fi
fi

rm -f "$VARS" "$LOG"
[ "$fail" -eq 0 ] && echo "EFI gate green" || echo "EFI gate FAILED"
exit $fail
