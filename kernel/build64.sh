#!/bin/sh
# build64.sh - the 64-bit kernel. Long mode, entered from multiboot.
#
# Everything except the boot path and the GDT is shared with the 32-bit build:
# the zl-generated code, the runtime and every driver compile unchanged for
# x86-64. -mno-red-zone is mandatory in a kernel (an interrupt would otherwise
# clobber the 128 bytes below RSP that the ABI lets leaf functions use), and
# -mcmodel=large keeps addresses absolute rather than RIP-relative-with-2GB.
set -e
cd "$(dirname "$0")"

python3 ./tools/generators/gen-app-manifest.py --check
python3 ./tools/generators/gen-build-identity.py --write

ZL_STDLIB="$PWD/apps" ../compile src/kernel.zl >/dev/null
cp out.c _gen64.c

INCLUDES=$(find src boot -type d -printf ' -I%p' | sort)
CFLAGS="-m64 -O2 -ffreestanding -nostdlib -fno-stack-protector -fno-pic
        -fno-builtin -mno-red-zone -mcmodel=large -DZL_64
        -Wall -Wextra -Werror -Wno-unused-parameter
        -I.. -I../src/frontend -I../src/runtime $INCLUDES"

# shellcheck disable=SC2086
gcc $CFLAGS -DZL_KERNEL_SERIAL -c ../freestanding/runtime_kernel.c -o _rt64.o
# shellcheck disable=SC2086
gcc $CFLAGS -c _gen64.c   -o _gen64.o
gcc $CFLAGS -c boot/gdt64.c -o _gdt64.o

# THE SHARED SOURCE LIST. See ./SOURCES. This is the build where fb.c's SIMD
# path is live (__SSE2__ is baseline on x86-64 and boot64.S sets CR4.OSFXSR),
# so it is also the one that would fault first if that guard were wrong.
OBJS=""
while read -r f; do
    case "$f" in ''|\#*) continue ;; esac
    o="_$(basename "$f" .c)64.o"
    EXTRA=""
    case "$f" in */idt.c|*/apic.c) EXTRA="-mgeneral-regs-only" ;; esac
    # shellcheck disable=SC2086
    gcc $CFLAGS $EXTRA -c "$f" -o "$o"
    OBJS="$OBJS $o"
done < SOURCES

# Freestanding interpreter: repo-root lexer/parser/interp + ksetjmp.
# shellcheck disable=SC2086
gcc $CFLAGS -DZL_FREESTANDING -DBUILD_PARSER -c ../src/frontend/lexer.c -o _lexer64.o
# shellcheck disable=SC2086
gcc $CFLAGS -DZL_FREESTANDING -DBUILD_INTERP -c ../src/frontend/parser.c -o _parser64.o
# shellcheck disable=SC2086
gcc $CFLAGS -DZL_FREESTANDING -c ../src/runtime/interp.c -o _interp64.o
# shellcheck disable=SC2086
gcc $CFLAGS -c src/arch/x86/ksetjmp.S -o _ksetjmp64.o
OBJS="$OBJS _lexer64.o _parser64.o _interp64.o _ksetjmp64.o"

gcc $CFLAGS -c boot/smp_trampoline64.S -o _smptr64.o
gcc -m64 -c boot/boot64.S -o _boot64.o

# shellcheck disable=SC2086
ld -m elf_x86_64 -T boot/link64.ld -o kernel64.elf \
   _boot64.o _gen64.o _rt64.o _gdt64.o _smptr64.o $OBJS

echo "built kernel64.elf"
echo "  undefined symbols: $(nm -u kernel64.elf 2>/dev/null | wc -l)   (0 = no libc, no OS)"
echo "  size:              $(stat -c%s kernel64.elf) bytes"
echo "  arch:              $(readelf -h kernel64.elf | grep -m1 Class | awk '{print $2}')"
