/* settingstest.c - the settings block against a fake disk, and the two-pane
 * layout against the client rectangle it actually gets.
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
 * ...AND THE LAYOUT, because the failure mode of an immediate-mode app is not
 * a crash. ui.c's place() never reports running out of room: a widget past the
 * bottom of the client area is still laid out, still counted for widget
 * identity, and simply drawn outside the scissor - invisible AND unclickable,
 * with nothing anywhere saying so. This file has already shipped that bug once
 * (roughly 995 px of content in a 642 px client). So the last section asserts,
 * for every page and at every UI scale the app can be set to:
 *
 *   - the page's content ends INSIDE the client rectangle it really gets
 *     (486x332 design px minus wm.c's own chrome), with slack to spare
 *   - the draw pass and the hit-test pass emit the SAME widget count, which is
 *     the whole of widget identity in an immediate-mode toolkit
 *   - the shadow cursor never fails a step (settings_flow_fault)
 *   - every sidebar row selects its own page, and selecting a page WRITES
 *     NOTHING - which pane is open is not a setting
 *   - one write per gesture still holds, driven through the real slider on the
 *     Devices page rather than through whatever a blind y-scan first hit
 *
 * Build and run:  ./build.sh && ./settingstest
 */
#include <stdio.h>
#include <string.h>

#include "../../src/graphics/ui/ui.h"
#include "../../src/graphics/ui/design.h"

int  settings_save(void);
int  settings_load(void);
int  settings_event(int app,int win,int type,int code,int x,int y);
void settings_draw(int app,int x,int y,int w,int h,int focused);
void settings_apply(void);
/* Declared here rather than in ui.h: they exist for this gate, and ui.h is the
 * contract between the compositor, the toolkit and the apps. */
int  settings_page(void);
int  settings_page_count(void);
const char *settings_page_name(int i);
int  settings_probe_fit(int page,int w,int h);
int  settings_flow_fault(void);

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

/* fb.c is NOT linked. settings.c, ui.c and uikit.c between them touch two dozen
 * fb_* symbols and none of them matters to a disk block, so they are stubs -
 * the test stays about the record and links in a fraction of a second. ui.c
 * AND uikit.c are linked for real, so ui_theme_init/ui_theme_set and the whole
 * widget catalogue behave exactly as they do in the kernel: "the loaded scale
 * was actually applied" and "this page fits" both mean something.
 *
 * THE TEXT METRICS ARE NOT ARBITRARY. fb.c sizes proportional text by ROLE and
 * follows the UI scale - `role_base[3] = {8, 12, 16}` design px, scaled, with a
 * 12 px legibility floor (fb.c prop_cell, ~line 2921). A stub returning a
 * constant would make every layout assertion below scale-blind, and scale is
 * precisely the axis the overflow bug lived on. So prop_cell is mirrored here.
 * The per-glyph ADVANCE is still an approximation - half the cell, against
 * fb.c's generated advance tables - so widths here are indicative while
 * heights are exact, and the assertions below lean on heights and leave slack
 * on widths. */
void fb_set_subpixel(int on){ (void)on; }
unsigned int fb_pxw(void){ return 1280; }
unsigned int fb_pxh(void){ return 800; }

static int stub_strlen(const char*s){ int n=0; if(!s) return 0; while(s[n])n++; return n; }
static int prop_cell_stub(int role)
{
    static const int base[3] = { 8, 12, 16 };   /* fb.c role_base, verbatim */
    int q8 = ui_metric(UI_METRIC_SCALE_Q8), h;
    if (role < 0) role = 0;
    if (role > 2) role = 2;
    h = (base[role] * q8 + 128) / 256;
    return h < 12 ? 12 : h;                     /* fb.c's legibility floor  */
}
int  fb_text_role_h(int role){ return prop_cell_stub(role); }
int  fb_text_role_w(const char*s,int role,int weight)
{ (void)weight; return stub_strlen(s) * prop_cell_stub(role) / 2; }
void fb_text_role(int x,int y,const char*s,unsigned c,int role,int weight)
{(void)x;(void)y;(void)s;(void)c;(void)role;(void)weight;}
void fb_text_aa(int x,int y,const char*s,unsigned c){(void)x;(void)y;(void)s;(void)c;}

