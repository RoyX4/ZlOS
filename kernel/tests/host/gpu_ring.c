/* gpu_ring.c - can a SOLE OWNER drive the Gen9 blitter's legacy ring?
 *
 * THE ONE QUESTION THIS ANSWERS
 * -----------------------------
 * gpu_blt.c proved the XY_COLOR_BLT encoding on this silicon by handing the
 * batch to i915 and letting i915 schedule it. That settles the command stream
 * and settles nothing about submission.
 *
 * i915 drives this part through EXECLISTS - all four engines report EL_STAT,
 * and bcs0's RING_CTL reads 0 while idle because the context owns the ring, not
 * the driver. zlOS will have no execlist machinery and no context scheduler; it
 * wants what i915 itself used on Gen8 before it switched: write RING_START,
 * RING_CTL, put commands in the ring, advance RING_TAIL, wait for RING_HEAD.
 *
 * ANSWERED, 2026-08-19, on 8086:9B41: YES. It works. RING_START, RING_CTL,
 * commands in a page, advance RING_TAIL, and the engine runs them -
 * 16384/16384 destination pixels filled, 0 still poison. No execlists, no
 * context scheduler. This file is now a regression test rather than an open
 * question.
 *
 * IT REQUIRES i915 TO BE UNBOUND. Two owners of one ring is the exact bug class
 * that broke the pointer for a week (docs/evidence/POINTER-EVIDENCE.md - two things
 * draining one xHCI event ring). Do not run --ring with i915 loaded; it checks
 * and refuses.
 *
 * NO DISPLAY REGISTERS ARE TOUCHED. Not the pipes, not the DPLL, and above all
 * not panel power - intel.c's hazard list (T12's 500 ms, AUX into an unpowered
 * panel) is about hardware this file never addresses. The blitter is a DMA
 * engine. The screen is dark during a run because i915 is unbound, not because
 * anything here programmed the display.
 *
 *   sudo ./gpu_ring --survey    READ-ONLY. Safe with i915 loaded. Run first.
 *   sudo ./gpu_ring --ring      the real thing: takes the ring and submits.
 *
 * --survey exists for the same reason modeset-run.sh's does: to prove the
 * plumbing (BAR0 mapping, forcewake, register reads, pagemap) before anything
 * is written, while the cost of being wrong is still zero.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/mman.h>

#include "../../src/drivers/display/gpu.c"          /* the command encoder proven on this silicon */

#define PCI_DEV   "0000:00:02.0"
#define BAR0_PATH "/sys/bus/pci/devices/" PCI_DEV "/resource0"

/* CLAUDE.md: map 8 MiB, not the full 16 - the kernel refuses the whole BAR
 * while i915 holds it, and everything we need is far below that. BCS is at
 * 0x22000 and the GGTT window starts at 0x800000, so 8 MiB covers both. */
/* SIXTEEN MiB, not eight - and this was the bug that made the first two --ring
 * runs draw nothing.
 *
 * CLAUDE.md says to map 8 MiB "because every display register is under 1 MiB",
 * and for the modeset work that is right. THE GGTT WINDOW IS AT 8 MiB. Mapping
 * exactly 8 MiB puts every PTE write one byte past the end of the mapping, so
 * not one GGTT entry was ever written - the engine read zeros, parsed them as
 * MI_NOOP, and advanced HEAD through them. "HEAD chased TAIL" looked like
 * success and meant the opposite.
 *
 * MEASURED, and the answer is stranger than either guess. A 16 MiB mmap of
 * resource0 is REFUSED with EINVAL - with i915 bound or unbound, so the
 * CLAUDE.md reasoning is not the whole story. And an 8 MiB mmap does NOT fault
 * at offset 0x800000: the kernel maps the whole BAR resource regardless of the
 * length asked for, so the GGTT window IS reachable through an 8 MiB mapping
 * even though it sits past its nominal end.
 *
 * So 8 MiB it is - not because the window is inside it, but because it is the
 * largest map the kernel grants and the window is reachable anyway. The PTE
 * readback in ggtt_map is what turns that from a hope into a check. */
#define BAR0_MAP_BYTES (8u << 20)

/* Engine MMIO bases, read off this machine's i915_engine_info rather than a
 * datasheet - see kernel/docs/drivers/display/gpu-blitter.md. */
