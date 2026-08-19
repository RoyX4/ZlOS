/* fs.c - zlfs, the smallest thing that is honestly a filesystem.
 *
 * Until now every file in zlOS lived in ten numbered RAM slots with no names
 * and no persistence: `fs_list()` printed "file 0: empty" and a power cycle
 * erased the lot. That is raw memory with an index, and it is the loudest
 * "this is not an OS yet" signal in the product.
 *
 * This is not ext4 and does not want to be. It is three ideas:
 *
 *   a SUPERBLOCK at a fixed LBA   magic, version, geometry, checksum
 *   a flat DIRECTORY              fixed-size entries, no subdirectories
 *   files as CONTIGUOUS RUNS      no fragmentation, no free list, no extents
 *
 * Contiguous runs are "wrong" for a real disk and exactly right for this one.
 * A file that outgrows its run is COPIED to a bigger one, which costs a memcpy
 * on a device that does not care and buys the absence of every allocator bug
 * there is. There is no heap here, and there is not going to be one: the file
 * count is fixed, the directory is a fixed array, and "no free slot" is a
 * refusal that PRINTS rather than a silent drop. That rule has been broken
 * twice in this project already.
 *
 * WHY THIS FILE CAN BE TESTED WITHOUT BOOTING ANYTHING
 * ----------------------------------------------------
 * Everything below talks to storage through exactly three functions -
 * dev_read, dev_write, dev_geom. In the kernel they are nvme.c. Under
 * -DFS_HOSTTEST they are whatever hosttest/fstest.c supplies, which is a RAM
 * array that can be made to fail a write on command. A filesystem's real bugs
 * are "the write that half-landed" and "the second create that lands on top of
 * the first file", and neither of those is findable by looking at a screen.
 *
 * THE RULE THIS FILE EXISTS TO ENFORCE
 * ------------------------------------
 * A filesystem that silently treats garbage as data is worse than one that
 * will not mount. Every refusal here says why, out loud, with the value that
 * caused it.
 */

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned short     u16;
typedef unsigned char      u8;

#if defined(ZL_64)
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

/* the one character sink - runtime_kernel.c in the kernel, the harness on the
 * host. A driver that cannot say why it refused is a driver that gets blamed
 * for the wrong thing. */
void zl_putc_pub(char c);

/* ---- geometry ------------------------------------------------------------
 * FS_BLK_MAX is a real limit, not a guess: nvme.c issues every transfer with
 * PRP1 only and no PRP2, which means one 4 KiB page per command. A device
 * reporting a larger logical block would DMA past that page. Refusing is the
 * only honest answer. */
#define FS_BLK_MAX     4096
#define FS_MAGIC       0x5A4C4653u        /* 'ZLFS', little end first        */
#define FS_VERSION     1
#define FS_MAXFILES    32
#define FS_ENT_BYTES   64                 /* power of two: index is a shift  */
#define FS_NAME_MAX    24                 /* including the terminating NUL   */
#define FS_DIR_BYTES   (FS_MAXFILES * FS_ENT_BYTES)   /* 2048               */

/* The volume starts 1 MiB in - the conventional first-partition offset, so if
 * a partition table is ever put on this disk we are already where one would be
 * expected, and LBA 0 stays untouched either way.
 *
 * Expressed in BYTES rather than as a fixed LBA 2048, because 2048 is only
 * 1 MiB on a 512-byte-block device; on a 4096-byte one it is 8 MiB in, which
 * silently refused to format any disk smaller than that. The host harness
 * caught it on the second geometry it tried. */
#define FS_START_BYTES (1024u * 1024u)

#define FE_USED        (1u << 0)

/* ---- what a directory entry is, byte for byte ----------------------------
 * Held as raw bytes rather than a struct so the on-disk layout cannot drift
 * with a compiler's padding rules across the 32-bit, 64-bit and EFI builds -
 * three different targets compile this file, and a struct that packs
 * differently in one of them is a disk that only mounts on two of them.
 *
 *   0..23   name, NUL padded
 *   24..27  start LBA, absolute
 *   28..31  length in bytes
 *   32..35  blocks reserved for the run
 *   36..39  flags
 *   40..43  mtime (seconds; 0 until the RTC lands - item 3)
 *   44..63  reserved, must be zero
 */
#define FE_NAME    0
#define FE_START   24
#define FE_LEN     28
#define FE_BLOCKS  32
#define FE_FLAGS   36
#define FE_MTIME   40

