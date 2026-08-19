/* gputest.c - pin gpu.c's command stream to the bytes the hardware accepted.
 *
 * gpu_blt.c proves the encoding on the real GPU, but only for one rectangle
 * and only on a machine that has an Intel GPU. This asserts the same bytes on
 * any machine, in milliseconds, and covers the paths hardware cannot reach
 * cheaply: the refusals, and the overflow.
 *
 * THE GOLDEN DWORDS BELOW ARE NOT A GUESS. They were read out of the batch
 * that filled 270000/270000 pixels of a rectangle on 8086:9B41 on 2026-08-19
 * (kernel/docs/gpu-blitter.md). If a change to gpu.c makes this file fail, the
 * change altered a command stream that is known to work on silicon - so the
 * burden is on the change, not on this test.
 *
 * The overflow case is the one that matters most for the kernel. A batch
 * buffer overrun means the GPU parses whatever happened to follow it in
 * memory, which on a machine with no MMU protecting us is arbitrary
 * execution by a DMA engine. So it is checked with a canary, not just a
 * return value.
 */
#include <stdio.h>
#include <string.h>

#include "../gpu.c"

/* The ring arithmetic. gpuring.c's MMIO half needs intel.c, and deliberately
 * does NOT get it: these stubs report no GPU, so gpu_ring_init() refuses and
 * not one register write is reachable from this harness. That is the honest
 * arrangement - the arithmetic below is tested, the MMIO is not, and pretending
 * otherwise by stubbing a fake register file would be a test that proves the
 * code runs against a model of the hardware rather than the hardware. */
static int    intel_present(void)   { return 0; }
static int    intel_supported(void) { return 0; }
static unsigned intel_mmio(void)    { return 0; }
__attribute__((unused)) static unsigned intel_ggtt_size(void) { return 0; }
static int    intel_ggtt_map(unsigned p, unsigned a) { (void)p; (void)a; return 0; }
#include "../gpuring.c"

static int failures = 0;
static int checks = 0;

static void ok(int cond, const char *what)
{
    checks++;
    if (!cond) { printf("  FAIL  %s\n", what); failures++; }
}

static void eq32(gpu_u32 got, gpu_u32 want, const char *what)
{
    checks++;
    if (got != want) {
        printf("  FAIL  %s: got 0x%08X want 0x%08X\n", what, got, want);
        failures++;
    }
}

/* ---- 1. the exact stream the GPU accepted --------------------------------- */

static void test_golden(void)
{
    /* The parameters gpu_blt.c ran: rect (100,50)..(700,500), 32bpp surface,
     * pitch 15360 bytes, destination graphics address 0x200000, colour
     * 0x60D2EB. */
    gpu_u32 buf[16];
    struct gpu_batch b;
    memset(buf, 0xAA, sizeof buf);
    gpu_batch_init(&b, buf, 16);

    ok(gpu_fill_rect(&b, 0x200000ull, 15360, 100, 50, 700, 500, 0x60D2EBu),
       "gpu_fill_rect accepted the verified rectangle");
    ok(gpu_batch_end(&b), "gpu_batch_end closed it");

    /* DW0 0x54300005: client 2 | opcode 0x50 | write-alpha | write-rgb | len 5.
     * This is the value printed by the run that drew the rectangle. If bit 20
     * (write-rgb) is ever lost, the hardware silently writes nothing - the
     * failure --negative plants on purpose. */
    eq32(buf[0], 0x54300005u, "DW0 opcode word");
    ok((buf[0] & GPU_BLT_WRITE_RGB) != 0, "DW0 still sets BLT_WRITE_RGB");

    /* BR13 0x03F03C00: 32bpp (3<<24) | PATCOPY (0xF0<<16) | pitch 15360. */
    eq32(buf[1], 0x03F03C00u, "DW1 BR13");
    eq32(buf[1] & 0xFFFFu, 15360u, "BR13 carries the pitch in BYTES");

    eq32(buf[2], (50u << 16) | 100u, "DW2 top-left  y<<16|x");
    eq32(buf[3], (500u << 16) | 700u, "DW3 bottom-right y<<16|x");
    eq32(buf[4], 0x00200000u, "DW4 destination address low");
    eq32(buf[5], 0x00000000u, "DW5 destination address high");
    eq32(buf[6], 0x60D2EBu, "DW6 colour");
    eq32(buf[7], 0x05000000u, "DW7 MI_BATCH_BUFFER_END");

    /* 7 dwords of command + 1 end = 8, already even, so no pad is added. */
    ok(gpu_batch_bytes(&b) == 32, "batch is 32 bytes");
}

