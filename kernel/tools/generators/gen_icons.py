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

KERNEL_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(KERNEL_ROOT, "src", "graphics", "icons", "icons.c")

SS    = 4                # supersample factor
SIZES = (24, 48)         # every output size emitted, smallest first
N     = SIZES[0]         # the size the geometry below is written in terms of
S     = N * SS           # master size, 96 - the LOGICAL drawing space
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



# ---- the second set, v10 SS6.9 ---------------------------------------------
# Taken from the vocabulary the prototype's own markup uses - Places, Devices,
# Properties, Rename, Close, Cancel, OK, Unlock, End Process, Update interval,
# Accent colour, plus the header's network state and its search field ("No
# matches"). These are the controls that UI actually needs; the first ten were
# app launchers, which is a different job.

def icon_search(d):
    """a magnifier: ring plus a handle on the lower right diagonal"""
    cx, cy, r = 40, 40, 24
    ring(d, cx, cy, r, width=W_MAIN)
    a = polar(cx, cy, r + W_MAIN / 2, 45)
    path(d, [a, (80, 80)], width=W_BOLD)


def icon_lock(d):
    """a padlock: shackle arc over a body. The shackle is drawn as an arc
    rather than a rounded rect so the two uprights stay parallel at 24px."""
    d.arc([30, 16, 66, 56], start=180, end=360, fill=INK, width=W_MAIN)
    path(d, [(30, 40), (30, 50)], width=W_MAIN, caps=False)
    path(d, [(66, 40), (66, 50)], width=W_MAIN, caps=False)
    d.rounded_rectangle([22, 48, 74, 84], radius=8, outline=INK, width=W_MAIN)
    dot(d, 48, 66, 6)


def icon_drive(d):
    """a disk: a wide rounded body with a status lamp - the Devices row"""
    d.rounded_rectangle([LO, 30, HI, 68], radius=10, outline=INK, width=W_MAIN)
    path(d, [(20, 49), (56, 49)], width=W_THIN)
    dot(d, 74, 49, 6)


def icon_close(d):
    """an X. Two strokes, and they must CROSS at the centre - drawing them as
    two independent diagonals with different lengths reads as a wonky X."""
    path(d, [(26, 26), (70, 70)], width=W_BOLD)
    path(d, [(70, 26), (26, 70)], width=W_BOLD)


def icon_check(d):
    """a tick - OK, and the on state of anything"""
    path(d, [(22, 50), (40, 68), (76, 28)], width=W_BOLD)


def icon_chevron(d):
    """a right chevron: submenus, disclosure, the next page"""
    path(d, [(38, 24), (64, 48), (38, 72)], width=W_BOLD)


def icon_clock(d):
    """a clock face with two hands - Update interval, and the tray"""
    ring(d, S / 2, S / 2, 32, width=W_MAIN)
    path(d, [(S / 2, S / 2), (S / 2, 26)], width=W_MAIN)        # minute
    path(d, [(S / 2, S / 2), (66, 56)], width=W_MAIN)           # hour


def icon_network(d):
    """three rising bars under an arc - the header's "net up" """
    for x, top in ((26, 66), (44, 54), (62, 42)):
        path(d, [(x, 78), (x, top)], width=10)
    d.arc([18, 18, 78, 78], start=225, end=315, fill=INK, width=W_THIN)


def icon_volume(d):
    """a speaker cone plus one wave. Two waves do not survive 24px - the outer
    arc lands within a pixel of the inner one and they merge."""
    poly(d, [(20, 40), (34, 40), (50, 24), (50, 72), (34, 56), (20, 56)],
         width=W_THIN)
    d.arc([44, 28, 76, 68], start=300, end=60, fill=INK, width=W_MAIN)


def icon_grid(d):
    """four rounded squares - Activities, the app grid"""
    for cx in (32, 64):
        for cy in (32, 64):
            d.rounded_rectangle([cx - 15, cy - 15, cx + 15, cy + 15],
                                radius=5, outline=INK, width=W_MAIN)


# ---- navigation, window and application controls -------------------------

def icon_browser(d):
    ring(d, 48, 48, 34, width=W_MAIN)
    path(d, [(14, 48), (82, 48)], width=W_THIN)
    d.ellipse([31, 12, 65, 84], outline=INK, width=W_THIN)


def icon_home(d):
    path(d, [(16, 45), (48, 17), (80, 45)], width=W_MAIN)
    poly(d, [(23, 42), (23, 80), (73, 80), (73, 42)], width=W_MAIN)
    path(d, [(42, 80), (42, 59), (55, 59), (55, 80)], width=W_THIN, caps=False)


def icon_minimize(d):
    path(d, [(28, 64), (68, 64)], width=W_BOLD)


def icon_maximize(d):
    d.rounded_rectangle([23, 23, 73, 73], radius=5, outline=INK, width=W_MAIN)


