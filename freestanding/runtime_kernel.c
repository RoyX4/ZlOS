/* runtime_kernel.c - the FREESTANDING runtime for the kernel subset.
 *
 * runtime.c is the full boxed runtime: lists, strings, 93 builtins, and
 * about 70 libc symbols under it (all of libm, stdio, opendir, fork...).
 * None of that can exist in a kernel, and design_kernel.md is explicit
 * about why: "No implicit allocation in the kernel subset - a zl_alloc
 * reaching a kernel binary is a fault with no diagnostic."
 *
 * So this is a SECOND implementation of the same runtime.h API, covering
 * only what a kernel may use:
 *
 *     numbers (i64 semantics), the arithmetic and comparison operators,
 *     print of a number or a string LITERAL, exit, and raw memory.
 *
 * Everything else - lists, string values, concatenation, the maths
 * builtins - is a hard error rather than a silent surprise, which is the
 * project's standing rule (MASTER_PLAN §8 risk 9: reject, never answer
 * differently).
 *
 * ZERO libc. The only outside contact is zl_putc, and that is deliberately
 * the single seam: on Linux it is a write syscall, in a kernel it is the
 * serial port (design_kernel.md §7 - "serial before interrupts, always").
 * Swapping targets is one function, which is what makes this backend
 * -agnostic in the way that matters.
 */
#include "../runtime.h"

/* ---------------------------------------------------------------- seam */
#ifdef ZL_KERNEL_SERIAL
/* In a kernel: COM1, polled. §7.1's registers. outb is the intrinsic the
 * kernel backend will provide; until then this path is compiled only when
 * the caller supplies one. */
extern void zl_outb(unsigned short port, unsigned char val);
extern unsigned char zl_inb(unsigned short port);
#define COM1 0x3F8
/* the VGA text console lives in the kernel dir - it is board support, not
 * language runtime, so it is only linked in on the kernel target */
/* console.c chooses VGA text or a UEFI framebuffer at run time, so the
 * runtime - and therefore kernel.zl - never learns which screen it is on */
