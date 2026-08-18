/* settingstest.c - the settings block, against a fake disk.
 *
 * This is the FIRST CODE IN THE PROJECT THAT WRITES TO A DISK, and its stated
 * gate - change a setting, reboot in QEMU, confirm it survived; then corrupt
 * the block and confirm it falls back and says so - needs a booting kernel.
 * The tree does not link (T-13, another session's uncommitted work), so that
 * gate cannot run.
 *
 * This runs the SAME settings.c against a fake NVMe: a 64 MiB array of blocks
 * and a transfer page, with read/write that can be made to fail on demand.
 * It is strictly more thorough than the reboot gate, because a reboot can only
 * show one corruption at a time and this walks every byte of the record:
 *
 *   - a save/load round trip returns exactly what went in
 *   - LOADING NEVER WRITES - the property "never write on boot" hangs on it
 *   - bad magic       -> defaults, and a line saying so
 *   - EVERY single-bit flip in the record -> defaults, and a line
 *   - wrong version   -> defaults, and a line
 *   - a short/absent device -> refuses, and a line
 *   - a valid checksum carrying insane values -> clamped, not applied
 *
 * The one thing it cannot show is that the real NVMe driver round-trips a
 * block, which is exactly what the QEMU gate is for. Run it the moment T-13
 * closes.
 *
 * Build and run:  ./build.sh && ./settingstest
 */
#include <stdio.h>
#include <string.h>

#include "../ui.h"

int  settings_save(void);
int  settings_load(void);
int  settings_event(int app,int win,int type,int code,int x,int y);
void settings_apply(void);

/* ---- the fake disk -------------------------------------------------------- */
#define BLOCKS   131072u          /* 64 MiB of 512-byte blocks, like try.sh   */
#define BLKSZ    512u
static unsigned char disk[BLOCKS][BLKSZ];
static unsigned char page[4096];

static int fail_read = 0, fail_write = 0, ready = 1, blocksize = BLKSZ;
static unsigned nblocks = BLOCKS;
static int writes = 0, reads = 0;

int  nvme_ready(void)        { return ready; }
unsigned nvme_blocksize(void){ return (unsigned)blocksize; }
unsigned nvme_blocks_lo(void){ return nblocks; }
unsigned nvme_blocks_hi(void){ return 0; }
int  nvme_data_byte(int i)   { return (i >= 0 && i < 4096) ? page[i] : 0; }
void nvme_data_set(int i,int v){ if (i >= 0 && i < 4096) page[i] = (unsigned char)v; }

int nvme_read_block(unsigned lo, unsigned hi)
{
    reads++;
    if (fail_read || hi || lo >= nblocks) return 0;
    memcpy(page, disk[lo], BLKSZ);
    return 1;
}

int nvme_write_block(unsigned lo, unsigned hi)
{
    writes++;
    if (fail_write || hi || lo >= nblocks) return 0;
    memcpy(disk[lo], page, BLKSZ);
    return 1;
}

/* ---- the rest of settings.c's world --------------------------------------- */
static char logbuf[4096];
static int  loglen;
void zl_putc_pub(char c){ if (loglen < (int)sizeof logbuf - 1) logbuf[loglen++] = c; }
static void logclear(void){ loglen = 0; logbuf[0] = 0; }
static const char *logtext(void){ logbuf[loglen] = 0; return logbuf; }

/* fb.c is NOT linked. settings.c and ui.c between them touch a dozen fb_*
 * symbols and none of them matters to a disk block, so they are stubs - the
 * test stays about the record and links in a fraction of a second. ui.c IS
 * linked for real, so ui_theme_init/ui_theme_set behave exactly as they do in
 * the kernel and "the loaded scale was actually applied" means something. */