def icon_restore(d):
    d.rounded_rectangle([31, 20, 75, 64], radius=4, outline=INK, width=W_MAIN)
    d.rounded_rectangle([20, 31, 64, 75], radius=4, outline=INK, width=W_MAIN)


def icon_menu(d):
    for y in (28, 48, 68): path(d, [(23, y), (73, y)], width=W_BOLD)


def icon_refresh(d):
    d.arc([17, 17, 79, 79], start=35, end=330, fill=INK, width=W_MAIN)
    d.polygon([(71, 15), (84, 31), (63, 32)], fill=INK)


def icon_plus(d):
    path(d, [(48, 22), (48, 74)], width=W_BOLD)
    path(d, [(22, 48), (74, 48)], width=W_BOLD)


def icon_folder_add(d):
    icon_files(d)
    path(d, [(60, 45), (60, 66)], width=W_MAIN)
    path(d, [(49, 55), (71, 55)], width=W_MAIN)


def icon_trash(d):
    d.rounded_rectangle([27, 31, 69, 80], radius=5, outline=INK, width=W_MAIN)
    path(d, [(20, 27), (76, 27)], width=W_MAIN)
    path(d, [(38, 18), (58, 18)], width=W_MAIN)
    for x in (40, 56): path(d, [(x, 42), (x, 68)], width=W_THIN)


def icon_download(d):
    path(d, [(48, 17), (48, 62)], width=W_BOLD)
    path(d, [(30, 46), (48, 64), (66, 46)], width=W_BOLD)
    path(d, [(22, 78), (74, 78)], width=W_MAIN)


def icon_upload(d):
    path(d, [(48, 65), (48, 20)], width=W_BOLD)
    path(d, [(30, 36), (48, 18), (66, 36)], width=W_BOLD)
    path(d, [(22, 78), (74, 78)], width=W_MAIN)


def icon_copy(d):
    d.rounded_rectangle([31, 20, 76, 68], radius=4, outline=INK, width=W_MAIN)
    d.rounded_rectangle([20, 31, 65, 79], radius=4, outline=INK, width=W_MAIN)


def icon_paste(d):
    d.rounded_rectangle([24, 24, 72, 80], radius=5, outline=INK, width=W_MAIN)
    d.rounded_rectangle([35, 15, 61, 32], radius=5, fill=ERASE, outline=INK, width=W_MAIN)
    for y in (48, 62): path(d, [(36, y), (61, y)], width=W_THIN)


def icon_warning(d):
    poly(d, [(48, 14), (84, 78), (12, 78)], width=W_MAIN)
    path(d, [(48, 36), (48, 57)], width=W_BOLD)
    dot(d, 48, 68, 4)


def icon_bell(d):
    d.arc([25, 19, 71, 66], start=180, end=360, fill=INK, width=W_MAIN)
    path(d, [(25, 43), (25, 65), (18, 73), (78, 73), (71, 65), (71, 43)], width=W_MAIN)
    d.arc([39, 68, 57, 84], start=0, end=180, fill=INK, width=W_MAIN)


def icon_back(d):
    path(d, [(61, 20), (32, 48), (61, 76)], width=W_BOLD)


def icon_forward(d):
    path(d, [(35, 20), (64, 48), (35, 76)], width=W_BOLD)


def icon_external(d):
    d.rounded_rectangle([18, 30, 66, 79], radius=5, outline=INK, width=W_MAIN)
    path(d, [(46, 18), (78, 18), (78, 50)], width=W_MAIN)
    path(d, [(77, 19), (46, 50)], width=W_BOLD)


def icon_user(d):
    ring(d, 48, 34, 14, width=W_MAIN)
    d.arc([19, 43, 77, 88], start=180, end=360, fill=INK, width=W_MAIN)


def icon_calendar(d):
    d.rounded_rectangle([18, 22, 78, 80], radius=6, outline=INK, width=W_MAIN)
    path(d, [(18, 39), (78, 39)], width=W_MAIN)
    for x in (34, 62): path(d, [(x, 15), (x, 30)], width=W_BOLD)


def icon_camera(d):
    d.rounded_rectangle([14, 30, 82, 76], radius=8, outline=INK, width=W_MAIN)
    poly(d, [(31, 30), (37, 20), (59, 20), (65, 30)], width=W_MAIN)
    ring(d, 48, 53, 13, width=W_MAIN)


def icon_play(d):
    d.polygon([(31, 20), (76, 48), (31, 76)], fill=INK)


def icon_pause(d):
    d.rounded_rectangle([27, 20, 42, 76], radius=4, fill=INK)
    d.rounded_rectangle([54, 20, 69, 76], radius=4, fill=INK)


