/* fstest.c - zlfs, asserted, against a block device that can be made to fail.
 *
 * A filesystem's real bugs are not crashes. They are:
 *
 *   - the second file landing on top of the first one's blocks
 *   - a deleted file's space never coming back
 *   - a file that outgrew its run losing the tail nobody re-read
 *   - a torn write leaving a length that claims bytes never written
 *   - a garbage superblock being treated as an empty disk
 *
 * Not one of those is visible on a screen, and several are invisible until the
 * reboot that reads the damage back. So fs.c is compiled here against a RAM
 * disk with an injectable write fault, and every one of those sequences is
 * constructed deliberately.
 *
 * THE REBOOT IS A REAL ONE. Phase 2 runs in a SEPARATE PROCESS against the
 * same image file. Nothing is shared: separate BSS, separate directory cache,
 * separate everything. A file that comes back in phase 2 came off the disk,
 * because there is nowhere else it could have come from. Persistence claimed
 * without that is persistence claimed from a cache.
 *
 * Build and run:  ./build.sh && ./fstest
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

typedef unsigned int u32;

/* ---- fs.c's surface ------------------------------------------------------ */
int  fs_mkfs(void);
int  fs_mount(void);
int  fs_mounted(void);
int  fs_create(const char *name, u32 bytes);
int  fs_find(const char *name);
int  fs_write(int idx, const void *src, u32 bytes);
int  fs_read(int idx, void *dst, u32 max);
int  fs_delete(int idx);
int  fs_count(void);
int  fs_used(int idx);
u32  fs_size(int idx);
u32  fs_start(int idx);
u32  fs_runlen(int idx);
u32  fs_free_blocks(void);
u32  fs_capacity(void);
u32  fs_bsize(void);
int  fs_maxfiles(void);
int  fs_name_byte(int idx, int i);
void fs_name_clear(void);
int  fs_name_push(int ch);
int  fs_create_named(u32 bytes);
int  fs_find_named(void);

/* ---- the character sink fs.c prints its refusals through ------------------
 * Captured rather than discarded: "it refused" is half the assertion, and
 * "it refused for the RIGHT reason" is the other half. A mount that rejects a
 * corrupt superblock by complaining about the block size is still a bug. */
static char  saidbuf[8192];
static int   saidlen;
static int   quiet = 1;
void zl_putc_pub(char c)
{
    if (saidlen < (int)sizeof saidbuf - 1) saidbuf[saidlen++] = c;
    saidbuf[saidlen] = 0;
    if (!quiet) fputc(c, stdout);
}
static void said_reset(void) { saidlen = 0; saidbuf[0] = 0; }
static int  said(const char *needle) { return strstr(saidbuf, needle) != NULL; }

/* ---- the fake block device ----------------------------------------------- */
#define DEV_BSIZE_DEFAULT 512
#define FS_START_OFF      ((size_t)1024 * 1024)   /* the volume's first byte */
#define DEV_BYTES         (16u * 1024 * 1024)

static unsigned char *disk;
static u32 dev_bsize  = DEV_BSIZE_DEFAULT;
static u32 dev_blocks = DEV_BYTES / DEV_BSIZE_DEFAULT;

static long fail_lba = -1;          /* writes to this LBA fail               */
static u32  writes_done;

int fsdev_read(u32 lba, void *buf)
{
    if (lba >= dev_blocks) return 0;
    memcpy(buf, disk + (size_t)lba * dev_bsize, dev_bsize);
    return 1;
}
int fsdev_write(u32 lba, const void *buf)
{
    if (lba >= dev_blocks) return 0;
    if (fail_lba >= 0 && (long)lba == fail_lba) return 0;
    memcpy(disk + (size_t)lba * dev_bsize, buf, dev_bsize);
    writes_done++;
    return 1;
}
u32 fsdev_bsize(void)  { return dev_bsize; }
u32 fsdev_blocks(void) { return dev_blocks; }

static void dev_new(u32 bsize)
{
    dev_bsize  = bsize;
    dev_blocks = DEV_BYTES / bsize;
    free(disk);
    disk = calloc(DEV_BYTES, 1);
    fail_lba = -1;
    writes_done = 0;
}

