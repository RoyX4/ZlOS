#!/usr/bin/env python3
# gen_icons.py - draw the zlOS desktop icon set and emit it as a coverage atlas.
#
# The dock and the window titlebars need pictures, not letters. Rather than
# ship a bitmap file the kernel would have to parse, every icon is DRAWN here
# with plain geometry - lines, arcs, polygons - at 4x (96x96), then box-filtered
# down to 24x24. The box filter over a 4x4 block is exactly "what fraction of
# this pixel is ink", so the result is the same kind of array the fonts already
# use: one byte of coverage per pixel, 0..255.
#
#   icons24[N][24][24]   - alpha only, no colour
#
# No font, no emoji, no external art: the shapes are code, so they can be
# retuned by editing numbers here and re-running. fb.c blends a caller-chosen
# colour over the framebuffer by these coverage values (see fb_icon24), which
# is why one atlas serves a light dock and a dark titlebar both.
#
# Coverage quantisation: a 4x4 supersample averages 16 binary samples, so edge
# pixels land on multiples of 255/16 (~16). That is 17 distinct levels, which
# is plenty to kill the staircase at this size.

import os
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
OUT  = os.path.join(HERE, "icons.c")

SS   = 4                 # supersample factor
N    = 24                # final icon size, px
S    = N * SS            # master size, 96
PAD  = 2 * SS            # ~2px of breathing room inside the 24x24 box
# PIL's rectangle/ellipse bounds are INCLUSIVE, so the far edge has to be
# S-PAD-1, not S-PAD - otherwise the shape bleeds a quarter-covered column
# into pixel 22 and the icon is a half pixel off centre.
LO, HI = PAD, S - PAD - 1                # usable master box: 8..87

INK   = 255
ERASE = 0

# stroke weights, in MASTER pixels (divide by SS for the 24x24 weight)
W_MAIN = 8               # 2.0px  - frames, outlines
W_BOLD = 10              # 2.5px  - the one stroke that carries the icon
W_THIN = 7               # 1.75px - interior detail (text rules, fold lines)


# ---- little geometry helpers ---------------------------------------------

def poly(d, pts, width=W_MAIN, fill=INK):
    """closed outline with round joints - PIL's polygon(width=) mitres badly"""
    d.line(list(pts) + [pts[0]], fill=fill, width=width, joint="curve")
    r = width / 2.0
    for (x, y) in pts:                     # round off every corner
        d.ellipse([x - r, y - r, x + r, y + r], fill=fill)


def path(d, pts, width=W_MAIN, fill=INK, caps=True):
    """open polyline with round joints and optional round caps"""
    d.line(list(pts), fill=fill, width=width, joint="curve")
    r = width / 2.0
    ends = pts if caps else pts[1:-1]
    for (x, y) in ends:
        d.ellipse([x - r, y - r, x + r, y + r], fill=fill)


def dot(d, x, y, r, fill=INK):
    d.ellipse([x - r, y - r, x + r, y + r], fill=fill)


def ring(d, cx, cy, r, width=W_MAIN, fill=INK):
    """circle outline with the stroke CENTRED on radius r (PIL draws inward)"""
    o = r + width / 2.0
    d.ellipse([cx - o, cy - o, cx + o, cy + o], outline=fill, width=width)


def polar(cx, cy, r, deg):
    from math import cos, sin, radians
    a = radians(deg)
    return (cx + r * cos(a), cy + r * sin(a))


# ---- the icons ------------------------------------------------------------
# Each returns nothing; it draws into the 96x96 master `d`.

def icon_terminal(d):
    """a > prompt inside a rounded rect"""
    d.rounded_rectangle([LO, 14, HI, 82], radius=14,
                        outline=INK, width=W_MAIN)
    path(d, [(28, 38), (46, 50), (28, 62)], width=W_BOLD)      # the chevron
    path(d, [(54, 64), (72, 64)], width=W_BOLD)                # the cursor rule


