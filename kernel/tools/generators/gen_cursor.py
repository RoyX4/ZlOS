#!/usr/bin/env python3
# gen_cursor.py - draw the zlOS mouse cursors and emit them as coverage atlases.
#
# WHY THIS EXISTS
#
# Every other asset in this renderer is a gamma-correct coverage atlas: both
# mono fonts, the proportional font, the icons at two sizes. The mouse cursor
# was the last 1-bit asset in the whole pipeline - fb_cursor_arrow built a 12x18
# triangle row by row with hard edges and a fixed width per row:
#
#     for (int r = 0; r < 16; r++) {
#         int w = r < 12 ? r + 1 : (16 - r) * 2;
#         fb_fill_px(x, y + r, w, 1, fill);      <- a hard step per row
#         put_pixel(x + w, y + r, edge);
#     }
#
# and it is the single thing the eye follows constantly. docs/desktop/desktop-look.md found
# and fixed three resampling bugs; this is the fourth, missed because the cursor
# is DRAWN rather than resampled, so no resampling audit could see it.
#
# It also never scaled. `sc` is 2 on every screen 1400px or wider - which is
# every screen actually used - and the icons, the fonts and every metric follow
# ui(). The cursor stayed 9x16 physical pixels on a 2560-wide panel.
#
# WHAT IT EMITS
#
#   cur_body16[N][16][16]   silhouette: the shape DILATED by the outline width
#   cur_fill16[N][16][16]   interior: the shape itself
#   cur_hot16[N][2]         the hotspot, in atlas pixels
#   ...and the same at 32.
#
# TWO PLANES, NOT ONE. A cursor is not an icon: it needs a light fill AND a dark
# outline, or it vanishes against a background of its own colour. One coverage
# plane can carry one colour. So fb.c blends the edge colour by `body` and then
# the fill colour by `fill` on top - two blend_cov calls, both anti-aliased, and
# the partial coverage where `fill` fades out lets the edge colour through as a
# soft rim. That is what makes it read on a white window and a dark wallpaper
# both, which the old hard-edged version only managed by luck.
#
# EACH SIZE IS RASTERIZED SEPARATELY at 4x supersampling, for the reason
# gen_icons.py records: box-filtering the 64x64 master by 2x2 to reach 16 gives
# FIVE coverage levels, and five levels is a staircase again. The master grows
# with the output, so 32x32 is real detail and not a 16x16 blown up.
#
# Geometry is written in a 16-UNIT LOGICAL SPACE with floats, and scaled on the
# way to PIL. gen_icons.py needed a ScaledDraw proxy because its sixty literals
# were hand-tuned in master units and could not be rewritten; this file is new,
# so it can simply use the logical space directly and multiply once.

import os
from math import cos, sin, radians

from PIL import Image, ImageDraw

KERNEL_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(KERNEL_ROOT, "src", "drivers", "display", "assets", "cursor.inc")

SS = 4                      # supersample factor: 17 coverage levels
SIZES = (16, 32)            # every output size, smallest first
U = 16.0                    # the logical drawing space is U x U, always

# The outline half-width, in LOGICAL units. The body plane is the fill plane
# dilated by this much, so this is exactly how thick the dark rim looks at
# 16x16; at 32x32 it is twice as many physical pixels, which is what "the
# cursor scales with ui()" means.
#
# It is also a CONSTRAINT ON EVERY SHAPE: the rim grows inward as well as
# outward at a concave vertex, so any neck narrower than 2*OUTLINE closes up
# completely and the shape falls into two pieces. The arrow's first draft had a
# 2.0-unit neck between head and tail against a 0.62 rim, which left 0.76 units
# of fill - invisible on a dark background, and on a LIGHT one, where the white
# body vanishes and only the rim is left, the arrow read as two detached
# shapes. Found by rendering it over four backgrounds and looking at it at 8x,
# which is the only way that particular failure shows.
OUTLINE = 0.55


# ---- the drawing surface --------------------------------------------------