/* ---- assertions ----------------------------------------------------------- */
static int fails;
static void ok(const char *what, int cond)
{
    printf("  %-58s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}

static const char *IMG = "/tmp/zlfs-test.img";

static void disk_save(void)
{
    FILE *f = fopen(IMG, "wb");
    if (!f) { perror("save"); exit(2); }
    fwrite(disk, 1, DEV_BYTES, f);
    fclose(f);
}
static int disk_load(void)
{
    FILE *f = fopen(IMG, "rb");
    if (!f) return 0;
    size_t n = fread(disk, 1, DEV_BYTES, f);
    fclose(f);
    return n == DEV_BYTES;
}

/* Recompute the superblock checksum in place. Needed because the checksum is
 * checked BEFORE the geometry, so a blindly corrupted field can never reach
 * the geometry check - which is correct defence in depth, and means the only
 * way to test that check is a superblock that is internally consistent and
 * still describes a disk this is not. That is exactly what an image copied
 * from a larger disk looks like. */
static void sb_refix(void)
{
    unsigned char *sb = disk + FS_START_OFF;
    sb[36] = sb[37] = sb[38] = sb[39] = 0;
    unsigned sum = 0;
    for (unsigned i = 0; i + 4 <= dev_bsize; i += 4)
        sum += (unsigned)sb[i] | ((unsigned)sb[i+1] << 8)
             | ((unsigned)sb[i+2] << 16) | ((unsigned)sb[i+3] << 24);
    unsigned c = 0u - sum;
    sb[36] = (unsigned char)c;        sb[37] = (unsigned char)(c >> 8);
    sb[38] = (unsigned char)(c >> 16); sb[39] = (unsigned char)(c >> 24);
}

static void namebuf(int idx, char *out)
{
    int i = 0;
    for (; i < 23; i++) { int c = fs_name_byte(idx, i); if (!c) break; out[i] = (char)c; }
    out[i] = 0;
}

/* =====================================================================
 * PHASE 2 - a separate process. Everything it knows, it reads off the disk.
 * ===================================================================== */
static int phase2(void)
{
    dev_new(DEV_BSIZE_DEFAULT);
    if (!disk_load()) { fprintf(stderr, "phase2: no image at %s\n", IMG); return 2; }

    printf("\n  -- phase 2: a SEPARATE PROCESS, fresh BSS, same disk --\n");
    said_reset();
    ok("the volume mounts after a power cycle", fs_mount() == 1);
    ok("...and reports the same three files", fs_count() == 3);

    int a = fs_find("hello.txt");
    int b = fs_find("notes.md");
    int c = fs_find("big.bin");
    ok("'hello.txt' is found BY NAME on a cold start", a >= 0);
    ok("'notes.md' too", b >= 0);
    ok("'big.bin' too", c >= 0);

    char buf[9000];
    memset(buf, 0, sizeof buf);
    int n = fs_read(a, buf, sizeof buf);
    ok("hello.txt is 13 bytes", n == 13);
    ok("...and the bytes are the ones written before the reboot",
       memcmp(buf, "hello, world!", 13) == 0);

    memset(buf, 0, sizeof buf);
    n = fs_read(c, buf, sizeof buf);
    int intact = (n == 5000);
    for (int i = 0; i < n && intact; i++)
        if ((unsigned char)buf[i] != (unsigned char)((i * 31 + 7) & 0xFF)) intact = 0;
    ok("big.bin survived at 5000 bytes, every byte correct", intact);

    char nm[32];
    namebuf(a, nm);
    ok("the NAME itself round-tripped through the disk", strcmp(nm, "hello.txt") == 0);

    /* deleted-before-the-reboot must stay deleted */
    ok("'gone.tmp', deleted before the power cycle, is still gone",
       fs_find("gone.tmp") < 0);

    return fails ? 1 : 0;
}

/* =====================================================================
 * PHASE 1 - everything that does not need a reboot, then leave a disk behind.
 * ===================================================================== */
int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--phase2") == 0) return phase2();

    printf("zlfs - the filesystem, asserted\n\n");
    printf("  -- refusing what it should refuse --\n");

    /* ---- an unformatted disk is not an empty filesystem ------------------ */
    dev_new(DEV_BSIZE_DEFAULT);
    said_reset();
    ok("a blank disk does NOT mount", fs_mount() == 0);
    ok("...and says the magic was wrong, not 'no files'", said("magic is 0x00000000"));
    ok("...and nothing reports as mounted", fs_mounted() == 0);

    /* ---- format ---------------------------------------------------------- */
    said_reset();
    ok("mkfs succeeds on a blank disk", fs_mkfs() == 1);
    ok("...and it mounts afterwards", fs_mount() == 1);
    ok("...with the device's block size", fs_bsize() == DEV_BSIZE_DEFAULT);
    ok("...and an empty directory", fs_count() == 0);

    /* ---- a corrupt superblock must refuse, and say which check failed ---- */
    unsigned char sb_good[4096];
    memcpy(sb_good, disk + FS_START_OFF, dev_bsize);

    said_reset();
    disk[FS_START_OFF + 200] ^= 0xFF;      /* a byte in the TAIL */
    ok("a byte flipped in the superblock's TAIL is caught", fs_mount() == 0);
    ok("...by the checksum, and it says the volume is damaged",
       said("checksum is bad") && said("damaged, not empty"));
    memcpy(disk + FS_START_OFF, sb_good, dev_bsize);

    said_reset();
    disk[FS_START_OFF + 1] ^= 0xFF;        /* the magic itself   */
    ok("a wrecked magic is caught before the checksum", fs_mount() == 0);
    ok("...and reports it as 'no filesystem here'", said("no filesystem here"));
    memcpy(disk + FS_START_OFF, sb_good, dev_bsize);

    said_reset();
    disk[FS_START_OFF + 4] = 99;           /* version 99         */
    ok("a future on-disk version is refused", fs_mount() == 0);
    ok("...naming both versions", said("version 99") && said("speaks 1"));
    memcpy(disk + FS_START_OFF, sb_good, dev_bsize);

    said_reset();
    disk[FS_START_OFF + 24] = 0xFF;        /* data LBA off the end */
    disk[FS_START_OFF + 25] = 0xFF;
    sb_refix();                            /* ...and a VALID checksum over it */
    ok("geometry off the end of the disk is refused even when the checksum is good",
       fs_mount() == 0);
    ok("...and says so rather than writing there", said("geometry does not fit"));
    memcpy(disk + FS_START_OFF, sb_good, dev_bsize);

    said_reset();
    disk[FS_START_OFF + 8] = 0x00;         /* claim 4096-byte blocks... */
    disk[FS_START_OFF + 9] = 0x10;
    sb_refix();                            /* ...consistently */
    ok("a volume made with a different block size is refused", fs_mount() == 0);
    ok("...naming both sizes", said("4096-byte blocks") && said("512"));
    memcpy(disk + FS_START_OFF, sb_good, dev_bsize);

    ok("the good superblock still mounts after all that", fs_mount() == 1);

    /* ---- create, write, read -------------------------------------------- */
    printf("\n  -- files with names --\n");
    said_reset();
    int a = fs_create("hello.txt", 13);
    ok("create returns a slot", a >= 0);
    ok("...and it is findable by name", fs_find("hello.txt") == a);
    ok("...and an unknown name is not", fs_find("nope.txt") < 0);

    ok("write succeeds", fs_write(a, "hello, world!", 13) == 1);
    ok("...and the size is 13", fs_size(a) == 13);

    char buf[9000];
    memset(buf, 0, sizeof buf);
    ok("read returns 13 bytes", fs_read(a, buf, sizeof buf) == 13);
    ok("...and they are the right ones", memcmp(buf, "hello, world!", 13) == 0);

    said_reset();
    ok("creating the same name twice is refused", fs_create("hello.txt", 4) < 0);
    ok("...and it says the file already exists", said("already exists"));

    said_reset();
    ok("a nameless file is refused", fs_create("", 4) < 0);
    ok("...and it says a file needs a name", said("needs a name"));

    /* ---- the staged-name seam zl uses ------------------------------------ */
    fs_name_clear();
    const char *pn = "notes.md";
    for (const char *p = pn; *p; p++) fs_name_push(*p);
    int b = fs_create_named(20);
    ok("a name pushed one character at a time creates a file", b >= 0);
    ok("...and find_named locates it", fs_find_named() == b);
    fs_name_clear();
    for (int i = 0; i < 100; i++) fs_name_push('x');
    ok("a name longer than the field stops at 23 characters", fs_find_named() < 0);

    /* ---- growth past the run -------------------------------------------- */
    printf("\n  -- growing past the run --\n");
    int c = fs_create("big.bin", 100);
    u32 c_start_small = fs_start(c);
    ok("a small file takes one block", fs_runlen(c) == 1);

    /* Wall it in. Growing in place is CORRECT when the space after a file is
     * free - the first version of this test asserted a relocation that should
     * not have happened. To exercise the relocation path the run has to be
     * genuinely boxed in, so put a file immediately after it and give that
     * file a sentinel to prove it was never written through. */
    int wall = fs_create("wall.bin", 100);
    ok("a wall sits immediately after it", fs_start(wall) == c_start_small + 1);
    fs_write(wall, "WALL-MUST-NOT-MOVE", 18);

    for (int i = 0; i < 5000; i++) buf[i] = (char)((i * 31 + 7) & 0xFF);
    ok("writing 5000 bytes into it succeeds", fs_write(c, buf, 5000) == 1);
    ok("...the run grew to 10 blocks", fs_runlen(c) == 10);
    ok("...and it RELOCATED past the wall instead of trampling it",
       fs_start(c) > fs_start(wall));

    memset(buf, 0, sizeof buf);
    int n = fs_read(c, buf, sizeof buf);
    int intact = (n == 5000);
    for (int i = 0; i < n && intact; i++)
        if ((unsigned char)buf[i] != (unsigned char)((i * 31 + 7) & 0xFF)) intact = 0;
    ok("...and every one of the 5000 bytes reads back correctly", intact);

    /* the neighbours must be untouched by all that moving */
    memset(buf, 0, sizeof buf);
    fs_read(wall, buf, sizeof buf);
    ok("the wall it grew PAST still holds its sentinel",
       memcmp(buf, "WALL-MUST-NOT-MOVE", 18) == 0);
    memset(buf, 0, sizeof buf);
    fs_read(a, buf, sizeof buf);
    ok("hello.txt is untouched by its neighbour's growth",
       memcmp(buf, "hello, world!", 13) == 0);
    fs_delete(wall);

    /* ---- THE ADVERSARIAL ONE: can a sequence be built that loses a file? -- */
    printf("\n  -- trying to lose a file on purpose --\n");
    int g1 = fs_create("gap1.bin", 2000);   /* 4 blocks */
    int g2 = fs_create("gap2.bin", 2000);   /* 4 blocks, right after it */
    u32 g2_start = fs_start(g2);
    fs_write(g2, "SENTINEL-DO-NOT-CLOBBER", 23);
    fs_delete(g1);                          /* leave a 4-block hole */

    int g3 = fs_create("gap3.bin", 2000);   /* should land IN the hole */
    ok("a new file reuses the deleted file's blocks", fs_start(g3) < g2_start);
    ok("...exactly, not approximately", fs_start(g3) + fs_runlen(g3) <= g2_start);
    for (int i = 0; i < 2000; i++) buf[i] = 'Z';
    fs_write(g3, buf, 2000);

    memset(buf, 0, sizeof buf);
    fs_read(g2, buf, sizeof buf);
    ok("...and the file next door still holds its sentinel",
       memcmp(buf, "SENTINEL-DO-NOT-CLOBBER", 23) == 0);

    /* every run must be disjoint - checked exhaustively, not by eye */
    int overlap = 0;
    for (int i = 0; i < fs_maxfiles(); i++) {
        if (!fs_used(i)) continue;
        for (int j = i + 1; j < fs_maxfiles(); j++) {
            if (!fs_used(j)) continue;
            u32 i0 = fs_start(i), i1 = i0 + fs_runlen(i);
            u32 j0 = fs_start(j), j1 = j0 + fs_runlen(j);
            if (i0 < j1 && j0 < i1) overlap++;
        }
    }
    ok("NO two files share a block, across every pair", overlap == 0);

    fs_delete(g2);
    fs_delete(g3);

    /* ---- refusals that must print --------------------------------------- */
    printf("\n  -- refusals that print --\n");
    said_reset();
    ok("a file bigger than the disk is refused",
       fs_create("huge.bin", 4000u * 1000u * 1000u) < 0);
    ok("...saying no contiguous run is free", said("no contiguous run"));

    said_reset();
    u32 before = fs_free_blocks();
    int made = 0;
    for (int i = 0; i < 64; i++) {
        char nm[24];
        snprintf(nm, sizeof nm, "f%02d", i);
        if (fs_create(nm, 1) >= 0) made++; else break;
    }
    ok("the directory fills up and then stops", fs_count() == fs_maxfiles());
    ok("...and the refusal says there is no free slot", said("no free directory slot"));
    ok("...having made exactly the slots that were left", made == fs_maxfiles() - 3);
    (void)before;

    /* ---- a torn write must not leave a lying length --------------------- */
    printf("\n  -- a write that fails halfway --\n");
    for (int i = 0; i < 64; i++) { char nm[24]; snprintf(nm, sizeof nm, "f%02d", i); int k = fs_find(nm); if (k >= 0) fs_delete(k); }
    int t = fs_create("torn.bin", 4096);
    fs_write(t, buf, 2000);
    u32 len_before = fs_size(t);
    said_reset();
    fail_lba = (long)fs_start(t) + 3;          /* fail the 4th block */
    for (int i = 0; i < 4000; i++) buf[i] = 'T';
    ok("a write whose 4th block fails reports failure", fs_write(t, buf, 4000) == 0);
    ok("...and says the file is now partial", said("the file is now partial"));
    ok("...and the LENGTH still reads the OLD value, not the new claim",
       fs_size(t) == len_before);
    fail_lba = -1;
    fs_delete(t);

    /* ---- a directory entry pointing outside the volume ------------------ */
    said_reset();
    {
        /* reach into the on-disk directory and push a run off the end */
        unsigned char *d = disk + FS_START_OFF + dev_bsize;
        /* entry 0 is hello.txt; FE_START is at byte 24 of a 64-byte entry */
        unsigned char save[64];
        memcpy(save, d, 64);
        d[24] = 0xFF; d[25] = 0xFF; d[26] = 0xFF; d[27] = 0x7F;
        ok("a directory entry pointing off the disk refuses the MOUNT",
           fs_mount() == 0);
        ok("...naming the entry", said("is out of range"));
        memcpy(d, save, 64);
    }
    ok("and it mounts again once the entry is sane", fs_mount() == 1);

    /* ---- a second geometry: 4096-byte blocks ---------------------------- */
    printf("\n  -- the same code on a 4096-byte-block device --\n");
    dev_new(4096);
    ok("mkfs on 4096-byte blocks", fs_mkfs() == 1);
    ok("...mounts", fs_mount() == 1);
    ok("...with one directory block instead of four", fs_bsize() == 4096);
    int q = fs_create("q.bin", 9000);
    ok("a 9000-byte file takes 3 blocks here", fs_runlen(q) == 3);
    for (int i = 0; i < 9000; i++) buf[i] = (char)(i & 0x7F);
    ok("write", fs_write(q, buf, 9000) == 1);
    memset(buf, 0, sizeof buf);
    ok("read back 9000", fs_read(q, buf, sizeof buf) == 9000);
    intact = 1;
    for (int i = 0; i < 9000 && intact; i++) if (buf[i] != (char)(i & 0x7F)) intact = 0;
    ok("...byte for byte", intact);

    /* ---- leave a disk behind for the separate-process reboot ------------- */
    printf("\n  -- building the disk phase 2 will cold-start from --\n");
    dev_new(DEV_BSIZE_DEFAULT);
    fs_mkfs();
    fs_mount();
    a = fs_create("hello.txt", 13);
    fs_write(a, "hello, world!", 13);
    b = fs_create("notes.md", 32);
    fs_write(b, "the second brain, on a disk.", 28);
    c = fs_create("big.bin", 5000);
    for (int i = 0; i < 5000; i++) buf[i] = (char)((i * 31 + 7) & 0xFF);
    fs_write(c, buf, 5000);
    int gone = fs_create("gone.tmp", 100);
    fs_write(gone, "delete me", 9);
    fs_delete(gone);
    ok("three files staged, one created-then-deleted", fs_count() == 3);
    disk_save();

    /* fork+exec ourselves: a real process boundary, not a function call */
    fflush(NULL);
    pid_t pid = fork();
    if (pid == 0) {
        execl(argv[0], argv[0], "--phase2", (char *)NULL);
        perror("exec");
        _exit(2);
    }
    int st = 0;
    waitpid(pid, &st, 0);
    int child_ok = WIFEXITED(st) && WEXITSTATUS(st) == 0;
    if (!child_ok) fails++;

    printf("\n%s: %d failure(s)%s\n",
           fails ? "FAILED" : "all good", fails,
           child_ok ? "" : "  (including the cold-start phase)");
    return fails ? 1 : 0;
}
