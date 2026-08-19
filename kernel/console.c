/* console.c - one console API over two very different screens.
 *
 * BIOS gives you a VGA text buffer at 0xB8000. UEFI gives you a linear
 * framebuffer and no text mode at all. Rather than ship two images, the
 * kernel asks GRUB for a graphics mode and decides HERE, at run time,
 * which screen it actually got.
 *
 * Everything above this - runtime_kernel.c, and therefore kernel.zl -
 * talks only to these functions and never learns which one it is on.
 */

/* Pointer-sized. NOT `unsigned long`, which is 4 bytes under buildefi.sh's
 * LLP64 target and 8 everywhere else - the whole hazard class this file's
 * multiboot pointer and fb_phys prototype below sit in. Same shape as fb.c's
 * fb_uptr and xhci.c's uptr. */
#if defined(ZL_64) || defined(__x86_64__)
typedef unsigned long long con_uptr;
#else
typedef unsigned int con_uptr;
#endif

/* vga.c - the BIOS text buffer */
void vga_clear(void);
void vga_putc(char c);
void vga_setcolor(unsigned char attr);
void vga_bar(int row, unsigned char attr);
void vga_at(int row, int col, const char *s, unsigned char attr);
void vga_set_row(int r);
int  vga_get_row(void);
int  vga_get_col(void);

/* fb.c - the UEFI framebuffer */
int  fb_active(void);
/* 64 bits for the SAME reason fb_setup is, and it was not until 2026-08-19:
 * the address coming IN had been widened end to end while the one going back
 * OUT still went through `unsigned long`, which is 32 bits under buildefi.sh. */
unsigned long long fb_phys(void);
/* 64 bits, on every target. See the note in fb_setup's definition - a
 * framebuffer address is a PHYSICAL address and has no business being sized by
 * whatever `long` happens to mean on the current ABI. */
void fb_setup(unsigned long long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
void fb_clear(void);
void fb_putc(char c, int log_top, int log_bot);
void fb_setcolor(unsigned char attr);
void fb_bar(int row, unsigned char attr);
void fb_at(int row, int col, const char *s, unsigned char attr);
void fb_set_row(int r, int log_top, int log_bot);
int  fb_get_row(void);
int  fb_get_cols(void);
int  fb_get_rows(void);
int  fb_get_col(void);
unsigned int fb_pxw(void);
unsigned int fb_pxh(void);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_gradient(int x, int y, int w, int h, unsigned int top, unsigned int bot);
void fb_text_scaled(int px, int py, const char *s, int scale, unsigned int fg);
unsigned int fb_attr_rgb(unsigned char attr);
void fb_cursor(int row, int col, int on, unsigned char attr);
void fb_box(int x, int y, int w, int h, unsigned int rgb);
void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb);
void fb_cursor_arrow(int x, int y, unsigned int fill, unsigned int edge);
unsigned int fb_get_px(int x, int y);
void fb_shade(int x, int y, int w, int h, int num, int den);
void fb_shadow(int x, int y, int w, int h, int off, int soft);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_text_aa(int px, int py, const char *s, unsigned int fg);
void fb_text_aa2x(int px, int py, const char *s, unsigned int fg);
void fb_glyph_aa(int px, int py, char c, unsigned int fg);
void fb_set_text_box(int c0, int c1);
void fb_cube(int cx, int cy, int size, int angle, unsigned int color);
void fb_pointer_show(int x, int y);
void fb_pointer_hide(void);
void fb_present(void);
void fb_icon24(int px, int py, int n, unsigned int fg);
int  fb_cell_w(void);
int  fb_cell_h(void);
void fb_cube_filled(int cx, int cy, int size, int angle, unsigned int base);
void fb3d_set_clip(int x0, int y0, int x1, int y1);

/* The log scrolls between the title bar and the status bar. On VGA that is
 * rows 1..23 of 25; on a framebuffer the screen is taller, so the bottom is
 * worked out from the real row count. */
static int log_top = 1;
static int log_bot = 23;
static int status_row = 24;

