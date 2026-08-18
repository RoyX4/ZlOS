/* fb.c - a framebuffer text console, for UEFI.
 *
 * On a BIOS machine the VGA text buffer at 0xB8000 exists and vga.c drives
 * it. Under UEFI it does NOT: firmware hands you a linear framebuffer
 * through GOP and no text mode at all. GRUB says so out loud -
 * "no suitable video mode found" - and the screen stays black.
 *
 * So the kernel asks GRUB for a graphics mode in its multiboot header, and
 * GRUB fills in the framebuffer address, pitch and depth in the multiboot
 * info structure. This file renders glyphs into it a pixel at a time.
 *
 * console.c picks between this and vga.c at run time, so ONE image boots
 * both ways: legacy BIOS and modern UEFI.
 */

extern const unsigned char font8x16[95][16];
/* the anti-aliased twin of the same font: a coverage (alpha 0..255) per pixel,
 * generated from font8x16 by gen_aa_font.py. Blended over the background so
 * glyph edges go grey instead of staircasing. */
#include "font_prop.h"
extern const unsigned char font8x16_aa[95][16][8];
#define FONT_FIRST 32
#define FONT_LAST  126
#define GLYPH_W    8
#define GLYPH_H    16

/* The CONSOLE cell can be bigger than one glyph. At 1920x1200 an 8x16 cell
 * gives 240x75 characters of ant-sized text - technically more information,
 * actually less readable, and it is the main reason a high-resolution
 * software desktop still reads as "blocky". So on a large screen the console
 * switches to 16x32 cells and draws from the double-size coverage atlas,
 * which is real detail rather than a scaled-up 8x16. */
extern const unsigned char font16x32_aa[95][32][16];
/* the subpixel twins: three coverages per pixel, one per LCD colour stripe */
extern const unsigned char font8x16_sub[95][16][8][3];
extern const unsigned char font16x32_sub[95][32][16][3];
static int subpixel_on = 1;      /* LCD stripe rendering; off = grayscale */
void fb_set_subpixel(int on) { subpixel_on = on ? 1 : 0; }
int  fb_get_subpixel(void)   { return subpixel_on; }
static int cell_w = GLYPH_W, cell_h = GLYPH_H;
int fb_cell_w(void) { return cell_w; }
int fb_cell_h(void) { return cell_h; }

/* ---- THE UI SCALE, WHICH IS NOT THE FONT CELL -----------------------------
 * These were the same number and that was the bug behind "everything looks
 * small on a big screen".
 *
 * The desktop's layout is written in DESIGN UNITS and the original layout was
 * drawn for an 800-unit-wide screen. ui() multiplied those units, and ui() was
 * cell_w / 8 - so it was 1 below 1400 pixels and 2 at or above, and 2 forever
 * after. The console font cell has exactly two atlases, so it could not be
 * anything else. The consequence, in units of layout space available for an
 * 800-unit design:
 *
 *      1280 wide   ui 1   1280 units    1.6x too much room
 *      1920 wide   ui 2    960 units    about right
 *      2560 wide   ui 2   1280 units    1.6x too much room   <- the ThinkPad
 *      3840 wide   ui 2   1920 units    2.4x too much room
 *
 * The scale stopped growing while the screen kept growing, so the bigger the
 * panel the smaller the desktop looked on it. On the laptop's own 2560x1440
 * that is the difference between a desktop and a postage stamp in the corner
 * of one.
 *
 * They are now two different questions. The console cell still has two sizes
 * because it is drawn from a fixed atlas and a resampled console would be both
 * slow and soft. The LAYOUT scale is an integer derived from the screen, and
 * proportional text resamples to meet it - which the type scale's atlases and
 * blend_cov_scaled already make possible.
 */
static int ui_scale = 1;

int fb_ui_scale(void) { return ui_scale; }

unsigned int fb_get_px(int x, int y);   /* defined below; used by the AA text path */
void idt_set_pointer_bounds(int w, int h);   /* the mouse clamp, pushed not pulled */
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);  /* the fast row fill */
void fb_clip_none(void);                /* the scissor; defined below with put_pixel */

static unsigned char *fb_base;
static unsigned int   fb_pitch;      /* bytes per scanline, NOT pixels */
static unsigned int   fb_w, fb_h;
static unsigned int   fb_bpp;        /* bits per pixel */
static int fb_cols, fb_rows;
static int fb_col, fb_row;
static unsigned int fb_fg = 0xAAAAAA, fb_bg = 0x000000;

/* The console can be confined to a column band [tx0..tx1] - a "text box" - so
 * the shell can live inside a floating terminal window instead of owning the
 * whole width. Flowing text starts at tx0, wraps at tx1, and (crucially)
 * scrolling only moves those columns, so windows either side are never smeared.
 * Default is the full width, so nothing changes until zl sets a box. */
static int tx0 = 0, tx1 = 100000;
static int text_c1(void) { return tx1 < fb_cols - 1 ? tx1 : fb_cols - 1; }

void fb_set_text_box(int c0, int c1)
{
    tx0 = (c0 < 0) ? 0 : c0;
    tx1 = c1;
    if (tx0 > fb_cols - 1) tx0 = fb_cols - 1;
    fb_col = tx0;
}

int fb_active(void) { return fb_base != 0; }

/* Where the card actually scans out of. NOT the back buffer below: this is
 * real video memory, which is what a poke/peek proof has to write to. */
unsigned long fb_phys(void) { return (unsigned long)fb_base; }

int fb_get_cols(void) { return fb_cols; }
int fb_get_rows(void) { return fb_rows; }

/* ---- the back buffer -----------------------------------------------------
 * Video memory is write-combining: writes are fast but READS are ~30-50x
 * slower than ordinary RAM, and this renderer reads constantly - every
 * antialiased glyph pixel, every shadow pixel, every rounded corner blends
 * against what is already on screen. So all drawing goes into a plain RAM
 * buffer instead, and only the part that actually changed is copied out to
 * the card. That one change makes reads free AND turns the whole screen
 * refresh into a single tight row copy.
 *
 * The dirty box is tracked automatically by put_pixel, so nothing above this
 * file has to know the back buffer exists - callers just draw, then present.
 */
/* ---- the fixed high-RAM map ----------------------------------------------
 * The map itself now lives in memmap.h, declared once and checked by the
 * compiler, because this comment used to carry the list AND tell you not to
 * trust it. It was already wrong when it said that: i2c_hid.c had put its
 * report buffer 9 MiB into the blur arena below, and no build ever complained.
 * Read memmap.h for the regions and for why they are a header.
 *
 * What stays here is what fb.c owns: two of those regions, and the reason the
 * ceiling of each is a SUBTRACTION rather than a pixel count. A pixel count
 * silently stops being true when the panel gets bigger - BACK_MAX was
 * 1920*1200, so the ThinkPad's 2560x1440 made back_on 0 and took the back
 * buffer, subpixel text, fast read-back AND window dragging with it, without
 * printing a word. desktop-TODO 0a, T-1.
 *
 * C4 CHANGED THIS MAP, and the change is the point of C4. 128 MiB and 160 MiB
 * used to hold bg_buf and sp_buf - a whole-screen snapshot of the desktop and a
 * bitmap of the window being dragged, the "sticky note" way to move a window
 * without a GPU. The compositor repaints from damage and needs neither, so both
 * are gone and `back` moved down into the 48 MiB they occupied.
 *
 * What that bought, all of it measurable rather than argued:
 *
 *   - the back buffer now covers 3840x2160 (31.6 MiB of 48). It could not
 *     before: its own span is 16 MiB, so 4K fell back to writing straight to
 *     VRAM, where a full-screen fill costs 7.97 cyc/px against 2.93.
 *   - the 640x480 drag ceiling is gone. It is why the 1256x944 terminal could
 *     never be dragged - nearly 4x over sp_buf's limit.
 *   - the 12 px shadow smear is gone with it. fb_shadow reaches x + w + 28 at
 *     ui() == 2 and the drag erased only w + 16, so every drag step left a
 *     sliver of shadow behind. Damage-based repaint has no such arithmetic to
 *     get wrong: a window's damage IS its frame plus its shadow.
 *   - 192..208 MiB, which back has vacated, becomes the cached-blur arena.
 */
#include "memmap.h"

/* Each region's ceiling is the base of whoever comes next. `back` runs up to
 * sched.c's stacks; the blur arena runs up to nvme.c's queues. */
#define BACK_LIMIT ((unsigned int)(HI_SCHED - HI_BACK))  /* 48 MiB */
#define BLUR_LIMIT ((unsigned int)(HI_NVME  - HI_BLUR))  /* 16 MiB */

/* 3840x2160x4 must actually fit in what back was given, or the headline claim
 * of C4 is false in a way nobody would notice until a 4K panel */
_Static_assert(3840UL * 2160UL * 4UL <= (unsigned long)BACK_LIMIT,
               "the back buffer no longer covers 3840x2160");
/* and the blur arena must still be the 16 MiB its allocator advertises */
_Static_assert((unsigned long)HI_BLUR + BLUR_LIMIT <= HI_NVME,
               "the blur arena overruns nvme's queues");

static unsigned int *back = (unsigned int *)HI_BACK;
static int back_on = 0;

/* ---- SIMD, and exactly where it is allowed --------------------------------
 * cpu.c has detected SSE/SSE2/SSE3/SSSE3 since it was written and NOTHING has
 * ever used it. The reason to be careful rather than eager:
 *
 *   SSE IS ONLY ENABLED ON THE 64-BIT PATH. boot64.S sets CR4.OSFXSR (bit 9);
 *   boot.S, the 32-bit multiboot entry that verify.sh boots, does not touch
 *   CR4 at all. An SSE instruction there faults. So this cannot simply be
 *   switched on for "the kernel" - fb.c is compiled into both.
 *
 * __SSE2__ is exactly the right predicate and needs no build-system change:
 * gcc defines it for x86-64 (always, SSE2 is baseline) and leaves it undefined
 * for -m32 without -msse2, which is what build.sh uses. So the 64-bit kernel,
 * the UEFI application - the path the ThinkPad actually takes - and
 * hosttest/fbbench all get the vector path, and the 32-bit kernel keeps the
 * scalar one. The two must produce IDENTICAL pixels, which the FNV scene hash
 * checks on every run.
 *
 * WHAT IS NOT VECTORISED, AND WHY: blend_rgb and blend_sub. They are three
 * table lookups into srgb_to_lin per pixel, and a gather is the one thing SSE2
 * cannot do. Vectorising around it would mean giving up the gamma-correct
 * linear-light blend, which is the single best thing about this renderer. Not
 * worth it, and saying so here is cheaper than someone rediscovering it.
 */
/* -DFB_NO_SIMD forces the scalar path even where SSE2 exists. That is not a
 * debug switch, it is the A/B: DECISIONS.md #25 records an optimisation that
 * was argued from an instruction count, shipped, and turned out 25% SLOWER
 * when finally measured. Keeping both paths buildable is what makes the
 * comparison a command rather than an opinion. */
#if defined(__SSE2__) && !defined(FB_NO_SIMD)
#include <emmintrin.h>
#define FB_SIMD 1
#else
#define FB_SIMD 0
#endif

/* Fill n 32-bit pixels with one colour. Sixteen bytes a go once aligned.
 * GCC will not do this itself at -O2: the "very-cheap" cost model refuses any
 * loop whose trip count is a runtime value, because it would need a scalar
 * epilogue - verified by objdump, which shows no xmm in fb_fill_px at all. */
static inline void fill32(unsigned int *d, unsigned int rgb, int n)
{
#if FB_SIMD
    while (n > 0 && ((unsigned long)d & 15)) { *d++ = rgb; n--; }
    __m128i v = _mm_set1_epi32((int)rgb);
    while (n >= 16) {
        _mm_store_si128((__m128i *)d,      v);
        _mm_store_si128((__m128i *)d + 1,  v);
        _mm_store_si128((__m128i *)d + 2,  v);
        _mm_store_si128((__m128i *)d + 3,  v);
        d += 16; n -= 16;
    }
    while (n >= 4) { _mm_store_si128((__m128i *)d, v); d += 4; n -= 4; }
#endif
    while (n-- > 0) *d++ = rgb;
}

/* Copy n 32-bit pixels. This is the present blit, which crosses into
 * write-combining VRAM on real hardware - wide stores are what WC is for. */
static inline void copy32(unsigned int *d, const unsigned int *s, int n)
{
#if FB_SIMD
    while (n >= 4) {
        _mm_storeu_si128((__m128i *)d, _mm_loadu_si128((const __m128i *)s));
        d += 4; s += 4; n -= 4;
    }
#endif
    while (n-- > 0) *d++ = *s++;
}

/* ---- the damage list -----------------------------------------------------
 * This was ONE rectangle that every touched pixel grew. A clock ticking in one
 * corner and a monitor updating in the other unioned to the whole screen, every
 * second, so the present blit copied 2.3 million pixels to move a few hundred.
 * GNOME repaints ~2% of the screen per frame; zlOS repainted 100%.
 *
 * Eight rectangles, merged on contact. When it fills, everything collapses into
 * one - which IS the old single box, so the worst case is "as slow as it was",
 * never "wrong". That property is why 8 is enough and why there is no dynamic
 * growth to get wrong.
 *
 * THE PIXEL ACCUMULATOR IS NOT AN OPTIMISATION, IT IS THE DESIGN. put_pixel is
 * the hottest path in the renderer - every shadow pixel, every antialiased
 * glyph edge, every rounded corner - and making it search an eight-entry list
 * per pixel would tax the whole file to speed up the blit. So put_pixel keeps
 * growing a single box exactly as before, at the same four compares, and that
 * box is flushed into the list whenever a rect-shaped primitive reports its own
 * damage, or at present time. Per-pixel primitives are spatially coherent
 * anyway, so the box is the right shape for them.
 */
#define DMG_MAX 8
struct dmg_rect { int x0, y0, x1, y1; };
static struct dmg_rect dmg[DMG_MAX];
static int ndmg;

static int px0, py0, px1, py1, pdirty;   /* the put_pixel accumulator */

static void mark(int x, int y)
{
    if (!pdirty) { px0 = x; py0 = y; px1 = x + 1; py1 = y + 1; pdirty = 1; return; }
    if (x < px0) px0 = x;
    if (y < py0) py0 = y;
    if (x + 1 > px1) px1 = x + 1;
    if (y + 1 > py1) py1 = y + 1;
}

/* Touching counts as overlapping. Two rectangles sharing an edge are cheaper
 * to blit as one than as two, and merging them cannot cover a pixel that was
 * not already going to be covered by one of them. */
static int dmg_touches(const struct dmg_rect *a, int x0, int y0, int x1, int y1)
{
    return !(x0 > a->x1 || x1 < a->x0 || y0 > a->y1 || y1 < a->y0);
}

