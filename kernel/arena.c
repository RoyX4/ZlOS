/* arena.c - the memory a foreign program is allowed to touch, and the wall it
 * cannot cross.
 *
 * Every other multi-megabyte buffer in this kernel belongs to the kernel: the
 * back buffer, the DMA rings, the task stacks. They are written by code that
 * was compiled in, reviewed, and gated. This one is different - it exists to
 * be handed to code the kernel was NOT built with, which means the interesting
 * property is not what it allocates but what it REFUSES to.
 *
 * Three things follow from that, and they are the whole design:
 *
 *   1. It is a bump allocator with a RESET, not a free list. Programs here are
 *      short-lived. Reset between runs and use-after-free stops being a class
 *      of bug rather than a bug you fix one at a time - there is no free(), so
 *      there is nothing to call twice, and every pointer into the arena dies at
 *      the same instant.
 *
 *   2. The ceiling is a BUDGET, not the geometry. The span this could have
 *      taken is 120 MiB (see the map below); it takes 16. That gap is the
 *      point: when a program hits the ceiling it means the program misbehaved,
 *      not that the map happened to run out. A ceiling set to "whatever was
 *      left" tells you nothing when you hit it.
 *
 *   3. A refusal PRINTS. This project has twice shipped a silent fallback and
 *      twice paid for it - the back buffer that turned itself off without a
 *      word (desktop-TODO 0a, T-1) is the same shape. An allocation that fails
 *      quietly turns into a null dereference three frames later, in someone
 *      else's file.
 */

/* ---- where the arena goes, and why it is NOT in the high-RAM map ----------
 *
 * fb.c:94-119 documents a fixed map running from 128 MiB (bg_buf) up to
 * 255 MiB (the end of virtio-gpu's framebuffer), and the obvious place for a
 * new buffer is above it.
 *
 * THE ORIGINAL REASON THIS IS AT 8 MiB HAS EXPIRED. Read it, because the
 * arithmetic below still depends on the neighbours and not on the reason:
 *
 *     MEASURED, not assumed - qemu-system-i386 with no -m flag gives the guest
 *     exactly 134217728 bytes, which is 128 MiB:
 *
 *         $ (echo '{"execute":"qmp_capabilities"}'; sleep 0.4;
 *            echo '{"execute":"query-memory-size-summary"}'; sleep 0.4) |
 *           qemu-system-i386 -machine pc -display none -S -qmp stdio
 *         {"return": {"base-memory": 134217728, "plugged-memory": 0}}
 *
 *     and NOT ONE gate passes -m. verify.sh, verify-raw.sh and verify-iso.sh
 *     all boot the default, so on every gate this project has, 0x08000000 and
 *     every address above it is unbacked RAM.
 *
 * The measurement is still true of a bare qemu. The conclusion is not: every
 * gate now passes `-m 1G`, which is memmap.h's HI_TOP, and check-ram.sh fails
 * the build if any of them stops. So placing a buffer above 128 MiB is no
 * longer writing dead code.
 *
 * The arena STAYS at 8 MiB anyway, and that is a decision rather than an
 * oversight: moving a live region buys nothing on its own, and the reasons it
 * is a good address (2 MiB aligned, clear of the raw-boot stack, clear of the
 * map) were never about the RAM ceiling. What changed is that it is no longer
 * FORCED here. When a real allocator wants a heap bigger than the 120 MiB hole
 * below the map, the space above the map is now genuinely available.
 *
 * None of this weakens ram_backed() below - it is what proves the memory is
 * there on the ONE machine no gate covers, the laptop, where firmware chose the
 * layout. That is also why fb.c probes before trusting the back buffer and why
 * xhci.c has xhci_ram_ok().
 *
 * The span between the kernel image and bg_buf, each entry read out of the file
 * that owns it:
 *
 *   0x00000000..0x00006FFF   boot64.S:40-42     PML4, PDPT, PD0..PD3
 *   0x00008FE0..0x00008FEF   smp.c:57           CR3_PTR
 *   0x00008FF0..0x00008FFF   smp.c:56           ENTRY_PTR
 *   0x00009000..0x00009FF0   smp.c:55,124       the SMP trampoline
 *   0x000A0000..0x000FFFFF   -                  VGA hole and BIOS ROM
 *   0x00100000..0x002E15C0   link-raw.ld        the kernel image - MEASURED
 *   0x00100000..0x00700000   raw_boot.asm       what the LOADER fills: CHUNKS
 *                                               x 32 KiB = 6 MiB from 1 MiB
 *   0x00C00000               raw_entry.S        raw-boot stack TOP, grows DOWN
 *                            raw_boot.asm       the same address, same stack
 *   0x00E00000..0x01E00000   HERE               the program arena
 *   0x02000000               kernel.zl          SNAKE_X, the fixed zl buffers
 *   0x04000000..0x04100000   virtio_net.c       NET_BASE, the NIC's rings
 *   0x08000000               fb.c:120           bg_buf - the high map starts
 *
 * The kernel image end is measured too, because it is the one entry above that
 * moves every time somebody adds a file:
 *
 *     $ readelf -s kernel_raw.elf | grep -E '__bss_start|__bss_end'
 *     __bss_start  0028a2d0        __bss_end  002e15c0
 *     -> image ends at 0x002E15C0 = 2.880 MiB, of which the LOADER must carry
 *        0x100000..0x28A2D0 = 1.540 MiB (.bss is NOBITS and zeroed by
 *        raw_entry.S, so it is not on the disk)
 *
 * ARENA_BASE is 14 MiB, and that is arithmetic rather than taste:
 *
 *   - it clears the raw-boot stack top at 12 MiB, which grows DOWN, by 2 MiB;
 *     the kernel image has 9.1 MiB of room to grow UP into before it reaches
 *     that stack, and the linker script fails the build if it ever does
 *   - it is 2 MiB aligned, which the 64-bit path cares about: boot64.S maps
 *     the low address space with 2 MiB pages
 *   - it ends at 30 MiB, 2 MiB below kernel.zl's fixed buffers at 32 MiB, and
 *     98 MiB below bg_buf. It used to end 104 MiB below the RAM ceiling as
 *     well - the same number for two entirely different reasons, asserted
 *     separately below because "the day somebody passes -m 512 they stop being
 *     the same number". That day has been and gone: HI_TOP is 1 GiB now and
 *     the checks have visibly different slack, which is why they were two.
 *
 * IT WAS 8 MiB UNTIL raw_boot.asm's CHUNKS WENT TO 192. The loader fills
 * 1 MiB .. 7 MiB now, which put the old raw-boot stack (6 MiB) inside the
 * region being loaded; the stack went to 12 MiB and the arena had to clear it.
 * Both moves are one commit, because half of it is a machine that overwrites
 * its own stack while booting.
 */

