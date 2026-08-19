/* wmshot.c - render one frame of the compositor to a PPM, from Linux.
 *
 * wmtest asserts 47 properties and nobody has LOOKED at any of them. Both
 * matter and they catch different things: assertions catch a click landing on
 * the wrong window, eyes catch a title bar that is four pixels too tall. This
 * is the second kind, and it costs no boot - the whole stack is C against
 * memory, so it renders here in milliseconds.
 *
 * The apps below are written the way a real app must be: through ui_* calls
 * only, position-pure, no loop. They are also therefore a worked example of
 * the app contract in ui.h, which is worth having somewhere executable.
 *
 *   ./wmshot [out.ppm] [width height]
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../ui.h"
#include "../../runtime.h"

void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_gradient(int x, int y, int w, int h, unsigned int top, unsigned int bot);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_text_aa(int px, int py, const char *s, unsigned int fg);
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
void fb_icon24(int px, int py, int n, unsigned int fg);
void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb);
void fb_present(void);
unsigned int fb_get_px(int x, int y);
int  fb_cell_h(void);

/* fake hardware, same as wmtest */
static int fake_x = 1290, fake_y = 342, fake_btn = 0;
static unsigned fake_ticks = 1;
int idt_mouse_x(void)   { return fake_x; }
int idt_mouse_y(void)   { return fake_y; }
int idt_mouse_btn(void) { return fake_btn; }
/* the scroll wheel: read-and-clear, so a harness with no wheel must return
 * 0 rather than a stale notch (desktop/feel-and-control added this). */
int idt_mouse_wheel(void) { return 0; }
unsigned long long cpu_tsc(void) { static unsigned long long t; t += 2000000; return t; }
unsigned int cpu_tsc_khz(void) { return 2000000; }

unsigned int idt_ticks(void) { return fake_ticks; }
int idt_scan(void)      { return 0; }
int xhci_key(void)      { return 0; }
int ser_rx(void)        { return -1; }   /* no UART in the harness */

/* THE FRAME TIMER's clock. wm.c times itself with the TSC; the harness has no
 * cpu.c, and a frame measured here would be measuring this machine rather than
 * the guest anyway. Returning a monotonically rising count keeps the timing
 * code on the same path it takes in the kernel - a stub that returned 0 would
 * make wm_frame() take the "TSC unavailable" branch and stop exercising it. */
static unsigned int fake_tsc;
unsigned int cpu_tsc_lo(void)  { return (fake_tsc += 20000000); }
/* (duplicate cpu_tsc_khz stub removed - the merge gave this harness two) */
static int fake_usb_ptr = 0, fake_ux = 0, fake_uy = 0, fake_ubtn = 0;

/* No USB pointer in the harness - which is a case worth being able to express,
 * because it is the PS/2 fallback the laptop's TrackPoint takes. */
int xhci_ptr_ready(void) { return fake_usb_ptr; }
int xhci_ptr_poll(void)  { return 0; }
int xhci_poll(int max)   { (void)max; return 0; }  /* the one ring drainer */
/* When this harness has a USB pointer at all, what it supplies is an
 * ABSOLUTE position - so it is a tablet, and it must say so. input.c no
 * longer infers "absolute" from "a USB pointer exists", because that is
 * exactly what sent every relative usb-mouse down the tablet branch. */
int xhci_ptr_abs(void)   { return 1; }
/* input.c reads the USB wheel through this. A target that does not stub it
 * fails to LINK, build.sh stops, and every target after it keeps its
 * binary from the previous run - so a change that does not compile can
 * report "all good". */
int xhci_ptr_take_wheel(void) { return 0; }
int xhci_ptr_take_dx(void) { return 0; }
int xhci_ptr_take_dy(void) { return 0; }
int xhci_ptr_x(void)     { return fake_ux; }
int xhci_ptr_y(void)     { return fake_uy; }
int xhci_ptr_btn(void)   { return fake_ubtn; }

void idt_set_pointer_bounds(int w, int h) { (void)w; (void)h; }
void zl_putc_pub(char c) { (void)c; }
Value zl_num(double n) { Value v; memset(&v, 0, sizeof v); v.type = V_NUM; v.num = n; return v; }

static int W = 1920, H = 1200;