static void dmg_add(int x0, int y0, int x1, int y1)
{
    if (x0 >= x1 || y0 >= y1) return;

    /* Absorb everything it touches, and RESTART after each absorption: one
     * union makes the rectangle bigger, which can bring it into contact with
     * something it did not touch a moment ago. Bounded by DMG_MAX either way. */
    for (int i = 0; i < ndmg; ) {
        if (dmg_touches(&dmg[i], x0, y0, x1, y1)) {
            if (dmg[i].x0 < x0) x0 = dmg[i].x0;
            if (dmg[i].y0 < y0) y0 = dmg[i].y0;
            if (dmg[i].x1 > x1) x1 = dmg[i].x1;
            if (dmg[i].y1 > y1) y1 = dmg[i].y1;
            dmg[i] = dmg[--ndmg];
            i = 0;
            continue;
        }
        i++;
    }

    if (ndmg >= DMG_MAX) {              /* full: degrade to the old single box */
        for (int i = 0; i < ndmg; i++) {
            if (dmg[i].x0 < x0) x0 = dmg[i].x0;
            if (dmg[i].y0 < y0) y0 = dmg[i].y0;
            if (dmg[i].x1 > x1) x1 = dmg[i].x1;
            if (dmg[i].y1 > y1) y1 = dmg[i].y1;
        }
        ndmg = 0;
    }
    dmg[ndmg].x0 = x0; dmg[ndmg].y0 = y0;
    dmg[ndmg].x1 = x1; dmg[ndmg].y1 = y1;
    ndmg++;
}

static void dmg_flush_pixels(void)
{
    if (!pdirty) return;
    pdirty = 0;
    dmg_add(px0, py0, px1, py1);
}

/* A primitive that knows its own rectangle says so here, instead of marking
 * every pixel it wrote. That is what keeps two far-apart updates apart. */
void fb_damage(int x, int y, int w, int h)
{
    dmg_flush_pixels();
    dmg_add(x, y, x + w, y + h);
}

/* what fb_present would blit right now: how many rectangles, and how many
 * pixels in total. The whole claim of this change is "the presented area is
 * measurably smaller", and that needs a number - see hosttest/fbbench.c. */
int fb_damage_count(void) { dmg_flush_pixels(); return ndmg; }

unsigned int fb_damage_area(void)
{
    dmg_flush_pixels();
    unsigned int a = 0;
    for (int i = 0; i < ndmg; i++)
        a += (unsigned)(dmg[i].x1 - dmg[i].x0) * (unsigned)(dmg[i].y1 - dmg[i].y0);
    return a;
}

/* copy every damaged rectangle out to the card, then forget them */
void fb_present(void)
{
    if (!back_on) return;
    dmg_flush_pixels();
    if (!ndmg) return;
    int bpx = (int)(fb_bpp / 8);
    for (int i = 0; i < ndmg; i++) {
        int x0 = dmg[i].x0, y0 = dmg[i].y0, x1 = dmg[i].x1, y1 = dmg[i].y1;
        if (x0 < 0) x0 = 0;
        if (y0 < 0) y0 = 0;
        if (x1 > (int)fb_w) x1 = (int)fb_w;
        if (y1 > (int)fb_h) y1 = (int)fb_h;
        if (x0 >= x1 || y0 >= y1) continue;
        for (int y = y0; y < y1; y++) {
            unsigned int  *src = back + (unsigned long)y * fb_w + x0;
            unsigned char *dst = fb_base + (unsigned long)y * fb_pitch + (unsigned long)x0 * bpx;
            if (bpx == 4) {
                copy32((unsigned int *)dst, src, x1 - x0);
            } else {
                for (int x = x0; x < x1; x++) {
                    unsigned int c = *src++;
                    dst[0] = (unsigned char)(c & 0xFF);
                    dst[1] = (unsigned char)((c >> 8) & 0xFF);
                    dst[2] = (unsigned char)((c >> 16) & 0xFF);
                    dst += 3;
                }
            }
        }
    }
    ndmg = 0;
}

/* ---- saying so out loud --------------------------------------------------
 * The degradation above is legitimate. The SILENCE was the bug: four features
 * turned themselves off at 2560x1440 and nothing anywhere said a word, so the
 * first symptom would have been "the desktop is inexplicably a slideshow on
 * the laptop and dragging does nothing".
 *
 * This goes out through zl_putc_pub, the same character sink the rest of the
 * kernel prints through, so it lands on the serial log. On the FIRST call the
 * screen is wiped immediately afterwards by console_init's fb_clear(), so the
 * line is serial-only there - which is fine, because the serial log is what an
 * unattended gate reads. On a later runtime mode switch (the `n` command, via
 * console_init_fb) it lands on both.
 *
 * Note this cannot reach the text-mode path: fb_setup is only ever called when
 * there IS a framebuffer, and on verify.sh's `-kernel -display none` there is
 * not one, so golden.txt is untouched.
 */
void zl_putc_pub(char c);

static void fb_puts(const char *s) { while (*s) zl_putc_pub(*s++); }

static void fb_putu(unsigned int v)
{
    char b[12];
    int i = 0;
    if (!v) { zl_putc_pub('0'); return; }
    while (v) { b[i++] = (char)('0' + v % 10u); v /= 10u; }
    while (i) zl_putc_pub(b[--i]);
}

/* There used to be a second verdict on this line - "drag ON/OFF" - because
 * dragging went through its own pair of fixed buffers with their own ceiling,
 * and at 2560x1440 it switched itself off without a word. C4 deleted those
 * buffers: dragging is damage-based repaint now and has no buffer to run out
 * of, so there is nothing left to report about it. That is the difference
 * between removing a warning and removing the thing it warned about. */
static void fb_report_mode(unsigned int need)
{
    fb_puts("  fb: ");
    fb_putu(fb_w); fb_puts("x"); fb_putu(fb_h); fb_puts("x"); fb_putu(fb_bpp);
    fb_puts(" cell "); fb_putu((unsigned)cell_w); fb_puts("x"); fb_putu((unsigned)cell_h);
    fb_puts(" ui "); fb_putu((unsigned)ui_scale); fb_puts("x");
    fb_puts(", back "); fb_puts(back_on ? "ON" : "OFF");
    fb_puts("  ("); fb_putu(need >> 10); fb_puts(" KiB/mode)\n");

    if (!back_on) {
        fb_puts("      back OFF: wants "); fb_putu(need >> 10);
        fb_puts(" KiB, "); fb_putu(BACK_LIMIT >> 10);
        fb_puts(" KiB free below sched - no subpixel text, read-back hits VRAM\n");
    }
    /* Say where it ends, not just that it fits. "back ON" is a claim; an
     * address is a fact somebody can check against the map in this file. */
    if (back_on) {
        unsigned long top = (unsigned long)back + need;
        fb_puts("      back at ");   fb_putu((unsigned)((unsigned long)back >> 20));
        fb_puts(" MiB, ends at ");   fb_putu((unsigned)(top >> 20));
        fb_puts(" MiB, ceiling ");   fb_putu((unsigned)(HI_SCHED >> 20));
        fb_puts(" MiB");
        if (top > HI_SCHED) fb_puts("  *** OVERRUN - THIS WILL CORRUPT THE NEXT BUFFER ***");
        fb_puts("\n");
    }
}

/* NOTE, AND IT IS NOT THIS FILE'S BUG TO FIX: `addr` arrives TRUNCATED in the
 * EFI build, and this signature is one link in that chain rather than its
 * origin.
 *
 * buildefi.sh targets x86_64-unknown-windows, which is LLP64 - `unsigned long`
 * is FOUR bytes there and eight everywhere else. Proven, not assumed:
 * a _Static_assert(sizeof(unsigned long) == 8) fails on that target.
 *
 * The chain, all of it `unsigned long`:
 *     efi.c:250   fb_addr = (unsigned long)gop->mode->framebuffer_base;
 *                 <- the UINT64 loses its top half HERE, at the source
 *     efi.c:287   console_init_efi(fb_addr, ...)
 *     console.c   console_init_efi -> fb_setup(addr, ...)
 *     here        fb_base = (unsigned char *)addr
 * and back out through fb_phys() -> console_vram() -> the `vram` builtin.
 *
 * CLAUDE.md already names this bug class - "never put a pointer through
 * unsigned long in the EFI build" - and buildefi.sh carries four -Werror flags
 * against it. THOSE FLAGS ARE SILENT HERE, verified by compiling both cast
 * shapes with the exact build line: they catch pointer<->int, and this is a
 * UINT64 narrowed by an EXPLICIT cast, which no warning catches.
 *
 * Latent rather than active: a GOP framebuffer base is normally a PCI BAR in
 * the 32-bit MMIO window, which is why QEMU and OVMF never show it. Firmware
 * that places it above 4 GiB would give a black screen or write into whatever
 * lives at the truncated address.
 *
 * The fix is three declarations, not one, and two of the files were mid-flight
 * in another session: efi.c's fb_addr, console_init_efi, and this parameter all
 * become `unsigned long long`. Changing only this one is WORSE than leaving it
 * - console.c's declaration would then disagree with the definition about the
 * size of a register argument. Logged as T-11. */
void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp)
{
    /* Only 32- and 24-bit packed pixel modes are handled. Anything else is
     * refused rather than drawn as garbage - a wrong depth paints noise and
     * looks like a crash. */
    if (!addr || (bpp != 32 && bpp != 24)) {
        /* Refusing is right - a wrong depth paints noise and reads as a crash.
         * Refusing SILENTLY is the bug, and it is the same one 0a was about:
         * the machine ends up with no framebuffer and nothing anywhere says
         * why. On a real UEFI boot this is the difference between "the panel
         * is black" and "the panel is black because firmware handed us 16bpp". */
        fb_base = 0;
        fb_puts("  fb: REFUSED the mode - ");
        if (!addr) fb_puts("no framebuffer address\n");
        else { fb_puts("bpp "); fb_putu(bpp); fb_puts(", only 24 and 32 are handled\n"); }
        return;
    }

    fb_base  = (unsigned char *)addr;
    fb_pitch = pitch;
    fb_w     = width;
    fb_h     = height;
    fb_bpp   = bpp;
    cell_w   = (width >= 1400) ? GLYPH_W * 2 : GLYPH_W;
    cell_h   = (width >= 1400) ? GLYPH_H * 2 : GLYPH_H;
    /* Rounded, not truncated: 1200 wide is much closer to 1.5 designs than to
     * 1, and truncation would leave it at 1 with half the screen empty. Capped
     * at 4 because beyond that a 24px title becomes 96px and the layout runs
     * out of design units before it runs out of screen. */
    ui_scale = (int)((width + 400u) / 800u);
    if (ui_scale < 1) ui_scale = 1;
    if (ui_scale > 4) ui_scale = 4;
    fb_cols  = (int)(width  / cell_w);
    fb_rows  = (int)(height / cell_h);
    fb_col   = 0;
    fb_row   = 0;
    tx0      = 0;
    tx1      = fb_cols - 1;      /* full width until zl opens a text box */
    fb_clip_none();              /* the scissor follows the new geometry */

    /* Draw into RAM and blit, rather than drawing straight into the card.
     * Refused only when the mode does not fit between `back` and its
     * neighbour, in which case everything still works - just slower, straight
     * to VRAM - and the boot log SAYS SO. See the high-RAM map above. */
    unsigned int need = width * height * 4u;
    /* One span, one test. There used to be a fallback into the drag buffers'
     * arena because back's own 16 MiB could not hold 4K; C4 gave that arena to
     * back outright, so the fallback has nothing left to fall back FROM.
     *
     * The static asserts above prove the MAP is sane, and one of them proves
     * 3840x2160 fits. This is the run-time half: a mode arrives from firmware
     * and `need` is computed from it, so a compile-time check cannot cover
     * whether THIS mode's buffer stays inside the span. */
    back = (unsigned int *)HI_BACK;
    back_on = (need <= BACK_LIMIT);
    ndmg    = 0;                 /* the mode changed; old damage means nothing */
    pdirty  = 0;
    fb_report_mode(need);

    /* Tell the mouse ISR how big the screen is. It cannot ask: idt.c is built
     * -mgeneral-regs-only so that an interrupt never touches SSE, and calling
     * out of that file into one that can use XMM corrupts whatever the
     * interrupted code had in those registers. Every zl number is a double, so
     * that is the interpreter itself - it killed the 64-bit boot outright. */
    idt_set_pointer_bounds((int)width, (int)height);
}

/* ---- the scissor ---------------------------------------------------------
 * Every primitive in this file clipped to the SCREEN and nothing else, which
 * is what made a compositor impossible: there was no way to say "repaint only
 * this rectangle" and be sure nothing escaped it. That, not the absence of
 * window code, was the blocker (desktop-TODO 0b, DECISIONS.md #6).
 *
 * It is one rectangle, not a stack. A stack would need push/pop and a depth
 * limit for no gain: the repaint loop sets the scissor twice per window - once
 * to the frame so chrome cannot bleed onto a neighbour, then NARROWER to the
 * client area so an app physically cannot draw over its own title bar - and
 * both are computed, not nested.
 *
 * x1/y1 are EXCLUSIVE, and fb_clip() clamps to the screen on the way in, so
 * the scissor test below subsumes the screen test it replaces rather than
 * being an extra one.
 */
static int clip_x0, clip_y0, clip_x1, clip_y1;

/* THE SCISSOR, READABLE. It has been write-only since it was built, which was
 * fine while every customer was a fb_* primitive that folds it into its own
 * loop bounds. An APP cannot do that: term_draw walks its whole scrollback and
 * calls fb_text_aa per row, and a row outside the scissor costs a full string
 * walk and a per-pixel reject before producing nothing.
 *
 * Being able to ASK is what lets a caller skip work instead of having it
 * thrown away. The scissor stays a correctness guarantee either way - this is
 * about not doing the work twice over. */
int fb_clip_top(void)   { return clip_y0; }
int fb_clip_bot(void)   { return clip_y1; }
int fb_clip_left(void)  { return clip_x0; }
int fb_clip_right(void) { return clip_x1; }


void fb_clip(int x, int y, int w, int h)
{
    int x1 = x + w, y1 = y + h;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x1 > (int)fb_w) x1 = (int)fb_w;
    if (y1 > (int)fb_h) y1 = (int)fb_h;
    if (x1 < x) x1 = x;              /* an empty rect clips everything away */
    if (y1 < y) y1 = y;
    clip_x0 = x; clip_y0 = y; clip_x1 = x1; clip_y1 = y1;
}

void fb_clip_none(void)
{
    clip_x0 = 0; clip_y0 = 0;
    clip_x1 = (int)fb_w; clip_y1 = (int)fb_h;
}

static void put_pixel(unsigned int x, unsigned int y, unsigned int rgb)
{
    /* Signed, deliberately. Callers pass (unsigned)(px + x) where px + x can
     * be negative, and that arrives here as a huge unsigned - which the old
     * `x >= fb_w` test rejected for the right reason by accident. Casting back
     * to int makes any value at or above 2^31 negative, and everything below
     * it is caught by the upper bound, so both halves still reject it. */
    if ((int)x < clip_x0 || (int)x >= clip_x1 ||
        (int)y < clip_y0 || (int)y >= clip_y1) return;
    if (back_on) { back[(unsigned long)y * fb_w + x] = rgb; mark((int)x, (int)y); return; }
    unsigned char *p = fb_base + (unsigned long)y * fb_pitch + (unsigned long)x * (fb_bpp / 8);
    p[0] = (unsigned char)(rgb & 0xFF);           /* B */
    p[1] = (unsigned char)((rgb >> 8) & 0xFF);    /* G */
    p[2] = (unsigned char)((rgb >> 16) & 0xFF);   /* R */
}

