#!/bin/sh
# memmap-guard-test.sh - prove the high-RAM map guards actually guard.
#
# A _Static_assert nobody has ever seen fail is not a verified guard, it is a
# decoration. This perturbs the map on purpose and checks the build BREAKS, one
# case per hazard, including a replay of the real bug: i2c_hid.c's report buffer
# sitting 9 MiB inside fb.c's cached-blur arena.
#
# Host-only and fast - it is -fsyntax-only against the same CFLAGS build.sh
# uses, so it needs no toolchain, no QEMU and no hardware. Seconds, not minutes.
#
#   cd kernel/hosttest && ./memmap-guard-test.sh
#
# Exit status is 0 only if the baseline compiles AND every break is caught.
set -u

KDIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
CFLAGS="-m32 -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -Wno-unused-parameter"
OWNERS="fb.c sched.c i2c_hid.c nvme.c xhci.c virtio_gpu.c"

pass=0; fail=0
work=$(mktemp -d) || exit 1
trap 'rm -rf "$work"' EXIT INT TERM

# A sandbox that is the real kernel dir, except memmap.h which we can rewrite.
# Symlinks, so this never touches the tree it is testing.
mkroom() {
    room="$work/$1"; rm -rf "$room"; mkdir -p "$room"
    for f in "$KDIR"/*; do
        [ "$(basename "$f")" = memmap.h ] && continue
        ln -sf "$f" "$room/"
    done
    cp "$KDIR/memmap.h" "$room/memmap.h"
}

# compile_all <room> -> prints diagnostics, returns 0 if every owner compiled
compile_all() {
    ( cd "$1" && for c in $OWNERS; do
          gcc $CFLAGS -I.. -fsyntax-only "$c" 2>&1 || echo "FAILED:$c"
      done )
}

# expect_pass <label>
expect_pass() {
    out=$(compile_all "$work/base")
    if printf '%s' "$out" | grep -q FAILED; then
        echo "  FAIL  $1 - baseline does not compile"
        printf '%s\n' "$out" | sed 's/^/        /' | head -20
        fail=$((fail + 1))
    else
        echo "  ok    $1"
        pass=$((pass + 1))
    fi
}

# expect_break <label> <sed-script-on-memmap.h> <substring the error must contain>
expect_break() {
    mkroom broken
    sed -i "$2" "$work/broken/memmap.h"
    out=$(compile_all "$work/broken")
    if printf '%s' "$out" | grep -q "$3"; then
        echo "  ok    $1"
        echo "        caught by: $(printf '%s' "$out" | grep -o "\"[^\"]*$3[^\"]*\"" | head -1)"
        pass=$((pass + 1))
    else
        echo "  FAIL  $1 - the map was broken and NOTHING complained"
        printf '%s\n' "$out" | sed 's/^/        /' | head -20
        fail=$((fail + 1))
    fi
}

echo "memmap guard test - $KDIR"
echo
mkroom base
expect_pass "baseline: all six owners compile against the real map"

echo
echo "each of these breaks the map on purpose; the build MUST refuse it:"

# The actual reported bug, replayed: HID buffers back inside the blur arena.
expect_break "the real bug: HID_BUF back at 0x0C900000, inside the blur arena" \
    's/^#define HI_HID    0x0B800000UL/#define HI_HID    0x0C900000UL/' \
    "out of order: hid >= blur"

# Ordering: any two bases swapped underflows every "does it fit" subtraction.
expect_break "map out of order: blur placed below sched" \
    's/^#define HI_BLUR   0x0C000000UL/#define HI_BLUR   0x0A000000UL/' \
    "out of order"

# A region squeezed until its owner's buffers no longer fit inside it.
expect_break "back buffer no longer covers 3840x2160" \
    's/^#define HI_SCHED  0x0B000000UL/#define HI_SCHED  0x09000000UL/' \
    "3840x2160"

# The 256 MiB ceiling: crossing it fails as ERR_UNSPEC at run time, not loudly.
expect_break "virtio-gpu framebuffer pushed over the 256 MiB guest ceiling" \
    's/^#define HI_VGPU   0x0F000000UL/#define HI_VGPU   0x0F900000UL/' \
    "256 MiB"

# The top half of the map, which had no assertion at all before this.
expect_break "xhci arena pushed into virtio-gpu's region" \
    's/^#define HI_VGPU   0x0F000000UL/#define HI_VGPU   0x0E100000UL/' \
    "out of order\|escapes into virtio-gpu"


# ---- the addresses must not have MOVED ------------------------------------
# Rebasing each driver's bases onto memmap.h rewrote address literals into
# arithmetic. Arithmetic can be wrong. These are the exact literals that were in
# the tree before that rewrite, checked against what the expressions now
# produce - so "I only changed how it is written" is a claim with a test behind
# it rather than an assurance. Only i2c_hid.c's two buffers are meant to differ,
# and they are asserted at their new home instead.
same() {
    file=$1; shift
    mkroom same; rm -f "$work/same/$file"; cp "$KDIR/$file" "$work/same/$file"
    { echo; for a in "$@"; do
        echo "_Static_assert(($a), \"ADDRESS MOVED: $a\");"
      done } >> "$work/same/$file"
    out=$( cd "$work/same" && gcc $CFLAGS -I.. -fsyntax-only "$file" 2>&1 )
    if printf '%s' "$out" | grep -q "ADDRESS MOVED"; then
        echo "  FAIL  $file - an address changed value"
        printf '%s\n' "$out" | grep "ADDRESS MOVED" | sed 's/^/        /'
        fail=$((fail + 1))
    else
        echo "  ok    $file - all $# addresses identical to the literals"
        pass=$((pass + 1))
    fi
}

echo
echo "and the rebased addresses must still be the numbers they always were:"

same sched.c \
    "STACK_BASE == 0x0B000000u" "COUNTER_BASE == 0x0B0F0000u"

same nvme.c \
    "NMEM_ASQ == 0x0D000000u"  "NMEM_ACQ  == 0x0D001000u" \
    "NMEM_IOSQ == 0x0D002000u" "NMEM_IOCQ == 0x0D003000u" \
    "NMEM_IDENT == 0x0D004000u" "NMEM_DATA == 0x0D010000u"

same xhci.c \
    "XMEM_DCBAA == 0x0E000000u"   "XMEM_CMDRING == 0x0E001000u" \
    "XMEM_EVTRING == 0x0E002000u" "XMEM_ERST == 0x0E003000u" \
    "XMEM_CTX == 0x0E004000u"     "XMEM_XFER == 0x0E010000u" \
    "XMEM_DATA == 0x0E020000u"    "XMEM_SCRATCH_ARR == 0x0E030000u" \
    "XMEM_SCRATCH == 0x0E040000u" "MSC_IN_RING(0) == 0x0E500000u" \
    "MSC_OUT_RING(0) == 0x0E508000u" "MSC_IN_RING(3) == 0x0E500000u + 3*RING_STRIDE" \
    "MSC_CBW == 0x0E510000u"      "MSC_CSW == 0x0E510200u" \
    "MSC_DATA == 0x0E511000u"

same virtio_gpu.c \
    "VMEM_DESC == 0x0F000000u"  "VMEM_AVAIL == 0x0F001000u" \
    "VMEM_USED == 0x0F002000u"  "VMEM_CMD == 0x0F003000u" \
    "VMEM_RESP == 0x0F004000u"  "VMEM_SGLIST == 0x0F005000u" \
    "VMEM_FB == 0x0F100000u"

same fb.c \
    "HI_BACK == 0x08000000UL" "HI_BLUR == 0x0C000000UL" \
    "BACK_LIMIT == 0x03000000u" "BLUR_LIMIT == 0x01000000u"

# i2c_hid.c is the one that is SUPPOSED to have moved - out of the blur arena.
same i2c_hid.c \
    "HID_BUF == 0x0B800000u" "HID_DESC_BUF == 0x0B800100u" \
    "HID_BUF != 0x0C900000u" "HID_DESC_BUF != 0x0C900100u"

echo
echo "passed $pass, failed $fail"
[ "$fail" -eq 0 ] || exit 1
