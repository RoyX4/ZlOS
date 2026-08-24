#!/usr/bin/env bash
# mkiso.sh - build a bootable GRUB ISO for the 32- or 64-bit zlOS kernel.
#
# QEMU's -kernel flag loads a multiboot kernel for us. A real machine has no
# such favour, so the ISO ships GRUB, and GRUB does the multiboot load.
#
#   ./mkiso.sh                 -> zlOS.iso (32-bit multiboot kernel)
#   ./mkiso64.sh               -> zlOS64.iso (owned 32->64 transition)
#   qemu-system-i386 -cdrom zlOS.iso     (test it the way a real boot goes)
#   sudo dd if=zlOS.iso of=/dev/sdX bs=4M status=progress && sync
#
# NOTE, and it matters: this is a BIOS/legacy boot image. Most machines made
# after ~2020 are UEFI-only with no CSM, and will not boot it. Test in QEMU
# first; on real hardware you need a box that can still do legacy boot.
set -euo pipefail
cd "$(dirname "$0")"

# Reproducible ISO metadata. xorriso uses SOURCE_DATE_EPOCH for the volume
# dates, GPT GUID derivation and synthetic file dates. Without it, two builds
# from byte-identical kernel.elf files produced different zlOS.iso hashes.
# The exact dirty source is carried by build-identity.json; the committed epoch
# is only the stable timestamp seed and never a substitute for source identity.
if [ -z "${SOURCE_DATE_EPOCH:-}" ]; then
    SOURCE_DATE_EPOCH=$(git -C .. show -s --format=%ct HEAD)
fi
export SOURCE_DATE_EPOCH
ISO_BUILD_DATE=$(date -u -d "@${SOURCE_DATE_EPOCH}" +%Y%m%d%H%M%S00)

# Select the kernel payload. mkiso64.sh is deliberately a tiny named wrapper
# around this shared packaging path so reproducibility and GRUB configuration
# cannot drift between the 32- and 64-bit ISO routes.
BITS=${ZLOS_ISO_BITS:-32}
case "$BITS" in
    32) BUILD_SCRIPT=./build.sh; KERNEL=kernel.elf; OUTPUT=zlOS.iso ;;
    64) BUILD_SCRIPT=./build64.sh; KERNEL=kernel64.elf; OUTPUT=zlOS64.iso ;;
    *) echo "FAIL: ZLOS_ISO_BITS must be 32 or 64" >&2; exit 2 ;;
esac

# Which zl source to build. Overridable so a 32-bit gate can boot a VARIANT kernel -
# one that asks for a different resolution, say - without editing the tracked
# kernel.zl, which another session may be part-way through.
#
#   ZLOS_SRC=/tmp/kernel-2560.zl ./mkiso.sh
SRC_ARGS=()
[ -n "${ZLOS_SRC:-}" ] && SRC_ARGS=("$ZLOS_SRC")
if [ "$BITS" = 64 ] && [ ${#SRC_ARGS[@]} -ne 0 ]; then
    echo "FAIL: ZLOS_SRC variants are not supported by build64.sh" >&2
    exit 2
fi
"$BUILD_SCRIPT" "${SRC_ARGS[@]+"${SRC_ARGS[@]}"}" >/dev/null

# The mode GRUB asks for. Overridable so a gate can force a resolution the
# kernel would not otherwise be handed - desktop-TODO 0a needs a 2560x1440
# boot to prove the back buffer survives the ThinkPad's panel, and the kernel
# only re-modesets itself when what it was given is under 1900 wide.
#
#   ZLOS_GFXMODE=2560x1440,auto ./mkiso.sh
GFXMODE="${ZLOS_GFXMODE:-1280x720,1280x800,1024x768,auto}"

rm -rf _iso
mkdir -p _iso/boot/grub
cp "$KERNEL" "_iso/boot/$KERNEL"

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

menuentry "zlOS ${BITS}-bit - a kernel written in zl" {
    multiboot /boot/$KERNEL
    boot
}

menuentry "zlOS ${BITS}-bit (serial console on COM1)" {
    multiboot /boot/$KERNEL console=serial
    boot
}
EOF

# grub-mkrescue supplies later xorriso options which can override
# SOURCE_DATE_EPOCH. Pass all three reproducibility controls explicitly so the
# outer ISO, its directory records and the hybrid GPT use the same stable seed.
REPRO_TIME_SO=$(mktemp "${TMPDIR:-/tmp}/zlos-repro-time.XXXXXX.so")
trap 'rm -f "${REPRO_TIME_SO}"; rm -rf _iso' EXIT
gcc -shared -fPIC -O2 -Wall -Wextra -Werror \
    -o "${REPRO_TIME_SO}" tools/reproducible_time.c

LD_PRELOAD="${REPRO_TIME_SO}${LD_PRELOAD:+:${LD_PRELOAD}}" grub-mkrescue -o "$OUTPUT" \
    --modification-date="${ISO_BUILD_DATE}" \
    --set_all_file_dates "${ISO_BUILD_DATE}" \
    --gpt_disk_guid modification-date \
    _iso 2>/dev/null

rm -rf _iso
rm -f "${REPRO_TIME_SO}"
trap - EXIT
echo "built $OUTPUT ($(stat -c%s "$OUTPUT") bytes)"
echo
echo "test it:      qemu-system-x86_64 -cdrom $OUTPUT"
echo "write a USB:  sudo dd if=$OUTPUT of=/dev/sdX bs=4M status=progress && sync"
echo
echo "Hybrid image: boots on BOTH legacy BIOS and 64-bit UEFI (efi/boot/bootx64.efi)."