# ---- the reference's own 45 app glyphs ------------------------------------
#
# Everything above is geometry written by hand in the 96-unit master. The apps
# below are different: docs/design/ds-reference.html assigns each of its 53
# apps an icon id and DRAWS that icon, as inline SVG, in its ICONS table
# (ds-reference.html:1121-1211). Redrawing those by eye would be inventing a
# glyph the reference already shows, so the path data is transcribed verbatim
# and rendered here.
#
# The transform is linear and there is only one number in it. The reference
# renders `viewBox="0 0 20 20"` into a size x size box; our master IS the icon
# box at 96 units. So SC = 96/20 = 4.8, applied to every coordinate. The
# reference's own stroke weight, 1.7 user units, lands at 1.7*4.8 = 8.16 - i.e.
# W_MAIN, 2.0px at 24px - which is the check that the mapping is the right one
# rather than a guess.
#
# TWO DELIBERATE DEPARTURES, both forced by PIL:
#   - the reference asks for strokeLinejoin:'miter'; PIL's only wide-line join
#     is round (joint="curve"). At W_MAIN the corner radius is 4 master units,
#     one pixel at 24px, so a stroked rectangle's corners come out very
#     slightly rounded. Every hand-drawn icon above already does this.
#   - strokeLinecap:'square' becomes a round cap for the same reason.
# The GLYPH - which strokes exist, where they run, what is filled - is exact.
#
# Fills use PIL's polygon, which is nonzero-winding with no hole support. No
# fill path in the table below has a hole; check that before adding one.

import re

SVG_VB = 20.0                  # the reference's viewBox extent
SC     = S / SVG_VB            # 4.8 master units per reference unit

_NUM = re.compile(r'[-+]?(?:\d*\.\d+|\d+\.?\d*)(?:[eE][-+]?\d+)?')


class _PathReader:
    """SVG path-data scanner.

    Not a general one - it covers exactly the commands the reference uses
    (M L H V C S A Z and their relative forms) and raises on anything else,
    which is the point: a silently-ignored command would draw a wrong glyph.

    The one trap worth naming: an arc's large-arc and sweep parameters are
    FLAGS, single characters, and the minifier packs them against the next
    number - `a11 11 0 0113.8 0` is (0, 1, 13.8, 0), not (0, 113.8, ...).
    They must be read one character at a time, never as numbers.
    """

    def __init__(self, s):
        self.s, self.i = s, 0

    def _ws(self):
        while self.i < len(self.s) and self.s[self.i] in " ,\t\r\n":
            self.i += 1

    def eof(self):
        self._ws()
        return self.i >= len(self.s)

    def peek_cmd(self):
        self._ws()
        c = self.s[self.i]
        return c if c.isalpha() else None

    def cmd(self):
        c = self.peek_cmd()
        self.i += 1
        return c

    def num(self):
        self._ws()
        m = _NUM.match(self.s, self.i)
        if not m:
            raise ValueError("expected a number at %d in %r" % (self.i, self.s))
        self.i = m.end()
        return float(m.group())

    def flag(self):
        self._ws()
        c = self.s[self.i]
        self.i += 1
        if c not in "01":
            raise ValueError("expected an arc flag at %d in %r" % (self.i, self.s))
        return c == "1"


def _bezier(x0, y0, x1, y1, x2, y2, x3, y3, steps=16):
    out = []
    for i in range(1, steps + 1):
        t = i / float(steps)
        u = 1.0 - t
        a, b, c, e = u*u*u, 3*u*u*t, 3*u*t*t, t*t*t
        out.append((a*x0 + b*x1 + c*x2 + e*x3, a*y0 + b*y1 + c*y2 + e*y3))
    return out


def _svg_arc(x1, y1, rx, ry, phi_deg, large, sweep, x2, y2):
    """endpoint -> centre parameterisation, W3C SVG 1.1 appendix F.6.5"""
    from math import cos, sin, acos, sqrt, radians, ceil, pi
    if rx == 0 or ry == 0 or (x1 == x2 and y1 == y2):
        return [(x2, y2)]
    rx, ry = abs(rx), abs(ry)
    phi = radians(phi_deg)
    cp, sp = cos(phi), sin(phi)
    dx, dy = (x1 - x2) / 2.0, (y1 - y2) / 2.0
    x1p, y1p = cp * dx + sp * dy, -sp * dx + cp * dy
    lam = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry)
    if lam > 1.0:                              # radii too small: scale both up
        k = sqrt(lam)
        rx, ry = rx * k, ry * k
    den = rx * rx * y1p * y1p + ry * ry * x1p * x1p
    num = rx * rx * ry * ry - den
    co = sqrt(max(0.0, num / den))
    if large == sweep:
        co = -co
    cxp, cyp = co * rx * y1p / ry, -co * ry * x1p / rx
    cx = cp * cxp - sp * cyp + (x1 + x2) / 2.0
    cy = sp * cxp + cp * cyp + (y1 + y2) / 2.0

    def ang(ux, uy, vx, vy):
        n = sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy))
        if n == 0:
            return 0.0
        a = acos(max(-1.0, min(1.0, (ux * vx + uy * vy) / n)))
        return -a if ux * vy - uy * vx < 0 else a

    ux, uy = (x1p - cxp) / rx, (y1p - cyp) / ry
    vx, vy = (-x1p - cxp) / rx, (-y1p - cyp) / ry
    th0 = ang(1.0, 0.0, ux, uy)
    dth = ang(ux, uy, vx, vy)
    if not sweep and dth > 0:
        dth -= 2 * pi
    elif sweep and dth < 0:
        dth += 2 * pi
    # one segment per 7.5 degrees: at 96 units the chord error is under 0.02px
    n = max(4, int(ceil(abs(dth) / (pi / 24.0))))
    out = []
    for i in range(1, n + 1):
        t = th0 + dth * i / n
        c2, s2 = cos(t), sin(t)
        out.append((cp * rx * c2 - sp * ry * s2 + cx,
                    sp * rx * c2 + cp * ry * s2 + cy))
    return out


