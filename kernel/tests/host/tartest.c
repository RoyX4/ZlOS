/* tartest.c - the archive the Archive Manager writes, read back by real tar.
 *
 * THE DEFECT THIS EXISTS FOR. "Create archive" was a primary-styled button
 * whose whole effect was `s3ar_open = 1` - a view flag. No ustar header was
 * emitted, no block was written, no file appeared, and the toolbar named
 * "/tmp/build.tar", a path nothing on the volume answered to.
 *
 * WHY THIS IS NOT ENOUGH ON ITS OWN, AND WHAT MAKES IT ENOUGH. A test that
 * parses back the same header layout the writer just wrote agrees with itself
 * by construction - it can only catch a typo, never a misreading of the
 * format. So this writes the archive to a file and the SHELL'S OWN tar reads
 * it: `tar -tvf` for the listing and `tar -xOf` for the bytes. If GNU tar and
 * this file disagree, GNU tar is right. That is the whole point of choosing a
 * format somebody else already implemented.
 *
 * The in-process assertions below are the ones tar cannot make for us: that
 * tar_size() predicts the exact length tar_build() produces (the Archive
 * Manager refuses on that figure BEFORE it commits, so a wrong prediction is a
 * wrong refusal), and that a full buffer is refused rather than truncated.
 *
 * Build and run:  ./build.sh && ./tartest
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef unsigned int u32;

int  fs_mkfs(void);
int  fs_mount(void);
int  fs_create(const char *name, u32 bytes);
int  fs_write(int idx, const void *src, u32 bytes);
int  fs_find(const char *name);
u32  tar_size(void);
u32  tar_build(void *dst, u32 cap);

void zl_putc_pub(char c) { (void)c; }

/* ---- the same RAM disk fstest.c uses, minus the fault injection ----------- */
#define DEV_BSIZE 512
#define DEV_BYTES (16u * 1024 * 1024)
static unsigned char *disk;
int fsdev_read(u32 lba, void *buf)
{
    if (lba >= DEV_BYTES / DEV_BSIZE) return 0;
    memcpy(buf, disk + (size_t)lba * DEV_BSIZE, DEV_BSIZE);
    return 1;
}
int fsdev_write(u32 lba, const void *buf)
{
    if (lba >= DEV_BYTES / DEV_BSIZE) return 0;
    memcpy(disk + (size_t)lba * DEV_BSIZE, buf, DEV_BSIZE);
    return 1;
}
int fsdev_sync(void)   { return 1; }
u32 fsdev_bsize(void)  { return DEV_BSIZE; }
u32 fsdev_blocks(void) { return DEV_BYTES / DEV_BSIZE; }

static int fails;
static void check(const char *what, int ok, const char *detail)
{
    printf("  %-46s %s  %s\n", what, ok ? "ok  " : "FAIL", detail ? detail : "");
    if (!ok) fails++;
}

/* run a shell command and return its stdout, trimmed */
static char *shell(const char *cmd)
{
    static char buf[8192];
    buf[0] = 0;
    FILE *f = popen(cmd, "r");
    if (!f) return buf;
    size_t n = fread(buf, 1, sizeof buf - 1, f);
    buf[n] = 0;
    pclose(f);
    while (n && (buf[n-1] == '\n' || buf[n-1] == ' ')) buf[--n] = 0;
    return buf;
}

static void mkfile(const char *name, const char *body)
{
    u32 len = (u32)strlen(body);
    int i = fs_create(name, len);
    if (i < 0) { printf("  could not create %s\n", name); fails++; return; }
    if (!fs_write(i, body, len)) { printf("  could not write %s\n", name); fails++; }
}

