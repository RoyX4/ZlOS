/* heap.c - a real allocator: free, reuse, coalesce, and a bounded worst case.
 *
 * arena.c is a bump allocator with a RESET and no free(), and it says so in its
 * own first paragraph. That is the right design for what it does - memory handed
 * to a foreign program, thrown away wholesale between runs - and it is NOT
 * touched by this file. Nothing that allocates from the arena today changes.
 *
 * This is the other half: memory the KERNEL keeps, where objects come and go
 * individually and the machine has to run for hours without the used byte count
 * only ever going up.
 *
 * ============================================================================
 * WHY NOT A TEXTBOOK MALLOC
 * ============================================================================
 * Three constraints here that a textbook one does not have, and each one
 * changed the design rather than decorating it:
 *
 *   1. FREESTANDING. No libc, no sbrk, no mmap, nothing underneath. The heap is
 *      one span of physical memory and that is all there will ever be. So there
 *      is no "grow the heap" path, which means the interesting case - running
 *      out - is the COMMON case rather than the one nobody tests.
 *
 *   2. A COMPOSITOR RUNS IN THIS. fb.c redraws on a frame deadline. An
 *      allocator whose bad case is "walk a free list of 40,000 blocks" does not
 *      fail, it JUDDERS, once, unreproducibly, and the bug report says the
 *      window manager is slow. So every operation here is bounded, the bound is
 *      stated, and heaptest.c MEASURES it rather than trusting this paragraph.
 *      Worst case is 2 bitmap scans and a fixed number of pointer writes: no
 *      loop in alloc or free is unbounded, and there is no search.
 *
 *   3. THIS PROJECT HAS SHIPPED SILENT FALLBACKS TWICE and paid for both. So a
 *      refusal PRINTS and a corruption HALTS. See "when it goes wrong" below.
 *
 * ============================================================================
 * THE DESIGN: segregated free lists + boundary tags
 * ============================================================================
 * Two ideas, both old, and the combination is what buys the bounded worst case.
 *
 * SEGREGATED FREE LISTS. Free blocks are filed by size into one of 44 bins.
 * Small sizes (32..512 bytes) get an EXACT bin every 16 bytes - 31 of them.
 * Larger sizes share a bin per power of two - 13 of them. A bitmap records
 * which bins are non-empty, so "find a block big enough" is: mask off the bins
 * that are too small, count trailing zeros, take the head of that list. No
 * search, no walk, no first-fit-versus-best-fit argument. That is the whole
 * reason for the bins.
 *
 * BOUNDARY TAGS. Every block knows its own size AND the size of the block
 * physically before it. So on free, both neighbours are reachable by
 * arithmetic - forward is `this + size`, backward is `this - prev_size` - and
 * merging two adjacent free blocks is a constant number of writes. Without
 * this, coalescing needs a search and the heap fragments into dust instead.
 *
 * ============================================================================
 * OFFSETS, NOT POINTERS, AND THIS IS NOT A STYLE CHOICE
 * ============================================================================
 * The free-list links are u32 offsets from HEAP_BASE, never pointers.
 *
 * CLAUDE.md opens with the rule this protects: buildefi.sh targets
 * x86_64-unknown-windows, which is LLP64, and this project has been bitten
 * TWICE by a pointer-sized quantity that was 4 bytes on one build and 8 on
 * another - once as a struct field (idt_ptr/gdt_ptr came out 6 bytes instead of
 * 10) and once as a cast that the struct fix did not cover. An allocator's
 * metadata is exactly that shape of hazard: a header whose size differs between
 * the 32-bit, 64-bit and EFI builds is three different heaps compiled from one
 * file, and the symptom is corruption on one target only.
 *
 * With u32 offsets the header is 16 bytes on all three, asserted below. It also
 * makes the whole heap position-independent for free, which Stage 4 (paging)
 * will want.
 *
 * ============================================================================
 * THE BLOCK
 * ============================================================================
 *      +----------------------------------+  <- 16-byte aligned
 *      | size       u32  incl. header     |
 *      | flags      u32  FREE             |   16-byte header
 *      | magic      u32  corruption trip  |
 *      | prev_size  u32  0 = first block  |
 *      +----------------------------------+  <- payload, 16-byte aligned
 *      | payload...                       |
 *      | (when free: u32 next, u32 prev,  |
 *      |  the free-list links, unused     |
 *      |  space after them)               |
 *      +----------------------------------+
 *
 * prev_size in the header is what replaces the usual FOOTER. The classic layout
 * puts a size copy at the END of each free block; putting it at the start of
 * the NEXT block costs the same four bytes, is valid for allocated blocks too,
 * and means a corrupted block is detected when its neighbour is touched rather
 * than silently coalesced into.
 *
 * Overhead is 16 bytes per allocation and the minimum block is 32. That is more
 * than a tuned malloc spends. It buys a header that is the same on three
 * targets and a magic word on every block, and in a kernel with no MMU
 * protection between subsystems (see Stage 5 - there is no user mode yet) that
 * trade is not close.
 *
 * ============================================================================
 * WHEN IT GOES WRONG
 * ============================================================================
 * OUT OF MEMORY returns 0 and PRINTS, with the same suppression rule arena.c
 * uses - the first few in full, then a count - because a program looping on a
 * refused allocation must not be able to wedge the machine writing to a 115200
 * baud serial line.
 *
 * CORRUPTION (a bad magic, a size that runs off the end, a free of something
 * that is not a block) is NOT recoverable and does not return an error. It
 * prints what it found and calls heap_panic(). A heap whose metadata is already
 * wrong cannot be reasoned about, and continuing means the next allocation
 * hands out a pointer into the middle of live data. This is the one place in
 * this file where stopping is the safe option.
 */