extern void console_putc(char c);
extern void console_clear(void);
extern void console_setcolor(unsigned char attr);
extern void console_bar(int row, unsigned char attr);
extern void console_at(int row, int col, const char *s, unsigned char attr);
extern void console_set_row(int r);
extern int  console_get_row(void);
extern int  console_get_col(void);
extern int  console_status_row(void);
extern int  console_loader(void);
extern int  console_pxw(void);
extern int  console_pxh(void);
extern void console_fill(int x, int y, int w, int h, unsigned char attr);
extern void console_gradient(int x, int y, int w, int h, unsigned char at, unsigned char ab);
extern void console_logo(int px, int py, const char *s, int scale, unsigned char attr);
extern void console_cursor(int row, int col, int on, unsigned char attr);
extern void console_set_region(int top, int bot);
extern void gdt_init(void);
extern void idt_init(void);
extern unsigned int idt_ticks(void);
extern int  idt_scan(void);
extern void console_at_num(int row, int col, long n, unsigned char attr);
extern void console_fill_rgb(int x, int y, int w, int h, unsigned int rgb);
extern void console_gradient_rgb(int x, int y, int w, int h, unsigned int top, unsigned int bot);
extern void console_text_rgb(int px, int py, const char *s, unsigned int rgb);
extern int  console_get_px(int x, int y);
extern void console_shade(int x, int y, int w, int h, int num, int den);
extern void console_shadow(int x, int y, int w, int h, int off, int soft);
extern void console_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
extern void console_text_aa(int px, int py, const char *s, unsigned int rgb);
extern void console_text_aa2x(int px, int py, const char *s, unsigned int rgb);
extern void console_num_aa(int px, int py, long n, unsigned int rgb);
extern void console_puthex(unsigned long v, int digits);
extern void console_char_aa(int px, int py, int code, unsigned int rgb);
extern void console_set_text_box(int c0, int c1);
extern void console_cube(int cx, int cy, int size, int angle, unsigned int rgb);
extern void console_pointer_show(int x, int y);
extern void console_pointer_hide(void);
extern void console_present(void);
extern void console_icon(int px, int py, int n, unsigned int rgb);
extern void console_bg_snapshot(void);
extern void console_bg_restore(int x, int y, int w, int h);
extern void console_grab(int x, int y, int w, int h);
extern void console_stamp(int x, int y);
extern void console_cube_filled(int cx, int cy, int size, int angle, unsigned int rgb);
extern void console_cube_clip(int x0, int y0, int x1, int y1);
extern int  cpu_brand_byte(int i);
extern void speaker_on(unsigned freq);
extern void speaker_off(void);
extern void kreboot(void);
extern int  idt_mouse_x(void);
extern int  idt_mouse_y(void);
extern int  idt_mouse_btn(void);
extern void console_box(int x, int y, int w, int h, unsigned char attr);
extern void console_line(int x0, int y0, int x1, int y1, unsigned char attr);
extern void console_mouse_cursor(int x, int y, unsigned char fill, unsigned char edge);
extern int  console_kind(void);
extern int  console_cols(void);
extern int  console_cell_w(void);
extern int  console_cell_h(void);
extern void fb_set_subpixel(int on);
extern int  fb_get_subpixel(void);
/* the PCI bus driver and our own modesetting driver */
extern void pci_scan(void);
extern int  pci_count(void);
extern int  pci_vendor(int i);
extern int  pci_device(int i);
extern int  pci_class(int i);
extern int  pci_find_class(int cls, int sub);
extern unsigned int pci_bar(int i, int which);
extern int  bga_present(void);
extern int  bga_version(void);
extern int  bga_find(void);
extern unsigned int bga_framebuffer(void);
extern unsigned int bga_vram_bytes(void);
extern int  bga_set_mode(int w, int h, int bpp);
extern int  bga_get_width(void);
extern int  bga_get_height(void);
extern int  console_set_res(int w, int h);
extern int  bga_get_pitch(void);
extern int  bga_reg(int idx);
/* the xHCI USB host controller driver */
extern int  xhci_find(void);
extern int  xhci_present(void);
extern int  xhci_version(void);
extern int  xhci_slots(void);
extern int  xhci_ports(void);
extern int  xhci_ctx_size(void);
extern unsigned int xhci_mmio(void);
extern int  xhci_reset(void);
extern int  xhci_halted(void);
extern int  xhci_port_connected(int p);
extern int  xhci_port_speed(int p);
extern int  xhci_devices_attached(void);
extern int  xhci_init_rings(void);
extern int  xhci_running(void);
extern int  xhci_test_noop(void);
extern int  xhci_port_reset(int p);
extern int  xhci_port_enabled(int p);
extern int  xhci_enumerate(int p);
extern int  xhci_device_address(void);
extern int  xhci_desc_vendor(void);
extern int  xhci_desc_product(void);
extern int  xhci_desc_usbver(void);
extern int  xhci_desc_class(void);
extern int  xhci_desc_mps0(void);
extern int  xhci_desc_byte(int i);
/* the Intel Gen9 display driver */
extern int  intel_find(void);
extern int  intel_present(void);
extern int  intel_supported(void);
extern int  intel_devid(void);
extern unsigned int intel_mmio(void);
extern unsigned int intel_aperture(void);
extern unsigned int intel_stolen_base(void);
extern unsigned int intel_stolen_size(void);
extern unsigned int intel_ggtt_size(void);
extern int  intel_pipe_width(void);
extern int  intel_pipe_height(void);
extern int  intel_stride(void);
extern int  intel_plane_enabled(void);
extern int  intel_pipe_enabled(void);
extern unsigned int intel_surface(void);
extern int  intel_frame_count(void);
extern int  console_rows(void);