/* ---- module state -------------------------------------------------------
 * A few KB in BSS, which is fine - the fixed-high-RAM rule is about the
 * multi-megabyte buffers that collide with the DMA arena, not about this. */
static u8  dirbuf[FS_DIR_BYTES];          /* the whole directory, in memory  */
static u8  blkbuf[FS_BLK_MAX];            /* one block, for I/O staging      */
static u8  sbbuf[FS_BLK_MAX];             /* the superblock block            */

static int  mounted;
static u32  dev_bsize;
static u32  dev_nblocks;
static u32  sb_dir_lba, sb_dir_blocks;
static u32  sb_data_lba, sb_data_blocks;
static u32  now_secs;                     /* set by fs_set_time()            */

/* the name being staged one character at a time, because the zl kernel subset
 * has no string VALUES - the same seam term.c uses for typed commands */
static char stage[FS_NAME_MAX];
static int  stage_len;

/* ---- the smallest possible libc ----------------------------------------- */
static void bzero_n(void *p, u32 n)   { u8 *d = (u8 *)p; while (n--) *d++ = 0; }
static void bcopy_n(void *d, const void *s, u32 n)
{
    u8 *dd = (u8 *)d; const u8 *ss = (const u8 *)s;
    if (dd < ss) { while (n--) *dd++ = *ss++; }
    else { dd += n; ss += n; while (n--) *--dd = *--ss; }
}
static int nameeq(const char *a, const char *b)
{
    for (int i = 0; i < FS_NAME_MAX; i++) {
        if (a[i] != b[i]) return 0;
        if (a[i] == 0)    return 1;
    }
    return 1;
}

static u32 rd32(const u8 *p) { return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) | ((u32)p[3] << 24); }
static void wr32b(u8 *p, u32 v) { p[0] = (u8)v; p[1] = (u8)(v >> 8); p[2] = (u8)(v >> 16); p[3] = (u8)(v >> 24); }

static void p_str(const char *s) { while (*s) zl_putc_pub(*s++); }
static void p_u32(u32 v)
{
    char b[12]; int i = 0;
    if (!v) { zl_putc_pub('0'); return; }
    while (v) { b[i++] = (char)('0' + (v % 10)); v /= 10; }
    while (i) zl_putc_pub(b[--i]);
}
static void p_hex(u32 v)
{
    p_str("0x");
    for (int s = 28; s >= 0; s -= 4) {
        int d = (int)((v >> s) & 0xF);
        zl_putc_pub((char)(d < 10 ? '0' + d : 'a' + d - 10));
    }
}
static void p_name(const char *n)
{
    for (int i = 0; i < FS_NAME_MAX && n[i]; i++) zl_putc_pub(n[i]);
}

/* ---- the block device seam ----------------------------------------------
 * Three functions. Everything above this line is storage-agnostic, which is
 * what lets the whole filesystem be exercised on the host in milliseconds
 * against a RAM array that can be told to fail. */
#ifdef FS_HOSTTEST
int  fsdev_read (u32 lba, void *buf);
int  fsdev_write(u32 lba, const void *buf);
u32  fsdev_bsize(void);
u32  fsdev_blocks(void);
#else
extern int nvme_ready(void);
extern int nvme_setup(void);
extern int nvme_read_to(u32 dst, u32 lba_lo, u32 lba_hi);
extern int nvme_write_from(u32 src, u32 lba_lo, u32 lba_hi);
extern u32 nvme_blocksize(void);
extern u32 nvme_blocks_lo(void);

/* An address truncated to 32 bits is this project's recurring bug, five times
 * so far, and it reads as a protocol bug every time. On the 64-bit and EFI
 * builds these buffers are BSS in a kernel linked low, so this never fires -
 * but "never fires" is a thing to ASSERT, not to assume. */
static int ptr32(const void *p, u32 *out)
{
    uptr a = (uptr)p;
#if defined(ZL_64)
    /* guarded by #if rather than `sizeof(uptr) > 4 &&` because on the 32-bit
     * target that comparison is always false and -Wextra says so, and a new
     * warning in a build that has none is how a real one gets missed */
    if ((u64)a > 0xFFFFFFFFull) {
        p_str("  zlfs: buffer above 4 GiB, refusing DMA\n");
        return 0;
    }
#endif
    *out = (u32)a;
    return 1;
}

