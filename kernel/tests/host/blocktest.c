/* blocktest.c - write-back never becomes surprise foreground I/O. */
#include <stdio.h>
#include <string.h>

typedef unsigned int u32;
static unsigned char disk[512 * 512];
static int writes;
static u32 write_lba[1024];
static u32 fake_tsc;

int nvme_ready(void) { return 1; }
u32 nvme_blocksize(void) { return 512; }
u32 nvme_blocks_lo(void) { return 512; }
u32 cpu_tsc_lo(void) { return fake_tsc += 2300; }
u32 cpu_tsc_khz(void) { return 2300000; }
int nvme_read_to(u32 dst, u32 lba, u32 hi)
{
    (void)hi; memcpy((void *)(unsigned long)dst, disk + lba * 512, 512); return 1;
}
int nvme_write_from(u32 src, u32 lba, u32 hi)
{
    (void)hi; memcpy(disk + lba * 512, (void *)(unsigned long)src, 512);
    write_lba[writes++] = lba; return 1;
}

#include "../../src/drivers/storage/block.c"

static int bad;
static void ok(const char *s, int yes)
{
    printf("  %-66s %s\n", s, yes ? "ok" : "FAIL");
    if (!yes) bad++;
}

int main(void)
{
    static unsigned char a[512], b[512], out[512];
    memset(a, 0x11, sizeof a); memset(b, 0x22, sizeof b);
    ok("write is accepted into WB RAM", block_write(10, a));
    ok("foreground write performs no device I/O", writes == 0 && disk[10 * 512] == 0);
    ok("read-your-write is a cache hit", block_read(10, out) && out[0] == 0x11);
    ok("one service call writes at most one block", block_service() == 1 && writes == 1);
    ok("service published the queued bytes", disk[10 * 512] == 0x11);

    block_write(20, a);
    block_write(21, b);
    block_write(20, b);
    int before = writes;
    block_service();
    ok("dirty sequence preserves data-before-republished-metadata order",
       writes == before + 1 && write_lba[before] == 21);

    block_flush();
    for (u32 i = 100; i < 228; i++)
        if (!block_write(i, a)) bad++;
    ok("a full dirty cache refuses instead of synchronously evicting",
       block_write(300, b) == 0 && block_refusals() > 0);
    int queued = (int)block_dirty_blocks();
    before = writes;
    block_service();
    ok("writeback budget remains exactly one block", writes == before + 1 &&
       block_dirty_blocks() == (u32)(queued - 1));
    ok("forced flush drains the bounded remainder", block_flush() && block_dirty_blocks() == 0);
    ok("cache telemetry reports hits, refusals and latency",
       block_cache_hits() > 0 && block_refusals() > 0 &&
       block_completion_us_max() > 0 && block_forced_syncs() > 0);
    printf("\n%s: %d failure(s)\n", bad ? "FAILED" : "all good", bad);
    return bad ? 1 : 0;
}