void fb_fill_px(int x,int y,int w,int h,unsigned c){(void)x;(void)y;(void)w;(void)h;(void)c;}
void fb_box(int x,int y,int w,int h,unsigned c){(void)x;(void)y;(void)w;(void)h;(void)c;}
void fb_line(int x0,int y0,int x1,int y1,unsigned c){(void)x0;(void)y0;(void)x1;(void)y1;(void)c;}
void fb_rrect(int x,int y,int w,int h,int r,unsigned c){(void)x;(void)y;(void)w;(void)h;(void)r;(void)c;}
void fb_fill_blend(int x,int y,int w,int h,unsigned c,int a){(void)x;(void)y;(void)w;(void)h;(void)c;(void)a;}
void fb_rrect_blend(int x,int y,int w,int h,int r,unsigned c,int a){(void)x;(void)y;(void)w;(void)h;(void)r;(void)c;(void)a;}
void fb_text_prop(int x,int y,const char*s,unsigned c){(void)x;(void)y;(void)s;(void)c;}
int  fb_text_prop_w(const char*s){ return fb_text_role_w(s, 1, 0); }
int  fb_text_prop_h(void){ return prop_cell_stub(1); }
/* the console cell, which fb.c also scales - uikit.c draws every mono run in it */
int  fb_cell_w(void){ return (8 * ui_metric(UI_METRIC_SCALE_Q8) + 128) / 256; }
int  fb_cell_h(void){ return (16 * ui_metric(UI_METRIC_SCALE_Q8) + 128) / 256; }
void fb_clip(int x,int y,int w,int h){(void)x;(void)y;(void)w;(void)h;}
void fb_clip_none(void){}
void fb_clip_get(int*a,int*b,int*c,int*d){ if(a)*a=0; if(b)*b=0; if(c)*c=1280; if(d)*d=800; }

/* THE RICH-TEXT PAIR, and this test could not LINK without them.
 *
 * uikit.c started calling fb_text_rich / fb_text_rich_w at 3b8692b - the depth
 * pass that gave chips, tabs, menus and the segmented control a real
 * proportional measure - and settingstest's stub set never followed. It has not
 * built since, which means it has not RUN since, and nothing said so: the host
 * build.sh carries on past a link failure and prints its next line, so a
 * green-looking run had one binary silently missing from it.
 *
 * That is docs/GUARDS-THAT-DID-NOT-GUARD.md exactly, with the twist that the
 * gate did not go quiet on its own - a change of mine put it out and the
 * failure was three lines above where anyone was reading.
 *
 * Measured the same way ui_text_w's stub is, so widths stay hand-checkable:
 * half an em per character at the size asked for. */
int  fb_text_rich_w(const char *s, int len, int size, int style)
{ (void)s; (void)style; return len * size / 2; }
void fb_text_rich(int px, int py, const char *s, int len, unsigned int fg,
                  int size, int style)
{ (void)px; (void)py; (void)s; (void)len; (void)fg; (void)size; (void)style; }
void input_set_speed(int p){ (void)p; }
void input_set_accel(int o){ (void)o; }
void wm_set_anim(int o){ (void)o; }
void wm_damage(int x,int y,int w,int h){ (void)x;(void)y;(void)w;(void)h; }

/* THE CLIENT RECTANGLE THE APP REALLY GETS. kernel.zl:4357 opens Settings at
 * 486x332 design px; wm.c's client_of takes 2 px of border off each side and
 * the title bar off the top. This stub used to return a made-up 400x600, which
 * would let a page that does not fit the real window pass the gate below - and
 * a page that does not fit is invisible and unclickable with nothing anywhere
 * reporting it. Ask the toolkit for the numbers rather than writing them down. */
static int client_w(void){ return UI_DP(ui_theme(), 486) - 4; }
static int client_h(void)
{ return UI_DP(ui_theme(), 332) - ui_metric(UI_METRIC_TITLE_H) - 2; }
void wm_client(int win,int*x,int*y,int*w,int*h)
{ (void)win; *x=0; *y=0; *w=client_w(); *h=client_h(); }

/* ui.c and uikit.c are linked for real, so the theme and every widget behave
 * exactly as they do in the kernel */