static void zl_putc(char c)
{
    /* screen for a human, serial for verify.sh - both, always, so a
     * headless test still sees everything the user would */
    console_putc(c);
    while ((zl_inb(COM1 + 5) & 0x20) == 0) { }   /* wait for THR empty */
    zl_outb(COM1, (unsigned char)c);
}
#else
/* On Linux, for testing the pipeline: write(1, &c, 1) by raw syscall.
 * No libc, no CRT - this is what proves the freestanding claim. */
static void zl_putc(char c)
{
    long ret;
    __asm__ volatile("syscall"
                     : "=a"(ret)
                     : "a"(1L), "D"(1L), "S"(&c), "d"(1L)
                     : "rcx", "r11", "memory");
    (void)ret;
}
#endif

static void zl_puts(const char *s) { while (*s) zl_putc(*s++); }

/* the one character sink, exposed so console.c can print hex through it */
void zl_putc_pub(char c) { zl_putc(c); }

/* signed 64-bit to decimal, no snprintf */
static void zl_put_i64(long long v)
{
    char buf[24];
    int  i = 0;
    if (v == 0) { zl_putc('0'); return; }
    if (v < 0) { zl_putc('-'); }
    unsigned long long u = (v < 0) ? (unsigned long long)(-(v + 1)) + 1ULL
                                   : (unsigned long long)v;
    while (u) { buf[i++] = (char)('0' + (u % 10)); u /= 10; }
    while (i) zl_putc(buf[--i]);
}

/* A kernel has no way to report a fault except to say so and stop. */
static void kfatal(const char *msg)
{
    zl_puts("\nkernel runtime error: ");
    zl_puts(msg);
    zl_putc('\n');
    for (;;) {
#ifdef ZL_KERNEL_SERIAL
        __asm__ volatile("hlt");
#else
        __asm__ volatile("syscall" :: "a"(60L), "D"(1L) : "rcx", "r11");
#endif
    }
}

/* ------------------------------------------------------- making values */
Value zl_nil(void)        { Value v; v.type = V_NIL; v.num = 0; v.str = 0;
                            v.items = 0; v.nitems = 0; v.cap = 0; v.tip = 0;
                            v.fnptr = 0; v.fnargs = 0; return v; }
Value zl_num(double n)    { Value v = zl_nil(); v.type = V_NUM;  v.num = n; return v; }
Value zl_bool(int b)      { Value v = zl_nil(); v.type = V_BOOL; v.num = b ? 1 : 0; return v; }

/* A string VALUE needs a heap. A string LITERAL handed straight to print
 * does not, so zl_str keeps the pointer and print is the only consumer. */
Value zl_str(const char *s) { Value v = zl_nil(); v.type = V_STR; v.str = (char *)s; return v; }

Value zl_list_n(int count, ...)
{
    (void)count;
    kfatal("lists are not available in the kernel subset");
    return zl_nil();
}

/* ------------------------------------------------------- using values */
int zl_truthy(Value v)
{
    switch (v.type) {
        case V_NIL:  return 0;
        case V_BOOL: return v.num != 0;
        case V_NUM:  return v.num != 0;
        case V_STR:  return v.str && v.str[0] != '\0';
        default:     return 1;
    }
}

/* Integer semantics on purpose. design_kernel.md §2 is built on
 * nativegen's exact-i64 arithmetic; a kernel doing floating-point div on a
 * descriptor is how you get a wrong GDT entry. Numbers here are whole. */
Value zl_binop(const char *op, Value a, Value b)
{
    if (a.type == V_STR || b.type == V_STR)
        kfatal("string operators are not available in the kernel subset");

    long long x = (long long)a.num, y = (long long)b.num;

    if (op[0] == '+' && !op[1]) return zl_num((double)(x + y));
    if (op[0] == '-' && !op[1]) return zl_num((double)(x - y));
    if (op[0] == '*' && !op[1]) return zl_num((double)(x * y));
    if (op[0] == '/' && !op[1]) { if (!y) kfatal("divide by zero"); return zl_num((double)(x / y)); }
    if (op[0] == '%' && !op[1]) { if (!y) kfatal("modulo by zero");  return zl_num((double)(x % y)); }

    if (op[0] == '=' && op[1] == '=') return zl_bool(x == y);
    if (op[0] == '!' && op[1] == '=') return zl_bool(x != y);
    if (op[0] == '<' && op[1] == '=') return zl_bool(x <= y);
    if (op[0] == '>' && op[1] == '=') return zl_bool(x >= y);
    if (op[0] == '<' && !op[1])       return zl_bool(x <  y);
    if (op[0] == '>' && !op[1])       return zl_bool(x >  y);

    kfatal("operator not available in the kernel subset");
    return zl_nil();
}