int fsdev_read(u32 lba, void *buf)
{
    u32 a;
    if (!nvme_ready() || !ptr32(buf, &a)) return 0;
    return nvme_read_to(a, lba, 0);
}
int fsdev_write(u32 lba, const void *buf)
{
    u32 a;
    if (!nvme_ready() || !ptr32(buf, &a)) return 0;
    return nvme_write_from(a, lba, 0);
}
u32 fsdev_bsize(void)  { return nvme_blocksize(); }
u32 fsdev_blocks(void) { return nvme_blocks_lo(); }
#endif

/* ---- the superblock ------------------------------------------------------
 * Checksummed over the WHOLE block, not just the header. A header-only
 * checksum passes on a block whose tail is garbage, and the tail is exactly
 * where a half-completed write leaves garbage.
 *
 * The field holds the two's complement of the sum of every other word, so a
 * good superblock sums to zero and the check is one comparison with no
 * special case for the checksum's own position.
 */
#define SB_MAGIC    0
#define SB_VERSION  4
#define SB_BSIZE    8
#define SB_BLOCKS   12
#define SB_DIRLBA   16
#define SB_DIRBLK   20
#define SB_DATALBA  24
#define SB_DATABLK  28
#define SB_MAXFILES 32
#define SB_CSUM     36

static u32 sb_sum(const u8 *b, u32 bsize)
{
    u32 s = 0;
    for (u32 i = 0; i + 4 <= bsize; i += 4) s += rd32(b + i);
    return s;
}

static u32 dir_blocks_for(u32 bsize)
{
    return (FS_DIR_BYTES + bsize - 1) / bsize;
}

/* where the volume begins, in blocks. Derived, never stored: a superblock
 * that disagrees with this is a superblock we refuse. */
static u32 start_lba(void) { return FS_START_BYTES / dev_bsize; }

/* ---- directory helpers --------------------------------------------------- */
static u8  *ent(int i)          { return dirbuf + (u32)i * FS_ENT_BYTES; }
static int  ent_used(int i)     { return (rd32(ent(i) + FE_FLAGS) & FE_USED) != 0; }
static u32  ent_start(int i)    { return rd32(ent(i) + FE_START); }
static u32  ent_blocks(int i)   { return rd32(ent(i) + FE_BLOCKS); }
static u32  ent_len(int i)      { return rd32(ent(i) + FE_LEN); }

/* Divide FIRST, then round up. The obvious `(bytes + dev_bsize - 1) / dev_bsize`
 * wraps for the top 511 values of a u32 at 512-byte blocks and returns ZERO,
 * and a zero-block run is not a small bug: alloc_run hands out a run of length
 * 0 on top of a live file, the mount-time range check then rejects the whole
 * volume forever, and alloc_run's cursor stops advancing so the NEXT create
 * spins in an infinite loop. One overflow, three symptoms, every other file on
 * the disk lost. Found by an adversarial reviewer, not by this file's author. */
static u32 blocks_for(u32 bytes)
{
    if (bytes == 0) return 1;                   /* every file owns >= 1 block */
    u32 n = bytes / dev_bsize;                  /* cannot overflow            */
    if (bytes % dev_bsize) n++;                 /* ...nor can this            */
    return n;
}

static int dir_flush(void)
{
    for (u32 b = 0; b < sb_dir_blocks; b++) {
        bzero_n(blkbuf, dev_bsize);
        u32 off = b * dev_bsize;
        u32 n   = FS_DIR_BYTES - off;
        if (n > dev_bsize) n = dev_bsize;
        if (off < FS_DIR_BYTES) bcopy_n(blkbuf, dirbuf + off, n);
        if (!fsdev_write(sb_dir_lba + b, blkbuf)) {
            p_str("  zlfs: directory write failed at LBA ");
            p_u32(sb_dir_lba + b); zl_putc_pub('\n');
            return 0;
        }
    }
    return 1;
}

static u8 *ent(int i);

/* Publish one changed entry, or leave the volume exactly as it was.
 *
 * dir_flush() writes several blocks from one in-memory image, so a failure
 * part-way through leaves the ON-DISK directory a mixture: some blocks new,
 * some old. Only one entry ever changes at a time, so that mixture either
 * contains the change or does not - and the callers used to just undo their
 * in-memory edit and return, which meant a create that reported failure could
 * still have landed a PHANTOM FILE on the platter, pointing at blocks nobody
 * ever wrote. It would appear at the next mount, out of nowhere.
 *
 * So: undo the change and flush AGAIN. If that succeeds the disk holds the old
 * state and nothing was lost. If it fails too, this volume is no longer under
 * our control, and the honest response is to stop writing to it rather than to
 * keep going and hope.
 *
 * (An adversarial reviewer named this as the gap its own testing had not
 * covered. It was right.)
 */