#define BCS_BASE   0x22000u
#define RING_TAIL  (BCS_BASE + 0x30u)
#define RING_HEAD  (BCS_BASE + 0x34u)
#define RING_START (BCS_BASE + 0x38u)
#define RING_CTL   (BCS_BASE + 0x3Cu)

/* RING_CTL: bit 0 enables the ring, bits 20:12 hold (length in 4 KiB pages - 1).
 * A one-page ring is therefore 0 in the length field. */
#define RING_VALID 1u

/* FORCEWAKE. On Gen9 the GT power wells sleep, and a register read of a
 * sleeping well returns 0 while a write is DISCARDED. That failure is silent
 * and looks exactly like "the hardware ignored me" - so it must be held before
 * any BCS register is touched. The MT (multi-threaded) form takes a mask in the
 * high half and the value in the low half. */
#define FORCEWAKE_BLITTER_GEN9     0x0A188u
#define FORCEWAKE_ACK_BLITTER_GEN9 0x130044u
#define FW_KERNEL_BIT 1u

/* GGTT: intel.c's own layout. Entries live in the upper half of BAR0, one per
 * 4 KiB graphics page, 8 bytes each, and on Gen9 an entry is simply the
 * physical page address with bit 0 for present. */
#define GGTT_OFFSET 0x800000u
#define GGTT_WINDOW_BYTES (8u << 20)

/* Graphics addresses we will use. High enough to be clear of whatever the
 * firmware and i915 left mapped low. */
#define GFX_RING 0x00400000ull
#define GFX_DEST 0x00500000ull

#define DEST_W 256
#define DEST_H 64
#define DEST_PITCH (DEST_W * 4)
#define DEST_BYTES (DEST_PITCH * DEST_H)
#define FILL_COLOR 0x60D2EBu

static volatile unsigned char *bar0;   /* registers: BAR0 offset 0, 8 MiB */
static volatile unsigned char *ggtt;   /* the PTE table: BAR0 offset 8 MiB  */

static unsigned mmio_r(unsigned off)
{
    return *(volatile unsigned *)(bar0 + off);
}
static void mmio_w(unsigned off, unsigned v)
{
    *(volatile unsigned *)(bar0 + off) = v;
}

