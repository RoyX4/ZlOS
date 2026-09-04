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
# serial log looks healthy. kernel/tools/probes/probe-uefi.py measures which devices work.
set -uo pipefail
cd "$(dirname "$0")/../.." || exit 1
. tools/checks/qemu-crash.sh

OVMF_CODE=/usr/share/OVMF/OVMF_CODE_4M.fd
OVMF_VARS=/usr/share/OVMF/OVMF_VARS_4M.fd
fail=0

command -v qemu-system-x86_64 >/dev/null || { echo "skip: no qemu-system-x86_64"; exit 0; }
[ -f "$OVMF_CODE" ] || { echo "skip: no OVMF firmware (apt install ovmf)"; exit 0; }

if [ "${ZLOS_SKIP_BUILD:-0}" = 1 ]; then
    [ -s zlOS-usb.img ] || {
        echo "FAIL: ZLOS_SKIP_BUILD=1 but zlOS-usb.img is missing"; exit 1;
    }
else
    if ! BUILD_OUTPUT=$(./tools/images/mkusb.sh 2>&1); then
        echo "FAIL: the UEFI image did not build"
        printf '%s\n' "$BUILD_OUTPUT" | tail -80
        exit 1
    fi
fi
./tests/host/efi_stage0_test.py || exit 1
python3 ./tests/host/efi_kernel_witness_test.py || exit 1

VARS=$(mktemp)
LOG=$(mktemp)
BOOT_IMAGE=$(mktemp --suffix=.img)

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
if [ "${ZLOS_FORCE_TCG:-0}" = 1 ]; then
    echo "  note  TCG forced for emulator-fallback verification"
    ACCEL=(-cpu max -accel tcg)
    ACCEL_KIND=tcg
elif [ -r /dev/kvm ] && [ -w /dev/kvm ]; then
    ACCEL=(-cpu host -accel kvm)
    ACCEL_KIND=kvm
else
    echo "  note  no /dev/kvm - falling back to TCG (slower, still correct)"
    ACCEL=(-cpu max -accel tcg)
    ACCEL_KIND=tcg
fi

boot_once() {
    cp "$OVMF_VARS" "$VARS"
    cp zlOS-usb.img "$BOOT_IMAGE"
    : > "$LOG"
    timeout "$CEILING" qemu-system-x86_64 \
        -m 1G -smp 2 "$@" \
        -drive if=pflash,format=raw,unit=0,readonly=on,file="$OVMF_CODE" \
        -drive if=pflash,format=raw,unit=1,file="$VARS" \
        -device qemu-xhci,id=xhci \
        -drive format=raw,file="$BOOT_IMAGE",if=none,id=boot \
        -device usb-storage,bus=xhci.0,drive=boot,removable=on \
        -device usb-kbd,bus=xhci.0 \
        -device usb-mouse,bus=xhci.0 \
        -vga std -display none -no-reboot \
        -serial "file:$LOG" >/dev/null 2>&1 &
    QPID=$!
    for _ in $(seq $((CEILING * 2))); do
        grep -q "ready\." "$LOG" 2>/dev/null && break
        kill -0 "$QPID" 2>/dev/null || break
        sleep 0.5
    done
    sleep 1                    # let the line after the prompt land
    kill "$QPID" 2>/dev/null
    wait "$QPID" 2>/dev/null
    QSTATUS=$?
    tr -d '\r' < "$LOG" > "$LOG.c" && mv "$LOG.c" "$LOG"
}

boot_once "${ACCEL[@]}"
if qemu_crashed "$QSTATUS"; then
    if [ "$ACCEL_KIND" = kvm ] && ! grep -q "ready\." "$LOG"; then
        echo "  note  retrying once with TCG after the KVM emulator crash"
        ACCEL=(-cpu max -accel tcg)
        ACCEL_KIND=tcg
        boot_once "${ACCEL[@]}"
        qemu_crashed "$QSTATUS" || true
    fi