static int dir_flush(void);
static int dir_commit(int idx, const u8 *prev)
{
    if (dir_flush()) return 1;
    bcopy_n(ent(idx), prev, FS_ENT_BYTES);
    if (dir_flush()) {
        p_str("  zlfs: the change was rolled back - the volume is unharmed\n");
        return 0;
    }
    p_str("  zlfs: the directory is inconsistent on disk and cannot be repaired\n");
    p_str("  zlfs: unmounting rather than writing to it again\n");
    mounted = 0;
    return 0;
}

static int dir_load(void)
{
    for (u32 b = 0; b < sb_dir_blocks; b++) {
        if (!fsdev_read(sb_dir_lba + b, blkbuf)) {
            p_str("  zlfs: directory read failed at LBA ");
            p_u32(sb_dir_lba + b); zl_putc_pub('\n');
            return 0;
        }
        u32 off = b * dev_bsize;
        if (off >= FS_DIR_BYTES) break;
        u32 n = FS_DIR_BYTES - off;
        if (n > dev_bsize) n = dev_bsize;
        bcopy_n(dirbuf + off, blkbuf, n);
    }
    return 1;
}

/* ---- geometry check, shared by mkfs and mount ---------------------------- */
static int probe_device(void)
{
#ifndef FS_HOSTTEST
    if (!nvme_ready() && !nvme_setup()) {
        p_str("  zlfs: no disk - NVMe controller did not come ready\n");
        return 0;
    }
#endif
    dev_bsize   = fsdev_bsize();
    dev_nblocks = fsdev_blocks();

    if (dev_bsize == 0 || (dev_bsize & (dev_bsize - 1)) != 0) {
        p_str("  zlfs: block size "); p_u32(dev_bsize);
        p_str(" is not a power of two - refusing\n");
        return 0;
    }
    if (dev_bsize > FS_BLK_MAX) {
        p_str("  zlfs: block size "); p_u32(dev_bsize);
        p_str(" exceeds the "); p_u32(FS_BLK_MAX);
        p_str("-byte single-page DMA limit - refusing\n");
        return 0;
    }
    sb_dir_blocks = dir_blocks_for(dev_bsize);
    sb_dir_lba    = start_lba() + 1;
    sb_data_lba   = sb_dir_lba + sb_dir_blocks;

    if (dev_nblocks <= sb_data_lba + 1) {
        p_str("  zlfs: disk has only "); p_u32(dev_nblocks);
        p_str(" blocks - too small for a volume at LBA ");
        p_u32(start_lba()); zl_putc_pub('\n');
        return 0;
    }
    sb_data_blocks = dev_nblocks - sb_data_lba;
    return 1;
}

/* ---- format -------------------------------------------------------------- */
int fs_mkfs(void)
{
    mounted = 0;
    if (!probe_device()) return 0;

    bzero_n(dirbuf, FS_DIR_BYTES);
    bzero_n(sbbuf, dev_bsize);
    wr32b(sbbuf + SB_MAGIC,    FS_MAGIC);
    wr32b(sbbuf + SB_VERSION,  FS_VERSION);
    wr32b(sbbuf + SB_BSIZE,    dev_bsize);
    wr32b(sbbuf + SB_BLOCKS,   dev_nblocks);
    wr32b(sbbuf + SB_DIRLBA,   sb_dir_lba);
    wr32b(sbbuf + SB_DIRBLK,   sb_dir_blocks);
    wr32b(sbbuf + SB_DATALBA,  sb_data_lba);
    wr32b(sbbuf + SB_DATABLK,  sb_data_blocks);
    wr32b(sbbuf + SB_MAXFILES, FS_MAXFILES);
    wr32b(sbbuf + SB_CSUM,     0);
    wr32b(sbbuf + SB_CSUM,     (u32)(0u - sb_sum(sbbuf, dev_bsize)));

    /* INVALIDATE the old superblock before touching anything else.
     *
     * This used to write the directory first and the superblock last, on the
     * reasoning that a failure between them leaves a volume that will not
     * mount. That is true of a blank disk and false of the case that matters -
     * a REFORMAT, where the old superblock is still perfectly valid, so a
     * failed mkfs left a volume that mounted happily with half its directory
     * erased and four files out of twenty surviving. Wiping the superblock
     * first makes the failure look like the failure it is. */
    bzero_n(blkbuf, dev_bsize);
    if (!fsdev_write(start_lba(), blkbuf)) {
        p_str("  zlfs: cannot write the superblock - disk is read-only or absent\n");
        return 0;
    }
    if (!dir_flush()) return 0;
    if (!fsdev_write(start_lba(), sbbuf)) {
        p_str("  zlfs: superblock write failed\n");
        return 0;
    }
    mounted = 1;
    return 1;
}

