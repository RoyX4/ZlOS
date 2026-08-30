/* imgen.c - the three pictures the Image Viewer names.
 *
 * WHY THIS EXISTS. apps_sys3.zl's Image Viewer drew three file tabs -
 * "gradient.ppm", "plasma.ppm", "mandelbrot.pgm" - and four zoom pills over a
 * canvas that was one flat fill. s3iv_sel and s3iv_zoom each had exactly three
 * occurrences in the whole tree: the declaration, the setter, and the test that
 * lit the pill. Nothing else read either. Clicking a tab lit it and the stat
 * strip twelve pixels below still read "IMAGE  none loaded", because that
 * string was passed unconditionally.
 *
 * The pane cited ds-reference.html, where the canvas is blank because ds.html
 * imports seven sibling modules that were never delivered - so loadImage()
 * never runs. Cloning the blankness of a document that is blank BY ACCIDENT is
 * how a missing feature becomes a specification.
 *
 * THERE IS NO IMAGE DECODER IN THIS KERNEL AND THESE THREE DO NOT NEED ONE.
 * A gradient, a plasma and a Mandelbrot set are COMPUTED, not read: no PPM
 * parser, no PGM parser, no file. That is also why the tabs lost their
 * extensions - a name ending in .ppm claims a file on a volume, and there is
 * none. They are pictures this machine can make, named for what they are.
 *
 * Freestanding rules: integer only, no libc, no float, no GPU. Pixels reach the
 * screen through fb.c's fb_fill_px, which clips to the scissor, so the canvas
 * rectangle needs no bounds checking here.
 */

extern void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);

/* Bhaskara I's sine, scaled by 1024 - fb.c's and fb3d.c's are both static, so
 * this is the third copy and the comment in fb3d.c explaining why applies
 * unchanged: zl cannot include a header and C cannot reach a static. */
static int ig_sin(int deg)
{
    deg %= 360; if (deg < 0) deg += 360;
    int sign = 1;
    if (deg > 180) { deg -= 180; sign = -1; }
    int t = deg * (180 - deg);
    int den = 40500 - t; if (den == 0) den = 1;
    return sign * (4 * t * 1024 / den);
}

/* scale a packed 0xRRGGBB to bright/255 of its intensity, per channel */
static unsigned int ig_shade(unsigned int rgb, int bright)
{
    if (bright < 0) bright = 0;
    if (bright > 255) bright = 255;
    int r = (int)((rgb >> 16) & 0xFF) * bright / 255;
    int g = (int)((rgb >> 8)  & 0xFF) * bright / 255;
    int b = (int)( rgb        & 0xFF) * bright / 255;
    return ((unsigned)r << 16) | ((unsigned)g << 8) | (unsigned)b;
}

/* THE SOURCE GRID IS THE CANVAS DIVIDED BY THE ZOOM, which is what makes the
 * zoom pill mean something: at 1x one source pixel is one screen pixel and the
 * picture carries the most detail the canvas can hold; at 4x it is a 4x4 block
 * and the same picture is nine times cheaper and visibly coarser. The two
 * figures the stat strip prints are these, so they cannot disagree with what
 * was drawn. */
int img_src_w(int w, int px) { if (px < 1) px = 1; return w / px; }
int img_src_h(int h, int px) { if (px < 1) px = 1; return h / px; }

/* MANDELBROT ITERATION CEILING. Every source pixel costs up to this many
 * 64-bit multiplies and the whole picture is recomputed on every damage - a
 * window drag repaints it per frame on a CPU with no FPU behind it. 24 is
 * where the set's outline is fully resolved at the sizes this pane uses; the
 * banding beyond it is interior detail nobody can see at 3x. It is one number
 * and it is named so the cost can be moved without hunting the loop. */
#define IG_MAXIT 24

/* 16.16 fixed point. zr and zi stay under 2.0 (131072), which fits an int, but
 * their PRODUCTS are 34 bits - so every multiply widens to long long first.
 * Doing that in int is the classic silent overflow here: the set fills with
 * noise at the edges and still looks like a Mandelbrot set. */
#define IG_ONE 65536

static int ig_mandel(int sx, int sy, int sw, int sh)
{
    if (sw < 1) sw = 1;
    if (sh < 1) sh = 1;
    int x0 = -2 * IG_ONE - IG_ONE / 5 + (int)((long long)sx * 3 * IG_ONE / sw);
    int y0 = -(IG_ONE + IG_ONE / 5) + (int)((long long)sy * 12 * IG_ONE / 5 / sh);
    int zr = 0, zi = 0, it = 0;
    while (it < IG_MAXIT) {
        int zr2 = (int)(((long long)zr * zr) >> 16);
        int zi2 = (int)(((long long)zi * zi) >> 16);
        if (zr2 + zi2 > 4 * IG_ONE) break;
        int t = (int)(((long long)zr * zi) >> 15);
        zr = zr2 - zi2 + x0;
        zi = t + y0;
        it++;
    }
    return it;
}

/* img_draw - one of the three, into the rectangle given.
 *   kind  0 gradient, 1 plasma, 2 mandelbrot
 *   px    the zoom: how many screen pixels wide one source pixel is
 *   base  0xRRGGBB at full light - the pane passes its own accent, so the
 *         picture stays inside the ladder rather than importing a palette */
void img_draw(int kind, int x, int y, int w, int h, int px, unsigned int base)
{
    if (px < 1) px = 1;
    int sw = img_src_w(w, px), sh = img_src_h(h, px);
    if (sw < 1 || sh < 1) return;

    for (int sy = 0; sy < sh; sy++) {
        for (int sx = 0; sx < sw; sx++) {
            int v;
            if (kind == 1) {
                /* three sine terms across x, y and the diagonal - the classic
                 * demoscene plasma, in integers. Each term is +-1024, so the
                 * sum is +-3072 and maps onto 0..255 by a single divide. */
                int a = ig_sin(sx * 1080 / sw);
                int b = ig_sin(sy * 1800 / sh);
                int c = ig_sin((sx + sy) * 1440 / (sw + sh));
                v = (a + b + c + 3072) * 255 / 6144;
            } else if (kind == 2) {
                int it = ig_mandel(sx, sy, sw, sh);
                /* inside the set is ink-black; outside ramps by escape time */
                v = (it >= IG_MAXIT) ? 0 : (30 + it * 225 / IG_MAXIT);
            } else {
                /* a two-axis ramp: x carries most of it, y shades it */
                v = (sx * 255 / (sw > 1 ? sw - 1 : 1)) * 3 / 4
                  + (sy * 255 / (sh > 1 ? sh - 1 : 1)) / 4;
            }
            fb_fill_px(x + sx * px, y + sy * px, px, px, ig_shade(base, v));
        }
    }
}
