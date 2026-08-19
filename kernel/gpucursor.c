/* gpucursor.c - the pointer, drawn by the display engine instead of by us.
 *
 * WHY THIS IS WORTH HAVING, and it is not fill rate.
 *
 * Today the pointer is a software sprite: `fb.c` saves the pixels under it,
 * draws two coverage planes over them, and restores them on the next move. Every
 * mouse movement is a read, a composite and a write, inside the frame loop, and
 * it can only move as often as the compositor runs - which is a 100 Hz tick
 * against a panel measured at 59.998 Hz.
 *
 * Gen9's display engine has a cursor PLANE. It composites a 64x64 ARGB image
 * over the primary plane AT SCANOUT, for free, and moving it is one register
 * write. The pointer stops costing pixels and stops being tied to frame rate.
 *
 * `intel.c` already has all three registers (`intel_cursor_enable`,
 * `intel_cursor_move`, `intel_cursor_disable`) and `runtime_kernel.c` already
 * publishes them to zl as `cur_on`, `cur_move` and `cur_off`. **Nothing calls
 * any of it**, and there is no cursor image anywhere, so `cur_on` would point
 * the display engine at an unmapped graphics address. This file is the missing
 * middle: an image, in memory the GPU can reach, in the format the plane wants.
 *
 * HONEST STATUS: the compositing arithmetic below is exact and tested. The
 * install path writes display registers and HAS NEVER RUN - it is gated behind
 * `gpu_cursor_arm()`, off by default, exactly like gpuring.c and intel.c's
 * `lt_armed`. No panel power is touched; the cursor plane is not panel power.
 *
 * WHAT IS NOT SETTLED, and must be before this is trusted on hardware: whether
 * Gen9's cursor plane wants PREMULTIPLIED or straight alpha. This file produces
 * BOTH from one pass and says which is which, because guessing would give a
 * cursor with a dark halo and no obvious cause.
 */

#include "memmap.h"
#include "cursor.inc"

typedef unsigned int       gc_u32;
typedef unsigned char      gc_u8;
typedef unsigned long long gc_u64;

#if defined(ZL_64) || defined(__x86_64__)
typedef unsigned long long gc_uptr;
#else
typedef unsigned int       gc_uptr;
#endif

int intel_present(void);
int intel_supported(void);
int intel_cursor_enable(gc_u32 gfx_addr, int size64);
int intel_cursor_move(int x, int y);
int intel_cursor_disable(void);
int intel_ggtt_map(gc_u32 gfx_page, gc_u32 phys_addr);

/* ---- where the image lives ----------------------------------------------
 * HI_GPU is gpuring.c's region: 4 MiB for a 4 KiB ring. The cursor image is
 * 64x64x4 = 16 KiB and goes immediately after it, declared here rather than
 * picked out of the air, so the two cannot drift into each other. */
#define GPU_CURSOR_DIM   64u
#define GPU_CURSOR_BYTES (GPU_CURSOR_DIM * GPU_CURSOR_DIM * 4u)
#define GPU_CURSOR_PHYS  ((gc_u64)HI_GPU + 4096u)          /* after the ring */
#define GPU_CURSOR_GFX   0x04001000u                        /* ring gfx + one page */

_Static_assert(GPU_CURSOR_PHYS + GPU_CURSOR_BYTES <= (gc_u64)HI_BLUR,
               "the cursor image runs past HI_GPU into the blur arena");

/* ---- the compositing, which is the part that can be tested ---------------
 *
 * `fb.c` draws the pointer as two coverage planes over whatever is behind it:
 *
 *     bg' = bg*(1-b) + edge*b        the dilated silhouette, dark
 *     bg'' = bg'*(1-f) + fill*f      the interior, light, on top
 *
 * Expanding gives the single source-over layer the display engine needs:
 *
 *     A = 1 - (1-b)(1-f)
 *     C = edge*b*(1-f) + fill*f      <- PREMULTIPLIED by A
 *
 * That C is premultiplied because it is the contribution to the final pixel,
 * not the colour of the cursor at that point. Straight alpha wants C/A, and
 * handing a premultiplied image to a plane expecting straight alpha (or the
 * reverse) produces a cursor with a dark fringe - a symptom with no obvious
 * cause. Hence `premul`, and hence saying so here.
 */