#include "memmap.h"

typedef unsigned int   u32;
typedef unsigned char  u8;

/* Same reasoning as arena.c:95 - __UINTPTR_TYPE__ is a predefined macro,
 * available freestanding, and it is right on the 32-bit kernel, the 64-bit
 * kernel, the EFI target and the 64-bit Linux host that runs heaptest.c. The
 * ZL_64 form is not: hosttest compiles this file WITHOUT ZL_64 on a host where
 * pointers are eight bytes, so it would silently narrow every pointer. */
#if defined(__UINTPTR_TYPE__)
typedef __UINTPTR_TYPE__   uptr;
#elif defined(ZL_64)
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

/* ---- where the heap lives -------------------------------------------------
 *
 * 256 MiB, in the span Stage 1 opened. This is a NEW region above every
 * existing one, which is deliberate: nothing already there moves, so this file
 * cannot collide with a buffer somebody else is holding. memmap.h declares it
 * as HI_HEAP and the ordering chain there covers it.
 *
 * 64 MiB is a budget, not the geometry, and the argument is arena.c's: the
 * span available is 768 MiB and this takes 64, so hitting the ceiling means
 * something leaked rather than that the map ran out. A ceiling set to
 * "whatever was left" tells you nothing when you reach it.
 */
#define HEAP_BASE   HI_HEAP
#define HEAP_BYTES  0x04000000UL          /* 64 MiB */
#define HEAP_END    (HEAP_BASE + HEAP_BYTES)

_Static_assert(HEAP_BASE >= HI_VGPU, "the heap is below the high-RAM map");
_Static_assert(HEAP_END <= HI_TOP,
               "the heap runs past HI_TOP - the RAM every gate promises");
_Static_assert((HEAP_BASE & 0xFFFFFUL) == 0, "the heap base is not 1 MiB aligned");

#define ALIGN_BYTES 16UL
#define HDR_BYTES   16UL
#define MIN_BLOCK   32UL                  /* header + 16 bytes of payload */

#define F_FREE      0x00000001u
#define BLK_MAGIC   0x7A4C4845u           /* "zLHE" */

/* The offset that means "no block". 0 is a real offset - it is the first
 * block - so it cannot double as the null. HEAP_BYTES is one past the end and
 * can never be a valid block offset. */
#define NIL         ((u32)HEAP_BYTES)

struct blk {
    u32 size;        /* total, including this header. multiple of 16.  */
    u32 flags;
    u32 magic;
    u32 prev_size;   /* 0 if this is the first block in the heap       */
};

_Static_assert(sizeof(struct blk) == HDR_BYTES,
               "the block header is not 16 bytes - it must be identical on the "
               "32-bit, 64-bit and EFI builds, see the LLP64 note above");
_Static_assert(MIN_BLOCK >= HDR_BYTES + 8UL,
               "the minimum block cannot hold the two free-list links");
_Static_assert((ALIGN_BYTES & (ALIGN_BYTES - 1UL)) == 0, "alignment not a power of two");
_Static_assert(HDR_BYTES % ALIGN_BYTES == 0,
               "the header is not a whole number of alignment units, so payloads "
               "would not be 16-byte aligned");