/* ---- mount ---------------------------------------------------------------
 * Every refusal below prints the value that caused it. */
int fs_mount(void)
{
    mounted = 0;
    if (!probe_device()) return 0;

    if (!fsdev_read(start_lba(), sbbuf)) {
        p_str("  zlfs: cannot read the superblock at LBA ");
        p_u32(start_lba()); zl_putc_pub('\n');
        return 0;
    }

    u32 magic = rd32(sbbuf + SB_MAGIC);
    if (magic != FS_MAGIC) {
        p_str("  zlfs: no filesystem here - magic is "); p_hex(magic);
        p_str(", expected "); p_hex(FS_MAGIC);
        p_str("\n  zlfs: format it first\n");
        return 0;
    }
    u32 ver = rd32(sbbuf + SB_VERSION);
    if (ver != FS_VERSION) {
        p_str("  zlfs: on-disk version "); p_u32(ver);
        p_str(", this kernel speaks "); p_u32(FS_VERSION);
        p_str(" - refusing\n");
        return 0;
    }
    if (sb_sum(sbbuf, dev_bsize) != 0) {
        p_str("  zlfs: superblock checksum is bad - refusing to mount\n");
        p_str("  zlfs: the volume is damaged, not empty. Nothing was read.\n");
        return 0;
    }

    u32 bs = rd32(sbbuf + SB_BSIZE);
    if (bs != dev_bsize) {
        p_str("  zlfs: volume was made with "); p_u32(bs);
        p_str("-byte blocks, this disk has "); p_u32(dev_bsize);
        p_str(" - refusing\n");
        return 0;
    }
    if (rd32(sbbuf + SB_MAXFILES) != FS_MAXFILES) {
        p_str("  zlfs: directory holds "); p_u32(rd32(sbbuf + SB_MAXFILES));
        p_str(" entries, this kernel expects "); p_u32(FS_MAXFILES);
        p_str(" - refusing\n");
        return 0;
    }

    sb_dir_lba     = rd32(sbbuf + SB_DIRLBA);
    sb_dir_blocks  = rd32(sbbuf + SB_DIRBLK);
    sb_data_lba    = rd32(sbbuf + SB_DATALBA);
    sb_data_blocks = rd32(sbbuf + SB_DATABLK);

    /* Geometry out of the superblock is attacker-shaped input as far as this
     * code is concerned: it decides which LBAs get written. Check it against
     * the device rather than trusting it. */
    /* `sb_data_lba + sb_data_blocks > dev_nblocks` reads correctly and WRAPS:
     * a superblock claiming 4.29 billion data blocks made the sum come out
     * small and passed. Subtract instead - dev_nblocks - sb_data_lba cannot
     * underflow once sb_data_lba <= dev_nblocks has been established. */
    if (sb_dir_blocks != dir_blocks_for(dev_bsize) ||
        sb_dir_lba    != start_lba() + 1 ||
        sb_data_lba   != sb_dir_lba + sb_dir_blocks ||
        sb_data_lba   >  dev_nblocks ||
        sb_data_blocks > dev_nblocks - sb_data_lba) {
        p_str("  zlfs: superblock geometry does not fit this disk - refusing\n");
        return 0;
    }

    if (!dir_load()) return 0;

    /* A directory entry also decides which LBAs get written. One with a run
     * outside the data area, or a length longer than its run, is corruption
     * that a later write would turn into someone else's file. */
    for (int i = 0; i < FS_MAXFILES; i++) {
        if (!ent_used(i)) continue;
        u32 st = ent_start(i), nb = ent_blocks(i), ln = ent_len(i);
        /* `st + nb > vol_end` wraps too, and this one is worse than the
         * superblock's: an entry with start 3000 and 4294964396 blocks summed
         * to 100 and passed, after which alloc_run inherited the wrapped end
         * as its cursor and handed out blocks BELOW the data area - so an
         * ordinary create-and-write, with no crafted arguments at all,
         * overwrote the superblock and destroyed the volume. Subtract. */
        u32 vol_end = sb_data_lba + sb_data_blocks;   /* checked above, no wrap */
        if (nb == 0 || st < sb_data_lba || st > vol_end || nb > vol_end - st ||
            ln > nb * dev_bsize) {
            p_str("  zlfs: directory entry "); p_u32((u32)i);
            p_str(" is out of range - refusing to mount\n");
            return 0;
        }
    }

    mounted = 1;
    return 1;
}