def _flatten(dstr):
    """path data -> [(points, closed), ...] in REFERENCE units"""
    r = _PathReader(dstr)
    subs, pts = [], None
    cx = cy = sx = sy = 0.0
    prev_c2, cmd = None, None
    while not r.eof():
        if r.peek_cmd() is not None:
            cmd = r.cmd()
        elif cmd == "M":                       # an implicit repeat of M is L,
            cmd = "L"                          # of m is l; everything else
        elif cmd == "m":                       # repeats itself
            cmd = "l"
        if cmd is None:
            raise ValueError("path does not start with a command: %r" % dstr)
        rel, k = cmd.islower(), cmd.upper()
        if k == "M":
            if pts and len(pts) > 1:
                subs.append((pts, False))
            x, y = r.num(), r.num()
            if rel:
                x, y = x + cx, y + cy
            cx, cy = sx, sy = x, y
            pts, prev_c2 = [(x, y)], None
        elif k == "L":
            x, y = r.num(), r.num()
            if rel:
                x, y = x + cx, y + cy
            cx, cy = x, y
            pts.append((x, y))
            prev_c2 = None
        elif k == "H":
            x = r.num()
            cx = x + cx if rel else x
            pts.append((cx, cy))
            prev_c2 = None
        elif k == "V":
            y = r.num()
            cy = y + cy if rel else y
            pts.append((cx, cy))
            prev_c2 = None
        elif k in ("C", "S"):
            if k == "C":
                x1, y1 = r.num(), r.num()
                if rel:
                    x1, y1 = x1 + cx, y1 + cy
            else:                              # smooth: reflect the last control
                x1, y1 = ((2 * cx - prev_c2[0], 2 * cy - prev_c2[1])
                          if prev_c2 else (cx, cy))
            x2, y2, x, y = r.num(), r.num(), r.num(), r.num()
            if rel:
                x2, y2, x, y = x2 + cx, y2 + cy, x + cx, y + cy
            pts.extend(_bezier(cx, cy, x1, y1, x2, y2, x, y))
            prev_c2, cx, cy = (x2, y2), x, y
        elif k == "A":
            rx, ry, rot = r.num(), r.num(), r.num()
            large, sweep = r.flag(), r.flag()
            x, y = r.num(), r.num()
            if rel:
                x, y = x + cx, y + cy
            pts.extend(_svg_arc(cx, cy, rx, ry, rot, large, sweep, x, y))
            cx, cy, prev_c2 = x, y, None
        elif k == "Z":
            if pts and len(pts) > 1:
                subs.append((pts, True))
            cx, cy = sx, sy                    # a relative command after z is
            pts, prev_c2 = [(sx, sy)], None    # relative to the SUBPATH start
        else:
            raise ValueError("unsupported SVG path command %r in %r" % (cmd, dstr))
    if pts and len(pts) > 1:
        subs.append((pts, False))
    return subs


def _stroke(d, pts, width):
    """wide polyline, round joins (PIL has no miter) and round caps"""
    d.line(pts, fill=INK, width=width, joint="curve")
    r = width / 2.0
    for (x, y) in (pts[0], pts[-1]):
        d.ellipse([x - r, y - r, x + r, y + r], fill=INK)


def ref_icon(name):
    """build a draw function for one ds-reference.html ICONS entry"""
    spec = REF[name]
    strokes = [_flatten(p) for p in spec.get("s", ())]
    fills   = [_flatten(p) for p in spec.get("f", ())]

    def draw(d):
        for subs in strokes:
            for pts, closed in subs:
                p = [(x * SC, y * SC) for x, y in pts]
                if closed:
                    p.append(p[0])
                _stroke(d, p, W_MAIN)
        for subs in fills:
            for pts, _closed in subs:
                if len(pts) >= 3:
                    d.polygon([(x * SC, y * SC) for x, y in pts], fill=INK)
    draw.__name__ = "ref_" + name
    return draw