/* Map a VGA attribute byte to RGB, so the same zl code colours both
 * consoles - the kernel should not have to know which one it is talking to. */
static const unsigned int vga_rgb[16] = {
    0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
    0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
    0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
    0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
};

void fb_setcolor(unsigned char attr)
{
    fb_fg = vga_rgb[attr & 0x0F];
    fb_bg = vga_rgb[(attr >> 4) & 0x0F];
}

static void fill_cell(int cx, int cy, unsigned int rgb)
{
    fb_fill_px(cx * cell_w, cy * cell_h, cell_w, cell_h, rgb);
}

/* (div255() lived here - a shift-add replacement for /255, defined and NEVER
 * called. GCC already strength-reduces the /255 in the blend path, verified by
 * objdump: there is not a single div instruction in it. Deleted rather than
 * left as a trap for the next person optimising this file. desktop-TODO 0i.) */

/* ---- gamma-correct blending ---------------------------------------------
 * A screen value of 128 is NOT half the light of 255 - the display applies a
 * curve of roughly x^2.2. So averaging two sRGB numbers does not average the
 * light, and antialiased text pays for it: light glyphs on a dark ground come
 * out too THIN and dark glyphs on a light ground come out too FAT, because the
 * partly-covered edge pixels land at the wrong brightness. Every real text
 * renderer fixes this by blending in linear light.
 *
 * Doing that honestly means a pow() per pixel, which a kernel with no floating
 * point cannot afford - so it is a pair of lookup tables, exactly how it ships
 * in practice: one to expand sRGB to linear at higher precision, one to
 * squeeze it back. The tables are built once at boot from integer maths only.
 */
#define LIN_MAX 4095                       /* 12-bit linear working space */
static unsigned short srgb_to_lin[256];
static unsigned char  lin_to_srgb[LIN_MAX + 1];
static int gamma_ready = 0;

/* integer x^(12/5) and its inverse, by binary search on a monotone curve -
 * no powf, no doubles, and only ever run once */
/* v^2.2 scaled to 0..LIN_MAX. Strictly 32-bit: a 64-bit divide would pull in
 * libgcc's __udivdi3, and this kernel links no libgcc at all. v^2.2 is
 * approximated by mixing v^2 and v^3 four-to-one, which is monotone and lands
 * within a code value of the real curve across the whole range. */
static unsigned int ipow22(unsigned int v255)
{
    unsigned int v2 = v255 * v255;              /* 0..65025            */
    unsigned int v3 = (v2 * v255) / 255u;       /* 0..65025, no overflow */
    unsigned int m  = (v2 * 4u + v3) / 5u;      /* ~gamma 2.2          */
    return (m * (unsigned int)LIN_MAX) / 65025u;
}

static void gamma_init(void)
{
    for (int i = 0; i < 256; i++) srgb_to_lin[i] = (unsigned short)ipow22((unsigned)i);
    /* invert by walking the monotone table once */
    int s = 0;
    for (int l = 0; l <= LIN_MAX; l++) {
        while (s < 255 && srgb_to_lin[s + 1] <= l) s++;
        lin_to_srgb[l] = (unsigned char)s;
    }
    gamma_ready = 1;
}

/* Per-CHANNEL blend: each colour stripe gets its own coverage. This is what
 * makes subpixel text sharper than grayscale - the red, green and blue
 * stripes of one pixel are lit independently, so a stem edge can land a third
 * of a pixel in rather than snapping to the whole one. */
static unsigned int blend_sub(unsigned int bg, unsigned int fg, int ar, int ag, int ab)
{
    if (!gamma_ready) gamma_init();
    int lr = (srgb_to_lin[(fg >> 16) & 0xFF] * ar + srgb_to_lin[(bg >> 16) & 0xFF] * (255 - ar)) / 255;
    int lg = (srgb_to_lin[(fg >>  8) & 0xFF] * ag + srgb_to_lin[(bg >>  8) & 0xFF] * (255 - ag)) / 255;
    int lb = (srgb_to_lin[ fg        & 0xFF] * ab + srgb_to_lin[ bg        & 0xFF] * (255 - ab)) / 255;
    if (lr > LIN_MAX) lr = LIN_MAX;
    if (lg > LIN_MAX) lg = LIN_MAX;
    if (lb > LIN_MAX) lb = LIN_MAX;
    return ((unsigned)lin_to_srgb[lr] << 16)
         | ((unsigned)lin_to_srgb[lg] << 8)
         |  (unsigned)lin_to_srgb[lb];
}

/* Blend fg over bg by coverage a (0..255) IN LINEAR LIGHT, then convert the
 * result back to what the screen expects. This is the single change that
 * stops antialiased text looking anaemic. */
static unsigned int blend_rgb(unsigned int bg, unsigned int fg, int a)
{
    if (!gamma_ready) gamma_init();
    int ia = 255 - a;
    int lr = (srgb_to_lin[(fg >> 16) & 0xFF] * a + srgb_to_lin[(bg >> 16) & 0xFF] * ia) / 255;
    int lg = (srgb_to_lin[(fg >>  8) & 0xFF] * a + srgb_to_lin[(bg >>  8) & 0xFF] * ia) / 255;
    int lb = (srgb_to_lin[ fg        & 0xFF] * a + srgb_to_lin[ bg        & 0xFF] * ia) / 255;
    if (lr > LIN_MAX) lr = LIN_MAX;
    if (lg > LIN_MAX) lg = LIN_MAX;
    if (lb > LIN_MAX) lb = LIN_MAX;
    return ((unsigned)lin_to_srgb[lr] << 16)
         | ((unsigned)lin_to_srgb[lg] << 8)
         |  (unsigned)lin_to_srgb[lb];
}

/* ---- coverage blitting ---------------------------------------------------
 * A coverage bitmap is one byte of alpha per pixel: the icons and every font
 * atlas in this kernel are exactly that, and so is a resampled version of one.
 * These three are the whole vocabulary for putting one on the screen, and
 * everything that used to open-code the loop now goes through them.
 */

/* one coverage value, blended over whatever is already there */
static void blend_px(int x, int y, unsigned int rgb, int a)
{
    if (a <= 0) return;
    if (a >= 255) { put_pixel((unsigned)x, (unsigned)y, rgb); return; }
    put_pixel((unsigned)x, (unsigned)y, blend_rgb(fb_get_px(x, y), rgb, a));
}

/* a coverage bitmap, one destination pixel per source pixel. `stride` is the
 * source's row length, which is not always the width being drawn: a
 * proportional glyph sits in a cell far wider than its own ink, and blitting
 * the whole cell is up to 7x the pixels for no visible difference. */
static void blend_cov_s(int px, int py, const unsigned char *src,
                        int sw, int sh, int stride, unsigned int fg)
{
    for (int y = 0; y < sh; y++)
        for (int x = 0; x < sw; x++)
            blend_px(px + x, py + y, fg, src[(unsigned long)y * stride + x]);
}

static void blend_cov(int px, int py, const unsigned char *src,
                      int sw, int sh, unsigned int fg)
{
    blend_cov_s(px, py, src, sw, sh, sw, fg);
}

/* the same, resampled BILINEARLY into a dw x dh box.
 *
 * The source coordinate of destination pixel i is (i + 0.5) * sw / dw - 0.5,
 * held in 16.16 fixed point. Those half-pixel terms are not decoration: drop
 * them and the resampled image drifts half a destination pixel up and left,
 * which at icon sizes is visible as a wonky icon.
 *
 * This exists because the two places that used to scale a coverage bitmap
 * both did it by COPYING pixels - fb_icon24 at 2x and fb_glyph_scaled for the
 * logo - and copying is what throws the anti-aliasing away. */
/* The strided form. A font atlas row is `stride` wide while the ink that has
 * to be resampled is `sw` - passing sw as the stride would step into the next
 * glyph's row a fraction at a time, which reads as text smeared to the right.
 * blend_cov_scaled is this with stride == sw. */
static void blend_cov_scaled_s(int px, int py, const unsigned char *src,
                               int sw, int sh, int stride,
                               int dw, int dh, unsigned int fg)
{
    if (dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0) return;
    if (dw == sw && dh == sh) { blend_cov_s(px, py, src, sw, sh, stride, fg); return; }
    int lastx = (sw - 1) << 16, lasty = (sh - 1) << 16;
    for (int y = 0; y < dh; y++) {
        int syq = (2 * y + 1) * sh * 32768 / dh - 32768;
        if (syq < 0) syq = 0;
        if (syq > lasty) syq = lasty;
        int y0 = syq >> 16, fy = syq & 0xFFFF;
        int y1 = (y0 + 1 < sh) ? y0 + 1 : y0;
        const unsigned char *r0 = src + (unsigned long)y0 * stride;
        const unsigned char *r1 = src + (unsigned long)y1 * stride;
        for (int x = 0; x < dw; x++) {
            int sxq = (2 * x + 1) * sw * 32768 / dw - 32768;
            if (sxq < 0) sxq = 0;
            if (sxq > lastx) sxq = lastx;
            int x0 = sxq >> 16, fx = sxq & 0xFFFF;
            int x1 = (x0 + 1 < sw) ? x0 + 1 : x0;
            int top = (r0[x0] * (65536 - fx) + r0[x1] * fx) >> 16;
            int bot = (r1[x0] * (65536 - fx) + r1[x1] * fx) >> 16;
            blend_px(px + x, py + y, fg, (top * (65536 - fy) + bot * fy) >> 16);
        }
    }
}

static void blend_cov_scaled(int px, int py, const unsigned char *src,
                             int sw, int sh, int dw, int dh, unsigned int fg)
{
    blend_cov_scaled_s(px, py, src, sw, sh, sw, dw, dh, fg);
}

/* a glyph in a text cell, anti-aliased over a SOLID cell background. The core
 * of the stroke is opaque (coverage 255 = fg); only the edges blend, so text
 * stays crisp but the diagonal staircase is gone. This is the whole console's
 * draw path, so every line the kernel prints is now anti-aliased. */
static void draw_glyph(int cx, int cy, char c, unsigned int fg, unsigned int bg)
{
    if (c < FONT_FIRST || c > FONT_LAST) c = '?';
    int ox = cx * cell_w, oy = cy * cell_h;
    int cw = cell_w, ch = cell_h;
    /* one atlas or the other - same coverage semantics, different cell */
    const unsigned char *cov = (cw == GLYPH_W)
        ? &font8x16_aa[(int)c - FONT_FIRST][0][0]
        : &font16x32_aa[(int)c - FONT_FIRST][0][0];

    /* subpixel path: three alphas per pixel instead of one */
    if (subpixel_on && back_on && ox >= 0 && oy >= 0 &&
        ox + cw <= (int)fb_w && oy + ch <= (int)fb_h) {
        const unsigned char *sub = (cw == GLYPH_W)
            ? &font8x16_sub[(int)c - FONT_FIRST][0][0][0]
            : &font16x32_sub[(int)c - FONT_FIRST][0][0][0];
        /* clipped BOUNDS, not a clipped test per pixel: this path writes the
         * back buffer directly and never sees put_pixel, so the scissor has to
         * be folded into the loop range or the glyph escapes it. */
        int gx0 = ox > clip_x0 ? ox : clip_x0, gx1 = ox + cw < clip_x1 ? ox + cw : clip_x1;
        int gy0 = oy > clip_y0 ? oy : clip_y0, gy1 = oy + ch < clip_y1 ? oy + ch : clip_y1;
        if (gx0 >= gx1 || gy0 >= gy1) return;
        for (int y = gy0; y < gy1; y++) {
            unsigned int *row = back + (unsigned long)y * fb_w;
            const unsigned char *sr = sub + (unsigned long)(y - oy) * cw * 3;
            for (int x = gx0; x < gx1; x++) {
                int i = (x - ox) * 3;
                int ar = sr[i], ag = sr[i + 1], ab = sr[i + 2];
                if (!(ar | ag | ab)) { row[x] = bg; continue; }
                row[x] = blend_sub(bg, fg, ar, ag, ab);
            }
        }
        fb_damage(gx0, gy0, gx1 - gx0, gy1 - gy0);
        return;
    }

    if (back_on && ox >= 0 && oy >= 0 &&
        ox + cw <= (int)fb_w && oy + ch <= (int)fb_h) {
        int gx0 = ox > clip_x0 ? ox : clip_x0, gx1 = ox + cw < clip_x1 ? ox + cw : clip_x1;
        int gy0 = oy > clip_y0 ? oy : clip_y0, gy1 = oy + ch < clip_y1 ? oy + ch : clip_y1;
        if (gx0 >= gx1 || gy0 >= gy1) return;
        for (int y = gy0; y < gy1; y++) {
            unsigned int *row = back + (unsigned long)y * fb_w;
            const unsigned char *cr = cov + (unsigned long)(y - oy) * cw;
            for (int x = gx0; x < gx1; x++) {
                int a = cr[x - ox];
                row[x] = (a <= 0) ? bg : (a >= 255) ? fg : blend_rgb(bg, fg, a);
            }
        }
        fb_damage(gx0, gy0, gx1 - gx0, gy1 - gy0);
        return;
    }
    for (int y = 0; y < ch; y++)
        for (int x = 0; x < cw; x++) {
            int a = cov[(unsigned long)y * cw + x];
            unsigned int col = (a <= 0) ? bg : (a >= 255) ? fg : blend_rgb(bg, fg, a);
            put_pixel((unsigned)(ox + x), (unsigned)(oy + y), col);
        }
}

/* anti-aliased text at a pixel position, blended over whatever is ALREADY on
 * the framebuffer (read back per pixel). This is for labels over a gradient or
 * the wallpaper - window titles, the header subtitle - where the background is
 * not a flat cell colour. Transparent where coverage is zero. */
void fb_glyph_aa(int px, int py, char c, unsigned int fg)
{
    if (c < FONT_FIRST || c > FONT_LAST) c = '?';
    /* follow the console cell: on a big screen this is the 16x32 atlas, so
     * window labels and the dock scale with everything else instead of
     * staying 8px tall on a 1920-wide desktop */
    int gw = cell_w, gh = cell_h;
    const unsigned char *cov = (gw == GLYPH_W)
        ? &font8x16_aa[(int)c - FONT_FIRST][0][0]
        : &font16x32_aa[(int)c - FONT_FIRST][0][0];
    for (int y = 0; y < gh; y++)
        for (int x = 0; x < gw; x++) {
            int a = cov[(unsigned long)y * gw + x];
            if (a <= 0) continue;
            if (a >= 255) { put_pixel((unsigned)(px + x), (unsigned)(py + y), fg); continue; }
            unsigned int bg = fb_get_px(px + x, py + y);
            put_pixel((unsigned)(px + x), (unsigned)(py + y), blend_rgb(bg, fg, a));
        }
}

void fb_text_aa(int px, int py, const char *s, unsigned int fg)
{
    while (*s) { fb_glyph_aa(px, py, *s++, fg); px += cell_w; }
}

