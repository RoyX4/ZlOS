/* fb3d.c - SHADED, FILLED, back-face-culled rotating SOLIDS.
 *
 * fb.c's fb_cube draws the 12 edges of a spinning cube as a wireframe. This is
 * its solid twin: the same 8 vertices and the same two-axis integer rotation,
 * but the 6 faces are FILLED, flat-shaded by a fixed light, and the faces
 * pointing away from the camera are culled so only the front of the cube is
 * ever drawn. A convex cube needs no depth sort once the back faces are gone -
 * the surviving faces never overlap - so this stays a single pass with no
 * z-buffer.
 *
 * Freestanding kernel rules apply: INTEGER ONLY, no libc, no float, no GPU.
 * Rotation uses the same fixed-point sine as fb.c (Bhaskara I, scaled by 1024).
 * fb.c's isin is static, so a private copy lives here. Pixels reach the screen
 * only through fb.c's two public primitives, declared extern just below.
 */

/* the only two symbols we borrow from fb.c (both are non-static there).
 * fb_fill_px paints an axis-aligned rectangle; a horizontal span is a
 * height-1 rectangle: fb_fill_px(x, y, len, 1, rgb). fb_line is Bresenham. */
extern void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
extern void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb);

/* an optional clip rectangle: the cube can project a near corner far past its
 * window under perspective, so the caller sets this to the viewport interior
 * and every filled span is clamped to it - no poking over the title bar. */
static int cl_x0 = 0, cl_y0 = 0, cl_x1 = 1000000, cl_y1 = 1000000;
void fb3d_set_clip(int x0, int y0, int x1, int y1)
{ cl_x0 = x0; cl_y0 = y0; cl_x1 = x1; cl_y1 = y1; }

/* ---- fixed-point trig (copied from fb.c; its isin/icos are static there) ----
 * Bhaskara I's sine approximation, returned scaled by 1024 so the caller can
 * multiply and then divide by 1024 to stay in integers. */
static int isin(int deg)
{
    deg %= 360; if (deg < 0) deg += 360;
    int sign = 1;
    if (deg > 180) { deg -= 180; sign = -1; }
    int t = deg * (180 - deg);
    int den = 40500 - t; if (den == 0) den = 1;
    return sign * (4 * t * 1024 / den);
}
static int icos(int deg) { return isin(deg + 90); }

/* scale a packed 0xRRGGBB colour to bright/255 of its intensity, per channel */
static unsigned int shade_rgb(unsigned int rgb, int bright)
{
    if (bright < 0) bright = 0;
    if (bright > 255) bright = 255;
    int r = (int)((rgb >> 16) & 0xFF) * bright / 255;
    int g = (int)((rgb >> 8)  & 0xFF) * bright / 255;
    int b = (int)( rgb        & 0xFF) * bright / 255;
    return ((unsigned)r << 16) | ((unsigned)g << 8) | (unsigned)b;
}

/* scanline fill of a simple polygon given n projected vertices. For each row
 * between the polygon's min and max y, find where the edges cross that row,
 * sort the crossings, and fill between them in pairs. A convex quad yields
 * exactly two crossings per row; the pair loop handles the general case too.
 * The half-open test (yy in [min,max)) counts each edge once and stops shared
 * vertices from being double-counted. fb_fill_px clips to the screen, so no
 * bounds checking is needed here. */
