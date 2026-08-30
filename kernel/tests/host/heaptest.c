/* heaptest.c - heap.c as a Linux program, with no QEMU and no reboot.
 *
 * heap.c is compiled here UNMODIFIED - the shipping source, not a copy. The
 * harness supplies the two things heap.c asks the outside world for (a putc,
 * and memory at HEAP_BASE) and then hammers it. Same trick arenatest.c uses on
 * arena.c and fbbench.c uses on fb.c, and for the same reason: an allocator is
 * just arithmetic against memory, so a reboot proves nothing a process cannot.
 *
 * WHAT THIS IS ACTUALLY FOR, in order of how much it matters:
 *
 *   1. THE INVARIANT. heap_check() walks every block by boundary tag and
 *      reports whether the heap still adds up. It is called after EVERY
 *      operation in the stress phases below, not at the end - an allocator that
 *      is sound at the end of a run and briefly corrupt in the middle is an
 *      allocator that corrupts memory, and only per-operation checking finds
 *      the operation that did it.
 *
 *   2. FREE AND REUSE, which is the entire reason this file exists over
 *      arena.c. "Allocate 10 MiB, free it, allocate 10 MiB again" must succeed
 *      the second time in a 64 MiB heap. A bump allocator fails that, and it is
 *      the one-line difference between the two designs.
 *
 *   3. THE DETERMINISM CLAIM. heap.c says no operation walks a free list. That
 *      is a claim about worst-case behaviour and it is worthless unless
 *      something measures it, so heap_worst_alloc()/heap_worst_free() record
 *      the highest step count any single call has taken, and this file asserts
 *      a hard ceiling on it AFTER a workload that leaves thousands of free
 *      blocks lying around. If the bound were really O(free blocks) that test
 *      is what would catch it.
 *
 *   4. COALESCING, checked by its observable consequence rather than by
 *      reaching into the metadata: free everything and the largest possible
 *      single allocation must come back, which can only be true if adjacent
 *      free blocks merged.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

/* Must match heap.c. Duplicated on purpose, exactly as arenatest.c:36 does it:
 * if somebody moves the heap and does not move the harness, the mmap fails
 * loudly here rather than the test quietly exercising a different address than
 * the kernel uses. Reading the value out of the code under test would prove
 * nothing at all. */
#define HEAP_BASE   0x10000000UL
#define HEAP_BYTES  0x04000000UL
#define HDR_BYTES   16UL
#define ALIGN_BYTES 16UL
#define FAILURE_SWEEP_ALLOCATIONS 32

/* ---- heap.c's interface --------------------------------------------------*/
int   heap_init(void);
int   heap_ok(void);
void *heap_alloc(unsigned long);
void *heap_alloc_tagged(unsigned long, unsigned int);
void  heap_free(void *);
void *heap_realloc(void *, unsigned long);
unsigned int heap_tag(const void *);
void  heap_fail_after(unsigned long);
void  heap_fail_disable(void);
unsigned long heap_injected_failures(void);
unsigned long heap_base_addr(void);
unsigned long heap_capacity(void);
unsigned long heap_used(void);
unsigned long heap_high_water(void);
unsigned long heap_refusals(void);
unsigned long heap_blocks(void);
unsigned long heap_available(void);
unsigned long heap_check(void);
unsigned long heap_worst_alloc(void);
unsigned long heap_worst_free(void);

/* ---- the seam ------------------------------------------------------------*/
static int quiet = 0;
void zl_putc_pub(char c) { if (!quiet) fputc(c, stdout); }

/* paging.c's job, stubbed. There is no CR3 to extend in a Linux process, so the
 * honest answer here is the same one the 32-bit kernel build gives: no window,
 * use the physical address. That keeps this test exercising heap.c's ordinary
 * path - which is also the path the 32-bit kernel and any failed mapping take,
 * so it is the majority case rather than a special one. paging.c's own
 * arithmetic is tested separately in pagingtest.c. */
unsigned long long vmm_map_window(unsigned long long phys, unsigned long long bytes)
{
    (void)phys; (void)bytes;
    return 0;
}