int fs_mounted(void) { return mounted; }

/* ---- allocation ----------------------------------------------------------
 * First fit over the gaps between existing runs, which is O(n^2) with n = 32
 * and reclaims the space a deleted file leaves behind. A bump allocator would
 * be shorter and would leak every deletion until reformat; that is the kind of
 * "simpler" that is only simpler for the person writing it.
 *
 * It takes NO `skip` argument. It used to, so that a growing file could ignore
 * its own old run and be allowed to overlap it - and that is precisely what
 * made a failed relocation unrecoverable, because the copy had already eaten
 * the bytes it was copying FROM. Copying a file needs both runs to exist at
 * once; the cost is that a file cannot grow past half the free space, which is
 * a stated limit rather than a window in which a power cut loses the file.
 */
static int alloc_run(u32 need, u32 *out)
{
    u32 cursor = sb_data_lba;
    u32 end    = sb_data_lba + sb_data_blocks;

    for (;;) {
        u32 next_start = end;
        u32 next_end   = end;
        int found = 0;
        for (int i = 0; i < FS_MAXFILES; i++) {
            if (!ent_used(i)) continue;
            u32 st = ent_start(i);
            if (st < cursor) continue;
            if (!found || st < next_start) {
                next_start = st;
                next_end   = st + ent_blocks(i);
                found      = 1;
            }
        }
        if (next_start - cursor >= need) { *out = cursor; return 1; }
        if (!found) return 0;
        cursor = next_end;
        if (cursor >= end) return 0;
    }
}

/* ---- the file operations ------------------------------------------------- */
int fs_find(const char *name)
{
    if (!mounted) return -1;
    for (int i = 0; i < FS_MAXFILES; i++)
        if (ent_used(i) && nameeq((const char *)(ent(i) + FE_NAME), name)) return i;
    return -1;
}

int fs_create(const char *name, u32 bytes)
{
    if (!mounted) { p_str("  zlfs: not mounted\n"); return -1; }
    if (name[0] == 0) { p_str("  zlfs: a file needs a name\n"); return -1; }

    /* A name that does not fit used to be TRUNCATED into the entry while
     * fs_find went on comparing the caller's full string - so the file could
     * never be found again, the duplicate check therefore never fired, and
     * creating it twice produced two entries with byte-identical names and one
     * of them unreachable. Refusing is the only answer that does not lose a
     * file quietly. */
    {
        int n = 0;
        while (n < FS_NAME_MAX && name[n]) n++;
        if (n >= FS_NAME_MAX) {
            p_str("  zlfs: that name is longer than ");
            p_u32(FS_NAME_MAX - 1);
            p_str(" characters - refusing rather than truncating it\n");
            return -1;
        }
    }

    if (fs_find(name) >= 0) {
        p_str("  zlfs: '"); p_name(name); p_str("' already exists\n");
        return -1;
    }

    int slot = -1;
    for (int i = 0; i < FS_MAXFILES; i++) if (!ent_used(i)) { slot = i; break; }
    if (slot < 0) {
        p_str("  zlfs: no free directory slot - the volume holds ");
        p_u32(FS_MAXFILES); p_str(" files and all are in use\n");
        return -1;
    }

    u32 need = blocks_for(bytes), start;
    if (!alloc_run(need, &start)) {
        p_str("  zlfs: no contiguous run of "); p_u32(need);
        p_str(" block(s) free - refusing\n");
        return -1;
    }

    u8 *e = ent(slot);
    u8 prev[FS_ENT_BYTES];
    bcopy_n(prev, e, FS_ENT_BYTES);          /* the free slot, to go back to */

    bzero_n(e, FS_ENT_BYTES);
    for (int i = 0; i < FS_NAME_MAX - 1 && name[i]; i++) e[FE_NAME + i] = (u8)name[i];
    wr32b(e + FE_START,  start);
    wr32b(e + FE_LEN,    0);
    wr32b(e + FE_BLOCKS, need);
    wr32b(e + FE_FLAGS,  FE_USED);
    wr32b(e + FE_MTIME,  now_secs);
    if (!dir_commit(slot, prev)) return -1;
    return slot;
}