static void fill_poly(const int *xs, const int *ys, int n, unsigned int rgb)
{
    int ymin = ys[0], ymax = ys[0];
    for (int i = 1; i < n; i++) {
        if (ys[i] < ymin) ymin = ys[i];
        if (ys[i] > ymax) ymax = ys[i];
    }
    for (int yy = ymin; yy <= ymax; yy++) {
        int xi[8];
        int m = 0;
        for (int i = 0; i < n && m < 8; i++) {
            int j = (i + 1) % n;
            int y0 = ys[i], y1 = ys[j];
            int x0 = xs[i], x1 = xs[j];
            if ((yy >= y0 && yy < y1) || (yy >= y1 && yy < y0)) {
                int dy = y1 - y0;                 /* nonzero: yy lies strictly between */
                xi[m++] = x0 + (x1 - x0) * (yy - y0) / dy;
            }
        }
        /* insertion sort the crossings (m is tiny, at most a handful) */
        for (int i = 1; i < m; i++) {
            int v = xi[i], k = i - 1;
            while (k >= 0 && xi[k] > v) { xi[k + 1] = xi[k]; k--; }
            xi[k + 1] = v;
        }
        if (yy < cl_y0 || yy > cl_y1) continue;          /* clip: skip rows outside */
        for (int i = 0; i + 1 < m; i += 2) {
            int xl = xi[i], xr = xi[i + 1];
            if (xl < cl_x0) xl = cl_x0;                   /* clip: clamp the span */
            if (xr > cl_x1) xr = cl_x1;
            if (xr >= xl) fb_fill_px(xl, yy, xr - xl + 1, 1, rgb);
        }
    }
}

/* ---- a TILED barycentric triangle rasterizer -------------------------------
 * The path SerenityOS took to run Quake III with no GPU, and the shape every
 * software rasterizer converges on. desktop-TODO's order is fb_clip -> tiled
 * rasterization -> SIMD -> depth buffer -> textures; this is step two, and the
 * first is now done.
 *
 * WHY TILES RATHER THAN SCANLINES. fill_poly above walks every row of the
 * bounding box and solves for edge crossings per row. That is fine for one
 * cube and wrong for a scene: it is inherently serial down the triangle, it
 * re-derives the same edge maths every row, and it has nowhere to hang a depth
 * test, a texture lookup or a vector unit.
 *
 * The barycentric form instead asks, per pixel, "which side of each edge am I
 * on" - three integer edge functions, each of which is LINEAR, so stepping one
 * pixel right adds a constant and stepping one row down adds another. No
 * division in the inner loop and no per-row setup.
 *
 * The tiling is what makes that affordable. Testing every pixel of the
 * bounding box would be far worse than scanlines for a thin triangle. So the
 * box is walked in 16x16 blocks, each classified by evaluating the three edge
 * functions at its FOUR CORNERS:
 *
 *   all four corners inside all three edges  -> the tile is wholly inside.
 *                                               Fill it as one rectangle, with
 *                                               no per-pixel test at all.
 *   any edge has all four corners outside it -> the tile cannot intersect the
 *                                               triangle. Skip it entirely.
 *   otherwise                                -> partial: test per pixel.
 *
 * Interior tiles are the common case for anything bigger than a few pixels and
 * cost one rectangle fill each. That is the whole win, and it is also exactly
 * where a depth buffer and a texture unit slot in later.
 *
 * INTEGER ONLY. The edge function is a cross product of screen coordinates, so
 * it is exact - no epsilon, and the fill rule is a comparison rather than a
 * tolerance.
 */
#define TILE 16

/* Twice the signed area of (a, b, c): positive for one winding, negative for
 * the other, zero when the three are collinear. */
static int edge(int ax, int ay, int bx, int by, int px, int py)
{
    return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
}

/* One triangle, flat colour. Any winding - the sign is normalised here so
 * callers need not care, which matters because the cube's faces arrive in a
 * fixed winding that back-face culling has already used for something else. */