#define APP_SHELL   0
#define APP_MONITOR 1
#define APP_ABOUT   2
#define APP_FILES   3

/* ---- the apps, through ui_* only ------------------------------------------ */
static int cpu_pct = 34, mem_pct = 62, wrap_on = 1, volume = 70, sel = 2;
static int list_off;

static void app_draw(int app, int x, int y, int w, int h, int focused)
{
    const struct ui_theme *t = ui_theme();
    /* the pointer, so hover states are live. click is 0 here - a screenshot of
     * a pressed button would be a screenshot of a lie. */
    ui_begin(x, y, w, h, UI_DRAW, fake_x, fake_y, 0);

    if (app == APP_SHELL) {
        ui_label_dim("zlOS 0.3   -   a desktop written in zl");
        ui_sep();
        ui_label("[  OK  ] framebuffer console, 120x37");
        ui_label("[  OK  ] APIC: IRQs via I/O APIC, 4 CPU(s)");
        ui_label("[  OK  ] zl runtime, kernel subset");
        ui_label_dim("[ INFO ] no heap, zlfs mounts on demand, no scheduler");
        ui_space(t->gap);
        ui_label("ready.");
        ui_space(t->gap);
        ui_row();
        ui_button("help");
        ui_button("snake");
        ui_button("3D cube");
        ui_endrow();
        return;
    }
    if (app == APP_MONITOR) {
        ui_num("CPU", cpu_pct);
        ui_bar(cpu_pct);
        ui_num("MEM", mem_pct);
        ui_bar(mem_pct);
        ui_space(t->gap);
        ui_toggle("subpixel", &wrap_on);
        ui_label_dim("volume");
        ui_slider(&volume, 0, 100);
        return;
    }
    if (app == APP_ABOUT) {
        ui_label("zlOS 0.3");
        ui_label_dim("no OS   no libc   no GNU");
        ui_sep();
        ui_label_dim("11,374 lines, hand written");
        (void)focused;
        return;
    }
    if (app == APP_FILES) {
        static const char *names[] = {
            "kernel.zl", "fb.c", "wm.c", "ui.c", "input.c", "intel.c",
            "xhci.c", "nvme.c", "apic.c", "smp.c", "cpu.c", "sched.c",
        };
        ui_label_dim("kernel/");
        ui_scroll_begin(h - 4 * t->row_h, &list_off);
        for (int i = 0; i < 12; i++) ui_list_row(names[i], i == sel);
        ui_scroll_end(&list_off);
        return;
    }
}

static int app_event(int a, int win, int ty, int c, int x, int y)
{ (void)a;(void)win;(void)ty;(void)c;(void)x;(void)y; return 0; }
static int app_tick(int a, int win) { (void)a; (void)win; return 0; }

/* ---- the furniture: wallpaper, header, dock -------------------------------- */
static void desk_draw(int x, int y, int w, int h)
{
    const struct ui_theme *t = ui_theme();
    (void)x; (void)y; (void)w; (void)h;
    fb_gradient(0, 0, W, H, 0x141A2E, 0x2A3350);

    int hb = t->title_h + UI_S1(t);
    fb_gradient(0, 0, W, hb, 0x1B2340, 0x141A2E);
    fb_fill_px(0, hb, W, 1, t->border);
    fb_rrect(UI_S3(t), (hb - UI_S3(t)) / 2, UI_S3(t), UI_S3(t), UI_S1(t) / 2, t->accent);
    fb_text_prop(UI_S3(t) * 3, (hb - fb_text_prop_h()) / 2, "zlOS", t->text);
    fb_text_prop(UI_S3(t) * 3 + UI_S6(t) * 3, (hb - fb_text_prop_h()) / 2, "Activities", t->text_dim);
    /* Report the size actually rendered. This was the literal "1920 x 1200",
     * which made every render at another size a screenshot of a lie - and the
     * resolution cliff is exactly the class this harness is meant to catch. */
    char res[32];
    snprintf(res, sizeof res, "%d x %d", W, H);
    fb_text_prop(W - UI_S6(t) * 8, (hb - fb_text_prop_h()) / 2, res, t->text_dim);

    int dh = UI_S6(t) * 3;
    int dy = H - dh;
    fb_gradient(0, dy, W, dh, 0x282E42, 0x121420);
    fb_fill_px(0, dy, W, 1, t->border);
    int tile = UI_S6(t) * 3, ix = UI_S6(t);
    fb_rrect(ix, dy + UI_S2(t), tile * 2, dh - 2 * UI_S2(t), UI_S1(t), t->title);
    fb_rrect(ix + UI_S3(t), dy + dh / 2 - UI_S2(t), UI_S4(t), UI_S4(t), UI_S1(t) / 2, t->accent);
    fb_text_prop(ix + UI_S6(t) * 2, dy + (dh - fb_text_prop_h()) / 2, "zlOS", t->text);
    ix += tile * 2 + UI_S6(t);
    for (int i = 0; i < 7; i++) {
        fb_rrect(ix, dy + UI_S2(t), tile, dh - 2 * UI_S2(t), UI_S1(t), 0x1B2236);
        fb_icon24(ix + (tile - 48) / 2, dy + (dh - 48) / 2, i, 0xC8D4EC);
        ix += tile + UI_S2(t);
    }
    /* the tray text starts AFTER the last tile, never on top of it - the
     * dock in kernel.zl has the same rule and the same reason */
    int tray = ix + UI_S6(t);
    int want = W - UI_S6(t) * 9;
    fb_text_prop(want > tray ? want : tray, dy + (dh - fb_text_prop_h()) / 2,
               "state: ready", t->text_dim);
}