/* double-size (16x32) anti-aliased text - titles and headers. Same read-back
 * blend as fb_text_aa, but from the 16x32 coverage font, so a heading is both
 * bigger and smooth. This is what reads as "modern" instead of "1990". */
extern const unsigned char font16x32_aa[95][32][16];
#define GLYPH2_W 16
#define GLYPH2_H 32

void fb_glyph_aa2x(int px, int py, char c, unsigned int fg)
{
    if (c < FONT_FIRST || c > FONT_LAST) c = '?';
    const unsigned char (*g)[GLYPH2_W] = font16x32_aa[(int)c - FONT_FIRST];
    for (int y = 0; y < GLYPH2_H; y++)
        for (int x = 0; x < GLYPH2_W; x++) {
            int a = g[y][x];
            if (a <= 0) continue;
            if (a >= 255) { put_pixel((unsigned)(px + x), (unsigned)(py + y), fg); continue; }
            unsigned int bg = fb_get_px(px + x, py + y);
            put_pixel((unsigned)(px + x), (unsigned)(py + y), blend_rgb(bg, fg, a));
        }
}

void fb_text_aa2x(int px, int py, const char *s, unsigned int fg)
{
    while (*s) { fb_glyph_aa2x(px, py, *s++, fg); px += GLYPH2_W; }
}

/* (the 24x48 "huge" font was removed - unused, and its 109 KiB pushed the raw
 * kernel past the 640 KiB low-memory limit our own bootloader loads into) */

void fb_clear(void)
{
    for (unsigned int y = 0; y < fb_h; y++)
        for (unsigned int x = 0; x < fb_w; x++)
            put_pixel(x, y, fb_bg);
    fb_col = tx0;
    fb_row = 0;
}

void fb_bar(int row, unsigned char attr)
{
    if (row < 0 || row >= fb_rows) return;
    unsigned int bg = vga_rgb[(attr >> 4) & 0x0F];
    for (int c = 0; c < fb_cols; c++) fill_cell(c, row, bg);
}

void fb_at(int row, int col, const char *s, unsigned char attr)
{
    if (row < 0 || row >= fb_rows) return;
    unsigned int fg = vga_rgb[attr & 0x0F], bg = vga_rgb[(attr >> 4) & 0x0F];
    while (*s && col < fb_cols) { draw_glyph(col, row, *s++, fg, bg); col++; }
}

/* ---- graphics primitives, framebuffer only ------------------------------
 * The UEFI framebuffer is raw RGB pixels, so the console can draw more than
 * text: filled bands, panel borders, and text scaled up for a real logo.
 * None of this exists on the VGA text path - a text grid has no pixels - so
 * console.c makes each a no-op there and the boot still reads fine. */

unsigned int fb_pxw(void) { return fb_w; }
unsigned int fb_pxh(void) { return fb_h; }

/* Fill a pixel rectangle (clipped to the screen). Clipped ONCE up front and
 * then written a row at a time straight into the back buffer - the per-pixel
 * bounds test and address multiply that put_pixel does are the single most
 * repeated cost in the whole renderer, and this is the path every panel,
 * gradient row, text cell and window frame goes through. */
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb)
{
    if (w <= 0 || h <= 0) return;
    int x0 = x, y0 = y, x1 = x + w, y1 = y + h;
    /* against the SCISSOR, not the screen. fb_clip() already clamped the
     * scissor to the screen, so this is the same test plus the window. */
    if (x0 < clip_x0) x0 = clip_x0;
    if (y0 < clip_y0) y0 = clip_y0;
    if (x1 > clip_x1) x1 = clip_x1;
    if (y1 > clip_y1) y1 = clip_y1;
    if (x0 >= x1 || y0 >= y1) return;

    if (back_on) {
        for (int yy = y0; yy < y1; yy++)
            fill32(back + (unsigned long)yy * fb_w + x0, rgb, x1 - x0);
        fb_damage(x0, y0, x1 - x0, y1 - y0);
        return;
    }
    for (int yy = y0; yy < y1; yy++)
        for (int xx = x0; xx < x1; xx++)
            put_pixel((unsigned)xx, (unsigned)yy, rgb);
}

/* ---- translucency ---------------------------------------------------------
 * The v10 prototype uses rgba() 107 times: every panel, every hover state,
 * every overlay is a colour laid over what is behind it rather than instead of
 * it. This is the primitive that makes those expressible, and it is small
 * because the hard part was already here - blend_rgb() has done gamma-correct
 * linear-light compositing for every antialiased glyph and shadow pixel since
 * the text renderer was written. A translucent fill is that, over a rectangle.
 *
 * IT IS ALSO THE PREREQUISITE FOR HALF THE ANIMATION KEYFRAMES. zpop, ztoast
 * and zov are opacity fades, and a fade needs the thing composited at
 * fractional opacity against whatever is behind it. wm.c's open animation is a
 * SCALE rather than a fade precisely because this did not exist.
 *
 * Cost, measured rather than assumed: a translucent fill reads back every
 * pixel and blends it, so it is nearer the shadow's cost than the fill's - see
 * fbbench. Use fb_fill_px when the alpha is 255; this does that for you, but
 * knowing why matters when a design asks for 254.
 */
void fb_fill_blend(int x, int y, int w, int h, unsigned int rgb, int a)
{
    if (w <= 0 || h <= 0 || a <= 0) return;
    if (a >= 255) { fb_fill_px(x, y, w, h, rgb); return; }

    int x0 = x, y0 = y, x1 = x + w, y1 = y + h;
    /* against the SCISSOR, folded into the loop bounds rather than tested per
     * pixel - the same rule as every other primitive in this file, and the
     * reason a clipped draw costs no more per pixel than an unclipped one */
    if (x0 < clip_x0) x0 = clip_x0;
    if (y0 < clip_y0) y0 = clip_y0;
    if (x1 > clip_x1) x1 = clip_x1;
    if (y1 > clip_y1) y1 = clip_y1;
    if (x0 >= x1 || y0 >= y1) return;

    if (back_on) {
        /* straight at the back buffer: the read-back IS the destination, so
         * there is no reason to go through fb_get_px/put_pixel and pay their
         * bounds test and address multiply twice per pixel */
        for (int yy = y0; yy < y1; yy++) {
            unsigned int *row = back + (unsigned long)yy * fb_w;
            for (int xx = x0; xx < x1; xx++)
                row[xx] = blend_rgb(row[xx], rgb, a);
        }
        fb_damage(x0, y0, x1 - x0, y1 - y0);
        return;
    }
    /* no back buffer: every read crosses PCIe at 30-50x the cost. Correct,
     * just slow, and the boot log has already said the back buffer is off. */
    for (int yy = y0; yy < y1; yy++)
        for (int xx = x0; xx < x1; xx++)
            put_pixel((unsigned)xx, (unsigned)yy,
                      blend_rgb(fb_get_px(xx, yy), rgb, a));
}

/* a vertical gradient band - top colour fading to bottom colour. A flat bar
 * reads as "text mode"; a gradient reads as "a real UI". */
/* A gradient, DITHERED. Truncating the interpolation to whole bytes leaves
 * horizontal stripes wherever the colour holds for several rows - the classic
 * banding that reads as "256 colours, 1995", and it sits right behind the text
 * where the eye is already looking. So the interpolation is kept at 8 extra
 * bits of precision and an ordered 4x4 threshold decides whether each pixel
 * rounds up: neighbouring pixels then straddle the boundary instead of the
 * whole row stepping at once, and the band edge dissolves into noise. */
static const unsigned char dither4[4][4] = {
    {  0, 136,  34, 170 },
    { 204,  68, 238, 102 },
    {  51, 187,  17, 153 },
    { 255, 119, 221,  85 }
};

void fb_gradient(int x, int y, int w, int h, unsigned int top, unsigned int bot)
{
    if (h <= 0) return;
    /* the union of every row actually written, reported once at the end */
    int gx0 = 1 << 30, gy0 = 1 << 30, gx1 = -(1 << 30), gy1 = -(1 << 30);
    int tr = (top >> 16) & 0xFF, tg = (top >> 8) & 0xFF, tb = top & 0xFF;
    int br = (bot >> 16) & 0xFF, bg = (bot >> 8) & 0xFF, bb = bot & 0xFF;
    for (int i = 0; i < h; i++) {
        /* .8 fixed point: the whole part is the colour, the fraction is how
         * far this row sits between two representable values */
        int r8 = (tr << 8) + ((br - tr) * i * 256) / h;
        int g8 = (tg << 8) + ((bg - tg) * i * 256) / h;
        int b8 = (tb << 8) + ((bb - tb) * i * 256) / h;
        const unsigned char *drow = dither4[i & 3];
        int yy = y + i;
        /* the SCISSOR, not the screen - fb_clip() clamped it to the screen
         * already. The back_on branch below writes rows straight into the back
         * buffer without going near put_pixel, so this is the only thing
         * keeping the wallpaper inside a damage rectangle. */
        if (yy < clip_y0 || yy >= clip_y1) continue;
        int j0 = 0, j1 = w;
        if (x + j0 < clip_x0) j0 = clip_x0 - x;
        if (x + j1 > clip_x1) j1 = clip_x1 - x;
        if (j0 >= j1) continue;

        /* The dither pattern repeats every 4 pixels across, and the colour is
         * constant along a row - so a whole 1920-pixel row only has FOUR
         * distinct values. Compute those once and then the inner loop is a
         * masked table read and a store, instead of six compares and a pack
         * per pixel. The wallpaper is a full screen of this every redraw, so
         * this is the hottest loop in the renderer. */
        unsigned int quad[4];
        for (int k = 0; k < 4; k++) {
            int t = drow[k];
            int r = (r8 & 0xFF) > t ? (r8 >> 8) + 1 : (r8 >> 8);
            int g = (g8 & 0xFF) > t ? (g8 >> 8) + 1 : (g8 >> 8);
            int b = (b8 & 0xFF) > t ? (b8 >> 8) + 1 : (b8 >> 8);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            quad[k] = ((unsigned)r << 16) | ((unsigned)g << 8) | (unsigned)b;
        }
        if (back_on) {
            unsigned int *row = back + (unsigned long)yy * fb_w + x + j0;
            for (int j = j0; j < j1; j++) *row++ = quad[j & 3];
            /* accumulate, do not report per row: a full-screen wallpaper is
             * 1200 rows, and 1200 list insertions to describe one rectangle
             * would cost more than the blit they are meant to shrink */
            if (x + j0 < gx0) gx0 = x + j0;
            if (x + j1 > gx1) gx1 = x + j1;
            if (yy < gy0) gy0 = yy;
            if (yy + 1 > gy1) gy1 = yy + 1;
        } else {
            for (int j = j0; j < j1; j++)
                put_pixel((unsigned)(x + j), (unsigned)yy, quad[j & 3]);
        }
    }
    if (gx0 < gx1 && gy0 < gy1) fb_damage(gx0, gy0, gx1 - gx0, gy1 - gy0);
}

/* one glyph, scaled up by an integer factor, drawn at a pixel position.
 *
 * This read the ONE-BIT font8x16 and drew each set bit as a solid scale x
 * scale square. It was the blockiest path in the codebase and it drew the
 * largest text on screen - the zlOS logo (desktop-look.md, bug 3). text_big
 * already did the right thing; logo was the odd one out.
 *
 * Now it resamples the 16x32 COVERAGE atlas - the same real-TrueType,
 * FreeType-hinted glyphs the console draws - into the 8*scale x 16*scale box
 * this function has always occupied. At scale 2 that is 1:1 with the atlas and
 * costs nothing extra; above it, an interpolated real glyph rather than
 * staircase squares. Transparent where coverage is zero, exactly as before. */
void fb_glyph_scaled(int px, int py, char c, int scale, unsigned int fg)
{
    if (c < FONT_FIRST || c > FONT_LAST) c = '?';
    if (scale < 1) scale = 1;
    blend_cov_scaled(px, py, &font16x32_aa[(int)c - FONT_FIRST][0][0],
                     GLYPH2_W, GLYPH2_H,
                     GLYPH_W * scale, GLYPH_H * scale, fg);
}

/* a whole string, scaled - this is how the zlOS logo is drawn big */
void fb_text_scaled(int px, int py, const char *s, int scale, unsigned int fg)
{
    while (*s) { fb_glyph_scaled(px, py, *s++, scale, fg); px += GLYPH_W * scale; }
}

/* map a VGA attribute's foreground to RGB, so zl keeps using colour indices */
unsigned int fb_attr_rgb(unsigned char attr) { return vga_rgb[attr & 0x0F]; }

/* ---- read-back: the primitive the whole desktop depends on ---------------
 * Reading a pixel back out of the framebuffer is what makes soft shadows,
 * anti-aliased text over the wallpaper, and a non-smearing cursor possible.
 * It MUST mirror put_pixel's byte order exactly (B,G,R at p[0..2]) or every
 * shadow and blended glyph comes back with red and blue swapped. */
unsigned int fb_get_px(int x, int y)
{
    if ((unsigned)x >= fb_w || (unsigned)y >= fb_h) return 0;
    /* out of RAM, not out of the card - this is the read that used to cost
     * 30-50x more than it should, and it runs once per blended pixel */
    if (back_on) return back[(unsigned long)y * fb_w + x];
    unsigned char *p = fb_base + (unsigned long)y * fb_pitch + (unsigned long)x * (fb_bpp / 8);
    return ((unsigned)p[2] << 16) | ((unsigned)p[1] << 8) | (unsigned)p[0];
}

/* darken a rectangle to num/den of its current brightness, by reading each
 * pixel back and scaling it. den<=0 is a no-op. This is one shadow pass. */
void fb_shade(int x, int y, int w, int h, int num, int den)
{
    if (den <= 0) return;
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++) {
            if ((unsigned)xx >= fb_w || (unsigned)yy >= fb_h) continue;
            unsigned int c = fb_get_px(xx, yy);
            int r = (int)((c >> 16) & 0xFF) * num / den;
            int g = (int)((c >> 8)  & 0xFF) * num / den;
            int b = (int)( c        & 0xFF) * num / den;
            put_pixel((unsigned)xx, (unsigned)yy,
                      ((unsigned)r << 16) | ((unsigned)g << 8) | (unsigned)b);
        }
}

/* a soft drop shadow, offset down-right of a window's footprint. No gaussian
 * blur on a CPU - instead, nested darkening passes from the outside in: a
 * pixel near the window edge is inside every pass (darkest), one far out is
 * inside only the outer pass (lightest), giving a graded falloff. `soft` is
 * the number of steps. The window is drawn on top afterwards, so only the
 * overhang shows. */
