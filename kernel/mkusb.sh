#!/usr/bin/env bash
# mkusb.sh - build zlOS-usb.img, a bootable UEFI USB image.
#
# There is no GRUB here. UEFI firmware executes the tiny removable-media
# witness EFI/BOOT/BOOTX64.EFI, which records entry and immediately chainloads
# the real kernel at EFI/ZLOS/ZLOS.EFI. This wraps both in a GPT-partitioned
# disk with a separate raw ZLLOG flight-recorder partition.
#
#   ./mkusb.sh                  -> zlOS-usb.img
#   ./mkusb.sh --log-mb 512     -> same image with eight times the history
#   ./mkusb.sh /dev/sdX         -> ...and write it to that device (asks first)
#   ./mkusb.sh --boot-next /dev/sdX
#                              -> flash, refresh the GPT-bound UEFI entry, and
#                                 make the next restart boot zlOS once
#
# Everything below runs unprivileged: mtools writes into the FAT image without
# mounting it, so no sudo is needed to BUILD. Only writing to a real device is.
set -euo pipefail
cd "$(dirname "$0")"

BOOT_NEXT=0
LOG_MB=64
TARGET=
while [ "$#" -gt 0 ]; do
    case "$1" in
        --boot-next) BOOT_NEXT=1; shift ;;
        --log-mb)
            [ "$#" -ge 2 ] || { echo "FAIL: --log-mb needs 64..512" >&2; exit 64; }
            LOG_MB=$2; shift 2 ;;
        --*) echo "usage: $0 [--log-mb 64..512] [--boot-next] [/dev/device]" >&2; exit 64 ;;
        *)
            [ -z "$TARGET" ] || { echo "usage: $0 [--log-mb 64..512] [--boot-next] [/dev/device]" >&2; exit 64; }
            TARGET=$1; shift ;;
    esac
done
case "$LOG_MB" in *[!0-9]*|'') echo "FAIL: --log-mb must be a whole MiB value" >&2; exit 64;; esac
[ "$LOG_MB" -ge 64 ] && [ "$LOG_MB" -le 512 ] || {
    echo "FAIL: --log-mb must be between 64 and 512" >&2; exit 64;
}
if [ "$BOOT_NEXT" -eq 1 ] && [ -z "$TARGET" ]; then
    echo "usage: $0 --boot-next [/dev/device]" >&2
    exit 64
fi

IMG=zlOS-usb.img
SIZE_MB=$((68 + LOG_MB))
ESP_START_LBA=2048
ESP_END_LBA=129023       # 62 MiB exactly
ZLLOG_START_LBA=131072   # 64 MiB disk offset, 1 MiB aligned
ZLLOG_END_LBA=$((ZLLOG_START_LBA + LOG_MB * 2048 - 1))
ZLLOG_TYPE_GUID=a2bc51d4-225d-4ad4-8db5-b0095953aa19
PART=_esp.img

cleanup() {
    rm -f "$PART"
}
trap cleanup EXIT

./buildefi.sh >/dev/null
[ -s BOOTX64.EFI ] || { echo "FAIL: BOOTX64.EFI was not built"; exit 1; }
[ -s ZLOS.EFI ] || { echo "FAIL: ZLOS.EFI was not built"; exit 1; }

rm -f "$IMG"
truncate -s "${SIZE_MB}M" "$IMG"

# Use explicit endpoints. The remaining tail holds the backup GPT and padding;
# neither partition silently grows when the image size changes.
sgdisk --clear \
       --new=1:"$ESP_START_LBA":"$ESP_END_LBA" \
       --typecode=1:ef00 \
       --change-name=1:"zlOS EFI" \
       --new=2:"$ZLLOG_START_LBA":"$ZLLOG_END_LBA" \
       --typecode=2:"$ZLLOG_TYPE_GUID" \
       --change-name=2:ZLLOG "$IMG" >/dev/null
sgdisk --verify "$IMG" >/dev/null

# The partition starts at LBA 2048 = 1 MiB. Build the FAT filesystem separately
# and dd it into place, which avoids needing loop devices or root.
rm -f "$PART"
truncate -s 62M "$PART"
# The FAT BPB describes the filesystem at its real GPT offset.  Some firmware
# ignores BPB_HiddSec, but removable-media loaders that cross-check it reject a
# partition image claiming sector 0 when GPT says sector 2048.
mkfs.vfat -F 32 -h "$ESP_START_LBA" -n ZLOS "$PART" >/dev/null

mmd   -i "$PART" ::/EFI ::/EFI/BOOT ::/EFI/ZLOS
mcopy -i "$PART" BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI
mcopy -i "$PART" ZLOS.EFI ::/EFI/ZLOS/ZLOS.EFI

dd if="$PART" of="$IMG" bs=1M seek=1 conv=notrunc status=none
rm -f "$PART"

# Initialize only the exact GUID+label-matched raw partition. The tool records
# GPT bounds, the partition's unique GUID, and the real ZLOS.EFI kernel SHA-256
# in both CRC-protected superblocks. The tiny witness is infrastructure; using
# its hash would make different kernels look identical. The tool refuses an
# unlabelled or unsupported-sized target.
ESP_HASH_BEFORE=$(dd if="$IMG" bs=1M skip=1 count=62 status=none | sha256sum | awk '{print $1}')
../tools/zllog.py init "$IMG" --image-file ZLOS.EFI >/dev/null
ESP_HASH_AFTER=$(dd if="$IMG" bs=1M skip=1 count=62 status=none | sha256sum | awk '{print $1}')
[ "$ESP_HASH_BEFORE" = "$ESP_HASH_AFTER" ] || {
    echo "FAIL: initializing ZLLOG changed the EFI System Partition" >&2
    exit 1
}
sgdisk --verify "$IMG" >/dev/null

