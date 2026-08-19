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

int main(void)
{
    printf("gputest: gpu.c emits the stream the GPU accepted\n\n");
    test_golden();
    test_high_address();
    test_refusals();
    test_overflow();
    test_padding();

    printf("\n  %d checks, %d failures\n", checks, failures);
    if (failures == 0)
        printf("  ok    the kernel emits the bytes that drew on 8086:9B41\n");
    return failures != 0;
}