# Verbatim from ds-reference.html:1121-1211. Do not retune these by hand: the
# whole point is that they are the reference's, not ours. If the reference
# changes, re-extract.
REF = {
    "log": {"s": ["M7.5 6h9.3", "M7.5 10h6.9", "M7.5 14h8.1"], "f": ["M3.3 5.3h2.2v1.5H3.3z", "M3.3 9.3h2.2v1.5H3.3z", "M3.3 13.3h2.2v1.5H3.3z"]},
    "render": {"s": ["M10 3.1l6.2 3.6v6.6L10 16.9l-6.2-3.6V6.7z", "M3.8 6.7L10 10.3l6.2-3.6", "M10 10.3v6.6"]},
    "hex": {"s": ["M10 3l6 3.5v7L10 17l-6-3.5v-7z"], "f": ["M7.6 8.4h1.7v1.7H7.6z", "M10.7 8.4h1.7v1.7h-1.7z", "M7.6 11.3h1.7v1.7H7.6z", "M10.7 11.3h1.7v1.7h-1.7z"]},
    "snap": {"s": ["M3.4 4.7h13.2v10.6H3.4z", "M10 4.7v10.6"]},
    "tty": {"s": ["M2.8 4.4h14.4v11.2H2.8z", "M2.8 7.2h14.4", "M5.4 10h3.2", "M5.4 12.4h6.4"], "f": ["M4.6 5.4h1.1v1.1H4.6z", "M6.6 5.4h1.1v1.1H6.6z"]},
    "calcApp": {"s": ["M4.8 3.6h10.4v12.8H4.8z", "M6.8 6h6.4v2.2H6.8z"], "f": ["M7 10h1.6v1.6H7z", "M9.4 10H11v1.6H9.4z", "M11.8 10h1.6v1.6h-1.6z", "M7 12.8h1.6v1.6H7z", "M9.4 12.8H11v1.6H9.4z", "M11.8 12.8h1.6v1.6h-1.6z"]},
    "font": {"s": ["M4.4 15.2L9 4.8h2l4.6 10.4", "M6.6 11.8h6.8"]},
    "disk": {"s": ["M10 4.2c3.4 0 6.2 1 6.2 2.2s-2.8 2.2-6.2 2.2S3.8 7.6 3.8 6.4 6.6 4.2 10 4.2z", "M3.8 6.4v7.2c0 1.2 2.8 2.2 6.2 2.2s6.2-1 6.2-2.2V6.4", "M3.8 10c0 1.2 2.8 2.2 6.2 2.2s6.2-1 6.2-2.2"]},
    "svc": {"s": ["M3.6 5h12.8v3.4H3.6z", "M3.6 11.6h12.8V15H3.6z"], "f": ["M5.6 6.2h1.4v1.2H5.6z", "M5.6 12.8h1.4v1.2H5.6z"]},
    "binary": {"s": ["M4.4 4.4h11.2v11.2H4.4z"], "f": ["M6.5 6.5h2.1v2.1H6.5z", "M11.4 6.5h2.1v2.1h-2.1z", "M6.5 11.4h2.1v2.1H6.5z", "M11.4 11.4h2.1v2.1h-2.1z"]},
    "imgv": {"s": ["M3.4 4.6h13.2v10.8H3.4z", "M3.4 12.2l3.6-3.4 2.8 2.6 3-2.8 4.2 3.8"], "f": ["M6.6 6.8a1.3 1.3 0 110 2.6 1.3 1.3 0 010-2.6z"]},
    "chip": {"s": ["M6.4 6.4h7.2v7.2H6.4z", "M4.3 8.2h2.1", "M4.3 11.8h2.1", "M13.6 8.2h2.1", "M13.6 11.8h2.1", "M8.2 4.3v2.1", "M11.8 4.3v2.1", "M8.2 13.6v2.1", "M11.8 13.6v2.1"]},
    "uClip": {"s": ["M6.4 4.6h7.2v2.4H6.4z", "M5 6.2h10v9.6H5z", "M7.6 10h4.8", "M7.6 12.6h3.2"]},
    "uColor": {"s": ["M10 3.6c3.5 0 6.4 2.6 6.4 5.9 0 2.2-1.9 3.2-3.4 3.2h-1.2c-1 0-1.7.7-1.7 1.6 0 1.2-.9 2.1-2.1 2.1-3.3 0-5.4-2.9-5.4-6.4 0-3.6 3.9-6.4 7.4-6.4z"], "f": ["M6.8 9a1.1 1.1 0 110 2.2 1.1 1.1 0 010-2.2z", "M9.4 6.4a1.1 1.1 0 110 2.2 1.1 1.1 0 010-2.2z", "M13 7.6a1.1 1.1 0 110 2.2 1.1 1.1 0 010-2.2z"]},
    "uRegex": {"s": ["M10 4.4v6.4", "M7.2 6.2l5.6 3.2", "M12.8 6.2L7.2 9.4", "M6.6 14.6h3.2"], "f": ["M12.6 13a1.5 1.5 0 110 3 1.5 1.5 0 010-3z"]},
    "uBase": {"s": ["M4.4 5.2h4v4h-4z", "M11.6 5.2h4v4h-4z", "M4.4 11.6h4v3.2h-4z", "M11.6 11.6h4v3.2h-4z", "M8.4 7.2h3.2", "M8.4 13.2h3.2"]},
    "uDiff": {"s": ["M4 4.6h5.6v10.8H4z", "M10.4 4.6H16v10.8h-5.6", "M5.4 8h2.8", "M12.2 8h2.4", "M12.2 11.4h2.4"]},
    "uHash": {"s": ["M6.8 4.2L5.6 15.8", "M13.2 4.2L12 15.8", "M4.2 7.6h11.6", "M4.2 12.4h11.6"]},
    "uUnit": {"s": ["M3.4 12.8h13.2", "M4.8 12.8V8.2h4.4v4.6", "M11 12.8V5.6h4.2v7.2", "M6 10.4h2", "M12.2 8h2"]},
    "uNote": {"s": ["M4.6 3.8h10.8v8.4l-3.6 3.6H4.6z", "M15.4 12.2h-3.6v3.6", "M7 7h6", "M7 9.8h4"]},
    "uKeys": {"s": ["M3.2 6.2h13.6v8.2H3.2z"], "f": ["M5.2 8h1.8v1.6H5.2z", "M8 8h1.8v1.6H8z", "M10.8 8h1.8v1.6h-1.8z", "M13.6 8h1.4v1.6h-1.4z", "M6.6 11h7.4v1.6H6.6z"]},
    "uBench": {"s": ["M10 14.4V9", "M4.6 14.4a5.4 5.4 0 1110.8 0z", "M13.4 8.2l-3 3.2"]},
    "gTetris": {"s": ["M3.8 3.8h5.2v5.2H3.8z", "M9 9h5.2v5.2H9z"], "f": ["M9 3.8h3.4v3.4H9z"]},
    "gPong": {"s": ["M4.4 6.4v7.2", "M15.6 6.4v7.2"], "f": ["M9.2 9.2h2.2v2.2H9.2z"]},
    "gBrick": {"s": ["M3.6 4.4h5.6v3H3.6z", "M10.8 4.4h5.6v3h-5.6z", "M3.6 8.6h5.6v3H3.6z", "M10.8 8.6h5.6v3h-5.6z", "M6.4 14.6h7.2"]},
    "gMine": {"s": ["M10 4.4v2.2", "M10 13.4v2.2", "M4.4 10h2.2", "M13.4 10h2.2"], "f": ["M10 6.8a3.2 3.2 0 110 6.4 3.2 3.2 0 010-6.4z"]},
    "gNum": {"s": ["M4 4h12v12H4z", "M4 10h12", "M10 4v12"]},
    "gLife": {"f": ["M4.2 4.2h3.4v3.4H4.2z", "M12.4 4.2h3.4v3.4h-3.4z", "M8.3 8.3h3.4v3.4H8.3z", "M4.2 12.4h3.4v3.4H4.2z", "M12.4 12.4h3.4v3.4h-3.4z"]},
    "gRock": {"s": ["M10 3.6l5.4 3.4-1.8 6.4H6.4L4.6 7z"]},
    "gAlien": {"s": ["M6 6.6h8v4.4H6z", "M4.2 8.8h1.8", "M14 8.8h1.8", "M7 13.4h2", "M11 13.4h2"], "f": ["M7.6 8h1.2v1.2H7.6z", "M11.2 8h1.2v1.2h-1.2z"]},
    "gCross": {"s": ["M4 8h12", "M4 12h12", "M8 4v12", "M12 4v12"]},
    "gTiles": {"s": ["M3.8 3.8h12.4v12.4H3.8z", "M3.8 10h12.4", "M10 3.8v12.4"], "f": ["M4.8 4.8h4.2v4.2H4.8z"]},
    "gDrop": {"s": ["M4 4.6h12v10.8H4z"], "f": ["M6.4 11.4a1.7 1.7 0 110 3.4 1.7 1.7 0 010-3.4z", "M10 11.4a1.7 1.7 0 110 3.4 1.7 1.7 0 010-3.4z"]},
    "gDisc": {"s": ["M3.8 3.8h12.4v12.4H3.8z"], "f": ["M7 6.4a2.4 2.4 0 110 4.8 2.4 2.4 0 010-4.8z", "M13 8.8a2.4 2.4 0 110 4.8 2.4 2.4 0 010-4.8z"]},
    "gBulb": {"s": ["M3.8 3.8h12.4v12.4H3.8z", "M3.8 10h12.4", "M10 3.8v12.4"], "f": ["M10.8 10.8h4.4v4.4h-4.4z"]},
    "gWave": {"s": ["M3.6 12.6l3.2-5 3.2 5 3.2-5 3.2 5"]},
    "gMaze": {"s": ["M3.8 3.8h12.4v12.4H3.8z", "M3.8 7.4h6.4", "M13.2 7.4h3", "M6.8 11h9.4", "M3.8 14h6.4"]},
    "gCrate": {"s": ["M4.6 4.6h10.8v10.8H4.6z", "M4.6 4.6l10.8 10.8", "M15.4 4.6L4.6 15.4"]},
    "gBird": {"s": ["M4.4 10.6a4 4 0 018 0", "M12.4 8.6l3.2-2.2"], "f": ["M10.6 8.2a.9.9 0 110 1.8.9.9 0 010-1.8z"]},
    "gTarget": {"s": ["M10 3.8v3", "M10 13.2v3", "M3.8 10h3", "M13.2 10h3", "M10 6.6a3.4 3.4 0 110 6.8 3.4 3.4 0 010-6.8z"]},
    "gTower": {"s": ["M10 4v11", "M4.4 15.4h11.2"], "f": ["M7.4 6.6h5.2v2H7.4z", "M6 9.6h8v2H6z", "M4.8 12.6h10.4v2H4.8z"]},
    "gSticks": {"s": ["M6 5v10", "M10 5v10", "M14 5v10"]},
    "gCard": {"s": ["M5 3.8h10v12.4H5z"], "f": ["M9.2 7.2l2.4 3-2.4 3-2.4-3z"]},
    "gFrog": {"s": ["M4.6 12.8h10.8", "M6.4 12.8V9.4a3.6 3.6 0 017.2 0v3.4"], "f": ["M7.6 6.4a1.1 1.1 0 110 2.2 1.1 1.1 0 010-2.2z", "M12.4 6.4a1.1 1.1 0 110 2.2 1.1 1.1 0 010-2.2z"]},
    "gWord": {"s": ["M3.8 5.4h12.4v9.2H3.8z", "M7.2 5.4v9.2", "M10.6 5.4v9.2", "M13.4 5.4v9.2"]},
}