/* ---- the bins -------------------------------------------------------------
 *
 * SMALL: exact fit. bin i holds blocks of EXACTLY (i+2)*16 bytes, so bin 0 is
 * 32 (MIN_BLOCK) and bin 30 is 512. Anything in the bin fits, always.
 *
 * LARGE: two levels, which is the TLSF idea and it is here for a specific
 * reason rather than because it is clever.
 *
 * The obvious scheme - one bin per power of two - is WRONG in a way that only
 * shows under fragmentation. A bin then holds a RANGE (512..1023, say), so its
 * head may be smaller than the request even though the bin is "big enough".
 * The allocator must then either walk the list, which is the unbounded pause
 * this file exists to prevent, or skip the bin, which REFUSES an allocation
 * while a block that would have fitted sits in the very list it skipped. The
 * first cut of this file did the second, and "returns null with memory
 * available" is a far worse bug than a slow allocator, because it surfaces as
 * an out-of-memory report that the numbers contradict.
 *
 * Two levels fix it: each power of two is cut into LARGE_SL linear slices, and
 * a request is rounded UP to a slice boundary before the bin is chosen. Then
 * every block in the chosen bin is at least the bin's lower bound, which is at
 * least the request - so the HEAD always fits, with no walk and no skip. The
 * cost is at most 1/LARGE_SL of the block wasted, 12.5% at eight slices.
 *
 * LARGE_FL is 18 because HEAP_BYTES is 64 MiB: the largest block is 2^26, the
 * smallest large block is 2^9, and 26-9+1 = 18. The assert below is what keeps
 * that true if the heap size changes.
 */
#define SMALL_BINS  31
#define LARGE_MIN   512UL
#define LARGE_SL    8                       /* slices per power of two, 3 bits */
#define LARGE_SL_BITS 3
#define LARGE_FL    18                      /* powers of two, 512 .. 64 MiB   */
#define LARGE_BINS  (LARGE_FL * LARGE_SL)

_Static_assert(((unsigned long)SMALL_BINS + 1UL) * ALIGN_BYTES >= LARGE_MIN,
               "there is a gap between the small bins and the large bins");
_Static_assert((1UL << LARGE_SL_BITS) == LARGE_SL, "LARGE_SL is not 2^LARGE_SL_BITS");
_Static_assert(HEAP_BYTES <= (LARGE_MIN << (LARGE_FL - 1)),
               "LARGE_FL does not reach a block the size of the whole heap");
_Static_assert(LARGE_FL <= 32, "the first-level bitmap no longer fits in a u32");

static u32 small_head[SMALL_BINS];
static u32 large_head[LARGE_BINS];
static u32 small_map;                       /* bit i = small_head[i] non-empty */
static u32 fl_map;                          /* bit f = some slice of f is used */
static u8  sl_map[LARGE_FL];                /* bit s = large bin (f,s) non-empty */

/* ---- state ----------------------------------------------------------------*/
static unsigned long used;            /* bytes in allocated blocks, incl hdrs */
static unsigned long high_water;
static unsigned long refusals;
static unsigned long live_blocks;
static int  live;                     /* RAM probed OK and init ran           */
static int  tried;

/* The measured worst case, kept honestly rather than claimed. heaptest.c reads
 * these back and fails if a single operation ever exceeded its stated bound -
 * which is the only reason to believe the determinism claim at the top. */
static unsigned long worst_alloc_steps;
static unsigned long worst_free_steps;

/* ---- the seam, exactly as arena.c does it ---------------------------------
 * Two things from outside, which is what lets hosttest/heaptest.c compile THIS
 * SOURCE unmodified as a Linux program: the harness mmaps HEAP_BASE and
 * supplies putc. Same trick fbbench.c uses on fb.c. */
void zl_putc_pub(char c);

static void hp(const char *s) { while (*s) zl_putc_pub(*s++); }

static void hpu(unsigned long v)
{
    char b[24];
    int i = 0;
    if (!v) { zl_putc_pub('0'); return; }
    while (v) { b[i++] = (char)('0' + (int)(v % 10UL)); v /= 10UL; }
    while (i) zl_putc_pub(b[--i]);
}

/* ---- block <-> offset -----------------------------------------------------*/
static struct blk *at(u32 off) { return (struct blk *)(uptr)(HEAP_BASE + off); }

/* ---- corruption -----------------------------------------------------------
 * Not recoverable, and deliberately not an error return. See the header.
 * Declared before use because every checker calls it. */
static int panicked;

static void heap_panic(const char *why, u32 off)
{
    if (panicked) return;              /* one message, not a cascade */
    panicked = 1;
    live = 0;                          /* refuse everything from here on */
    hp("\n  *** HEAP CORRUPT: ");
    hp(why);
    hp(" at offset ");
    hpu(off);
    hp(" ***\n  The heap's own metadata is wrong, so nothing it says can be\n"
       "  trusted. Allocation is now refused rather than handing out a\n"
       "  pointer into the middle of live data.\n");
}

/* Every entry point runs this on any block it is about to believe. It is four
 * comparisons; the alternative is trusting a u32 that some other subsystem may
 * have walked over, and in a kernel with no memory protection between
 * subsystems that trust is not earned. */
