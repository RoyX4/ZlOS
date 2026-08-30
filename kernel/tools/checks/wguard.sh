#!/bin/sh
# wguard.sh - prove buildefi.sh's four -Werror= flags actually bite.
#
# WHY THIS EXISTS. From 2026-08-18 to 2026-08-19 buildefi.sh carried
#
#     -w -Werror=shift-count-overflow -Werror=void-pointer-to-int-cast
#        -Werror=pointer-to-int-cast  -Werror=int-to-pointer-cast
#
# and a comment in both that script and CLAUDE.md asserting that the ordering
# made them live, because "clang applies flags left to right". That claim was
# never run. It is false: -w suppresses these regardless of position. The guard
# against the one bug class that has bitten this kernel twice - a pointer put
# through `unsigned long` on an LLP64 target - reported nothing for its whole
# life, and 33 casts of that class accumulated behind it, one of them the
# documented bug verbatim (smp.c's ENTRY_PTR store).
#
# A guard nobody has watched go red is not a guard. This runs both directions:
#
#   A. the four flags catch a planted defect          -> must FAIL
#   B. adding -w silences them again                  -> must PASS (the bug)
#   C. the real EFI build is clean under them         -> must PASS
#
# Exits non-zero if any direction disagrees. No QEMU, ~2 seconds.
set -e
cd "$(dirname "$0")/../.."

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT INT TERM

# One instance of each class the four flags name. Every line here is a shape
# that really occurred in this tree.
cat > "$TMP/planted.c" <<'EOF'
typedef unsigned int u32;
static int idt;
/* pointer-to-int-cast: the shape of smp.c's old ENTRY_PTR store */
unsigned long a(void)      { return (unsigned long)&idt; }
/* void-pointer-to-int-cast */
unsigned long b(void *p)   { return (unsigned long)p; }
/* int-to-pointer-cast: the shape of xhci.c's MMIO casts */
int *c(unsigned long v)    { return (int *)v; }
/* shift-count-overflow: the one clang compiled to a bare `ret` */
u32  d(void)               { unsigned long v = (unsigned long)&idt; return (u32)(v >> 32); }
EOF

GUARD="-Werror=shift-count-overflow -Werror=void-pointer-to-int-cast \
       -Werror=pointer-to-int-cast  -Werror=int-to-pointer-cast"
BASE="-target x86_64-unknown-windows -ffreestanding -fno-stack-protector \
      -fshort-wchar -mno-red-zone -O2"

rc=0
say() { printf '%-58s %s\n' "$1" "$2"; }

# ---- A. the guard must catch the planted defect --------------------------
# shellcheck disable=SC2086
if clang $BASE $GUARD -c "$TMP/planted.c" -o "$TMP/a.o" 2>"$TMP/a.log"; then
    say "A. guard catches a planted truncation" "FAIL - it compiled"
    rc=1
else
    n=$(grep -c 'error:' "$TMP/a.log" || true)
    if [ "$n" -ge 4 ]; then
        say "A. guard catches a planted truncation" "ok ($n errors)"
    else
        say "A. guard catches a planted truncation" "FAIL - only $n errors, want >=4"
        cat "$TMP/a.log"
        rc=1
    fi
fi

# ---- B. -w must be shown to silence it -----------------------------------
# This direction FAILING would mean the ordering claim was true after all and
# this whole change was unnecessary. It is here so the reason is re-checkable
# rather than believed.
# shellcheck disable=SC2086
if clang $BASE -w $GUARD -c "$TMP/planted.c" -o "$TMP/b.o" 2>"$TMP/b.log"; then
    say "B. -w before the flags still silences them" "ok (confirms -w must not return)"
else
    say "B. -w before the flags still silences them" "FAIL - clang changed behaviour"
    echo "   buildefi.sh's -w could be restored; re-read this script's header."
    cat "$TMP/b.log"
    rc=1
fi

# ---- C. the real build must be clean under the live guard ----------------
if [ -f SOURCES ]; then
    # ../compile PRINTS "wrote out.c" and writes the C to ./out.c - redirecting
    # its stdout captures the message, not the program. Same two lines as
    # buildefi.sh, on purpose.
    ZL_STDLIB="$PWD/apps" ../compile src/kernel.zl >/dev/null
    cp out.c _genefi.c
    CORE=$(grep -vE '^[[:space:]]*(#|$)' SOURCES | tr '\n' ' ')
    dirty=0
    # shellcheck disable=SC2086
    INCLUDES=$(find src boot -type d -printf ' -I%s' | sort)
    for f in boot/efi.c _genefi.c ../freestanding/runtime_kernel.c boot/gdt64.c $CORE; do
        EXTRA=""
        case "$f" in */idt.c|*/apic.c) EXTRA="-mgeneral-regs-only" ;; esac
        # shellcheck disable=SC2086
        if ! clang $BASE -DZL_64 -DZL_EFI -I.. -I../src/runtime $INCLUDES -Wno-excessive-regsave $GUARD \
                  $EXTRA -DZL_KERNEL_SERIAL -fsyntax-only "$f" 2>"$TMP/c.log"; then
            echo "   $f:"; sed 's/^/     /' "$TMP/c.log"; dirty=1
        fi
    done
    if [ "$dirty" -eq 0 ]; then
        say "C. the EFI source set is clean under the guard" "ok"
    else
        say "C. the EFI source set is clean under the guard" "FAIL - see above"
        rc=1
    fi
else
    say "C. the EFI source set is clean under the guard" "SKIP - no SOURCES here"
fi

echo
if [ "$rc" -eq 0 ]; then
    echo "wguard: the guard is live and the tree is clean under it."
else
    echo "wguard: FAILED."
fi
exit "$rc"
