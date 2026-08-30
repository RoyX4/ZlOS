#!/bin/sh
# buildefi.sh - zlOS as a native UEFI application. No GRUB, no bootloader.
#
# The firmware loads the tiny EFI/BOOT/BOOTX64.EFI witness first.  It writes an
# ESP trace and immediately chainloads EFI/ZLOS/ZLOS.EFI, the real kernel.
# Both are PE32+ EFI applications built without gnu-efi or edk2.
set -e
cd "$(dirname "$0")"

ZL_STDLIB="$PWD/apps" ../compile src/kernel.zl >/dev/null
cp out.c _genefi.c

# -fshort-wchar: UEFI strings are UTF-16
# -mno-red-zone: mandatory - interrupts would clobber it
# -fno-stack-protector / -ffreestanding: no runtime under us
# THERE IS NO -w HERE ANY MORE, AND THAT IS THE POINT. Do not put it back.
#
# This target is LLP64: `unsigned long` is 4 bytes here and 8 everywhere else,
# so casting a pointer through it silently drops the top half, and shifting
# such a value by 32 is undefined - clang compiled one such shift to a bare
# `ret`, leaving an IDT gate's high half to whatever was in eax. The four
# -Werror= flags below name that class exactly.
#
# They used to sit AFTER a -w, on the stated reasoning that "clang applies
# flags left to right, so these must stay after the -w". MEASURED FALSE on
# 2026-08-19 with clang 21.1.8: -w suppresses these regardless of position -
# before them, after them, either way. The guard reported nothing for its
# entire life, while 33 casts of exactly this class sat in the build. One of
# them, smp.c's ENTRY_PTR store, is the documented bug verbatim: a 64-bit
# destination handed an address already truncated to 32 bits.
#
# ./wguard.sh reproduces both halves - the suppression, and the guard biting
# once -w is gone. Run it if you are tempted to change this line.
#
# -Wexcessive-regsave is the ONLY warning -w was legitimately buying: it fires
# 11 times in idt.c because an __attribute__((interrupt)) handler saves a lot
# of registers, which is the entire job of an interrupt handler. It is
# suppressed BY NAME, so that a new warning class shows up rather than being
# swallowed with it.
INCLUDES=$(find src boot -type d -printf ' -I%s' | sort)
CF="-target x86_64-unknown-windows -ffreestanding -fno-stack-protector \
    -fshort-wchar -mno-red-zone -O2 -DZL_64 -DZL_EFI -I.. -I../src/runtime $INCLUDES \
    -Wno-excessive-regsave \
    -Werror=shift-count-overflow -Werror=void-pointer-to-int-cast \
    -Werror=pointer-to-int-cast -Werror=int-to-pointer-cast"

# THE SHARED SOURCE LIST. See ./SOURCES - one file, read by all four build
# scripts. This build and mkdisk.sh are the two that broke on 2026-08-18 when a
# .c was added to build.sh alone, and verify.sh cannot see either of them.
#
# The four files ahead of the list are this target's own: the UEFI entry point
# instead of a multiboot header, the generated C under its own name, the
# runtime, and the 64-bit GDT. Everything after comes from SOURCES.
CORE=$(grep -vE '^[[:space:]]*(#|$)' SOURCES | tr '\n' ' ')

OBJS=""
# shellcheck disable=SC2086
for f in boot/efi.c _genefi.c ../freestanding/runtime_kernel.c boot/gdt64.c $CORE; do
    o="_efi_$(basename "$f" .c).o"
    # idt.c and apic.c hold the interrupt handlers, and they must be built
    # -mgeneral-regs-only so a handler can never touch SSE. build.sh and
    # build64.sh have always done this; THIS script did not, and the omission
    # was invisible for as long as the handlers happened to be small enough
    # that the compiler had no reason to reach for XMM. Grow one and it does -
    # and an ISR that clobbers XMM without saving it lands on the zl
    # interpreter, because every zl number is a double. The symptom is the
    # 64-bit boot dying inside setup_idt() with no diagnostic at all, while
    # the 32-bit build is perfectly happy.
    EXTRA=""
    case "$f" in */idt.c|*/apic.c) EXTRA="-mgeneral-regs-only" ;; esac
    # shellcheck disable=SC2086
    clang $CF $EXTRA -DZL_KERNEL_SERIAL -c "$f" -o "$o"
    OBJS="$OBJS $o"
done

# The AP trampoline is assembly, not C, so it is not in the loop above.
clang $CF -c boot/smp_trampoline64.S -o _efi_smptr.o
OBJS="$OBJS _efi_smptr.o"

# UEFI images are relocated by firmware.  A zero preferred base, subsystem
# version 0.0 and a present .reloc directory are the conventional removable-
# media shape.  The old 6 GiB DLL-style image also boots a measured 7 GiB OVMF
# guest, so this is firmware compatibility hardening, not a claimed root cause.
# 4 KiB file alignment and no IMAGE_FILE_DLL match common shim/GRUB images.
PEFLAGS="-subsystem:efi_application,0.0 -nodefaultlib -base:0 -filealign:4096"

# shellcheck disable=SC2086
lld-link $PEFLAGS -entry:efi_main -out:ZLOS.EFI $OBJS

# Keep stage 0 independent of the kernel object graph.  If the large image is
# malformed or faults, this small application can still say so and persist the
# exact LoadImage/StartImage status before returning to firmware.
clang $CF -c boot/efi_stage0.c -o _efi_stage0.o
# shellcheck disable=SC2086
lld-link $PEFLAGS -entry:efi_stage0_main -out:BOOTX64.EFI _efi_stage0.o

echo "built BOOTX64.EFI stage 0 ($(stat -c%s BOOTX64.EFI) bytes)"
echo "built ZLOS.EFI kernel ($(stat -c%s ZLOS.EFI) bytes)"
