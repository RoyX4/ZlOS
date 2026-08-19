/* browsershot.c - the same document at three widths, in one picture.
 *
 * htmltest asserts that narrower means more lines. That is the correct
 * assertion and it is not sufficient: a layout can satisfy every numeric
 * invariant and still put a list marker inside its own text, hang a heading's
 * descender into the line under it, or set <code> at a size that does not
 * match the prose around it. Those are found by looking, and looking costs a
 * boot unless the whole stack runs on the host - which it does, because
 * html.c and layout.c have no pixels in them and fb.c is C against memory.
 *
 * THREE COLUMNS, ONE FRAME, deliberately: reflow is a comparison, and a
 * comparison between two pictures taken at different times is a comparison
 * nobody actually makes. Side by side, a line break that did not move is
 * obvious.
 *
 *   ./browsershot [out.ppm]
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../memmap.h"
#include "../css.h"
#include "../ui.h"
#include "../../runtime.h"

void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_gradient(int x, int y, int w, int h, unsigned int top, unsigned int bot);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
int  fb_prop_em(void);
unsigned int fb_get_px(int x, int y);

void browser_home(void);
void browser_draw(int x, int y, int w, int h, int focused);
void browser_load(const char *src, int len);
int  browser_height(void);
int  browser_lines(void);
int  browser_runs(void);
int  browser_scroll_by(int d);
int  browser_link_at(int cx, int cy);
const char *browser_title(void);

/* fake hardware - fb.c reaches for the tick counter and the pointer */
static unsigned fake_ticks = 1;
int idt_mouse_x(void)   { return -1; }
int idt_mouse_y(void)   { return -1; }
int idt_mouse_btn(void) { return 0; }
/* the scroll wheel: read-and-clear, so a harness with no wheel must return
 * 0 rather than a stale notch (desktop/feel-and-control added this). */
int idt_mouse_wheel(void) { return 0; }
unsigned int idt_ticks(void) { return fake_ticks; }
/* browser.c now reaches http.c -> tcp.c -> net.c, and net.c wants the
 * calibrated TSC for its round-trip timing. Nothing in a still picture uses
 * it, so it is stubbed rather than the whole clock being dragged in. */
unsigned long long cpu_tsc(void)  { return 0; }
unsigned int cpu_tsc_khz(void)    { return 0; }
int idt_scan(void)      { return 0; }
int xhci_key(void)      { return 0; }
void idt_set_pointer_bounds(int w, int h) { (void)w; (void)h; }
void zl_putc_pub(char c) { (void)c; }
Value zl_num(double n) { Value v; memset(&v, 0, sizeof v); v.type = V_NUM; v.num = n; return v; }

static int W, H = 1000;

/* fb_setup() tells input.c the new screen size so the pointer clamp follows
 * the mode (desktop/feel-and-control). Harnesses that link fb.c without
 * input.c have to answer for it. */
void input_set_bounds(int w, int h) { (void)w; (void)h; }

