#!/usr/bin/env python3
# gen_icons_rgb.py - the zlOS desktop icon set again, this time in FULL COLOUR.
#
# gen_icons.py emits SHAPES: one byte of coverage per pixel, and the caller
# picks a colour. That is the right answer for a glyph and the wrong answer for
# an app icon - a folder is amber, a terminal is not, and a single-colour
# silhouette cannot say so. This emits the picture instead:
#
#   icons_rgb[N][32][32][4]   - R,G,B,A per pixel, STRAIGHT (not premultiplied)
#
# STRAIGHT alpha is deliberate. fb.c already owns a correct compositor -
# blend_rgb(bg, fg, a) mixes fg into bg by coverage a in LINEAR light - and it
# wants an honest fg colour plus a separate coverage. Premultiplied RGB would
# have to be divided back out in the kernel, per pixel, with no divider worth
# the name. So the division happens here, once, in float, at build time.
#
# Everything is still DRAWN, not imported: geometry at 4x (128x128), box-filtered
# to 32x32. No font, no emoji, no bitmap file to parse. Retune by editing the
# numbers below and re-running.
#
# Two things make this harder than the coverage version:
#
#   1. Downsampling RGBA is not "resize the four channels". Outside the shape
#      the colour bytes are 0, so averaging colour independently of alpha drags
#      every edge pixel toward black and the icons come out with a dirty halo.
#      The box filter therefore runs on PREMULTIPLIED colour, and the result is
#      un-premultiplied afterwards - which is the only way an edge pixel ends up
#      holding the shape's real colour at partial coverage.
#
#   2. Depth has to be built, not implied. Each icon is a small coherent palette
#      with a vertical gradient down its body, a 1px lighter band on its top and
#      left edges and a 1px darker band on its bottom and right edges. The light
#      is at the top-left for the whole set, so the icons read as one family of
#      objects rather than ten unrelated stickers. The edge bands are derived
#      from the shape's own mask (mask minus mask-shifted-by-1px), so they follow
#      any silhouette - including the INSIDE of a hole, where the shading comes
#      out inverted for free, which is exactly what a hole looks like.
#
# Master edges are hard: PIL's draw is aliased, so at 128x128 every mask is
# strictly 0 or 255 and every colour is exact. All the antialiasing comes from
# the box filter, so an edge pixel's alpha is a true area fraction (17 levels)
# and its colour is the true average of the covered sub-pixels.

import os
from math import cos, sin, radians, sqrt, pi

import numpy as np
from PIL import Image, ImageChops, ImageDraw

KERNEL_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(KERNEL_ROOT, "src", "graphics", "icons", "icons_rgb.c")

SS = 4                       # supersample factor
N = 32                       # final icon size, px
S = N * SS                   # master size, 128
PAD = 2 * SS                 # 2px of breathing room inside the 32x32 box
# PIL's rectangle/ellipse bounds are INCLUSIVE, so the far edge is S-PAD-1.
LO, HI = PAD, S - PAD - 1    # usable master box: 8..119

BEVEL = SS                   # 1px edge band, in master pixels
SIDE_STRENGTH = 110          # left/right bands are weaker than top/bottom (0..255)


# ---- colour ---------------------------------------------------------------

def rgb(h):
    return ((h >> 16) & 0xFF, (h >> 8) & 0xFF, h & 0xFF)


def mix(a, b, t):
    return tuple(int(round(a[i] + (b[i] - a[i]) * t)) for i in range(3))


def lit(c, t):
    """toward white - a highlight"""
    return mix(c, (255, 255, 255), t)


def dim(c, t):
    """toward black - a shadow"""
    return mix(c, (0, 0, 0), t)


# ---- the master canvas ----------------------------------------------------

def _gradient(top, bot, y0, y1):
    """a full-canvas RGBA image whose rows ramp top->bot between y0 and y1"""
    ys = np.arange(S, dtype=np.float64)
    t = np.clip((ys - y0) / max(1.0, float(y1 - y0)), 0.0, 1.0)[:, None]
    col = np.array(top, float)[None, :] * (1.0 - t) + np.array(bot, float)[None, :] * t
    arr = np.zeros((S, S, 4), np.uint8)
    arr[:, :, :3] = np.round(col)[:, None, :].repeat(S, axis=1)
    arr[:, :, 3] = 255
    return Image.fromarray(arr, "RGBA")