int console_status_row(void) { return status_row; }

/* Let zl reserve rows for a graphical header/footer: the log scrolls only
 * between top and bot, so a tall logo band or a status strip is not scrolled
 * away. */
void console_set_region(int top, int bot) { log_top = top; log_bot = bot; }

/* Which screen did we actually get, and how big is it? The boot log should
 * say what is true, not what was hoped for - claiming "VGA text 80x25"
 * while rendering into a UEFI framebuffer is exactly the sort of
 * comfortable lie that hides a broken assumption later. */
int console_kind(void) { return fb_active() ? 1 : 0; }      /* 0 VGA, 1 framebuffer */

/* Real video memory for whichever console is up. In a linear framebuffer mode
 * the legacy text window at 0xB8000 is not decoded to anything, so a write
 * there is dropped and the read back returns 0xFF - which is exactly what the
 * poke/peek demo reported on the desktop before this existed. */
unsigned long long console_vram(void) { return fb_active() ? fb_phys() : 0xB8000ULL; }
int console_cols(void) { return fb_active() ? fb_get_cols() : 80; }
/* the console cell size in pixels - zl needs it to turn a window rect into a
 * text box, and it is no longer always 8x16 */
int console_cell_w(void) { return fb_active() ? fb_cell_w() : 8; }
/* The LAYOUT scale, which is no longer the font cell - see fb.c. On the VGA
 * text path there are no pixels to scale, so it is 1. */
int fb_ui_scale(void);
int console_ui_scale(void) { return fb_active() ? fb_ui_scale() : 1; }
int fb_ui_scale_q8(void);
int console_ui_scale_q8(void) { return fb_active() ? fb_ui_scale_q8() : 256; }
int console_cell_h(void) { return fb_active() ? fb_cell_h() : 16; }
int console_rows(void) { return fb_active() ? fb_get_rows() : 25; }

/* WHERE VIDEO MEMORY ACTUALLY IS. The `m` command pokes a byte into it and
 * reads it back, which is a real proof that we own the display - but only if
 * it pokes the right memory. Poking 0xB8000 unconditionally read back 255 on
 * the desktop, because in a framebuffer mode nothing decodes the VGA text
 * buffer and the write went nowhere. So whichever console is up answers with
 * its own memory: the linear framebuffer, or the text buffer at 0xB8000. */

/* Which loader booted us. GRUB always passes a non-null multiboot info
 * pointer; our own raw_boot.asm hands over with ebx = 0. So a null mb_addr
 * means we came up on our own bootloader, and the boot log should say so
 * instead of claiming a multiboot handoff that did not happen. */
static int loaded_by_multiboot = 0;
int console_loader(void) { return loaded_by_multiboot; }    /* 1 = GRUB/multiboot, 0 = ours */

/* The multiboot info structure GRUB fills in. Only the fields this kernel
 * reads are named; the offsets are fixed by the multiboot 1 spec. */
struct mb_info {
    unsigned int flags;
    unsigned int mem_lower, mem_upper;
    unsigned int boot_device;
    unsigned int cmdline;
    unsigned int mods_count, mods_addr;
    unsigned int syms[4];
    unsigned int mmap_length, mmap_addr;
    unsigned int drives_length, drives_addr;
    unsigned int config_table;
    unsigned int boot_loader_name;
    unsigned int apm_table;
    unsigned int vbe_control_info, vbe_mode_info;
    unsigned short vbe_mode, vbe_interface_seg, vbe_interface_off, vbe_interface_len;
    unsigned long long framebuffer_addr;
    unsigned int  framebuffer_pitch;
    unsigned int  framebuffer_width;
    unsigned int  framebuffer_height;
    unsigned char framebuffer_bpp;
    unsigned char framebuffer_type;
};

#define MB_FLAG_FRAMEBUFFER (1u << 12)
#define FB_TYPE_RGB 1

