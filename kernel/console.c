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

/* vga.c - the BIOS text buffer */
void vga_clear(void);
void vga_putc(char c);
void vga_setcolor(unsigned char attr);
void vga_bar(int row, unsigned char attr);
void vga_at(int row, int col, const char *s, unsigned char attr);
void vga_set_row(int r);
int  vga_get_row(void);

/* fb.c - the UEFI framebuffer */
int  fb_active(void);
void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
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

/* The log scrolls between the title bar and the status bar. On VGA that is
 * rows 1..23 of 25; on a framebuffer the screen is taller, so the bottom is
 * worked out from the real row count. */
static int log_top = 1;
static int log_bot = 23;
static int status_row = 24;

int console_status_row(void) { return status_row; }

/* Which screen did we actually get, and how big is it? The boot log should
 * say what is true, not what was hoped for - claiming "VGA text 80x25"
 * while rendering into a UEFI framebuffer is exactly the sort of
 * comfortable lie that hides a broken assumption later. */
int console_kind(void) { return fb_active() ? 1 : 0; }      /* 0 VGA, 1 framebuffer */
int console_cols(void) { return fb_active() ? fb_get_cols() : 80; }
int console_rows(void) { return fb_active() ? fb_get_rows() : 25; }

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
    struct mb_info *mb = (struct mb_info *)mb_addr;

    /* Take the framebuffer only if GRUB says it gave us a packed-RGB one.
     * type 2 is EGA text living behind the same fields, and drawing pixels
     * into that would paint garbage over a perfectly good text console. */
    if (mb && (mb->flags & MB_FLAG_FRAMEBUFFER) &&
        mb->framebuffer_type == FB_TYPE_RGB) {
        fb_setup((unsigned long)mb->framebuffer_addr,
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

void console_clear(void)                 { if (fb_active()) fb_clear(); else vga_clear(); }
void console_putc(char c)                { if (fb_active()) fb_putc(c, log_top, log_bot); else vga_putc(c); }
void console_setcolor(unsigned char a)   { if (fb_active()) fb_setcolor(a); else vga_setcolor(a); }
void console_bar(int r, unsigned char a) { if (fb_active()) fb_bar(r, a); else vga_bar(r, a); }
void console_at(int r, int c, const char *s, unsigned char a)
                                         { if (fb_active()) fb_at(r, c, s, a); else vga_at(r, c, s, a); }
void console_set_row(int r)              { if (fb_active()) fb_set_row(r, log_top, log_bot); else vga_set_row(r); }
int  console_get_row(void)               { return fb_active() ? fb_get_row() : vga_get_row(); }
