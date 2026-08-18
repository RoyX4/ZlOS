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

static u32 blocks_for(u32 bytes)
{
    if (bytes == 0) return 1;                   /* every file owns >= 1 block */
    return (bytes + dev_bsize - 1) / dev_bsize;
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

    /* directory first, superblock last. If the power goes out between the
     * two, the volume does not mount and nothing claims to be a file - which
     * is the failure this order is chosen to produce. */
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
    if (sb_dir_blocks != dir_blocks_for(dev_bsize) ||
        sb_dir_lba    != start_lba() + 1 ||
        sb_data_lba   != sb_dir_lba + sb_dir_blocks ||
        sb_data_lba + sb_data_blocks > dev_nblocks) {
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
        if (nb == 0 || st < sb_data_lba || st + nb > sb_data_lba + sb_data_blocks ||
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
 * `skip` lets a grow-in-place caller ignore its own old run while looking for
 * the new one.
 */
static int alloc_run(u32 need, int skip, u32 *out)
{
    u32 cursor = sb_data_lba;
    u32 end    = sb_data_lba + sb_data_blocks;

    for (;;) {
        u32 next_start = end;
        u32 next_end   = end;
        int found = 0;
        for (int i = 0; i < FS_MAXFILES; i++) {
            if (i == skip || !ent_used(i)) continue;
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
    if (!alloc_run(need, -1, &start)) {
        p_str("  zlfs: no contiguous run of "); p_u32(need);
        p_str(" block(s) free - refusing\n");
        return -1;
    }

    u8 *e = ent(slot);
    bzero_n(e, FS_ENT_BYTES);
    for (int i = 0; i < FS_NAME_MAX - 1 && name[i]; i++) e[FE_NAME + i] = (u8)name[i];
    wr32b(e + FE_START,  start);
    wr32b(e + FE_LEN,    0);
    wr32b(e + FE_BLOCKS, need);
    wr32b(e + FE_FLAGS,  FE_USED);
    wr32b(e + FE_MTIME,  now_secs);
    if (!dir_flush()) { bzero_n(e, FS_ENT_BYTES); return -1; }
    return slot;
}

int fs_write(int idx, const void *src, u32 bytes)
{
    if (!mounted) { p_str("  zlfs: not mounted\n"); return 0; }
    if (idx < 0 || idx >= FS_MAXFILES || !ent_used(idx)) {
        p_str("  zlfs: no such file\n"); return 0;
    }

    u32 need = blocks_for(bytes);

    /* Outgrown its run: find a new one, copy, then publish. The old run is
     * only released once the new one holds the data, so a failure anywhere
     * leaves the file exactly as it was. */
    if (need > ent_blocks(idx)) {
        u32 start;
        if (!alloc_run(need, idx, &start)) {
            p_str("  zlfs: '"); p_name((const char *)(ent(idx) + FE_NAME));
            p_str("' needs "); p_u32(need);
            p_str(" blocks and there is no run that long - refusing\n");
            return 0;
        }
        wr32b(ent(idx) + FE_START,  start);
        wr32b(ent(idx) + FE_BLOCKS, need);
    }

    u32 base = ent_start(idx);
    const u8 *s = (const u8 *)src;
    for (u32 b = 0; b < need; b++) {
        bzero_n(blkbuf, dev_bsize);
        u32 off = b * dev_bsize;
        u32 n   = (bytes > off) ? (bytes - off) : 0;
        if (n > dev_bsize) n = dev_bsize;
        if (n) bcopy_n(blkbuf, s + off, n);
        if (!fsdev_write(base + b, blkbuf)) {
            p_str("  zlfs: write failed at LBA "); p_u32(base + b);
            p_str(" - the file is now partial\n");
            return 0;
        }
    }

    /* Length last. Until this lands the file still reports its old size, so a
     * torn write is a stale file rather than a file claiming bytes that were
     * never written. */
    wr32b(ent(idx) + FE_LEN,   bytes);
    wr32b(ent(idx) + FE_MTIME, now_secs);
    return dir_flush();
}

int fs_read(int idx, void *dst, u32 max)
{
    if (!mounted) { p_str("  zlfs: not mounted\n"); return 0; }
    if (idx < 0 || idx >= FS_MAXFILES || !ent_used(idx)) {
        p_str("  zlfs: no such file\n"); return 0;
    }
    u32 len = ent_len(idx);
    if (len > max) len = max;
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
    bzero_n(ent(idx), FS_ENT_BYTES);
    return dir_flush();
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

int fs_name_len(void)            { return stage_len; }
int fs_create_named(u32 bytes)   { return fs_create(stage, bytes); }
int fs_find_named(void)          { return fs_find(stage); }