void console_init(unsigned long mb_addr)
{
    loaded_by_multiboot = (mb_addr != 0);
    struct mb_info *mb = (struct mb_info *)(con_uptr)mb_addr;

    /* Take the framebuffer only if GRUB says it gave us a packed-RGB one.
     * type 2 is EGA text living behind the same fields, and drawing pixels
     * into that would paint garbage over a perfectly good text console. */
    if (mb && (mb->flags & MB_FLAG_FRAMEBUFFER) &&
        mb->framebuffer_type == FB_TYPE_RGB) {
        fb_setup(mb->framebuffer_addr,
                 mb->framebuffer_pitch,
                 mb->framebuffer_width,
                 mb->framebuffer_height,
                 mb->framebuffer_bpp);
    }

    if (fb_active()) {
        int rows = fb_get_rows();
        log_top    = 1;
        log_bot    = rows - 2;
        status_row = rows - 1;
        fb_clear();
    } else {
        log_top = 1; log_bot = 23; status_row = 24;
        vga_clear();
    }
}

/* The path where OUR bootloader did the whole job: raw_boot.asm asked the card
 * for a linear framebuffer through VBE itself and handed the details straight
 * over, so there is no multiboot info and no GRUB anywhere in the story. */
void console_init_fb(unsigned long long addr, unsigned int pitch, unsigned int width,
                     unsigned int height, unsigned int bpp);

/* Called only from the UEFI entry: the firmware loaded us directly, so there
 * was no bootloader of any kind in between - not GRUB, not ours. */
void console_init_efi(unsigned long long addr, unsigned int pitch, unsigned int width,
                      unsigned int height, unsigned int bpp)
{
    /* CHANGING ONE OF THESE IS WORSE THAN CHANGING NONE: the declaration and
     * the definition would then disagree about the size of a register
     * argument, which is undefined behaviour rather than a diagnostic. This
     * fails the build if it is ever narrowed back below pointer size, which is
     * exactly the shape T-11 had on LLP64. */
    _Static_assert(sizeof(addr) >= sizeof(void *),
                   "console_init_efi's address must be able to hold a pointer");
    console_init_fb(addr, pitch, width, height, bpp);
    loaded_by_multiboot = 2;            /* 2 = booted as a UEFI application */
}

void console_init_fb(unsigned long long addr, unsigned int pitch, unsigned int width,
                     unsigned int height, unsigned int bpp)
{
    _Static_assert(sizeof(addr) >= sizeof(void *),
                   "console_init_fb's address must be able to hold a pointer");
    loaded_by_multiboot = 0;
    fb_setup(addr, pitch, width, height, (unsigned char)bpp);

    if (fb_active()) {
        int rows = fb_get_rows();
        log_top    = 1;
        log_bot    = rows - 2;
        status_row = rows - 1;
        fb_clear();
    } else {
        log_top = 1; log_bot = 23; status_row = 24;
        vga_clear();
    }
}

/* A RUNTIME MODE SWITCH - the thing a real OS can do and a fixed-at-boot
 * hobby OS cannot. Our own BGA driver reprograms the display controller,
 * then the console is re-pointed at the new geometry and the framebuffer
 * address the card reports through PCI. Nothing here goes near the BIOS. */
int bga_set_mode(int w, int h, int bpp);
unsigned int bga_framebuffer(void);
int bga_get_pitch(void);

int console_set_res(int w, int h)
{
    /* A mode switch must not rewrite history: who loaded us is still true
     * afterwards, and console_init_fb would otherwise clear the flag and make
     * the boot log claim we came up on our own bootloader when we did not. */
    int was_multiboot = loaded_by_multiboot;
    if (!bga_set_mode(w, h, 32)) return 0;
    unsigned int lfb = bga_framebuffer();
    if (!lfb) return 0;
    /* ask the card what stride it settled on rather than assuming w*4 */
    int pitch = bga_get_pitch();
    if (pitch < w * 4) pitch = w * 4;
    console_init_fb((unsigned long long)lfb, (unsigned int)pitch,
                    (unsigned int)w, (unsigned int)h, 32);
    loaded_by_multiboot = was_multiboot;
    return 1;
}

