#!/usr/bin/env bash
# check-dma.sh - does every address handed to a device go through dma.h?
#
# WHAT THIS PROTECTS
# ------------------
# zlOS runs identity-mapped, so a pointer's value is its physical address and
# dma_addr() is the identity function. The day that stops being true - which is
# the whole of Stage 4 - every address handed to a device is wrong, silently:
# nothing faults, nothing logs, the device simply writes to whatever that number
# points at now. HANDOFF.md counts this bug class SIX times already, every time
# as a symptom that read like a protocol bug.
#
# dma.h exists so that switching to a real mapping is a change to ONE function.
# That is only true while every site actually goes through it, and "every site"
# is exactly the kind of claim that is true on the day it is written and false
# two commits later. Hence a check.
#
# THE TWO RULES
# -------------
#   1. Every driver in the DMA set includes dma.h and uses dma_addr at least
#      once. A driver that stops using it has either lost a site or stopped
#      doing DMA, and both are worth a human look.
#   2. NO cast of a memory-region base to a 64-bit device field survives. After
#      conversion, `(u64)XMEM_DCBAA` is spelled `dma_addr(XMEM_DCBAA)`, so any
#      remaining `(u64)<REGION>_<NAME>` is a site that skipped the seam. This is
#      the rule that catches a NEW site written in the old style, which is how
#      it would actually happen - somebody copies the line above.
#
# WHAT IT CANNOT SEE, said plainly rather than left to be discovered
# -----------------------------------------------------------------
# It is a text check. A driver that computes a device address some entirely new
# way - a helper that returns one, an address stored in a struct and written out
# later - is invisible to rule 2. kernel/docs/dma-sites.md carries the argument for why
# the inventory is closed TODAY; this script keeps the known shape from
# regressing, it does not re-derive the inventory. If you add a driver, add it
# to DMA_FILES below and to that document, and neither of those is automatic.
#
# Static: greps source. No build, no QEMU, so it cannot fail because the host is
# busy.
set -uo pipefail
cd "$(dirname "$0")" || exit

[ -f dma.h ] || { echo "FAIL: no dma.h - the seam is gone"; exit 1; }

# The drivers that hand addresses to hardware. From kernel/docs/dma-sites.md, which
# carries the reasoning; this is the list, not the argument.
DMA_FILES="virtio_gpu.c virtio_net.c xhci.c nvme.c"

# Region-base prefixes used by those drivers. A cast of one of these to a
# 64-bit type is an address on its way to a device.
PREFIXES="XMEM|NMEM|VMEM|RX_DESC|RX_AVAIL|RX_USED|TX_DESC|TX_AVAIL|TX_USED|RX_BUF|TX_BUF"

fail=0

echo "  the DMA seam is dma.h:dma_addr()"