/* A 64-bit graphics address must split across DW4/DW5 rather than truncating -
 * the LLP64 lesson from the same day, in a different place. */
static void test_high_address(void)
{
    gpu_u32 buf[16];
    struct gpu_batch b;
    gpu_batch_init(&b, buf, 16);
    ok(gpu_fill_rect(&b, 0x1234567890ull, 4096, 0, 0, 8, 8, 0), "high address accepted");
    eq32(buf[4], 0x34567890u, "low  half of a >4 GiB address");
    eq32(buf[5], 0x00000012u, "high half of a >4 GiB address");
}

/* ---- 1b. the copy, pinned to what really copied ---------------------------- */

static void test_copy_golden(void)
{
    /* The parameters gpu_blt --copy ran when it matched 786432/786432 source
     * pixels on 8086:9B41: dst rect (64,32)..(1088,800), src origin (16,8),
     * both pitches 15360. */
    gpu_u32 buf[24];
    struct gpu_batch b;
    memset(buf, 0xAA, sizeof buf);
    gpu_batch_init(&b, buf, 24);

    ok(gpu_copy_rect(&b, 0x200000ull, 15360, 64, 32, 1088, 800,
                     0x10000000ull, 15360, 16, 8), "gpu_copy_rect accepted it");

    eq32(buf[0], 0x54F00008u, "copy DW0 opcode word");
    /* SRCCOPY, not PATCOPY. PATCOPY here ignores the source entirely and fills
     * with a colour - a copy that silently produces a solid rectangle, and it
     * passes any test whose source is uniform. */
    eq32(buf[1], 0x03CC3C00u, "copy DW1 BR13 (ROP must be SRCCOPY 0xCC)");
    ok(((buf[1] >> 16) & 0xFFu) == 0xCCu, "ROP is SRCCOPY");
    eq32(buf[2], (32u << 16) | 64u,    "dst top-left");
    eq32(buf[3], (800u << 16) | 1088u, "dst bottom-right");
    eq32(buf[4], 0x00200000u, "dst address low");
    eq32(buf[5], 0u,          "dst address high");
    eq32(buf[6], (8u << 16) | 16u, "SRC top-left - a copy from the wrong origin still copies");
    eq32(buf[7], 15360u,      "SRC pitch is its own field, not the dst pitch");
    eq32(buf[8], 0x10000000u, "src address low");
    eq32(buf[9], 0u,          "src address high");
    ok(gpu_batch_end(&b), "closed");
    ok(gpu_batch_bytes(&b) % 8 == 0, "copy batch is a multiple of 8 bytes");
}

static void test_copy_refusals(void)
{
    gpu_u32 buf[64];
    struct gpu_batch b;
#define FRESH() gpu_batch_init(&b, buf, 64)
    FRESH(); ok(!gpu_copy_rect(&b, 0x1000, 0, 0,0,8,8, 0x2000, 4096, 0,0), "dst pitch 0 refused");
    FRESH(); ok(!gpu_copy_rect(&b, 0x1000, 4096, 0,0,8,8, 0x2000, 0, 0,0), "src pitch 0 refused");
    FRESH(); ok(!gpu_copy_rect(&b, 0x1000, 4096, 8,0,8,8, 0x2000, 4096, 0,0), "empty dst rect refused");
    FRESH(); ok(!gpu_copy_rect(&b, 0x1000, 4096, 0,0,8,8, 0x2000, 4096, -1,0), "negative src x refused");
    /* A source rectangle that would run past the 16-bit coordinate field wraps
     * in hardware rather than clipping, so it must be refused here. */
    FRESH(); ok(!gpu_copy_rect(&b, 0x1000, 4096, 0,0,64,8, 0x2000, 4096, 0xFFF0,0),
                "src rect running past the coord field refused");
#undef FRESH
}

/* ---- 2. the refusals ------------------------------------------------------ */

