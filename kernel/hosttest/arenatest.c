/* arenatest.c - the program arena, asserted.
 *
 * arena.c is compiled here UNMODIFIED - the shipping source, not a copy. It
 * needs exactly two things from outside itself, so the harness supplies them:
 * the memory at the address it hardcodes (mmap MAP_FIXED_NOREPLACE, the same
 * trick fbbench.c:11-12 plays on fb.c) and zl_putc_pub. Everything else is
 * ordinary C against ordinary memory, which is why this gate needs no QEMU and
 * runs in milliseconds.
 *
 * WHAT IS ACTUALLY BEING TESTED, and why each one is here rather than being
 * obvious-by-inspection:
 *
 *   - the ceiling holds, and the refusal PRINTS. "It returns NULL" is half a
 *     gate; this project's documented failure mode is the silent fallback, so
 *     the printed line is the part that matters and the harness captures it.
 *   - a request near ULONG_MAX is refused. Written as `start + want > cap`
 *     that addition wraps and the allocator hands out a pointer to the whole
 *     machine. The request comes from a SCRIPT, so this is reachable input,
 *     not a theoretical one.
 *   - reset gives the space back, and gives back ALL of it. An off-by-one in
 *     the reset leaks the arena one run at a time and nothing notices until
 *     the fourth program refuses to start.
 *   - the memory is really writable end to end. A ceiling test that never
 *     touches the memory passes just as happily on an arena that is not there.
 *
 * Build and run:  ./build.sh && ./arenatest
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <sys/mman.h>

/* Must match arena.c. Duplicated on purpose: if somebody moves the arena and
 * does not move the harness, the mmap fails loudly here rather than the test
 * quietly exercising a different address than the kernel uses. */
#define ARENA_BASE   0x00800000UL
#define ARENA_BYTES  0x01000000UL
#define ARENA_ALIGN  16UL

/* ---- arena.c's interface -------------------------------------------------*/
int   arena_init(void);
int   arena_ok(void);
void *arena_alloc(unsigned long bytes);
void  arena_reset(void);
unsigned long arena_base_addr(void);
unsigned long arena_capacity(void);
unsigned long arena_used(void);
unsigned long arena_high_water(void);
unsigned long arena_refusals(void);
unsigned long arena_resets(void);
unsigned long arena_available(void);

/* ---- the seam: capture what the kernel would have printed ----------------*/
static char  out[1 << 16];
static size_t outn;

void zl_putc_pub(char c)
{
    if (outn < sizeof out - 1) out[outn++] = c;
    out[outn] = 0;
}

static void out_clear(void) { outn = 0; out[0] = 0; }
static int  said(const char *s) { return strstr(out, s) != NULL; }

/* ---- assertions ----------------------------------------------------------*/
static int checks, fails;

static void ok(int cond, const char *what)
{
    checks++;
    if (!cond) { fails++; printf("  FAIL  %s\n", what); }
}

static void okv(int cond, const char *what, unsigned long got, unsigned long want)
{
    checks++;
    if (!cond) {
        fails++;
        printf("  FAIL  %s  (got %lu, wanted %lu)\n", what, got, want);
    }
}