static int blk_ok(u32 off, const char *ctx)
{
    const struct blk *b;
    if (off >= (u32)HEAP_BYTES)          { heap_panic(ctx, off); return 0; }
    if (off % ALIGN_BYTES)               { heap_panic(ctx, off); return 0; }
    b = at(off);
    if (b->magic != BLK_MAGIC)           { heap_panic(ctx, off); return 0; }
    if (b->size < MIN_BLOCK)             { heap_panic(ctx, off); return 0; }
    if (b->size % ALIGN_BYTES)           { heap_panic(ctx, off); return 0; }
    if ((unsigned long)off + b->size > HEAP_BYTES) { heap_panic(ctx, off); return 0; }
    return 1;
}

/* ---- which bin ------------------------------------------------------------
 * Returns 0..SMALL_BINS-1 for a small block, or SMALL_BINS + f*LARGE_SL + s for
 * a large one. __builtin_clz is a compiler intrinsic, not a libc call - it
 * compiles to BSR on x86 and is available freestanding on gcc and clang alike.
 */
#define BIN_OF_LARGE(f, s) (SMALL_BINS + (f) * LARGE_SL + (s))

/* The (first level, second level) of a size. Split out because bin_of() and
 * the allocator's rounding both need it and they must agree exactly - two
 * copies of this arithmetic that drift is a block filed in one bin and looked
 * for in another. */
static void large_idx(unsigned long size, int *fl, int *sl)
{
    int msb = 31 - __builtin_clz((unsigned int)size);   /* >= 9 for size >= 512 */
    int f = msb - 9;

    if (f >= LARGE_FL) {          /* bigger than the largest class: top bin */
        *fl = LARGE_FL - 1;
        *sl = LARGE_SL - 1;
        return;
    }
    *fl = f;
    *sl = (int)((size >> (msb - LARGE_SL_BITS)) & (unsigned long)(LARGE_SL - 1));
}

static int bin_of(unsigned long size)
{
    int f, s;
    if (size < LARGE_MIN)
        return (int)((size / ALIGN_BYTES) - 2UL);      /* 32 -> 0, 512 -> 30 */
    large_idx(size, &f, &s);
    return BIN_OF_LARGE(f, s);
}

static u32 *head_of(int bin)
{
    return (bin < SMALL_BINS) ? &small_head[bin] : &large_head[bin - SMALL_BINS];
}

static void map_set(int bin)
{
    if (bin < SMALL_BINS) { small_map |= (1u << bin); return; }
    {
        int i = bin - SMALL_BINS, f = i / LARGE_SL, s = i % LARGE_SL;
        sl_map[f] |= (u8)(1u << s);
        fl_map    |= (1u << f);
    }
}

static void map_clear(int bin)
{
    if (bin < SMALL_BINS) { small_map &= ~(1u << bin); return; }
    {
        int i = bin - SMALL_BINS, f = i / LARGE_SL, s = i % LARGE_SL;
        sl_map[f] &= (u8)~(1u << s);
        if (!sl_map[f]) fl_map &= ~(1u << f);
    }
}

/* ---- the free list --------------------------------------------------------
 * Doubly linked through the payload, so unlinking a block found by coalescing
 * is O(1). A singly linked list would need a walk to find the predecessor,
 * which is exactly the unbounded pause this file exists to avoid.
 *
 * The links are read and written through helpers rather than a struct overlay:
 * the payload is only guaranteed 16-byte aligned and 8 bytes long, and a struct
 * with two u32 members is fine, but naming the accessors makes the aliasing
 * obvious to the next reader. */
static u32 *lnk_next(struct blk *b) { return (u32 *)(void *)((u8 *)b + HDR_BYTES); }
static u32 *lnk_prev(struct blk *b) { return (u32 *)(void *)((u8 *)b + HDR_BYTES + 4); }

static void list_push(u32 off)
{
    struct blk *b = at(off);
    int bin = bin_of(b->size);
    u32 *h = head_of(bin);

    *lnk_next(b) = *h;
    *lnk_prev(b) = NIL;
    if (*h != NIL) *lnk_prev(at(*h)) = off;
    *h = off;
    map_set(bin);
}

static void list_remove(u32 off)
{
    struct blk *b = at(off);
    int bin = bin_of(b->size);
    u32 n = *lnk_next(b), p = *lnk_prev(b);

    if (p != NIL) *lnk_next(at(p)) = n;
    else          *head_of(bin)    = n;
    if (n != NIL) *lnk_prev(at(n)) = p;

    if (*head_of(bin) == NIL) map_clear(bin);
}

/* ---- init -----------------------------------------------------------------*/