static void test_refusals(void)
{
    gpu_u32 buf[64];
    struct gpu_batch b;

#define FRESH() gpu_batch_init(&b, buf, 64)

    FRESH(); ok(!gpu_fill_rect(&b, 0x1000, 0,      0, 0, 8, 8, 0), "pitch 0 refused");
    FRESH(); ok(!gpu_fill_rect(&b, 0x1000, 0x10000, 0, 0, 8, 8, 0),
                "pitch past BR13's 16-bit field refused (would wrap, not truncate visibly)");
    FRESH(); ok(!gpu_fill_rect(&b, 0x1000, 4096,  8, 0, 8, 8, 0), "empty rect (x2==x1) refused");
    FRESH(); ok(!gpu_fill_rect(&b, 0x1000, 4096,  9, 0, 8, 8, 0), "inverted rect refused");
    FRESH(); ok(!gpu_fill_rect(&b, 0x1000, 4096, -1, 0, 8, 8, 0),
                "negative x refused (the hardware reads coords unsigned)");
    FRESH(); ok(!gpu_fill_rect(&b, 0x1000, 4096,  0, 0, 0x10000, 8, 0), "x2 past 16 bits refused");

    /* A refusal must not leave anything behind. */
    FRESH();
    gpu_fill_rect(&b, 0x1000, 0, 0, 0, 8, 8, 0);
    ok(gpu_batch_bytes(&b) == 0, "a refused rectangle emits no dwords");
    ok(!gpu_batch_overflowed(&b), "a refused rectangle is not an overflow");
#undef FRESH
}

/* ---- 3. overflow, with a canary ------------------------------------------- */

static void test_overflow(void)
{
    /* Capacity is deliberately not a multiple of the 7-dword command, so the
     * last attempt lands mid-command - the case where a bounds check applied
     * per-dword instead of per-command would write a partial command and then
     * stop. */
    gpu_u32 storage[32];
    const gpu_u32 CANARY = 0xC0FFEE00u;
    for (unsigned i = 0; i < 32; i++) storage[i] = CANARY;

    struct gpu_batch b;
    gpu_batch_init(&b, storage, 10);          /* only 10 of the 32 usable */

    ok(gpu_fill_rect(&b, 0x1000, 4096, 0, 0, 8, 8, 1), "first fill fits (7 of 10)");
    ok(!gpu_fill_rect(&b, 0x1000, 4096, 0, 0, 8, 8, 2), "second fill refused (needs 7, 3 left)");
    ok(gpu_batch_overflowed(&b), "overflow flag set");
    ok(gpu_batch_bytes(&b) == 0, "an overflowed batch reports 0 bytes, never a partial length");

    /* Nothing beyond the declared capacity may have been touched. This is the
     * assertion that a return-value check alone would not make. */
    int clobbered = 0;
    for (unsigned i = 10; i < 32; i++) if (storage[i] != CANARY) clobbered++;
    ok(clobbered == 0, "nothing was written past the batch capacity");

    /* And the partial command must not be there either: dwords 7..9 are inside
     * capacity but were never legitimately claimed. */
    int partial = 0;
    for (unsigned i = 7; i < 10; i++) if (storage[i] != CANARY) partial++;
    ok(partial == 0, "no partial command was left in the tail of the batch");

    /* Once overflowed, it stays overflowed - a caller that ignores one return
     * value must not get a valid-looking batch from the next call. */
    ok(!gpu_batch_end(&b), "gpu_batch_end refuses an overflowed batch");
}

/* ---- 4. padding ----------------------------------------------------------- */

static void test_padding(void)
{
    /* One fill is 7 dwords; + END = 8, even, no pad. Two fills = 14; + END =
     * 15, odd, so a second END is appended to reach a multiple of 8 bytes,
     * which is what the submission path requires. */
    gpu_u32 buf[64];
    struct gpu_batch b;
    gpu_batch_init(&b, buf, 64);
    gpu_fill_rect(&b, 0x1000, 4096, 0, 0, 8, 8, 0);
    gpu_fill_rect(&b, 0x1000, 4096, 0, 0, 8, 8, 0);
    ok(gpu_batch_end(&b), "two fills closed");
    ok(gpu_batch_bytes(&b) % 8 == 0, "batch length is a multiple of 8 bytes");
    eq32(buf[14], 0x05000000u, "first END");
    eq32(buf[15], 0x05000000u, "pad is a second END, not a NOOP");
}