typedef unsigned int   u32;
typedef unsigned char  u8;

/* sched.c:46 keys this off ZL_64, which is right for a file only the kernel
 * ever compiles. This one is ALSO compiled by hosttest/arenatest.c on a 64-bit
 * Linux host, where ZL_64 is not set and pointers are still eight bytes - so
 * the ZL_64 form silently narrows every pointer the arena hands back and the
 * only warning is one that build.sh's -w suppresses. __UINTPTR_TYPE__ is a
 * predefined macro, available freestanding, and it is right on all three. */
#if defined(__UINTPTR_TYPE__)
typedef __UINTPTR_TYPE__   uptr;
#elif defined(ZL_64)
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

#define ARENA_BASE    0x00E00000UL     /*  14 MiB */
#define ARENA_BYTES   0x01000000UL     /*  16 MiB - the BUDGET, not the span */
#define ARENA_END     (ARENA_BASE + ARENA_BYTES)   /* 30 MiB */

/* The neighbours, by the file and line that owns each one. Re-grep these; the
 * comment in fb.c invites exactly that and it was right to - see T-EXEC-1.
 *
 * HI_BG and RAM_CEILING WERE HAND-COPIED LITERALS and are now taken from
 * memmap.h, which is the file that owns both. That header is the whole point of
 * this exercise - the i2c_hid collision it was written for happened because two
 * files each held their own copy of an address - and arena.c holding a third
 * copy of the map's floor was the same shape waiting to happen. `#include
 * "memmap.h"` resolves relative to THIS file's directory, so hosttest's
 * `gcc arenatest.c ../arena.c` from one directory down still finds it; that is
 * checked by hosttest/build.sh, not assumed.
 *
 * arenatest.c keeps its own ARENA_BASE copy on purpose (arenatest.c:36) and
 * that stays - a test that reads the value under test from the code under test
 * proves nothing. These two are different: they are not under test here, they
 * are facts about someone else's memory. */
#include "memmap.h"

#define RAW_STACK_TOP 0x00C00000UL     /* raw_entry.S, raw_boot.asm pm_entry */
#define HI_BG         HI_BACK          /* memmap.h - bg_buf, the map's floor */
#define RAM_CEILING   HI_TOP           /* memmap.h - the RAM we promise      */