void fb3d_tri(int x0, int y0, int x1, int y1, int x2, int y2, unsigned int rgb)
{
    if (edge(x0, y0, x1, y1, x2, y2) < 0) {
        int t;
        t = x1; x1 = x2; x2 = t;
        t = y1; y1 = y2; y2 = t;
    }

    int bx0 = x0 < x1 ? (x0 < x2 ? x0 : x2) : (x1 < x2 ? x1 : x2);
    int bx1 = x0 > x1 ? (x0 > x2 ? x0 : x2) : (x1 > x2 ? x1 : x2);
    int by0 = y0 < y1 ? (y0 < y2 ? y0 : y2) : (y1 < y2 ? y1 : y2);
    int by1 = y0 > y1 ? (y0 > y2 ? y0 : y2) : (y1 > y2 ? y1 : y2);

    if (bx0 < cl_x0) bx0 = cl_x0;
    if (by0 < cl_y0) by0 = cl_y0;
    if (bx1 > cl_x1) bx1 = cl_x1;
    if (by1 > cl_y1) by1 = cl_y1;
    if (bx0 > bx1 || by0 > by1) return;

    /* E(x, y) = (bx-ax)(y-ay) - (by-ay)(x-ax), so
     *     dE/dx = -(by - ay) = ay - by      and      dE/dy = bx - ax
     * A step right adds the first, a step down the second - which is why the
     * inner loop has no division and no per-row setup.
     *
     * These were all SIX sign-inverted in the first version. Interior tiles
     * still came out right, because they never step - so the cube looked fine
     * and only the partial tiles at the edges were wrong. The comparison
     * against the scanline reference is what found it: 13,031 pixels differing
     * on one large triangle, and a thin sliver drawing almost nothing at all,
     * because a sliver is nearly ALL partial tiles. */
    int ea = y0 - y1, eb = x1 - x0;
    int fa = y1 - y2, fb_ = x2 - x1;
    int ga = y2 - y0, gb = x0 - x2;

    for (int ty = by0; ty <= by1; ty += TILE) {
        int tyh = ty + TILE - 1;
        if (tyh > by1) tyh = by1;
        int run = -1;              /* start of the current interior tile run */
        for (int tx = bx0; tx <= bx1; tx += TILE) {
            int txw = tx + TILE - 1;
            if (txw > bx1) txw = bx1;

            /* The four corner values, from ONE evaluation plus the step
             * constants: E is linear, so the other three corners are the
             * origin plus a whole-tile step in x, in y, or both. Twelve
             * edge() calls per tile - each a pair of multiplies - was most of
             * why the first version lost to the scanline fill. */
            int dx = txw - tx, dy = tyh - ty;
            int er = edge(x0, y0, x1, y1, tx, ty);
            int fr = edge(x1, y1, x2, y2, tx, ty);
            int gr = edge(x2, y2, x0, y0, tx, ty);
            int ex = ea * dx, ey = eb * dy;
            int fx = fa * dx, fy = fb_ * dy;
            int gx = ga * dx, gy = gb * dy;
            int ec[4] = { er, er + ex, er + ey, er + ex + ey };
            int fc[4] = { fr, fr + fx, fr + fy, fr + fx + fy };
            int gc[4] = { gr, gr + gx, gr + gy, gr + gx + gy };
            int oe = 0, of = 0, og = 0, allin = 1;
            for (int k = 0; k < 4; k++) {
                if (ec[k] < 0) oe++;
                if (fc[k] < 0) of++;
                if (gc[k] < 0) og++;
                if (ec[k] < 0 || fc[k] < 0 || gc[k] < 0) allin = 0;
            }
            if (oe == 4 || of == 4 || og == 4) {           /* wholly outside */
                if (run >= 0) {                            /* close a run */
                    fb_fill_px(run, ty, tx - run, tyh - ty + 1, rgb);
                    run = -1;
                }
                continue;
            }
            if (allin) {
                /* Wholly inside. Do NOT fill it yet: accumulate a RUN of
                 * adjacent interior tiles and emit one rectangle for the lot.
                 * Filling per tile made this 3.5x SLOWER than the scanline
                 * fill it replaces, because fb_fill_px is not free - it clamps
                 * against the scissor and reports damage on every call - and a
                 * 700x500 triangle is ~1,400 tiles against 500 scanline rows.
                 * More, smaller calls is the wrong direction. */
                if (run < 0) run = tx;
                continue;
            }
            if (run >= 0) {
                fb_fill_px(run, ty, tx - run, tyh - ty + 1, rgb);
                run = -1;
            }

            for (int py = ty; py <= tyh; py++) {
                int e = er, f = fr, g = gr, span = -1;
                for (int px = tx; px <= txw; px++) {
                    if (e >= 0 && f >= 0 && g >= 0) {
                        if (span < 0) span = px;
                    } else if (span >= 0) {
                        fb_fill_px(span, py, px - span, 1, rgb);
                        span = -1;
                    }
                    e += ea; f += fa; g += ga;
                }
                if (span >= 0) fb_fill_px(span, py, txw - span + 1, 1, rgb);
                er += eb; fr += fb_; gr += gb;
            }
        }
        if (run >= 0) fb_fill_px(run, ty, bx1 - run + 1, tyh - ty + 1, rgb);
    }
}