void fb_set_subpixel(int on){ (void)on; }
unsigned int fb_pxw(void){ return 1280; }
unsigned int fb_pxh(void){ return 800; }
void fb_fill_px(int x,int y,int w,int h,unsigned c){(void)x;(void)y;(void)w;(void)h;(void)c;}
void fb_rrect(int x,int y,int w,int h,int r,unsigned c){(void)x;(void)y;(void)w;(void)h;(void)r;(void)c;}
void fb_text_prop(int x,int y,const char*s,unsigned c){(void)x;(void)y;(void)s;(void)c;}
int  fb_text_prop_w(const char*s){ int n=0; while(s[n])n++; return n*8; }
int  fb_text_prop_h(void){ return 16; }
int  fb_cell_w(void){ return 8; }
int  fb_cell_h(void){ return 16; }
void fb_clip(int x,int y,int w,int h){(void)x;(void)y;(void)w;(void)h;}
void fb_clip_none(void){}
void fb_clip_get(int*a,int*b,int*c,int*d){ if(a)*a=0; if(b)*b=0; if(c)*c=1280; if(d)*d=800; }
void input_set_speed(int p){ (void)p; }
void input_set_accel(int o){ (void)o; }
void wm_set_anim(int o){ (void)o; }
void wm_damage(int x,int y,int w,int h){ (void)x;(void)y;(void)w;(void)h; }
void wm_client(int win,int*x,int*y,int*w,int*h)
{ (void)win; *x=0; *y=0; *w=400; *h=600; }

/* ui.c is linked for real, so ui_theme_init/set behave as they do in the kernel */