void console_clear(void)                 { if (fb_active()) fb_clear(); else vga_clear(); }
/* Push whatever has been drawn since the last call out to the card. Cheap when
 * nothing changed, and only ever copies the rectangle that actually moved. */
void console_present(void) { if (fb_active()) fb_present(); }

/* ---- muting, for when something else owns the screen -----------------------
 * The console draws text at ITS cursor, inside ITS scrolling region, and knows
 * nothing about windows. That is correct while it is the whole machine and
 * wrong the moment a compositor is repainting from damage: `help` typed into
 * the shell window printed the help text twice - once into the window, from
 * term.c's scrollback, and once as raw console text scrolling across the
 * desktop underneath it, leaving a black band where the region had scrolled.
 *
 * Muting stops the PIXELS only. zl_putc still tees every character into
 * term.c's scrollback and still writes it to COM1, so the window redraws from
 * a complete transcript and every automated gate reads exactly what it read
 * before. That split is the whole point: the serial log must not depend on who
 * owns the framebuffer. */
/* desktop/exec-track arrived with the same feature under a second name -
 * console_quiet() over `static int quiet` - and added the reason from its own
 * side: the console's text region is set once at boot from the STATIC
 * desktop's terminal frame, so under the compositor the shell lives in a
 * wm_open window somewhere else entirely and every character printed by a
 * command in that window landed as an opaque glyph on the wallpaper, at
 * coordinates from a layout no longer on screen - plus an fb_present() per
 * line, fighting wm_frame for the blit.
 *
 * One flag, one function. Both builtin names existed and kernel.zl called
 * BOTH, so the two could disagree about who owns the screen. */
static int con_muted = 0;

void console_mute(int on) { con_muted = on ? 1 : 0; }
int  console_muted(void)  { return con_muted; }

/* A FATAL ERROR IS NEVER QUIET. kfatal prints through zl_putc, whose console
 * sink this mutes - so a zl runtime fault during a compositor session would
 * halt the machine having drawn nothing, with the diagnostic sitting in a
 * scrollback nobody will ever repaint. That is the silent-refusal bug class
 * this project is most expensive about, reintroduced by the very flag that was
 * added to stop the console scribbling.
 *
 * console_unmute() is what the fatal path calls: it drops the mute for good,
 * so everything after it reaches the screen. apps-in-windows called this
 * console_unquiet() over its own `quiet` flag; one flag, one name. */
void console_unmute(void) { con_muted = 0; }

void console_putc(char c)
{
    if (con_muted) return;
    if (!fb_active()) { vga_putc(c); return; }
    fb_putc(c, log_top, log_bot);
    /* flush a line at a time so the boot log still streams live, without
     * paying for a blit on every single character */
    if (c == '\n') fb_present();
}
void console_setcolor(unsigned char a)   { if (fb_active()) fb_setcolor(a); else vga_setcolor(a); }
void console_bar(int r, unsigned char a) { if (fb_active()) fb_bar(r, a); else vga_bar(r, a); }
void console_at(int r, int c, const char *s, unsigned char a)
                                         { if (fb_active()) fb_at(r, c, s, a); else vga_at(r, c, s, a); }
void console_set_row(int r)              { if (fb_active()) fb_set_row(r, log_top, log_bot); else vga_set_row(r); }
int  console_get_row(void)               { return fb_active() ? fb_get_row() : vga_get_row(); }
int  console_get_col(void)               { return fb_active() ? fb_get_col() : vga_get_col(); }

/* ---- rich graphics, exposed to zl. All no-op on the VGA text path, where
 * there are no pixels; the boot still reads fine without them. ---- */
int  console_pxw(void) { return fb_active() ? (int)fb_pxw() : 0; }
int  console_pxh(void) { return fb_active() ? (int)fb_pxh() : 0; }

void console_fill(int x, int y, int w, int h, unsigned char attr)
{ if (fb_active()) fb_fill_px(x, y, w, h, fb_attr_rgb(attr)); }