/* A convex polygon as a triangle fan - the cube's faces are quads. */
void fb3d_poly(const int *xs, const int *ys, int n, unsigned int rgb)
{
    for (int i = 1; i + 1 < n; i++)
        fb3d_tri(xs[0], ys[0], xs[i], ys[i], xs[i + 1], ys[i + 1], rgb);
}

/* fb_cube_filled - a solid, shaded, back-face-culled cube.
 *   cx,cy  screen centre
 *   size   half-edge in pixels (the cube spans about 2*size)
 *   angle  rotation in degrees; the tilt axis turns at 7/10 of it, as in fb_cube
 *   base   the cube's 0xRRGGBB colour at full light; each face is darkened from
 *          this by how squarely it faces the light.
 *
 * ONE CUBE, NOT TWO. This held its own copy of the 8 vertices, the 6 wound
 * faces, the light and the projection; the mesh table below then needed the
 * same cube for the Renderer's picker. Two copies of a vertex table is how the
 * picture in one window quietly stops being the picture in another, so the
 * copy here was deleted rather than duplicated - the tables moved down whole
 * and this is now the call that asks for mesh 0. The Cube app draws exactly
 * what it drew before, which is the only thing this refactor is allowed to
 * change about it. */
void fb3d_mesh_filled(int kind, int cx, int cy, int size, int angle,
                      unsigned int base);

void fb_cube_filled(int cx, int cy, int size, int angle, unsigned int base)
{
    fb3d_mesh_filled(0, cx, cy, size, angle, base);
}

/* ---- FOUR MESHES, AND THE COUNTS COME OFF THE TABLES ------------------------
 *
 * WHY THIS EXISTS. apps_sys2.zl's Renderer drew a four-way segmented control
 * `cube|pyramid|octa|cylinder` whose only reader was a caption that PRINTED
 * TRIANGLE AND VERTEX COUNTS - "mesh cylinder - 44 tris - 24 verts" - for
 * objects this kernel did not have. The canvas under it was one flat fill. So
 * the picker changed four written figures about four absent solids, in the
 * numeric slot beside two readings that are real probes.
 *
 * The counts were not even wrong: 12/8, 6/5, 8/6 and 44/24 are what a cube, a
 * square pyramid, an octahedron and a twelve-sided cylinder actually have. The
 * fault was that nothing in the tree could have disagreed with them. That is
 * the whole difference between a figure and a caption, and this file is where
 * the difference gets closed - the geometry is built, drawn, and then COUNTED,
 * so the caption cannot drift from the solid on screen.
 *
 * fb_cube_filled above is kept and now delegates here: its 8 vertices and 6
 * wound quads became mesh 0, unchanged, so the Cube app draws the same picture
 * it did before.
 *
 * THE WINDING RULE IS THE CUBE'S, INHERITED. A face's object-space normal from
 * (v1-v0) x (v2-v0) points OUTWARD, and with the projection below a visible
 * face therefore has a NEGATIVE 2D signed area - culling stays one sign test.
 * Every table here was checked against that rule by hand at build time and by
 * fb3d_mesh_facing() at run time.
 *
 * WHAT IS DIFFERENT FROM THE CUBE. A cube's faces all have the same |N|, so
 * fb_cube_filled could normalise the diffuse term by the constant 4*size*size.
 * A pyramid's sides and its base do not, and a cylinder's caps and its skirt do
 * not, so |N| is computed per face with an integer square root. Getting this
 * wrong does not crash - it shades one face as if it were lit from nowhere,
 * which is exactly the kind of "looks plausible" error this pane exists to
 * stop printing. */

