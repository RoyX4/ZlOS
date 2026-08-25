/* block.c - bounded write-back cache and the ordinary block-device seam.
 *
 * Foreground filesystem mutations copy one logical block into WB RAM and
 * return.  The main loop calls block_service() outside wm_frame(), which
 * writes at most one dirty block.  Dirty sequence order preserves the zlfs
 * data-before-directory publication order.  A full dirty cache refuses new
 * work; it never performs surprise I/O in an input or paint callback. */

typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned char u8;

#define BLOCK_CACHE_N 128
#define BLOCK_BYTES_MAX 4096

extern int nvme_ready(void);
extern int nvme_read_to(u32 dst, u32 lba_lo, u32 lba_hi);
extern int nvme_write_from(u32 src, u32 lba_lo, u32 lba_hi);
extern u32 nvme_blocksize(void);
extern u32 nvme_blocks_lo(void);
extern u32 cpu_tsc_lo(void);
extern u32 cpu_tsc_khz(void);

#if defined(ZL_64)
typedef unsigned long long uptr;
#else
typedef unsigned int uptr;
#endif

struct cache_page {
    u32 lba, seq, age;
    u8 valid, dirty;
    u8 data[BLOCK_BYTES_MAX];
};

static struct cache_page pages[BLOCK_CACHE_N];
static u32 clock_hand, write_seq;
static u32 hits, misses, refusals, dirty_n, dirty_peak;
static u32 completions, completion_us_max, writeback_us;
static u32 forced_syncs;

static void copy_n(void *dst, const void *src, u32 n)
{
    u8 *d = (u8 *)dst; const u8 *s = (const u8 *)src;
    while (n--) *d++ = *s++;
}

static int dma32(void *p, u32 *out)
{
    uptr a = (uptr)p;
#if defined(ZL_64)
    if ((u64)a > 0xffffffffULL) return 0;
#endif
    *out = (u32)a;
    return 1;
}

static int find_page(u32 lba)
{
    for (int i = 0; i < BLOCK_CACHE_N; i++)
        if (pages[i].valid && pages[i].lba == lba) return i;
    return -1;
}

static int take_page(void)
{
    int best = -1;
    u32 best_age = 0xffffffffu;
    for (int i = 0; i < BLOCK_CACHE_N; i++) {
        if (!pages[i].valid) return i;
        if (!pages[i].dirty && pages[i].age <= best_age) {
            best = i; best_age = pages[i].age;
        }
    }
    return best;
}

int block_read(u32 lba, void *buf)
{
    u32 bs = nvme_blocksize();
    if (!nvme_ready() || !buf || !bs || bs > BLOCK_BYTES_MAX ||
        lba >= nvme_blocks_lo()) return 0;
    int at = find_page(lba);
    if (at >= 0) {
        hits++;
        pages[at].age = ++clock_hand;
        copy_n(buf, pages[at].data, bs);
        return 1;
    }
    misses++;
    at = take_page();
    if (at < 0) { refusals++; return 0; }
    u32 addr;
    if (!dma32(pages[at].data, &addr) || !nvme_read_to(addr, lba, 0)) return 0;
    pages[at].valid = 1; pages[at].dirty = 0;
    pages[at].lba = lba; pages[at].age = ++clock_hand;
    copy_n(buf, pages[at].data, bs);
    return 1;
}

int block_write(u32 lba, const void *buf)
{
    u32 bs = nvme_blocksize();
    if (!nvme_ready() || !buf || !bs || bs > BLOCK_BYTES_MAX ||
        lba >= nvme_blocks_lo()) return 0;
    int at = find_page(lba);
    if (at < 0) at = take_page();
    if (at < 0) { refusals++; return 0; }
    if (!pages[at].dirty) {
        dirty_n++;
        if (dirty_n > dirty_peak) dirty_peak = dirty_n;
    }
    copy_n(pages[at].data, buf, bs);
    pages[at].valid = 1; pages[at].dirty = 1;
    pages[at].lba = lba; pages[at].age = ++clock_hand;
    pages[at].seq = ++write_seq;
    return 1;
}

int block_service(void)
{
    int at = -1;
    u32 seq = 0xffffffffu;
    for (int i = 0; i < BLOCK_CACHE_N; i++)
        if (pages[i].dirty && pages[i].seq <= seq) { at = i; seq = pages[i].seq; }
    if (at < 0) return 0;
    u32 addr;
    if (!dma32(pages[at].data, &addr)) { refusals++; return -1; }
    u32 begin = cpu_tsc_lo();
    if (!nvme_write_from(addr, pages[at].lba, 0)) { refusals++; return -1; }
    u32 khz = cpu_tsc_khz();
    u32 us = khz >= 1000 ? (cpu_tsc_lo() - begin) / (khz / 1000) : 0;
    pages[at].dirty = 0;
    if (dirty_n) dirty_n--;
    completions++;
    writeback_us += us;
    if (us > completion_us_max) completion_us_max = us;
    return 1;
}

int block_flush(void)
{
    forced_syncs++;
    for (int guard = 0; guard < BLOCK_CACHE_N; guard++) {
        int r = block_service();
        if (r == 0) return 1;
        if (r < 0) return 0;
    }
    return dirty_n == 0;
}

u32 block_cache_hits(void) { return hits; }
u32 block_cache_misses(void) { return misses; }
u32 block_dirty_blocks(void) { return dirty_n; }
u32 block_dirty_peak(void) { return dirty_peak; }
u32 block_refusals(void) { return refusals; }
u32 block_completions(void) { return completions; }
u32 block_completion_us_max(void) { return completion_us_max; }
u32 block_writeback_us(void) { return writeback_us; }
u32 block_forced_syncs(void) { return forced_syncs; }