void console_gradient(int x, int y, int w, int h, unsigned char a_top, unsigned char a_bot)
{ if (fb_active()) fb_gradient(x, y, w, h, fb_attr_rgb(a_top), fb_attr_rgb(a_bot)); }

/* true-colour versions: zl passes a packed 0xRRGGBB it computes as
 * r*65536 + g*256 + b, escaping the 16-colour VGA palette for the modern
 * gradients, shadows and wallpaper a desktop needs. */
/* the v10 primitives: translucency, the two gradient shapes, the blur cache */
void fb_fill_blend(int x, int y, int w, int h, unsigned int rgb, int a);
void fb_rrect_blend(int x, int y, int w, int h, int r, unsigned int rgb, int a);
void fb_glow(int cx, int cy, int rx, int ry, unsigned int rgb, int a_in, int a_out, int stop);
void fb_wedge(int cx, int cy, unsigned int rgb, int a0, int f, int m, int e);
int  fb_blur_cache(int x, int y, int w, int h, int radius);
void fb_blur_paint(int slot, int x, int y);
void fb_blur_free_all(void);

/* THE PROPORTIONAL PATH, exposed to zl. Everything kernel.zl draws - the dock,
 * the start menu, the tray, the About card, the System Monitor - went through
 * console_text_aa, which is DejaVu Sans MONO. wm.c's window titles were the
 * only proportional text on screen. Uniform advance is the strongest "this is
 * a terminal" signal there is (desktop-look.md item 4) and it was being applied
 * to every label that is not a terminal. */
void fb_text_role(int px, int py, const char *s, unsigned int fg, int role, int weight);
int  fb_text_role_w(const char *s, int role, int weight);
int  fb_text_role_h(int role);

void console_text_role(int x, int y, const char *s, unsigned int rgb, int role, int weight)
{ if (fb_active()) fb_text_role(x, y, s, rgb, role, weight); }
int  console_text_role_w(const char *s, int role, int weight)
{ return fb_active() ? fb_text_role_w(s, role, weight) : 0; }
int  console_text_role_h(int role)
{ return fb_active() ? fb_text_role_h(role) : 16; }

/* Numbers, the same way. zl has no string values, so a number cannot be
 * formatted into one - it is rendered digit by digit here, exactly as
 * console_num_aa does for the mono path. */
void console_num_role(int x, int y, long v, unsigned int rgb, int role, int weight)
{
    if (!fb_active()) return;
    char b[24];
    int n = 0, neg = v < 0;
    unsigned long u = neg ? (unsigned long)(-v) : (unsigned long)v;
    if (!u) b[n++] = '0';
    while (u) { b[n++] = (char)('0' + u % 10); u /= 10; }
    if (neg) b[n++] = '-';
    char out[24];
    for (int i = 0; i < n; i++) out[i] = b[n - 1 - i];
    out[n] = 0;
    fb_text_role(x, y, out, rgb, role, weight);
}

void console_blend(int x, int y, int w, int h, unsigned int rgb, int a)
{ if (fb_active()) fb_fill_blend(x, y, w, h, rgb, a); }
void console_rrblend(int x, int y, int w, int h, int r, unsigned int rgb, int a)
{ if (fb_active()) fb_rrect_blend(x, y, w, h, r, rgb, a); }
void console_glow(int cx, int cy, int rx, int ry, unsigned int rgb, int ai, int ao, int stop)
{ if (fb_active()) fb_glow(cx, cy, rx, ry, rgb, ai, ao, stop); }
int  console_wall_save(void);
void console_wall_paint(int x, int y, int w, int h);
void console_wedge(int cx, int cy, unsigned int rgb, int a0, int f, int m, int e)
{ if (fb_active()) fb_wedge(cx, cy, rgb, a0, f, m, e); }
int  console_blur(int x, int y, int w, int h, int r)
{ return fb_active() ? fb_blur_cache(x, y, w, h, r) : -1; }
void console_blur_paint(int slot, int x, int y)
{ if (fb_active()) fb_blur_paint(slot, x, y); }
void console_blur_free(void) { if (fb_active()) fb_blur_free_all(); }