/* input.c's USB keyboard path takes RAW HID EVENTS now, not decoded chars
 * (claude/ecstatic-lewin-f617bb - there is no character for Up, so a decoded
 * arrow came back 0 and 0 means "nothing typed"). These harnesses drive the
 * pointer, so no USB keyboard is present. */
int xhci_key_event(void) { return 0; }
int xhci_kbd_mods(void)  { return 0; }

int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : "wmshot.ppm";
    if (argc > 3) { W = atoi(argv[2]); H = atoi(argv[3]); }

    /* ONE buffer now. C4 deleted the drag background and sprite, and the back
     * buffer moved down into the space they freed - see fb.c's high-RAM map.
     * 0x08000000..0x0A800000 is 40 MiB, bounded by the AP stacks. */
    struct { unsigned long a, n; } bufs[] = {
        { 0x08000000UL, 0x0A800000UL - 0x08000000UL },
    };
    for (unsigned i = 0; i < 1; i++) {
        void *p = mmap((void *)bufs[i].a, bufs[i].n, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        if (p != (void *)bufs[i].a) { fprintf(stderr, "mmap\n"); return 1; }
        memset(p, 0, bufs[i].n);
    }
    void *vram = mmap(NULL, 64UL << 20, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    fb_setup((unsigned long)vram, (unsigned)W * 4, (unsigned)W, (unsigned)H, 32);

    ui_theme_init(2);
    wm_init();
    wm_hooks(app_draw, app_event, app_tick, desk_draw);

    const struct ui_theme *t = ui_theme();
    int hb = t->title_h + UI_S1(t);
    wm_open(APP_SHELL,   "zl shell   ~",    UI_S6(t), hb + UI_S6(t), 1180, 720);
    wm_open(APP_FILES,   "Files",           1260, hb + UI_S6(t), 560, 420);
    wm_open(APP_MONITOR, "System Monitor",  1260, hb + UI_S6(t) + 450, 560, 380);
    int wabout = wm_open(APP_ABOUT, "About", 700, 700, 520, 300);
    /* H2: several apps in one frame, grouped by task - the Essence idea. The
     * About window becomes a tabbed one so the strip is visible in the shot. */
    wm_add_tab(wabout, APP_MONITOR, "Stats");
    wm_add_tab(wabout, APP_FILES, "Files");

    /* let the open animation settle, then take the picture */
    for (int i = 0; i < 7; i++) { fake_ticks++; wm_frame(); }

    FILE *f = fopen(out, "wb");
    if (!f) { perror("open"); return 1; }
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            unsigned c = fb_get_px(x, y);
            unsigned char px[3] = { (unsigned char)(c >> 16),
                                    (unsigned char)(c >> 8),
                                    (unsigned char)c };
            fwrite(px, 1, 3, f);
        }
    fclose(f);
    printf("wrote %s  %dx%d   (%d windows, focus %d)\n",
           out, W, H, wm_count(), wm_focused());
    return 0;
}