/* Same two-pattern probe arena.c:205 uses, and for the same reason: absent RAM
 * either reads back as zeroes/ones or WRAPS to a lower address, and one pattern
 * at one address catches neither reliably. This one probes THREE points because
 * the heap is 64 MiB - four times the biggest buffer this project had placed
 * before - so "the bottom is backed and the top is not" is a live possibility
 * here in a way it was not for 16 MiB.
 *
 * IT RESTORES WHAT IT FOUND, and the reason is a limit worth being plain about
 * rather than a nicety. This probe answers "is there RAM here". It does NOT
 * answer "is this RAM unclaimed", and on the EFI path that second question is
 * open: firmware chooses where to load the image and where to put its own
 * structures, and nothing here consults the UEFI memory map. That is true of
 * every region in memmap.h - it is why each driver ships a *_ram_ok() - and the
 * heap is no worse, but it is also no better, and heap_init() writes a block
 * header immediately after this returns.
 *
 * So: restoring costs nothing and means a PROBE alone never damages anything,
 * which matters because the probe is the part that could reasonably run
 * speculatively later. Turning the convention into an actual check means
 * reading the UEFI memory map, which is Stage 4 work. Until then the empirical
 * evidence is verify-efi.sh booting green with this running. */
static int ram_backed(void)
{
    volatile u32 *lo  = (volatile u32 *)(uptr)HEAP_BASE;
    volatile u32 *mid = (volatile u32 *)(uptr)(HEAP_BASE + HEAP_BYTES / 2UL);
    volatile u32 *hi  = (volatile u32 *)(uptr)(HEAP_END - 4UL);
    u32 s_lo = *lo, s_mid = *mid, s_hi = *hi;
    int ok;

    *lo = 0xA5A5F00Du; *mid = 0xC3C3BEEFu; *hi = 0x5A5A0FF0u;
    ok = (*lo == 0xA5A5F00Du) && (*mid == 0xC3C3BEEFu) && (*hi == 0x5A5A0FF0u);

    *lo = s_lo; *mid = s_mid; *hi = s_hi;
    return ok;
}

int heap_init(void)
{
    int i;
    struct blk *b;

    tried = 1;
    panicked = 0;
    used = 0; high_water = 0; refusals = 0; live_blocks = 0;
    worst_alloc_steps = 0; worst_free_steps = 0;
    small_map = 0; fl_map = 0;
    for (i = 0; i < LARGE_FL; i++) sl_map[i] = 0;
    for (i = 0; i < SMALL_BINS; i++) small_head[i] = NIL;
    for (i = 0; i < LARGE_BINS; i++) large_head[i] = NIL;

    live = ram_backed();

    if (live) {
        /* One block covering everything. Every later block is a piece of this
         * one, which is what makes "did I leak" answerable: free everything and
         * the heap must be a single block of HEAP_BYTES again. heaptest.c
         * asserts exactly that. */
        b = at(0);
        b->size      = (u32)HEAP_BYTES;
        b->flags     = F_FREE;
        b->magic     = BLK_MAGIC;
        b->prev_size = 0;
        list_push(0);
    }

    hp("  heap: ");
    hpu(HEAP_BYTES >> 20);
    hp(" MiB at ");
    hpu(HEAP_BASE >> 20);
    hp(" MiB, ends at ");
    hpu(HEAP_END >> 20);
    hp(" MiB");
    if (!live) hp("  *** NOT BACKED BY RAM - heap allocation refused ***");
    hp("\n");

    return live;
}

int heap_ok(void) { return live; }

unsigned long heap_base_addr(void)   { return HEAP_BASE; }
unsigned long heap_capacity(void)    { return HEAP_BYTES; }
unsigned long heap_used(void)        { return used; }
unsigned long heap_high_water(void)  { return high_water; }
unsigned long heap_refusals(void)    { return refusals; }
unsigned long heap_blocks(void)      { return live_blocks; }
unsigned long heap_available(void)   { return live ? HEAP_BYTES - used : 0UL; }
unsigned long heap_worst_alloc(void) { return worst_alloc_steps; }
unsigned long heap_worst_free(void)  { return worst_free_steps; }

/* The same suppression rule arena.c:172 uses, for the same reason. */
#define REFUSE_LOUD 8

static void refuse(const char *why, unsigned long want)
{
    refusals++;
    if (refusals > REFUSE_LOUD + 1) return;
    if (refusals == REFUSE_LOUD + 1) {
        hp("  heap: further refusals suppressed - heap_refusals() has the count\n");
        return;
    }
    hp("  heap REFUSED ");
    hpu(want);
    hp(" bytes: ");
    hp(why);
    hp("  (");
    hpu(used);
    hp(" of ");
    hpu(HEAP_BYTES);
    hp(" used)\n");
}

/* ---- split ----------------------------------------------------------------
 * Carve `need` bytes off the front of block `off` and return the remainder to
 * the free lists. Only splits if the remainder can stand on its own as a block;
 * otherwise the caller gets the few spare bytes as slack, which is preferable
 * to creating a fragment too small to ever satisfy anything. */