fi
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
    if grep -Eq "vmm: 64 MiB mapped: virtual [0-9]+ GiB -> physical 256 MiB" "$LOG"; then
        echo "  ok    transactional heap page tables were committed and reached through the alias"
    else
        echo "  FAIL  transactional heap mapping was absent or fell back to identity"
        grep -E "vmm:|page-table transaction|window" "$LOG" | tail -5 | sed 's/^/          /'
        fail=1
    fi
    if grep -Eq '^  pmm: [1-9][0-9]*/[1-9][0-9]* pages free in \[320, 1024\) MiB$' "$LOG" &&
       grep -q "physical allocator reserved floor, owner quota/mismatch, double-free and zero/reuse passed; baseline restored" "$LOG"; then
        echo "  ok    typed physical allocator admitted firmware RAM and restored its self-test baseline"
    else
        echo "  FAIL  physical allocator map/ownership/zero-reuse proof missing or failed"
        grep -E "pmm:|physical allocator" "$LOG" | tail -4 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "ring 3 64: u1 <- iretq/int80/iretq, 6 syscalls, process exited, kernel alive" "$LOG"; then
        echo "  ok    protected 64-bit Ring 3 entered, made syscalls, and exited"
    else
        echo "  FAIL  64-bit Ring 3 proof missing or incomplete"
        grep -E "ring 3 64:|FAULT|fault" "$LOG" | tail -5 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "syscall ABI: zero/gap/sign-bit/max refused with ENOSYS" "$LOG"; then
        echo "  ok    syscall ABI rejects zero, gaps and sign-bit IDs with ENOSYS"
    else
        echo "  FAIL  unknown-syscall ENOSYS proof missing or failed"
        grep -E "syscall ABI:" "$LOG" | tail -2 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "process lifecycle generation reuse rejected the stale handle and retained exact exit custody" "$LOG"; then
        echo "  ok    process slot reuse advances generation and preserves signed exit custody"
    else
        echo "  FAIL  generation-safe process identity proof missing or failed"
        grep -E "process lifecycle generation|generation/exit custody" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "anonymous memory: M <- reserve/commit zero-fill, cross-page copy and release passed" "$LOG"; then
        echo "  ok    Ring-3 anonymous pages reserve, commit zeroed frames, cross a page and release"
    else
        echo "  FAIL  anonymous reserve/commit/release proof missing or failed"
        grep -E "anonymous memory:" "$LOG" | tail -2 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "reserved anonymous page stayed absent; sibling V exited" "$LOG"; then
        echo "  ok    reserved anonymous pages remain non-present and fault only their owner"
    else
        echo "  FAIL  reserved anonymous-page fault proof missing or failed"
        grep -E "reserved anonymous page" "$LOG" | tail -2 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "released anonymous page faulted exactly; sibling R exited" "$LOG"; then
        echo "  ok    released anonymous pages are unmapped, reclaimed and fault exactly"
    else
        echo "  FAIL  released anonymous-page fault proof missing or failed"
        grep -E "released anonymous page" "$LOG" | tail -2 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "ring 3 hostile: cli GP, kernel/device PF, crossing pointer refused; kernel alive" "$LOG"; then
        echo "  ok    hostile Ring-3 faults/refusals leave the kernel alive"
    else
        echo "  FAIL  hostile Ring-3 isolation proof missing or failed"
        grep -E "ring 3 hostile:" "$LOG" | tail -2 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "two PML4 processes yielded/resumed AB12 and exited independently" "$LOG"; then
        echo "  ok    two Ring-3 processes switch CR3/kernel stacks and resume saved frames"
    else
        echo "  FAIL  multi-process yield/resume proof missing or failed"
        grep -E "PML4 processes|multi-process|AB12" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "process-owned page tables/code/stacks reclaimed; PMM baseline restored" "$LOG"; then
        echo "  ok    process page tables, code and stacks are PMM-owned and reclaimed"
    else
        echo "  FAIL  process-frame ownership/reclamation proof missing or failed"
        grep -E "process-owned page tables|process frame reclamation" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "process memory accounting: fixed/anonymous quotas and owner totals passed" "$LOG"; then
        echo "  ok    process and anonymous frame owners return to zero under bounded quotas"
    else
        echo "  FAIL  process/anonymous owner accounting proof missing or failed"
        grep -E "process memory accounting" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "process lifecycle slots reaped with generation history retained" "$LOG"; then
        echo "  ok    process identities reap only after resources and retain generation history"
    else
        echo "  FAIL  process identity teardown proof missing or failed"
        grep -E "process lifecycle slots reaped|process lifecycle final teardown" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "persistent service scheduled cooperative ST12 across four kernel turns; exact exit custody reaped" "$LOG"; then
        echo "  ok    persistent Ring-3 service returned across four cooperative kernel turns and reaped exact exits"
    else
        echo "  FAIL  persistent user-process service proof missing or failed"
        grep -E "persistent service scheduled|persistent user-process service" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "PIT preempted two non-yielding Ring-3 loops PQ" "$LOG"; then
        echo "  ok    timer preemption switches non-yielding Ring-3 processes"
    else
        echo "  FAIL  timer-preemptive process switch proof missing or failed"
        grep -E "PIT preempted|timer process preemption" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "one process GP-faulted; its sibling ran and exited" "$LOG"; then
        echo "  ok    a faulted Ring-3 process cannot stop its sibling"
    else
        echo "  FAIL  sibling process fault-isolation proof missing or failed"
        grep -E "sibling|GP-faulted" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "process lifecycle retained exact GP-fault custody and independent sibling exit" "$LOG"; then
        echo "  ok    fault records remain distinct from the sibling's signed exit status"
    else
        echo "  FAIL  process fault/exit custody proof missing or failed"
        grep -E "process lifecycle retained exact|process lifecycle fault/exit" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "lower stack guard PF error 6 at exact address; sibling G exited" "$LOG"; then
        echo "  ok    the lower user-stack guard faults exactly and kills only its offender"
    else
        echo "  FAIL  lower user-stack guard fault proof missing or failed"
        grep -E "lower stack guard|guard fault containment" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "two guarded supervisor TSS stacks bounded through syscall/preempt/fault paths" "$LOG" &&
       grep -Eq '^  kernel stacks high-water: P0 [1-9][0-9]* P1 [1-9][0-9]* bytes$' "$LOG"; then
        echo "  ok    two guarded supervisor TSS stacks retain bounded high-water evidence"
    else
        echo "  FAIL  guarded supervisor TSS stack proof missing or failed"
        grep -E "kernel stacks high-water|guarded supervisor TSS" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "bounded IPC crossed PML4s hi/ok with sender IDs h1o2" "$LOG"; then
        echo "  ok    bounded PID-addressed IPC copies messages across address spaces"
    else
        echo "  FAIL  cross-process IPC proof missing or failed"
        grep -E "bounded IPC|h1o2" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    if grep -q "Ring-3 window presented text, polled input W, and closed" "$LOG"; then
        echo "  ok    Ring-3 window/text/input/close ABI reached the real WM seam"
    else
        echo "  FAIL  Ring-3 window/input ABI proof missing or failed"
        grep -E "Ring-3 window|window/input ABI" "$LOG" | tail -3 | sed 's/^/          /'
        fail=1
    fi
    MANIFEST_MARKER=$(python3 ./tools/generators/gen-app-manifest.py --marker)
    if grep -Fq "$MANIFEST_MARKER" "$LOG"; then
        echo "  ok    running UEFI image reports the generated app manifest"
    else
        echo "  FAIL  running UEFI image did not report the generated app manifest"
        fail=1
    fi