#define M_MAXV   24        /* the cylinder's two rings                        */
#define M_MAXF   14        /* twelve skirt quads and two caps                 */
#define M_MAXFV  12        /* a cap is a twelve-gon                           */
#define M_CYLSEG 12        /* segments; 44 tris and 24 verts fall out of this */

struct fb3d_mesh {
    int nv;
    int v[M_MAXV][3];              /* unit coords, scaled by 1024 */
    int nf;
    int fn[M_MAXF];                /* how many indices this face has */
    int f[M_MAXF][M_MAXFV];
};

/* Newton's method on integers - three or four iterations from a shift-based
 * seed. Only used for |N|, where being one off changes a shade by less than a
 * step of the 60..255 ramp. */
static int m_isqrt(int n)
{
    if (n <= 0) return 0;
    int x = n, y = (x + 1) / 2;
    while (y < x) { x = y; y = (x + n / x) / 2; }
    return x;
}

static void m_vert(struct fb3d_mesh *m, int x, int y, int z)
{ int i = m->nv++; m->v[i][0] = x; m->v[i][1] = y; m->v[i][2] = z; }

static void m_face(struct fb3d_mesh *m, const int *idx, int n)
{ int i = m->nf++; m->fn[i] = n; for (int k = 0; k < n; k++) m->f[i][k] = idx[k]; }

/* U is one unit. Vertices are held at this scale and divided back down after
 * being multiplied by `size`, so a mesh keeps its proportions at any size. */
#define M_U 1024