def icon_snake(d):
    """a tapered body coiled into a spiral, notched into segments, head outside.

    The coil pitch is the whole design: one turn must advance far enough that
    the ring does not touch the one inside it, or the icon collapses into a
    disc at 24px. pitch = 2*pi*K, so K is chosen from (body + gap), not taste."""
    from math import cos, sin, pi
    cx, cy = S / 2, S / 2 + 4              # +4: the coil's mass sits high
    # pitch = 2*pi*K must clear (body diameter + a visible gap), and the body
    # has to stay thick enough to read at 24px - so 1.5 turns, not 3. PHASE
    # just spins the whole coil so the head finishes at the top right.
    R0, K, TURNS, PHASE, STEPS = 5.0, 2.86, 1.5, 2.356, 400
    RAD = lambda f: 3.2 + 2.3 * f          # body radius, tail -> head
    pts, arc = [], [0.0]
    for i in range(STEPS + 1):
        t = TURNS * 2 * pi * i / STEPS
        r = R0 + K * t
        pts.append((cx + r * cos(t + PHASE), cy + r * sin(t + PHASE)))
        if i:
            (x0, y0), (x1, y1) = pts[-2], pts[-1]
            arc.append(arc[-1] + ((x1 - x0) ** 2 + (y1 - y0) ** 2) ** 0.5)

    # taper the body from a thin tail at the centre to a full stroke at the
    # head, by stamping overlapping discs - PIL's line() cannot vary width.
    for i, (x, y) in enumerate(pts):
        dot(d, x, y, RAD(i / STEPS))

    # Notch it into segments. The notch must be cut by ARC LENGTH, not by
    # sample index (the spiral's step length grows with radius), and its half
    # height must track the LOCAL body radius - a fixed height slices into the
    # neighbouring coil and shreds the icon.
    SPACING, next_cut = 24.0, 24.0
    for i in range(2, len(pts) - 30):
        if arc[i] < next_cut:
            continue
        next_cut += SPACING
        (x0, y0), (x1, y1) = pts[i - 1], pts[i + 1]
        dx, dy = x1 - x0, y1 - y0
        L = (dx * dx + dy * dy) ** 0.5 or 1.0
        nx, ny = -dy / L, dx / L
        h = RAD(i / STEPS) + 0.8
        px, py = pts[i]
        d.line([(px - nx * h, py - ny * h), (px + nx * h, py + ny * h)],
               fill=ERASE, width=3)

    hx, hy = pts[-1]                       # head, a touch fatter than the body
    dot(d, hx, hy, 8.0)
    ux, uy = hx - cx, hy - cy              # outward normal, for the eye
    L = (ux * ux + uy * uy) ** 0.5 or 1.0
    dot(d, hx + ux / L * 3.0, hy + uy / L * 3.0, 3.0, fill=ERASE)


def icon_paint(d):
    """a brush held at 45 degrees: handle, ferrule, loaded tip"""
    from math import sqrt
    ax, ay = 16.0, 82.0                    # tip end (bottom-left)
    bx, by = 76.0, 18.0                    # handle end (top-right)
    L = sqrt((bx - ax) ** 2 + (by - ay) ** 2)
    ux, uy = (bx - ax) / L, (by - ay) / L  # along the brush
    nx, ny = -uy, ux                       # across it

    def P(t, off):
        return (ax + ux * t + nx * off, ay + uy * t + ny * off)

    # The head reads by SILHOUETTE, not by internal detail: bristles wider than
    # the ferrule, ferrule wider than the handle. A drawn groove between them
    # is under a pixel at this size and only chews holes in the shape.
    path(d, [P(32, 0), P(L, 0)], width=W_BOLD)                           # handle
    d.polygon([P(20, -8), P(33, -8), P(33, 8), P(20, 8)], fill=INK)      # ferrule
    d.polygon([P(21, -14), P(21, 14), P(5, 9), P(5, -9)], fill=INK)      # bristles
    dot(d, *P(5, -8), r=3.0)
    dot(d, *P(5, 8), r=3.0)                # round the two tip corners only
    # A V-notch to suggest separate bristles was tried and cut: at 24px it is
    # one pixel wide and reads as damage, not detail. The shoulder where the
    # bristles step out past the ferrule is what carries the shape.


def icon_editor(d):
    """a document with a folded corner and ruled text lines"""
    poly(d, [(19, 13), (57, 13), (77, 33), (77, 83), (19, 83)], width=W_MAIN)
    path(d, [(57, 13), (57, 33), (77, 33)], width=W_MAIN, caps=False)   # the fold
    for y, x1 in ((48, 65), (61, 65), (74, 55)):
        path(d, [(31, y), (x1, y)], width=W_THIN)


def icon_monitor(d):
    """a rising bar chart on a baseline"""
    base = 78
    path(d, [(13, base + 4), (83, base + 4)], width=W_MAIN)             # axis
    for x, top in ((24, 56), (48, 38), (72, 16)):
        path(d, [(x, base), (x, top)], width=12)


def icon_about(d):
    """a lowercase i inside a circle"""
    ring(d, S / 2, S / 2, 34, width=W_MAIN)
    dot(d, S / 2, 30, 5.0)
    path(d, [(S / 2, 44), (S / 2, 70)], width=W_BOLD)


