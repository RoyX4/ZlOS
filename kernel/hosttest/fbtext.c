/* fbtext.c - the proportional text engine, asserted. No kernel, no boot.
 *
 * WHY THIS EXISTS. The browser's rich-text API (fb_text_rich, fb_text_rich_w,
 * fb_prop_em) was rewritten twice: once when desktop/browser landed and its
 * own 16x32-atlas implementation could not compile against this tree's
 * prop16/prop24/prop32 atlases, and once here to give back what that rewrite
 * cost. Both regressions were INVISIBLE to every gate that existed:
 *
 *   - Italic silently rendered upright. browsertest asserts the app's logic,
 *     htmltest asserts the box model, and browsershot draws a picture nobody
 *     diffs pixel by pixel. `<em>` looking exactly like its surrounding text
 *     is not a crash, not a wrong number, and not a failed assertion - it is
 *     a page that looks subtly wrong to a person who is not looking for it.
 *   - Six heading sizes collapsed onto two. layout.c emits em*2, em*3/2,
 *     em*5/4, em*11/10, em and em*9/10; the old shim rounded each to the
 *     nearest of three ROLES, and at the em the browser actually runs at (24)
 *     that put h1/h2/h3 all at 32px and h4/h5/h6 all at 24px. Every numeric
 *     invariant in htmltest still held, because line COUNT does not change
 *     when a heading that already fits on one line is set too small.
 *
 * So the assertions here are about the two things a picture and a line count
 * both miss: that a requested pixel size is the size that gets drawn, and that
 * a style flag changes the pixels.
 *
 * Build and run:  ./build.sh && ./fbtext
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

/* ---- fb.c's surface, declared rather than included (there is no fb.h) ---- */
void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_clip(int x, int y, int w, int h);
void fb_clip_none(void);
unsigned int fb_get_px(int x, int y);
void fb_image(int px, int py, int w, int h,
              const unsigned int *src, int sw, int sh);

int  fb_prop_em(void);
int  fb_text_rich_w(const char *s, int len, int size, int style);
void fb_text_rich(int px, int py, const char *s, int len, unsigned int fg,
                  int size, int style);
int  fb_text_role_w(const char *s, int role, int weight);
int  fb_text_role_h(int role);

/* mirrors of fb.c's own defines - it exports no header, and a harness that
 * guessed these numbers wrong would assert against the wrong atlas */
#define TEXT_CAPTION 0
#define TEXT_BODY    1
#define TEXT_TITLE   2
#define TEXT_REGULAR 0
#define TEXT_BOLD    1

#define FBT_BOLD 1
#define FBT_ITAL 2
#define FBT_MONO 4

/* ---- fake hardware. fb.c reaches for the tick counter and the pointer ---- */
int idt_mouse_x(void)   { return -1; }
int idt_mouse_y(void)   { return -1; }
int idt_mouse_btn(void) { return 0; }
int idt_mouse_wheel(void) { return 0; }
unsigned int idt_ticks(void) { return 1; }
void idt_set_pointer_bounds(int w, int h) { (void)w; (void)h; }
void input_set_bounds(int w, int h) { (void)w; (void)h; }
/* fb.c's console path mirrors every character to the zl runtime's stdout */
void zl_putc_pub(char c) { (void)c; }

/* ---- the harness ---------------------------------------------------------- */
#define W 1600
#define H 900
#define FG 0xFFFFFF
#define BG 0x000000

static int checks, failures;

static void ok(int cond, const char *what, ...)
{
    checks++;
    if (!cond) { failures++; printf("  FAIL  %s\n", what); }
}

static void okf(int cond, const char *fmt, long a, long b)
{
    checks++;
    if (!cond) { failures++; printf("  FAIL  "); printf(fmt, a, b); printf("\n"); }
}

/* every pixel of ink in a box, as a bounding box and a count */
struct ink { int n, x0, x1, y0, y1; };

static struct ink measure_ink(int x, int y, int w, int h)
{
    struct ink k = { 0, 1 << 30, -(1 << 30), 1 << 30, -(1 << 30) };
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++)
            if (fb_get_px(xx, yy) != BG) {
                k.n++;
                if (xx < k.x0) k.x0 = xx;
                if (xx > k.x1) k.x1 = xx;
                if (yy < k.y0) k.y0 = yy;
                if (yy > k.y1) k.y1 = yy;
            }
    return k;
}