static void split(u32 off, unsigned long need)
{
    struct blk *b = at(off);
    unsigned long rest = b->size - need;
    u32 roff;
    struct blk *r;

    if (rest < MIN_BLOCK) return;               /* keep the slack */

    roff = off + (u32)need;

    /* ABSORB A FREE SUCCESSOR FIRST, or this creates two adjacent free blocks
     * and the heap stops satisfying its own invariant.
     *
     * From heap_alloc this cannot happen: the block being split was just taken
     * off a free list, and a free block can never have a free neighbour (they
     * would have merged when the second one was freed). From heap_realloc's
     * SHRINK path it happens routinely - the block being split is LIVE, so its
     * successor is free as often as not.
     *
     * That is exactly what heaptest.c caught: 1921 of 1923 checks passed and
     * the two that did not were both "realloc shrink", reported by
     * heap_check()'s two-free-neighbours rule. Worth recording because the
     * failure is invisible from inside alloc/free - the metadata stays
     * self-consistent, the heap merely stops coalescing properly and
     * fragments, which would have surfaced days later as an out-of-memory that
     * the byte counts contradict. */
    if ((unsigned long)roff + rest < HEAP_BYTES) {
        struct blk *nx = at(roff + (u32)rest);
        if (nx->magic == BLK_MAGIC && (nx->flags & F_FREE)) {
            list_remove(roff + (u32)rest);
            rest += nx->size;
            nx->magic = 0;                      /* no longer a block */
        }
    }

    r = at(roff);
    r->size      = (u32)rest;
    r->flags     = F_FREE;
    r->magic     = BLK_MAGIC;
    r->prev_size = (u32)need;

    b->size = (u32)need;

    /* The block AFTER the remainder now has a new physical predecessor. Miss
     * this and backward coalescing walks to a wrong address later - the defect
     * would be invisible until two frees happened to be adjacent. */
    if ((unsigned long)roff + rest < HEAP_BYTES)
        at(roff + (u32)rest)->prev_size = (u32)rest;

    list_push(roff);
}

/* ---- alloc ----------------------------------------------------------------*/
void *heap_alloc(unsigned long bytes)
{
    unsigned long need;
    int bin, chosen;
    u32 mask, off;
    struct blk *b;
    unsigned long steps = 0;

    if (!tried) { refuse("the heap was never initialised", bytes); return 0; }
    if (!live)  { refuse("the heap is not backed by RAM", bytes); return 0; }

    /* A zero-byte allocation still gets its own distinct address, same rule as
     * arena.c:274 - returning one pointer for two objects is correct right up
     * until something compares them for identity. */
    if (!bytes) bytes = 1;

    /* THE OVERFLOW, written as a comparison against a constant BEFORE any
     * addition. `bytes` is caller-controlled; the addition form wraps and a
     * request for 4 GiB is granted. arena.c:287 makes the same argument. */
    if (bytes > HEAP_BYTES - HDR_BYTES) {
        refuse("larger than the whole heap", bytes);
        return 0;
    }
    need = (bytes + HDR_BYTES + (ALIGN_BYTES - 1UL)) & ~(ALIGN_BYTES - 1UL);
    if (need < MIN_BLOCK) need = MIN_BLOCK;

    /* FIND A BIN, IN MASKED BITMAP SCANS. No free list is ever walked, so the
     * number of free blocks does not appear in the cost at all.
     *
     * SMALL first, exact fit: every block in small bin i is exactly (i+2)*16
     * bytes, so anything at or above bin_of(need) fits by construction.
     *
     * LARGE second, and this is where the rounding earns its keep. `need` is
     * rounded UP to a second-level slice boundary before the bin is chosen, so
     * every block in that bin - and in every bin above it - is at least as big
     * as the request. The head always fits. No walk, no skip, no false
     * refusal. That rounding is the ONE line that makes this true, so it is
     * asserted at the end of the function rather than assumed. */
    bin = bin_of(need);
    chosen = -1;
    steps++;

    if (bin < SMALL_BINS) {
        mask = small_map & ~((1u << bin) - 1u);       /* this bin and above */
        if (mask) chosen = (int)__builtin_ctz(mask);
    }
    if (chosen < 0) {
        unsigned long search = need;
        int f, s;
        u32 sm;

        if (search < LARGE_MIN) {
            f = 0; s = 0;                              /* any large block fits */
        } else {
            /* TLSF's mapping_search: add one slice minus one, so the size lands
             * in the bin whose LOWER bound is >= the original request. */
            int msb = 31 - __builtin_clz((unsigned int)search);
            unsigned long slice = 1UL << (msb - LARGE_SL_BITS);
            unsigned long rounded = search + (slice - 1UL);
            if (rounded > HEAP_BYTES) rounded = HEAP_BYTES;   /* whole-heap edge */
            large_idx(rounded, &f, &s);
        }

        /* this slice and above, within this power of two */
        sm = (u32)sl_map[f] & ~((1u << s) - 1u);
        steps++;
        if (sm) {
            chosen = BIN_OF_LARGE(f, (int)__builtin_ctz(sm));
        } else {
            u32 fm = fl_map & ~((1u << (f + 1)) - 1u);  /* strictly higher class */
            steps++;
            if (fm) {
                int nf = (int)__builtin_ctz(fm);
                chosen = BIN_OF_LARGE(nf, (int)__builtin_ctz((u32)sl_map[nf]));
            }
        }
    }

    if (chosen < 0) {
        /* Genuinely nothing big enough. This is fragmentation OR exhaustion and
         * the caller cannot tell them apart from a null, so say which. */
        refuse(heap_available() >= need
                   ? "no single block that large - the heap is fragmented"
                   : "not enough room left",
               bytes);
        if (steps > worst_alloc_steps) worst_alloc_steps = steps;
        return 0;
    }

    off = *head_of(chosen);
    if (!blk_ok(off, "free-list head is not a block")) return 0;
    b = at(off);
    if (!(b->flags & F_FREE)) { heap_panic("allocated block on a free list", off); return 0; }

    /* THE ROUNDING ABOVE IS WHAT MAKES THIS UNREACHABLE, so it is checked
     * rather than trusted. If the second-level mapping is ever wrong - a
     * changed LARGE_SL, an off-by-one in large_idx - this is where it surfaces,
     * immediately and by name, instead of as a payload that overruns its block
     * and corrupts the neighbour's header some time later. */
    if (b->size < need) { heap_panic("bin head smaller than the request", off); return 0; }

    list_remove(off);
    split(off, need);

    b->flags &= ~(u32)F_FREE;
    used += b->size;
    live_blocks++;
    if (used > high_water) high_water = used;
    if (steps > worst_alloc_steps) worst_alloc_steps = steps;

    return (void *)((u8 *)b + HDR_BYTES);
}