class Pen:
    """An ImageDraw over an n*SS master, taking coordinates in the 16-unit
    logical space and scaling them once on the way through.

    Lengths (widths, radii) scale too. Angles must not, and none are passed.
    """

    def __init__(self, n):
        self.master = n * SS
        self.k = self.master / U
        self.img = Image.new("L", (self.master, self.master), 0)
        self.d = ImageDraw.Draw(self.img)

    def _p(self, pts):
        return [(x * self.k, y * self.k) for (x, y) in pts]

    def _w(self, v):
        """a stroke width in logical units -> master pixels, never below 1"""
        return max(1, int(round(v * self.k)))

    def polygon(self, pts, fill=255):
        self.d.polygon(self._p(pts), fill=fill)

    def outline(self, pts, width, fill=255):
        """stroke a CLOSED path, centred on it, with round joints and caps.

        Centred is the point: half the width lands outside the polygon and half
        inside, so filling the polygon and then stroking its outline at 2*OUTLINE
        gives exactly the polygon dilated by OUTLINE. PIL's polygon(width=)
        mitres badly at sharp angles - and an arrow tip is the sharpest angle
        there is - so this strokes the segments and rounds every vertex by hand,
        the same fix gen_icons.py's poly() makes.
        """
        p = self._p(list(pts) + [pts[0]])
        w = self._w(width)
        self.d.line(p, fill=fill, width=w, joint="curve")
        r = w / 2.0
        for (x, y) in p:
            self.d.ellipse([x - r, y - r, x + r, y + r], fill=fill)

    def line(self, pts, width, fill=255):
        """stroke an OPEN path, centred, with round joints and caps"""
        p = self._p(pts)
        w = self._w(width)
        self.d.line(p, fill=fill, width=w, joint="curve")
        r = w / 2.0
        for (x, y) in p:
            self.d.ellipse([x - r, y - r, x + r, y + r], fill=fill)

    def disc(self, cx, cy, r, fill=255):
        x, y, rr = cx * self.k, cy * self.k, r * self.k
        self.d.ellipse([x - rr, y - rr, x + rr, y + rr], fill=fill)

    def ring(self, cx, cy, r, width, fill=255):
        """circle outline with the stroke CENTRED on radius r (PIL draws inward)"""
        w = self._w(width)
        o = r * self.k + w / 2.0
        x, y = cx * self.k, cy * self.k
        self.d.ellipse([x - o, y - o, x + o, y + o], outline=fill, width=w)

    def coverage(self, n):
        small = self.img.resize((n, n), Image.BOX)
        return [[small.getpixel((x, y)) for x in range(n)] for y in range(n)]


def polar(cx, cy, r, deg):
    a = radians(deg)
    return (cx + r * cos(a), cy + r * sin(a))


# ---- the cursors -----------------------------------------------------------
# Each is (name, hotspot, shape_fn). shape_fn(pen, plane) draws into `pen`;
# plane is "fill" for the interior and "body" for the dilated silhouette. Most
# shapes are a polygon, so the default handler below covers them and the
# function only has to return the point list.
#
# The hotspot is in LOGICAL units and is the pixel the pointer position means.
# An arrow points with its tip, so its hotspot is the tip; everything else is
# symmetric and points with its middle. Getting this wrong is not cosmetic -
# it is a systematic offset between where the user aims and what gets clicked.


# The arrow's tip cannot sit at (0,0). The body plane is the fill DILATED by
# OUTLINE, so an outline around a tip at the origin lands at negative
# coordinates and is clipped away by the atlas edge - leaving the arrow with no
# dark rim along its top and left, which are the two edges that need one most
# (that is the side that meets a light window). Inset the whole shape by this
# much and make the hotspot follow it, so the tip still means the tip.
ARROW_INSET = 0.75          # > OUTLINE, in logical units