for f in $DMA_FILES; do
    if [ ! -f "$f" ]; then
        echo "FAIL: $f is in the DMA set and does not exist"
        echo "      Either it was renamed - update DMA_FILES and kernel/docs/dma-sites.md -"
        echo "      or the check is now watching nothing."
        fail=1
        continue
    fi

    # ---- rule 1: includes the seam and uses it ----------------------------
    if ! grep -q '#include[[:space:]]*"dma\.h"' "$f"; then
        printf 'FAIL: %s hands addresses to a device and does not include dma.h\n' "$f"
        fail=1
    fi

    n=$(grep -c 'dma_addr(' "$f")
    if [ "$n" -eq 0 ]; then
        printf 'FAIL: %s includes dma.h but never calls dma_addr()\n' "$f"
        printf '      Either a site was lost, or this driver stopped doing DMA.\n'
        fail=1
    fi

    # ---- rule 2: no raw cast of a region base to 64 bits ------------------
    # Comments stripped first: dma.h and docs quote the OLD spelling on purpose,
    # and so does this file's own header. A checker that flagged prose would be
    # switched off within a day - the same argument check-himap.sh makes.
    stripped=$(gcc -fpreprocessed -dD -E -P "$f" 2>/dev/null)
    raw=$(grep -oE "\((u64|unsigned long long)\)[[:space:]]*($PREFIXES)[A-Za-z0-9_]*" <<<"$stripped" | sort -u)
    if [ -n "$raw" ]; then
        while IFS= read -r hit; do
            printf 'FAIL: %s has %s - a device address that skips dma_addr()\n' "$f" "$hit"
        done <<<"$raw"
        printf '      Spell it dma_addr(...) instead. When Stage 4 gives the kernel a\n'
        printf '      non-identity mapping, this line hands the device a virtual address\n'
        printf '      and it writes to the wrong memory, with nothing to fault on.\n'
        fail=1
    fi

    # ---- rule 3: no raw cast in an argument to an address-carrying sink ----
    #
    # RULE 2 WAS NOT ENOUGH AND THIS IS WHY IT EXISTS. An independent audit of
    # xhci.c found FOUR live sites rule 2 is structurally incapable of seeing:
    #
    #     (u64)KBD_REPORT        a macro that EXPANDS to XMEM_DATA + 0x400
    #     (u64)PTR_BUF(ptr_enq)  likewise, and never spelled with the prefix
    #     (u64)ring              a LOCAL VARIABLE holding EP0_RING(slot)
    #     (u64)(ring | 1u)       the same, with the DCS bit already OR'd on
    #
    # Rule 2 matches on the region PREFIX, and none of those four contains one.
    # The check was green on all four while they were live paths - every
    # keystroke and every mouse report went through two of them. That is the
    # guard-that-does-not-guard shape, in a guard written the same day.
    #
    # So rule 3 matches on the SINK instead of on the value: these functions
    # take a device address, so a raw 64-bit cast anywhere in their arguments is
    # a site that skipped the seam, whatever the value is spelled like.
    SINKS='trb_write|cmd_submit|ep0_push|wr64|desc_set'
    #
    # THE ALLOWLIST IS THE WEAK POINT AND IT IS ONE LINE LONG. Every entry must
    # be an expression that is provably NOT an address, with the reason here. A
    # new site written in the old style is not in it, and is therefore caught -
    # which is the failure mode that actually happens.
    #
    #   xhci.c SETUP stage: the eight request bytes ARE the parameter field
    #   (IDT, immediate data), not a pointer to them. It is a bit composition,
    #   `((u64)hi << 32) | (u64)lo`, and there is no address anywhere in it.
    ALLOW='setup_hi'
    sink=$(grep -nE "($SINKS)\(" <<<"$stripped" \
           | grep -E "\((u64|unsigned long long)\)" \
           | grep -vE "$ALLOW" || true)
    if [ -n "$sink" ]; then
        while IFS= read -r hit; do
            printf 'FAIL: %s passes a raw 64-bit cast to an address sink:\n' "$f"
            printf '      %s\n' "$(sed 's/^[0-9]*://; s/^[[:space:]]*//' <<<"$hit")"
        done <<<"$sink"
        printf '      Those arguments are DEVICE addresses. Spell them dma_addr(...).\n'
        fail=1
    fi

    # ---- rule 4: a region macro reaching a sink without the seam ----------
    #
    # RULE 3 WAS STILL NOT ENOUGH, and a second independent audit measured why.
    # Run the earlier rules against the PRE-conversion files: nvme.c produced a
    # full set of hits, virtio_gpu.c produced two of six, and virtio_net.c
    # produced ZERO. Both virtio drivers passed region bases with no cast at all
    #
    #     mmio_w(cfg_common + CC_QUEUE_DESC + 0, VMEM_DESC);
    #     desc_set(VQ_RX, i, RX_BUF(i), BUF_SZ, DESC_WRITE, 0);
    #
    # so a rule that looks for a CAST sees nothing. For 11 of the 20 sites in
    # those two files the only thing standing guard was rule 1 - "the file calls
    # dma_addr at least once" - which stays green if six of virtio_net's seven
    # sites regress together.
    #
    # Rule 4 matches on the VALUE reaching the SINK, cast or not: a line that
    # names a region and calls a sink must also call dma_addr.
    REGIONS='VMEM_|NMEM_|XMEM_|RX_BUF|TX_BUF|RX_DESC|RX_AVAIL|RX_USED|TX_DESC|TX_AVAIL|TX_USED|q_desc|q_avail|q_used|EP0_RING|INT_RING|MSC_IN_RING|MSC_OUT_RING|CTX_INPUT|CTX_DEVICE|KBD_REPORT|PTR_BUF|CFG_BUF'
    SINKS4="$SINKS|mmio_w"
    # trb_write()'s FIRST argument is the ring's own CPU address - where WE
    # write the TRB - and is never a device address; the device learned that
    # ring's address separately, through CRCR or a TR Dequeue Pointer. So strip
    # argument one before matching, or every `trb_write(XMEM_CMDRING, ...)` is a
    # false positive, and one false positive is all it takes for a checker to be
    # switched off. Same reasoning as check-himap.sh's page-aligned filter.
    bare=$(grep -nE "($SINKS4)\(" <<<"$stripped" \
           | sed 's/trb_write([^,]*,/trb_write(_,/' \
           | grep -E "($REGIONS)" \
           | grep -v 'dma_addr' \
           | grep -vE "$ALLOW" || true)
    if [ -n "$bare" ]; then
        while IFS= read -r hit; do
            printf 'FAIL: %s hands a region address to a device sink unwrapped:\n' "$f"
            printf '      %s\n' "$(sed 's/^[0-9]*://; s/^[[:space:]]*//' <<<"$hit")"
        done <<<"$bare"
        printf '      Wrap the device-visible argument in dma_addr().\n'
        fail=1
    fi

    printf '    %-16s dma_addr x%-3s  include ok\n' "$f" "$n"
done

# The GGTT is the one device-side page table and it must keep taking a REAL
# physical address - the CPU's page tables have no effect on it. Wrapping it in
# dma_addr() would be actively wrong, so this asserts the opposite of rule 2.
if grep -q 'dma_addr' intel.c 2>/dev/null; then
    echo "FAIL: intel.c uses dma_addr() - the GGTT is a second translation layer"
    echo "      and needs a true physical address regardless of CPU paging."
    echo "      See kernel/docs/dma-sites.md, section (c)."
    fail=1
fi

[ "$fail" = 0 ] || exit 1
echo "  OK: every driver in the DMA set goes through the seam, and intel.c's GGTT does not"
exit 0