/* Non-static ON PURPOSE. Every shipped cursor asset has fill coverage <= body
 * coverage everywhere (checked: 0 pixels violate it across all four kinds), so
 * `alpha = b` is arithmetically identical to the correct combine FOR THESE
 * ASSETS - and a test driven by the assets cannot tell the two apart. It was
 * not able to: that mutation survived a 102-check suite. Exposing the combine
 * lets the test drive coverage values the assets do not contain, which is the
 * only way to pin the general formula rather than the current artwork. */
gc_u32 gpu_cursor_pixel(gc_u8 b, gc_u8 f, gc_u32 fill, gc_u32 edge, int premul)
{
    /* alpha, in 0..255, from the two coverages */
    gc_u32 a = 255u - (((255u - b) * (255u - f)) / 255u);
    if (a == 0) return 0;                       /* fully transparent: all zero */

    gc_u32 out = a << 24;
    for (int sh = 0; sh <= 16; sh += 8) {
        gc_u32 e = (edge >> sh) & 0xFFu;
        gc_u32 l = (fill >> sh) & 0xFFu;
        /* premultiplied contribution: edge*b*(1-f) + fill*f */
        gc_u32 c = (e * b * (255u - f)) / (255u * 255u) + (l * f) / 255u;
        if (!premul) c = (c * 255u) / a;        /* straight alpha wants C/A */
        if (c > 255u) c = 255u;
        out |= c << sh;
    }
    return out;
}

/* Build a 64x64 ARGB cursor from the 32x32 coverage assets, scaled by `scale`
 * (1 or 2). Returns the number of non-transparent pixels, which is a cheap way
 * for a caller - or a test - to notice it produced an empty cursor. */
gc_u32 gpu_cursor_build(gc_u32 *argb, int kind, gc_u32 fill, gc_u32 edge,
                        int scale, int premul)
{
    if (!argb) return 0;
    if ((unsigned)kind >= CUR_N) return 0;
    if (scale != 1 && scale != 2) return 0;

    for (gc_u32 i = 0; i < GPU_CURSOR_DIM * GPU_CURSOR_DIM; i++) argb[i] = 0;

    gc_u32 opaque = 0;
    gc_u32 span = (gc_u32)CUR_SZ32 * (gc_u32)scale;
    if (span > GPU_CURSOR_DIM) return 0;

    for (gc_u32 y = 0; y < span; y++) {
        for (gc_u32 x = 0; x < span; x++) {
            gc_u32 sx = x / (gc_u32)scale, sy = y / (gc_u32)scale;
            gc_u8 b = cur_body32[kind][sy][sx];
            gc_u8 f = cur_fill32[kind][sy][sx];
            gc_u32 px = gpu_cursor_pixel(b, f, fill, edge, premul);
            argb[y * GPU_CURSOR_DIM + x] = px;
            if (px >> 24) opaque++;
        }
    }
    return opaque;
}

/* ---- the hardware half: never executed ----------------------------------- */

static int cursor_armed = 0;
static int cursor_live  = 0;

void gpu_cursor_arm(int on) { cursor_armed = on ? 1 : 0; }
int  gpu_cursor_is_live(void) { return cursor_live; }

/* Build the image, put it where the display engine can reach it, and enable
 * the plane. Returns 1 if every step reported success. */
int gpu_cursor_install(int kind, gc_u32 fill, gc_u32 edge, int scale, int premul)
{
    cursor_live = 0;
    if (!cursor_armed) return 0;
    if (!intel_present() || !intel_supported()) return 0;

    gc_u32 *img = (gc_u32 *)(gc_uptr)GPU_CURSOR_PHYS;
    if (!gpu_cursor_build(img, kind, fill, edge, scale, premul)) return 0;

    /* 16 KiB is four pages, and every one of them needs a GGTT entry. Mapping
     * only the first is a cursor whose top quarter is correct and whose rest is
     * whatever those stale entries point at. */
    for (gc_u32 p = 0; p * 4096u < GPU_CURSOR_BYTES; p++)
        if (!intel_ggtt_map((GPU_CURSOR_GFX >> 12) + p,
                            (gc_u32)(GPU_CURSOR_PHYS + p * 4096u))) return 0;

    if (!intel_cursor_enable(GPU_CURSOR_GFX, 1)) return 0;   /* 1 = 64x64 */
    cursor_live = 1;
    return 1;
}

/* One register write per pointer move. This is the whole point. */
int gpu_cursor_move(int x, int y)
{
    if (!cursor_live) return 0;
    return intel_cursor_move(x, y);
}

int gpu_cursor_off(void)
{
    cursor_live = 0;
    if (!cursor_armed) return 0;
    return intel_cursor_disable();
}