# The order here is the ATLAS ORDER, and an atlas index is a number burned into
# apps_registry.zl. Append only - never insert, never reorder.
REF_ORDER = [
    # system apps (ds-reference.html APPS)
    "log", "render", "hex", "snap", "tty", "calcApp", "font",
    "disk", "svc", "binary", "imgv", "chip",
    # utilities (UTILS)
    "uClip", "uColor", "uRegex", "uBase", "uDiff",
    "uHash", "uUnit", "uNote", "uKeys", "uBench",
    # games (GAME_APPS)
    "gTetris", "gPong", "gBrick", "gMine", "gNum", "gLife", "gRock", "gAlien",
    "gCross", "gTiles", "gDrop", "gDisc", "gBulb", "gWave", "gMaze", "gCrate",
    "gBird", "gTarget", "gTower", "gSticks", "gCard", "gFrog", "gWord",
]

# term/files/mon/edit/set/clock/wifi are NOT here: the atlas already carries a
# glyph of the same design for each (terminal, files, monitor, editor,
# settings, clock, network), and those indices are already wired up. gSnake is
# not here either - the atlas's `snake` serves the one app that needs it, and
# nothing else in the tree can reach a gSnake index today.

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
    # v10 SS6.9 - the control set, from the prototype's own vocabulary
    ("search",   icon_search),
    ("lock",     icon_lock),
    ("drive",    icon_drive),
    ("close",    icon_close),
    ("check",    icon_check),
    ("chevron",  icon_chevron),
    ("clock",    icon_clock),
    ("network",  icon_network),
    ("volume",   icon_volume),
    ("grid",     icon_grid),
    ("browser",  icon_browser),
    ("home",     icon_home),
    ("minimize", icon_minimize),
    ("maximize", icon_maximize),
    ("restore",  icon_restore),
    ("menu",     icon_menu),
    ("refresh",  icon_refresh),
    ("plus",     icon_plus),
    ("folder_add", icon_folder_add),
    ("trash",    icon_trash),
    ("download", icon_download),
    ("upload",   icon_upload),
    ("copy",     icon_copy),
    ("paste",    icon_paste),
    ("warning",  icon_warning),
    ("bell",     icon_bell),
    ("back",     icon_back),
    ("forward",  icon_forward),
    ("external", icon_external),
    ("user",     icon_user),
    ("calendar", icon_calendar),
    ("camera",   icon_camera),
    ("play",     icon_play),
    ("pause",    icon_pause),
] + [(nm, ref_icon(nm)) for nm in REF_ORDER]