static double now_s(void)
{
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

static int i915_bound(void)
{
    return access("/sys/bus/pci/devices/" PCI_DEV "/driver", F_OK) == 0;
}

static int map_bar0(void)
{
    int fd = open(BAR0_PATH, O_RDWR | O_SYNC);
    if (fd < 0) { fprintf(stderr, "open %s: %s\n", BAR0_PATH, strerror(errno)); return 0; }
    void *p = mmap(NULL, BAR0_MAP_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { close(fd); fprintf(stderr, "mmap BAR0: %s\n", strerror(errno)); return 0; }
    bar0 = p;

    /* THE GGTT IS A SECOND MAPPING, AND GETTING THIS WRONG COST THREE RUNS.
     *
     * BAR0 is 16 MiB: 8 MiB of registers, then 8 MiB of PTE table (confirmed -
     * MGGC0 reads GGMS=3, so the table is 8 MiB / 1048576 entries). The obvious
     * move is to map all 16 and index at 0x800000. THAT FAILS: a 16 MiB mmap of
     * resource0 is refused with EINVAL, bound or unbound.
     *
     * The trap is what happens next. Mapping 8 MiB and writing at 0x800000
     * anyway does NOT fault - it lands in whatever the process mapped after the
     * BAR. A read there returned 0x65725F5F, which is the ASCII of
     * "__res_context_hostalias": glibc's symbol table. Every GGTT entry this
     * harness wrote went into its own heap, the engine saw no mapping, read
     * zeros, parsed them as MI_NOOP and advanced HEAD through them. "HEAD
     * chased TAIL" looked like a clean submission and meant nothing had been
     * mapped at all.
     *
     * mmap'ing the window at file offset 0x800000, as its own mapping, works.
     *
     * NOTE THIS IS A HARNESS BUG, NOT A DRIVER BUG. kernel/gpuring.c reaches the
     * same table as intel_mmio() + 0x800000 with no mmap in the way, which is
     * correct for a kernel addressing physical memory directly. */
    ggtt = mmap(NULL, GGTT_WINDOW_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GGTT_OFFSET);
    close(fd);
    if (ggtt == MAP_FAILED) {
        fprintf(stderr, "mmap GGTT window at 0x%X: %s\n", GGTT_OFFSET, strerror(errno));
        return 0;
    }
    return 1;
}

/* Hold forcewake on the blitter well, and CHECK THE ACK. Returning without
 * verifying the ack is how a run reports "the hardware ignored every write" -
 * it did, because the well was asleep. */
static int forcewake_get(void)
{
    mmio_w(FORCEWAKE_BLITTER_GEN9, (FW_KERNEL_BIT << 16) | FW_KERNEL_BIT);
    for (int i = 0; i < 5000; i++) {
        if (mmio_r(FORCEWAKE_ACK_BLITTER_GEN9) & FW_KERNEL_BIT) return 1;
        usleep(100);
    }
    return 0;
}
static void forcewake_put(void)
{
    mmio_w(FORCEWAKE_BLITTER_GEN9, (FW_KERNEL_BIT << 16) | 0u);
}

/* Physical address of a locked userspace page, via pagemap. zlOS will never
 * need this - it knows its own physical addresses - but a userspace stand-in
 * has to ask the kernel where its memory actually is before it can hand the
 * address to a DMA engine. */
static unsigned long long phys_of(void *va)
{
    int fd = open("/proc/self/pagemap", O_RDONLY);
    if (fd < 0) return 0;
    unsigned long long vaddr = (unsigned long long)(uintptr_t)va;
    unsigned long long entry = 0;
    off_t off = (off_t)(vaddr / 4096) * 8;
    if (pread(fd, &entry, 8, off) != 8) { close(fd); return 0; }
    close(fd);
    if (!(entry & (1ull << 63))) return 0;              /* not present */
    return ((entry & ((1ull << 55) - 1)) * 4096) + (vaddr % 4096);
}

/* Allocate pages that will not move and whose physical address we can learn. */
static void *alloc_locked(size_t bytes)
{
    void *p = mmap(NULL, bytes, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED | MAP_POPULATE, -1, 0);
    if (p == MAP_FAILED) return NULL;
    memset(p, 0, bytes);                                 /* fault them in */
    if (mlock(p, bytes) != 0) { /* MAP_LOCKED should have done it; not fatal */ }
    return p;
}

static int ggtt_map(unsigned gfx_page, unsigned long long phys)
{
    volatile unsigned *pte = (volatile unsigned *)(ggtt + (unsigned long)gfx_page * 8u);
    unsigned want = (unsigned)(phys & 0xFFFFF000ull) | 1u;
    pte[0] = want;
    pte[1] = (unsigned)(phys >> 32) & 0x7Fu;             /* HAW=39 on a client part */
    /* READ IT BACK. A PTE write that goes nowhere is invisible: the engine then
     * reads zeros, parses them as MI_NOOP and advances HEAD, which looks
     * exactly like a successful submission that drew nothing. Two runs were
     * lost to that before this check existed. */
    if (pte[0] != want) {
        fprintf(stderr, "  FAIL  GGTT[%u] read back 0x%08X, wrote 0x%08X\n",
                gfx_page, pte[0], want);
        return 0;
    }
    return 1;
}

static void dump_ring(const char *when)
{
    printf("  %-22s TAIL=0x%08X HEAD=0x%08X START=0x%08X CTL=0x%08X\n",
           when, mmio_r(RING_TAIL), mmio_r(RING_HEAD),
           mmio_r(RING_START), mmio_r(RING_CTL));
}

/* ---- --survey: read-only, safe with i915 loaded --------------------------- */

static int survey(void)
{
    printf("survey (read-only):\n");
    printf("  i915                   %s\n", i915_bound() ? "BOUND (holding the GPU)" : "unbound");

    int fw = forcewake_get();
    printf("  forcewake blitter      %s\n", fw ? "acked" : "NO ACK - reads below are unreliable");

    dump_ring("bcs0 ring");

    /* A GGTT entry, just read. Entry 0 usually maps the scratch page. */
    volatile unsigned *pte0 = (volatile unsigned *)ggtt;
    printf("  GGTT[0]                0x%08X%08X\n", pte0[1], pte0[0]);

    /* ---- the things today's one-off probes established, made permanent ----
     *
     * Every check below is read-only and runs with i915 loaded, so it costs a
     * user nothing. They exist because each was verified once by hand while
     * chasing the ring, and a fact verified once by hand is a fact that goes
     * stale silently.
     */

    /* GGTT size, from GGMS in MGGC0. This is what bounds gfx_page, and
     * intel_ggtt_map's bound was overflowing until it was compared as a page
     * count instead of a byte offset. */
    int cf = open("/sys/bus/pci/devices/" PCI_DEV "/config", O_RDONLY);
    unsigned short mggc0 = 0;
    if (cf >= 0) { if (pread(cf, &mggc0, 2, 0x50) != 2) mggc0 = 0; close(cf); }
    unsigned ggms = (mggc0 >> 6) & 3u;
    unsigned ggtt_mb = ggms ? (1u << ggms) : 0u;
    printf("  MGGC0 / GGMS           0x%04X / %u  -> GGTT %u MiB, %u entries\n",
           mggc0, ggms, ggtt_mb, (ggtt_mb << 20) / 8u);

    /* A LIVE PTE, decoded. This is what proved the entry format zlOS writes:
     * low dword = address | present, high dword = address bits 39:32. It also
     * proves the high dword is really used - intel_ggtt_map writes zero there
     * and can therefore never map above 4 GiB, which is a driver ceiling and
     * not a hardware one. */
    unsigned surf = mmio_r(0x7019Cu) & 0xFFFFF000u;      /* PLANE_SURF_1_A */
    if (surf) {
        unsigned pg = surf >> 12;
        volatile unsigned *e = (volatile unsigned *)(ggtt + (unsigned long)pg * 8u);
        unsigned long long ph = ((unsigned long long)(e[1] & 0x7Fu) << 32)
                              | (unsigned long long)(e[0] & 0xFFFFF000u);
        printf("  live scanout PTE       GGTT[0x%05X] = %08X %08X -> phys 0x%010llX %s\n",
               pg, e[1], e[0], ph, (e[0] & 1u) ? "present" : "NOT PRESENT");
        if (!(e[0] & 1u))
            printf("      the scanout entry is not marked present - the format assumption is wrong\n");
        if (e[1])
            printf("      high dword is NON-ZERO, so this part does map above 4 GiB\n");
    }

    /* The cursor mode constant gpucursor.c hands the display engine. 0x27 is
     * intel.c's CUR_MODE_64_ARGB; nothing checked it against hardware until a
     * live read did. */
    unsigned cc = mmio_r(0x70080u);                      /* CUR_CTL_A */
    printf("  CUR_CTL_A              0x%08X  mode 0x%02X %s\n", cc, cc & 0x3Fu,
           (cc & 0x3Fu) == 0x27u ? "== intel.c CUR_MODE_64_ARGB" :
           (cc & 0x3Fu) == 0x22u ? "== intel.c CUR_MODE_128_ARGB" :
                                   "matches NEITHER intel.c constant");

    /* RCS as well as BCS. The render engine is where the measured 48x on
     * blending lives, and its ring registers are the same four at a different
     * base - so a survey that only looks at BCS cannot say that. */
    printf("  rcs0 ring              TAIL=0x%08X HEAD=0x%08X START=0x%08X CTL=0x%08X\n",
           mmio_r(0x2000u + 0x30u), mmio_r(0x2000u + 0x34u),
           mmio_r(0x2000u + 0x38u), mmio_r(0x2000u + 0x3Cu));

    /* Prove the pagemap path works before --ring depends on it. */
    void *p = alloc_locked(4096);
    if (p) {
        unsigned long long ph = phys_of(p);
        printf("  pagemap                va %p -> phys 0x%llx %s\n",
               p, ph, ph ? "" : "(FAILED - --ring cannot work without this)");
        munmap(p, 4096);
    }

    if (fw) forcewake_put();
    printf("\n  Read-only. Nothing was written. If the numbers above are all\n");
    printf("  zero, forcewake did not take and --ring would fail silently.\n");
    return 0;
}

/* ---- --ring: take the ring and submit ------------------------------------- */

static int ring_test(void)
{
    if (i915_bound()) {
        fprintf(stderr, "REFUSING: i915 is still bound.\n");
        fprintf(stderr, "  Two owners of one ring is the bug class that cost this project a\n");
        fprintf(stderr, "  week on the xHCI event ring. Unbind i915 first - gpu-ring-run.sh\n");
        fprintf(stderr, "  does it and always gives the display back.\n");
        return 2;
    }
    printf("ring test (i915 is gone, we own the blitter):\n");

    if (!forcewake_get()) {
        fprintf(stderr, "  FAIL  forcewake never acked - the GT well is asleep and every\n");
        fprintf(stderr, "        write below would be silently discarded.\n");
        return 1;
    }
    printf("  forcewake              acked\n");

    unsigned *ring = alloc_locked(4096);
    unsigned char *dest = alloc_locked(DEST_BYTES);
    if (!ring || !dest) { fprintf(stderr, "  FAIL  allocation\n"); return 1; }

    unsigned long long ring_phys = phys_of(ring), dest_phys = phys_of(dest);
    if (!ring_phys || !dest_phys) { fprintf(stderr, "  FAIL  pagemap gave no physical address\n"); return 1; }
    printf("  ring   va %p phys 0x%llx -> gfx 0x%llx\n", (void *)ring, ring_phys, GFX_RING);
    printf("  dest   va %p phys 0x%llx -> gfx 0x%llx\n", (void *)dest, dest_phys, GFX_DEST);

    /* Map both through the GGTT. The destination is DEST_BYTES, so every page
     * of it needs an entry - mapping only the first page is a blit that writes
     * one page correctly and scribbles wherever the stale entries point. */
    if (!ggtt_map((unsigned)(GFX_RING >> 12), ring_phys)) { forcewake_put(); return 1; }
    for (unsigned i = 0; i * 4096 < DEST_BYTES; i++)
        if (!ggtt_map((unsigned)(GFX_DEST >> 12) + i, phys_of(dest + i * 4096))) {
            forcewake_put(); return 1;
        }
    (void)mmio_r(GGTT_OFFSET);            /* posting read: flush the PTE writes */

    /* Poison the destination so "filled" cannot be confused with "never ran" -
     * the same discipline gpu_blt.c uses. */
    const unsigned POISON = 0xDEADBEEFu;
    for (unsigned i = 0; i < DEST_BYTES / 4; i++) ((unsigned *)dest)[i] = POISON;

    /* Build the command straight into the ring. No batch buffer and no
     * MI_BATCH_BUFFER_START: the ring executes commands directly, and one
     * indirection fewer is one fewer thing to be wrong on the first attempt. */
    struct gpu_batch b;
    gpu_batch_init(&b, ring, 4096 / 4);
    if (!gpu_fill_rect(&b, GFX_DEST, DEST_PITCH, 0, 0, DEST_W, DEST_H, FILL_COLOR)) {
        fprintf(stderr, "  FAIL  gpu_fill_rect refused\n"); return 1;
    }
    /* MI_FLUSH_DW, and its absence is why the first run drew nothing.
     *
     * gpu_blt.c gets away without one because it submits through i915, and GEM
     * does the domain management - GEM_WAIT plus SET_DOMAIN(CPU) is what makes
     * the blit visible to a CPU read there. THERE IS NO GEM HERE. We own the
     * ring, so we own the coherency: without a flush the blitter's writes sit
     * in the render cache and the CPU reads stale memory, which looks exactly
     * like "the blit did nothing".
     *
     * Gen8+ MI_FLUSH_DW is 5 dwords: opcode 0x26 in the MI client, length
     * (5-2). No post-sync write, so the address and immediate are zero - the
     * flush itself is the point. */
    ring[b.at++] = (0x26u << 23) | 3u;   /* MI_FLUSH_DW */
    ring[b.at++] = 0;                    /* address low  (no post-sync) */
    ring[b.at++] = 0;                    /* address high */
    ring[b.at++] = 0;                    /* immediate low */
    ring[b.at++] = 0;                    /* immediate high */

    /* Pad to a qword boundary with MI_NOOPs; the ring's tail must be qword
     * aligned and the engine must not read past what we wrote. */
    while (b.at & 1u) ring[b.at++] = 0;   /* MI_NOOP is 0 */
    unsigned tail = b.at * 4;
    printf("  command                %u dwords, DW0=0x%08X\n", b.at, ring[0]);

    dump_ring("before");

    /* Program the ring. Order matters: stop it, point it at our page, enable
     * it, then move the tail. Writing TAIL first tells a ring that is still
     * pointing at i915's old buffer to execute whatever is there. */
    mmio_w(RING_CTL, 0);                          /* disable */
    mmio_w(RING_HEAD, 0);
    mmio_w(RING_TAIL, 0);
    mmio_w(RING_START, (unsigned)GFX_RING);       /* graphics address, page aligned */
    (void)mmio_r(RING_START);                     /* posting read */
    mmio_w(RING_CTL, RING_VALID);                 /* one page, enabled */
    (void)mmio_r(RING_CTL);

    dump_ring("armed");
    if (!(mmio_r(RING_CTL) & RING_VALID)) {
        printf("\n  ANSWER: the ring would not enable. RING_CTL reads back 0x%08X with\n",
               mmio_r(RING_CTL));
        printf("  RING_VALID clear, so Gen9.5 will not take legacy ring submission this\n");
        printf("  way and zlOS needs the execlist path instead. That is a real result.\n");
        forcewake_put();
        return 1;
    }

    /* Go. */
    double t0 = now_s();
    mmio_w(RING_TAIL, tail);

    unsigned head = 0;
    int done = 0;
    while (now_s() - t0 < 2.0) {
        head = mmio_r(RING_HEAD) & 0x1FFFFCu;
        if (head == tail) { done = 1; break; }
        usleep(200);
    }
    double el = (now_s() - t0) * 1e3;
    dump_ring("after");
    printf("  HEAD chased TAIL        %s in %.2f ms\n", done ? "YES" : "NO (timed out)", el);

    /* The only verification that counts: read the pixels. */
    unsigned *px = (unsigned *)dest;
    size_t filled = 0, poisoned = 0;
    for (size_t i = 0; i < DEST_BYTES / 4; i++) {
        if ((px[i] & 0xFFFFFFu) == FILL_COLOR) filled++;
        else if (px[i] == POISON) poisoned++;
    }
    size_t want = (size_t)DEST_W * DEST_H;
    printf("  destination             %zu/%zu filled, %zu still poison\n", filled, want, poisoned);

    int rc;
    if (filled == want) {
        printf("\n  ANSWER: YES. A sole owner CAN drive the Gen9.5 blitter's legacy ring.\n");
        printf("  zlOS's path is RING_START/CTL/TAIL, no execlists needed.\n");
        rc = 0;
    } else if (done) {
        printf("\n  ANSWER: the ring CONSUMED the command (HEAD reached TAIL) but the\n");
        printf("  pixels did not land. That is an addressing problem - GGTT mapping or\n");
        printf("  the destination graphics address - not a submission problem.\n");
        rc = 1;
    } else {
        printf("\n  ANSWER: the ring never advanced. Either legacy submission is off on\n");
        printf("  this part, or RING_START/CTL were not accepted. Compare the register\n");
        printf("  dumps above.\n");
        rc = 1;
    }

    mmio_w(RING_CTL, 0);              /* leave it disabled for i915 to re-init */
    forcewake_put();
    return rc;
}

int main(int argc, char **argv)
{
    const char *mode = argc > 1 ? argv[1] : "--survey";
    /* 77 is SKIP, not failure - the autotools convention gates/land-gate.sh
     * adopted after gpu_blt tripped over it. This binary lives in hosttest/,
     * and that gate GLOBS every executable there and runs it as an ordinary
     * user. Returning 2 for "not root" made the whole land gate red, blaming a
     * harness that had correctly declined to run. Same for a box with no Intel
     * GPU: absent hardware is a skip, not a defect. */
    if (geteuid() != 0) {
        fprintf(stderr, "gpu_ring needs root (it maps BAR0) - skipping.\n");
        return 77;
    }
    if (!map_bar0()) {
        fprintf(stderr, "no Intel BAR0 here - skipping.\n");
        return 77;
    }

    printf("gpu_ring: can a sole owner drive the Gen9 blitter ring?\n");
    printf("  BAR0 mapped            %u MiB of " PCI_DEV "\n\n", BAR0_MAP_BYTES >> 20);

    if (!strcmp(mode, "--survey")) return survey();
    if (!strcmp(mode, "--ring"))   return ring_test();
    fprintf(stderr, "usage: %s [--survey|--ring]\n", argv[0]);
    return 2;
}