Value zl_unop(const char *op, Value a)
{
    if (op[0] == '-' && !op[1]) return zl_num(-(double)(long long)a.num);
    if (op[0] == 'n')           return zl_bool(!zl_truthy(a));   /* not */
    kfatal("unary operator not available in the kernel subset");
    return zl_nil();
}

Value zl_index(Value seq, Value idx)      { (void)seq; (void)idx;
    kfatal("indexing needs lists, which the kernel subset does not have"); return zl_nil(); }
void  zl_set(Value l, Value i, Value v)   { (void)l; (void)i; (void)v;
    kfatal("index-assign needs lists, which the kernel subset does not have"); }
int   zl_len_list(Value v)                { (void)v;
    kfatal("len needs lists, which the kernel subset does not have"); return 0; }
Value zl_item(Value v, int i)             { (void)v; (void)i;
    kfatal("lists are not available in the kernel subset"); return zl_nil(); }

Value zl_fn(void *fnptr, int nargs)       { Value v = zl_nil(); v.type = V_FN;
                                            v.fnptr = fnptr; v.fnargs = nargs; return v; }

Value zl_callv(Value f, int n, ...)       { (void)f; (void)n;
    kfatal("calling a function value is not available in the kernel subset"); return zl_nil(); }

/* --------------------------------------------------------- the builtins
 * Only the ones a kernel may legitimately use. Everything else is a fault
 * with a name, rather than a link error or silence. */
static int streq(const char *a, const char *b)
{
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}