/* THE NEIGHBOUR NOBODY WAS CHECKING. kernel.zl parks its fixed buffers from
 * 0x02000000 up - SNAKE_X is the lowest, then FS_META, FS_DATA, LINE_BUF,
 * HIST_BUF, DISK_SCRATCH, PAINT_BUF. check-memmap.sh sweeps those against each
 * other and memmap.h's chain covers the high map, and the 16 MiB arena sat
 * between the two, in NEITHER. It had 8 MiB of clearance and so the gap never
 * mattered; moving the arena up by 6 MiB is exactly the change that would make
 * it matter, which is the right moment to write the assert rather than the
 * moment after. check-memmap.sh now folds the arena into its overlap sweep as
 * well, so this is checked from both directions. */
#define ZL_FIXED_FLOOR 0x02000000UL    /* kernel.zl: SNAKE_X, the lowest one  */

/* Sixteen bytes, not eight: the 64-bit build's `long double` and any SSE value
 * the interpreter ends up boxing want it, and an arena that hands out
 * under-aligned memory fails in a way that only shows on one of the two
 * builds. Cheap insurance - the waste is at most 15 bytes per allocation. */
#define ARENA_ALIGN   16UL

/* THE MAP MUST BE IN ORDER AND THE COMPILER SHOULD SAY SO - the same argument
 * fb.c:152-169 makes, applied to the one buffer that is not the kernel's. */
_Static_assert(ARENA_BASE >= RAW_STACK_TOP,
               "the program arena starts below the raw-boot stack top");
_Static_assert(ARENA_END <= ZL_FIXED_FLOOR,
               "the program arena runs into kernel.zl's fixed buffers at 32 MiB");
_Static_assert(ARENA_END <= HI_BG,
               "the program arena runs into bg_buf, the high-RAM map's floor");
_Static_assert(ARENA_END <= RAM_CEILING,
               "the program arena runs past HI_TOP, the RAM every gate promises");
_Static_assert((ARENA_BASE & (2UL * 1024 * 1024 - 1)) == 0,
               "the program arena is not 2 MiB aligned - boot64.S maps in 2 MiB pages");
_Static_assert(ARENA_BYTES >= ARENA_ALIGN,
               "the program arena is smaller than one allocation");
_Static_assert((ARENA_ALIGN & (ARENA_ALIGN - 1)) == 0,
               "ARENA_ALIGN is not a power of two - the rounding below is wrong");

/* ---- the seam ------------------------------------------------------------
 * Two things this file needs from outside itself, which is what lets
 * hosttest/arenatest.c compile THIS SOURCE, unmodified, as a Linux program:
 * the harness mmaps ARENA_BASE with MAP_FIXED_NOREPLACE and supplies putc.
 * Same trick fbbench.c uses on fb.c (hosttest/fbbench.c:11-12). */
void zl_putc_pub(char c);

static void ap(const char *s) { while (*s) zl_putc_pub(*s++); }

static void apu(unsigned long v)
{
    char b[24];
    int i = 0;
    if (!v) { zl_putc_pub('0'); return; }
    while (v) { b[i++] = (char)('0' + (int)(v % 10UL)); v /= 10UL; }
    while (i) zl_putc_pub(b[--i]);
}

/* ---- state ---------------------------------------------------------------
 * The bump pointer is an OFFSET, not an address. Every ceiling test below is
 * then a comparison between two numbers that cannot be a pointer past the end
 * of an object, and the overflow reasoning is about one quantity instead of
 * two. It costs one addition per allocation. */
static unsigned long used;          /* bytes handed out, including padding */
static unsigned long high_water;    /* the most `used` has ever been       */
static unsigned long refusals;      /* how many allocations were refused   */
static unsigned long resets;
static int  live;                   /* RAM probed OK and init ran          */
static int  tried;                  /* init ran at all, pass or fail       */

/* Print the first few refusals in full and then say, once, that the rest are
 * suppressed. Not silence - this project has been bitten twice by silence -
 * but not unbounded either: a program looping on a refused allocation would
 * otherwise pin the machine writing to a 115200 baud serial line, which is
 * itself a way to wedge it, and Item 2 of the brief says a runaway program
 * must not be able to do that. The COUNT is always exact and always
 * retrievable through arena_refusals(), so the suppression hides volume and
 * never hides the fact. */
#define REFUSE_LOUD 8

static void refuse(const char *why, unsigned long want)
{
    refusals++;
    if (refusals > REFUSE_LOUD + 1) return;
    if (refusals == REFUSE_LOUD + 1) {
        ap("  arena: further refusals suppressed - use `ps` for the count\n");
        return;
    }
    ap("  arena REFUSED ");
    apu(want);
    ap(" bytes: ");
    ap(why);
    ap("  (");
    apu(used);
    ap(" of ");
    apu(ARENA_BYTES);
    ap(" used)\n");
}