int fs_write(int idx, const void *src, u32 bytes)
{
    if (!mounted) { p_str("  zlfs: not mounted\n"); return 0; }
    if (idx < 0 || idx >= FS_MAXFILES || !ent_used(idx)) {
        p_str("  zlfs: no such file\n"); return 0;
    }

    u32 need       = blocks_for(bytes);
    u32 old_start  = ent_start(idx);
    u32 old_blocks = ent_blocks(idx);
    u32 old_len    = ent_len(idx);
    u32 old_mtime  = rd32(ent(idx) + FE_MTIME);
    u32 base       = old_start;
    int moving     = 0;

    /* Outgrown its run: find another one. NOTHING is written into the
     * directory entry here.
     *
     * The previous version published FE_START and FE_BLOCKS at this point and
     * only then began copying, on the reasoning that the length is written
     * last so a torn write leaves a stale length. That reasoning was wrong,
     * and the comment that stated it as an invariant was worse than the bug:
     * a write that failed mid-relocation left the entry pointing at the NEW
     * run - which holds whatever a deleted file left there - while the file's
     * real bytes sat orphaned at the old LBA with nothing referencing them.
     * The next dir_flush() from any unrelated operation made that permanent.
     * The file read back as some other file's deleted contents.
     *
     * So: allocate, copy, and only publish start/blocks/length TOGETHER once
     * every block has landed. Until that moment the entry is untouched and the
     * old run is intact, which is what "a failure leaves the file exactly as
     * it was" actually requires. */
    if (need > ent_blocks(idx)) {
        if (!alloc_run(need, &base)) {
            p_str("  zlfs: '"); p_name((const char *)(ent(idx) + FE_NAME));
            p_str("' needs "); p_u32(need);
            p_str(" blocks and there is no run that long free - refusing\n");
            return 0;
        }
        moving = 1;
    }

    const u8 *s = (const u8 *)src;
    for (u32 b = 0; b < need; b++) {
        bzero_n(blkbuf, dev_bsize);
        u32 off = b * dev_bsize;
        u32 n   = (bytes > off) ? (bytes - off) : 0;
        if (n > dev_bsize) n = dev_bsize;
        if (n) bcopy_n(blkbuf, s + off, n);
        if (!fsdev_write(base + b, blkbuf)) {
            p_str("  zlfs: write failed at LBA "); p_u32(base + b);
            if (moving) {
                /* alloc_run never overlaps a live run, including this file's
                 * own, so the old copy is untouched and still published. */
                p_str(" - the file is unchanged, still at LBA ");
                p_u32(old_start); zl_putc_pub('\n');
            } else {
                p_str(" - the file is now partial\n");
            }
            return 0;
        }
    }

    /* Publish. Start, blocks and length go in together, then one flush.
     *
     * The run never SHRINKS in place. Writing 2000 bytes into a file that owns
     * eight blocks leaves it owning eight: giving them back would mean the
     * next write past 2000 bytes relocates, and a file that is written short
     * and then long again is the common case, not the rare one. The space is
     * reclaimed by deleting the file, which is the only place this design
     * reclaims anything. */
    u32 pub_blocks = (need > old_blocks) ? need : old_blocks;
    u8 *e = ent(idx);
    u8 prev[FS_ENT_BYTES];
    bcopy_n(prev, e, FS_ENT_BYTES);
    (void)old_len; (void)old_mtime;          /* prev carries them verbatim */

    wr32b(e + FE_START,  base);
    wr32b(e + FE_BLOCKS, pub_blocks);
    wr32b(e + FE_LEN,    bytes);
    wr32b(e + FE_MTIME,  now_secs);
    return dir_commit(idx, prev);
}

