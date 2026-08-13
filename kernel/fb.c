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
#define FONT_FIRST 32
#define FONT_LAST  126
#define GLYPH_W    8
#define GLYPH_H    16

static unsigned char *fb_base;
static unsigned int   fb_pitch;      /* bytes per scanline, NOT pixels */
static unsigned int   fb_w, fb_h;
static unsigned int   fb_bpp;        /* bits per pixel */
static int fb_cols, fb_rows;
static int fb_col, fb_row;
static unsigned int fb_fg = 0xAAAAAA, fb_bg = 0x000000;

int fb_active(void) { return fb_base != 0; }
int fb_get_cols(void) { return fb_cols; }
int fb_get_rows(void) { return fb_rows; }

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
    fb_cols  = (int)(width  / GLYPH_W);
    fb_rows  = (int)(height / GLYPH_H);
    fb_col   = 0;
    fb_row   = 0;
}

static void put_pixel(unsigned int x, unsigned int y, unsigned int rgb)
{
    if (x >= fb_w || y >= fb_h) return;
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
    for (int y = 0; y < GLYPH_H; y++)
        for (int x = 0; x < GLYPH_W; x++)
            put_pixel((unsigned)(cx * GLYPH_W + x), (unsigned)(cy * GLYPH_H + y), rgb);
}

static void draw_glyph(int cx, int cy, char c, unsigned int fg, unsigned int bg)
{
    if (c < FONT_FIRST || c > FONT_LAST) c = '?';
    const unsigned char *g = font8x16[(int)c - FONT_FIRST];
    for (int y = 0; y < GLYPH_H; y++) {
        unsigned char bits = g[y];
        for (int x = 0; x < GLYPH_W; x++)
            put_pixel((unsigned)(cx * GLYPH_W + x), (unsigned)(cy * GLYPH_H + y),
                      (bits & (0x80 >> x)) ? fg : bg);
    }
}

void fb_clear(void)
{
    for (unsigned int y = 0; y < fb_h; y++)
        for (unsigned int x = 0; x < fb_w; x++)
            put_pixel(x, y, fb_bg);
    fb_col = 0;
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

/* Scrolling a framebuffer means moving every byte of every scanline. It is
 * the slowest thing this console does, which is why the log region is kept
 * small rather than scrolling the whole screen. */
static void fb_scroll(int top, int bot)
{
    unsigned long rowbytes = (unsigned long)fb_pitch * GLYPH_H;
    unsigned char *dst = fb_base + (unsigned long)top * rowbytes;
    unsigned char *src = dst + rowbytes;
    unsigned long n = (unsigned long)(bot - top) * rowbytes;
    for (unsigned long i = 0; i < n; i++) dst[i] = src[i];
    for (int c = 0; c < fb_cols; c++) fill_cell(c, bot, fb_bg);
    fb_row = bot;
}

void fb_putc(char c, int log_top, int log_bot)
{
    if (c == '\n') { fb_col = 0; fb_row++; }
    else if (c == '\r') { fb_col = 0; }
    else if (c == '\b') { if (fb_col > 0) { fb_col--; fill_cell(fb_col, fb_row, fb_bg); } }
    else if (c == '\t') { fb_col = (fb_col + 8) & ~7; }
    else {
        draw_glyph(fb_col, fb_row, c, fb_fg, fb_bg);
        fb_col++;
    }
    if (fb_col >= fb_cols) { fb_col = 0; fb_row++; }
    if (fb_row > log_bot) fb_scroll(log_top, log_bot);
}

void fb_set_row(int r, int log_top, int log_bot)
{
    if (r < log_top) r = log_top;
    if (r > log_bot) r = log_bot;
    fb_row = r;
    fb_col = 0;
}

int fb_get_row(void) { return fb_row; }
