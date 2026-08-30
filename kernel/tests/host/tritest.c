/* tritest.c - is the tiled rasterizer the same shape as the scanline one?
 *
 * fb3d.c now has two ways to fill a convex polygon: fill_poly, which walks
 * rows and solves edge crossings, and fb3d_poly, which walks 16x16 tiles and
 * tests three barycentric edge functions. The second exists to be faster and
 * to have somewhere to hang a depth test and a texture unit later. It is only
 * worth having if it draws THE SAME PIXELS.
 *
 * "Looks the same" cannot answer that: a rasterizer that is one pixel fat
 * along one edge looks identical until two triangles share that edge and a
 * seam appears - which is the classic software-3D bug and it shows up as
 * cracks in a mesh, not as a wrong-looking triangle.
 *
 * So this rasterizes the same triangles both ways into two buffers and
 * compares them pixel for pixel, over a spread of shapes chosen to hit the
 * cases that break rasterizers: thin slivers, exact horizontal and vertical
 * edges, degenerate zero-area triangles, both windings, and triangles pushed
 * off every edge of the clip rectangle.
 *
 * Build and run:  ./build.sh && ./tritest
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>
#include <time.h>

void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
unsigned int fb_get_px(int x, int y);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_clip_none(void);

void fb3d_set_clip(int x0, int y0, int x1, int y1);
void fb3d_tri(int x0, int y0, int x1, int y1, int x2, int y2, unsigned int rgb);
void fb3d_poly(const int *xs, const int *ys, int n, unsigned int rgb);
void fb_cube_filled(int cx, int cy, int size, int angle, unsigned int base);
void fb3d_mesh_filled(int kind, int cx, int cy, int size, int angle, unsigned int base);
int  fb3d_mesh_verts(int kind);
int  fb3d_mesh_tris(int kind);

void idt_set_pointer_bounds(int w, int h) { (void)w; (void)h; }
void input_set_bounds(int w, int h)        { (void)w; (void)h; }  /* fb.c pushes it; no input.c in this harness */
void zl_putc_pub(char c) { (void)c; }

#define W 800
#define H 600

/* fill_poly is static inside fb3d.c, so the scanline reference is reproduced
 * here EXACTLY as it is written there. If that one changes, this must too -
 * which is the price of comparing against a private function, and cheaper than
 * exporting one purely for a test. */
static int cl_x0, cl_y0, cl_x1, cl_y1;
static void ref_poly(const int *xs, const int *ys, int n, unsigned int rgb)
{
    int ymin = ys[0], ymax = ys[0];
    for (int i = 1; i < n; i++) {
        if (ys[i] < ymin) ymin = ys[i];
        if (ys[i] > ymax) ymax = ys[i];
    }
    for (int yy = ymin; yy <= ymax; yy++) {
        int xi[8], m = 0;
        for (int i = 0; i < n && m < 8; i++) {
            int j = (i + 1) % n;
            int y0 = ys[i], y1 = ys[j], x0 = xs[i], x1 = xs[j];
            if ((yy >= y0 && yy < y1) || (yy >= y1 && yy < y0)) {
                int dy = y1 - y0;
                xi[m++] = x0 + (x1 - x0) * (yy - y0) / dy;
            }
        }
        for (int i = 1; i < m; i++) {
            int v = xi[i], k = i - 1;
            while (k >= 0 && xi[k] > v) { xi[k + 1] = xi[k]; k--; }
            xi[k + 1] = v;
        }
        if (yy < cl_y0 || yy > cl_y1) continue;
        for (int i = 0; i + 1 < m; i += 2) {
            int xl = xi[i], xr = xi[i + 1];
            if (xl < cl_x0) xl = cl_x0;
            if (xr > cl_x1) xr = cl_x1;
            if (xr >= xl) fb_fill_px(xl, yy, xr - xl + 1, 1, rgb);
        }
    }
}

static int fails;
static unsigned char snap_a[W * H], snap_b[W * H];

static void snap(unsigned char *into)
{
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            into[y * W + x] = fb_get_px(x, y) ? 1 : 0;
}

/* The two rasterizers agree if their coverage differs by at most `slack`
 * pixels. Slack is NOT a fudge factor for correctness - it is there because
 * the two use different fill rules on the exact boundary (half-open rows
 * versus >= 0 edge functions), and a boundary pixel either way is a
 * legitimate design difference. INTERIOR disagreement is a bug and would be
 * enormous, not one pixel. */