/* ---- free -----------------------------------------------------------------
 * Coalesce both ways, then file the result. Every step is arithmetic on
 * boundary tags; nothing is searched. */
void heap_free(void *p)
{
    u32 off, noff;
    struct blk *b, *n;
    unsigned long steps = 0;

    if (!p) return;                       /* free(NULL) is a no-op, as always */
    if (!live) return;

    {
        uptr a = (uptr)p;
        if (a < (uptr)HEAP_BASE + HDR_BYTES || a >= (uptr)HEAP_END) {
            /* NOT a panic. A pointer from somewhere else - the arena, a static
             * buffer - reaching here is a caller bug, but it is a caller bug
             * that says nothing about whether the HEAP's metadata is sound, and
             * halting the machine over it would be wrong. Ignore it loudly. */
            hp("  heap: free() of a pointer that is not in the heap - ignored\n");
            return;
        }
        off = (u32)(a - (uptr)HEAP_BASE - HDR_BYTES);
    }

    if (!blk_ok(off, "free() of something that is not a block")) return;
    b = at(off);
    if (b->flags & F_FREE) { heap_panic("double free", off); return; }

    used -= b->size;
    live_blocks--;
    b->flags |= F_FREE;

    /* FORWARD: absorb the next block if it is free. */
    noff = off + b->size;
    steps++;
    if ((unsigned long)noff < HEAP_BYTES) {
        if (!blk_ok(noff, "the block after this one is corrupt")) return;
        n = at(noff);
        if (n->flags & F_FREE) {
            list_remove(noff);
            b->size += n->size;
            n->magic = 0;                  /* it is not a block any more */
            steps++;
        }
    }

    /* BACKWARD: be absorbed BY the previous block if it is free. prev_size is
     * 0 only for the first block, which has no predecessor - and 0 is not a
     * valid size for any real block, so it is unambiguous. */
    if (b->prev_size) {
        u32 poff = off - b->prev_size;
        steps++;
        if (!blk_ok(poff, "the block before this one is corrupt")) return;
        {
            struct blk *pb = at(poff);
            if (pb->size != b->prev_size) {
                heap_panic("prev_size disagrees with the previous block's size", off);
                return;
            }
            if (pb->flags & F_FREE) {
                list_remove(poff);
                pb->size += b->size;
                b->magic = 0;
                off = poff;
                b = pb;
                steps++;
            }
        }
    }

    /* Whatever block we ended up with, the one after it must learn its size. */
    if ((unsigned long)off + b->size < HEAP_BYTES)
        at(off + b->size)->prev_size = b->size;

    list_push(off);
    if (steps > worst_free_steps) worst_free_steps = steps;
}