int main(int argc, char **argv)
{
    const char *out = argc > 1 ? argv[1] : "browsershot.ppm";
    /* HOW FAR DOWN THE DOCUMENT TO LOOK. The page is now several screens tall
     * and the new box model - flex, grid, borders, and a decoded picture - is
     * below the first one, so a gate that can only photograph the top cannot
     * see the thing it exists to watch. This is the whole of the fix: the same
     * scroll the app already has, driven from the command line. */
    int scroll_px = argc > 2 ? atoi(argv[2]) : 0;
    /* AN ARBITRARY PAGE, not just the built-in one. The home page is the gate
     * document and it is written to exercise what this browser claims; that
     * makes it exactly the wrong thing to judge a REAL page by, because it was
     * authored against the engine. Pointing this at 52 KB of Google is how you
     * find out what a page nobody here wrote looks like. */
    const char *html_file = argc > 3 ? argv[3] : 0;

    /* The high-RAM arenas the shipping source writes into, at the addresses it
     * hardcodes. 0x02000000 is memmap.h's HI_IMG - png.c's decoded pictures
     * and browser.c's base64 scratch - and it is here for the same reason the
     * other three are: so browser.c and png.c compile unmodified, with no
     * #ifdef and no host-only fallback buffer that would make the harness test
     * something the kernel does not run. */
    struct { unsigned long a, n; } bufs[] = {
        /* from memmap.h, not from a literal - see browsertest.c's note */
        { (unsigned long)HI_IMG, (unsigned long)(HI_IMG_END - HI_IMG) },
        /* the document, the tree, its text, the stylesheet and the runs - all
         * six of browser.c's arenas live in this one region now. See the note
         * in browsertest.c's map_high_ram. */
        { (unsigned long)HI_DOM, (unsigned long)(HI_DOM_END - HI_DOM) },
        { 0x08000000UL, 32UL << 20 }, { 0x0A000000UL, 16UL << 20 },
        { 0x0C000000UL, 16UL << 20 },
    };
    for (unsigned i = 0; i < sizeof bufs / sizeof bufs[0]; i++) {
        void *p = mmap((void *)bufs[i].a, bufs[i].n, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        if (p != (void *)bufs[i].a) { fprintf(stderr, "mmap\n"); return 1; }
        memset(p, 0, bufs[i].n);
    }

    int cols[3] = { 760, 480, 300 };
    int gap = 24, pad = 24;
    W = pad;
    for (int i = 0; i < 3; i++) W += cols[i] + gap;
    W = W - gap + pad;

    void *vram = mmap(NULL, 64UL << 20, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    fb_setup((unsigned long)vram, (unsigned)W * 4, (unsigned)W, (unsigned)H, 32);
    ui_theme_init(1);
    const struct ui_theme *t = ui_theme();

    fb_gradient(0, 0, W, H, 0x0E1220, t->bg);
    browser_home();

    int label_h = fb_text_prop_h() + 10;
    int x = pad;
    int hh[3], ll[3], rr[3];
    for (int i = 0; i < 3; i++) {
        char cap[64];
        snprintf(cap, sizeof cap, "%d px wide", cols[i]);
        fb_text_prop(x, pad - fb_text_prop_h() + 6 > 0 ? pad - fb_text_prop_h() + 6 : 0,
                     cap, t->text_dim);
        int by = pad + label_h;
        int bh = H - by - pad;
        fb_rrect(x - 2, by - 2, cols[i] + 4, bh + 4, 6, t->border);
        /* Draw once to lay out at THIS width, then scroll, then draw again -
         * the scroll is clamped against the content height, which is not
         * known until the first layout. Two paints, and the second is the one
         * that is photographed. */
        if (html_file) {
            /* 2 MiB, matching browser.c's DOC_MAX. It was 1 MiB, which is 66 KB
             * SHORT of the English Wikipedia article this is usually pointed at
             * - so the harness would have truncated the page before browser.c
             * got the chance, and the truncation flag under test would have
             * been the harness's rather than the browser's. */
            static char buf[2 << 20];
            FILE *hf = fopen(html_file, "rb");
            if (!hf) { fprintf(stderr, "cannot open %s\n", html_file); return 1; }
            int hn = (int)fread(buf, 1, sizeof buf - 1, hf);
            fclose(hf);
            buf[hn] = 0;
            browser_load(buf, hn);
            /* AND ITS EXTERNAL STYLESHEETS, which on a real page are most of
             * the CSS. browser.c fetches these over the network; here they are
             * files, so the rendering can be judged without a wire. */
            const char *extra = getenv("ZLOS_EXTRA_CSS");
            if (extra) {
                static char css[1 << 20];
                char one[512];
                int p0 = 0;
                while (*extra) {
                    int k = 0;
                    while (*extra && *extra != ':' && k < (int)sizeof one - 1)
                        one[k++] = *extra++;
                    one[k] = 0;
                    if (*extra == ':') extra++;
                    FILE *cf = fopen(one, "rb");
                    if (!cf) { fprintf(stderr, "no css %s\n", one); continue; }
                    int cn = (int)fread(css, 1, sizeof css - 1, cf);
                    fclose(cf);
                    css[cn] = 0;
                    int took = css_add_sheet(css, cn);
                    fprintf(stderr, "  + %s: %d bytes, %d rules taken, overflow %d\n",
                            one, cn, took, css_overflowed());
                    p0++;
                }
            }
        }
        browser_draw(x, by, cols[i], bh, i == 0);
        if (scroll_px) {
            browser_scroll_by(-1000000);
            browser_scroll_by(scroll_px);
            browser_draw(x, by, cols[i], bh, i == 0);
        }
        hh[i] = browser_height();
        ll[i] = browser_lines();
        rr[i] = browser_runs();
        x += cols[i] + gap;
    }

    FILE *f = fopen(out, "wb");
    if (!f) { perror("open"); return 1; }
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int y = 0; y < H; y++)
        for (int xx = 0; xx < W; xx++) {
            unsigned c = fb_get_px(xx, y);
            unsigned char px[3] = { (unsigned char)(c >> 16),
                                    (unsigned char)(c >> 8),
                                    (unsigned char)c };
            fwrite(px, 1, 3, f);
        }
    fclose(f);

    printf("wrote %s  %dx%d   title '%s'  em %dpx\n",
           out, W, H, browser_title(), fb_prop_em());
    printf("  width   height   lines   runs\n");
    for (int i = 0; i < 3; i++)
        printf("  %5d   %6d   %5d   %4d\n", cols[i], hh[i], ll[i], rr[i]);

    /* the same claim htmltest makes, restated against the REAL font metrics -
     * the synthetic measure could hide a metric bug that only the real
     * advances produce */
    int ok = ll[1] > ll[0] && ll[2] > ll[1] && hh[1] > hh[0] && hh[2] > hh[1];
    printf("  reflow: %s\n", ok ? "narrower gives more lines, every step"
                                : "NOT MONOTONIC - the layout is not reflowing");
    return ok ? 0 : 1;
}