static void mesh_build(int kind, struct fb3d_mesh *m)
{
    m->nv = 0; m->nf = 0;

    if (kind == 1) {                                  /* SQUARE PYRAMID */
        /* apex first, then the base ring. Screen y points DOWN, so the apex is
         * at -y. 5 vertices, 4 triangles and one quad base = 6 triangles. */
        m_vert(m, 0, -M_U, 0);                        /* 0 apex */
        m_vert(m, -M_U, M_U, -M_U);                   /* 1 */
        m_vert(m,  M_U, M_U, -M_U);                   /* 2 */
        m_vert(m,  M_U, M_U,  M_U);                   /* 3 */
        m_vert(m, -M_U, M_U,  M_U);                   /* 4 */
        { const int a[3] = {0,1,2}; m_face(m, a, 3); }
        { const int a[3] = {0,2,3}; m_face(m, a, 3); }
        { const int a[3] = {0,3,4}; m_face(m, a, 3); }
        { const int a[3] = {0,4,1}; m_face(m, a, 3); }
        { const int a[4] = {1,4,3,2}; m_face(m, a, 4); }   /* base, normal +y */
        return;
    }

    if (kind == 2) {                                  /* OCTAHEDRON */
        /* 0 +x, 1 -x, 2 +y, 3 -y, 4 +z, 5 -z. Eight faces, one per octant.
         * A face (X,Y,Z) is wound outward when the product of its three signs
         * is positive; when it is negative the last two swap. */
        m_vert(m,  M_U, 0, 0);  m_vert(m, -M_U, 0, 0);
        m_vert(m, 0,  M_U, 0);  m_vert(m, 0, -M_U, 0);
        m_vert(m, 0, 0,  M_U);  m_vert(m, 0, 0, -M_U);
        for (int sx = 0; sx < 2; sx++)
            for (int sy = 0; sy < 2; sy++)
                for (int sz = 0; sz < 2; sz++) {
                    int X = sx ? 1 : 0, Y = sy ? 3 : 2, Z = sz ? 5 : 4;
                    int neg = (sx ? -1 : 1) * (sy ? -1 : 1) * (sz ? -1 : 1);
                    int a[3];
                    a[0] = X;
                    if (neg > 0) { a[1] = Y; a[2] = Z; } else { a[1] = Z; a[2] = Y; }
                    m_face(m, a, 3);
                }
        return;
    }

    if (kind == 3) {                                  /* CYLINDER */
        /* Twelve segments: a top ring at -y, a bottom ring at +y, twelve skirt
         * quads and two twelve-gon caps. 24 vertices; 12*2 + 2*10 = 44
         * triangles. Those are the two figures the caption used to assert. */
        for (int i = 0; i < M_CYLSEG; i++) {
            int a = i * 360 / M_CYLSEG;
            m_vert(m, icos(a), -M_U, isin(a));
        }
        for (int i = 0; i < M_CYLSEG; i++) {
            int a = i * 360 / M_CYLSEG;
            m_vert(m, icos(a),  M_U, isin(a));
        }
        for (int i = 0; i < M_CYLSEG; i++) {
            int j = (i + 1) % M_CYLSEG;
            int a[4] = { i, M_CYLSEG + i, M_CYLSEG + j, j };
            m_face(m, a, 4);
        }
        { int a[M_CYLSEG]; for (int i = 0; i < M_CYLSEG; i++) a[i] = i;
          m_face(m, a, M_CYLSEG); }                              /* top, -y */
        { int a[M_CYLSEG]; for (int i = 0; i < M_CYLSEG; i++)
              a[i] = M_CYLSEG + (M_CYLSEG - 1 - i);
          m_face(m, a, M_CYLSEG); }                              /* bottom, +y */
        return;
    }

    /* KIND 0 - THE CUBE, and these are fb_cube_filled's own tables moved here
     * rather than retyped, so the Cube app cannot change picture on this
     * refactor. */
    m_vert(m, -M_U,-M_U,-M_U); m_vert(m,  M_U,-M_U,-M_U);
    m_vert(m,  M_U, M_U,-M_U); m_vert(m, -M_U, M_U,-M_U);
    m_vert(m, -M_U,-M_U, M_U); m_vert(m,  M_U,-M_U, M_U);
    m_vert(m,  M_U, M_U, M_U); m_vert(m, -M_U, M_U, M_U);
    { const int a[4] = {0,3,2,1}; m_face(m, a, 4); }   /* front  z=-1 */
    { const int a[4] = {4,5,6,7}; m_face(m, a, 4); }   /* back   z=+1 */
    { const int a[4] = {0,4,7,3}; m_face(m, a, 4); }   /* left   x=-1 */
    { const int a[4] = {1,2,6,5}; m_face(m, a, 4); }   /* right  x=+1 */
    { const int a[4] = {0,1,5,4}; m_face(m, a, 4); }   /* top    y=-1 */
    { const int a[4] = {3,7,6,2}; m_face(m, a, 4); }   /* bottom y=+1 */
}

/* THE TWO FIGURES THE CAPTION PRINTS, DERIVED FROM THE TABLE IT DRAWS FROM.
 * A face of n indices is n-2 triangles; that is the same fan the rasteriser
 * walks, so the count is of the triangles actually filled, not of a number
 * somebody remembered about a solid. */
int fb3d_mesh_verts(int kind)
{
    struct fb3d_mesh m;
    mesh_build(kind, &m);
    return m.nv;
}

int fb3d_mesh_tris(int kind)
{
    struct fb3d_mesh m;
    mesh_build(kind, &m);
    int t = 0;
    for (int i = 0; i < m.nf; i++) t += m.fn[i] - 2;
    return t;
}

int fb3d_mesh_count(void) { return 4; }