Value zl_calln(const char *name, int n, ...)
{
    __builtin_va_list ap;
    Value a[8];
    int i;
    __builtin_va_start(ap, n);
    for (i = 0; i < n && i < 8; i++) a[i] = __builtin_va_arg(ap, Value);
    __builtin_va_end(ap);

    if (streq(name, "print")) {
        for (i = 0; i < n; i++) {
            if (i) zl_putc(' ');
            if (a[i].type == V_STR)       zl_puts(a[i].str);
            else if (a[i].type == V_BOOL) zl_puts(a[i].num ? "true" : "false");
            else if (a[i].type == V_NIL)  zl_puts("nil");
            else                          zl_put_i64((long long)a[i].num);
        }
        zl_putc('\n');
        return zl_nil();
    }

    /* put(x) - print with NO trailing newline. print() always ends the
     * line, which makes "[ OK ] message" impossible to build out of
     * separately coloured pieces. */
    if (streq(name, "put")) {
        for (i = 0; i < n; i++) {
            if (a[i].type == V_STR)       zl_puts(a[i].str);
            else if (a[i].type == V_BOOL) zl_puts(a[i].num ? "true" : "false");
            else if (a[i].type == V_NIL)  zl_puts("nil");
            else                          zl_put_i64((long long)a[i].num);
        }
        return zl_nil();
    }

    if (streq(name, "exit")) {
#ifdef ZL_KERNEL_SERIAL
        for (;;) __asm__ volatile("hlt");
#else
        __asm__ volatile("syscall" :: "a"(60L), "D"((long)(long long)a[0].num) : "rcx", "r11");
#endif
        return zl_nil();
    }

    /* raw memory - the whole point of a kernel runtime */
    if (streq(name, "peek8"))  return zl_num((double)*(volatile unsigned char  *)(unsigned long)a[0].num);
    if (streq(name, "peek16")) return zl_num((double)*(volatile unsigned short *)(unsigned long)a[0].num);
    if (streq(name, "peek32")) return zl_num((double)*(volatile unsigned int   *)(unsigned long)a[0].num);
    if (streq(name, "poke8"))  { *(volatile unsigned char  *)(unsigned long)a[0].num = (unsigned char )(unsigned long long)a[1].num; return zl_nil(); }
    if (streq(name, "poke16")) { *(volatile unsigned short *)(unsigned long)a[0].num = (unsigned short)(unsigned long long)a[1].num; return zl_nil(); }
    if (streq(name, "poke32")) { *(volatile unsigned int   *)(unsigned long)a[0].num = (unsigned int  )(unsigned long long)a[1].num; return zl_nil(); }

    /* Port I/O - design_kernel.md §6.3 lists these among the intrinsics a
     * kernel backend must provide. Exposing them as builtins is what lets a
     * DRIVER be written in zl rather than in C: the serial and keyboard
     * drivers in kernel.zl are ordinary zl functions that call inb/outb. */
#ifdef ZL_KERNEL_SERIAL
    if (streq(name, "inb"))  return zl_num((double)zl_inb((unsigned short)(unsigned long long)a[0].num));
    if (streq(name, "outb")) { zl_outb((unsigned short)(unsigned long long)a[0].num,
                                       (unsigned char )(unsigned long long)a[1].num); return zl_nil(); }
#endif

#ifdef ZL_KERNEL_SERIAL
    /* the text console, driven from zl - colour, bars and cursor rows are
     * what turn a scrolling log into something that reads as an OS */
    if (streq(name, "cls"))      { console_clear(); return zl_nil(); }
    if (streq(name, "color"))    { console_setcolor((unsigned char)(unsigned long long)a[0].num); return zl_nil(); }
    if (streq(name, "bar"))      { console_bar((int)a[0].num, (unsigned char)(unsigned long long)a[1].num); return zl_nil(); }
    if (streq(name, "at"))       { if (a[2].type == V_STR)
                                       console_at((int)a[0].num, (int)a[1].num, a[2].str,
                                                  (unsigned char)(unsigned long long)a[3].num);
                                   return zl_nil(); }
    if (streq(name, "row"))      return zl_num((double)console_get_row());
    /* the status bar row differs between VGA (24) and a taller
     * framebuffer, so zl asks rather than hardcoding it */
    if (streq(name, "status_row")) return zl_num((double)console_status_row());
    if (streq(name, "con_kind"))   return zl_num((double)console_kind());
    if (streq(name, "con_cols"))   return zl_num((double)console_cols());
    if (streq(name, "con_rows"))   return zl_num((double)console_rows());
    if (streq(name, "cell_w"))     return zl_num((double)console_cell_w());
    if (streq(name, "cell_h"))     return zl_num((double)console_cell_h());
    if (streq(name, "bits"))       return zl_num((double)(sizeof(void *) * 8));
    if (streq(name, "hex"))        { console_puthex((unsigned long)(long long)a[0].num, (int)a[1].num); return zl_nil(); }
    if (streq(name, "subpix"))     { fb_set_subpixel((int)a[0].num); return zl_nil(); }
    if (streq(name, "subpix_on"))  return zl_num((double)fb_get_subpixel());
    if (streq(name, "pci_scan"))   { pci_scan(); return zl_nil(); }
    if (streq(name, "pci_count"))  return zl_num((double)pci_count());
    if (streq(name, "pci_vendor")) return zl_num((double)pci_vendor((int)a[0].num));
    if (streq(name, "pci_device")) return zl_num((double)pci_device((int)a[0].num));
    if (streq(name, "pci_class"))  return zl_num((double)pci_class((int)a[0].num));
    if (streq(name, "gpu_find"))   return zl_num((double)bga_find());
    if (streq(name, "gpu_present"))return zl_num((double)bga_present());
    if (streq(name, "gpu_version"))return zl_num((double)bga_version());
    if (streq(name, "gpu_fb"))     return zl_num((double)bga_framebuffer());
    if (streq(name, "gpu_vram"))   return zl_num((double)bga_vram_bytes());
    if (streq(name, "gpu_mode"))   return zl_num((double)bga_set_mode((int)a[0].num,(int)a[1].num,(int)a[2].num));
    if (streq(name, "gpu_w"))      return zl_num((double)bga_get_width());
    if (streq(name, "gpu_h"))      return zl_num((double)bga_get_height());
    if (streq(name, "set_res"))    return zl_num((double)console_set_res((int)a[0].num,(int)a[1].num));
    if (streq(name, "gpu_pitch"))  return zl_num((double)bga_get_pitch());
    if (streq(name, "gpu_reg"))    return zl_num((double)bga_reg((int)a[0].num));
    if (streq(name, "usb_find"))   return zl_num((double)xhci_find());
    if (streq(name, "usb_ok"))     return zl_num((double)xhci_present());
    if (streq(name, "usb_ver"))    return zl_num((double)xhci_version());
    if (streq(name, "usb_slots"))  return zl_num((double)xhci_slots());
    if (streq(name, "usb_ports"))  return zl_num((double)xhci_ports());
    if (streq(name, "usb_ctxsz"))  return zl_num((double)xhci_ctx_size());
    if (streq(name, "usb_mmio"))   return zl_num((double)xhci_mmio());
    if (streq(name, "usb_reset"))  return zl_num((double)xhci_reset());
    if (streq(name, "usb_conn"))   return zl_num((double)xhci_port_connected((int)a[0].num));
    if (streq(name, "usb_speed"))  return zl_num((double)xhci_port_speed((int)a[0].num));
    if (streq(name, "usb_count"))  return zl_num((double)xhci_devices_attached());
    if (streq(name, "usb_rings"))  return zl_num((double)xhci_init_rings());
    if (streq(name, "usb_run"))    return zl_num((double)xhci_running());
    if (streq(name, "usb_noop"))   return zl_num((double)xhci_test_noop());
    if (streq(name, "usb_prst"))   return zl_num((double)xhci_port_reset((int)a[0].num));
    if (streq(name, "usb_pen"))    return zl_num((double)xhci_port_enabled((int)a[0].num));
    if (streq(name, "usb_enum"))   return zl_num((double)xhci_enumerate((int)a[0].num));
    if (streq(name, "usb_addr"))   return zl_num((double)xhci_device_address());
    if (streq(name, "usb_vid"))    return zl_num((double)xhci_desc_vendor());
    if (streq(name, "usb_pid"))    return zl_num((double)xhci_desc_product());
    if (streq(name, "usb_uver"))   return zl_num((double)xhci_desc_usbver());
    if (streq(name, "usb_cls"))    return zl_num((double)xhci_desc_class());
    if (streq(name, "usb_mps"))    return zl_num((double)xhci_desc_mps0());
    if (streq(name, "usb_dbyte"))  return zl_num((double)xhci_desc_byte((int)a[0].num));
    if (streq(name, "intel_find"))  return zl_num((double)intel_find());
    if (streq(name, "intel_ok"))    return zl_num((double)intel_supported());
    if (streq(name, "intel_id"))    return zl_num((double)intel_devid());
    if (streq(name, "intel_mmio"))  return zl_num((double)intel_mmio());
    if (streq(name, "intel_aper"))  return zl_num((double)intel_aperture());
    if (streq(name, "intel_sbase")) return zl_num((double)intel_stolen_base());
    if (streq(name, "intel_ssize")) return zl_num((double)intel_stolen_size());
    if (streq(name, "intel_ggtt"))  return zl_num((double)intel_ggtt_size());
    if (streq(name, "intel_w"))     return zl_num((double)intel_pipe_width());
    if (streq(name, "intel_h"))     return zl_num((double)intel_pipe_height());
    if (streq(name, "intel_stride"))return zl_num((double)intel_stride());
    if (streq(name, "intel_plane")) return zl_num((double)intel_plane_enabled());
    if (streq(name, "intel_pipe"))  return zl_num((double)intel_pipe_enabled());
    if (streq(name, "intel_surf"))  return zl_num((double)intel_surface());
    if (streq(name, "loader"))     return zl_num((double)console_loader());
    if (streq(name, "px_w"))       return zl_num((double)console_pxw());
    if (streq(name, "px_h"))       return zl_num((double)console_pxh());
    if (streq(name, "fill_rect")) { console_fill((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned char)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "gradient"))  { console_gradient((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned char)(unsigned long long)a[4].num,(unsigned char)(unsigned long long)a[5].num); return zl_nil(); }
    if (streq(name, "logo"))      { if (a[2].type==V_STR) console_logo((int)a[0].num,(int)a[1].num,a[2].str,(int)a[3].num,(unsigned char)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "cursor"))    { console_cursor((int)a[0].num,(int)a[1].num,(int)a[2].num,(unsigned char)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "region"))    { console_set_region((int)a[0].num,(int)a[1].num); return zl_nil(); }
    if (streq(name, "setup_gdt")) { gdt_init(); return zl_nil(); }
    if (streq(name, "setup_idt")) { idt_init(); return zl_nil(); }
    if (streq(name, "ticks"))     return zl_num((double)idt_ticks());
    if (streq(name, "scan_get"))  return zl_num((double)idt_scan());
    if (streq(name, "at_num"))    { console_at_num((int)a[0].num,(int)a[1].num,(long)a[2].num,(unsigned char)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "fill_rgb"))  { console_fill_rgb((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned int)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "grad_rgb"))  { console_gradient_rgb((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned int)(unsigned long long)a[4].num,(unsigned int)(unsigned long long)a[5].num); return zl_nil(); }
    if (streq(name, "text_rgb"))  { if (a[2].type==V_STR) console_text_rgb((int)a[0].num,(int)a[1].num,a[2].str,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "get_px"))    return zl_num((double)console_get_px((int)a[0].num,(int)a[1].num));
    if (streq(name, "shade"))     { console_shade((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num,(int)a[5].num); return zl_nil(); }
    if (streq(name, "shadow"))    { console_shadow((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num,(int)a[5].num); return zl_nil(); }
    if (streq(name, "rrect"))     { console_rrect((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(int)a[4].num,(unsigned int)(unsigned long long)a[5].num); return zl_nil(); }
    if (streq(name, "text_aa"))   { if (a[2].type==V_STR) console_text_aa((int)a[0].num,(int)a[1].num,a[2].str,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "text_big"))  { if (a[2].type==V_STR) console_text_aa2x((int)a[0].num,(int)a[1].num,a[2].str,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "num_aa"))    { console_num_aa((int)a[0].num,(int)a[1].num,(long)a[2].num,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "char_aa"))   { console_char_aa((int)a[0].num,(int)a[1].num,(int)a[2].num,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "text_box"))  { console_set_text_box((int)a[0].num,(int)a[1].num); return zl_nil(); }
    if (streq(name, "cube"))      { console_cube((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned int)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "mpoint"))    { console_pointer_show((int)a[0].num,(int)a[1].num); return zl_nil(); }
    if (streq(name, "mhide"))     { console_pointer_hide(); return zl_nil(); }
    if (streq(name, "present"))   { console_present(); return zl_nil(); }
    if (streq(name, "icon"))      { console_icon((int)a[0].num,(int)a[1].num,(int)a[2].num,(unsigned int)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "bg_snap"))   { console_bg_snapshot(); return zl_nil(); }
    if (streq(name, "bg_rest"))   { console_bg_restore((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "grab"))      { console_grab((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "stamp"))     { console_stamp((int)a[0].num,(int)a[1].num); return zl_nil(); }
    if (streq(name, "cube3d"))    { console_cube_filled((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned int)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "cube_clip")) { console_cube_clip((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num); return zl_nil(); }
    if (streq(name, "cpu_char"))  return zl_num((double)cpu_brand_byte((int)a[0].num));
    if (streq(name, "emit"))      { zl_putc((char)(int)a[0].num); return zl_nil(); }
    if (streq(name, "sc"))        { console_putc((char)(int)a[0].num); return zl_nil(); }
    if (streq(name, "col"))       return zl_num((double)console_get_col());
    if (streq(name, "beep_on"))   { speaker_on((unsigned)(long long)a[0].num); return zl_nil(); }
    if (streq(name, "beep_off"))  { speaker_off(); return zl_nil(); }
    if (streq(name, "reboot"))    { kreboot(); return zl_nil(); }
    if (streq(name, "mouse_x"))   return zl_num((double)idt_mouse_x());
    if (streq(name, "mouse_y"))   return zl_num((double)idt_mouse_y());
    if (streq(name, "mouse_btn")) return zl_num((double)idt_mouse_btn());
    if (streq(name, "box"))       { console_box((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned char)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "line"))      { console_line((int)a[0].num,(int)a[1].num,(int)a[2].num,(int)a[3].num,(unsigned char)(unsigned long long)a[4].num); return zl_nil(); }
    if (streq(name, "mcursor"))   { console_mouse_cursor((int)a[0].num,(int)a[1].num,(unsigned char)(unsigned long long)a[2].num,(unsigned char)(unsigned long long)a[3].num); return zl_nil(); }
    if (streq(name, "goto_row")) { console_set_row((int)a[0].num); return zl_nil(); }
#endif

    /* Bitwise ops. A driver cannot be written without them - every status
     * register is read by masking a bit. Same names as runtime.c's. */
    if (streq(name, "band")) return zl_num((double)((long long)a[0].num & (long long)a[1].num));
    if (streq(name, "bor"))  return zl_num((double)((long long)a[0].num | (long long)a[1].num));
    if (streq(name, "bxor")) return zl_num((double)((long long)a[0].num ^ (long long)a[1].num));
    if (streq(name, "bnot")) return zl_num((double)(~(long long)a[0].num));
    if (streq(name, "shl"))  return zl_num((double)((long long)a[0].num << (long long)a[1].num));
    if (streq(name, "shr"))  return zl_num((double)((unsigned long long)a[0].num >> (long long)a[1].num));
    /* peek64/poke64 carry the 2^53 hazard (design_kernel.md §2); the two
     * -halves rule means a kernel should not need them for descriptors. */
    if (streq(name, "peek64")) {
        unsigned long long v = *(volatile unsigned long long *)(unsigned long)a[0].num;
        if (v > 9007199254740992ULL) kfatal("peek64 above 2^53 - read two peek32 halves");
        return zl_num((double)v);
    }
    if (streq(name, "poke64")) {
        unsigned long long v = (unsigned long long)a[1].num;
        if (v > 9007199254740992ULL) kfatal("poke64 above 2^53 - write two poke32 halves");
        *(volatile unsigned long long *)(unsigned long)a[0].num = v;
        return zl_nil();
    }
    if (streq(name, "fill_mem")) {
        volatile unsigned char *p = (volatile unsigned char *)(unsigned long)a[0].num;
        unsigned long long cnt = (unsigned long long)a[2].num;
        unsigned char b = (unsigned char)(unsigned long long)a[1].num;
        while (cnt--) *p++ = b;
        return zl_nil();
    }
    if (streq(name, "copy_mem")) {
        volatile unsigned char *d = (volatile unsigned char *)(unsigned long)a[0].num;
        volatile const unsigned char *s = (volatile const unsigned char *)(unsigned long)a[1].num;
        unsigned long long cnt = (unsigned long long)a[2].num;
        if (d < s) { while (cnt--) *d++ = *s++; }
        else       { d += cnt; s += cnt; while (cnt--) *--d = *--s; }
        return zl_nil();
    }

    kfatal("builtin not available in the kernel subset");
    return zl_nil();
}