echo "built $IMG ($(stat -c%s "$IMG") bytes)"
mdir -i "$IMG@@1M" ::/EFI/BOOT | sed 's/^/  /'
mdir -i "$IMG@@1M" ::/EFI/ZLOS | sed 's/^/  /'
../tools/zllog.py inspect "$IMG" | sed 's/^/  /'

# ---- optionally write it to a real USB stick --------------------------------
if [ -n "$TARGET" ]; then
    DEV="$TARGET"
    [ -b "$DEV" ] || { echo "FAIL: $DEV is not a block device"; exit 1; }
    echo
    echo "TARGET: $DEV"
    lsblk -o NAME,SIZE,TYPE,TRAN,MODEL,MOUNTPOINT "$DEV"
    echo
    echo "This ERASES everything on $DEV."
    printf 'Type the device name again to confirm: '
    read -r confirm
    [ "$confirm" = "$DEV" ] || { echo "aborted"; exit 1; }

    # A previous fixed-size image may have left a stale backup GPT at the
    # physical end of a larger stick. If it survives, sgdisk can see two
    # disagreeing disk identities after the new prefix is copied and firmware
    # may bounce straight back to its boot menu. Clear only the final 1 MiB
    # first; the fresh image below supplies the authoritative primary GPT and
    # its own temporary backup, which --move-second-header then relocates.
    DEV_SECTORS_512=$(sudo blockdev --getsz "$DEV")
    [ "$DEV_SECTORS_512" -gt 2048 ] || { echo "FAIL: target is too small"; exit 1; }
    sudo dd if=/dev/zero of="$DEV" bs=512 \
        seek="$((DEV_SECTORS_512 - 2048))" count=2048 conv=notrunc status=none
    sudo dd if="$IMG" of="$DEV" bs=4M oflag=direct status=progress
# The image may use 64..512 MiB for history while real sticks are larger. Move
# the backup GPT to
    # the physical end, then stamp the journal with the REAL device capacity.
    # Without the second step zllog correctly refuses the stick: its image
    # superblock says 270336 sectors while READ CAPACITY reports millions.
    sudo sgdisk --move-second-header "$DEV" >/dev/null
    sudo sgdisk --verify "$DEV" >/dev/null
    sudo ../tools/zllog.py init "$DEV" --image-file ZLOS.EFI >/dev/null
    sync
    sudo ../tools/zllog.py inspect "$DEV" | sed 's/^/  /'

    if [ "$BOOT_NEXT" -eq 1 ]; then
        command -v efibootmgr >/dev/null || {
            echo "FAIL: --boot-next needs efibootmgr" >&2
            exit 1
        }
        [ -d /sys/firmware/efi/efivars ] || {
            echo "FAIL: --boot-next needs a Linux session booted through UEFI" >&2
            exit 1
        }
        REAL_DEV=$(readlink -f "$DEV")
        [ "$(lsblk -dnro TYPE "$REAL_DEV")" = disk ] || {
            echo "FAIL: --boot-next target is not a whole disk" >&2
            exit 1
        }

        # A GPT device path includes partition 1's unique GUID. mkusb creates a
        # fresh GPT on every flash, so an older `zlOS USB` NVRAM entry points at
        # a GUID that no longer exists and silently falls through to Linux.
        # Remove only entries with our exact label, recreate one from the live
        # target geometry, and verify the new GUID before publishing BootNext.
        sudo efibootmgr | awk '
            /^Boot[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][* ]+zlOS USB[[:space:]]/ {
                print substr($1, 5, 4)
            }' | while read -r old_entry; do
                [ -n "$old_entry" ] && sudo efibootmgr -b "$old_entry" -B >/dev/null
            done
        sudo efibootmgr -c -d "$REAL_DEV" -p 1 -L "zlOS USB" \
            -l '\EFI\BOOT\BOOTX64.EFI' >/dev/null
        NEW_ENTRY=$(sudo efibootmgr | awk '
            /^Boot[0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][0-9A-Fa-f][* ]+zlOS USB[[:space:]]/ {
                if (!found) { print substr($1, 5, 4); found = 1 }
            }')
        [ -n "$NEW_ENTRY" ] || {
            echo "FAIL: efibootmgr did not create the zlOS USB entry" >&2
            exit 1
        }
        PART1=$(lsblk -nrpo NAME,PARTN "$REAL_DEV" | awk '$2 == 1 && !found { print $1; found = 1 }')
        [ -n "$PART1" ] || {
            echo "FAIL: flashed target has no partition 1" >&2
            exit 1
        }
        PART_GUID=$(sudo blkid -s PARTUUID -o value "$PART1")
        [ -n "$PART_GUID" ] || {
            echo "FAIL: flashed partition 1 has no PARTUUID" >&2
            exit 1
        }
        sudo efibootmgr -v | grep -i "^Boot${NEW_ENTRY}" |
            grep -i -F "GPT,${PART_GUID}," >/dev/null || {
            echo "FAIL: zlOS USB entry does not name the flashed partition GUID" >&2
            exit 1
        }
        sudo efibootmgr -n "$NEW_ENTRY" >/dev/null
        echo "firmware entry Boot$NEW_ENTRY matches GPT $PART_GUID; BootNext set."
    fi
    echo "written, device-sized ZLLOG verified. eject and boot it."
fi
