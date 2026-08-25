#!/usr/bin/env bash
# Bounded laboratory tracing for one zlOS investigation.
# Normal telemetry belongs in ZLLOG; instruction tracing is opt-in, capped,
# and runs against a snapshot so it cannot mutate the built USB image.
set -euo pipefail
cd "$(dirname "$0")/../.."

mode=events
seconds=20
max_mib=256
out=""
while [ "$#" -gt 0 ]; do
    case "$1" in
        --events) mode=events ;;
        --instructions) mode=instructions ;;
        --seconds) shift; seconds="${1:?missing seconds}" ;;
        --max-mib) shift; max_mib="${1:?missing MiB}" ;;
        --out) shift; out="${1:?missing output directory}" ;;
        -h|--help)
            echo "usage: $0 [--events|--instructions] [--seconds 1..30] [--max-mib 16..1024] [--out DIR]"
            exit 0 ;;
        *) echo "unknown option: $1" >&2; exit 64 ;;
    esac
    shift
done

case "$seconds" in *[!0-9]*|'') echo "seconds must be an integer" >&2; exit 64;; esac
case "$max_mib" in *[!0-9]*|'') echo "max-mib must be an integer" >&2; exit 64;; esac
[ "$seconds" -ge 1 ] && [ "$seconds" -le 30 ] || { echo "seconds must be 1..30" >&2; exit 64; }
[ "$max_mib" -ge 16 ] && [ "$max_mib" -le 1024 ] || { echo "max-mib must be 16..1024" >&2; exit 64; }

command -v qemu-system-x86_64 >/dev/null || { echo "qemu-system-x86_64 is required" >&2; exit 1; }
[ -f zlOS-usb.img ] || { echo "build zlOS-usb.img first with ./tools/images/mkusb.sh" >&2; exit 1; }
ovmf=/usr/share/OVMF/OVMF_CODE_4M.fd
vars_template=/usr/share/OVMF/OVMF_VARS_4M.fd
[ -f "$ovmf" ] && [ -f "$vars_template" ] || { echo "OVMF is required" >&2; exit 1; }

if [ -z "$out" ]; then out="/tmp/zlos-trace-$(date +%Y%m%d-%H%M%S)"; fi
mkdir -p "$out"
vars=$(mktemp /tmp/zlos-trace-vars-XXXX.fd)
trap 'rm -f "$vars"' EXIT
cp "$vars_template" "$vars"

image_sha=$(sha256sum zlOS-usb.img | awk '{print $1}')
kernel_sha=missing
[ ! -f ZLOS.EFI ] || kernel_sha=$(sha256sum ZLOS.EFI | awk '{print $1}')
qemu_version=$(qemu-system-x86_64 --version | head -1)
if command -v nm >/dev/null && [ -f kernel64.elf ]; then
    nm -n kernel64.elf >"$out/kernel64.symbols"
fi

qemu_debug=int,cpu_reset,guest_errors,unimp
if [ "$mode" = instructions ]; then qemu_debug=in_asm,exec,nochain; fi

# ulimit is a second independent bound if QEMU produces trace faster than the
# wall-clock timeout can stop it. The USB image is opened snapshot-on.
(
    ulimit -f $((max_mib * 1024))
    timeout --signal=TERM --kill-after=2 "$seconds" qemu-system-x86_64 \
        -m 1G -smp 2 -cpu max -accel tcg \
        -drive if=pflash,format=raw,unit=0,readonly=on,file="$ovmf" \
        -drive if=pflash,format=raw,unit=1,file="$vars" \
        -device qemu-xhci,id=xhci \
        -drive format=raw,file=zlOS-usb.img,if=none,id=boot,snapshot=on \
        -device usb-storage,bus=xhci.0,drive=boot \
        -device usb-kbd,bus=xhci.0 -device usb-mouse,bus=xhci.0 \
        -vga std -display none -no-reboot \
        -serial "file:$out/serial.log" \
        -d "$qemu_debug" -D "$out/qemu.log" \
        >"$out/qemu.stdout" 2>"$out/qemu.stderr" || status=$?
    case "${status:-0}" in 0|124|137) : ;; *) exit "$status" ;; esac
)

cat >"$out/README.txt" <<EOF
zlOS bounded QEMU laboratory trace
mode=$mode
seconds=$seconds
max_mib=$max_mib
qemu_debug=$qemu_debug
image=$(realpath zlOS-usb.img)
image_sha256=$image_sha
zlos_efi_sha256=$kernel_sha
qemu_version=$qemu_version
image_writes=snapshot-only
symbols=kernel64.symbols (when kernel64.elf and nm were available)
EOF
echo "$out"