/* ---- assertions ----------------------------------------------------------- */
static int fails;
static void ok(const char *what, int cond)
{
    printf("  %-58s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}

#define LBA 64u
#define RECLEN 36
/* input.c's own SPD_MIN/SPD_MAX, which settings.c mirrors. */
#define SPD_MIN  25
#define SPD_MAX 400

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

    /* ====================================================================
     * THE TWO-PANE LAYOUT - ds-reference.html 700-740
     *
     * Everything above this line is about 36 bytes on a disk. Everything below
     * is about the app being reachable, which is the other half of "it works"
     * and the half that has already failed silently once.
     * ==================================================================== */
    printf("\n  -- the two-pane layout --\n");

    memset(disk, 0, sizeof disk[0] * 4);
    set_all(0, 2, 100, 1, 1, 1);              /* a known scale to measure at */

    /* The sidebar's own geometry, asked of the toolkit rather than written
     * down here - settings.c lays the nav rows out with exactly these three
     * calls, so a change to any of them moves the test's clicks with it. */
    int nav_x0 = UI_DP(ui_theme(), ZD_SIDEBAR_PX);
    int nav_y0 = UI_DP(ui_theme(), ZD_SIDEBAR_PY);
    int nav_st = ui_nav_h();
    int nav_w  = ui_sidebar_w() - 2 * nav_x0;
    int npage  = settings_page_count();

    ok("the sidebar carries five pages", npage == 5);
    ok("...named as the reference names them (3854)",
       strcmp(settings_page_name(0), "Appearance") == 0 &&
       strcmp(settings_page_name(1), "Windows")    == 0 &&
       strcmp(settings_page_name(2), "Displays")   == 0 &&
       strcmp(settings_page_name(3), "Devices")    == 0 &&
       strcmp(settings_page_name(4), "About")      == 0);
    ok("...and the sidebar rows fit inside the window",
       nav_y0 + npage * nav_st <= client_h());

    /* ---- EVERY PAGE FITS, AT EVERY SCALE --------------------------------
     * ui.c's place() does not report a widget that fell off the bottom: it is
     * laid out, counted for widget identity, and drawn outside the scissor -
     * invisible and unclickable, with nothing saying so. So the check has to
     * be arithmetic on the layout, and it has to run at every scale the UI
     * scale slider can select, because a page that fits at 1x is not a page
     * that fits at 3x. */
    {
        int worst = 1 << 30, worst_pg = -1, worst_sc = -1;
        int overflow = 0;
        for (int sc = 1; sc <= 3; sc++) {
            ui_theme_init(sc);
            for (int p = 0; p < npage; p++) {
                int slack = settings_probe_fit(p, client_w(), client_h());
                if (slack < 0) overflow++;
                if (slack < worst) { worst = slack; worst_pg = p; worst_sc = sc; }
            }
        }
        ok("no page overflows its client area at any UI scale", overflow == 0);
        printf("       (tightest: %s at %dx, %d px of slack)\n",
               settings_page_name(worst_pg), worst_sc, worst);
        ui_theme_init(2);
        settings_apply();
    }

    /* ---- THE DRAW PASS AND THE HIT-TEST PASS EMIT THE SAME SEQUENCE -----
     * This is the whole of widget identity in an immediate-mode toolkit: a
     * widget IS its ordinal. Two passes that emit different sequences hit-test
     * the wrong control, and they drift the moment someone edits one branch of
     * build_ui and not the other. Both passes now go through run_ui(), so the
     * counts must agree on every page. */
    {
        int mismatch = 0, empty = 0;
        for (int p = 0; p < npage; p++) {
            /* select the page through the real event path */
            settings_event(3, 0, 4, 1, nav_x0 + nav_w / 2,
                           nav_y0 + p * nav_st + nav_st / 2);
            settings_event(3, 0, 4, 0, 0, 0);
            settings_draw(3, 0, 0, client_w(), client_h(), 1);
            int drawn = ui_widget_count();
            /* a hit-test pass with the pointer nowhere: same sequence, no fire */
            settings_event(3, 0, 4, 1, -50, -50);
            int hit = ui_widget_count();
            settings_event(3, 0, 4, 0, 0, 0);
            if (drawn != hit) mismatch++;
            /* ui_widget_count counts what can FIRE. Four pages carry controls;
             * About carries ui_kv rows, which are information and fire
             * nothing - so it is the nav rows and only the nav rows. */
            if (p == 4 ? drawn != npage : drawn <= npage) empty++;
        }
        ok("draw and hit-test emit the same widget count on every page",
           mismatch == 0);
        ok("...four pages carry controls and About carries none", empty == 0);
        ok("...and the cursor never failed a step", settings_flow_fault() == 0);
    }

    /* ---- A PAGE IS NOT A SETTING ---------------------------------------
     * The sidebar changes what is on screen and nothing else. If selecting a
     * pane applied or persisted anything, reading the About page would issue a
     * disk write - and this app's whole write policy is "only when a setting
     * changes". */
    {
        int wrong = 0;
        writes = 0;
        for (int p = 0; p < npage; p++) {
            settings_event(3, 0, 4, 1, nav_x0 + nav_w / 2,
                           nav_y0 + p * nav_st + nav_st / 2);
            if (settings_page() != p) wrong++;
            settings_event(3, 0, 4, 0, 0, 0);          /* release the gesture */
        }
        ok("every sidebar row selects its own page", wrong == 0);
        ok("...and selecting a page writes NOTHING", writes == 0);
    }

    /* ---- ONE WRITE PER GESTURE, NOT PER MOUSE EVENT ---------------------
     * wm.c hands the app a pointer grab for the whole duration of a press and
     * delivers an event per mouse motion, so saving inside settings_commit
     * issued a synchronous block write for every pixel a slider was dragged -
     * the adversarial review measured 376 for one gesture. A change now marks
     * the block dirty and the write happens on button-up.
     *
     * This used to hunt for "a live control" by pressing every second row of a
     * made-up 400x600 client. That found whatever happened to be first, which
     * in a two-pane app is a sidebar row - and a sidebar row changes no
     * setting, so the hunt would have failed while reporting a layout problem
     * as a persistence problem. It now names the control: the Devices pane's
     * pointer-speed slider. */
    memset(disk, 0, sizeof disk[0] * 4);
    set_all(0, 2, 100, 1, 1, 1);
    settings_event(3, 0, 4, 1, nav_x0 + nav_w / 2,
                   nav_y0 + 3 * nav_st + nav_st / 2);      /* Devices */
    settings_event(3, 0, 4, 0, 0, 0);
    ok("the Devices pane is open", settings_page() == 3);

    writes = 0;
    int moved = 0, press_x = ui_sidebar_w() + UI_DP(ui_theme(), 16 + 13) + 4;
    for (int y = 0; y < client_h(); y += 2) {
        int before = settings_speed();
        settings_event(3, 0, 4 /*EV_MOUSE*/, 1, press_x, y);
        if (settings_speed() != before) { moved = y; break; }
    }
    ok("a press on the pointer-speed slider moves it", moved != 0);
    ok("...and it is inside the first card, not off the bottom",
       moved > 0 && moved < client_h());
    /* ...now drag: 40 more motion events with the button still down */
    for (int i = 0; i < 40; i++)
        settings_event(3, 0, 4, 1, press_x + i * 5, moved);
    ok("...and 40 motion events later, STILL nothing has been written",
       writes == 0);
    ok("...and the drag actually moved the value",
       settings_speed() != 100);
    settings_event(3, 0, 4, 0, 300, moved);            /* button up */
    ok("...the write happens once, on release", writes == 1);
    settings_event(3, 0, 4, 0, 300, moved);            /* a second release */
    ok("...and a release with nothing pending writes nothing", writes == 1);

    /* ---- THE SLIDER STILL REACHES BOTH ENDS -----------------------------
     * ui.c's slider had an off-by-one that made its maximum unreachable, and
     * it was found by driving THIS app's own slider. The control has moved
     * from the full client width into a card, so the span it is driven over
     * changed and the property has to be re-established at the new width.
     *
     * The sweep does not hardcode where the card is: ui.c's fire() needs the
     * pointer inside the widget's rect, so pressing every column of the row
     * and taking the extremes finds the track wherever the layout put it.
     *
     * It starts at the sidebar's right edge and not at zero. A press left of
     * that lands on a nav row, which changes the page - and then there is no
     * pointer-speed slider to sweep. That is a two-pane app's version of the
     * "find a live control by scanning" mistake, and it cost this block one
     * debug cycle. */
    {
        int lo_seen = 1 << 30, hi_seen = -1;
        /* EVERY column, not every second one. The bucket arithmetic gives the
         * top value exactly one pixel of track (376 values over ~536 px), so a
         * stride of 2 reports 399 and calls the maximum unreachable - which is
         * the very bug this assertion exists to catch. */
        for (int x = ui_sidebar_w(); x < client_w(); x++) {
            settings_event(3, 0, 4, 1, x, moved);
            if (settings_speed() < lo_seen) lo_seen = settings_speed();
            if (settings_speed() > hi_seen) hi_seen = settings_speed();
        }
        settings_event(3, 0, 4, 0, 0, 0);
        ok("sweeping the slider reaches its minimum", lo_seen == SPD_MIN);
        ok("...and reaches its maximum", hi_seen == SPD_MAX);
        printf("       (swept %d..%d over the card's track)\n", lo_seen, hi_seen);
    }

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}