def icon_cube3d(d):
    """an isometric cube: hexagon silhouette plus the three edges to centre"""
    cx, cy, R = S / 2, S / 2, 35
    hexa = [polar(cx, cy, R, 90 + 60 * k) for k in range(6)]
    poly(d, hexa, width=W_MAIN)
    for k in (0, 2, 4):                    # top, lower-left, lower-right
        path(d, [(cx, cy), hexa[k]], width=W_MAIN, caps=False)
    dot(d, cx, cy, W_MAIN / 2)


def icon_files(d):
    """a folder with a tab"""
    poly(d, [(13, 75), (13, 21), (37, 21), (45, 32), (83, 32), (83, 75)],
         width=W_MAIN)


def icon_settings(d):
    """a gear: rim, eight radial teeth, hub.

    A true toothed-silhouette outline does not survive 24px - the valleys come
    out ~1.5px wide and the two sides of the stroke merge into a blob. Drawing
    the rim as a ring with separate teeth keeps every gap at least 1.7px."""
    cx = cy = S / 2
    ring(d, cx, cy, 25, width=9)                       # rim
    for i in range(8):
        a = 45.0 * i
        path(d, [polar(cx, cy, 26, a), polar(cx, cy, 35, a)],
             width=10, caps=False)                     # teeth
    ring(d, cx, cy, 9.5, width=8)                      # hub


def icon_power(d):
    """the IEC power mark: broken ring plus a stem through the gap"""
    cx, cy, R = S / 2, 50, 33
    o = R + W_MAIN / 2.0
    d.arc([cx - o, cy - o, cx + o, cy + o], start=302, end=238,
          fill=INK, width=W_MAIN)
    path(d, [(cx, 13), (cx, 48)], width=W_BOLD)


ICONS = [
    ("terminal", icon_terminal),
    ("snake",    icon_snake),
    ("paint",    icon_paint),
    ("editor",   icon_editor),
    ("monitor",  icon_monitor),
    ("about",    icon_about),
    ("cube3d",   icon_cube3d),
    ("files",    icon_files),
    ("settings", icon_settings),
    ("power",    icon_power),
]


# ---- render + emit --------------------------------------------------------

def render(fn):
    """draw at 96x96, box-filter to 24x24, return rows of coverage bytes"""
    img = Image.new("L", (S, S), 0)
    fn(ImageDraw.Draw(img))
    small = img.resize((N, N), Image.BOX)
    return [[small.getpixel((x, y)) for x in range(N)] for y in range(N)]


def emit(f, name, icons):
    f.write(f"const unsigned char {name}[{len(icons)}][{N}][{N}] = {{\n")
    for i, (nm, rows) in enumerate(icons):
        f.write(f"  {{ /* {i} {nm} */\n")
        for row in rows:
            f.write("    {" + ",".join("%3d" % v for v in row) + "},\n")
        f.write("  },\n")
    f.write("};\n")


RAMP = " .:-=+*#%@"

def preview(name, rows):
    print(f"\n  {name}")
    print("  +" + "-" * N + "+")
    for row in rows:
        line = "".join(RAMP[min(len(RAMP) - 1, v * len(RAMP) // 256)] for v in row)
        print("  |" + line + "|")
    print("  +" + "-" * N + "+")


def main():
    built = [(nm, render(fn)) for nm, fn in ICONS]

    with open(OUT, "w") as f:
        f.write("/* icons.c - GENERATED by gen_icons.py. Do not edit by hand.\n")
        f.write(f" * {len(built)} desktop icons, {N}x{N}, one byte of COVERAGE per pixel\n")
        f.write(" * (0 = background shows through, 255 = solid foreground). Drawn as pure\n")
        f.write(f" * geometry at {SS}x ({S}x{S}) and box-filtered down, so the edges are\n")
        f.write(" * anti-aliased the same way the font atlases are - no font, no bitmap\n")
        f.write(" * file, no colour. fb.c's fb_icon24 blends a caller-chosen colour over\n")
        f.write(" * the framebuffer by these values, so one atlas works on any background.\n")
        f.write(" *\n")
        f.write(" * index:")
        for i, (nm, _) in enumerate(built):
            f.write(("\n *   %2d %s" % (i, nm)))
        f.write("\n */\n")
        emit(f, "icons24", built)

    lo = min(min(min(r) for r in rows) for _, rows in built)
    hi = max(max(max(r) for r in rows) for _, rows in built)
    levels = sorted({v for _, rows in built for r in rows for v in r})
    mids = [v for v in levels if 0 < v < 255]

    print(f"wrote {OUT}: {len(built)} icons, {N}x{N}, coverage {lo}..{hi}")
    print(f"  distinct levels: {len(levels)}   intermediate (0<v<255): {len(mids)}")
    print(f"  bytes of data:   {len(built) * N * N}")

    for nm in ("terminal", "cube3d", "power"):
        preview(nm, dict(built)[nm])


if __name__ == "__main__":
    main()
