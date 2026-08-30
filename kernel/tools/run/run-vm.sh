#!/usr/bin/env bash
# run-vm.sh - boot zlOS.iso in a VM, the same way your laptop would.
#
#   ./run-vm.sh          UEFI  (what a ThinkPad X1 Carbon Gen 8 actually does)
#   ./run-vm.sh --bios   legacy BIOS, for comparison
#
# A window opens. Click into it and TYPE - the PS/2 keyboard driver is real,
# so this is the same input path as on bare metal. Ctrl-Alt-G releases the
# mouse, Ctrl-Alt-Q quits.
set -euo pipefail
cd "$(dirname "$0")/../.."

OVMF_CODE=/usr/share/OVMF/OVMF_CODE_4M.fd
OVMF_VARS=/usr/share/OVMF/OVMF_VARS_4M.fd

[ -f zlOS.iso ] || ./tools/images/mkiso.sh >/dev/null

if [ "${1:-}" = "--bios" ]; then
    echo "booting zlOS.iso - legacy BIOS"
    exec qemu-system-i386 -cdrom zlOS.iso -m 1G -no-reboot "${@:2}"
fi

[ -f "$OVMF_CODE" ] || { echo "no OVMF firmware. apt install ovmf"; exit 1; }

VARS=$(mktemp /tmp/zlos-vars.XXXX.fd)
cp "$OVMF_VARS" "$VARS"
trap 'rm -f "$VARS"' EXIT

cat <<'EOT'
booting zlOS.iso under UEFI - the same firmware path your laptop uses.

  Click into the window, then type:
    h        help
    20f      fib(20)
    10s      sum of squares
    m        poke a byte into video memory, read it back
    c        clear
    q        halt

  Ctrl-Alt-G releases the mouse.  Ctrl-Alt-Q quits QEMU.

EOT

exec qemu-system-x86_64 \
    -drive if=pflash,format=raw,unit=0,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,unit=1,file="$VARS" \
    -cdrom zlOS.iso -m 1G -no-reboot "$@"