fi

# The serial transcript proves the child kernel ran.  The ESP witness proves
# the removable-media BOOTX64 entry itself ran and that LoadImage accepted the
# separate kernel before StartImage transferred control.  It is flushed before
# the child starts, so killing QEMU after the prompt cannot lose this evidence.
TRACE=$(mktemp)
if ! mtype -i "$BOOT_IMAGE"@@1M ::/EFI/ZLOS/WITNESS.LOG >"$TRACE" 2>/dev/null; then
    echo "  FAIL  stage 0 left no ESP witness"; fail=1
elif ! grep -q "STAGE0 ENTER" "$TRACE"; then
    echo "  FAIL  ESP witness has no entry marker"; fail=1
elif ! grep -q "LOAD_IMAGE 0x0000000000000000" "$TRACE"; then
    echo "  FAIL  stage 0 did not record a successful LoadImage"
    sed 's/^/          /' "$TRACE" | tail -12
    fail=1
elif ! grep -q "START_IMAGE CALL" "$TRACE"; then
    echo "  FAIL  stage 0 never called StartImage"; fail=1
elif ! grep -q "KERNEL_ENTRY" "$TRACE"; then
    echo "  FAIL  ZLOS.EFI never reached its EFI entry"; fail=1
elif ! grep -q "KERNEL_IMAGE base=" "$TRACE"; then
    echo "  FAIL  kernel image placement was not recorded"; fail=1