/* the wallpaper cache: zl draws the wallpaper it wants once, then saves it,
 * and every repaint after that is a blit */
int  fb_wall_save(void);
void fb_wall_paint(int x, int y, int w, int h);
int  fb_wall_ok(void);
int  console_wall_save(void)  { return fb_active() ? fb_wall_save() : 0; }
void console_wall_paint(int x, int y, int w, int h)
{ if (fb_active()) fb_wall_paint(x, y, w, h); }
int  console_wall_ok(void)    { return fb_active() ? fb_wall_ok() : 0; }

void fb_rrect_grad_top(int x, int y, int w, int h, int r, unsigned int t, unsigned int b);
void console_gradtop(int x, int y, int w, int h, int r, unsigned int t, unsigned int b)
{ if (fb_active()) fb_rrect_grad_top(x, y, w, h, r, t, b); }

void console_fill_rgb(int x, int y, int w, int h, unsigned int rgb)
{ if (fb_active()) fb_fill_px(x, y, w, h, rgb); }

void console_gradient_rgb(int x, int y, int w, int h, unsigned int top, unsigned int bot)
{ if (fb_active()) fb_gradient(x, y, w, h, top, bot); }

/* text drawn in a true-colour foreground at a pixel position, transparent
 * background - so labels sit on a gradient without a black box behind them */
void console_text_rgb(int px, int py, const char *s, unsigned int rgb)
{ if (fb_active()) fb_text_scaled(px, py, s, 1, rgb); }

/* read a pixel back (0 on the text console, which has no pixels) */
int console_get_px(int x, int y) { return fb_active() ? (int)fb_get_px(x, y) : 0; }

/* darken a region: one shadow pass, num/den of current brightness */
void console_shade(int x, int y, int w, int h, int num, int den)
{ if (fb_active()) fb_shade(x, y, w, h, num, den); }

/* a soft drop shadow offset down-right of a window footprint */
void console_shadow(int x, int y, int w, int h, int off, int soft)
{ if (fb_active()) fb_shadow(x, y, w, h, off, soft); }

/* a filled rounded rectangle - window chrome */
void console_rrect(int x, int y, int w, int h, int r, unsigned int rgb)
{ if (fb_active()) fb_rrect(x, y, w, h, r, rgb); }

/* anti-aliased text over whatever is already drawn (gradient/wallpaper).
 * On VGA text there are no pixels, so fall back to nothing - callers that
 * need a text-console label use console_at instead. */
void console_text_aa(int px, int py, const char *s, unsigned int rgb)
{ if (fb_active()) fb_text_aa(px, py, s, rgb); }

/* double-size anti-aliased text for titles/headers */
void console_text_aa2x(int px, int py, const char *s, unsigned int rgb)
{ if (fb_active()) fb_text_aa2x(px, py, s, rgb); }

/* one AA glyph by character code, at a pixel position - lets zl draw dynamic
 * strings it reads a byte at a time (the CPUID brand) inside a window */
void console_char_aa(int px, int py, int code, unsigned int rgb)
{ if (fb_active()) fb_glyph_aa(px, py, (char)code, rgb); }

/* confine the flowing console to a column band, so the shell can live inside a
 * floating terminal window. c0..c1 in cells; no-op on the VGA text path. */
void console_set_text_box(int c0, int c1)
{ if (fb_active()) fb_set_text_box(c0, c1); }

/* the software-rendered 3D cube, at true-colour */
void console_cube(int cx, int cy, int size, int angle, unsigned int rgb)
{ if (fb_active()) fb_cube(cx, cy, size, angle, rgb); }

/* a 24x24 icon, blended - the dock and menus draw with these */
void console_icon(int px, int py, int n, unsigned int rgb)
{ if (fb_active()) fb_icon24(px, py, n, rgb); }