def shape_arrow():
    """The classic left-pointing arrow: tip at the top-left, tail to the
    lower-right.

    Proportions follow the X11 left_ptr rather than being invented: the head is
    a right triangle whose hypotenuse runs from the tip, and the tail leaves it
    at the notch.

    THE NECK IS THE ONE DIMENSION THAT IS NOT FREE. It is the distance between
    the notch and the tail's inner corner, both on the head's bottom edge at
    y = 8.40, and the outline eats OUTLINE off each side of it. At 2.0 units
    against a 0.62 rim the first draft left 0.76 units of fill and the arrow
    fell into two pieces on a light background. 2.90 against 0.55 leaves 1.80 -
    more than a whole pixel of body at 16x16, and two at 32x32.
    """
    i = ARROW_INSET
    return [(x + i, y + i) for (x, y) in (
        (0.00,  0.00),      # the tip - and the hotspot
        (0.00, 11.60),      # straight down the left edge
        (2.90,  8.40),      # the notch, where the tail begins
        (5.30, 13.60),      # tail, outer side
        (7.55, 12.55),      # tail, across the bottom
        (5.80,  8.40),      # tail, inner side - neck = 5.80 - 2.90 = 2.90
        (8.70,  8.40),      # the head's bottom edge, out to the shoulder
    )]                      # ...and closed back to the tip: the hypotenuse


def shape_ibeam():
    """A text I-beam: a vertical stem with serifs top and bottom.

    Drawn as ONE closed polygon rather than three strokes, because the outline
    plane is the polygon dilated - and dilating three overlapping strokes
    separately would put a rim through the middle of the joins.
    """
    x, t, b = 8.0, 3.0, 13.0        # centre line, top, bottom
    hw, sw = 0.62, 2.30             # stem half-width, serif half-width
    sh = 0.60                       # serif thickness
    return [
        (x - sw, t), (x + sw, t), (x + sw, t + sh), (x + hw, t + sh),
        (x + hw, b - sh), (x + sw, b - sh), (x + sw, b), (x - sw, b),
        (x - sw, b - sh), (x - hw, b - sh), (x - hw, t + sh), (x - sw, t + sh),
    ]


def shape_resize():
    """A double-headed diagonal arrow, NW to SE - the corner grip.

    One polygon again, traced right round the outside: NW head, down the
    shaft's upper-right side, SE head, back up the lower-left side. The heads
    are isoceles triangles on the diagonal, so the whole thing is symmetric
    under a 180-degree rotation about the centre, which is what stops it
    reading as "drag this way" rather than "resize".
    """
    a, b = 2.6, 13.4                # the two tips, on the NW-SE diagonal
    hw = 0.72                       # shaft half-width, across the diagonal
    hd, hs = 3.5, 2.45              # head length along, half-span across

    # unit vectors along the diagonal and across it
    ux = uy = 0.70710678
    nx, ny = -uy, ux
    L = ((b - a) ** 2 * 2) ** 0.5   # the diagonal's length

    def at(d, off):
        """d units along the diagonal from the NW tip, off units across it"""
        return (a + ux * d + nx * off, a + uy * d + ny * off)

    return [
        at(0, 0),                   # NW tip
        at(hd,  hs), at(hd,  hw),   # NW head, right barb, down to the shaft
        at(L - hd,  hw), at(L - hd,  hs),
        at(L, 0),                   # SE tip
        at(L - hd, -hs), at(L - hd, -hw),
        at(hd, -hw), at(hd, -hs),
    ]


def draw_busy(pen, plane):
    """An hourglass. Not a spinner and not a watch, for a size reason: a ring
    with a wedge is ~1.5px of stroke at 16px and mushes into a disc, and a
    watch's hands are under a pixel. Two triangles meeting at a waist stay
    legible all the way down because the shape is carried by its silhouette."""
    top, bot = 2.6, 13.4
    half = 3.5                      # the flare, at top and bottom
    waist = 0.55                    # the pinch in the middle
    mid = (top + bot) / 2.0
    cx = 8.0
    grow = OUTLINE if plane == "body" else 0.0

    # the two bowls, as one polygon each, so the dilation is per-bowl and the
    # waist does not fill in when the outline is added
    for (y0, y1) in ((top, mid), (bot, mid)):
        s = 1.0 if y1 > y0 else -1.0
        pts = [(cx - half, y0), (cx + half, y0),
               (cx + waist, y1), (cx - waist, y1)]
        pen.polygon(pts)
        if grow:
            pen.outline(pts, 2 * grow)
    # the caps, which read as the frame
    pen.line([(cx - half - 0.5, top), (cx + half + 0.5, top)],
             1.1 + 2 * grow)
    pen.line([(cx - half - 0.5, bot), (cx + half + 0.5, bot)],
             1.1 + 2 * grow)


