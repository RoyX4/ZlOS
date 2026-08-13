/* vga.c - the VGA text-mode console.
 *
 * 0xB8000 is an 80x25 grid of 2-byte cells: character, then attribute
 * (low nibble foreground, high nibble background). Writing there puts
 * glyphs on screen with no driver, no firmware call and no BIOS - the
 * hardware simply scans that memory 60 times a second.
 *
 * This is the console the kernel prints THROUGH. Serial stays wired up in
 * parallel so a headless test can still capture a transcript; the screen
 * is for a human, the serial line is for verify.sh.
 */
#define VGA_MEM   0xB8000
#define VGA_COLS  80
#define VGA_ROWS  25

/* Row 0 is the title bar and row 24 the status bar; the log scrolls
 * between them. Without this the chrome scrolls away with the text and the
 * screen stops looking like a console after twenty lines. */
#define LOG_TOP   1
#define LOG_BOT   23

/* the CRT controller - used to move the blinking hardware cursor */
#define CRTC_IDX  0x3D4
#define CRTC_DAT  0x3D5

void zl_outb(unsigned short port, unsigned char val);
unsigned char zl_inb(unsigned short port);

static volatile unsigned short *const vga = (volatile unsigned short *)VGA_MEM;
static int  vga_row = 0;
static int  vga_col = 0;
static unsigned char vga_attr = 0x07;      /* light grey on black */

static unsigned short cell(char c, unsigned char a)
{
    return (unsigned short)c | ((unsigned short)a << 8);
}

void vga_setcolor(unsigned char attr) { vga_attr = attr; }

void vga_clear(void)
{
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++) vga[i] = cell(' ', vga_attr);
    vga_row = 0;
    vga_col = 0;
}

/* Move the blinking cursor to match where we will write next. Without this
 * the cursor sits wherever the BIOS left it and the screen looks dead. */
static void vga_move_cursor(void)
{
    unsigned short pos = (unsigned short)(vga_row * VGA_COLS + vga_col);
    zl_outb(CRTC_IDX, 0x0F); zl_outb(CRTC_DAT, (unsigned char)(pos & 0xFF));
    zl_outb(CRTC_IDX, 0x0E); zl_outb(CRTC_DAT, (unsigned char)((pos >> 8) & 0xFF));
}

/* Copy every row up one and blank the last. A real console scrolls; one
 * that stops at the bottom of the screen looks broken immediately. */
static void vga_scroll(void)
{
    for (int r = LOG_TOP + 1; r <= LOG_BOT; r++)
        for (int c = 0; c < VGA_COLS; c++)
            vga[(r - 1) * VGA_COLS + c] = vga[r * VGA_COLS + c];

    for (int c = 0; c < VGA_COLS; c++)
        vga[LOG_BOT * VGA_COLS + c] = cell(' ', vga_attr);

    vga_row = LOG_BOT;
}

void vga_putc(char c)
{
    if (c == '\n') { vga_col = 0; vga_row++; }
    else if (c == '\r') { vga_col = 0; }
    else if (c == '\b') { if (vga_col > 0) { vga_col--; vga[vga_row * VGA_COLS + vga_col] = cell(' ', vga_attr); } }
    else if (c == '\t') { vga_col = (vga_col + 8) & ~7; }
    else {
        vga[vga_row * VGA_COLS + vga_col] = cell(c, vga_attr);
        vga_col++;
    }

    if (vga_col >= VGA_COLS) { vga_col = 0; vga_row++; }
    if (vga_row > LOG_BOT) vga_scroll();

    vga_move_cursor();
}

/* Draw a full-width bar of `attr` at `row` - used for the title and status
 * lines, which is most of what makes a text screen read as "an OS". */
void vga_bar(int row, unsigned char attr)
{
    if (row < 0 || row >= VGA_ROWS) return;
    for (int c = 0; c < VGA_COLS; c++) vga[row * VGA_COLS + c] = cell(' ', attr);
}

/* Write a string at an exact cell without disturbing the cursor - for the
 * title and status bars, which must not scroll with the log. */
void vga_at(int row, int col, const char *s, unsigned char attr)
{
    if (row < 0 || row >= VGA_ROWS) return;
    while (*s && col < VGA_COLS) {
        vga[row * VGA_COLS + col] = cell(*s++, attr);
        col++;
    }
}

int vga_get_row(void) { return vga_row; }
int vga_get_col(void) { return vga_col; }

void vga_set_row(int r)
{
    if (r < LOG_TOP) r = LOG_TOP;
    if (r > LOG_BOT) r = LOG_BOT;
    vga_row = r;
    vga_col = 0;
    vga_move_cursor();
}