/* the live mouse pointer sprite */
void console_pointer_show(int x, int y) { if (fb_active()) fb_pointer_show(x, y); }
void console_pointer_hide(void)         { if (fb_active()) fb_pointer_hide(); }

/* The window-drag backing store used to be wrapped here - bg_snapshot,
 * bg_restore, grab, stamp. Deleted with fb.c's implementation (C4): dragging
 * is the compositor's damage-based repaint now and needs no backing store. */

/* the solid, flat-shaded, back-face-culled cube */
void console_cube_filled(int cx, int cy, int size, int angle, unsigned int rgb)
{ if (fb_active()) fb_cube_filled(cx, cy, size, angle, rgb); }

/* clip box for the 3D cube - set to a window's interior so it can't overdraw */
void console_cube_clip(int x0, int y0, int x1, int y1)
{ if (fb_active()) fb3d_set_clip(x0, y0, x1, y1); }


void console_logo(int px, int py, const char *s, int scale, unsigned char attr)
{ if (fb_active()) fb_text_scaled(px, py, s, scale, fb_attr_rgb(attr)); }

/* the software cursor - framebuffer draws a block; VGA has its own hardware
 * cursor already positioned by vga_putc, so nothing to do there. */
void console_cursor(int row, int col, int on, unsigned char attr)
{ if (fb_active()) fb_cursor(row, col, on, attr); }

void console_box(int x, int y, int w, int h, unsigned char attr)
{ if (fb_active()) fb_box(x, y, w, h, fb_attr_rgb(attr)); }

void console_line(int x0, int y0, int x1, int y1, unsigned char attr)
{ if (fb_active()) fb_line(x0, y0, x1, y1, fb_attr_rgb(attr)); }

void console_mouse_cursor(int x, int y, unsigned char fill, unsigned char edge)
{ if (fb_active()) fb_cursor_arrow(x, y, fb_attr_rgb(fill), fb_attr_rgb(edge)); }

/* draw a decimal number at an exact cell - the kernel subset has no str(),
 * so a live clock in the status bar needs this in C. */
void console_at_num(int row, int col, long n, unsigned char attr)
{
    char tmp[16], out[18];
    int t = 0, i = 0, neg = (n < 0);
    unsigned long u = neg ? (unsigned long)(-(n + 1)) + 1UL : (unsigned long)n;
    if (u == 0) tmp[t++] = '0';
    while (u) { tmp[t++] = (char)('0' + (u % 10)); u /= 10; }
    if (neg) out[i++] = '-';
    while (t) out[i++] = tmp[--t];
    out[i] = 0;
    console_at(row, col, out, attr);
}

/* Print a number in HEX to the console and the serial line. Every register,
 * address and device ID in a driver is quoted in hex in the documentation, so
 * reading them back in decimal makes them impossible to check against the
 * manual. */
void zl_putc_pub(char c);
void console_puthex(unsigned long v, int digits)
{
    const char *d = "0123456789ABCDEF";
    if (digits < 1) digits = 1;
    if (digits > 16) digits = 16;
    for (int i = digits - 1; i >= 0; i--)
        zl_putc_pub(d[(v >> (i * 4)) & 0xF]);
}

/* the same decimal number, but anti-aliased at a pixel position and blended
 * over whatever is there - so the dock clock sits on the gradient with no
 * solid box behind it, matching the AA labels around it. */
void console_num_aa(int px, int py, long n, unsigned int rgb)
{
    char tmp[16], out[18];
    int t = 0, i = 0, neg = (n < 0);
    unsigned long u = neg ? (unsigned long)(-(n + 1)) + 1UL : (unsigned long)n;
    if (u == 0) tmp[t++] = '0';
    while (u) { tmp[t++] = (char)('0' + (u % 10)); u /= 10; }
    if (neg) out[i++] = '-';
    while (t) out[i++] = tmp[--t];
    out[i] = 0;
    if (fb_active()) fb_text_aa(px, py, out, rgb);
}
