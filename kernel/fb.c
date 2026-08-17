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

unsigned int fb_get_px(int x, int y);   /* defined below; used by the AA text path */
void idt_set_pointer_bounds(int w, int h);   /* the mouse clamp, pushed not pulled */
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);  /* the fast row fill */

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
#define BACK_MAX (1920 * 1200)   /* covers 1080p and 16:10 1920x1200 */
static unsigned int *back = (unsigned int *)0x0C000000;   /* 192 MiB scratch */
static int back_on = 0;
static int dx0, dy0, dx1, dy1, dirty;

static void mark(int x, int y)
{
    if (!dirty) { dx0 = x; dy0 = y; dx1 = x + 1; dy1 = y + 1; dirty = 1; return; }
    if (x < dx0) dx0 = x;
    if (y < dy0) dy0 = y;
    if (x + 1 > dx1) dx1 = x + 1;
    if (y + 1 > dy1) dy1 = y + 1;
}

/* copy the dirty box out to the card, then forget it */
void fb_present(void)
{
    if (!back_on || !dirty) return;
    int x0 = dx0, y0 = dy0, x1 = dx1, y1 = dy1;
    dirty = 0;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > (int)fb_w) x1 = (int)fb_w;
    if (y1 > (int)fb_h) y1 = (int)fb_h;
    if (x0 >= x1 || y0 >= y1) return;
    int bpx = (int)(fb_bpp / 8);
    for (int y = y0; y < y1; y++) {
        unsigned int  *src = back + (unsigned long)y * fb_w + x0;
        unsigned char *dst = fb_base + (unsigned long)y * fb_pitch + (unsigned long)x0 * bpx;
        if (bpx == 4) {
            unsigned int *d = (unsigned int *)dst;
            for (int x = x0; x < x1; x++) *d++ = *src++;
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

void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp)
{
    /* Only 32- and 24-bit packed pixel modes are handled. Anything else is
     * refused rather than drawn as garbage - a wrong depth paints noise and
     * looks like a crash. */
    if (!addr || (bpp != 32 && bpp != 24)) { fb_base = 0; return; }

    fb_base  = (unsigned char *)addr;
    fb_pitch = pitch;
    fb_w     = width;
    fb_h     = height;
    fb_bpp   = bpp;
    cell_w   = (width >= 1400) ? GLYPH_W * 2 : GLYPH_W;
    cell_h   = (width >= 1400) ? GLYPH_H * 2 : GLYPH_H;
    fb_cols  = (int)(width  / cell_w);
    fb_rows  = (int)(height / cell_h);
    fb_col   = 0;
    fb_row   = 0;
    tx0      = 0;
    tx1      = fb_cols - 1;      /* full width until zl opens a text box */

    /* Draw into RAM and blit, rather than drawing straight into the card.
     * Only refused if the mode is bigger than the buffer we reserved, in
     * which case everything still works - just slower, straight to VRAM. */
    back_on = ((int)(width * height) <= BACK_MAX);
    dirty   = 0;

    /* Tell the mouse ISR how big the screen is. It cannot ask: idt.c is built
     * -mgeneral-regs-only so that an interrupt never touches SSE, and calling
     * out of that file into one that can use XMM corrupts whatever the
     * interrupted code had in those registers. Every zl number is a double, so
     * that is the interpreter itself - it killed the 64-bit boot outright. */
    idt_set_pointer_bounds((int)width, (int)height);
}

static void put_pixel(unsigned int x, unsigned int y, unsigned int rgb)
{
    if (x >= fb_w || y >= fb_h) return;
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

/* a whole coverage bitmap, one destination pixel per source pixel */
static void blend_cov(int px, int py, const unsigned char *src,
                      int sw, int sh, unsigned int fg)
{
    for (int y = 0; y < sh; y++)
        for (int x = 0; x < sw; x++)
            blend_px(px + x, py + y, fg, src[(unsigned long)y * sw + x]);
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
static void blend_cov_scaled(int px, int py, const unsigned char *src,
                             int sw, int sh, int dw, int dh, unsigned int fg)
{
    if (dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0) return;
    if (dw == sw && dh == sh) { blend_cov(px, py, src, sw, sh, fg); return; }
    int lastx = (sw - 1) << 16, lasty = (sh - 1) << 16;
    for (int y = 0; y < dh; y++) {
        int syq = (2 * y + 1) * sh * 32768 / dh - 32768;
        if (syq < 0) syq = 0;
        if (syq > lasty) syq = lasty;
        int y0 = syq >> 16, fy = syq & 0xFFFF;
        int y1 = (y0 + 1 < sh) ? y0 + 1 : y0;
        const unsigned char *r0 = src + (unsigned long)y0 * sw;
        const unsigned char *r1 = src + (unsigned long)y1 * sw;
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
        for (int y = 0; y < ch; y++) {
            unsigned int *row = back + (unsigned long)(oy + y) * fb_w + ox;
            const unsigned char *sr = sub + (unsigned long)y * cw * 3;
            for (int x = 0; x < cw; x++) {
                int ar = sr[x * 3], ag = sr[x * 3 + 1], ab = sr[x * 3 + 2];
                if (!(ar | ag | ab)) { row[x] = bg; continue; }
                row[x] = blend_sub(bg, fg, ar, ag, ab);
            }
        }
        mark(ox, oy);
        mark(ox + cw - 1, oy + ch - 1);
        return;
    }

    if (back_on && ox >= 0 && oy >= 0 &&
        ox + cw <= (int)fb_w && oy + ch <= (int)fb_h) {
        for (int y = 0; y < ch; y++) {
            unsigned int *row = back + (unsigned long)(oy + y) * fb_w + ox;
            const unsigned char *cr = cov + (unsigned long)y * cw;
            for (int x = 0; x < cw; x++) {
                int a = cr[x];
                row[x] = (a <= 0) ? bg : (a >= 255) ? fg : blend_rgb(bg, fg, a);
            }
        }
        mark(ox, oy);
        mark(ox + cw - 1, oy + ch - 1);
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
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > (int)fb_w) x1 = (int)fb_w;
    if (y1 > (int)fb_h) y1 = (int)fb_h;
    if (x0 >= x1 || y0 >= y1) return;

    if (back_on) {
        for (int yy = y0; yy < y1; yy++) {
            unsigned int *row = back + (unsigned long)yy * fb_w + x0;
            for (int xx = x0; xx < x1; xx++) *row++ = rgb;
        }
        mark(x0, y0);
        mark(x1 - 1, y1 - 1);
        return;
    }
    for (int yy = y0; yy < y1; yy++)
        for (int xx = x0; xx < x1; xx++)
            put_pixel((unsigned)xx, (unsigned)yy, rgb);
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
        if ((unsigned)yy >= fb_h) continue;
        int j0 = 0, j1 = w;
        if (x + j0 < 0) j0 = -x;
        if (x + j1 > (int)fb_w) j1 = (int)fb_w - x;
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
            mark(x + j0, yy);
            mark(x + j1 - 1, yy);
        } else {
            for (int j = j0; j < j1; j++)
                put_pixel((unsigned)(x + j), (unsigned)yy, quad[j & 3]);
        }
    }
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

/* ---- backing store, for dragging windows without a GPU -------------------
 * The sticky-note trick: keep a snapshot of the desktop background (no
 * windows), grab the window being dragged as a bitmap "sprite", then each
 * frame restore the background where the window WAS and stamp the sprite where
 * it now IS. Two copies per frame, no re-rendering - so a window glides.
 * Buffers are sized for up to 1024x768 background and a 640x480 window; bigger
 * modes fall back gracefully (dragging just no-ops). */
#define BG_MAX (1920 * 1200)    /* the widest mode the bootloader will pick */
#define SP_MAX (640 * 480)
/* These are multi-megabyte, so they must NOT live in BSS (the linker would put
 * them right after the kernel, where they collided with the stack/framebuffer
 * and corrupted memory). Park them in free high RAM instead - QEMU/-m 256 has
 * 256 MiB, and 16-25 MiB is clear of the kernel image and the framebuffer. */
static unsigned int *bg_buf = (unsigned int *)0x08000000;   /* 128 MiB scratch */
static unsigned int *sp_buf = (unsigned int *)0x0A000000;   /* 160 MiB scratch */
static int bg_w = 0, bg_h = 0, bg_ok = 0;
static int sp_w = 0, sp_h = 0, sp_ok = 0;

/* snapshot the WHOLE screen as the background - call it after drawing the
 * wallpaper/header/dock but before the draggable windows go on top */
void fb_bg_snapshot(void)
{
    if ((int)(fb_w * fb_h) > BG_MAX) { bg_ok = 0; return; }
    bg_w = (int)fb_w; bg_h = (int)fb_h;
    for (int y = 0; y < bg_h; y++)
        for (int x = 0; x < bg_w; x++)
            bg_buf[y * bg_w + x] = fb_get_px(x, y);
    bg_ok = 1;
}

/* paint a rectangle of the saved background back onto the screen */
void fb_bg_restore(int x, int y, int w, int h)
{
    if (!bg_ok) return;
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++) {
            if ((unsigned)xx >= (unsigned)bg_w || (unsigned)yy >= (unsigned)bg_h) continue;
            put_pixel((unsigned)xx, (unsigned)yy, bg_buf[yy * bg_w + xx]);
        }
}

/* grab a screen rectangle into the sprite buffer (the window being lifted) */
void fb_grab(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0 || w * h > SP_MAX) { sp_ok = 0; return; }
    sp_w = w; sp_h = h;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            sp_buf[j * w + i] = fb_get_px(x + i, y + j);
    sp_ok = 1;
}

/* stamp the grabbed sprite down at a new position */
void fb_stamp(int x, int y)
{
    if (!sp_ok) return;
    for (int j = 0; j < sp_h; j++)
        for (int i = 0; i < sp_w; i++)
            put_pixel((unsigned)(x + i), (unsigned)(y + j), sp_buf[j * sp_w + i]);
}

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
        int x0 = c0 * cell_w;
        int npx = (c1 - c0 + 1) * cell_w;
        for (int y = y_top; y < y_bot; y++) {
            unsigned int *d = back + (unsigned long)y * fb_w + x0;
            unsigned int *s = back + (unsigned long)(y + cell_h) * fb_w + x0;
            for (int i = 0; i < npx; i++) d[i] = s[i];
        }
        mark(x0, y_top);
        mark(x0 + npx - 1, y_bot - 1);
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