int main(void)
{
    disk = calloc(DEV_BYTES, 1);
    if (!fs_mkfs() || !fs_mount()) { printf("mkfs/mount failed\n"); return 1; }

    printf("tartest - the archive, read back by the shell's own tar\n\n");

    /* THREE MEMBERS CHOSEN TO HIT THE PADDING CASES. One that is exactly a
     * block, one that is one byte over (so the writer must pad a nearly-empty
     * second block) and one that is short. A tar writer that rounds the wrong
     * way passes on the short file and fails on the other two. */
    static char exact[512], over[513];
    memset(exact, 'A', sizeof exact);
    memset(over,  'B', sizeof over);
    mkfile("short.txt", "hello from zlfs\n");
    { u32 l = (u32)sizeof exact; int i = fs_create("exact.bin", l);
      if (i < 0 || !fs_write(i, exact, l)) { printf("  exact.bin failed\n"); fails++; } }
    { u32 l = (u32)sizeof over;  int i = fs_create("over.bin", l);
      if (i < 0 || !fs_write(i, over, l))  { printf("  over.bin failed\n");  fails++; } }

    u32 predicted = tar_size();
    static unsigned char arc[4u * 1024 * 1024];
    u32 wrote = tar_build(arc, (u32)sizeof arc);

    char d[128];
    snprintf(d, sizeof d, "(%u predicted, %u written)", predicted, wrote);
    check("tar_size predicts tar_build exactly", predicted == wrote && wrote > 0, d);

    /* THE EXPECTED LENGTH, TERM BY TERM. Written out rather than derived from
     * the writer, so a change in the padding rule fails here instead of moving
     * the expectation with it:
     *
     *     short.txt   16 bytes  ->  512 header + 512 data  = 1024
     *     exact.bin  512 bytes  ->  512 header + 512 data  = 1024
     *     over.bin   513 bytes  ->  512 header + 1024 data = 1536
     *     the two end blocks                               = 1024
     *                                                        ----
     *                                                        4608
     *
     * This said 5120 on its first run and the code said 4608. The code was
     * right - the sum above had been done in someone's head and 1536 was added
     * as if over.bin were 2048. An arithmetic slip in a test reads exactly like
     * a bug in the thing tested, which is why the terms are now on the page. */
    snprintf(d, sizeof d, "(%u bytes)", wrote);
    check("the length is what the members add up to", wrote == 4608, d);

    check("a buffer one byte short is REFUSED, not truncated",
          tar_build(arc, wrote - 1) == 0, "");

    const char *path = "/tmp/zltartest.tar";
    FILE *f = fopen(path, "wb");
    if (!f) { printf("  cannot write %s\n", path); return 1; }
    fwrite(arc, 1, wrote, f);
    fclose(f);

    /* ---- and now the part that is not us marking our own homework -------- */
    char cmd[256];
    snprintf(cmd, sizeof cmd, "tar -tf %s 2>&1 | tr '\\n' ' '", path);
    char *listing = strdup(shell(cmd));
    check("GNU tar lists all three members",
          strstr(listing, "short.txt") && strstr(listing, "exact.bin") &&
          strstr(listing, "over.bin"), listing);

    /* 2>/dev/null, NOT 2>&1. With stderr merged this counted tar's THREE
     * lines of "this does not look like a tar archive" as three members and
     * stayed green through a planted checksum defect that failed every other
     * assertion here. A count of error text is not a count of members. */
    snprintf(cmd, sizeof cmd, "tar -tf %s 2>/dev/null | grep -c . ", path);
    check("...and exactly three", atoi(shell(cmd)) == 3, "");

    snprintf(cmd, sizeof cmd, "tar -xOf %s short.txt 2>&1", path);
    check("the bytes come back through tar unchanged",
          strcmp(shell(cmd), "hello from zlfs") == 0, "");

    snprintf(cmd, sizeof cmd, "tar -xOf %s over.bin 2>&1 | wc -c", path);
    snprintf(d, sizeof d, "(%s bytes)", shell(cmd));
    check("the 513-byte member is 513 bytes, not 1024", atoi(shell(cmd)) == 513, d);

    /* tar reports a checksum error on stderr and still lists, so the exit
     * status of a strict read is the assertion - not the presence of names. */
    snprintf(cmd, sizeof cmd, "tar -tf %s >/dev/null 2>&1; echo $?", path);
    check("tar accepts the headers without complaint", atoi(shell(cmd)) == 0, "");

    /* ---- AN ARCHIVE MUST NOT CONTAIN ITSELF -----------------------------
     * The pane writes its output back onto the volume it just packed, so
     * without a skip the second press packs the first archive, the third packs
     * the second, and the file roughly doubles every time. That failure looks
     * exactly like ordinary use, which is why it is asserted rather than
     * commented. */
    { int i = fs_create("zlfs.tar", wrote);
      if (i < 0 || !fs_write(i, arc, wrote)) { printf("  could not place zlfs.tar\n"); fails++; } }
    u32 again = tar_size();
    snprintf(d, sizeof d, "(%u before, %u after zlfs.tar landed)", wrote, again);
    check("the archive does not grow by containing itself", again == wrote, d);

    u32 rebuilt = tar_build(arc, (u32)sizeof arc);
    f = fopen(path, "wb"); fwrite(arc, 1, rebuilt, f); fclose(f);
    snprintf(cmd, sizeof cmd, "tar -tf %s 2>/dev/null | grep -c zlfs.tar", path);
    check("...and tar cannot find it in there", atoi(shell(cmd)) == 0, "");

    free(listing);
    unlink(path);
    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
