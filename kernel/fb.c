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

/* ---- graphics primitives, framebuffer only ------------------------------
 * The UEFI framebuffer is raw RGB pixels, so the console can draw more than
 * text: filled bands, panel borders, and text scaled up for a real logo.
 * None of this exists on the VGA text path - a text grid has no pixels - so
 * console.c makes each a no-op there and the boot still reads fine. */

unsigned int fb_pxw(void) { return fb_w; }
unsigned int fb_pxh(void) { return fb_h; }

/* fill a pixel rectangle with an RGB colour (clipped to the screen) */
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb)
{
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++)
            put_pixel((unsigned)xx, (unsigned)yy, rgb);
}

/* a vertical gradient band - top colour fading to bottom colour. A flat bar
 * reads as "text mode"; a gradient reads as "a real UI". */
void fb_gradient(int x, int y, int w, int h, unsigned int top, unsigned int bot)
{
    if (h <= 0) return;
    int tr = (top >> 16) & 0xFF, tg = (top >> 8) & 0xFF, tb = top & 0xFF;
    int br = (bot >> 16) & 0xFF, bg = (bot >> 8) & 0xFF, bb = bot & 0xFF;
    for (int i = 0; i < h; i++) {
        int r = tr + (br - tr) * i / h;
        int g = tg + (bg - tg) * i / h;
        int b = tb + (bb - tb) * i / h;
        unsigned int c = ((unsigned)r << 16) | ((unsigned)g << 8) | (unsigned)b;
        fb_fill_px(x, y + i, w, 1, c);
    }
}

/* one glyph, scaled up by an integer factor, drawn at a pixel position */
void fb_glyph_scaled(int px, int py, char c, int scale, unsigned int fg)
{
    if (c < FONT_FIRST || c > FONT_LAST) c = '?';
    const unsigned char *g = font8x16[(int)c - FONT_FIRST];
    for (int y = 0; y < GLYPH_H; y++) {
        unsigned char bits = g[y];
        for (int x = 0; x < GLYPH_W; x++)
            if (bits & (0x80 >> x))
                fb_fill_px(px + x * scale, py + y * scale, scale, scale, fg);
    }
}

/* a whole string, scaled - this is how the zlOS logo is drawn big */
void fb_text_scaled(int px, int py, const char *s, int scale, unsigned int fg)
{
    while (*s) { fb_glyph_scaled(px, py, *s++, scale, fg); px += GLYPH_W * scale; }
}

/* map a VGA attribute's foreground to RGB, so zl keeps using colour indices */
unsigned int fb_attr_rgb(unsigned char attr) { return vga_rgb[attr & 0x0F]; }

/* a rectangle OUTLINE - for panels, buttons and window frames */
void fb_box(int x, int y, int w, int h, unsigned int rgb)
{
    fb_fill_px(x, y, w, 1, rgb);
    fb_fill_px(x, y + h - 1, w, 1, rgb);
    fb_fill_px(x, y, 1, h, rgb);
    fb_fill_px(x + w - 1, y, 1, h, rgb);
}

/* a straight line, any angle - Bresenham, integer only, no floating point */
void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb)
{
    int dx = x1 - x0, dy = y1 - y0;
    int sx = dx < 0 ? -1 : 1, sy = dy < 0 ? -1 : 1;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    int err = (dx > dy ? dx : -dy) / 2, e2;
    for (;;) {
        put_pixel((unsigned)x0, (unsigned)y0, rgb);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
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
int fb_get_col(void) { return fb_col; }