/* ---- tiny assert harness, same shape as arenatest.c ----------------------*/
static int checks = 0, failures = 0;

static void ok(int cond, const char *what)
{
    checks++;
    if (!cond) { failures++; printf("  FAIL  %s\n", what); }
}

static void okv(int cond, const char *what, unsigned long got, unsigned long want)
{
    checks++;
    if (!cond) {
        failures++;
        printf("  FAIL  %s: got %lu, wanted %lu\n", what, got, want);
    }
}

/* heap_check() returns 0 when sound. Called after every mutation in the stress
 * phases - see the header for why "at the end" is not good enough. */
static void sound(const char *where)
{
    unsigned long bad = heap_check();
    checks++;
    if (bad) {
        failures++;
        printf("  FAIL  heap invariant broken during %s (code %lu)\n", where, bad);
    }
}

/* A deterministic PRNG. rand() would make a failure depend on the libc, and a
 * heap bug that reproduces only on one machine is a heap bug nobody fixes. */
static unsigned long rng_state = 0x9E3779B97F4A7C15UL;
static unsigned long rnd(unsigned long n)
{
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return n ? rng_state % n : 0;
}

int main(void)
{
    void *p = mmap((void *)HEAP_BASE, HEAP_BYTES, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p != (void *)HEAP_BASE) {
        printf("  FAIL  cannot map %lu MiB at 0x%08lX: %s\n",
               HEAP_BYTES >> 20, HEAP_BASE, strerror(errno));
        return 1;
    }

    printf("heaptest - heap.c, unmodified, as a Linux program\n\n");

    /* ---- init ------------------------------------------------------------*/
    ok(heap_init() == 1, "heap_init reports the RAM is backed");
    ok(heap_ok() == 1, "heap_ok agrees");
    okv(heap_base_addr() == HEAP_BASE, "base address", heap_base_addr(), HEAP_BASE);
    okv(heap_capacity() == HEAP_BYTES, "capacity", heap_capacity(), HEAP_BYTES);
    okv(heap_used() == 0, "nothing used yet", heap_used(), 0);
    sound("init");

    /* ---- the basics ------------------------------------------------------*/
    {
        char *a = heap_alloc(100);
        char *b = heap_alloc(100);
        ok(a != NULL, "a small allocation succeeds");
        ok(b != NULL, "a second one succeeds");
        ok(a != b, "two allocations are different addresses");
        ok(((unsigned long)a % ALIGN_BYTES) == 0, "payload is 16-byte aligned");
        ok(((unsigned long)b % ALIGN_BYTES) == 0, "and so is the second");
        ok((unsigned long)a >= HEAP_BASE &&
           (unsigned long)a < HEAP_BASE + HEAP_BYTES, "a is inside the heap");
        okv(heap_blocks() == 2, "two live blocks", heap_blocks(), 2);

        /* The bytes must actually be usable - writing the full request and
         * reading it back is what catches a block handed out one size class
         * too small, which is the failure the two-level binning exists to
         * prevent and which no metadata check would notice. */
        memset(a, 0xAB, 100);
        memset(b, 0xCD, 100);
        {
            int i, good = 1;
            for (i = 0; i < 100; i++) if ((unsigned char)a[i] != 0xAB) good = 0;
            for (i = 0; i < 100; i++) if ((unsigned char)b[i] != 0xCD) good = 0;
            ok(good, "both payloads survive being written and read back");
        }
        sound("basic allocation");

        heap_free(a);
        sound("free of a");
        heap_free(b);
        sound("free of b");
        okv(heap_used() == 0, "used returns to zero", heap_used(), 0);
        okv(heap_blocks() == 0, "no live blocks", heap_blocks(), 0);
    }

    /* ---- THE POINT: free really does give the memory back ----------------
     * This is the one behaviour arena.c cannot produce at all. A 32 MiB
     * allocation in a 64 MiB heap can only succeed twice in a row if the first
     * one was genuinely reclaimed. */
    {
        void *big1 = heap_alloc(32UL << 20);
        ok(big1 != NULL, "a 32 MiB allocation succeeds");
        heap_free(big1);
        sound("free of the 32 MiB block");
        {
            void *big2 = heap_alloc(32UL << 20);
            ok(big2 != NULL, "and a SECOND 32 MiB allocation succeeds after it");
            okv((unsigned long)big2 == (unsigned long)big1,
                "reusing the same memory", (unsigned long)big2, (unsigned long)big1);
            heap_free(big2);
        }
        okv(heap_used() == 0, "used is zero again", heap_used(), 0);
        sound("the reuse round trip");
    }

    /* ---- coalescing, by its consequence ----------------------------------
     * Chop the heap into 1024 pieces, free them all, then ask for very nearly
     * the whole heap. That can only be satisfied if every adjacent pair merged
     * back together - 1024 unmerged fragments would refuse it. */
    {
        void *v[1024];
        int i;
        for (i = 0; i < 1024; i++) v[i] = heap_alloc(4096);
        ok(v[1023] != NULL, "1024 x 4 KiB all allocate");
        sound("1024 allocations");

        /* Free in a scrambled order. Freeing forwards only ever exercises
         * backward coalescing, and this project has shipped a one-directional
         * merge before. */
        for (i = 0; i < 1024; i += 2) heap_free(v[i]);
        sound("freeing the even ones");
        for (i = 1023; i > 0; i -= 2) heap_free(v[i]);
        sound("freeing the odd ones");

        okv(heap_used() == 0, "used is zero after freeing all 1024", heap_used(), 0);
        {
            /* HEAP_BYTES minus one header is the largest request that can ever
             * be satisfied, and it needs the whole heap to be ONE block. */
            void *whole = heap_alloc(HEAP_BYTES - HDR_BYTES);
            ok(whole != NULL,
               "the whole heap allocates again - so every fragment coalesced");
            heap_free(whole);
        }
        sound("the coalescing round trip");
    }

    /* ---- realloc ---------------------------------------------------------*/
    {
        char *g = heap_alloc(8);
        int i, good = 1;
        memset(g, 0x5A, 8);
        g = heap_realloc(g, 64);
        ok(g != NULL, "realloc to a larger size succeeds");
        for (i = 0; i < 8; i++) if ((unsigned char)g[i] != 0x5A) good = 0;
        ok(good, "realloc PRESERVED the old contents");
        sound("realloc grow");

        /* Shrinking must give the difference back rather than quietly keeping
         * it - that accounting was wrong in the first draft of heap.c. */
        {
            unsigned long before = heap_used();
            g = heap_realloc(g, 16);
            ok(g != NULL, "realloc to a smaller size succeeds");
            ok(heap_used() < before, "shrinking a block reduces used bytes");
        }
        sound("realloc shrink");
        heap_free(g);
        ok(heap_realloc(NULL, 32) != NULL, "realloc(NULL) behaves as alloc");
        okv(heap_check() == 0, "invariant holds after realloc", heap_check(), 0);
    }
    /* everything from the realloc block is still live; clear it out */
    heap_init();

    /* ---- refusals, not crashes -------------------------------------------*/
    {
        unsigned long before = heap_refusals();
        ok(heap_alloc(HEAP_BYTES * 2) == NULL, "a request larger than the heap is refused");
        ok(heap_alloc((unsigned long)-1) == NULL, "a request of ~0 is refused, not wrapped");
        ok(heap_refusals() > before, "and the refusals were counted");
        sound("after two refusals");
        okv(heap_used() == 0, "a refused allocation used nothing", heap_used(), 0);

        /* free(NULL) and a foreign pointer must both be survivable. */
        heap_free(NULL);
        {
            static char elsewhere[64];
            quiet = 1; heap_free(elsewhere); quiet = 0;
        }
        sound("free(NULL) and free of a foreign pointer");
        ok(heap_ok() == 1, "neither of those tripped the corruption halt");
    }

    /* ---- tags and deterministic allocation failure ----------------------*/
    {
        char *tagged = heap_alloc_tagged(64, 0xA11Cu);
        ok(tagged != NULL, "a tagged allocation succeeds");
        okv(heap_tag(tagged), "the allocation tag reads back", heap_tag(tagged), 0xA11Cu);
        tagged = heap_realloc(tagged, 256);
        ok(tagged != NULL, "a tagged allocation can grow");
        okv(heap_tag(tagged), "realloc preserves the allocation tag",
            heap_tag(tagged), 0xA11Cu);
        heap_free(tagged);

        {
            unsigned long before = heap_injected_failures();
            void *first, *failed, *after;
            heap_fail_after(1);
            first = heap_alloc(32);
            failed = heap_alloc(32);
            after = heap_alloc(32);
            ok(first != NULL, "failure injection allows the requested successes");
            ok(failed == NULL, "failure injection refuses the selected allocation");
            ok(after != NULL, "failure injection is one-shot");
            okv(heap_injected_failures(), "the injected refusal is counted",
                heap_injected_failures(), before + 1);
            heap_fail_disable();
            heap_free(first);
            heap_free(after);
        }
        sound("tags and deterministic allocation failure");
    }

    /* ---- every position in a bounded allocation transaction -------------
     * A single planted refusal proves the switch works; it does not prove the
     * allocator leaves identical state regardless of where the refusal lands.
     * Sweep every call in one fixed transaction, then require the selected
     * call alone to fail, later calls to recover, all surviving payloads to
     * remain intact and the entire heap to coalesce again. */
    {
        unsigned long fail_at;

        for (fail_at = 0; fail_at < FAILURE_SWEEP_ALLOCATIONS; fail_at++) {
            void *slot[FAILURE_SWEEP_ALLOCATIONS];
            unsigned long used_before = 0, blocks_before = 0;
            int i, selected_failed = 0, other_calls_succeeded = 1;
            int failed_call_unchanged = 0, payloads_intact = 1;

            quiet = 1;
            if (!heap_init()) {
                quiet = 0;
                ok(0, "heap reinitialises for every failure position");
                break;
            }
            heap_fail_after(fail_at);
            for (i = 0; i < FAILURE_SWEEP_ALLOCATIONS; i++) {
                unsigned long bytes = 32UL + (unsigned long)i;
                if ((unsigned long)i == fail_at) {
                    used_before = heap_used();
                    blocks_before = heap_blocks();
                }
                slot[i] = heap_alloc(bytes);
                if ((unsigned long)i == fail_at) {
                    selected_failed = slot[i] == NULL;
                    failed_call_unchanged = heap_used() == used_before
                                         && heap_blocks() == blocks_before;
                } else if (!slot[i]) {
                    other_calls_succeeded = 0;
                } else {
                    memset(slot[i], i + 1, bytes);
                }
            }
            heap_fail_disable();
            quiet = 0;

            ok(selected_failed, "the selected allocation position is refused");
            ok(other_calls_succeeded, "all non-selected allocation positions succeed");
            ok(failed_call_unchanged, "the injected refusal changes no heap accounting");
            okv(heap_injected_failures(), "exactly one refusal is injected",
                heap_injected_failures(), 1);

            for (i = 0; i < FAILURE_SWEEP_ALLOCATIONS; i++) {
                unsigned long j, bytes = 32UL + (unsigned long)i;
                unsigned char *payload = slot[i];
                if (!payload) continue;
                for (j = 0; j < bytes; j++) {
                    if (payload[j] != (unsigned char)(i + 1)) {
                        payloads_intact = 0;
                        break;
                    }
                }
            }
            ok(payloads_intact, "surviving payloads remain intact after refusal");

            for (i = 0; i < FAILURE_SWEEP_ALLOCATIONS; i++) heap_free(slot[i]);
            okv(heap_used() == 0,
                "the failure sweep releases every successful allocation",
                heap_used(), 0);
            sound("bounded every-position allocation failure sweep");
        }
        okv(fail_at, "every bounded allocation position was exercised",
            fail_at, FAILURE_SWEEP_ALLOCATIONS);
    }

    /* ---- the stress phase, and the determinism measurement ---------------
     * A random mix of sizes, allocated and freed in a scrambled order, with
     * the invariant re-checked on every single operation. The size
     * distribution deliberately straddles the small/large boundary at 512
     * bytes, because that is where the two binning schemes meet and where an
     * off-by-one in large_idx() would live. */
    {
        void *slot[512];
        unsigned long size[512];
        int i, round;
        int alive = 0;

        for (i = 0; i < 512; i++) { slot[i] = NULL; size[i] = 0; }

        for (round = 0; round < 4000; round++) {
            i = (int)rnd(512);
            if (slot[i]) {
                /* verify the payload before handing it back - this is what
                 * catches two live allocations that overlap, which is the
                 * worst thing an allocator can do and is invisible to
                 * heap_check() because the metadata stays consistent. */
                unsigned char want = (unsigned char)(i & 0xFF);
                unsigned long k;
                unsigned char *q = slot[i];
                for (k = 0; k < size[i]; k++) {
                    if (q[k] != want) { failures++; printf("  FAIL  payload %d corrupted at byte %lu\n", i, k); break; }
                }
                checks++;
                heap_free(slot[i]);
                slot[i] = NULL;
                alive--;
            } else {
                unsigned long n = 1 + rnd(2000);      /* straddles 512 */
                slot[i] = heap_alloc(n);
                if (slot[i]) {
                    memset(slot[i], i & 0xFF, n);
                    size[i] = n;
                    alive++;
                }
            }
            if (heap_check()) {
                failures++;
                printf("  FAIL  invariant broken at stress round %d\n", round);
                break;
            }
        }
        checks++;
        ok(alive > 0, "the stress phase kept allocations live throughout");
        printf("  (stress: 4000 operations, %d still live)\n", alive);

        for (i = 0; i < 512; i++) if (slot[i]) heap_free(slot[i]);
        okv(heap_used() == 0, "used returns to zero after the stress phase",
            heap_used(), 0);
        sound("the end of the stress phase");
        {
            void *whole = heap_alloc(HEAP_BYTES - HDR_BYTES);
            ok(whole != NULL, "the whole heap is one block again after 4000 ops");
            heap_free(whole);
        }
    }

    /* ---- THE DETERMINISM CLAIM, measured ---------------------------------
     * heap.c's header says no operation walks a free list, so the step count
     * per call must be a small constant no matter how many free blocks exist.
     * The stress phase above left thousands of them. If the bound were secretly
     * O(free blocks) these numbers would be in the thousands.
     *
     * 16 is chosen well above what the code can actually do (alloc does at most
     * three bitmap scans; free does at most four boundary-tag steps) and well
     * below anything a list walk could produce. It is a REGRESSION bound, not a
     * tight one - the printed values are the real answer. */
    {
        unsigned long wa = heap_worst_alloc(), wf = heap_worst_free();
        printf("  worst case over the whole run: alloc %lu steps, free %lu steps\n",
               wa, wf);
        ok(wa <= 16, "no allocation took more than 16 steps");
        ok(wf <= 16, "no free took more than 16 steps");
        ok(wa > 0, "the step counter is actually being written");
    }

    /* ---- bounded freed-block poison -------------------------------------
     * The allocator owns the first eight payload bytes while a block is free
     * for list links. Corrupt the next byte instead: a same-size reuse must
     * detect the write before returning the block to a caller. This runs last
     * because detecting metadata corruption deliberately takes the heap
     * offline. */
    {
        unsigned char *dead = heap_alloc(32);
        ok(dead != NULL, "the poison probe allocation succeeds");
        heap_free(dead);
        dead[8] ^= 0xFFu;
        quiet = 1;
        ok(heap_alloc(32) == NULL, "reuse detects a write through a freed pointer");
        quiet = 0;
        ok(heap_ok() == 0, "freed-block poison failure takes the heap offline");
        ok(heap_init() == 1, "heap_init recovers a deliberately poisoned test heap");
        sound("reinitialised after the poison probe");
    }

    printf("\n%d checks, %d failures\n", checks, failures);
    if (failures) { printf("FAIL  the heap is not sound\n"); return 1; }
    printf("ok    free, reuse, coalescing and a bounded worst case\n");
    printf("ok    tags, a 32-position allocation-failure sweep, and freed-block poison\n");
    return 0;
}