/* the mean x of the ink on the rows in [y0, y1) - the number that tells a
 * leaning stem from an upright one */
static int mean_ink_x(int x, int w, int y0, int y1)
{
    long sum = 0, n = 0;
    for (int yy = y0; yy < y1; yy++)
        for (int xx = x; xx < x + w; xx++)
            if (fb_get_px(xx, yy) != BG) { sum += xx; n++; }
    return n ? (int)(sum / n) : -1;
}

static void clear(void) { fb_fill_px(0, 0, W, H, BG); }

int main(void)
{
    /* the high-RAM arenas fb.c writes into, at the addresses it hardcodes */
    struct { unsigned long a, n; } bufs[] = {
        { 0x08000000UL, 32UL << 20 }, { 0x0A000000UL, 16UL << 20 },
        { 0x0C000000UL, 16UL << 20 },
    };
    for (unsigned i = 0; i < sizeof bufs / sizeof bufs[0]; i++) {
        void *p = mmap((void *)bufs[i].a, bufs[i].n, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        if (p != (void *)bufs[i].a) { fprintf(stderr, "mmap failed\n"); return 1; }
        memset(p, 0, bufs[i].n);
    }
    void *vram = mmap(NULL, (unsigned long)W * H * 4, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (vram == MAP_FAILED) { fprintf(stderr, "vram mmap failed\n"); return 1; }
    fb_setup((unsigned long)vram, (unsigned)W * 4, (unsigned)W, (unsigned)H, 32);
    fb_clip_none();

    const char *S = "Handgloves quickly";
    const int L = (int)strlen(S);
    const int em = fb_prop_em();

    printf("fbtext: %dx%d, em %dpx\n", W, H, em);
    ok(em > 0, "fb_prop_em() is positive");

    /* ---- 1. the six sizes layout.c actually emits stay six ---------------
     * This is the regression that shipped. Under the old role-rounding shim
     * at em 24 these produced exactly two distinct widths. */
    printf("\n1. continuous sizes (h1..h6 as layout.c emits them)\n");
    int sizes[6] = { em * 2, em * 3 / 2, em * 5 / 4, em * 11 / 10, em, em * 9 / 10 };
    int ws[6];
    for (int i = 0; i < 6; i++) ws[i] = fb_text_rich_w(S, L, sizes[i], 0);
    for (int i = 0; i < 6; i++)
        printf("   h%d  size %3dpx -> width %4d\n", i + 1, sizes[i], ws[i]);

    int distinct = 0;
    for (int i = 0; i < 6; i++) {
        int seen = 0;
        for (int j = 0; j < i; j++) if (ws[j] == ws[i]) seen = 1;
        if (!seen) distinct++;
    }
    okf(distinct == 6, "six sizes give %ld distinct widths, want %ld",
        distinct, 6L);
    /* strictly ordered: a bigger size is a wider run, with no plateau */
    for (int i = 1; i < 6; i++)
        okf(ws[i] < ws[i - 1], "width at h%ld must be < width at h%ld",
            (long)i + 1, (long)i);

    /* a size the atlases do NOT cover exactly is still honoured - this is the
     * resampler doing the work, and it is the reason no new atlas was needed */
    for (int s = 12; s <= 64; s++) {
        int a = fb_text_rich_w(S, L, s, 0), b = fb_text_rich_w(S, L, s + 4, 0);
        if (a >= b) { okf(0, "width must grow from %ldpx to %ldpx", (long)s, (long)s + 4); break; }
    }
    checks++;   /* the sweep above counts as one check when it passes */

    /* ---- 2. italic changes the pixels ------------------------------------ */
    printf("\n2. italic\n");
    int wr = fb_text_rich_w(S, L, em, 0);
    int wi = fb_text_rich_w(S, L, em, FBT_ITAL);
    printf("   regular %d, italic %d\n", wr, wi);
    okf(wi > wr, "italic run must be wider than regular (%ld vs %ld)",
        (long)wi, (long)wr);

    clear();
    fb_text_rich(40, 40, S, L, FG, em, 0);
    struct ink kr = measure_ink(0, 0, W, 200);
    unsigned int *snap = malloc((size_t)W * 200 * 4);
    for (int y = 0; y < 200; y++)
        for (int x = 0; x < W; x++) snap[y * W + x] = fb_get_px(x, y);

    clear();
    fb_text_rich(40, 40, S, L, FG, em, FBT_ITAL);
    struct ink ki = measure_ink(0, 0, W, 200);

    int differing = 0;
    for (int y = 0; y < 200; y++)
        for (int x = 0; x < W; x++)
            if (snap[y * W + x] != fb_get_px(x, y)) differing++;
    free(snap);
    okf(differing > 0, "italic must draw different pixels than regular "
        "(%ld differ, want > %ld)", (long)differing, 0L);
    ok(kr.n > 0 && ki.n > 0, "both runs actually drew ink");

    /* the lean itself: a vertical stem's ink sits further right at the top
     * than at the bottom. Regular must NOT lean. */
    const char *STEM = "lll";
    clear();
    fb_text_rich(40, 40, STEM, 3, FG, em, FBT_ITAL);
    struct ink si = measure_ink(0, 0, W, 200);
    int it_top = mean_ink_x(0, W, si.y0, si.y0 + (si.y1 - si.y0) / 3 + 1);
    int it_bot = mean_ink_x(0, W, si.y1 - (si.y1 - si.y0) / 3, si.y1 + 1);
    printf("   italic stem: mean x top %d, bottom %d\n", it_top, it_bot);
    okf(it_top > it_bot, "italic stem must lean right: top %ld > bottom %ld",
        (long)it_top, (long)it_bot);

    clear();
    fb_text_rich(40, 40, STEM, 3, FG, em, 0);
    struct ink sr = measure_ink(0, 0, W, 200);
    int rg_top = mean_ink_x(0, W, sr.y0, sr.y0 + (sr.y1 - sr.y0) / 3 + 1);
    int rg_bot = mean_ink_x(0, W, sr.y1 - (sr.y1 - sr.y0) / 3, sr.y1 + 1);
    int rg_lean = rg_top - rg_bot; if (rg_lean < 0) rg_lean = -rg_lean;
    printf("   regular stem: mean x top %d, bottom %d\n", rg_top, rg_bot);
    okf(rg_lean <= 1, "regular stem must stay upright (lean %ld px, max %ld)",
        (long)rg_lean, 1L);

    /* ---- 3. the measure agrees with the draw ------------------------------
     * A width function that disagrees with the loop that draws it wraps a line
     * where the ink does not end. Additivity is the cheap, total form of it:
     * the pen is advanced per glyph by the same function both paths call. */
    printf("\n3. measure vs draw\n");
    int styles[4] = { 0, FBT_BOLD, FBT_ITAL, FBT_BOLD | FBT_ITAL };
    for (int i = 0; i < 4; i++) {
        int a = fb_text_rich_w("Hand", 4, em, styles[i]);
        int b = fb_text_rich_w("gloves", 6, em, styles[i]);
        int ab = fb_text_rich_w("Handgloves", 10, em, styles[i]);
        okf(a + b == ab, "style %ld: w(A)+w(B) must equal w(AB) (%ld apart)",
            (long)styles[i], (long)(a + b - ab));
    }

    /* the drawn ink must fit inside the advertised width, plus the lean the
     * last glyph is allowed to add */
    for (int i = 0; i < 4; i++) {
        clear();
        int w = fb_text_rich_w(S, L, em, styles[i]);
        fb_text_rich(100, 40, S, L, FG, em, styles[i]);
        struct ink k = measure_ink(0, 0, W, 200);
        int slack = (styles[i] & FBT_ITAL) ? em / 5 + 2 : 2;
        okf(k.x1 <= 100 + w + slack,
            "style %ld: ink must end by the measured width (%ld px over)",
            (long)styles[i], (long)(k.x1 - (100 + w + slack)));
        okf(k.x0 >= 100 - 1, "style %ld: ink must not start left of the pen (%ld)",
            (long)styles[i], (long)(k.x0 - 100));
    }

    /* ---- 4. no silent truncation ------------------------------------------
     * The shim copied the run through a 512-byte stack scratch and clipped
     * anything longer. browser.c hands out substrings of a document buffer, so
     * that was a live limit, not a theoretical one - and it cost a 512-byte
     * kernel stack frame per call on top. */
    printf("\n4. long runs\n");
    static char big[4096];
    memset(big, 'a', sizeof big);
    int w1 = fb_text_rich_w(big, 1, em, 0);
    int w511 = fb_text_rich_w(big, 511, em, 0);
    int w600 = fb_text_rich_w(big, 600, em, 0);
    int w4096 = fb_text_rich_w(big, 4096, em, 0);
    printf("   1 -> %d, 511 -> %d, 600 -> %d, 4096 -> %d\n", w1, w511, w600, w4096);
    okf(w600 > w511, "a 600-char run must measure wider than a 511-char one "
        "(%ld vs %ld)", (long)w600, (long)w511);
    okf(w4096 == w1 * 4096, "width must stay linear to 4096 chars (%ld vs %ld)",
        (long)w4096, (long)(w1 * 4096));

    /* And the DRAW must not truncate either. A 600-char run is 7800px wide and
     * the screen is 1600, so it is started at x = -w511: that puts character
     * 511 - the first one the old 512-byte scratch threw away - at x = 0. The
     * old shim's ink ended exactly at x = 0, so ANY ink in the visible area is
     * a character it would have dropped. */
    clear();
    fb_text_rich(-w511, 40, big, 600, FG, em, 0);
    struct ink kb = measure_ink(0, 0, W, 200);
    okf(kb.n > 0, "characters past the old 511-char clip must be drawn "
        "(%ld inked pixels at/after char 511, want > %ld)", (long)kb.n, 0L);

    /* ---- 5. the role API and the rich API must agree --------------------- */
    printf("\n5. role API vs rich API\n");
    okf(fb_text_role_w(S, TEXT_BODY, TEXT_REGULAR) == fb_text_rich_w(S, L, em, 0),
        "role BODY/regular must equal rich at em (%ld vs %ld)",
        (long)fb_text_role_w(S, TEXT_BODY, TEXT_REGULAR),
        (long)fb_text_rich_w(S, L, em, 0));
    okf(fb_text_role_w(S, TEXT_BODY, TEXT_BOLD) == fb_text_rich_w(S, L, em, FBT_BOLD),
        "role BODY/bold must equal rich bold at em (%ld vs %ld)",
        (long)fb_text_role_w(S, TEXT_BODY, TEXT_BOLD),
        (long)fb_text_rich_w(S, L, em, FBT_BOLD));
    /* bold is a DRAWN weight here, not a double strike, but it must still be
     * wider than regular or the atlas pair got crossed */
    okf(fb_text_rich_w(S, L, em, FBT_BOLD) >= fb_text_rich_w(S, L, em, 0),
        "bold must not be narrower than regular (%ld vs %ld)",
        (long)fb_text_rich_w(S, L, em, FBT_BOLD), (long)fb_text_rich_w(S, L, em, 0));

    /* ---- 6. the lean must respect the scissor ----------------------------
     * The shear adds a per-row x offset, which is a new way to write outside
     * the box a caller clipped to. put_pixel is the scissor; this proves it. */
    printf("\n6. clipping\n");
    clear();
    fb_clip(200, 30, 120, 60);
    fb_text_rich(210, 40, S, L, FG, em, FBT_ITAL);
    fb_clip_none();
    struct ink kc = measure_ink(0, 0, W, 200);
    ok(kc.n > 0, "clipped italic still drew something");
    okf(kc.x0 >= 200 && kc.x1 < 320, "italic ink must stay inside the clip x "
        "(%ld..%ld)", (long)kc.x0, (long)kc.x1);
    okf(kc.y0 >= 30 && kc.y1 < 90, "italic ink must stay inside the clip y "
        "(%ld..%ld)", (long)kc.y0, (long)kc.y1);

    /* drawing far off both edges must be silent, not a fault */
    clear();
    fb_text_rich(-500, 40, S, L, FG, em, FBT_ITAL);
    fb_text_rich(W - 4, 40, S, L, FG, em, FBT_ITAL);
    fb_text_rich(40, -500, S, L, FG, em, FBT_ITAL);
    fb_text_rich(40, H - 2, S, L, FG, em, FBT_ITAL);
    ok(1, "drawing off every edge did not fault");

    /* ---- 7. degenerate input ---------------------------------------------- */
    printf("\n7. degenerate input\n");
    ok(fb_text_rich_w(NULL, 5, em, 0) == 0, "NULL string measures 0");
    ok(fb_text_rich_w(S, 0, em, 0) == 0, "zero length measures 0");
    ok(fb_text_rich_w(S, -3, em, 0) == 0, "negative length measures 0");
    ok(fb_text_rich_w(S, L, 0, 0) == 0, "zero size measures 0");
    ok(fb_text_rich_w(S, L, -10, 0) == 0, "negative size measures 0");
    clear();
    fb_text_rich(40, 40, NULL, 5, FG, em, 0);
    fb_text_rich(40, 40, S, 0, FG, em, 0);
    fb_text_rich(40, 40, S, L, FG, 0, 0);
    fb_text_rich(40, 40, S, L, FG, -10, 0);
    ok(measure_ink(0, 0, W, 200).n == 0, "no degenerate call drew anything");

    /* bytes outside the atlas fall back to '?' rather than indexing out */
    char wild[4] = { (char)0x01, (char)0x7F, (char)0x80, (char)0xFF };
    clear();
    fb_text_rich(40, 40, wild, 4, FG, em, FBT_ITAL);
    ok(measure_ink(0, 0, W, 200).n > 0, "out-of-range bytes render a fallback glyph");

    /* mono is the fixed-cell font: size is ignored, but the measure and the
     * draw must agree about that */
    printf("\n8. mono\n");
    int m1 = fb_text_rich_w(S, L, em, FBT_MONO);
    int m2 = fb_text_rich_w(S, L, em * 2, FBT_MONO);
    okf(m1 == m2, "mono width is size-independent, by design (%ld vs %ld)",
        (long)m1, (long)m2);
    clear();
    fb_text_rich(40, 40, S, L, FG, em, FBT_MONO);
    struct ink km = measure_ink(0, 0, W, 200);
    ok(km.n > 0, "mono drew ink");
    okf(km.x1 <= 40 + m1 + 2, "mono ink fits its measured width (%ld vs %ld)",
        (long)km.x1, (long)(40 + m1 + 2));

    /* ---- 9. fb_image: a decoded picture, scaled into its box --------------
     * THE BROWSER'S FIRST RECTANGLE OF REAL PIXELS. Every other path in fb.c
     * draws a glyph, an icon or a solid, so nothing above exercises a scale
     * or a per-pixel alpha at all.
     *
     * These assert the three things a screenshot cannot: that a DOWNSCALE
     * averages rather than throwing away eight of every nine pixels (a
     * checkerboard must come out grey, not black and not white); that a
     * transparent pixel's stored COLOUR does not bleed into its neighbours
     * (the halo every hand-rolled scaler ships with once); and that the clip
     * rectangle contains it. A picture that is subtly wrong in any of those
     * still looks like a picture. */
    printf("\n9. fb_image - a decoded picture in a box\n");
    {
        enum { SW = 32, SH = 32 };
        static unsigned int src[SW * SH];

        /* opaque red, straight copy at 1:1 */
        for (int i = 0; i < SW * SH; i++) src[i] = 0xFFFF0000u;
        clear();
        fb_image(10, 10, SW, SH, src, SW, SH);
        ok(fb_get_px(10, 10) == 0xFF0000u && fb_get_px(41, 41) == 0xFF0000u,
           "a 1:1 opaque blit did not land the corner pixels");
        ok(fb_get_px(9, 10) == BG && fb_get_px(42, 41) == BG,
           "the blit wrote outside its box");

        /* FULLY TRANSPARENT MUST DRAW NOTHING AT ALL. An alpha of 0 whose
         * stored colour is white is what an encoder actually writes, so a
         * decoder that ignored alpha would paint a white square here. */
        for (int i = 0; i < SW * SH; i++) src[i] = 0x00FFFFFFu;
        clear();
        fb_image(10, 10, SW, SH, src, SW, SH);
        ok(fb_get_px(20, 20) == BG, "a fully transparent image painted pixels");

        /* HALF ALPHA LANDS BETWEEN the background and the colour, and not at
         * either end - which is the assertion that fails if the blend is
         * dropped and the one that fails if it is applied twice. */
        for (int i = 0; i < SW * SH; i++) src[i] = 0x80FFFFFFu;
        clear();
        fb_image(10, 10, SW, SH, src, SW, SH);
        unsigned int mid = fb_get_px(20, 20) & 0xFF;
        ok(mid > (BG & 0xFF) && mid < 0xFF,
           "half-alpha white over the background gave %u, not a blend", mid);

        /* A CHECKERBOARD SCALED DOWN 8:1 MUST GO GREY. Nearest-neighbour
         * returns whichever pixel it lands on, so it produces pure black or
         * pure white and this is the assertion that catches it. */
        for (int y = 0; y < SH; y++)
            for (int x = 0; x < SW; x++)
                src[y * SW + x] = ((x ^ y) & 1) ? 0xFFFFFFFFu : 0xFF000000u;
        clear();
        fb_image(10, 10, 4, 4, src, SW, SH);
        unsigned int g = fb_get_px(11, 11) & 0xFF;
        ok(g > 0x40 && g < 0xC0,
           "an 8:1 downscaled checkerboard came out %u - not averaged", g);

        /* NO HALO. Transparent BLACK beside opaque white must average to
         * white at reduced alpha, never to grey: weighting the colour by
         * alpha is what makes that true, and dropping the weighting is the
         * classic bug. Every other pixel transparent black. */
        for (int y = 0; y < SH; y++)
            for (int x = 0; x < SW; x++)
                src[y * SW + x] = ((x ^ y) & 1) ? 0xFFFFFFFFu : 0x00000000u;
        clear();
        fb_fill_px(0, 0, W, H, 0x000000u);          /* black page, so a halo shows */
        fb_image(10, 10, 4, 4, src, SW, SH);
        unsigned int hw = fb_get_px(11, 11) & 0xFF;
        ok(hw > 0x60,
           "transparent black bled into opaque white: got %u, expected a "
           "half-alpha white near 0x80", hw);
        clear();

        /* AN UPSCALE MUST COVER ITS WHOLE BOX. Computing the source index by
         * dividing before multiplying collapses every row onto source row 0,
         * which is a blank stripe rather than a wrong pixel. */
        for (int y = 0; y < SH; y++)
            for (int x = 0; x < SW; x++)
                src[y * SW + x] = (y < SH / 2) ? 0xFF00FF00u : 0xFF0000FFu;
        clear();
        fb_image(10, 10, 64, 64, src, SW, SH);
        ok((fb_get_px(20, 20) & 0xFFFFFF) == 0x00FF00u &&
           (fb_get_px(20, 70) & 0xFFFFFF) == 0x0000FFu,
           "a 2x upscale did not map both halves of the source");

        /* THE CLIP IS A CONTAINMENT GUARANTEE, not a suggestion - browser.c
         * relies on it to keep a picture inside the viewport. */
        for (int i = 0; i < SW * SH; i++) src[i] = 0xFFFF0000u;
        clear();
        fb_clip(20, 20, 10, 10);
        fb_image(10, 10, SW, SH, src, SW, SH);
        fb_clip_none();
        ok(fb_get_px(15, 15) == BG && fb_get_px(25, 25) == 0xFF0000u,
           "fb_image escaped the clip rectangle");

        /* DEGENERATE ARGUMENTS MUST DRAW NOTHING AND NOT FAULT. A zero
         * intrinsic dimension is a real PNG failure mode and a division. */
        clear();
        fb_image(10, 10, 0, 10, src, SW, SH);
        fb_image(10, 10, 10, 0, src, SW, SH);
        fb_image(10, 10, 10, 10, src, 0, SH);
        fb_image(10, 10, 10, 10, src, SW, 0);
        fb_image(10, 10, 10, 10, 0, SW, SH);
        fb_image(-40, -40, 10, 10, src, SW, SH);
        ok(measure_ink(0, 0, W, 200).n == 0,
           "a degenerate fb_image call drew something");
    }

    printf("\n%d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