def poly_shape(fn):
    """turn a point-list function into a (pen, plane) drawer"""
    def draw(pen, plane):
        pts = fn()
        pen.polygon(pts)
        if plane == "body":
            pen.outline(pts, 2 * OUTLINE)
    return draw


CURSORS = [
    ("arrow",  (ARROW_INSET, ARROW_INSET), poly_shape(shape_arrow)),
    ("ibeam",  (8.0, 8.0),  poly_shape(shape_ibeam)),
    ("resize", (8.0, 8.0),  poly_shape(shape_resize)),
    ("busy",   (8.0, 8.0),  draw_busy),
]


# ---- render + emit ---------------------------------------------------------

def render(drawer, plane, n):
    pen = Pen(n)
    drawer(pen, plane)
    return pen.coverage(n)


def bbox(rows):
    """tight bounds of every NONZERO coverage pixel: (x0, y0, x1, y1) exclusive"""
    n = len(rows)
    x0, y0, x1, y1 = n, n, 0, 0
    for y in range(n):
        for x in range(n):
            if rows[y][x]:
                if x < x0: x0 = x
                if y < y0: y0 = y
                if x >= x1: x1 = x + 1
                if y >= y1: y1 = y + 1
    if x0 > x1:
        return (0, 0, 0, 0)
    return (x0, y0, x1, y1)


def emit_atlas(f, name, planes, n):
    f.write("static const unsigned char %s[%d][%d][%d] = {\n"
            % (name, len(planes), n, n))
    for i, (nm, rows) in enumerate(planes):
        f.write("  { /* %d %s */\n" % (i, nm))
        for row in rows:
            f.write("    {" + ",".join("%3d" % v for v in row) + "},\n")
        f.write("  },\n")
    f.write("};\n")


RAMP = " .:-=+*#%@"


