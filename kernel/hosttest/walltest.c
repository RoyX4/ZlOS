/* walltest.c - does the wallpaper cache survive the mode the ThinkPad has?
 *
 * fb.c's own comment about the wallpaper cache is not modest about it:
 *
 *     "There is no version of this that is affordable per frame; caching is
 *      not an optimisation here, it is the only way the look exists."
 *
 * And it is true: the v10 background is a gradient plus three radial glows and
 * two conic wedges, all translucent, at 22 cyc/px - about 22 ms at 1920x1200
 * against a 16.67 ms budget, repainted inside EVERY damage rectangle. Cached
 * and blitted it is ~1.5 cyc/px. So "the cache held" is not a performance
 * detail, it is whether the desktop exists.
 *
 * The cache and the blur slots come out of ONE 16 MiB bump arena (fb.c's
 * HI_BLUR..HI_NVME) that never rewinds - fb_cache_reset() has no callers. So
 * whoever asks first wins, and kernel.zl's boot order decided that. It used to
 * take the dock blur FIRST:
 *
 *     draw_wallpaper();  dock_blur = blur(dock strip);  wall_save()
 *
 * which is fine at 1920x1200 and fatal at 2560x1440 - the ThinkPad X1 Carbon
 * Gen 8's panel, per kernel/docs/gen9-modeset-plan.txt ("2560x1440 active /
 * 2720x1481 total"). This asserts that arithmetic against the SHIPPING fb.c
 * rather than restating it, because the numbers involved are exactly the kind
 * that get re-derived wrongly in a doc a month later.
 *
 * WHY THIS IS A HOST TEST AND NOT A BOOT. 2560x1440 is not reachable under
 * QEMU here: GRUB's mode list falls back to 800x600 on the emulated card, and
 * kernel.zl's own set_res() ladder tops out at 1920x1200. Measured - two boots
 * with ZLOS_GFXMODE=2560x1440,auto both came up 1920x1200. So the only way to
 * exercise the panel's real mode before somebody flashes a USB stick is here,
 * where fb.c is ordinary C against ordinary memory.
 *
 * Build and run:  ./build.sh && ./walltest
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

/* Must match the high-RAM map in fb.c and memmap.h. Duplicated deliberately,
 * exactly as fbbench.c duplicates them: if somebody moves the arena and does
 * not move the harness, the mmap fails loudly here instead of the test quietly
 * exercising a different address than the kernel uses. */
#define BACK_ADDR 0x08000000UL
#define BACK_SIZE (0x0A800000UL - BACK_ADDR)   /* 40 MiB, ceiling = AP stacks */
#define BLUR_ADDR 0x0C000000UL
#define BLUR_SIZE (0x0D000000UL - BLUR_ADDR)   /* 16 MiB, ceiling = nvme      */

void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
int  fb_wall_save(void);
int  fb_wall_ok(void);
int  fb_blur_cache(int x, int y, int w, int h, int radius);
void fb_blur_free_all(void);
void fb_cache_reset(void);
unsigned int fb_pxw(void);
unsigned int fb_pxh(void);
int  fb_ui_scale(void);

void idt_set_pointer_bounds(int w, int h) { (void)w; (void)h; }
void input_set_bounds(int w, int h)       { (void)w; (void)h; }

/* fb.c reports every mode and every refusal through this one sink. Captured so
 * the assertions can be about what the boot log SAYS, not only about a return
 * code - this project's documented failure mode is the silent fallback, and a
 * refusal that returns 0 without printing is exactly that. */
static char log_buf[8192];
static int  log_n;
static int  log_on;
void zl_putc_pub(char c)
{
    if (log_on && log_n < (int)sizeof log_buf - 1) log_buf[log_n++] = c;
    else if (!log_on) fputc(c, stdout);
    log_buf[log_n] = 0;
}
static void log_start(void) { log_n = 0; log_buf[0] = 0; log_on = 1; }
static void log_stop(void)  { log_on = 0; }