/* ---- 5. the ring bookkeeping ----------------------------------------------
 *
 * gpuring.c's MMIO has never run on hardware and cannot be tested here. Its
 * ARITHMETIC can, and that is the half which fails quietly: an off-by-one in
 * the space calculation overwrites commands the engine has not read yet, and
 * the symptom is a GPU executing garbage rather than an error.
 */
#define RING 64u          /* a tiny ring, so the wrap is reached in a few writes */

static void test_ring_space(void)
{
    /* Empty ring: everything free except the reserved qword that keeps
     * "full" and "empty" distinguishable. */
    ok(gpu_ring_space(0, 0, RING) == RING - 8u, "empty ring: size minus the reserved qword");

    /* Engine at 0, we have written 16 bytes. */
    ok(gpu_ring_space(0, 16, RING) == RING - 16u - 8u, "16 bytes written");

    /* Wrapped: tail behind head. */
    ok(gpu_ring_space(32, 16, RING) == 32u - 16u - 8u, "tail behind head (wrapped)");

    /* Nearly full must NOT report unlimited. This is the unsigned-underflow
     * trap: `free - 8` with free < 8 wraps to about four billion, and every
     * subsequent submission then believes it fits. */
    for (gpu_u32 t = 0; t < RING; t += 4) {
        gpu_u32 sp = gpu_ring_space((t + 4u) % RING, t, RING);
        checks++;
        if (sp > RING) { printf("  FAIL  space %u > ring %u at tail %u\n", sp, RING, t); failures++; }
    }
    ok(gpu_ring_space(8, 4, RING) == 0, "a gap smaller than the reserved qword reports 0, not 4 billion");
    ok(gpu_ring_space(0, 0, 0) == 0, "zero-sized ring reports no space");
}

static void test_ring_write_wraps(void)
{
    gpu_u32 ring[RING / 4];
    for (unsigned i = 0; i < RING / 4; i++) ring[i] = 0xEEEEEEEEu;

    /* Start near the end so the copy has to wrap. */
    gpu_u32 dw[6] = { 1, 2, 3, 4, 5, 6 };
    gpu_u32 tail = gpu_ring_write(ring, RING, RING - 8u, dw, 6);

    ok(tail == 16u, "wrapped write leaves the tail past the start");
    eq32(ring[(RING - 8u) / 4u], 1u, "first dword at the old tail");
    eq32(ring[(RING - 4u) / 4u], 2u, "second dword in the last slot");
    eq32(ring[0], 3u, "third dword wrapped to the start");
    eq32(ring[3], 6u, "sixth dword in order after the wrap");
}

static void test_ring_pad(void)
{
    gpu_u32 ring[RING / 4];
    for (unsigned i = 0; i < RING / 4; i++) ring[i] = 0xEEEEEEEEu;

    /* An odd dword count leaves the tail 4-byte aligned; the hardware needs 8. */
    gpu_u32 t = gpu_ring_pad(ring, RING, 4u);
    ok(t == 8u, "pad advances a 4-aligned tail to 8");
    eq32(ring[1], 0u, "the pad is MI_NOOP (0), not left as stale data");

    ok(gpu_ring_pad(ring, RING, 8u) == 8u, "an already-aligned tail is not moved");

    /* Padding at the very end must wrap rather than write off the end. */
    gpu_u32 w = gpu_ring_pad(ring, RING, RING - 4u);
    ok(w == 0u, "a pad at the last dword wraps to 0");
    eq32(ring[(RING - 4u) / 4u], 0u, "and wrote its NOOP in the last slot");
}

int main(void)
{
    printf("gputest: gpu.c emits the stream the GPU accepted\n\n");
    test_golden();
    test_copy_golden();
    test_copy_refusals();
    test_high_address();
    test_refusals();
    test_overflow();
    test_padding();
    test_ring_space();
    test_ring_write_wraps();
    test_ring_pad();

    printf("\n  %d checks, %d failures\n", checks, failures);
    if (failures == 0)
        printf("  ok    the kernel emits the bytes that drew on 8086:9B41\n");
    return failures != 0;
}