void fb_shadow(int x, int y, int w, int h, int off, int soft)
{
    if (soft < 1) soft = 1;

    /* Nested darkening passes leave visible concentric rings, because every
     * pixel gets a whole 10% step at each ring boundary. Instead walk the
     * shadow band once and darken each pixel by an amount that falls off
     * SMOOTHLY with its distance outside the window: the ring edges vanish and
     * one pass touches each pixel exactly once instead of `soft` times. */
    int x0 = x + off - soft, y0 = y + off - soft;
    int x1 = x + off + w + soft, y1 = y + off + h + soft;
    int ix0 = x + off, iy0 = y + off, ix1 = x + off + w, iy1 = y + off + h;

    /* THE SCISSOR, FOLDED INTO THE LOOP BOUNDS - the rule every other
     * primitive in this file follows, and this one did not.
     *
     * It clipped to the SCREEN and left put_pixel to reject the rest a pixel
     * at a time. That is correct and it was the most expensive mistake in the
     * renderer: wm.c calls chrome() once per window per damage rectangle, and
     * a drag damages a band. Each call walked the WHOLE shadow band of a
     * window - the widest, softest, most per-pixel loop in fb.c, measured at
     * 0.88 ms for one 600x460 window - and threw nearly all of it away.
     *
     * Measured on wmbench, dragging one window across the shell at 1920x1200:
     * chrome and wallpaper alone cost 6,651 us per frame, 55% of a 12,061 us
     * frame, with three windows and no app drawing at all.
     *
     * fb_shadow predates fb_clip: desktop-TODO 0b listed the five direct
     * back-buffer writers that needed the scissor folded in, and this is not
     * one of them - it goes through put_pixel, so it was CORRECT the whole
     * time and simply slow. Correct-but-slow is the harder kind to notice. */
    if (x0 < clip_x0) x0 = clip_x0;
    if (y0 < clip_y0) y0 = clip_y0;
    if (x1 > clip_x1) x1 = clip_x1;
    if (y1 > clip_y1) y1 = clip_y1;
    if (x0 >= x1 || y0 >= y1) return;

    /* The caller draws the window itself on top of this immediately afterwards,
     * so every shadow pixel under the window's own footprint is computed and
     * then painted over. That is ~90% of the pixels this loop visits, and the
     * shadow is the most expensive thing in a window redraw - measured at 4.3 ms
     * of a 5.1 ms window at 1920x1200 (kernel/hosttest/fbbench.c).
     *
     * So skip that rectangle. It is inset by SHADOW_SKIP_INSET rather than
     * skipped exactly, because the window has ROUNDED corners: the pixels just
     * outside the arc are still visible and still need their shadow. The inset
     * must stay larger than any radius draw_window uses (5 * ui() = 10 at 2x).
     *
     * Verified pixel-identical: FNV hash of the whole 1920x1200 back buffer
     * after shadow + rrect + rrect + gradient + text is unchanged. */
#define SHADOW_SKIP_INSET 16
    int sk_on = (w > 2 * SHADOW_SKIP_INSET) && (h > 2 * SHADOW_SKIP_INSET);
    int skx0 = x + SHADOW_SKIP_INSET, skx1 = x + w - SHADOW_SKIP_INSET;
    int sky0 = y + SHADOW_SKIP_INSET, sky1 = y + h - SHADOW_SKIP_INSET;

    for (int yy = y0; yy < y1; yy++) {
        if ((unsigned)yy >= fb_h) continue;
        int skip_row = sk_on && yy >= sky0 && yy < sky1;
        for (int xx = x0; xx < x1; xx++) {
            if ((unsigned)xx >= fb_w) continue;
            /* jump straight past the strip the window will cover */
            if (skip_row && xx >= skx0 && xx < skx1) { xx = skx1 - 1; continue; }
            /* how far outside the window's own footprint is this pixel? */
            int dx = 0, dy = 0;
            if (xx < ix0) dx = ix0 - xx; else if (xx >= ix1) dx = xx - ix1 + 1;
            if (yy < iy0) dy = iy0 - yy; else if (yy >= iy1) dy = yy - iy1 + 1;
            int d = dx > dy ? dx : dy;               /* chebyshev - cheap, square-ish */
            if (d > soft) continue;
            /* darkest against the window edge, fading to nothing at the rim */
            int amount = 62 - (62 * d) / (soft + 1); /* 0..62 percent darker  */
            if (amount <= 0) continue;
            unsigned int c = fb_get_px(xx, yy);
            int k = 100 - amount;
            int r = (int)((c >> 16) & 0xFF) * k / 100;
            int g = (int)((c >>  8) & 0xFF) * k / 100;
            int b = (int)( c        & 0xFF) * k / 100;
            put_pixel((unsigned)xx, (unsigned)yy,
                      ((unsigned)r << 16) | ((unsigned)g << 8) | (unsigned)b);
        }
    }
}

/* a filled rectangle with rounded corners - window panels and title bars.
 * Corner pixels outside the quarter-circle of radius r are skipped. Per-pixel
 * and only for one-shot window draws, not per-frame fills. */
/* A filled rounded rectangle with ANTIALIASED corners. A plain inside/outside
 * radius test gives a hard binary edge, and at these radii the staircase is
 * large next to the arc - one of the strongest "1990s" tells. Instead each
 * corner pixel is supersampled 4x4 to get a coverage value and blended over
 * what is already there, so the arc comes out smooth. Only the four corner
 * squares pay for this; the whole interior is still a straight fill. */
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb)
{
    if (2 * r > w) r = w / 2;
    if (2 * r > h) r = h / 2;
    if (r < 0) r = 0;

    /* the interior, as three plain rectangles - no per-pixel test at all */
    fb_fill_px(x + r, y, w - 2 * r, h, rgb);              /* centre column */
    fb_fill_px(x, y + r, r, h - 2 * r, rgb);              /* left edge     */
    fb_fill_px(x + w - r, y + r, r, h - 2 * r, rgb);      /* right edge    */

    if (r <= 0) return;

    /* the four corners, supersampled for coverage */
    for (int cy = 0; cy < r; cy++)
        for (int cx = 0; cx < r; cx++) {
            int hits = 0;
            for (int sy = 0; sy < 4; sy++)
                for (int sx = 0; sx < 4; sx++) {
                    /* sample at the centre of each quarter-pixel cell */
                    int dx = (cx * 8 + sx * 2 + 1) - r * 8;
                    int dy = (cy * 8 + sy * 2 + 1) - r * 8;
                    if (dx * dx + dy * dy <= r * r * 64) hits++;
                }
            if (hits == 0) continue;
            int a = hits * 255 / 16;
            int px[4], py[4];
            px[0] = x + cx;             py[0] = y + cy;              /* top-left     */
            px[1] = x + w - 1 - cx;     py[1] = y + cy;              /* top-right    */
            px[2] = x + cx;             py[2] = y + h - 1 - cy;      /* bottom-left  */
            px[3] = x + w - 1 - cx;     py[3] = y + h - 1 - cy;      /* bottom-right */
            for (int k = 0; k < 4; k++) {
                if (a >= 255) { put_pixel((unsigned)px[k], (unsigned)py[k], rgb); continue; }
                unsigned int bgc = fb_get_px(px[k], py[k]);
                put_pixel((unsigned)px[k], (unsigned)py[k], blend_rgb(bgc, rgb, a));
            }
        }
}

/* The same shape, TRANSLUCENT - which is what the prototype's panels actually
 * are, because its 34 border-radius declarations and its 107 rgba() ones
 * overlap. It reuses fb_rrect's exact corner supersample rather than inventing
 * a second one: two roundings that disagree by a pixel is how a UI ends up
 * with panels whose edges do not line up with each other.
 *
 * Coverage and opacity MULTIPLY. A half-covered corner pixel of a 40% panel is
 * 20%, not 50% and not 40% - getting this wrong gives a translucent panel a
 * hard opaque rim, which is the one artifact that makes it read as fake. */
void fb_rrect_blend(int x, int y, int w, int h, int r, unsigned int rgb, int a)
{
    if (w <= 0 || h <= 0 || a <= 0) return;
    if (a >= 255) { fb_rrect(x, y, w, h, r, rgb); return; }
    if (2 * r > w) r = w / 2;
    if (2 * r > h) r = h / 2;
    if (r < 0) r = 0;

    fb_fill_blend(x + r, y, w - 2 * r, h, rgb, a);              /* centre column */
    fb_fill_blend(x, y + r, r, h - 2 * r, rgb, a);              /* left edge     */
    fb_fill_blend(x + w - r, y + r, r, h - 2 * r, rgb, a);      /* right edge    */
    if (r <= 0) return;

    for (int cy = 0; cy < r; cy++)
        for (int cx = 0; cx < r; cx++) {
            int hits = 0;
            for (int sy = 0; sy < 4; sy++)
                for (int sx = 0; sx < 4; sx++) {
                    int dx = (cx * 8 + sx * 2 + 1) - r * 8;
                    int dy = (cy * 8 + sy * 2 + 1) - r * 8;
                    if (dx * dx + dy * dy <= r * r * 64) hits++;
                }
            if (hits == 0) continue;
            int ca = (hits * 255 / 16) * a / 255;
            blend_px(x + cx,         y + cy,         rgb, ca);
            blend_px(x + w - 1 - cx, y + cy,         rgb, ca);
            blend_px(x + cx,         y + h - 1 - cy, rgb, ca);
            blend_px(x + w - 1 - cx, y + h - 1 - cy, rgb, ca);
        }
}

/* ---- radial and conic gradients -------------------------------------------
 * Read off the prototype rather than guessed at. Every one of its six is a
 * colour fading to TRANSPARENT, not one colour to another:
 *
 *   radial-gradient(64% 50% at 16% 8%, rgba(126,166,44,.3) 0%, transparent 62%)
 *   conic-gradient(from 214deg at 50% 50%, transparent 0deg,
 *                  rgba(184,232,56,.075) 34deg, transparent 82deg)
 *
 * So the primitive is a GLOW composited over what is behind it, elliptical and
 * off-centre, not a two-colour fill. A two-colour version would have been
 * easier and would have painted over the wallpaper these exist to sit on.
 */

/* Integer square root, 32-bit, no division and no libgcc. The restoring
 * algorithm: two bits of the operand per iteration, sixteen iterations. */
static unsigned int isqrt32(unsigned int n)
{
    unsigned int rem = 0, root = 0;
    for (int i = 0; i < 16; i++) {
        root <<= 1;
        rem = (rem << 2) | (n >> 30);
        n <<= 2;
        if (root < rem) { rem -= root + 1; root += 2; }
    }
    return root >> 1;
}

/* ANGLE, without a division per pixel and without atan.
 *
 * |dy|/(|dx|+|dy|) is the cheap "diamond angle": monotone in the true angle,
 * one divide, and WRONG by up to 4 degrees if used directly - which is fine
 * for sorting and not fine for a wedge whose stops are given in degrees.
 *
 * So the diamond value is corrected through a table, built once by the same
 * monotone-walk inversion gamma_init() uses. For a true angle t, the diamond
 * value is 256*sin(t)/(sin(t)+cos(t)); walking t from 0 to 90 degrees fills
 * the inverse. isin() is Bhaskara's approximation, so the result is good to a
 * fraction of a degree - far tighter than anything a soft glow can show.
 *
 * Units are 1/1024 of a turn throughout, because a turn in 1024 parts divides
 * by shifting and degrees do not. */
static int isin(int deg);          /* the 3D engine's Bhaskara sine, below */
static int icos(int deg);
static unsigned short diamond_deg[257];
static int angle_ready = 0;

static void angle_init(void)
{
    int prev = 0;
    for (int t = 0; t <= 256; t++) diamond_deg[t] = 0;
    /* quarter-degree steps: 361 samples over the first quadrant */
    for (int q = 0; q <= 360; q++) {
        int deg4 = q;                       /* q/4 degrees */
        int s = isin(deg4 / 4), c = icos(deg4 / 4);
        if (s < 0) s = 0;
        if (c < 0) c = 0;
        if (s + c == 0) continue;
        int d = s * 256 / (s + c);          /* the diamond value at this angle */
        if (d > 256) d = 256;
        int a = q * 256 / 360;              /* 0..256 = 0..90 deg, in 1/1024 turn */
        for (int k = prev; k <= d; k++) diamond_deg[k] = (unsigned short)a;
        if (d >= prev) prev = d + 1;
    }
    for (int k = prev; k <= 256; k++) diamond_deg[k] = 256;
    angle_ready = 1;
}

/* 0..1023 anticlockwise from due east, in 1/1024 of a turn */
static int fb_angle1024(int dx, int dy)
{
    if (!angle_ready) angle_init();
    int ax = dx < 0 ? -dx : dx;
    int ay = dy < 0 ? -dy : dy;
    int s = ax + ay;
    if (s == 0) return 0;
    int q = diamond_deg[(ay * 256) / s];         /* 0..256 within the quadrant */
    if (dx >= 0) return (dy >= 0) ? q : (1024 - q);
    return (dy >= 0) ? (512 - q) : (512 + q);
}

/* One glow pixel. Split out because the radial and the conic differ only in
 * how they compute alpha, and duplicating the back-buffer/VRAM branch is how
 * two primitives drift apart. */
static void glow_px(int xx, int yy, unsigned int rgb, int a)
{
    if (a <= 0) return;
    if (a > 255) a = 255;
    if (back_on) {
        unsigned int *p = back + (unsigned long)yy * fb_w + xx;
        *p = blend_rgb(*p, rgb, a);
        return;
    }
    put_pixel((unsigned)xx, (unsigned)yy, blend_rgb(fb_get_px(xx, yy), rgb, a));
}

/* Clip a rectangle to the scissor. Returns 0 if nothing is left. */
static int clip_rect(int *x0, int *y0, int *x1, int *y1)
{
    if (*x0 < clip_x0) *x0 = clip_x0;
    if (*y0 < clip_y0) *y0 = clip_y0;
    if (*x1 > clip_x1) *x1 = clip_x1;
    if (*y1 > clip_y1) *y1 = clip_y1;
    return (*x0 < *x1 && *y0 < *y1);
}

/* radial-gradient(rx ry at cx cy, rgba(rgb, a0) 0%, transparent stop%)
 *
 * rx and ry are the ellipse's radii IN PIXELS; the caller turns the
 * prototype's percentages into those, because a percentage of what is a layout
 * question and this file does not do layout. `stop` is where the ramp ends, as
 * a percentage of the radius - 62 in the example above.
 *
 * TWO ALPHAS, because the prototype uses this shape in both directions. A GLOW
 * is (a_in = 77, a_out = 0): opaque at the centre, gone by the stop. A
 * VIGNETTE is (a_in = 0, a_out = 209, stop = 100): nothing at the centre,
 * darkening to the edge and staying dark beyond it. One primitive, and the
 * fourth radial-gradient in the source is the second kind. */