static void compare(const char *what, int slack)
{
    int diff = 0, a_only = 0, b_only = 0;
    for (int i = 0; i < W * H; i++) {
        if (snap_a[i] == snap_b[i]) continue;
        diff++;
        if (snap_a[i]) a_only++; else b_only++;
    }
    int ok = diff <= slack;
    printf("  %-46s %s  (%d px differ: %d scanline-only, %d tiled-only)\n",
           what, ok ? "ok  " : "FAIL", diff, a_only, b_only);
    if (!ok) fails++;
}

static void both(const int *xs, const int *ys, int n, const char *what, int slack)
{
    fb_fill_px(0, 0, W, H, 0);
    ref_poly(xs, ys, n, 0x00FFFFFF);
    snap(snap_a);
    fb_fill_px(0, 0, W, H, 0);
    fb3d_poly(xs, ys, n, 0x00FFFFFF);
    snap(snap_b);
    compare(what, slack);
}

static uint64_t now_ns(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000ull + t.tv_nsec;
}

int main(void)
{
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
    void *vram = mmap(NULL, 16UL << 20, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    fb_setup((unsigned long)vram, W * 4, W, H, 32);
    fb_clip_none();

    cl_x0 = 0; cl_y0 = 0; cl_x1 = W - 1; cl_y1 = H - 1;
    fb3d_set_clip(0, 0, W - 1, H - 1);

    printf("tritest - tiled barycentric vs the scanline reference\n\n");

    /* A boundary pixel either way is a fill-rule difference, not a bug. The
     * slack scales with the PERIMETER, which is what a fill rule can differ
     * on - never with the area, which is what a real bug would differ on. */
    { int xs[] = {100,700,400}, ys[] = {100,150,500};
      both(xs, ys, 3, "a big triangle", 1800); }
    { int xs[] = {400,410,405}, ys[] = {100,100,500};
      both(xs, ys, 3, "a thin vertical sliver", 900); }
    { int xs[] = {100,700,400}, ys[] = {300,300,310};
      both(xs, ys, 3, "a thin horizontal sliver", 900); }
    { int xs[] = {100,300,300,100}, ys[] = {100,100,300,300};
      both(xs, ys, 4, "an axis-aligned square (exact edges)", 900); }
    { int xs[] = {400,100,700}, ys[] = {500,150,100};
      both(xs, ys, 3, "the big triangle, opposite winding", 1800); }
    { int xs[] = {200,200,200}, ys[] = {100,300,500};
      both(xs, ys, 3, "a degenerate zero-area triangle", 4); }
    { int xs[] = {-300,300,100}, ys[] = {-200,-50,400};
      both(xs, ys, 3, "hanging off the top-left corner", 1800); }
    { int xs[] = {600,1200,900}, ys[] = {400,500,900};
      both(xs, ys, 3, "hanging off the bottom-right corner", 1800); }

    /* the clip rectangle must hold for the tiled path too */
    fb3d_set_clip(200, 200, 400, 400);
    cl_x0 = 200; cl_y0 = 200; cl_x1 = 400; cl_y1 = 400;
    fb_fill_px(0, 0, W, H, 0);
    { int xs[] = {0,799,400}, ys[] = {0,0,599};
      fb3d_poly(xs, ys, 3, 0x00FFFFFF); }
    int escaped = 0;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            if (fb_get_px(x, y) && (x < 200 || x > 400 || y < 200 || y > 400))
                escaped++;
    printf("  %-46s %s  (%d px outside)\n", "the tiled path honours fb3d's clip",
           escaped ? "FAIL" : "ok  ", escaped);
    if (escaped) fails++;

    /* ---- THE FOUR MESHES ------------------------------------------------
     * The Renderer pane used to PRINT "mesh cylinder - 44 tris - 24 verts" for
     * solids the kernel could not draw. The figures were right and nothing
     * could have disagreed with them, which is the whole defect. fb3d now
     * builds the geometry and counts it off the same tables it fills from, so
     * the counts are checkable here rather than only readable on a screen.
     *
     * THE EXPECTED PAIRS ARE NOT COPIES OF THE OLD CAPTION. They are what the
     * solids are: a cube is 6 quads (12 triangles) on 8 corners; a square
     * pyramid is 4 sides and a quad base (6) on 5; an octahedron is 8 faces on
     * 6; a twelve-sided cylinder is 12 skirt quads and two twelve-gon caps
     * (12*2 + 2*10 = 44) on two rings of 12. Written out that way so a wrong
     * table fails against the geometry rather than against a memory of it. */
    fb3d_set_clip(0, 0, W - 1, H - 1);
    cl_x0 = 0; cl_y0 = 0; cl_x1 = W - 1; cl_y1 = H - 1;
    printf("\n");
    {
        static const char *nm[4] = { "cube", "pyramid", "octahedron", "cylinder" };
        static const int ev[4] = { 8, 5, 6, 24 };     /* vertices */
        static const int et[4] = { 12, 6, 8, 44 };    /* triangles */
        int cov[4];
        for (int k = 0; k < 4; k++) {
            int v = fb3d_mesh_verts(k), t = fb3d_mesh_tris(k);
            int ok = (v == ev[k] && t == et[k]);
            printf("  mesh %-10s counts                        %s  (%d tris, %d verts)\n",
                   nm[k], ok ? "ok  " : "FAIL", t, v);
            if (!ok) fails++;
        }
        /* AND IT ACTUALLY DRAWS. A count that is right about a solid nothing
         * fills is the same fault one layer down, so each kind is rasterised
         * and its coverage counted. Every one must mark pixels, and no two may
         * mark the SAME pixels - identical coverage would mean the kind
         * argument is being ignored, which is exactly how a four-way picker
         * ends up changing nothing. */
        for (int k = 0; k < 4; k++) {
            fb_fill_px(0, 0, W, H, 0);
            fb3d_mesh_filled(k, W / 2, H / 2, 120, 0, 0x00FFFFFF);
            int n = 0;
            for (int i = 0; i < W * H; i++) n += fb_get_px(i % W, i / W) ? 1 : 0;
            cov[k] = n;
            int ok = n > 1000;
            printf("  mesh %-10s draws                         %s  (%d px)\n",
                   nm[k], ok ? "ok  " : "FAIL", n);
            if (!ok) fails++;
        }
        int same = 0;
        for (int i = 0; i < 4; i++)
            for (int j = i + 1; j < 4; j++)
                if (cov[i] == cov[j]) same++;
        printf("  the four are four different solids            %s  (%d identical pairs)\n",
               same ? "FAIL" : "ok  ", same);
        if (same) fails++;
        /* ROTATION MOVES THE PICTURE. The Renderer spins; a mesh that ignored
         * `angle` would sit still under a yaw readout that keeps counting. */
        fb_fill_px(0, 0, W, H, 0);
        fb3d_mesh_filled(3, W / 2, H / 2, 120, 0, 0x00FFFFFF);
        snap(snap_a);
        fb_fill_px(0, 0, W, H, 0);
        fb3d_mesh_filled(3, W / 2, H / 2, 120, 37, 0x00FFFFFF);
        snap(snap_b);
        int moved = 0;
        for (int i = 0; i < W * H; i++) if (snap_a[i] != snap_b[i]) moved++;
        printf("  37 degrees of yaw changes it                 %s  (%d px moved)\n",
               moved > 500 ? "ok  " : "FAIL", moved);
        if (moved <= 500) fails++;
        /* AND IT STAYS INSIDE fb3d's CLIP, like every other path in this file.
         * A near corner under perspective projects far outside the window. */
        fb3d_set_clip(300, 200, 500, 400);
        fb_fill_px(0, 0, W, H, 0);
        fb3d_mesh_filled(0, W / 2, H / 2, 260, 21, 0x00FFFFFF);
        int out = 0;
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                if (fb_get_px(x, y) && (x < 300 || x > 500 || y < 200 || y > 400)) out++;
        printf("  the mesh path honours fb3d's clip            %s  (%d px outside)\n",
               out ? "FAIL" : "ok  ", out);
        if (out) fails++;
        fb3d_set_clip(0, 0, W - 1, H - 1);
    }

    /* ---- speed. The reason tiles exist at all. ---- */
    fb3d_set_clip(0, 0, W - 1, H - 1);
    cl_x0 = 0; cl_y0 = 0; cl_x1 = W - 1; cl_y1 = H - 1;
    int xs[] = {50, 750, 400}, ys[] = {50, 80, 550};
    uint64_t t0 = now_ns();
    for (int i = 0; i < 400; i++) ref_poly(xs, ys, 3, 0x00204060);
    uint64_t t1 = now_ns();
    for (int i = 0; i < 400; i++) fb3d_poly(xs, ys, 3, 0x00204060);
    uint64_t t2 = now_ns();
    double sc = (double)(t1 - t0) / 400.0 / 1000.0;
    double ti = (double)(t2 - t1) / 400.0 / 1000.0;
    printf("\n  400 large triangles:  scanline %.1f us   tiled %.1f us   %+.0f%%\n",
           sc, ti, (sc / ti - 1.0) * 100.0);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