static int fails;
static void ok(int cond, const char *what)
{
    printf("  %s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

/* kernel.zl's dock strip, in pixels: dock_y() = px_h() - 64 * ui(), and the
 * blur covers from there to the bottom - so 64 * ui() rows, full width. */
static int dock_rows(void) { return 64 * fb_ui_scale(); }

static unsigned long kib(unsigned long px) { return px * 4u / 1024u; }

/* One mode, one boot order. Returns 1 if the wallpaper cache held. */
static int run(unsigned long vram, int w, int h, int blur_first, int verbose)
{
    fb_cache_reset();                 /* rewind the bump arena between cases */
    log_start();
    fb_setup(vram, (unsigned)w * 4, (unsigned)w, (unsigned)h, 32);
    int blur = -1;
    if (blur_first)
        blur = fb_blur_cache(0, h - dock_rows(), w, dock_rows(), 10 * fb_ui_scale());
    int saved = fb_wall_save();
    log_stop();
    if (verbose) {
        printf("      %dx%d ui %dx  wallpaper %lu KiB, dock strip %lu KiB x2\n",
               w, h, fb_ui_scale(), kib((unsigned long)w * h),
               kib((unsigned long)w * dock_rows()));
        for (char *p = log_buf, *e; *p; p = e) {
            e = strchr(p, '\n'); if (!e) break; e++;
            if (strstr(p, "wallpaper") || strstr(p, "blur refused"))
                printf("      | %.*s\n", (int)(e - p - 1), p);
        }
        if (blur_first) printf("      blur slot: %s\n", blur >= 0 ? "taken" : "refused");
    }
    fb_blur_free_all();
    return saved && fb_wall_ok();
}

int main(void)
{
    void *b = mmap((void *)BACK_ADDR, BACK_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    void *a = mmap((void *)BLUR_ADDR, BLUR_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (b == MAP_FAILED || a == MAP_FAILED) {
        printf("walltest: could not map fb.c's fixed addresses - "
               "the high-RAM map moved and this harness did not\n");
        return 1;
    }
    /* 4K of VRAM, more than any mode below needs */
    unsigned long vram = (unsigned long)mmap(NULL, 3840UL * 2160 * 4,
                                             PROT_READ | PROT_WRITE,
                                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    printf("walltest - the wallpaper cache against the arena it shares\n\n");

    printf("  arena is %lu KiB (fb.c HI_BLUR..HI_NVME)\n\n", BLUR_SIZE / 1024);

    printf("  A. NO DOCK BLUR - the shipping boot order\n");
    int a1920 = run(vram, 1920, 1200, 0, 1);
    int a2560 = run(vram, 2560, 1440, 0, 1);
    printf("\n");

    printf("  B. DOCK BLUR TAKEN FIRST - the boot order before DECISIONS #29\n");
    int b1920 = run(vram, 1920, 1200, 1, 1);
    int b2560 = run(vram, 2560, 1440, 1, 1);
    printf("\n");

    printf("  assertions\n");
    ok(a1920, "1920x1200 without the blur: wallpaper cached");
    ok(b1920, "1920x1200 WITH the blur: wallpaper cached "
              "(so QEMU could never show the problem)");
    ok(a2560, "2560x1440 without the blur: wallpaper cached "
              "<- the ThinkPad's panel");
    ok(!b2560, "2560x1440 WITH the blur: wallpaper REFUSED "
               "<- the regression this decision removed");
    ok(strstr(log_buf, "wallpaper") != NULL,
       "a refusal PRINTS rather than falling back silently");

    /* 4K is refused either way and that is EXPECTED, not a defect: 31.6 MiB of
     * wallpaper cannot fit a 16 MiB arena and no boot order changes that. It is
     * asserted so that a future arena resize has to come and look at this line
     * rather than silently changing what the desktop does at 4K. */
    int a3840 = run(vram, 3840, 2160, 0, 0);
    ok(!a3840, "3840x2160: refused even without the blur - "
               "31.6 MiB will not fit 16, by design");

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "all passed",
           fails, fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
