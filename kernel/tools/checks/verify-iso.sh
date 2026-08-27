#!/usr/bin/env bash
# verify-iso.sh - the slow gate: does the ISO boot BOTH ways?
#
# verify.sh boots kernel.elf directly with QEMU's -kernel, which is fast and
# exercises the VGA text path. This one builds the real ISO and boots it
# through GRUB twice - once on legacy BIOS, once on UEFI firmware - because
# those are the two paths a real machine actually takes, and the UEFI one
# has no VGA text mode at all.
#
# Kept out of run_tests.sh on purpose: it costs ~40s and needs OVMF.
set -uo pipefail
cd "$(dirname "$0")/../.." || exit

OVMF_CODE=/usr/share/OVMF/OVMF_CODE_4M.fd
OVMF_VARS=/usr/share/OVMF/OVMF_VARS_4M.fd
fail=0
CEILING=${ZLOS_BOOT_CEILING:-360}

command -v qemu-system-x86_64 >/dev/null || { echo "skip: no qemu-system-x86_64"; exit 0; }

if [ "${ZLOS_SKIP_BUILD:-0}" = 1 ]; then
    [ -s zlOS.iso ] || { echo "FAIL: ZLOS_SKIP_BUILD=1 but zlOS.iso is missing"; exit 1; }
else
    if ! BUILD_OUTPUT=$(./tools/images/mkiso.sh 2>&1); then
        echo "FAIL: ISO did not build"
        printf '%s\n' "$BUILD_OUTPUT" | tail -80
        exit 1
    fi
fi

check() {
    local label=$1 log=$2
    if ! grep -q "zlOS starting" "$log" 2>/dev/null; then
        echo "  FAIL  $label - kernel never started"; return 1
    fi
    if ! grep -q "ready\." "$log" 2>/dev/null; then
        echo "  FAIL  $label - booted but never reached the prompt"; return 1
    fi
    # THE COMPOSITOR IS THE BOOT STATE. Both legs here boot through GRUB, which
    # supplies a multiboot framebuffer tag, so wm_avail() is true and kernel.zl
    # must take the wm_session() branch rather than the text shell. "It reached
    # the prompt" no longer distinguishes a working desktop from a kernel that
    # silently fell back to text - and on the UEFI leg falling back to text
    # means a blank screen, which is the exact failure this script exists for.
    if ! grep -q "compositor: [1-9]" "$log" 2>/dev/null; then
        echo "  FAIL  $label - reached the prompt but the compositor never opened a window"; return 1
    fi
    local manifest_marker build_id build_head build_dirty
    manifest_marker=$(python3 ./tools/generators/gen-app-manifest.py --marker)
    build_id=$(python3 -c 'import json; print(json.load(open("metadata/build-identity.json"))["identity_sha256"])')
    build_head=$(python3 -c 'import json; print(json.load(open("metadata/build-identity.json"))["git"]["head"])')
    build_dirty=$(python3 -c 'import json; print(1 if json.load(open("metadata/build-identity.json"))["git"]["dirty"] else 0)')
    grep -Fq "$manifest_marker" "$log" || {
        echo "  FAIL  $label - running image reported the wrong app manifest"; return 1;
    }
    grep -q "build-identity: schema=1 id=$build_id" "$log" || {
        echo "  FAIL  $label - running image reported the wrong build identity"; return 1;
    }
    grep -q "build-source: head=$build_head dirty=$build_dirty" "$log" || {
        echo "  FAIL  $label - running image reported the wrong source state"; return 1;
    }
    echo "  ok    $label - $(grep -oE '(framebuffer|VGA text) console, [0-9]+x[0-9]+' "$log" | head -1), $(grep -oE 'compositor: [0-9]+ windows' "$log" | head -1)"
    return 0
}