int fs_read(int idx, void *dst, u32 max)
{
    if (!mounted) { p_str("  zlfs: not mounted\n"); return 0; }
    if (idx < 0 || idx >= FS_MAXFILES || !ent_used(idx)) {
        p_str("  zlfs: no such file\n"); return 0;
    }
    u32 len = ent_len(idx);
    if (len > max) len = max;

    /* Never read past the file's OWN run, whatever the length field says. The
     * length is checked at mount, but "checked once at mount" and "cannot walk
     * into the next file" are different guarantees, and a stale or crafted
     * length used to give the caller its neighbour's bytes. Compared in blocks
     * so nb * dev_bsize cannot overflow on the way. */
    u32 nb = ent_blocks(idx);
    if (len / dev_bsize >= nb) {
        u32 cap = nb * dev_bsize;               /* nb is bounded by the volume */
        if (len > cap) len = cap;
    }

    u32 base = ent_start(idx);
    u8 *d = (u8 *)dst;
    u32 done = 0;
    while (done < len) {
        if (!fsdev_read(base + done / dev_bsize, blkbuf)) {
            p_str("  zlfs: read failed at LBA ");
            p_u32(base + done / dev_bsize); zl_putc_pub('\n');
            return (int)done;
        }
        u32 n = len - done;
        if (n > dev_bsize) n = dev_bsize;
        bcopy_n(d + done, blkbuf, n);
        done += n;
    }
    return (int)done;
}

int fs_delete(int idx)
{
    if (!mounted) { p_str("  zlfs: not mounted\n"); return 0; }
    if (idx < 0 || idx >= FS_MAXFILES || !ent_used(idx)) {
        p_str("  zlfs: no such file\n"); return 0;
    }
    u8 prev[FS_ENT_BYTES];
    bcopy_n(prev, ent(idx), FS_ENT_BYTES);
    bzero_n(ent(idx), FS_ENT_BYTES);
    return dir_commit(idx, prev);
}

/* ---- what the shell needs to draw a listing ------------------------------ */
int fs_used(int idx)  { return (idx >= 0 && idx < FS_MAXFILES && mounted) ? ent_used(idx) : 0; }
u32 fs_size(int idx)  { return fs_used(idx) ? ent_len(idx) : 0; }
u32 fs_start(int idx) { return fs_used(idx) ? ent_start(idx) : 0; }
u32 fs_runlen(int idx){ return fs_used(idx) ? ent_blocks(idx) : 0; }
u32 fs_mtime(int idx) { return fs_used(idx) ? rd32(ent(idx) + FE_MTIME) : 0; }
int fs_maxfiles(void) { return FS_MAXFILES; }
u32 fs_bsize(void)    { return dev_bsize; }
u32 fs_capacity(void) { return sb_data_blocks; }

int fs_count(void)
{
    int n = 0;
    if (mounted) for (int i = 0; i < FS_MAXFILES; i++) if (ent_used(i)) n++;
    return n;
}

u32 fs_free_blocks(void)
{
    if (!mounted) return 0;
    u32 used = 0;
    for (int i = 0; i < FS_MAXFILES; i++) if (ent_used(i)) used += ent_blocks(i);
    return sb_data_blocks - used;
}

/* one character of a name, because zl has no strings to hand back */
int fs_name_byte(int idx, int i)
{
    if (!fs_used(idx) || i < 0 || i >= FS_NAME_MAX) return 0;
    return (int)ent(idx)[FE_NAME + i];
}

void fs_set_time(u32 secs) { now_secs = secs; }

/* ---- the staged name ----------------------------------------------------
 * zl pushes a filename one character at a time and then calls the _named
 * variant. The comparison happens here, in C, for the same reason term.c's
 * command table does: the zl kernel subset has string literals and no string
 * values, so two runtime strings can never meet in zl. */
void fs_name_clear(void) { stage_len = 0; bzero_n(stage, FS_NAME_MAX); }

int fs_name_push(int ch)
{
    if (ch < 32 || ch > 126) return 0;              /* printable ASCII only  */
    if (stage_len >= FS_NAME_MAX - 1) return 0;
    stage[stage_len++] = (char)ch;
    stage[stage_len]   = 0;
    return 1;
}

int fs_name_pop(void)
{
    if (stage_len <= 0) return 0;
    stage[--stage_len] = 0;
    return 1;
}

int fs_name_stage_len(void) { return stage_len; }

int fs_name_stage_byte(int i)
{
    if (i < 0 || i >= stage_len) return 0;
    return (unsigned char)stage[i];
}

int fs_name_len(void)            { return stage_len; }
int fs_create_named(u32 bytes)   { return fs_create(stage, bytes); }
int fs_find_named(void)          { return fs_find(stage); }