/* ---- assertions ----------------------------------------------------------- */
static int fails;
static void ok(const char *what, int cond)
{
    printf("  %-58s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}

#define LBA 64u
#define RECLEN 36

/* Build a VALID record independently of settings.c and put it on the disk.
 *
 * The test deliberately re-implements the on-disk format rather than calling a
 * setter, and that is the point: settings.c's reader is then checked against a
 * second implementation of the layout instead of against itself. A writer and
 * a reader that share one buggy encoder agree perfectly and are both wrong.
 * There is no setter API either - the app owns its state, which is ui.c's whole
 * design - so this is also the only way in that does not add kernel code whose
 * sole caller is a test. */
static void mk_block(int accent,int scale,int speed,int accel,int sub,int anim)
{
    unsigned char r[RECLEN];
    memset(r, 0, sizeof r);
    r[0]='z'; r[1]='l'; r[2]='S'; r[3]='1';
    r[4]=1; r[5]=0;                 /* version 1 */
    r[6]=6; r[7]=0;                 /* six fields */
    unsigned v[6] = { (unsigned)accent, (unsigned)scale, (unsigned)speed,
                      (unsigned)accel, (unsigned)sub, (unsigned)anim };
    for (int f=0; f<6; f++)
        for (int b=0; b<4; b++)
            r[12 + f*4 + b] = (unsigned char)((v[f] >> (8*b)) & 0xFF);
    unsigned h = 2166136261u;                    /* FNV-1a, checksum field zero */
    for (int i=0;i<RECLEN;i++){ h ^= r[i]; h *= 16777619u; }
    for (int i=0;i<4;i++) r[8+i] = (unsigned char)((h >> (8*i)) & 0xFF);
    memset(disk[LBA], 0, BLKSZ);
    memcpy(disk[LBA], r, RECLEN);
}

/* ...and adopt it, which is the only way to move settings.c's state */
static void set_all(int accent,int scale,int speed,int accel,int sub,int anim)
{
    mk_block(accent,scale,speed,accel,sub,anim);
    settings_load();
}

int main(void)
{
    printf("settingstest - the settings block, against a fake disk\n\n");
    ui_theme_init(2);
    settings_apply();

    /* ---- nothing on disk yet -------------------------------------------- */
    logclear();
    writes = 0;
    int got = settings_load();
    ok("a blank disk is not a valid block", got == 0);
    ok("...and it says so", strstr(logtext(), "bad magic") != NULL);
    ok("...and LOADING NEVER WRITES", writes == 0);

    /* ---- round trip ------------------------------------------------------
     * The fixture is built by mk_block, independently of settings.c. So:
     * load it, save it back, and the bytes must be IDENTICAL - which checks
     * settings.c's writer against a second implementation of the format, not
     * against its own reader. */
    mk_block(2, 3, 275, 0, 0, 1);
    unsigned char fixture[BLKSZ];
    memcpy(fixture, disk[LBA], BLKSZ);
    logclear();
    ok("loading the independently-built fixture succeeds", settings_load() == 1);
    memset(disk[LBA], 0, BLKSZ);
    ok("saving reports success", settings_save() == 1);
    ok("...and the magic landed at the fixed LBA",
       disk[LBA][0]=='z' && disk[LBA][1]=='l' &&
       disk[LBA][2]=='S' && disk[LBA][3]=='1');
    ok("...and the block it wrote is byte-identical to the fixture",
       memcmp(disk[LBA], fixture, BLKSZ) == 0);

    set_all(0, 1, 100, 1, 1, 0);              /* move the state away... */
    memcpy(disk[LBA], fixture, BLKSZ);        /* ...and put the fixture back */
    logclear();
    writes = 0;
    ok("loading a good block succeeds", settings_load() == 1);
    ok("...and it STILL does not write", writes == 0);
    ok("...accent survived",   settings_accent() == 2);
    ok("...scale survived",    settings_scale() == 3);
    ok("...speed survived",    settings_speed() == 275);
    ok("...accel survived",    settings_accel() == 0);
    ok("...subpixel survived", settings_subpixel() == 0);
    ok("...anim survived",     settings_anim() == 1);
    ok("...and it applied the theme it loaded",
       ui_theme()->scale == 3);

    /* keep a pristine copy to restore between corruption cases */
    unsigned char good[BLKSZ];
    memcpy(good, disk[LBA], BLKSZ);

    /* ---- bad magic -------------------------------------------------------
     * set_all writes the disk (it is the only way to move settings.c's state),
     * so it has to happen BEFORE the corruption or it wipes it. That ordering
     * slip made this case pass vacuously the first time it ran. */
    set_all(1, 2, 100, 1, 1, 1);
    memcpy(disk[LBA], good, BLKSZ);
    disk[LBA][1] ^= 0xFF;
    logclear();
    ok("a corrupted MAGIC is refused", settings_load() == 0);
    ok("...and says 'bad magic'", strstr(logtext(), "bad magic") != NULL);
    ok("...and the settings are untouched", settings_scale() == 2);

    /* ---- wrong version --------------------------------------------------- */
    memcpy(disk[LBA], good, BLKSZ);
    disk[LBA][4] = 99;
    logclear();
    ok("a future VERSION is refused", settings_load() == 0);
    ok("...and says which version it found",
       strstr(logtext(), "version 99") != NULL);

    /* ---- EVERY single-bit flip in the record ----------------------------- */
    int flips = 0, caught = 0, silent = 0;
    for (int byte = 0; byte < RECLEN; byte++)
        for (int bit = 0; bit < 8; bit++) {
            memcpy(disk[LBA], good, BLKSZ);
            disk[LBA][byte] ^= (unsigned char)(1 << bit);
            logclear();
            flips++;
            if (settings_load() == 0) {
                caught++;
                if (!loglen) silent++;
            }
        }
    ok("every single-bit flip in the record is refused", caught == flips);
    ok("...and none of them is refused SILENTLY", silent == 0);
    printf("       (%d flips, %d caught, %d silent)\n", flips, caught, silent);

    /* ---- a bit flip in the PAYLOAD TAIL, past the record ------------------ */
    memcpy(disk[LBA], good, BLKSZ);
    disk[LBA][RECLEN + 20] ^= 0xFF;
    logclear();
    ok("a flip PAST the record does not matter", settings_load() == 1);

    /* ---- torn write: the tail of a previous, longer block ---------------- */
    memcpy(disk[LBA], good, BLKSZ);
    memset(disk[LBA] + 8, 0, 4);              /* checksum zeroed, as if torn */
    logclear();
    ok("a zeroed checksum is refused", settings_load() == 0);
    ok("...and says CHECKSUM", strstr(logtext(), "CHECKSUM") != NULL);

    /* ---- a valid block carrying insane values ---------------------------- */
    /* Build one by hand: correct magic/version/count and a correct checksum,
     * but a UI scale of two billion. A checksum proves the bytes are the ones
     * that were written; it proves nothing about whether they are sensible. */
    {
        unsigned char r[RECLEN];
        memcpy(r, good, RECLEN);
        for (int i = 0; i < 4; i++) r[8+i] = 0;
        unsigned vals[6] = { 999u, 2000000000u, 4000000000u, 7u, 9u, 3u };
        for (int f = 0; f < 6; f++)
            for (int b = 0; b < 4; b++)
                r[12 + f*4 + b] = (unsigned char)((vals[f] >> (8*b)) & 0xFF);
        unsigned h = 2166136261u;
        for (int i = 0; i < RECLEN; i++) { h ^= r[i]; h *= 16777619u; }
        for (int i = 0; i < 4; i++) r[8+i] = (unsigned char)((h >> (8*i)) & 0xFF);
        memcpy(disk[LBA], r, RECLEN);
    }
    logclear();
    ok("a VALID block with insane values still loads", settings_load() == 1);
    ok("...but the scale is clamped", settings_scale() >= 1 && settings_scale() <= 4);
    ok("...and the speed is clamped", settings_speed() >= 25 && settings_speed() <= 400);
    ok("...and the accent index is in range",
       settings_accent() >= 0 && settings_accent() < 5);
    ok("...and the booleans are 0 or 1",
       (settings_accel()|settings_subpixel()|settings_anim()) <= 1);

    /* ---- the device is missing or unusable ------------------------------- */
    memcpy(disk[LBA], good, BLKSZ);
    ready = 0;
    logclear(); writes = 0;
    ok("with no NVMe, save refuses", settings_save() == 0);
    ok("...without attempting a write", writes == 0);
    ok("...and says so", strstr(logtext(), "no NVMe") != NULL);
    logclear();
    ok("with no NVMe, load refuses", settings_load() == 0);
    ok("...and says so", strstr(logtext(), "no NVMe") != NULL);
    ready = 1;

    blocksize = 16;                            /* smaller than the record */
    logclear(); writes = 0;
    ok("a block too small to hold the record refuses", settings_save() == 0);
    ok("...without attempting a write", writes == 0);
    ok("...and says so", strstr(logtext(), "block size") != NULL);
    blocksize = BLKSZ;

    nblocks = 32;                              /* LBA 64 is past the end */
    logclear(); writes = 0;
    ok("an LBA past the end of the namespace refuses", settings_save() == 0);
    ok("...without attempting a write", writes == 0);
    ok("...and says so", strstr(logtext(), "past the end") != NULL);
    nblocks = BLOCKS;

    /* ---- the write itself failing ---------------------------------------- */
    fail_write = 1;
    logclear();
    ok("a failed NVMe write is reported, not swallowed", settings_save() == 0);
    ok("...and says FAILED", strstr(logtext(), "FAILED") != NULL);
    fail_write = 0;

    /* A read failure is now caught by the disk-ownership probe, which reads
     * LBA 0 before anything else - earlier than the record read, and a
     * strictly better place to give up. */
    fail_read = 1;
    logclear();
    ok("a failed NVMe read falls back to defaults", settings_load() == 0);
    ok("...and says which read failed", strstr(logtext(), "cannot read LBA 0") != NULL);
    fail_read = 0;

    /* ---- the write stays inside its own block ---------------------------- */
    memset(disk, 0xA5, sizeof disk[0] * 4);    /* poison LBA 0..3 */
    memset(disk[LBA-1], 0xA5, BLKSZ);
    memset(disk[LBA+1], 0xA5, BLKSZ);
    set_all(1, 2, 150, 1, 1, 1);
    settings_save();
    int neigh_ok = 1;
    for (unsigned i = 0; i < BLKSZ; i++)
        if (disk[LBA-1][i] != 0xA5 || disk[LBA+1][i] != 0xA5) neigh_ok = 0;
    ok("the write touches NEITHER neighbouring block", neigh_ok);

    /* and the tail of our own block is zeroed rather than carrying whatever
     * the shared transfer page happened to hold */
    int tail_zero = 1;
    for (unsigned i = RECLEN; i < BLKSZ; i++) if (disk[LBA][i]) tail_zero = 0;
    ok("...and the block's tail is zeroed, not stale page contents", tail_zero);

    /* ---- IS THIS DISK EVEN OURS? ----------------------------------------
     * The bug the FEEL-PROMPT section 6 adversarial review caught, and the
     * only one here that could destroy a person's machine.
     *
     * nvme_find() takes the FIRST NVMe device it sees, with no filter on model,
     * serial or size. On the ThinkPad test laptop that is the internal 477 GB
     * system SSD, and install-esp.sh is a documented way to boot zlOS on that
     * laptop. The block was at LBA 2048 - which on this very machine is
     *
     *   $ cat /sys/block/nvme0n1/nvme0n1p1/start   ->  2048
     *   $ lsblk -o NAME,START,PARTTYPENAME         ->  nvme0n1p1  2048  EFI System
     *
     * so "open Settings and click a control" would have overwritten the EFI
     * System Partition's boot sector and the machine would not have booted.
     * These are the refusals that stop it. */
    memset(disk, 0, sizeof disk[0] * 4);
    mk_block(1, 2, 100, 1, 1, 1);
    ok("a blank scratch disk is still writable", settings_save() == 1);

    /* an MBR or a GPT protective MBR: 0x55 0xAA at the end of LBA 0 */
    disk[0][510] = 0x55; disk[0][511] = 0xAA;
    logclear(); writes = 0;
    ok("a disk with a PARTITION TABLE is refused", settings_save() == 0);
    ok("...without attempting a write", writes == 0);
    ok("...and says PARTITION TABLE", strstr(logtext(), "PARTITION TABLE") != NULL);
    logclear();
    ok("...and load is refused too", settings_load() == 0);
    disk[0][510] = 0; disk[0][511] = 0;

    /* a GPT header at LBA 1, even with no protective MBR */
    memcpy(disk[1], "EFI PART", 8);
    logclear(); writes = 0;
    ok("a GPT disk is refused", settings_save() == 0);
    ok("...without attempting a write", writes == 0);
    ok("...and says GPT", strstr(logtext(), "GPT") != NULL);
    memset(disk[1], 0, 8);

    /* ...and a disk far too big to be the 64 MiB scratch image */
    nblocks = 931 * 1024 * 1024 / 512 * 1024;   /* ~477 GB, the real laptop SSD */
    logclear(); writes = 0;
    ok("a namespace far too large to be the scratch disk is refused",
       settings_save() == 0);
    ok("...without attempting a write", writes == 0);
    ok("...and says too large", strstr(logtext(), "too large") != NULL);
    nblocks = BLOCKS;

    /* the guard must run on EVERY write, not once at startup */
    mk_block(1, 2, 100, 1, 1, 1);
    ok("a clean disk still saves after all that", settings_save() == 1);
    disk[0][510] = 0x55; disk[0][511] = 0xAA;
    writes = 0;
    ok("...and partitioning it later is caught on the NEXT write",
       settings_save() == 0 && writes == 0);
    disk[0][510] = 0; disk[0][511] = 0;

    /* ---- ONE WRITE PER GESTURE, NOT PER MOUSE EVENT ---------------------
     * wm.c hands the app a pointer grab for the whole duration of a press and
     * delivers an event per mouse motion, so saving inside settings_commit
     * issued a synchronous block write for every pixel a slider was dragged -
     * the adversarial review measured 376 for one gesture. A change now marks
     * the block dirty and the write happens on button-up. */
    memset(disk, 0, sizeof disk[0] * 4);
    mk_block(0, 1, 100, 1, 1, 1);
    settings_load();
    writes = 0;
    int moved = 0;
    for (int y = 0; y < 600; y += 2) {                 /* find a live control */
        int before = settings_scale() + settings_speed() + settings_accent();
        settings_event(3, 0, 4 /*EV_MOUSE*/, 1, 200, y);
        if (settings_scale() + settings_speed() + settings_accent() != before) {
            moved = y;
            break;
        }
    }
    ok("a press on a control changes it", moved != 0);
    /* ...now drag: 40 more motion events with the button still down */
    for (int i = 0; i < 40; i++)
        settings_event(3, 0, 4, 1, 100 + i * 5, moved);
    ok("...and 40 motion events later, STILL nothing has been written",
       writes == 0);
    settings_event(3, 0, 4, 0, 300, moved);            /* button up */
    ok("...the write happens once, on release", writes == 1);
    settings_event(3, 0, 4, 0, 300, moved);            /* a second release */
    ok("...and a release with nothing pending writes nothing", writes == 1);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}