/* ---- is the memory actually there? ---------------------------------------
 * xhci.c:406 established the pattern: two DIFFERENT patterns at two addresses,
 * because absent RAM either reads back as zeroes/ones or WRAPS to a lower
 * address, and one pattern at one address catches neither reliably.
 *
 * This version saves and restores what was there first. xhci_ram_ok() does
 * not, and it is right not to - it owns its arena outright. This one is
 * probing an address that a static assert says is free but that nothing has
 * proven is free on the EFI path, where firmware chose where to load us. If
 * the assert is somehow wrong, a probe that restores what it found does no
 * damage; a probe that does not have just corrupted eight bytes of something
 * and the symptom will surface somewhere else entirely. */
static int ram_backed(void)
{
    volatile u32 *lo = (volatile u32 *)(uptr)ARENA_BASE;
    volatile u32 *hi = (volatile u32 *)(uptr)(ARENA_END - 4UL);

    u32 save_lo = *lo, save_hi = *hi;
    int ok;

    *lo = 0xA5A5F00Du;
    *hi = 0x5A5A0FF0u;
    ok = (*lo == 0xA5A5F00Du) && (*hi == 0x5A5A0FF0u);

    *lo = save_lo;
    *hi = save_hi;
    return ok;
}

/* ---- the interface -------------------------------------------------------*/

int arena_init(void)
{
    tried = 1;
    used = 0;
    high_water = 0;
    refusals = 0;
    resets = 0;

    live = ram_backed();

    ap("  arena: ");
    apu(ARENA_BYTES >> 20);
    ap(" MiB at ");
    apu(ARENA_BASE >> 20);
    ap(" MiB, ends at ");
    apu(ARENA_END >> 20);
    ap(" MiB, ceiling ");
    apu(HI_BG >> 20);
    ap(" MiB (bg_buf)");
    if (!live) ap("  *** NOT BACKED BY RAM - programs cannot run ***");
    ap("\n");

    return live;
}

int arena_ok(void) { return live; }

unsigned long arena_base_addr(void) { return ARENA_BASE; }
unsigned long arena_capacity(void)  { return ARENA_BYTES; }
unsigned long arena_used(void)      { return used; }
unsigned long arena_high_water(void){ return high_water; }
unsigned long arena_refusals(void)  { return refusals; }
unsigned long arena_resets(void)    { return resets; }
unsigned long arena_available(void) { return live ? ARENA_BYTES - used : 0UL; }

void *arena_alloc(unsigned long bytes)
{
    unsigned long start, want;

    if (!tried) {
        /* Allocating before arena_init() is a wiring bug, not a budget
         * problem, and it deserves a different sentence. */
        refuse("the arena was never initialised", bytes);
        return 0;
    }
    if (!live) {
        refuse("the arena is not backed by RAM", bytes);
        return 0;
    }

    /* A zero-byte allocation still gets its own distinct address. Returning
     * the same pointer twice for two different objects is the kind of thing
     * that is correct until something compares two pointers for identity. */
    want = bytes ? bytes : 1UL;

    /* Round the bump pointer UP first, and check the rounded value, so that
     * the padding cannot be the thing that crosses the ceiling unnoticed. */
    start = (used + (ARENA_ALIGN - 1UL)) & ~(ARENA_ALIGN - 1UL);
    if (start > ARENA_BYTES) {              /* alignment alone ran off the end */
        refuse("no room left even to align", bytes);
        return 0;
    }

    /* THE OVERFLOW. Written as a subtraction, never as `start + want >
     * ARENA_BYTES`: `want` is caller-controlled and comes from a script, so
     * the addition form wraps for large values and a request for 4 GiB is
     * granted. start <= ARENA_BYTES is established one line above, so the
     * right-hand side cannot underflow. */
    if (want > ARENA_BYTES - start) {
        refuse("that would cross the ceiling", bytes);
        return 0;
    }

    used = start + want;
    if (used > high_water) high_water = used;
    return (void *)(uptr)(ARENA_BASE + start);
}

/* The whole point. Everything the program allocated dies here, at once, and no
 * pointer into the arena is valid afterwards. There is deliberately no partial
 * form - a "free the last allocation" would reintroduce ordering, and ordering
 * is what the reset exists to delete. */
void arena_reset(void)
{
    used = 0;
    resets++;
    refusals = 0;      /* the suppression notice is per-run, not per-boot */
}