elif ! grep -q "EFI_DIAG_ARM status=0x0000000000000000" "$TRACE"; then
    echo "  FAIL  kernel could not arm the firmware fallback diagnostic"; fail=1
elif ! grep -q "ACPI_RESULT rsdp=" "$TRACE"; then
    echo "  FAIL  kernel stopped before ACPI discovery completed"; fail=1
elif ! grep -q "GOP_RESULT status=" "$TRACE" ||
     ! grep -q "GOP_DETAILS " "$TRACE"; then
    echo "  FAIL  kernel stopped before GOP discovery completed"; fail=1
elif ! grep -q "MEMORY_MAP_RESULT status=0x0000000000000000" "$TRACE"; then
    echo "  FAIL  kernel did not record a successful memory map"; fail=1
elif ! grep -q "FIXED_MEMORY .*hi_back_overlap=yes" "$TRACE"; then
    echo "  FAIL  kernel did not classify the legacy 128..168 MiB backbuffer span"; fail=1
elif ! grep -q "FIXED_MEMORY safe" "$TRACE"; then
    echo "  FAIL  firmware owns part of a fixed zlOS memory range"
    grep "FIXED_MEMORY" "$TRACE" | tail -34 | sed 's/^/          /'
    fail=1
elif ! grep -q "BEFORE_EXIT_BOOT_SERVICES" "$TRACE"; then
    echo "  FAIL  kernel never reached the final firmware-exit boundary"; fail=1
else
    echo "  ok    stage 0 and kernel persisted every boundary through ExitBootServices"
fi

if [ "$fail" -eq 0 ]; then
    python3 ./tools/generators/write-app-manifest-boot-receipt.py \
        --route native-uefi64 --artifact zlOS-usb.img --log "$LOG" \
        --harness tools/checks/verify-efi.sh \
        --boot-origin "UEFI application - no GRUB, no bootloader" \
        --output docs/receipts/app-manifest-native-uefi64-qemu-2026-08-22.json \
        || fail=1
fi

if [ "$fail" -eq 0 ]; then
    python3 ./tools/checks/write-scheduler-receipt.py \
        --log "$LOG" --selftest \
        --output docs/receipts/scheduler-native-uefi64-qemu-2026-08-29.json \
        || fail=1
fi

if [ "$fail" -eq 0 ]; then
    python3 ./tools/checks/write-user-process-receipt.py \
        --log "$LOG" --selftest \
        --output docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json \
        || fail=1
fi

if [ "$fail" -eq 0 ]; then
    python3 ./tools/checks/write-pmm-receipt.py \
        --log "$LOG" --selftest \
        --output docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json \
        || fail=1
fi

JOURNAL=$(mktemp)
if ! python3 ../tools/zllog.py read "$BOOT_IMAGE" --latest >"$JOURNAL" 2>/dev/null; then
    echo "  FAIL  could not read the QEMU journal after boot"; fail=1
elif ! grep -q "cache=write-combining" "$JOURNAL"; then
    echo "  FAIL  framebuffer stayed uncacheable after the WC transition"; fail=1
else
    echo "  ok    live framebuffer mapping changed to write-combining"
fi

if [ "$fail" -eq 0 ]; then
    python3 ./tools/checks/write-page-table-receipt.py \
        --log "$LOG" --journal "$JOURNAL" --selftest \
        --output docs/receipts/page-table-native-uefi64-qemu-2026-08-29.json \
        || fail=1
fi

python3 ./tests/host/efi_runtime_diag_test.py || fail=1

rm -f "$VARS" "$LOG" "$BOOT_IMAGE" "$TRACE" "$JOURNAL"
[ "$fail" -eq 0 ] && echo "EFI gate green" || echo "EFI gate FAILED"
exit $fail