int main(void)
{
    printf("arenatest - the program arena, asserted\n\n");

    /* The memory arena.c hardcodes. MAP_FIXED_NOREPLACE rather than
     * MAP_FIXED: if something is already mapped there we want to be told, not
     * to silently unmap it and test against a lie. */
    void *p = mmap((void *)ARENA_BASE, ARENA_BYTES, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p != (void *)ARENA_BASE) {
        printf("  FAIL  could not map %lu MiB at %#lx - %s\n",
               ARENA_BYTES >> 20, ARENA_BASE, strerror(errno));
        printf("        (vm.mmap_min_addr too high, or something is there)\n");
        return 1;
    }

    /* ---- init ------------------------------------------------------------*/
    out_clear();
    int up = arena_init();
    ok(up == 1, "arena_init reports the arena is up");
    ok(arena_ok() == 1, "arena_ok agrees");
    ok(said("arena:"), "init printed a line saying where the arena is");
    ok(said("8 MiB"), "init printed the base");
    ok(said("16 MiB"), "init printed the size");
    ok(!said("NOT BACKED"), "init did not claim the RAM is missing");
    okv(arena_base_addr() == ARENA_BASE, "base address",
        arena_base_addr(), ARENA_BASE);
    okv(arena_capacity() == ARENA_BYTES, "capacity",
        arena_capacity(), ARENA_BYTES);
    okv(arena_used() == 0, "nothing used yet", arena_used(), 0);
    okv(arena_available() == ARENA_BYTES, "all of it available",
        arena_available(), ARENA_BYTES);

    /* ---- ordinary allocation --------------------------------------------*/
    out_clear();
    char *a = arena_alloc(100);
    char *b = arena_alloc(100);
    ok(a != NULL, "a 100-byte allocation succeeds");
    ok(b != NULL, "a second one succeeds");
    ok(a != b, "two allocations are not the same pointer");
    ok(b >= a + 100, "the second does not overlap the first");
    ok((unsigned long)a >= ARENA_BASE &&
       (unsigned long)a <  ARENA_BASE + ARENA_BYTES, "a is inside the arena");
    ok(((unsigned long)a % ARENA_ALIGN) == 0, "a is 16-byte aligned");
    ok(((unsigned long)b % ARENA_ALIGN) == 0, "b is 16-byte aligned");
    ok(!said("REFUSED"), "an ordinary allocation printed no refusal");

    /* it is real memory, and writing to it does not disturb the neighbour */
    memset(a, 0xAB, 100);
    memset(b, 0xCD, 100);
    ok(a[0] == (char)0xAB && a[99] == (char)0xAB, "a holds what was written");
    ok(b[0] == (char)0xCD && b[99] == (char)0xCD, "b holds what was written");
    ok(a[0] == (char)0xAB, "writing b did not touch a");

    /* ---- zero bytes still gets a distinct address ------------------------*/
    void *z1 = arena_alloc(0);
    void *z2 = arena_alloc(0);
    ok(z1 != NULL && z2 != NULL, "a zero-byte allocation succeeds");
    ok(z1 != z2, "two zero-byte allocations are distinct pointers");

    /* ---- THE OVERFLOW ----------------------------------------------------*/
    out_clear();
    unsigned long before = arena_used();
    void *huge = arena_alloc(ULONG_MAX);
    ok(huge == NULL, "a ULONG_MAX request is refused, not wrapped");
    ok(said("REFUSED"), "the ULONG_MAX refusal printed");
    okv(arena_used() == before, "a refused allocation consumed nothing",
        arena_used(), before);

    void *huge2 = arena_alloc(ARENA_BYTES + 1);
    ok(huge2 == NULL, "capacity+1 is refused");
    void *huge3 = arena_alloc(0xFFFFFFF0UL);
    ok(huge3 == NULL, "a 4 GiB request is refused");

    /* ---- allocate to the ceiling exactly ---------------------------------*/
    arena_reset();
    out_clear();
    void *whole = arena_alloc(ARENA_BYTES);
    ok(whole != NULL, "the WHOLE arena can be allocated in one call");
    okv(arena_used() == ARENA_BYTES, "used is exactly capacity",
        arena_used(), ARENA_BYTES);
    okv(arena_available() == 0, "nothing left", arena_available(), 0);
    ok(!said("REFUSED"), "allocating exactly to the ceiling is NOT a refusal");

    /* and it is all really writable - the last byte especially */
    memset(whole, 0x5A, ARENA_BYTES);
    ok(((unsigned char *)whole)[0] == 0x5A, "first byte of the arena writable");
    ok(((unsigned char *)whole)[ARENA_BYTES - 1] == 0x5A,
       "LAST byte of the arena writable");

    /* ---- one byte more: the refusal, and it PRINTS ------------------------*/
    out_clear();
    void *over = arena_alloc(1);
    ok(over == NULL, "one byte past the ceiling is refused");
    ok(said("REFUSED"), "the refusal PRINTED - not a silent NULL");
    ok(said("ceiling"), "the refusal says why");
    ok(said("16777216"), "the refusal states the ceiling in bytes");
    okv(arena_refusals() >= 1, "the refusal was counted",
        arena_refusals(), 1);

    /* the arena is unchanged by a refusal */
    ok(((unsigned char *)whole)[ARENA_BYTES - 1] == 0x5A,
       "a refused allocation did not scribble on the last byte");
    okv(arena_used() == ARENA_BYTES, "a refusal did not move the bump pointer",
        arena_used(), ARENA_BYTES);

    /* ---- refusals are bounded in VOLUME but never in COUNT ----------------*/
    out_clear();
    unsigned long r0 = arena_refusals();
    for (int i = 0; i < 200; i++) (void)arena_alloc(1);
    okv(arena_refusals() == r0 + 200, "every refusal was counted",
        arena_refusals(), r0 + 200);
    ok(said("suppressed"), "the suppression notice printed");
    ok(outn < 4096, "200 refusals did not print 200 lines");

    /* ---- reset: the space comes back -------------------------------------*/
    out_clear();
    unsigned long hw = arena_high_water();
    okv(hw == ARENA_BYTES, "high water recorded the full arena", hw, ARENA_BYTES);

    arena_reset();
    okv(arena_used() == 0, "reset zeroed the bump pointer", arena_used(), 0);
    okv(arena_available() == ARENA_BYTES, "reset gave back ALL of it",
        arena_available(), ARENA_BYTES);
    okv(arena_high_water() == hw, "reset did not erase the high water mark",
        arena_high_water(), hw);
    okv(arena_refusals() == 0, "reset cleared the per-run refusal count",
        arena_refusals(), 0);

    void *again = arena_alloc(ARENA_BYTES);
    ok(again != NULL, "the whole arena allocates AGAIN after a reset");
    okv((unsigned long)again == ARENA_BASE,
        "and it comes back at the base", (unsigned long)again, ARENA_BASE);
    ok(!said("REFUSED"), "no refusal on the second full allocation");

    /* ---- reset is idempotent and repeatable ------------------------------*/
    for (int i = 0; i < 50; i++) {
        arena_reset();
        void *q = arena_alloc(ARENA_BYTES);
        if (q == NULL) { ok(0, "50 reset/allocate cycles all succeed"); break; }
        if (i == 49) ok(1, "50 reset/allocate cycles all succeed");
    }
    okv(arena_resets() >= 50, "resets were counted", arena_resets(), 50);

    /* ---- many small allocations tile without gaps or overlap -------------*/
    arena_reset();
    out_clear();
    {
        const int N = 1000;
        unsigned char *prev = NULL;
        int overlap = 0, unaligned = 0;
        for (int i = 0; i < N; i++) {
            unsigned char *q = arena_alloc(64);
            if (!q) { overlap = 1; break; }
            if ((unsigned long)q % ARENA_ALIGN) unaligned = 1;
            if (prev && q < prev + 64) overlap = 1;
            memset(q, i & 0xFF, 64);
            prev = q;
        }
        ok(!overlap, "1000 x 64 bytes never overlap");
        ok(!unaligned, "1000 x 64 bytes are all aligned");
        okv(arena_used() == (unsigned long)N * 64UL,
            "64 is a multiple of 16 so there is no padding waste",
            arena_used(), (unsigned long)N * 64UL);
    }

    /* ---- padding is charged, and cannot smuggle past the ceiling ---------*/
    arena_reset();
    {
        void *q1 = arena_alloc(1);          /* uses 1, bumps to 16 next time */
        ok(q1 != NULL, "a 1-byte allocation succeeds");
        okv(arena_used() == 1, "1 byte charges 1 byte", arena_used(), 1);
        void *q2 = arena_alloc(ARENA_BYTES - 1);
        ok(q2 == NULL,
           "capacity-1 after a 1-byte allocation is refused - padding counted");
        void *q3 = arena_alloc(ARENA_BYTES - ARENA_ALIGN);
        ok(q3 != NULL, "capacity-16 after a 1-byte allocation fits exactly");
        okv(arena_available() == 0, "and lands exactly on the ceiling",
            arena_available(), 0);
    }

    printf("\n%d checks, %d failures\n", checks, fails);
    if (fails == 0) printf("ok    the arena holds its ceiling and gives it back\n");
    return fails ? 1 : 0;
}