def preview(name, body, fill):
    """the two planes side by side, as text. The gate for this file is that a
    human looks at the cursor magnified; this is that, at generation time."""
    n = len(body)
    print("\n  %s   (left: body/outline    right: fill/interior)" % name)
    print("  +" + "-" * n + "+  +" + "-" * n + "+")
    for y in range(n):
        a = "".join(RAMP[min(9, v * 10 // 256)] for v in body[y])
        b = "".join(RAMP[min(9, v * 10 // 256)] for v in fill[y])
        print("  |" + a + "|  |" + b + "|")
    print("  +" + "-" * n + "+  +" + "-" * n + "+")


def main():
    built = {}
    for n in SIZES:
        bodies = [(nm, render(fn, "body", n)) for nm, _, fn in CURSORS]
        fills = [(nm, render(fn, "fill", n)) for nm, _, fn in CURSORS]
        built[n] = (bodies, fills)

    with open(OUT, "w") as f:
        f.write("""\
/* cursor.inc - GENERATED by gen_cursor.py. Do not edit by hand.
 *
 * The mouse cursors, as COVERAGE atlases - the same one-byte-per-pixel format
 * the fonts and the icons already use, drawn as geometry at 4x and box-filtered
 * down. Before this, fb_cursor_arrow computed a 12x18 triangle row by row with
 * hard edges: the last 1-bit asset in a renderer where everything else is
 * gamma-correct coverage, and the one asset the eye follows constantly.
 *
 * TWO PLANES PER CURSOR:
 *   cur_bodyN   the silhouette, DILATED by the outline width -> the dark rim
 *   cur_fillN   the interior                                 -> the light body
 * fb.c blends the edge colour by body, then the fill colour by fill on top.
 * One plane could only carry one colour, and a cursor with no outline vanishes
 * against a background of its own colour.
 *
 * TWO SIZES, each rasterized separately at 4x - never one scaled up. That was
 * docs/desktop/desktop-look.md bug 1 for the icons and it is the same mistake here.
 *
 * cur_hotN[i] is the HOTSPOT in atlas pixels: the pixel that IS the pointer
 * position. An arrow points with its tip, everything else with its middle.
 *
 * cur_boxN[i] is the tight bounding box of nonzero coverage in the BODY plane,
 * as {x0, y0, w, h}. fb.c's save-under reads this, so the saved patch and the
 * drawn patch come from the same generated number and cannot drift apart. An
 * anti-aliased cursor blends with what is under it, so a save-under one pixel
 * short of the soft edge trails a halo behind the pointer forever.
 *
 * This file rides inside fb.c rather than being its own translation unit, for
 * the reason font_prop.inc records: four build scripts carry four separate
 * source lists, and adding a .c to only some of them broke two builds twice in
 * one day.
 */
""")
        f.write("\n#define CUR_N %d\n" % len(CURSORS))
        for i, (nm, _, _) in enumerate(CURSORS):
            f.write("#define CUR_%-8s %d\n" % (nm.upper(), i))

        for n in SIZES:
            bodies, fills = built[n]
            f.write("\n#define CUR_SZ%-3d %d\n" % (n, n))
            emit_atlas(f, "cur_body%d" % n, bodies, n)
            f.write("\n")
            emit_atlas(f, "cur_fill%d" % n, fills, n)
            f.write("\n/* hotspot {x, y}, in atlas pixels */\n")
            f.write("static const short cur_hot%d[%d][2] = {" % (n, len(CURSORS)))
            for nm, (hx, hy), _ in CURSORS:
                k = n / U
                f.write(" {%d,%d}," % (int(round(hx * k)), int(round(hy * k))))
            f.write(" };\n")
            f.write("/* tight bounds of nonzero BODY coverage {x0, y0, w, h} */\n")
            f.write("static const short cur_box%d[%d][4] = {" % (n, len(CURSORS)))
            for nm, rows in bodies:
                x0, y0, x1, y1 = bbox(rows)
                f.write(" {%d,%d,%d,%d}," % (x0, y0, x1 - x0, y1 - y0))
            f.write(" };\n")

    # ---- report, and the checks that must hold -----------------------------
    total = 0
    for n in SIZES:
        bodies, fills = built[n]
        for (nm, brows), (_, frows) in zip(bodies, fills):
            bb = bbox(brows)
            fb_ = bbox(frows)
            levels = sorted({v for r in brows for v in r})
            mids = [v for v in levels if 0 < v < 255]
            total += 2 * n * n
            # the fill must be strictly inside the body, or the outline has a
            # gap and the light body touches the background directly
            assert fb_[0] >= bb[0] and fb_[1] >= bb[1] \
                and fb_[2] <= bb[2] and fb_[3] <= bb[3], \
                "%s@%d: fill %s escapes body %s" % (nm, n, fb_, bb)
            for y in range(n):
                for x in range(n):
                    assert brows[y][x] >= frows[y][x], \
                        "%s@%d: fill exceeds body at %d,%d" % (nm, n, x, y)
            assert bb[2] <= n and bb[3] <= n, "%s@%d: ink escapes the box" % (nm, n)
            assert mids, "%s@%d: NO intermediate coverage - not anti-aliased" % (nm, n)
            # THE OUTLINE MUST NOT BE CLIPPED BY THE ATLAS EDGE. The body plane
            # is the fill dilated by OUTLINE, so a shape whose geometry touches
            # the border has its rim cut off on that side - which is how the
            # arrow shipped its first draft with no dark edge along the top and
            # left, exactly where it meets a light window. A shape that fits
            # only ever grazes the border with PARTIAL coverage; one that has
            # been cut saturates against it.
            border = ([brows[0][x] for x in range(n)]
                      + [brows[n - 1][x] for x in range(n)]
                      + [brows[y][0] for y in range(n)]
                      + [brows[y][n - 1] for y in range(n)])
            assert max(border) < 250, \
                "%s@%d: body saturates the atlas border (%d) - outline clipped" \
                % (nm, n, max(border))
            print("  cur %-7s @%2d  ink %2d,%2d %2dx%-2d  %2d levels (%2d soft)"
                  % (nm, n, bb[0], bb[1], bb[2] - bb[0], bb[3] - bb[1],
                     len(levels), len(mids)))
    print("wrote %s: %d bytes of atlas" % (OUT, total))

    for i, (nm, _, _) in enumerate(CURSORS):
        preview(nm, built[16][0][i][1], built[16][1][i][1])


if __name__ == "__main__":
    main()