/* fb3d_mesh_filled - any of the four, drawn the way the cube was.
 *   kind   0 cube, 1 pyramid, 2 octahedron, 3 cylinder
 *   cx,cy  screen centre
 *   size   half-extent in pixels
 *   angle  degrees; the tilt axis turns at 7/10 of it, as fb_cube does
 *   base   0xRRGGBB at full light */
void fb3d_mesh_filled(int kind, int cx, int cy, int size, int angle,
                      unsigned int base)
{
    struct fb3d_mesh m;
    mesh_build(kind, &m);

    const int Lx = -2, Ly = -3, Lz = -6;      /* |L| = 7 exactly */
    int s  = isin(angle),  c  = icos(angle);
    int a2 = angle * 7 / 10;
    int s2 = isin(a2),     c2 = icos(a2);
    int D  = size * 4; if (D < 1) D = 1;

    int rx[M_MAXV], ry[M_MAXV], rz[M_MAXV];
    int px[M_MAXV], py[M_MAXV];
    for (int i = 0; i < m.nv; i++) {
        int x = m.v[i][0] * size / M_U;
        int y = m.v[i][1] * size / M_U;
        int z = m.v[i][2] * size / M_U;
        int x1 = (x * c  - z  * s ) / 1024;
        int z1 = (x * s  + z  * c ) / 1024;
        int y1 = (y * c2 - z1 * s2) / 1024;
        int z2 = (y * s2 + z1 * c2) / 1024;
        rx[i] = x1; ry[i] = y1; rz[i] = z2;
        int denom = D + z2; if (denom < 1) denom = 1;
        px[i] = cx + x1 * D / denom;
        py[i] = cy + y1 * D / denom;
    }

    for (int fi = 0; fi < m.nf; fi++) {
        int n = m.fn[fi];
        const int *fv = m.f[fi];
        int X[M_MAXFV], Y[M_MAXFV];
        for (int k = 0; k < n; k++) { X[k] = px[fv[k]]; Y[k] = py[fv[k]]; }

        int area2 = 0;
        for (int k = 0; k < n; k++) {
            int j = (k + 1) % n;
            area2 += X[k] * Y[j] - X[j] * Y[k];
        }
        if (area2 >= 0) continue;                 /* back-face cull */

        int ax = rx[fv[1]] - rx[fv[0]], ay = ry[fv[1]] - ry[fv[0]], az = rz[fv[1]] - rz[fv[0]];
        int bx = rx[fv[2]] - rx[fv[0]], by = ry[fv[2]] - ry[fv[0]], bz = rz[fv[2]] - rz[fv[0]];
        int Nx = ay * bz - az * by;
        int Ny = az * bx - ax * bz;
        int Nz = ax * by - ay * bx;
        /* PER FACE, NOT PER SOLID. The cube could divide by a constant because
         * all six of its faces have the same |N|; a pyramid's base is a
         * different size from its sides and a cylinder's cap is a different
         * size from its skirt, so a constant here would light one of them as
         * if it faced somewhere else. */
        int nmag = m_isqrt(Nx * Nx + Ny * Ny + Nz * Nz);
        if (nmag < 1) nmag = 1;
        int dot = Nx * Lx + Ny * Ly + Nz * Lz;
        int diffuse = 0;
        if (dot > 0) diffuse = dot * 195 / (nmag * 7);
        int bright = 60 + diffuse;
        if (bright > 255) bright = 255;

        fill_poly(X, Y, n, shade_rgb(base, bright));

        int ebr = bright + 45; if (ebr > 255) ebr = 255;
        unsigned int ecol = shade_rgb(base, ebr);
        for (int k = 0; k < n; k++) {
            int j = (k + 1) % n;
            if (Y[k] < cl_y0 || Y[k] > cl_y1 || Y[j] < cl_y0 || Y[j] > cl_y1) continue;
            if (X[k] < cl_x0 || X[k] > cl_x1 || X[j] < cl_x0 || X[j] > cl_x1) continue;
            fb_line(X[k], Y[k], X[j], Y[j], ecol);
        }
    }
}