void fb_grad_radial(int x, int y, int w, int h, int cx, int cy,
                    int rx, int ry, unsigned int rgb, int a_in, int a_out,
                    int stop_pct)
{
    if (w <= 0 || h <= 0 || rx <= 0 || ry <= 0) return;
    if (a_in <= 0 && a_out <= 0) return;
    if (stop_pct <= 0) return;
    int x0 = x, y0 = y, x1 = x + w, y1 = y + h;
    if (!clip_rect(&x0, &y0, &x1, &y1)) return;

    /* distance is measured in 1/256 of a radius, so the stop is too */
    int stop = stop_pct * 256 / 100;
    if (stop <= 0) return;
    /* 16.16 per-pixel step, so the inner loop has no divide in it */
    int ustep = (int)((256 << 16) / rx);

    for (int yy = y0; yy < y1; yy++) {
        int v = (yy - cy) * 256 / ry;
        int vv = v * v;
        int uacc = ((x0 - cx) * 256 / rx) << 16;
        for (int xx = x0; xx < x1; xx++, uacc += ustep) {
            int u = uacc >> 16;
            int d2 = u * u + vv;
            if (d2 >= stop * stop) {
                /* past the stop the ramp has finished. With a_out == 0 - the
                 * glow case - that is nothing to draw, and skipping the square
                 * root here is most of what makes the glow affordable. */
                if (a_out <= 0) continue;
                glow_px(xx, yy, rgb, a_out);
                continue;
            }
            int d = (int)isqrt32((unsigned int)d2);
            glow_px(xx, yy, rgb, a_in + (a_out - a_in) * d / stop);
        }
    }
    fb_damage(x0, y0, x1 - x0, y1 - y0);
}

/* conic-gradient(from `from` at cx cy, transparent 0deg, rgba(rgb,a0) `mid`,
 * transparent `end`) - a soft wedge. Angles are degrees, as the source writes
 * them; they are converted to 1/1024 turn once, here, not per pixel. */
void fb_grad_conic(int x, int y, int w, int h, int cx, int cy,
                   unsigned int rgb, int a0, int from_deg, int mid_deg,
                   int end_deg)
{
    if (w <= 0 || h <= 0 || a0 <= 0) return;
    if (mid_deg <= 0 || end_deg <= mid_deg) return;
    int x0 = x, y0 = y, x1 = x + w, y1 = y + h;
    if (!clip_rect(&x0, &y0, &x1, &y1)) return;

    int from = (from_deg * 1024 / 360) & 1023;
    int mid  = mid_deg * 1024 / 360;
    int end  = end_deg * 1024 / 360;

    for (int yy = y0; yy < y1; yy++) {
        for (int xx = x0; xx < x1; xx++) {
            /* CSS measures a conic gradient CLOCKWISE from twelve o'clock;
             * fb_angle1024 measures anticlockwise from three. Getting this
             * wrong mirrors the wedge, which looks deliberate and is not. */
            int a = fb_angle1024(xx - cx, cy - yy);
            int rel = (256 - a - from) & 1023;
            if (rel >= end) continue;
            int alpha = (rel <= mid) ? (a0 * rel / mid)
                                     : (a0 * (end - rel) / (end - mid));
            glow_px(xx, yy, rgb, alpha);
        }
    }
    fb_damage(x0, y0, x1 - x0, y1 - y0);
}

/* The zl-facing forms. A background decoration covers the whole screen, so the
 * rectangle is implicit - which also keeps these inside the interpreter's
 * eight-argument ceiling, and inside the zl parser's rule that a call must fit
 * on one line. */
void fb_glow(int cx, int cy, int rx, int ry, unsigned int rgb, int a_in, int a_out, int stop_pct)
{ fb_grad_radial(0, 0, (int)fb_w, (int)fb_h, cx, cy, rx, ry, rgb, a_in, a_out, stop_pct); }

void fb_wedge(int cx, int cy, unsigned int rgb, int a0, int from_deg,
              int mid_deg, int end_deg)
{ fb_grad_conic(0, 0, (int)fb_w, (int)fb_h, cx, cy, rgb, a0, from_deg, mid_deg, end_deg); }

/* A TITLE BAR IS A ROUNDED-TOP GRADIENT, and it needs to be one primitive.
 *
 * The title band used to be a plain rectangle drawn inside the window's
 * rounded frame. At radius 5 nobody could see the difference; at the radius the
 * v10 metrics ask for - 12, from the prototype's 11..16 cluster - the square
 * corners of the band sit visibly proud of the round corners of the frame, and
 * the window reads as two shapes that do not fit each other.
 *
 * Top corners rounded, bottom square, because that is what a title bar is: the
 * bottom edge butts into the client area and must not round away from it. A
 * general "which corners" mask would be a parameter nobody would ever pass
 * anything else to. */
void fb_rrect_grad_top(int x, int y, int w, int h, int r,
                       unsigned int top, unsigned int bot)
{
    if (w <= 0 || h <= 0) return;
    if (2 * r > w) r = w / 2;
    if (r > h) r = h;
    if (r < 0) r = 0;

    /* the gradient, in full, but only where the shape has ink. Drawing the
     * whole band and then cutting the corners back would need to know what was
     * underneath them, which is exactly the information a corner has lost by
     * the time it is painted over. */
    /* THE TOP STRIP IS A SLICE OF THE SAME RAMP, not a ramp of its own.
     * fb_gradient interpolates top->bot across the height it is given, so
     * handing it (top, bot) over r rows compresses the whole ramp into the
     * first r rows and leaves a visible step where the strip meets the rest -
     * a lighter block across the top of every title bar. Work out the colour
     * the full-height ramp has reached at row r, and hand it that. */
    int tr = (top >> 16) & 0xFF, tg = (top >> 8) & 0xFF, tb = top & 0xFF;
    int br = (bot >> 16) & 0xFF, bg = (bot >> 8) & 0xFF, bb = bot & 0xFF;
    unsigned int at_r = (unsigned)((tr + (br - tr) * r / h) << 16)
                      | (unsigned)((tg + (bg - tg) * r / h) << 8)
                      | (unsigned)( tb + (bb - tb) * r / h);
    if (r > 0) fb_gradient(x + r, y, w - 2 * r, r, top, at_r);
    fb_gradient(x, y + r, w, h - r, at_r, bot);
    if (r <= 0) return;

    /* The two top corners, at the SAME 4x4 supersample fb_rrect uses, so a
     * title bar's curve and its frame's curve are the same curve. The colour
     * comes from the gradient at that row, which is why this cannot just call
     * fb_rrect with one colour. */
    for (int cy = 0; cy < r; cy++) {
        unsigned int c = (unsigned)((tr + (br - tr) * cy / h) << 16)
                       | (unsigned)((tg + (bg - tg) * cy / h) << 8)
                       | (unsigned)(tb + (bb - tb) * cy / h);
        for (int cx = 0; cx < r; cx++) {
            int hits = 0;
            for (int sy = 0; sy < 4; sy++)
                for (int sx = 0; sx < 4; sx++) {
                    int dx = (cx * 8 + sx * 2 + 1) - r * 8;
                    int dy = (cy * 8 + sy * 2 + 1) - r * 8;
                    if (dx * dx + dy * dy <= r * r * 64) hits++;
                }
            if (hits == 0) continue;
            int a = hits * 255 / 16;
            blend_px(x + cx,         y + cy, c, a);
            blend_px(x + w - 1 - cx, y + cy, c, a);
        }
    }
}

/* a rectangle OUTLINE - for panels, buttons and window frames */
void fb_box(int x, int y, int w, int h, unsigned int rgb)
{
    fb_fill_px(x, y, w, 1, rgb);
    fb_fill_px(x, y + h - 1, w, 1, rgb);
    fb_fill_px(x, y, 1, h, rgb);
    fb_fill_px(x + w - 1, y, 1, h, rgb);
}

/* A straight line, any angle - Wu's algorithm, integer only, no floating point.
 *
 * This was plain Bresenham: one hard on/off pixel per step, so every diagonal
 * staircased. The System Monitor sparkline is eight diagonal segments sitting
 * directly beside gamma-correct anti-aliased text, and the contrast made it
 * worse rather than hiding it (desktop-look.md, bug 2).
 *
 * Wu's insight is that a diagonal line passes BETWEEN two pixels, so light
 * both, in proportion to how close the true line is to each. The exact ratio
 * is the fractional part of the line's position, which the 16.16 accumulator
 * below already carries - so anti-aliasing costs one extra blended pixel per
 * step and no arithmetic that was not already there.
 *
 * The axis with the greater extent is the one stepped along (`steep` swaps x
 * and y to make that always x), which is what keeps the coverage sensible: on
 * the other axis the line would move more than a pixel per step and the two
 * lit pixels would not bracket it. */
void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    int adx = x1 - x0, ady = y1 - y0;
    if (adx < 0) adx = -adx;
    if (ady < 0) ady = -ady;

    int steep = ady > adx;
    if (steep) { int t; t = x0; x0 = y0; y0 = t; t = x1; x1 = y1; y1 = t; }
    if (x0 > x1) { int t; t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t; }

    int dx = x1 - x0, dy = y1 - y0;
    /* 16.16 pixels of secondary axis per whole pixel of primary axis. dx == 0
     * only for a single point, where the loop body runs once and grad is
     * never applied. */
    int grad = dx ? (int)(((long long)dy << 16) / dx) : 0;

    int acc = y0 << 16;
    for (int x = x0; x <= x1; x++) {
        int yi = acc >> 16;            /* arithmetic shift: floor, not trunc */
        int f  = acc & 0xFFFF;         /* ...so this fraction is always >= 0 */
        int a2 = (f * 255) >> 16;      /* how far into the NEXT pixel we are  */
        int a1 = 255 - a2;
        if (steep) {
            blend_px(yi,     x, rgb, a1);
            blend_px(yi + 1, x, rgb, a2);
        } else {
            blend_px(x, yi,     rgb, a1);
            blend_px(x, yi + 1, rgb, a2);
        }
        acc += grad;
    }
}

/* ---- a tiny software 3D engine ------------------------------------------
 * No GPU and no floating point, so rotation uses an integer sine (Bhaskara I's
 * approximation, scaled by 1024) and everything is fixed-point. This rotates a
 * unit cube on two axes, projects it to 2D with a perspective divide, and draws
 * the 12 edges as lines - the same wireframe TempleOS spun, done from scratch. */
static int isin(int deg)
{
    deg %= 360; if (deg < 0) deg += 360;
    int sign = 1;
    if (deg > 180) { deg -= 180; sign = -1; }
    int t = deg * (180 - deg);
    int den = 40500 - t; if (den == 0) den = 1;
    return sign * (4 * t * 1024 / den);
}
static int icos(int deg) { return isin(deg + 90); }

