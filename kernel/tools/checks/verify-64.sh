#!/usr/bin/env bash
# Boot the exact standalone kernel64.elf through GRUB under both BIOS and UEFI.
# GRUB enters the Multiboot kernel in 32-bit protected mode; boot64.S owns the
# page tables, PAE/EFER/paging sequence and far jump into 64-bit long mode.
set -uo pipefail
KERNEL_ROOT=$(cd "$(dirname "$0")/../.." && pwd)
cd "$KERNEL_ROOT"
. tools/checks/qemu-crash.sh

OVMF_CODE=/usr/share/OVMF/OVMF_CODE_4M.fd
OVMF_VARS=/usr/share/OVMF/OVMF_VARS_4M.fd
CEILING=${ZLOS_BOOT_CEILING:-360}
ORIGIN="multiboot handoff, then OUR jump into 64-bit long mode"
fail=0

for source in boot/boot64.S boot/smp_trampoline64.S; do
    if ! grep -Fq '$((1 << 8) | (1 << 11))' "$source"; then
        echo "FAIL: $source must enable EFER.LME and EFER.NXE together"
        exit 1
    fi
done

command -v qemu-system-x86_64 >/dev/null || {
    echo "skip: no qemu-system-x86_64"; exit 0;
}
if [ "${ZLOS_SKIP_BUILD:-0}" = 1 ]; then
    [ -s zlOS64.iso ] || { echo "FAIL: ZLOS_SKIP_BUILD=1 but zlOS64.iso is missing"; exit 1; }
else
    ./tools/images/mkiso64.sh >/dev/null 2>&1 || {
        echo "FAIL: 64-bit multiboot ISO did not build"; exit 1;
    }
fi

if [ -r /dev/kvm ] && [ -w /dev/kvm ]; then
    ACCEL=(-cpu host -accel kvm)
else
    echo "  note  no /dev/kvm - falling back to TCG (slower, still correct)"
    ACCEL=(-cpu max -accel tcg)
fi

boot_until() { # $1 log, remaining arguments are qemu arguments
    local log="$1"; shift
    local serial_socket="$log.sock"
    rm -f "$serial_socket"
    timeout "$CEILING" "$@" \
        -chardev "socket,id=ser0,path=$serial_socket,server=on,wait=on" \
        -serial chardev:ser0 -display none -no-reboot >/dev/null 2>&1 &
    local pid=$!
    # Send only after the kernel is ready. Preloading stdin works on the short
    # BIOS path but OVMF/GRUB consumes or drops it during firmware startup.
    python3 ./tools/serial_command.py \
        --socket "$serial_socket" --log "$log" --ceiling "$CEILING" \
        --ready "ready." --send $'fib 20\r' \
        --expect "compositor:" --expect "6765"
    local result=$?
    kill "$pid" 2>/dev/null
    wait "$pid" 2>/dev/null
    local qstatus=$?
    qemu_crashed "$qstatus" || true
    rm -f "$serial_socket"
    return "$result"
}

check() { # $1 label, $2 log
    local label="$1" log="$2"
    local manifest_marker build_id build_head build_dirty
    manifest_marker=$(python3 ./tools/generators/gen-app-manifest.py --marker)
    build_id=$(python3 -c 'import json; print(json.load(open("metadata/build-identity.json"))["identity_sha256"])')
    build_head=$(python3 -c 'import json; print(json.load(open("metadata/build-identity.json"))["git"]["head"])')
    build_dirty=$(python3 -c 'import json; print(1 if json.load(open("metadata/build-identity.json"))["git"]["dirty"] else 0)')
    for required in "zlOS starting" "$ORIGIN" "ready." "compositor:" "6765" \
            "$manifest_marker" \
            "build-identity: schema=1 id=$build_id" \
            "build-source: head=$build_head dirty=$build_dirty"; do
        if ! grep -Fq "$required" "$log" 2>/dev/null; then
            echo "  FAIL  $label - missing marker: $required"
            echo "        last successful markers:"
            grep -E 'zlOS starting|\[  OK  \]|ready\.|compositor:|6765|PANIC|FAULT' "$log" \
                | tail -8 | sed 's/^/          /'
            return 1
        fi
    done
    if grep -q "UEFI application - no GRUB" "$log"; then
        echo "  FAIL  $label - booted the native EFI route, not kernel64.elf"
        return 1
    fi
    echo "  ok    $label - exact kernel64.elf entered long mode, opened the compositor, fib(20)=6765"
    return 0
}

echo "== 64-bit multiboot: legacy BIOS + GRUB =="
BLOG=$(mktemp)
boot_ok=0
boot_until "$BLOG" qemu-system-x86_64 -m 1G -smp 2 "${ACCEL[@]}" \
    -cdrom zlOS64.iso && boot_ok=1
if [ "$boot_ok" -eq 1 ] && check "BIOS" "$BLOG"; then
    python3 ./tools/generators/write-app-manifest-boot-receipt.py \
        --route grub-bios64 --artifact zlOS64.iso --log "$BLOG" \
        --harness tools/checks/verify-64.sh --boot-origin "$ORIGIN" \
        --source-file tools/serial_command.py \
        --output docs/receipts/app-manifest-grub-bios64-qemu-2026-08-22.json \
        || fail=1
else
    fail=1
fi

echo "== 64-bit multiboot: UEFI + GRUB =="
if [ ! -f "$OVMF_CODE" ] || [ ! -f "$OVMF_VARS" ]; then
    echo "  skip  no OVMF firmware (apt install ovmf)"
else
    VARS=$(mktemp)
    cp "$OVMF_VARS" "$VARS"
    ULOG=$(mktemp)
    boot_ok=0
    boot_until "$ULOG" qemu-system-x86_64 -m 1G -smp 2 "${ACCEL[@]}" \
        -drive if=pflash,format=raw,unit=0,readonly=on,file="$OVMF_CODE" \
        -drive if=pflash,format=raw,unit=1,file="$VARS" \
        -cdrom zlOS64.iso && boot_ok=1
    if [ "$boot_ok" -eq 1 ] && check "UEFI" "$ULOG"; then
        python3 ./tools/generators/write-app-manifest-boot-receipt.py \
            --route grub-uefi64 --artifact zlOS64.iso --log "$ULOG" \
            --harness tools/checks/verify-64.sh --boot-origin "$ORIGIN" \
            --source-file tools/serial_command.py \
            --output docs/receipts/app-manifest-grub-uefi64-qemu-2026-08-22.json \
            || fail=1
        if grep -q "VGA text console" "$ULOG"; then
            echo "  FAIL  UEFI fell back to VGA text - the screen would be blank"
            fail=1
        fi
    else
        fail=1
    fi
    rm -f "$VARS" "$ULOG"
fi
rm -f "$BLOG"

[ "$fail" -eq 0 ] && echo "64-bit multiboot gate green" || echo "64-bit multiboot gate FAILED"
exit "$fail"