# ---- render + emit --------------------------------------------------------

class ScaledDraw:
    """An ImageDraw that multiplies every length by `k` before it hits PIL.

    The icons above are written in ONE coordinate space - the 96-unit master -
    and every literal in them was tuned by eye at that size. To get a 48x48
    atlas we cannot simply box-filter the 96x96 master by 2x2: a 4x4
    supersample gives 17 coverage levels, a 2x2 gives FIVE, and five levels is
    a staircase again. The master has to grow with the output.

    So rather than rewrite sixty hand-tuned literals, the geometry stays in
    96-unit space and this proxy scales it on the way through. Everything the
    icons call goes via a point list plus `width`/`radius`, all of which are
    lengths; `start`/`end` are ANGLES and must not be touched.
    """

    _SHAPES = ("line", "ellipse", "polygon", "rounded_rectangle", "arc",
               "rectangle", "chord", "pieslice")

    def __init__(self, img, k):
        self._d = ImageDraw.Draw(img)
        self._k = k

    def _pts(self, seq):
        if self._k == 1:
            return seq                           # identity: hand PIL the exact
                                                 # values it got before this
                                                 # class existed, ints and all
        out = []
        for p in seq:
            if isinstance(p, (tuple, list)):
                out.append(tuple(v * self._k for v in p))
            else:
                out.append(p * self._k)          # a flat [x0,y0,x1,y1] bbox
        return out

    def _len(self, v):
        """a stroke width or corner radius: scaled, and never thinner than 1px"""
        if self._k == 1:
            return v
        s = v * self._k
        return max(1, int(round(s)))

    def __getattr__(self, name):
        fn = getattr(self._d, name)
        if name not in self._SHAPES:
            return fn

        def wrapped(xy, *a, **kw):
            for key in ("width", "radius"):
                if key in kw:
                    kw[key] = self._len(kw[key])
            return fn(self._pts(xy), *a, **kw)
        return wrapped