void fb_cube(int cx, int cy, int size, int angle, unsigned int color)
{
    static const int V[8][3] = {
        {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
        {-1,-1, 1},{1,-1, 1},{1,1, 1},{-1,1, 1}};
    static const int E[12][2] = {
        {0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    int s = isin(angle),  c = icos(angle);
    int a2 = angle * 7 / 10;                 /* the tilt axis turns slower */
    int s2 = isin(a2), c2 = icos(a2);
    int D = size * 4;                        /* camera distance for the projection */
    int px[8], py[8];
    for (int i = 0; i < 8; i++) {
        int x = V[i][0] * size, y = V[i][1] * size, z = V[i][2] * size;
        int x1 = (x * c - z * s) / 1024;     /* rotate about Y */
        int z1 = (x * s + z * c) / 1024;
        int y1 = (y * c2 - z1 * s2) / 1024;  /* then about X */
        int z2 = (y * s2 + z1 * c2) / 1024;
        int denom = D + z2; if (denom < 1) denom = 1;
        px[i] = cx + x1 * D / denom;         /* perspective divide */
        py[i] = cy + y1 * D / denom;
    }
    for (int e = 0; e < 12; e++)
        fb_line(px[E[e][0]], py[E[e][0]], px[E[e][1]], py[E[e][1]], color);
}

/* a mouse pointer: a small filled arrow, plus a contrasting outline so it
 * shows on any background. Drawn/erased by the caller each frame. */
void fb_cursor_arrow(int x, int y, unsigned int fill, unsigned int edge)
{
    /* a simple 12x18 triangle-ish pointer, row by row */
    for (int r = 0; r < 16; r++) {
        int w = r < 12 ? r + 1 : (16 - r) * 2;
        if (w < 1) w = 1;
        if (w > 8) w = 8;
        fb_fill_px(x, y + r, w, 1, fill);
        put_pixel((unsigned)(x + w), (unsigned)(y + r), edge);   /* right edge */
    }
    fb_fill_px(x, y, 1, 16, edge);       /* left edge */
    fb_fill_px(x, y, 9, 1, edge);        /* top edge  */
}

/* ---- the live mouse pointer, as a sprite -------------------------------
 * No hardware cursor on a raw framebuffer, so we do it the way every pre-GPU
 * OS did: before drawing the arrow, SAVE the little patch of screen under it;
 * to move it, RESTORE that patch (erasing the arrow cleanly), then save + draw
 * at the new spot. Two tiny copies, no full redraw - the arrow glides. */
#define CUR_W 11
#define CUR_H 17
static unsigned int cur_buf[CUR_W * CUR_H];
static int cur_x = 0, cur_y = 0, cur_up = 0;

/* put the saved patch back, removing the arrow from the screen */
void fb_pointer_hide(void)
{
    if (!cur_up) return;
    for (int j = 0; j < CUR_H; j++)
        for (int i = 0; i < CUR_W; i++)
            put_pixel((unsigned)(cur_x + i), (unsigned)(cur_y + j), cur_buf[j * CUR_W + i]);
    cur_up = 0;
}

/* move/redraw the arrow to (x,y): hide the old one, save the new patch, draw */
void fb_pointer_show(int x, int y)
{
    fb_pointer_hide();
    cur_x = x; cur_y = y;
    for (int j = 0; j < CUR_H; j++)
        for (int i = 0; i < CUR_W; i++)
            cur_buf[j * CUR_W + i] = fb_get_px(x + i, y + j);
    cur_up = 1;
    fb_cursor_arrow(x, y, 0xEEF4FF, 0x0A0E18);   /* white arrow, dark outline */
}

/* ---- blur, cached ---------------------------------------------------------
 * The prototype asks for backdrop-filter:blur() in six places and filter:blur()
 * in two more. The v10 plan measured one menu-sized backdrop blur at 8.7 ms
 * against a 16.67 ms frame budget and drew the right conclusion: a blur behind
 * something that MOVES is half the frame, every frame, and cannot be afforded.
 * A blur behind something that opens and then sits still is paid once.
 *
 * So this is a cache, not a filter. fb_blur_cache() blurs what is on screen
 * behind a rectangle into a fixed slot; fb_blur_paint() blits that slot back,
 * as many frames as you like, for the cost of a copy.
 *
 * THE DESIGN CONSEQUENCE IS A UI DECISION, and it is better taken here than
 * discovered later: a panel that blurs must not be draggable, or must re-blur
 * only on drop. Dragging one would look exactly like a frame-rate collapse.
 *
 * The algorithm is a BOX blur run twice, horizontally then vertically each
 * time. Two box passes approximate a Gaussian closely enough that nobody can
 * tell at these radii, and a box blur with a running sum costs O(1) per pixel
 * per pass regardless of radius - so a 30 px blur costs what a 10 px blur
 * costs. A true Gaussian would be radius-proportional and would need the
 * multiply-heavy inner loop this kernel is trying to avoid.
 *
 * THE DIVIDE IS THE WHOLE COST, and that is worth writing down because the
 * first version of this measured 15.0 ms - nearly twice the budget it was
 * written to fit. Dividing the running sum by the window width is three
 * divides per pixel per pass and there are four passes: twelve integer divides
 * per pixel, at roughly twenty cycles each, which is 125 cyc/px of the 125
 * cyc/px measured. The window width is constant for the whole call, so it
 * becomes one reciprocal computed once and a multiply-and-shift per channel.
 * Same pixels, 5.4x faster. (Contrast desktop-TODO 0i, where tabling a divide
 * made a shadow SLOWER - that loop was not arithmetic-bound and this one is.
 * Which is why both were measured instead of reasoned about.)
 *
 * The arena is the 16 MiB at 192 MiB that the back buffer vacated in C4 -
 * three slots and one shared scratch, exactly filling it.
 */
/* ---- the cache arena ------------------------------------------------------
 * 16 MiB at 192 MiB, which is exactly the span the back buffer vacated in C4.
 * Two customers with very different shapes want it, so it is a BUMP ALLOCATOR
 * rather than a fixed split:
 *
 *   the WALLPAPER cache   one screen-sized bitmap, taken first
 *   the BLUR slots        a handful of panel-sized ones, out of what is left
 *
 * A fixed split would have to be sized for the largest screen and would then
 * waste most of the arena on the common one. The bump pointer only ever moves
 * forward and is reset wholesale; there is no free list, because nothing here
 * frees one buffer and keeps another.
 *
 * WHY THE WALLPAPER NEEDS A CACHE AT ALL, with the number that decides it: the
 * v10 background is a gradient plus three radial glows and two conic wedges,
 * all translucent. A translucent full-screen pass costs 22 cyc/px measured -
 * about 22 ms at 1920x1200 - and the compositor repaints the wallpaper inside
 * every damage rectangle, so five of them would be 100 ms of a 16.67 ms frame.
 * Drawn once and blitted, it is 1.5 cyc/px. There is no version of this that
 * is affordable per frame; caching is not an optimisation here, it is the only
 * way the look exists.
 */
static unsigned int *arena_next = (unsigned int *)HI_BLUR;

static unsigned int *arena_take(unsigned int px)
{
    unsigned long top = (unsigned long)arena_next + (unsigned long)px * 4u;
    if (top > (unsigned long)HI_BLUR + BLUR_LIMIT) return 0;
    unsigned int *p = arena_next;
    arena_next = (unsigned int *)top;
    return p;
}

static unsigned int arena_free_px(void)
{
    return (unsigned int)(((unsigned long)HI_BLUR + BLUR_LIMIT
                           - (unsigned long)arena_next) / 4u);
}

void fb_cache_reset(void)
{
    arena_next = (unsigned int *)HI_BLUR;
}

/* ---- the wallpaper cache -------------------------------------------------- */
static unsigned int *wall_buf;
static int wall_w, wall_h;

int fb_wall_ok(void) { return wall_buf != 0 && wall_w == (int)fb_w && wall_h == (int)fb_h; }

/* Copy what is currently on screen into the wallpaper cache. The caller draws
 * the wallpaper it wants - gradient, glows, wedges, whatever policy says - and
 * then calls this once. fb.c never decides what a wallpaper looks like. */
int fb_wall_save(void)
{
    if (!fb_active()) return 0;
    if (!wall_buf || wall_w != (int)fb_w || wall_h != (int)fb_h) {
        wall_buf = arena_take(fb_w * fb_h);
        if (!wall_buf) {
            fb_puts("  fb: wallpaper cache refused - wants ");
            fb_putu(fb_w * fb_h * 4u >> 10);
            fb_puts(" KiB, "); fb_putu(arena_free_px() * 4u >> 10);
            fb_puts(" KiB left in the arena. Plain gradient instead.\n");
            return 0;
        }
        wall_w = (int)fb_w; wall_h = (int)fb_h;
    }
    for (unsigned int y = 0; y < fb_h; y++)
        for (unsigned int x = 0; x < fb_w; x++)
            wall_buf[(unsigned long)y * fb_w + x] = fb_get_px((int)x, (int)y);
    fb_puts("  fb: wallpaper cached, "); fb_putu(fb_w * fb_h * 4u >> 10);
    fb_puts(" KiB; "); fb_putu(arena_free_px() * 4u >> 10);
    fb_puts(" KiB left for blur\n");
    return 1;
}

/* Blit part of the cached wallpaper back. This is what desk_draw costs now. */
void fb_wall_paint(int x, int y, int w, int h)
{
    if (!fb_wall_ok()) return;
    int x0 = x, y0 = y, x1 = x + w, y1 = y + h;
    if (!clip_rect(&x0, &y0, &x1, &y1)) return;
    if (back_on) {
        for (int yy = y0; yy < y1; yy++) {
            const unsigned int *s = wall_buf + (unsigned long)yy * wall_w + x0;
            unsigned int *o = back + (unsigned long)yy * fb_w + x0;
            for (int xx = x0; xx < x1; xx++) *o++ = *s++;
        }
        fb_damage(x0, y0, x1 - x0, y1 - y0);
        return;
    }
    for (int yy = y0; yy < y1; yy++)
        for (int xx = x0; xx < x1; xx++)
            put_pixel((unsigned)xx, (unsigned)yy,
                      wall_buf[(unsigned long)yy * wall_w + xx]);
}

/* ---- blur slots ------------------------------------------------------------ */
#define BLUR_SLOTS 4

static struct { unsigned int *px; int w, h, cap, used; } blur_slot[BLUR_SLOTS];
static unsigned int *blur_tmp;
static unsigned int  blur_tmp_cap;

void fb_blur_free(int slot)
{
    if (slot >= 0 && slot < BLUR_SLOTS) blur_slot[slot].used = 0;
}

void fb_blur_free_all(void)
{
    for (int i = 0; i < BLUR_SLOTS; i++) blur_slot[i].used = 0;
}

/* one box pass along rows, from src into dst, radius r */
#define BOX_SHIFT 22
static void box_h(unsigned int *dst, const unsigned int *src, int w, int h, int r)
{
    int d = 2 * r + 1;
    unsigned int rec = ((1u << BOX_SHIFT) + (unsigned)d - 1) / (unsigned)d;
    for (int y = 0; y < h; y++) {
        const unsigned int *s = src + (unsigned long)y * w;
        unsigned int *o = dst + (unsigned long)y * w;
        /* prime the running sum with the left edge clamped - clamping rather
         * than wrapping or zeroing is what stops a blurred panel growing a
         * dark border out of nowhere */
        unsigned int sr = 0, sg = 0, sb = 0;
        for (int i = -r; i <= r; i++) {
            int k = i < 0 ? 0 : (i >= w ? w - 1 : i);
            sr += (s[k] >> 16) & 0xFF; sg += (s[k] >> 8) & 0xFF; sb += s[k] & 0xFF;
        }
        for (int x = 0; x < w; x++) {
            o[x] = (((sr * rec) >> BOX_SHIFT) << 16)
                 | (((sg * rec) >> BOX_SHIFT) << 8)
                 |  ((sb * rec) >> BOX_SHIFT);
            int add = x + r + 1, sub = x - r;
            add = add >= w ? w - 1 : add;
            sub = sub < 0 ? 0 : sub;
            sr += ((s[add] >> 16) & 0xFF) - ((s[sub] >> 16) & 0xFF);
            sg += ((s[add] >>  8) & 0xFF) - ((s[sub] >>  8) & 0xFF);
            sb += ( s[add]        & 0xFF) - ( s[sub]        & 0xFF);
        }
    }
}

/* the same along columns */
static void box_v(unsigned int *dst, const unsigned int *src, int w, int h, int r)
{
    int d = 2 * r + 1;
    unsigned int rec = ((1u << BOX_SHIFT) + (unsigned)d - 1) / (unsigned)d;
    for (int x = 0; x < w; x++) {
        unsigned int sr = 0, sg = 0, sb = 0;
        for (int i = -r; i <= r; i++) {
            int k = i < 0 ? 0 : (i >= h ? h - 1 : i);
            unsigned int v = src[(unsigned long)k * w + x];
            sr += (v >> 16) & 0xFF; sg += (v >> 8) & 0xFF; sb += v & 0xFF;
        }
        for (int y = 0; y < h; y++) {
            dst[(unsigned long)y * w + x] = (((sr * rec) >> BOX_SHIFT) << 16)
                                          | (((sg * rec) >> BOX_SHIFT) << 8)
                                          |  ((sb * rec) >> BOX_SHIFT);
            int add = y + r + 1, sub = y - r;
            add = add >= h ? h - 1 : add;
            sub = sub < 0 ? 0 : sub;
            unsigned int va = src[(unsigned long)add * w + x];
            unsigned int vs = src[(unsigned long)sub * w + x];
            sr += ((va >> 16) & 0xFF) - ((vs >> 16) & 0xFF);
            sg += ((va >>  8) & 0xFF) - ((vs >>  8) & 0xFF);
            sb += ( va        & 0xFF) - ( vs        & 0xFF);
        }
    }
}

/* Take a slot big enough for w x h and copy the screen into it. The half of
 * fb_blur_cache that is not the blur - and the whole of fb_stash, which is the
 * same operation with no filter on the end. */
static int slot_capture(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) return -1;
    unsigned int need = (unsigned int)(w * h);

    /* an unused slot whose buffer is already big enough, or a free slot that
     * can take a new one out of the arena */
    int slot = -1;
    for (int i = 0; i < BLUR_SLOTS; i++)
        if (!blur_slot[i].used && blur_slot[i].px && (unsigned)blur_slot[i].cap >= need) { slot = i; break; }
    if (slot < 0)
        for (int i = 0; i < BLUR_SLOTS; i++)
            if (!blur_slot[i].used && !blur_slot[i].px) { slot = i; break; }
    if (slot < 0) { fb_puts("  fb: blur refused - every slot is in use\n"); return -1; }

    if (!blur_slot[slot].px || (unsigned)blur_slot[slot].cap < need) {
        unsigned int *p = arena_take(need);
        if (!p) {
            fb_puts("  fb: blur refused - wants "); fb_putu(need * 4u >> 10);
            fb_puts(" KiB, arena has "); fb_putu(arena_free_px() * 4u >> 10);
            fb_puts(" KiB\n");
            return -1;
        }
        blur_slot[slot].px = p;
        blur_slot[slot].cap = (int)need;
    }
    /* the horizontal pass needs somewhere to land that is not the destination */
    if (!blur_tmp || blur_tmp_cap < need) {
        unsigned int *p = arena_take(need);
        if (!p) { fb_puts("  fb: blur refused - no arena for the scratch pass\n"); return -1; }
        blur_tmp = p;
        blur_tmp_cap = need;
    }

    unsigned int *dst = blur_slot[slot].px;
    /* Read the source back. With a back buffer that is a row copy out of RAM;
     * without one every pixel crosses PCIe, and fb_get_px is the only thing
     * that knows the difference. Going through it per pixel cost a call and
     * two bounds tests 276,000 times in the measured case. */
    if (back_on && x >= 0 && y >= 0 &&
        x + w <= (int)fb_w && y + h <= (int)fb_h) {
        for (int j = 0; j < h; j++) {
            const unsigned int *s = back + (unsigned long)(y + j) * fb_w + x;
            unsigned int *o = dst + (unsigned long)j * w;
            for (int i = 0; i < w; i++) o[i] = s[i];
        }
    } else {
        for (int j = 0; j < h; j++)
            for (int i = 0; i < w; i++)
                dst[(unsigned long)j * w + i] = fb_get_px(x + i, y + j);
    }

    blur_slot[slot].w = w;
    blur_slot[slot].h = h;
    blur_slot[slot].used = 1;
    return slot;
}

/* Blur what is currently on screen inside (x,y,w,h) into a slot. Returns the
 * slot, or -1 with a reason on the serial log - a refusal, never a silent
 * no-op, the same rule as WM_MAX and ANIM_MAX. */
int fb_blur_cache(int x, int y, int w, int h, int radius)
{
    int slot = slot_capture(x, y, w, h);
    if (slot < 0) return -1;
    if (radius < 1) radius = 1;
    /* A box radius that reaches past the rectangle makes the running sum read
     * nothing but the clamped edge, which blurs the panel to a flat colour. */
    if (radius > w / 2) radius = w / 2;
    if (radius > h / 2) radius = h / 2;
    if (radius < 1) radius = 1;

    unsigned int *dst = blur_slot[slot].px;
    box_h(blur_tmp, dst, w, h, radius);      /* pass 1 */
    box_v(dst, blur_tmp, w, h, radius);
    box_h(blur_tmp, dst, w, h, radius);      /* pass 2 - two boxes ~ a Gaussian */
    box_v(dst, blur_tmp, w, h, radius);
    return slot;
}

/* ---- a real opacity fade --------------------------------------------------
 * The one effect the v10 run shipped WITHOUT compositing, and the reason was
 * honest: a fade needs the window drawn against what is BEHIND it at
 * fractional opacity, which needs a copy of the rectangle taken before the
 * window was drawn on it. ANIM_PULSE composited because a tint needs no copy.
 *
 * The cache arena can hold one, so:
 *
 *   stash the rect  ->  draw the window over it  ->  blend the stash back at
 *   (255 - alpha)
 *
 * which is algebraically exactly `window * a + behind * (1 - a)`. It is not an
 * approximation of a fade, it is one.
 *
 * A slot is taken and released every frame, and after the first frame that
 * costs nothing: slot_capture reuses an unused slot whose buffer is already
 * big enough, so the bump allocator only ever grows once per size. */
void fb_blur_paint(int slot, int x, int y);

int fb_stash(int x, int y, int w, int h) { return slot_capture(x, y, w, h); }

void fb_stash_blend(int slot, int x, int y, int a)
{
    if (slot < 0 || slot >= BLUR_SLOTS || !blur_slot[slot].used) return;
    if (a <= 0) return;
    int w = blur_slot[slot].w, h = blur_slot[slot].h;
    const unsigned int *src = blur_slot[slot].px;
    int x0 = x, y0 = y, x1 = x + w, y1 = y + h;
    if (!clip_rect(&x0, &y0, &x1, &y1)) return;

    if (a >= 255) { fb_blur_paint(slot, x, y); return; }
    if (back_on) {
        for (int yy = y0; yy < y1; yy++) {
            unsigned int *o = back + (unsigned long)yy * fb_w;
            const unsigned int *s = src + (unsigned long)(yy - y) * w - x;
            for (int xx = x0; xx < x1; xx++)
                o[xx] = blend_rgb(o[xx], s[xx], a);
        }
        fb_damage(x0, y0, x1 - x0, y1 - y0);
        return;
    }
    for (int yy = y0; yy < y1; yy++)
        for (int xx = x0; xx < x1; xx++)
            put_pixel((unsigned)xx, (unsigned)yy,
                      blend_rgb(fb_get_px(xx, yy),
                                src[(unsigned long)(yy - y) * w + (xx - x)], a));
}

/* Paint a cached blur. This is the per-frame cost and it is a copy - the whole
 * reason the cache exists. */
void fb_blur_paint(int slot, int x, int y)
{
    if (slot < 0 || slot >= BLUR_SLOTS || !blur_slot[slot].used) return;
    int w = blur_slot[slot].w, h = blur_slot[slot].h;
    const unsigned int *src = blur_slot[slot].px;
    int x0 = x, y0 = y, x1 = x + w, y1 = y + h;
    if (!clip_rect(&x0, &y0, &x1, &y1)) return;

    if (back_on) {
        for (int yy = y0; yy < y1; yy++) {
            unsigned int *o = back + (unsigned long)yy * fb_w + x0;
            const unsigned int *s = src + (unsigned long)(yy - y) * w + (x0 - x);
            for (int xx = x0; xx < x1; xx++) *o++ = *s++;
        }
        fb_damage(x0, y0, x1 - x0, y1 - y0);
        return;
    }
    for (int yy = y0; yy < y1; yy++)
        for (int xx = x0; xx < x1; xx++)
            put_pixel((unsigned)xx, (unsigned)yy,
                      src[(unsigned long)(yy - y) * w + (xx - x)]);
}

/* ---- what used to live here -----------------------------------------------
 * fb_bg_snapshot / fb_bg_restore / fb_grab / fb_stamp, plus bg_buf at 128 MiB
 * and sp_buf at 160 MiB: the sticky-note way to drag a window without a GPU.
 * Snapshot the whole desktop, grab the window as a bitmap, then per frame
 * restore the background where it WAS and stamp the sprite where it IS.
 *
 * DELETED, C4. Not because it was slow - it was fast, that was its point - but
 * because a compositor that repaints from damage has no use for it and it cost
 * three things that were nothing to do with dragging:
 *
 *   - 48 MiB of the high-RAM map, which the back buffer now has, and which is
 *     what lets the back buffer cover 3840x2160 at all
 *   - a 640x480 sprite ceiling, which is why the 1256x944 terminal was the one
 *     window that could never be dragged
 *   - a 12 px shadow smear on every drag step: fb_shadow reaches x + w + 28 at
 *     ui() == 2 and the restore erased w + 16
 *
 * KEPT, deliberately: fb_pointer_show / fb_pointer_hide above. Save-under for
 * an 11x17 cursor is the same technique at a size where it is correct - it has
 * no ceiling to exceed and no shadow to smear.
 */



/* a software cursor block at a text cell (framebuffer has no hardware one) */
void fb_cursor(int row, int col, int on, unsigned char attr)
{
    if (row < 0 || row >= fb_rows || col < 0 || col >= fb_cols) return;
    fill_cell(col, row, on ? vga_rgb[attr & 0x0F] : fb_bg);
}

/* Scrolling a framebuffer means moving every byte of every scanline. It is
 * the slowest thing this console does, which is why the log region is kept
 * small rather than scrolling the whole screen. */
static void fb_scroll(int top, int bot)
{
    /* This is the one draw path that writes via raw pointers instead of the
     * bounds-checked put_pixel, so a bad region must never reach it: clamp the
     * bottom row so the source read (row bot+1) can't run past the framebuffer. */
    if (bot > fb_rows - 1) bot = fb_rows - 1;
    if (top < 0) top = 0;
    if (top >= bot) return;
    int c0 = tx0, c1 = text_c1();
    int y_top = top * cell_h, y_bot = bot * cell_h;
    /* move each scanline up by one text row, but only within the text box's
     * columns - so a floating terminal scrolls without touching its neighbours */
    if (back_on) {
        /* the third and last direct writer of the back buffer, and it needs
         * the scissor for the same reason the other two do. In practice the
         * clamp is a no-op: once the shell is a window (E3) the text box sits
         * inside the client area, which is what the scissor will be. It exists
         * so that a NARROWER scissor scrolls only what it was told to. */
        int x0 = c0 * cell_w;
        int x1 = x0 + (c1 - c0 + 1) * cell_w;
        if (x0 < clip_x0) x0 = clip_x0;
        if (x1 > clip_x1) x1 = clip_x1;
        int ys = y_top < clip_y0 ? clip_y0 : y_top;
        int ye = y_bot < clip_y1 ? y_bot : clip_y1;
        int npx = x1 - x0;
        if (npx > 0 && ys < ye) {
            for (int y = ys; y < ye; y++) {
                unsigned int *d = back + (unsigned long)y * fb_w + x0;
                unsigned int *s = back + (unsigned long)(y + cell_h) * fb_w + x0;
                for (int i = 0; i < npx; i++) d[i] = s[i];
            }
            fb_damage(x0, ys, npx, ye - ys);
        }
    } else {
        int bpx = (int)(fb_bpp / 8);
        unsigned long xoff = (unsigned long)c0 * cell_w * bpx;
        int nbytes = (c1 - c0 + 1) * cell_w * bpx;
        for (int y = y_top; y < y_bot; y++) {
            unsigned char *dl = fb_base + (unsigned long)y * fb_pitch + xoff;
            unsigned char *sl = fb_base + (unsigned long)(y + cell_h) * fb_pitch + xoff;
            for (int b = 0; b < nbytes; b++) dl[b] = sl[b];
        }
    }
    for (int c = c0; c <= c1; c++) fill_cell(c, bot, fb_bg);
    fb_row = bot;
}

void fb_putc(char c, int log_top, int log_bot)
{
    int c1 = text_c1();
    if (c == '\n') { fb_col = tx0; fb_row++; }
    else if (c == '\r') { fb_col = tx0; }
    else if (c == '\b') { if (fb_col > tx0) { fb_col--; fill_cell(fb_col, fb_row, fb_bg); } }
    else if (c == '\t') { fb_col = (fb_col + 8) & ~7; if (fb_col > c1) fb_col = c1; }
    else {
        draw_glyph(fb_col, fb_row, c, fb_fg, fb_bg);
        fb_col++;
    }
    if (fb_col > c1) { fb_col = tx0; fb_row++; }
    if (fb_row > log_bot) fb_scroll(log_top, log_bot);
}

void fb_set_row(int r, int log_top, int log_bot)
{
    if (r < log_top) r = log_top;
    if (r > log_bot) r = log_bot;
    fb_row = r;
    fb_col = tx0;
}

int fb_get_row(void) { return fb_row; }
int fb_get_col(void) { return fb_col; }

/* ---- proportional text ----------------------------------------------------
 * fb_text_aa advances by exactly cell_w per character, so every string it
 * draws is monospace-positioned - window titles, dock labels, button captions,
 * all of it. desktop-look.md item 4: uniform advance is the single strongest
 * "this is a terminal" signal there is, and it was being applied to things
 * that are not terminals.
 *
 * This is the same blend and the same coverage-atlas machinery, with two
 * differences: the glyphs come from DejaVu SANS rather than Sans Mono, and the
 * pen moves by each glyph's real advance instead of by the cell.
 *
 * THE CONSOLE DOES NOT USE THIS AND MUST NOT. A terminal is a grid; text that
 * reflows under it would break fb_scroll, the text box, and the cursor. Only
 * labels change - which is exactly the split desktop-look.md asks for.
 */
/* The atlas itself, not a declaration of it. See the comment at the top of the
 * generated file for why it rides along with fb.c rather than being its own
 * translation unit. */
#include "font_prop.inc"

/* ---- the type scale, v10 SS6.8 --------------------------------------------
 * Three sizes and two weights, because that is what the prototype's source
 * asks for: its font-size declarations cluster at 9.5-13px for body, 15 and 19
 * for headings, and font-weight:700 appears fourteen times against a 400
 * default. Six atlases, generated together so they cannot drift.
 *
 * A ROLE, NOT A SIZE, is what callers ask for. TEXT_BODY at ui() 1 and at
 * ui() 2 are different pixel heights, and every caller that had to know which
 * would be a caller that breaks on a screen nobody tested.
 *
 * AT ui() == 1 THE SCALE COLLAPSES TO TWO STEPS, and that is deliberate rather
 * than an oversight. The console cell there is 8x16, so a caption below 16px
 * would be smaller than the smallest atlas that exists - and three distinct
 * sizes inside 16 pixels is not a type scale, it is three illegible sizes.
 * Caption and body share the 16px atlas; the title steps up to 24.
 */
#define TEXT_CAPTION 0
#define TEXT_BODY    1
#define TEXT_TITLE   2

#define TEXT_REGULAR 0
#define TEXT_BOLD    1

/* Follow the console's cell, the way fb_glyph_aa does, so UI text scales with
 * everything else rather than staying 8px tall on a 1920-wide desktop. */
static int prop_big(void) { return cell_w != GLYPH_W; }

/* role -> cell height IN PIXELS, following the UI scale rather than the font
 * cell. At ui 2 these land exactly on the three generated atlases (16/24/32);
 * at ui 1, 3 and 4 they land between them and the glyph is resampled, which is
 * what stops text staying 32px tall on a 4K panel while everything around it
 * grew. */
static const unsigned char role_base[3] = { 8, 12, 16 };

static int prop_cell(int role)
{
    if (role < 0) role = 0;
    if (role > TEXT_TITLE) role = TEXT_TITLE;
    int h = role_base[role] * ui_scale;
    if (h < 12) h = 12;                /* below this nothing is legible */
    return h;
}

/* the generated atlas nearest a wanted height: 16, 24 or 32 */
static int prop_atlas_cell(int want)
{
    if (want <= 20) return 16;
    if (want <= 28) return 24;
    return 32;
}

/* The atlas, its stride, its advances and its ink widths, all picked together.
 * Four parallel switches is how one of them ends up disagreeing with the other
 * three and a glyph gets indexed out of a neighbouring array. */
static const unsigned char *prop_atlas(int cell, int weight, int *stride,
                                       const unsigned char **adv,
                                       const unsigned char **ink)
{
    if (cell <= 16) {
        if (weight) { *stride = PROP16B_W; *adv = prop16b_adv; *ink = prop16b_ink;
                      return &prop16b[0][0][0]; }
        *stride = PROP16_W; *adv = prop16_adv; *ink = prop16_ink;
        return &prop16[0][0][0];
    }
    if (cell <= 24) {
        if (weight) { *stride = PROP24B_W; *adv = prop24b_adv; *ink = prop24b_ink;
                      return &prop24b[0][0][0]; }
        *stride = PROP24_W; *adv = prop24_adv; *ink = prop24_ink;
        return &prop24[0][0][0];
    }
    if (weight) { *stride = PROP32B_W; *adv = prop32b_adv; *ink = prop32b_ink;
                  return &prop32b[0][0][0]; }
    *stride = PROP32_W; *adv = prop32_adv; *ink = prop32_ink;
    return &prop32[0][0][0];
}

/* ---- the full form -------------------------------------------------------- */
int fb_text_role_h(int role) { return prop_cell(role); }

int fb_text_role_w(const char *s, int role, int weight)
{
    int want = prop_cell(role), cell = prop_atlas_cell(want);
    int stride;
    const unsigned char *adv, *ink;
    prop_atlas(cell, weight, &stride, &adv, &ink);
    int w = 0;
    while (*s) {
        char c = *s++;
        if (c < FONT_FIRST || c > FONT_LAST) c = '?';
        /* the advance scales with the glyph, or a resampled string measures
         * one width and draws another */
        w += adv[(int)c - FONT_FIRST] * want / cell;
    }
    return w;
}

void fb_text_role(int px, int py, const char *s, unsigned int fg,
                  int role, int weight)
{
    int want = prop_cell(role), cell = prop_atlas_cell(want);
    int stride;
    const unsigned char *adv, *ink;
    const unsigned char *atlas = prop_atlas(cell, weight, &stride, &adv, &ink);

    while (*s) {
        char c = *s++;
        if (c < FONT_FIRST || c > FONT_LAST) c = '?';
        int i = (int)c - FONT_FIRST;
        const unsigned char *g = atlas + (unsigned long)i * cell * stride;
        if (want == cell) {
            /* the common case, and the fast one: only the columns that can
             * hold ink, not the whole cell - a comma's ink is 4px inside a
             * 30px cell at title size */
            blend_cov_s(px, py, g, ink[i], cell, stride, fg);
        } else {
            /* No atlas at this size, so resample one. blend_cov_scaled wants a
             * tight bitmap, and the atlas row is `stride` wide - so the ink
             * width is passed as the source width and the stride is honoured
             * by stepping rows, which is exactly what the ink-width form does
             * for the unscaled case. */
            blend_cov_scaled_s(px, py, g, ink[i], cell, stride,
                               ink[i] * want / cell, want, fg);
        }
        px += adv[i] * want / cell;
    }
}

/* ---- the old two-argument form, which is TEXT_BODY ------------------------
 * Kept because most callers do not want to make a typographic decision, and a
 * default that has to be spelled out at every call site is a default nobody
 * uses. */
int fb_text_prop_w(const char *s) { return fb_text_role_w(s, TEXT_BODY, TEXT_REGULAR); }
int fb_text_prop_h(void)          { return prop_cell(TEXT_BODY); }

void fb_text_prop(int px, int py, const char *s, unsigned int fg)
{
    fb_text_role(px, py, s, fg, TEXT_BODY, TEXT_REGULAR);
}

/* ---- 24x24 icons: the dock, the start menu, window titlebars -------------
 * icons24 is a coverage atlas exactly like the font atlases - one byte of
 * alpha per pixel, generated by gen_icons.py - so an icon is a SHAPE, not a
 * picture, and the caller picks the colour. Same read-back blend as
 * fb_glyph_aa: transparent where coverage is zero, so an icon sits on the
 * wallpaper or a titlebar gradient with no box around it. */
extern const unsigned char icons24[10][24][24];
/* the same ten icons RE-RASTERIZED at 48x48, not the 24x24 set scaled up.
 * gen_icons.py draws each size from the geometry at its own 4x supersample,
 * so this carries real anti-aliased edges. See the comment in fb_icon24. */
extern const unsigned char icons48[10][48][48];
#define ICON_N  10
#define ICON_W  24
#define ICON_H  24
#define ICON2_W 48
#define ICON2_H 48

/* Draw icon `n` at the current UI scale.
 *
 * This used to be `ic[y / sc][x / sc]` - nearest-neighbour, i.e. each source
 * pixel copied into an sc x sc block. `sc` is 2 on every screen 1400px or
 * wider, which is every screen actually used, so the pipeline was: draw clean
 * geometry at 96x96, box-filter it to 24x24, then throw away every one of
 * those anti-aliased edge pixels by blowing it back up to 48x48 in squares.
 * desktop-look.md called it the single most visible source of blockiness in
 * the desktop, and it was.
 *
 * Now each scale reads the atlas that was RASTERIZED for it. Only a scale
 * neither atlas covers (3x and up, which F4's fractional UI scale will want)
 * resamples - and it interpolates rather than copies, from the 48x48 set. */
void fb_icon24(int px, int py, int n, unsigned int fg)
{
    if ((unsigned)n >= ICON_N) return;
    int sc = cell_w / GLYPH_W;
    if (sc < 1) sc = 1;

    if (sc == 1) { blend_cov(px, py, &icons24[n][0][0], ICON_W,  ICON_H,  fg); return; }
    if (sc == 2) { blend_cov(px, py, &icons48[n][0][0], ICON2_W, ICON2_H, fg); return; }

    blend_cov_scaled(px, py, &icons48[n][0][0], ICON2_W, ICON2_H,
                     ICON_W * sc, ICON_H * sc, fg);
}