def _solid(c):
    return Image.new("RGBA", (S, S), tuple(c) + (255,))


class Canvas:
    """An RGBA master. Shapes are built as binary masks and then painted, so a
    shape's colour and a shape's silhouette are never entangled: the same mask
    can carry a gradient, a bevel and a punched hole."""

    def __init__(self):
        self.img = Image.new("RGBA", (S, S), (0, 0, 0, 0))

    @staticmethod
    def mask():
        m = Image.new("L", (S, S), 0)
        return m, ImageDraw.Draw(m)

    def fill(self, mask, top, bot=None, box=None):
        """paint a vertical gradient through `mask`.

        The ramp spans the SHAPE's own vertical extent by default, so every part
        of every icon gets the same amount of shading regardless of its size."""
        if bot is None:
            bot = top
        bb = box
        if bb is None:
            g = mask.getbbox()
            if g is None:
                return
            bb = (g[1], g[3] - 1)
        self.img.paste(_gradient(top, bot, bb[0], bb[1]), (0, 0), mask)

    def flat(self, mask, c, strength=255):
        """paint one colour through `mask`; strength<255 blends with what is
        already there (the underlying gradient), which is how the side bands
        stay subtle without needing to know the body colour at that row."""
        m = mask if strength >= 255 else mask.point(lambda v: v * strength // 255)
        self.img.paste(_solid(c), (0, 0), m)

    def bevel(self, mask, hi, lo, k=BEVEL, side=None):
        """1px lighter top+left, 1px darker bottom+right - light from top-left.

        Derived from the mask itself: `mask - mask shifted down by k` is exactly
        the shape's top edge, whatever the shape is. Bottom/right go down first
        so that at a corner the highlight wins, which is what a lit edge does."""
        if side is None:
            side = max(1, k // 2)
        off = ImageChops.offset
        sub = ImageChops.subtract
        self.flat(sub(mask, off(mask, -side, 0)), lo, SIDE_STRENGTH)   # right
        self.flat(sub(mask, off(mask, 0, -k)), lo)                     # bottom
        self.flat(sub(mask, off(mask, side, 0)), hi, SIDE_STRENGTH)    # left
        self.flat(sub(mask, off(mask, 0, k)), hi)                      # top

    def punch(self, mask):
        """cut a real hole - alpha 0, the desktop shows through"""
        self.img.paste((0, 0, 0, 0), (0, 0), mask)

    def body(self, mask, top, bot, hi=0.30, lo=0.28, k=BEVEL):
        """the common case: gradient + bevel, highlight/shadow derived from the
        body colours so every icon is lit by the same rule"""
        self.fill(mask, top, bot)
        self.bevel(mask, lit(top, hi), dim(bot, lo), k)


# ---- mask geometry (all strokes are drawn at fill=255) --------------------

def mdot(md, x, y, r):
    md.ellipse([x - r, y - r, x + r, y + r], fill=255)


def mpath(md, pts, width, caps=True):
    """open polyline with round joints and optional round caps"""
    md.line(list(pts), fill=255, width=int(round(width)), joint="curve")
    r = width / 2.0
    for (x, y) in (pts if caps else pts[1:-1]):
        mdot(md, x, y, r)


def polar(cx, cy, r, deg):
    a = radians(deg)
    return (cx + r * cos(a), cy + r * sin(a))


def band(mask, y0, y1):
    """mask AND the horizontal slab y0..y1 - for a header band that has to
    inherit the page's rounded top corners instead of poking out of them"""
    slab, sd = Canvas.mask()
    sd.rectangle([0, y0, S, y1], fill=255)
    return ImageChops.multiply(mask, slab)


def union(*masks):
    out = masks[0]
    for m in masks[1:]:
        out = ImageChops.lighter(out, m)
    return out


def minus(a, b):
    return ImageChops.subtract(a, b)


# ---- the icons ------------------------------------------------------------
# Each takes a Canvas and paints the 128x128 master. Order is the ORDER OF THE
# ARRAY - see the note in main() about index drift from icons24.

def icon_terminal(c):
    """dark slate console, green prompt. A bezel with an inset screen, so the
    thing reads as a physical box: the screen's shading is the body's shading
    upside down (dark at the top edge), which is what a recess looks like."""
    body_t, body_b = rgb(0x3E4B5C), rgb(0x232C38)
    scr_t, scr_b = rgb(0x151C24), rgb(0x0E141B)
    grn = rgb(0x4ADE80)

    m, d = Canvas.mask()
    d.rounded_rectangle([LO, 16, HI, 111], radius=16, fill=255)
    c.body(m, body_t, body_b)

    sm, sd = Canvas.mask()
    sd.rounded_rectangle([18, 26, 109, 101], radius=9, fill=255)
    c.fill(sm, scr_t, scr_b)
    c.bevel(sm, dim(scr_t, 0.45), lit(body_b, 0.16))     # inverted = inset

    g, gd = Canvas.mask()
    mpath(gd, [(36, 50), (58, 66), (36, 82)], 10)        # the chevron
    mpath(gd, [(70, 82), (92, 82)], 9)                   # the cursor rule
    c.fill(g, lit(grn, 0.28), dim(grn, 0.22))
    c.bevel(g, lit(grn, 0.55), dim(grn, 0.45), k=2)


def icon_files(c):
    """amber folder: a darker back panel with a tab, a brighter front flap in
    front of it. Two panels, not one outline - the overlap IS the depth cue."""
    back_t, back_b = rgb(0xC98A21), rgb(0x9A6612)
    fr_t, fr_b = rgb(0xF7C948), rgb(0xD79420)

    b, bd = Canvas.mask()
    bd.rounded_rectangle([12, 26, 62, 62], radius=8, fill=255)     # the tab
    bd.rounded_rectangle([12, 38, 116, 104], radius=10, fill=255)  # the back
    c.body(b, back_t, back_b)

    f, fd = Canvas.mask()
    fd.rounded_rectangle([LO, 54, HI, 111], radius=12, fill=255)   # the flap
    c.body(f, fr_t, fr_b)


def icon_editor(c):
    """white page, blue header band, grey text rules"""
    pg_t, pg_b = rgb(0xFFFFFF), rgb(0xDDE2E9)
    hd_t, hd_b = rgb(0x3B8BDA), rgb(0x2A5C93)
    rule = rgb(0x9AA7B5)

    p, pd = Canvas.mask()
    pd.rounded_rectangle([20, 10, 108, 118], radius=8, fill=255)
    c.body(p, pg_t, pg_b, hi=0.0, lo=0.22)               # white cannot go lighter

    h = band(p, 0, 44)                                   # inherits the corners
    c.fill(h, hd_t, hd_b)
    c.bevel(h, lit(hd_t, 0.30), dim(hd_b, 0.30))

    r, rd = Canvas.mask()
    for y, x1 in ((58, 96), (72, 96), (86, 96), (100, 80)):
        rd.rectangle([32, y, x1, y + 5], fill=255)
    c.fill(r, rule, dim(rule, 0.10))


def icon_paint(c):
    """a brush at 45 degrees: wood handle, silver ferrule, loaded red tip.
    Three materials in a row is what sells it - the mono version could only
    say "wide, wider, widest" with the silhouette."""
    wood_t, wood_b = rgb(0xC08A46), rgb(0x74461D)
    fer_t, fer_b = rgb(0xEDF1F6), rgb(0x94A1AF)
    tip_t, tip_b = rgb(0xF05252), rgb(0xA8221E)

    ax, ay, bx, by = 24.0, 112.0, 110.0, 26.0            # tip end -> handle end
    L = sqrt((bx - ax) ** 2 + (by - ay) ** 2)
    ux, uy = (bx - ax) / L, (by - ay) / L
    nx, ny = -uy, ux

    def P(t, off):
        return (ax + ux * t + nx * off, ay + uy * t + ny * off)

    h, hd = Canvas.mask()
    mpath(hd, [P(44, 0), P(L, 0)], 12)
    c.fill(h, wood_t, wood_b)
    c.bevel(h, lit(wood_t, 0.32), dim(wood_b, 0.30), k=3)

    f, fd = Canvas.mask()
    fd.polygon([P(27, -13), P(48, -13), P(48, 13), P(27, 13)], fill=255)
    c.fill(f, fer_t, fer_b)
    c.bevel(f, lit(fer_t, 0.35), dim(fer_b, 0.32), k=3)

    t, td = Canvas.mask()
    td.polygon([P(30, -18), P(30, 18), P(9, 13), P(9, -13)], fill=255)
    mdot(td, *P(9, -12), r=3.2)
    mdot(td, *P(9, 12), r=3.2)                           # round the two tip corners
    c.fill(t, tip_t, tip_b)
    c.bevel(t, lit(tip_t, 0.35), dim(tip_b, 0.30), k=3)


def icon_snake(c):
    """a tapered body coiled into a spiral, with a darker outline.

    Same coil as the mono icon (pitch = 2*pi*K must clear body+gap, or at this
    size the spiral collapses into a disc), but the segment notches now cut the
    BODY only. The outline is a second, fatter stamp underneath, so a notch
    exposes the dark green instead of punching a hole in the icon."""
    out_c = rgb(0x14532D)
    bod_t, bod_b = rgb(0x5EE58F), rgb(0x1F9A55)

    cx, cy = S / 2, S / 2 + 5
    R0, K, TURNS, PHASE, STEPS = 6.67, 3.81, 1.5, 2.356, 500
    RAD = lambda fr: 4.3 + 3.1 * fr
    pts, arc = [], [0.0]
    for i in range(STEPS + 1):
        t = TURNS * 2 * pi * i / STEPS
        r = R0 + K * t
        pts.append((cx + r * cos(t + PHASE), cy + r * sin(t + PHASE)))
        if i:
            (x0, y0), (x1, y1) = pts[-2], pts[-1]
            arc.append(arc[-1] + sqrt((x1 - x0) ** 2 + (y1 - y0) ** 2))

    o, od = Canvas.mask()
    b, bd = Canvas.mask()
    for i, (x, y) in enumerate(pts):                     # taper: PIL cannot vary
        r = RAD(i / STEPS)                               # a line's width
        mdot(od, x, y, r + 2.2)
        mdot(bd, x, y, r)
    hx, hy = pts[-1]
    mdot(od, hx, hy, 12.8)
    mdot(bd, hx, hy, 10.6)

    # Notch by ARC LENGTH (the spiral's step grows with radius) and scale the
    # notch half-height to the LOCAL body radius, or it slices the next coil.
    SPACING, next_cut = 32.0, 32.0
    for i in range(2, len(pts) - 38):
        if arc[i] < next_cut:
            continue
        next_cut += SPACING
        (x0, y0), (x1, y1) = pts[i - 1], pts[i + 1]
        dx, dy = x1 - x0, y1 - y0
        ln = sqrt(dx * dx + dy * dy) or 1.0
        nxv, nyv = -dy / ln, dx / ln
        hh = RAD(i / STEPS) + 1.0
        px, py = pts[i]
        bd.line([(px - nxv * hh, py - nyv * hh), (px + nxv * hh, py + nyv * hh)],
                fill=0, width=4)

    c.fill(o, lit(out_c, 0.16), out_c)
    c.body(b, bod_t, bod_b, hi=0.26, lo=0.20, k=3)

    e, ed = Canvas.mask()                                # the eye, on the head
    ux, uy = hx - cx, hy - cy
    ln = sqrt(ux * ux + uy * uy) or 1.0
    mdot(ed, hx + ux / ln * 3.6, hy + uy / ln * 3.6, 3.6)
    c.fill(e, out_c, out_c)


def icon_cube3d(c):
    """an isometric cube: three rhombic faces in three shades of one indigo.

    The face shades ARE the light direction - top brightest, left mid, right
    darkest - so this icon carries the set's lighting rule most literally."""
    base = rgb(0x4F5BD5)
    top_f = (lit(base, 0.52), lit(base, 0.38))
    left_f = (lit(base, 0.06), dim(base, 0.14))
    right_f = (dim(base, 0.34), dim(base, 0.48))

    cx = cy = S / 2
    R = 47
    hexa = [polar(cx, cy, R, 90 + 60 * k) for k in range(6)]

    silo, sd = Canvas.mask()
    sd.polygon(hexa, fill=255)
    c.fill(silo, lit(base, 0.20), dim(base, 0.30))       # under-paint: no seams

    for pts, (ct, cb) in (
        ([(cx, cy), hexa[2], hexa[3], hexa[4]], top_f),      # top face
        ([(cx, cy), hexa[0], hexa[1], hexa[2]], left_f),     # left face
        ([(cx, cy), hexa[4], hexa[5], hexa[0]], right_f),    # right face
    ):
        fm, fd = Canvas.mask()
        fd.polygon(pts, fill=255)
        c.fill(fm, ct, cb)

    c.bevel(silo, lit(base, 0.72), dim(base, 0.62))


def icon_monitor(c):
    """a system monitor: dark panel on a stand, cyan bars on a recessed screen"""
    st_t, st_b = rgb(0x64707F), rgb(0x39424E)
    pan_t, pan_b = rgb(0x3C4757), rgb(0x1E252E)
    scr_t, scr_b = rgb(0x141A21), rgb(0x0D1218)
    bar = rgb(0x22D3EE)

    s, sd = Canvas.mask()
    sd.rectangle([56, 92, 71, 108], fill=255)                       # neck
    sd.rounded_rectangle([42, 104, 85, 117], radius=5, fill=255)    # foot
    c.body(s, st_t, st_b, k=3)

    m, d = Canvas.mask()
    d.rounded_rectangle([LO, 11, HI, 99], radius=12, fill=255)
    c.body(m, pan_t, pan_b)

    sm, smd = Canvas.mask()
    smd.rounded_rectangle([17, 20, 110, 90], radius=8, fill=255)
    c.fill(sm, scr_t, scr_b)
    c.bevel(sm, dim(scr_t, 0.40), lit(pan_b, 0.18))                 # inset

    for x0, top in ((32, 66), (56, 48), (80, 32)):
        bm, bmd = Canvas.mask()
        bmd.rectangle([x0, top, x0 + 15, 82], fill=255)
        c.fill(bm, lit(bar, 0.30), dim(bar, 0.34))
        c.bevel(bm, lit(bar, 0.62), dim(bar, 0.52), k=3)


def icon_about(c):
    """blue disc, white i"""
    b_t, b_b = rgb(0x54ABF2), rgb(0x1F5FA8)

    m, d = Canvas.mask()
    d.ellipse([LO, LO, HI, HI], fill=255)
    c.body(m, b_t, b_b, hi=0.34, lo=0.30)

    g, gd = Canvas.mask()
    mdot(gd, 64, 40, 7.0)
    gd.rounded_rectangle([57, 56, 70, 98], radius=6, fill=255)
    c.fill(g, rgb(0xFFFFFF), rgb(0xE3EEF9))
    c.bevel(g, rgb(0xFFFFFF), dim(rgb(0xE3EEF9), 0.16), k=2)


def icon_settings(c):
    """a steel gear: a disc with eight capsule teeth and a real hole.

    Filled, not outlined - colour buys a silhouette the coverage version could
    not afford. The teeth are capsules on a disc rather than a toothed outline
    so every gap stays ~4px wide at 32px instead of mushing into a blob. The hub
    is punched to alpha 0, and the bevel shades its inner wall the other way up
    on its own, which is what makes it read as a hole and not a grey dot."""
    st_t, st_b = rgb(0xDCE3EB), rgb(0x8494A6)

    cx = cy = S / 2
    m, d = Canvas.mask()
    d.ellipse([cx - 40, cy - 40, cx + 40, cy + 40], fill=255)
    for i in range(8):
        a = 45.0 * i
        mpath(d, [polar(cx, cy, 30, a), polar(cx, cy, 46, a)], 17, caps=False)
        mdot(d, *polar(cx, cy, 46, a), r=8.0)

    hole, hd = Canvas.mask()
    hd.ellipse([cx - 14, cy - 14, cx + 14, cy + 14], fill=255)

    gear = minus(m, hole)
    c.body(gear, st_t, st_b)
    c.punch(hole)


def icon_power(c):
    """the IEC power mark: broken ring plus a stem through the gap"""
    p_t, p_b = rgb(0xFB923C), rgb(0xC81E1E)
    W = 13

    cx, cy, R = S / 2, 71, 42
    m, d = Canvas.mask()
    o = R + W / 2.0
    d.arc([cx - o, cy - o, cx + o, cy + o], start=302, end=238, fill=255, width=W)
    for a in (302, 238):
        mdot(d, *polar(cx, cy, R, a), r=W / 2.0)         # round the arc ends
    mpath(d, [(cx, 17), (cx, 68)], W)
    c.body(m, p_t, p_b, hi=0.34, lo=0.30, k=3)


ICONS = [
    ("terminal", icon_terminal),
    ("files",    icon_files),
    ("editor",   icon_editor),
    ("paint",    icon_paint),
    ("snake",    icon_snake),
    ("cube3d",   icon_cube3d),
    ("monitor",  icon_monitor),
    ("about",    icon_about),
    ("settings", icon_settings),
    ("power",    icon_power),
]


# ---- render ---------------------------------------------------------------

def downsample(img):
    """box-filter 128x128 RGBA -> 32x32 RGBA, correctly.

    Premultiply, average, un-premultiply. Averaging straight colour would mix in
    the transparent pixels' zeroes and every edge would come out muddy; the
    premultiplied mean divided by the mean alpha is the true average colour OF
    THE COVERED AREA, which is what a partially covered pixel actually shows."""
    a = np.asarray(img, dtype=np.float64)
    al = a[:, :, 3]
    pm = a[:, :, :3] * (al[:, :, None] / 255.0)

    box = lambda ch: ch.reshape(N, SS, N, SS, ch.shape[-1]).mean(axis=(1, 3))
    pm_s = box(pm)
    al_s = box(al[:, :, None])[:, :, 0]

    out = np.zeros((N, N, 4), np.float64)
    nz = al_s > 0.0
    out[:, :, :3][nz] = pm_s[nz] * 255.0 / al_s[nz][:, None]
    out[:, :, 3] = al_s
    return np.clip(np.round(out), 0, 255).astype(np.uint8)


def render(fn):
    c = Canvas()
    fn(c)
    return downsample(c.img)


# ---- emit -----------------------------------------------------------------

def emit(f, name, icons):
    f.write("const unsigned char %s[%d][%d][%d][4] = {\n" % (name, len(icons), N, N))
    for i, (nm, px) in enumerate(icons):
        f.write("  { /* %d %s */\n" % (i, nm))
        for y in range(N):
            f.write("    { /* y %2d */\n" % y)
            for x0 in range(0, N, 8):
                cells = ",".join("{%3d,%3d,%3d,%3d}" % tuple(int(v) for v in px[y][x])
                                 for x in range(x0, x0 + 8))
                f.write("      " + cells + ("," if x0 + 8 < N else "") + "\n")
            f.write("    },\n")
        f.write("  },\n")
    f.write("};\n")


HEADER = """/* icons_rgb.c - GENERATED by gen_icons_rgb.py. Do not edit by hand.
 * %d desktop icons, %dx%d, FULL COLOUR: four bytes per pixel, R,G,B,A.
 *
 * Alpha is STRAIGHT, not premultiplied: [3] is coverage 0..255 and [0..2] hold
 * the icon's real colour at full strength even where coverage is partial. That
 * is exactly the shape fb.c's blend_rgb(bg, fg, a) wants, so a renderer is
 *
 *     a = p[3];  fg = p[0]<<16 | p[1]<<8 | p[2];
 *     put_pixel(x, y, a >= 255 ? fg : blend_rgb(fb_get_px(x, y), fg, a));
 *
 * and the linear-light blend the console already uses does the rest. Nothing
 * here needs a divide at run time - the un-premultiply happened at build time.
 *
 * Drawn as pure geometry at %dx (%dx%d) and box-filtered down, so the edges are
 * anti-aliased the same way the font atlases are - no font, no bitmap file. The
 * whole set is lit from the TOP-LEFT: a vertical gradient down each body, a 1px
 * lighter top/left edge and a 1px darker bottom/right edge.
 *
 * NOTE: this is NOT the icons24 index order - files/editor/paint/snake/cube3d/
 * monitor/about moved. Old icons24 -> new icons_rgb: 0->0, 1->4, 2->3, 3->2,
 * 4->6, 5->7, 6->5, 7->1, 8->8, 9->9.
 *
 * index:%s
 */
"""


# ---- previews -------------------------------------------------------------

RAMP = " .:-=+*#%@"
CHECK = ((60, 62, 70), (44, 46, 52))       # the backdrop previews composite over


def preview_alpha(name, px):
    print("\n  %s  (alpha)" % name)
    print("  +" + "-" * N + "+")
    for y in range(N):
        line = "".join(RAMP[min(len(RAMP) - 1, int(px[y][x][3]) * len(RAMP) // 256)]
                       for x in range(N))
        print("  |" + line + "|")
    print("  +" + "-" * N + "+")


def preview_color(name, px):
    """ANSI truecolour, composited over a checkerboard so alpha is visible"""
    print("\n  %s  (colour over a checkerboard)" % name)
    for y in range(N):
        row = ""
        for x in range(N):
            r, g, b, a = (int(v) for v in px[y][x])
            bg = CHECK[((x >> 2) + (y >> 2)) & 1]
            cr = (r * a + bg[0] * (255 - a)) // 255
            cg = (g * a + bg[1] * (255 - a)) // 255
            cb = (b * a + bg[2] * (255 - a)) // 255
            row += "\x1b[48;2;%d;%d;%dm  " % (cr, cg, cb)
        print("  " + row + "\x1b[0m")


def main():
    built = [(nm, render(fn)) for nm, fn in ICONS]

    idx = "".join("\n *   %2d %s" % (i, nm) for i, (nm, _) in enumerate(built))
    with open(OUT, "w") as f:
        f.write(HEADER % (len(built), N, N, SS, S, S, idx))
        emit(f, "icons_rgb", built)

    # ---- checks worth failing on
    alphas = np.concatenate([px[:, :, 3].ravel() for _, px in built])
    mids = alphas[(alphas > 0) & (alphas < 255)]
    print("wrote %s: %d icons, %dx%d, RGBA (straight alpha)" % (OUT, len(built), N, N))
    print("  bytes of data:      %d" % (len(built) * N * N * 4))
    print("  alpha range:        %d..%d over %d distinct levels"
          % (alphas.min(), alphas.max(), len(set(alphas.tolist()))))
    print("  partial-alpha px:   %d of %d (%.1f%%) - antialiased edge"
          % (len(mids), len(alphas), 100.0 * len(mids) / len(alphas)))
    for nm, px in built:
        a = px[:, :, 3].astype(int)
        print("    %-9s opaque %3d  partial %3d  clear %3d  levels %2d"
              % (nm, (a == 255).sum(), ((a > 0) & (a < 255)).sum(), (a == 0).sum(),
                 len(set(a.ravel().tolist()))))

    for nm, px in built:
        a = px[:, :, 3]
        assert px.shape == (N, N, 4), "%s: wrong shape %s" % (nm, px.shape)
        assert a.max() == 255, "%s: nothing is fully opaque" % nm
        # An axis-aligned edge lands on a whole pixel and SHOULD be hard, so the
        # bar is low: it only has to prove the curves and diagonals are filtered.
        assert ((a > 0) & (a < 255)).sum() >= 12, "%s: edges are not antialiased" % nm
        assert a[:2, :].sum() == 0 and a[-2:, :].sum() == 0, "%s: breaks 2px padding" % nm
        assert a[:, :2].sum() == 0 and a[:, -2:].sum() == 0, "%s: breaks 2px padding" % nm
        # a coloured icon that is grey everywhere is a bug, not a style
        opaque = px[a == 255][:, :3].astype(int)
        assert (opaque.max(axis=1) - opaque.min(axis=1)).max() >= 24, "%s: no colour" % nm
    print("  checks:             shape, opacity, AA edges, 2px padding, colour  OK")

    for nm in ("files", "cube3d", "terminal"):
        px = dict(built)[nm]
        preview_color(nm, px)
        preview_alpha(nm, px)


if __name__ == "__main__":
    main()