/* ---- realloc --------------------------------------------------------------
 * A real one, unlike the arena's. It knows the old size, so it copies the right
 * number of bytes, and it can grow IN PLACE when the next block is free - which
 * is the case that matters, because every realloc caller in interp.c is
 * doubling a capacity. */
void *heap_realloc(void *p, unsigned long bytes)
{
    u32 off, noff;
    struct blk *b;
    unsigned long old_payload, need;
    void *q;

    if (!p) return heap_alloc(bytes);
    if (!bytes) { heap_free(p); return heap_alloc(1); }
    if (!live) return 0;

    {
        uptr a = (uptr)p;
        if (a < (uptr)HEAP_BASE + HDR_BYTES || a >= (uptr)HEAP_END) {
            hp("  heap: realloc() of a pointer that is not in the heap\n");
            return 0;
        }
        off = (u32)(a - (uptr)HEAP_BASE - HDR_BYTES);
    }
    if (!blk_ok(off, "realloc() of something that is not a block")) return 0;
    b = at(off);
    if (b->flags & F_FREE) { heap_panic("realloc of a freed block", off); return 0; }

    old_payload = b->size - HDR_BYTES;

    if (bytes > HEAP_BYTES - HDR_BYTES) { refuse("larger than the whole heap", bytes); return 0; }
    need = (bytes + HDR_BYTES + (ALIGN_BYTES - 1UL)) & ~(ALIGN_BYTES - 1UL);
    if (need < MIN_BLOCK) need = MIN_BLOCK;

    /* SHRINK. split() reduces b->size and hands the tail back, so `used` - which
     * counted the whole old block - has to come down by exactly what was given
     * away. Reading b->size again AFTER the split is the only way to get that
     * right, because split() declines to divide a remainder smaller than
     * MIN_BLOCK and in that case nothing was handed back at all. */
    if (need <= b->size) {
        unsigned long before = b->size;
        split(off, need);
        used -= (before - b->size);
        return p;
    }

    /* GROW IN PLACE if the physically next block is free and big enough. */
    noff = off + b->size;
    if ((unsigned long)noff < HEAP_BYTES) {
        if (!blk_ok(noff, "the block after this one is corrupt")) return 0;
        {
            struct blk *n = at(noff);
            if ((n->flags & F_FREE) &&
                (unsigned long)b->size + n->size >= need) {
                unsigned long merged = (unsigned long)b->size + n->size;
                list_remove(noff);
                n->magic = 0;
                used -= b->size;
                b->size = (u32)merged;
                if ((unsigned long)off + merged < HEAP_BYTES)
                    at(off + (u32)merged)->prev_size = (u32)merged;
                split(off, need);
                used += b->size;
                return p;
            }
        }
    }

    q = heap_alloc(bytes);
    if (!q) return 0;
    {
        unsigned long n = old_payload < bytes ? old_payload : bytes;
        const u8 *s = (const u8 *)p;
        u8 *d = (u8 *)q;
        unsigned long i;
        for (i = 0; i < n; i++) d[i] = s[i];
    }
    heap_free(p);
    return q;
}

/* ---- the invariant, checkable at any moment -------------------------------
 * Walks every block by boundary tag and reports whether the heap still makes
 * sense: sizes chain exactly from 0 to HEAP_BYTES, prev_size always matches the
 * real predecessor, no two free blocks are adjacent (they would have been
 * coalesced), and the allocated bytes add up to `used`.
 *
 * This is the ONLY function here that is O(blocks), and it is deliberately not
 * called by alloc or free - it is for heaptest.c and for a `heap` shell command.
 * Returns 0 if sound, or the offset+1 of the first block that is not.
 */
unsigned long heap_check(void)
{
    u32 off = 0, prev = NIL;
    unsigned long acc = 0, count = 0;

    if (!live) return 0;

    while ((unsigned long)off < HEAP_BYTES) {
        struct blk *b = at(off);
        if (b->magic != BLK_MAGIC)   return (unsigned long)off + 1;
        if (b->size < MIN_BLOCK)     return (unsigned long)off + 1;
        if (b->size % ALIGN_BYTES)   return (unsigned long)off + 1;
        if ((unsigned long)off + b->size > HEAP_BYTES) return (unsigned long)off + 1;
        if (prev == NIL) { if (b->prev_size != 0) return (unsigned long)off + 1; }
        else {
            if (b->prev_size != at(prev)->size) return (unsigned long)off + 1;
            if ((at(prev)->flags & F_FREE) && (b->flags & F_FREE))
                return (unsigned long)off + 1;      /* two free neighbours */
        }
        if (!(b->flags & F_FREE)) acc += b->size;
        count++;
        prev = off;
        off += b->size;
    }
    if ((unsigned long)off != HEAP_BYTES) return (unsigned long)off + 1;
    if (acc != used) return HEAP_BYTES + 1;
    (void)count;
    return 0;
}