def render(fn, n=N):
    """draw the 96-unit geometry at n*SS, box-filter to nxn, return coverage rows

    Every output size gets its OWN 4x4-supersampled master, so a 48x48 icon is
    real detail rather than a 24x24 icon with its pixels copied - which is
    exactly the bug this replaces (fb.c fb_icon24, docs/desktop/desktop-look.md bug 1)."""
    master = n * SS
    # an exact integer ratio keeps every scaled literal an exact integer too,
    # which is what makes the 24x24 output provably unchanged (k == 1)
    k = master // S if master % S == 0 else master / float(S)
    img = Image.new("L", (master, master), 0)
    fn(ScaledDraw(img, k))
    small = img.resize((n, n), Image.BOX)
    return [[small.getpixel((x, y)) for x in range(n)] for y in range(n)]


def emit(f, name, icons, n=N):
    f.write(f"const unsigned char {name}[{len(icons)}][{n}][{n}] = {{\n")
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
    # one atlas per output size, each rasterized from the geometry at that size
    atlases = [(n, [(nm, render(fn, n)) for nm, fn in ICONS]) for n in SIZES]

    with open(OUT, "w") as f:
        f.write("/* icons.c - GENERATED by gen_icons.py. Do not edit by hand.\n")
        f.write(f" * {len(ICONS)} desktop icons at {', '.join('%dx%d' % (n, n) for n in SIZES)},\n")
        f.write(" * one byte of COVERAGE per pixel (0 = background shows through,\n")
        f.write(" * 255 = solid foreground). Drawn as pure geometry and box-filtered down,\n")
        f.write(" * so the edges are anti-aliased the same way the font atlases are - no\n")
        f.write(" * font, no bitmap file, no colour. fb.c's fb_icon24 blends a\n")
        f.write(" * caller-chosen colour over the framebuffer by these values, so one\n")
        f.write(" * atlas works on any background.\n")
        f.write(" *\n")
        f.write(" * EACH SIZE IS RASTERIZED SEPARATELY at 4x supersampling. The 48x48 set\n")
        f.write(" * is NOT the 24x24 set scaled up: doing that in fb_icon24 was the single\n")
        f.write(" * most visible source of blockiness in the desktop (docs/desktop/desktop-look.md,\n")
        f.write(" * bug 1), because it threw away every anti-aliased edge pixel.\n")
        f.write(" *\n")
        f.write(" * index:")
        for i, (nm, _) in enumerate(ICONS):
            f.write(("\n *   %2d %s" % (i, nm)))
        f.write("\n */\n")
        for n, built in atlases:
            f.write("\n")
            emit(f, "icons%d" % n, built, n)
        # The COUNT is generated too, and fb.c's bounds check reads it rather
        # than repeating the number. It used to repeat it - `#define ICON_N 44`
        # and `extern const unsigned char icons24[44][24][24]` - so growing the
        # atlas here left fb_icon24 silently refusing every new index, and the
        # extern's dimension disagreeing with the definition's.
        f.write("\n/* how many icons the two atlases above actually hold. */\n")
        f.write("const int icons_n = %d;\n" % len(ICONS))

    total = 0
    for n, built in atlases:
        lo = min(min(min(r) for r in rows) for _, rows in built)
        hi = max(max(max(r) for r in rows) for _, rows in built)
        levels = sorted({v for _, rows in built for r in rows for v in r})
        mids = [v for v in levels if 0 < v < 255]
        total += len(built) * n * n
        print(f"  icons{n}: {len(built)} icons, {n}x{n}, coverage {lo}..{hi}, "
              f"{len(levels)} levels ({len(mids)} intermediate), "
              f"{len(built) * n * n} bytes")
    print(f"wrote {OUT}: {total} bytes of atlas")

    for nm in ("terminal", "cube3d", "power", "chip", "uBench", "gTarget"):
        preview(nm, dict(atlases[0][1])[nm])


if __name__ == "__main__":
    main()