# Boot and wait for the console line, rather than for a fixed number of seconds.
# Two full QEMU boots run in this script and both were on fixed timeouts, which
# makes the gate a function of host load instead of of the kernel.
boot_until() {           # $1 = log file, rest = qemu argv
    local log="$1"; shift
    local ceiling="$CEILING"
    timeout "$ceiling" "$@" >/dev/null 2>&1 &
    local pid=$!
    # Wait for every late marker check() requires. The console
    # line is printed early in boot, so waiting on it kills QEMU before
    # "ready." is ever emitted and the gate then reports "booted but never
    # reached the prompt" on a perfectly healthy kernel. Do not assume the
    # compositor and ready markers have a fixed order: serial buffering and
    # the BIOS/UEFI paths have demonstrated both orders. Stopping after either
    # one alone manufactures a false failure under host load.
    for _ in $(seq $((ceiling * 2))); do
        if grep -q "compositor:" "$log" 2>/dev/null \
                && grep -q "ready\." "$log" 2>/dev/null; then
            break
        fi
        kill -0 "$pid" 2>/dev/null || break
        sleep 0.5
    done
    kill "$pid" 2>/dev/null; wait "$pid" 2>/dev/null
}

echo "== ISO: legacy BIOS boot =="
BLOG=$(mktemp)
# -m 1G is HI_TOP (memmap.h). Both boots below passed no -m for their whole
# lives and got qemu's 128 MiB default, so the high-RAM map above 128 MiB was
# unbacked on the only gate that boots through GRUB.
boot_until "$BLOG" qemu-system-i386 -cdrom zlOS.iso -m 1G -display none \
    -serial "file:$BLOG" -no-reboot
tr -d '\r' < "$BLOG" > "$BLOG.c" && mv "$BLOG.c" "$BLOG"
if check "BIOS" "$BLOG"; then
    python3 ./tools/generators/write-app-manifest-boot-receipt.py \
        --route grub-bios32 --artifact zlOS.iso --log "$BLOG" \
        --harness tools/checks/verify-iso.sh \
        --boot-origin "multiboot handoff, 32-bit protected mode" \
        --output docs/receipts/app-manifest-grub-bios32-qemu-2026-08-22.json \
        || fail=1
else
    fail=1
    echo "        last successful boot markers:"
    grep -E 'zlOS starting|\[OK  \]|ready\.|compositor:|PANIC|FAULT' "$BLOG" \
        | tail -5 | sed 's/^/          /'
fi

echo "== ISO: UEFI boot =="
if [ ! -f "$OVMF_CODE" ]; then
    echo "  skip  no OVMF firmware (apt install ovmf)"
else
    VARS=$(mktemp); cp "$OVMF_VARS" "$VARS"
    ULOG=$(mktemp)
    boot_until "$ULOG" qemu-system-x86_64 \
        -drive if=pflash,format=raw,unit=0,readonly=on,file="$OVMF_CODE" \
        -drive if=pflash,format=raw,unit=1,file="$VARS" \
        -cdrom zlOS.iso -m 1G -display none \
        -serial "file:$ULOG" -no-reboot
    tr -d '\r' < "$ULOG" > "$ULOG.c" && mv "$ULOG.c" "$ULOG"
    if check "UEFI" "$ULOG"; then
        python3 ./tools/generators/write-app-manifest-boot-receipt.py \
            --route grub-uefi32 --artifact zlOS.iso --log "$ULOG" \
            --harness tools/checks/verify-iso.sh \
            --boot-origin "multiboot handoff, 32-bit protected mode" \
            --output docs/receipts/app-manifest-grub-uefi32-qemu-2026-08-22.json \
            || fail=1
    else
        fail=1
        echo "        last successful boot markers:"
        grep -E 'zlOS starting|\[OK  \]|ready\.|compositor:|PANIC|FAULT' "$ULOG" \
            | tail -5 | sed 's/^/          /'
    fi
    # Under UEFI there is no VGA text mode, so a framebuffer is the ONLY way
    # anything reaches the screen. If it fell back to VGA the user sees black.
    if grep -q "VGA text console" "$ULOG" 2>/dev/null; then
        echo "  FAIL  UEFI fell back to VGA text - the screen would be blank"; fail=1
    fi
    rm -f "$VARS" "$ULOG"
fi
rm -f "$BLOG"

[ "$fail" -eq 0 ] && echo "ISO gate green" || echo "ISO gate FAILED"
exit $fail
