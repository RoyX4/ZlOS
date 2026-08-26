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
#
# ============================ PRESSWORK ==================================
#
# PRESSWORK's icons are a TECHNICAL DRAWING, not a picture. The rule is stated
# in the prototype's own icon block (docs/design/presswork-prototype.html:1147):
#
#   "MACHINED FROM THE SAME PLATE ... 45/90 armature only: every straight
#    segment is horizontal, vertical or exactly 45 degrees. Two stroke weights
#    - 2.0 silhouette, 1.5 interior. EXACTLY ONE SOLID MASS PER ICON and it is
#    the icon's subject. Curves appear only where the object is physically
#    round."
#
# That rule is the reason this file changed shape. Before PRESSWORK the icons
# were sixty hand-tuned Python functions in a 96-unit master, each free to pick
# its own stroke weight (there were three) and its own angles. A rule that
# every icon has to obey is worth more as a CHECKED rule than as a paragraph,
# so the geometry now lives in one declarative table, `PW`, written in the
# prototype's own 24-unit viewBox, and three checks run over it every time the
# generator runs:
#
#   1. ARMATURE   every authored straight segment is H, V or exactly 45.
#                 Exceptions must be declared per-icon in OFF_AXIS_OK, with a
#                 reason. Undeclared ones abort the build.
#   2. WEIGHTS    only two stroke widths reach PIL: W_MAIN and W_THIN.
#   3. MASS       every pictorial icon has exactly one solid mass; the pure
#                 control marks (close, check, chevron, the window buttons)
#                 have none. That split is the prototype's, not ours: its own
#                 window-control glyphs (presswork-prototype.html:2282-2284)
#                 are stroke-only with no fill at all, while all 18 of its app
#                 icons carry exactly one `.sol` element.
#
# Eighteen of the icons below are the prototype's ICO table transcribed
# verbatim; the rest are drawn here in the same idiom. Which is which is
# marked in the table.
#
# Light mode is deliberately out of scope - the atlas is coverage only, one
# byte per pixel, and carries no colour at all, so there is nothing here that
# could differ between the two ladders even if light mode were being shipped
# (it is not; see the brief).
#
# What PRESSWORK did NOT change, and what is preserved intact: the 4x
# supersample and box filter (so every edge is still anti-aliased the same way
# the font atlases are), the per-size rasterisation (48x48 is real geometry,
# not a scaled 24x24), the atlas ORDER and every icon NAME, the coverage-only
# output format, and integer-only output bytes. No floats and no heap reach the
# kernel: this is a build-time generator, and what it emits is a byte array.

import os
import re
from PIL import Image, ImageDraw

KERNEL_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(KERNEL_ROOT, "src", "graphics", "icons", "icons.c")

SS    = 4                # supersample factor
SIZES = (24, 48)         # every output size emitted, smallest first
N     = SIZES[0]         # the size the geometry below is written in terms of
S     = N * SS           # master size, 96 - the LOGICAL drawing space

INK   = 255
ERASE = 0

# ---- the stroke ladder ----------------------------------------------------
#
# PRESSWORK has exactly TWO weights, and the prototype states both: 2.0 for the
# silhouette and 1.5 for interior detail (presswork-prototype.html:1150, and
# the SVG itself - stroke-width="2" on the <svg>, overridden to 1.5 by the
# `w1` class at line 1179). At the 24px design size the master is 4x, so:
#
#   2.0 design px * SS(4) = 8 master units   W_MAIN
#   1.5 design px * SS(4) = 6 master units   W_THIN
#
# The predecessor had a third weight, W_BOLD at 10 (2.5px), used to make "the
# one stroke that carries the icon" heavier. PRESSWORK carries emphasis with
# the solid mass instead, so W_BOLD is gone. `_weights_seen` proves no third
# weight sneaks back in - every wide-line call in this file goes through the
# two helpers below.
W_MAIN = 8               # 2.0px - silhouette, container, subject strokes
W_THIN = 6               # 1.5px - interior detail: rules, grids, ticks

_weights_seen = set()

# Terminals: the prototype asks for stroke-linecap="round" and
# stroke-linejoin="round" (presswork-prototype.html:1178). At a 2.0 stroke the
# cap radius is 1.0 design px, which is the "minimally-radiused terminal" the
# design calls for; it is also the only wide-line join PIL can produce, so the
# generator and the prototype agree here for free rather than by compromise.


# ---- little geometry helpers ---------------------------------------------

def dot(d, x, y, r, fill=INK):
    d.ellipse([x - r, y - r, x + r, y + r], fill=fill)


def ring(d, cx, cy, r, width=W_MAIN, fill=INK):
    """circle outline with the stroke CENTRED on radius r (PIL draws inward)"""
    _weights_seen.add(width)
    o = r + width / 2.0
    d.ellipse([cx - o, cy - o, cx + o, cy + o], outline=fill, width=width)


def _stroke(d, pts, width, fill=INK):
    """wide polyline, round joins (PIL has no miter) and round caps"""
    _weights_seen.add(width)
    d.line(pts, fill=fill, width=width, joint="curve")
    r = width / 2.0
    for (x, y) in pts:                      # round every joint AND both ends
        d.ellipse([x - r, y - r, x + r, y + r], fill=fill)


# ---- the 45/90 armature check ---------------------------------------------
#
# A segment is legal if it is horizontal, vertical, or exactly 45 degrees.
# EPS is generous in absolute terms (0.02 of a 24-unit box = one twentieth of
# an output pixel) but tight enough that a real slope cannot slip past: the
# shallowest genuine violation in the prototype's own table, `clock`'s hour
# hand, is off by 1.5 units.
ARM_EPS = 0.02


def _armature_bad(p0, p1):
    dx, dy = abs(p1[0] - p0[0]), abs(p1[1] - p0[1])
    if dx < ARM_EPS and dy < ARM_EPS:
        return False                        # a zero-length hop, not a segment
    if dx < ARM_EPS or dy < ARM_EPS:
        return False                        # vertical / horizontal
    return abs(dx - dy) > ARM_EPS           # 45 degrees or it is a violation


# Every off-axis segment in the shipped table, with the reason it is allowed.
# These are ALL inherited from the prototype's own ICO entries - not one of the
# icons drawn here needed an exemption, which is the check earning its keep.
# The count is asserted below, so silently adding a slope somewhere else fails
# the build rather than quietly widening the rule.
OFF_AXIS_OK = {
    "terminal": (2, "prototype ICO.shell: the > prompt's chevron is 3:2.5, not 45 - "
                    "a 45 chevron at this size collides with the cursor rule below it"),
    "clock":    (1, "prototype ICO.clock: the hour hand points at ~4 o'clock. A clock "
                    "hand is a direction, and snapping it to 45 reads as 4:30 sharp"),
    "font":     (2, "prototype ICO.type: the two strokes of the letter A. The glyph is "
                    "the subject; an A with 45 legs is a different letter"),
    "volume":   (2, "prototype ICO.vol: the speaker cone flares 4.5:4. Snapping it to "
                    "45 makes the cone taller than the icon box"),
}


# ---- the PRESSWORK icon table ---------------------------------------------
#
# Written in the prototype's own coordinate system: a 24x24 viewBox, which is
# also the design size, so every number here is a design pixel and can be read
# straight off the prototype. The master is 96 units, hence:
PW_VB = 24.0
PWSC  = S / PW_VB                           # 4.0 master units per design px

# Each entry is a dict of lists:
#   "s"  strokes at W_MAIN  - the silhouette
#   "t"  strokes at W_THIN  - interior detail
#   "f"  THE solid mass     - at most one item, and it is the subject
#   "k"  knockouts          - punched back out of the mass (fill=0). A knockout
#                             is not a mass; it is a hole in one.
# An item is either an SVG path string in 24-unit space, or one of four
# shorthands that mirror the <rect>/<circle> elements the prototype writes:
#   ("R",  x, y, w, h, r)   rounded-rect outline
#   ("C",  cx, cy, r)       circle outline
#   ("FR", x, y, w, h)      solid rect
#   ("FC", cx, cy, r)       solid circle
#
# `PROTO` names the eighteen entries transcribed verbatim from the prototype's
# ICO table (presswork-prototype.html:1155-1173). Do not retune those by hand:
# the point is that they are the prototype's, not ours. If the prototype
# changes, re-transcribe.
PROTO = {"terminal", "editor", "monitor", "files", "settings", "power", "lock",
         "clock", "network", "volume", "grid", "log", "hex", "calcApp", "font",
         "disk", "binary", "chip"}

PW = {
    # ---- app launchers ----------------------------------------------------
    # ICO.shell
    "terminal": {"s": [("R", 3, 4, 18, 16, 2)],
                 "t": ["M6.5 9.5 L9.5 12 L6.5 14.5"],
                 "f": [("FR", 12, 14, 6, 1.6)]},

    # The zl mascot used to be a coiled, tapered, filled spiral. Under
    # PRESSWORK that is the one thing an icon may not be: an organic blob with
    # no armature and no constant weight. It is redrawn as the snake every
    # other part of this system already knows - a body on the grid, turning
    # only at right angles, with the head as the solid mass.
    "snake":    {"s": ["M5 7 H12 V17 H18.5"],
                 "f": [("FR", 16.8, 15.2, 3.8, 3.8)]},

    # A brush laid on the 45 diagonal, so the whole tool is one armature line.
    # Bristles wider than the ferrule, ferrule wider than the handle: the shape
    # reads by silhouette, exactly as it did before. What changed is that the
    # bristle block is now the single solid mass and the ferrule is an outline,
    # rather than both being filled.
    "paint":    {"s": ["M13.5 10.5 L20 4",
                       "M8.662 11.662 L12.338 15.338 L15.338 12.338 L11.662 8.662 Z"],
                 "f": ["M8.545 20.545 L13.045 16.045 L7.955 10.955 L3.455 15.455 Z"]},

    # ICO.edit
    "editor":   {"s": ["M5 3 L5 21 L19 21 L19 8 L14 3 Z"],
                 "t": ["M8 12 H16 M8 15 H16 M8 18 H13"],
                 "f": [("FR", 8, 6, 4, 3)]},

    # ICO.mon
    "monitor":  {"s": [("R", 3, 4, 18, 16, 2)],
                 "t": ["M6 16 L6 13 M10 16 L10 10", "M18 16 L18 12"],
                 "f": [("FR", 13, 7, 2.6, 9)]},

    # A lowercase i in a ring. The ring is a circle because the object is
    # round; the dot is the mass because the dot is what makes it an i.
    "about":    {"s": [("C", 12, 12, 8.5), "M12 11.5 V17"],
                 "f": [("FR", 11.1, 7, 1.8, 1.8)]},

    # A cabinet-projection box: the depth edges run at exactly 45, which an
    # isometric cube's 30 degrees cannot. The top face is the solid mass, which
    # also happens to be the face PRESSWORK's raking light strikes.
    "cube3d":   {"s": [("R", 5, 9, 11, 11, 0),
                       "M16 9 L20 5", "M20 5 V16", "M16 20 L20 16"],
                 "f": ["M9 5 H20 L16 9 H5 Z"]},

    # ICO.files
    "files":    {"s": ["M3 7 L3 19 L21 19 L21 9 L11.5 9 L9.5 7 Z"],
                 "f": [("FR", 5, 9.5, 7, 4)]},

    # ICO.set
    "settings": {"s": [("C", 12, 12, 7.5)],
                 "t": ["M12 2.5 V5 M12 19 V21.5 M2.5 12 H5 M19 12 H21.5 "
                       "M5.2 5.2 L7 7 M17 17 L18.8 18.8 M18.8 5.2 L17 7 M7 17 L5.2 18.8"],
                 "f": [("FC", 12, 12, 3)]},

    # ICO.power
    "power":    {"s": ["M6.5 6.5 A8 8 0 1 0 17.5 6.5"],
                 "f": [("FR", 11.2, 2.5, 1.6, 8)]},

    # ---- the control set --------------------------------------------------
    # These are marks, not pictures, so they carry NO solid mass - the same
    # choice the prototype makes for its own window buttons.
    "search":   {"s": [("C", 10, 10, 5.5), "M14.5 14.5 L20 20"]},

    # ICO.lock
    "lock":     {"s": [("R", 5, 10, 14, 10, 2)],
                 "t": ["M8 10 V7.5 A4 4 0 0 1 16 7.5 V10"],
                 "f": [("FR", 11, 13, 2, 4)]},

    "drive":    {"s": [("R", 3, 7, 18, 10, 2)],
                 "t": ["M6 12 H13"],
                 "f": [("FR", 16, 11, 2, 2)]},

    "close":    {"s": ["M5 5 L19 19", "M19 5 L5 19"]},
    "check":    {"s": ["M5 12.5 L10 17.5 L19 8.5"]},
    "chevron":  {"s": ["M9.5 5.5 L16 12 L9.5 18.5"]},

    # ICO.clock
    "clock":    {"s": [("C", 12, 12, 8.5)],
                 "t": ["M12 7 L12 12 L15.5 14"],
                 "f": [("FR", 11.2, 2.6, 1.6, 2.2)]},

    # ICO.net
    "network":  {"t": ["M4 10 A11 11 0 0 1 20 10", "M7 13.5 A7 7 0 0 1 17 13.5"],
                 "f": [("FR", 10.4, 16.4, 3.2, 3.2)]},

    # ICO.vol
    "volume":   {"t": ["M16 9 A5 5 0 0 1 16 15", "M18.5 6.5 A9 9 0 0 1 18.5 17.5"],
                 "f": ["M4 9.5 L8 9.5 L12.5 5.5 L12.5 18.5 L8 14.5 L4 14.5 Z"]},

    # ICO.grid
    "grid":     {"s": [("R", 3, 3, 18, 18, 2)],
                 "t": ["M3 9 H21 M3 15 H21 M9 3 V21 M15 3 V21"],
                 "f": [("FR", 3.8, 3.8, 4.4, 4.4)]},

    # A browser is a window with an address field, which is a truer picture of
    # the thing than a globe and does not need a mass invented for it.
    "browser":  {"s": [("R", 3, 4, 18, 16, 2), "M3 8.5 H21"],
                 "t": ["M5.5 6.25 H7.5"],
                 "f": [("FR", 10, 5.25, 8, 2.5)]},

    "home":     {"s": ["M2.5 12.5 L12 3 L21.5 12.5", "M5 10 V20.5 H19 V10"],
                 "f": [("FR", 10.2, 14.5, 3.6, 6)]},

    "minimize": {"s": ["M6 17 H18"]},
    "maximize": {"s": [("R", 5, 5, 14, 14, 0)]},
    "restore":  {"s": [("R", 8, 4, 12, 12, 0), ("R", 4, 8, 12, 12, 0)]},
    "menu":     {"s": ["M5 7 H19", "M5 12 H19", "M5 17 H19"]},

    # 300 degrees of arc with the gap at the top right, and a 45 arrowhead on
    # the leading end. The circle is a curve because rotation is round.
    "refresh":  {"s": ["M18.062 9 A7 7 0 1 1 12 5.5", "M9 2.5 L12 5.5 L9 8.5"]},

    "plus":     {"s": ["M12 5 V19", "M5 12 H19"]},

    "folder_add": {"s": ["M3 7 L3 19 L21 19 L21 9 L11.5 9 L9.5 7 Z"],
                   "t": ["M15.5 11.5 V17.5", "M12.5 14.5 H18.5"]},

    "trash":    {"s": [("R", 6, 6.2, 12, 13.8, 2)],
                 "t": ["M9.5 3 H14.5", "M10 10 V16.5", "M14 10 V16.5"],
                 "f": [("FR", 4, 4, 16, 2.2)]},

    "download": {"s": ["M12 3 V15", "M7 10 L12 15 L17 10", "M4.5 20 H19.5"]},
    "upload":   {"s": ["M12 16 V4", "M7 9 L12 4 L17 9", "M4.5 20 H19.5"]},
    # Only the front sheet is a closed rect; the sheet behind it is the two
    # edges you would actually see. Two full rects made this indistinguishable
    # from `restore` at 24px - the sole difference was the corner radius.
    "copy":     {"s": [("R", 3, 7, 13, 14, 2), "M8 7 V3 H21 V17 H16"]},

    "paste":    {"s": [("R", 4, 5, 16, 16, 2)],
                 "t": ["M7.5 11 H16.5", "M7.5 14.5 H14"],
                 "f": [("FR", 8.5, 2.6, 7, 3.4)]},

    # The ISO caution diamond rather than a triangle: a triangle whose sides
    # run at 45 is either squat or wider than the box, and the diamond is the
    # shape a technical document would actually print.
    "warning":  {"s": ["M12 3 L21 12 L12 21 L3 12 Z", "M12 8 V13.5"],
                 "f": [("FR", 11.1, 15.4, 1.8, 1.8)]},

    "bell":     {"s": ["M6.5 17 V11 A5.5 5.5 0 0 1 17.5 11 V17", "M4.5 17 H19.5"],
                 "f": [("FR", 10.8, 18.5, 2.4, 2.4)]},

    "back":     {"s": ["M15.5 5 L8.5 12 L15.5 19"]},
    "forward":  {"s": ["M8.5 5 L15.5 12 L8.5 19"]},
    "external": {"s": [("R", 3, 7, 12, 14, 2), "M12 3 H21 V12", "M21 3 L13 11"]},

    "user":     {"s": ["M4.5 20.5 A7.5 7.5 0 0 1 19.5 20.5"],
                 "f": [("FC", 12, 8, 4)]},

    "calendar": {"s": [("R", 3, 5, 18, 15, 2), "M3 10 H21"],
                 "t": ["M8 3 V7", "M16 3 V7"],
                 "f": [("FR", 6.5, 12.5, 4, 4)]},

    "camera":   {"s": [("R", 3, 7, 18, 13, 2), "M8.5 7 L11.5 4 H12.5 L15.5 7"],
                 "f": [("FC", 12, 13.5, 3.4)]},

    # A 45-sided triangle is necessarily half as wide as it is tall. That is
    # the armature's price and it is paid rather than dodged.
    "play":     {"s": ["M8.5 4.5 L16 12 L8.5 19.5 Z"]},
    "pause":    {"s": ["M9 5 V19", "M15 5 V19"]},

    # ---- system apps ------------------------------------------------------
    # ICO.log
    "log":      {"s": [("R", 3, 5, 18, 14, 2)],
                 "t": ["M6 9 H12 M6 12 H15 M6 15 H10"],
                 "f": [("FR", 16, 8, 2.6, 2.6)]},

    # Rendered vs not-yet-rendered, split on the diagonal.
    "render":   {"s": [("R", 4, 4, 16, 16, 1)],
                 "t": ["M4 20 L20 4"],
                 "f": ["M4 4 H12 L4 12 Z"]},

    # ICO.hex
    "hex":      {"s": [("R", 3, 4, 18, 16, 2)],
                 "t": ["M3 9 H21 M9 4 V20 M15 4 V20"],
                 "f": [("FR", 9.6, 9.6, 4.8, 4.8)]},

    "snap":     {"s": [("R", 3, 5, 18, 14, 1), "M12 5 V19"],
                 "f": [("FR", 4, 6, 7, 12)]},

    "tty":      {"s": [("R", 3, 4, 18, 16, 2), "M3 8 H21"],
                 "t": ["M6 12 H10", "M6 15.5 H14"],
                 "f": [("FR", 5.5, 5.2, 3, 1.6)]},

    # ICO.calc
    "calcApp":  {"s": [("R", 4, 3, 16, 18, 2)],
                 "t": ["M8 13 H10 M14 13 H16 M8 17 H10 M14 17 H16"],
                 "f": [("FR", 7, 6, 10, 3.4)]},

    # ICO.type
    "font":     {"s": [("R", 3, 4, 18, 16, 2)],
                 "t": ["M7 16 L10.5 8 L14 16 M8.4 13.4 H12.6"],
                 "f": [("FR", 16, 8, 2.4, 8)]},

    # ICO.disk
    "disk":     {"s": [("C", 12, 12, 8.5)],
                 "t": ["M12 3.5 L12 6 M16.5 16.5 L18.4 18.4"],
                 "f": [("FC", 12, 12, 2.4)]},

    "svc":      {"s": [("R", 3, 5, 18, 5, 1), ("R", 3, 14, 18, 5, 1)],
                 "t": ["M13 7.5 H18", "M13 16.5 H18"],
                 "f": [("FR", 5.5, 6.7, 1.6, 1.6)]},

    # ICO.regs, reused for the binary view: the prototype's register table is
    # already the picture this needs - a ruled grid with one cell struck out.
    "binary":   {"s": [("R", 3, 5, 18, 14, 2)],
                 "t": ["M3 12 H21 M8 5 V19 M13 5 V19"],
                 "f": [("FR", 13.8, 5.8, 6.4, 5.4)]},

    "imgv":     {"s": [("R", 3, 4, 18, 16, 2)],
                 "t": ["M4 16 L8.5 11.5 L13 16", "M11 14 L14.5 10.5 L20 16"],
                 "f": [("FC", 7.5, 8.5, 1.7)]},

    # ICO.chip
    "chip":     {"s": [("R", 6, 6, 12, 12, 1)],
                 "t": ["M9 3 V6 M12 3 V6 M15 3 V6 M9 18 V21 M12 18 V21 M15 18 V21 "
                       "M3 9 H6 M3 12 H6 M3 15 H6 M18 9 H21 M18 12 H21 M18 15 H21"],
                 "f": [("FR", 9.2, 9.2, 5.6, 5.6)]},

    # ---- utilities --------------------------------------------------------
    "uClip":    {"s": [("R", 4.5, 5, 15, 16, 2), ("R", 8.5, 2.5, 7, 4, 1)],
                 "t": ["M8 11 H16", "M8 14.5 H13"],
                 "f": [("FR", 8, 17.2, 5.5, 1.8)]},

    "uColor":   {"s": [("R", 3, 4, 18, 16, 2)],
                 "t": ["M3 15 H21", "M9 4 V15", "M15 4 V15"],
                 "f": [("FR", 5, 16.5, 14, 2.5)]},

    # A plus and an X sharing a centre: the only asterisk the armature allows,
    # and the two weights do the work a third weight used to.
    "uRegex":   {"s": ["M12 5 V15", "M7 10 H17"],
                 "t": ["M8.5 6.5 L15.5 13.5", "M15.5 6.5 L8.5 13.5"],
                 "f": [("FR", 10.6, 17.4, 2.8, 2.8)]},

    "uBase":    {"s": [("R", 3, 4, 18, 16, 2)],
                 "t": ["M3 9.33 H21", "M3 14.67 H21"],
                 "f": [("FR", 15.5, 5.4, 4.2, 2.8)]},

    "uDiff":    {"s": [("R", 3, 4, 8, 16, 1), ("R", 13, 4, 8, 16, 1)],
                 "t": ["M5 8 H9", "M15 8 H19", "M15 12 H19"],
                 "f": [("FR", 5, 11, 4, 2)]},

    # A hash is an input and a fixed-width digest, so the mass is the digest
    # bar under the mark. Putting the mass in the centre cell instead made this
    # the same picture as gLife.
    "uHash":    {"s": ["M8.5 3.5 V16", "M14.5 3.5 V16", "M3.5 8 H19.5", "M3.5 13 H19.5"],
                 "f": [("FR", 5.5, 18.4, 13, 2.6)]},

    "uUnit":    {"s": ["M3 19.5 H21", ("R", 13, 7, 6, 12.5, 0)],
                 "f": [("FR", 5, 11, 5, 8.5)]},

    "uNote":    {"s": [("R", 4, 5, 16, 15, 1), "M8 2.5 V7"],
                 "t": ["M7.5 11 H16.5", "M7.5 14.5 H13"],
                 "f": [("FR", 7.5, 16.8, 6, 1.8)]},

    "uKeys":    {"s": [("R", 3, 7, 18, 10, 2)],
                 "t": ["M6 10.5 H8", "M9.7 10.5 H11.7", "M13.4 10.5 H15.4",
                       "M17.1 10.5 H18.6"],
                 "f": [("FR", 8, 13, 8, 1.8)]},

    "uBench":   {"s": ["M4.5 17.5 A7.5 7.5 0 0 1 19.5 17.5", "M4.5 17.5 H19.5",
                       "M12 17.5 L17 12.5"],
                 "f": [("FR", 11, 16.5, 2, 2)]},

    # ---- games ------------------------------------------------------------
    "gTetris":  {"s": [("R", 4, 4, 7, 7, 0), ("R", 11, 11, 7, 7, 0)],
                 "f": [("FR", 11, 4, 7, 7)]},

    "gPong":    {"s": ["M5 7 V17", "M19 7 V17"],
                 "f": [("FR", 10.8, 10.8, 2.4, 2.4)]},

    "gBrick":   {"s": [("R", 3.5, 5, 7, 3.5, 0), ("R", 13.5, 5, 7, 3.5, 0),
                       ("R", 3.5, 10, 7, 3.5, 0), ("R", 13.5, 10, 7, 3.5, 0)],
                 "f": [("FR", 8.5, 17.5, 7, 2)]},

    # A mine is physically round, so it may be a circle; its spikes are the
    # four axes and the four 45s, which is the armature exactly.
    "gMine":    {"s": ["M12 3.5 V6", "M12 18 V20.5", "M3.5 12 H6", "M18 12 H20.5"],
                 "t": ["M6.5 6.5 L8.6 8.6", "M15.4 15.4 L17.5 17.5",
                       "M17.5 6.5 L15.4 8.6", "M8.6 15.4 L6.5 17.5"],
                 "f": [("FC", 12, 12, 4.2)]},

    "gNum":     {"s": [("R", 4, 4, 16, 16, 0)],
                 "t": ["M4 12 H20", "M12 4 V20"],
                 "f": [("FR", 12.5, 12.5, 7, 7)]},

    # Life has no board edge - the grid runs off the plate.
    "gLife":    {"t": ["M4 9.5 H20", "M4 14.5 H20", "M9.5 4 V20", "M14.5 4 V20"],
                 "f": [("FR", 9.5, 9.5, 5, 5)]},

    # A cut-corner plate: eight edges, four H/V and four at 45.
    "gRock":    {"s": ["M8 4 H16 L20 8 V16 L16 20 H8 L4 16 V8 Z"],
                 "f": [("FR", 10, 10, 4, 4)]},

    # The eyes are KNOCKED OUT of the body rather than drawn on top of it -
    # which is the same move the focused window header makes with its title.
    "gAlien":   {"s": ["M6 7 L3.5 4.5", "M18 7 L20.5 4.5"],
                 "t": ["M8.5 13 V16.5", "M15.5 13 V16.5"],
                 "f": [("FR", 6, 7, 12, 6)],
                 "k": [("FR", 8.2, 8.8, 2, 2), ("FR", 13.8, 8.8, 2, 2)]},

    # A crossword's blocked square, inset inside a middle cell. Two earlier
    # placements both failed to read: flush to the top-left corner it fused
    # with the frame stroke, and at 4x4 the cells were too small for the mass
    # to separate from the ruling. Square frame, not ICO.grid's rounded one.
    "gCross":   {"s": [("R", 3.5, 3.5, 17, 17, 0)],
                 "t": ["M3.5 9.17 H20.5", "M3.5 14.83 H20.5",
                       "M9.17 3.5 V20.5", "M14.83 3.5 V20.5"],
                 "f": [("FR", 15.6, 9.94, 4.2, 4.2)]},

    # Four separate tiles with real gutters, not a ruled grid: as a ruled grid
    # this was gCross with the solid cell moved, which is not an icon.
    "gTiles":   {"s": [("R", 3.5, 3.5, 7.5, 7.5, 1), ("R", 13, 3.5, 7.5, 7.5, 1),
                       ("R", 3.5, 13, 7.5, 7.5, 1)],
                 "f": [("FR", 13, 13, 7.5, 7.5)]},

    "gDrop":    {"s": [("R", 3.5, 4, 17, 16, 1)],
                 "t": ["M16 7 L18.5 9.5 L16 12 L13.5 9.5 Z"],
                 "f": ["M8 12 L10.5 14.5 L8 17 L5.5 14.5 Z"]},

    "gDisc":    {"s": [("R", 3.5, 3.5, 17, 17, 1)],
                 "t": [("C", 15.5, 14, 2.8)],
                 "f": [("FC", 8.5, 9, 2.8)]},

    "gBulb":    {"s": [("C", 12, 9.5, 5.5), "M9.5 17.5 H14.5", "M10.5 20 H13.5"],
                 "t": ["M10 15 H14"],
                 "f": [("FC", 12, 9.5, 2.2)]},

    "gWave":    {"s": ["M4 14 L8 10 L12 14 L16 10 L20 14"],
                 "f": [("FR", 10.75, 17.5, 2.5, 2.5)]},

    "gMaze":    {"s": [("R", 3.5, 3.5, 17, 17, 0)],
                 "t": ["M3.5 8.5 H12", "M16 8.5 H20.5", "M8 13 H20.5", "M3.5 17 H12"],
                 "f": [("FR", 16.2, 17.4, 2.6, 2.6)]},

    "gCrate":   {"s": [("R", 4.5, 4.5, 15, 15, 0),
                       "M4.5 4.5 L19.5 19.5", "M19.5 4.5 L4.5 19.5"],
                 "f": [("FR", 10.6, 10.6, 2.8, 2.8)]},

    "gBird":    {"s": ["M13.6 10.4 L17.6 6.4"],
                 "t": ["M7 13.5 H10.5"],
                 "f": [("FC", 10, 12, 4.2)],
                 "k": [("FR", 10.4, 10.2, 1.8, 1.8)]},

    # The ticks are thin and stop short of the ring - at W_MAIN reaching y=6
    # they ran into a circle whose top edge is at 5.6 and the crosshair fused
    # to the sight. Same tick geometry the prototype uses on ICO.set.
    "gTarget":  {"s": [("C", 12, 12, 6.4)],
                 "t": ["M12 2.5 V5", "M12 19 V21.5", "M2.5 12 H5", "M19 12 H21.5"],
                 "f": [("FC", 12, 12, 2.4)]},

    "gTower":   {"s": ["M12 4 V19.5", "M4 19.5 H20"],
                 "t": [("R", 6.5, 10.2, 11, 3, 0), ("R", 4.5, 14.2, 15, 3, 0)],
                 "f": [("FR", 8.5, 6.4, 7, 2.8)]},

    "gSticks":  {"s": ["M7 5 V19", "M17 5 V19"],
                 "f": [("FR", 10.8, 5, 2.4, 14)]},

    "gCard":    {"s": [("R", 5, 3.5, 14, 17, 1)],
                 "f": ["M12 8 L15.5 11.5 L12 15 L8.5 11.5 Z"]},

    "gFrog":    {"s": ["M4.5 19 H19.5"],
                 "f": ["M6.5 17.5 V13 A5.5 5.5 0 0 1 17.5 13 V17.5 Z"],
                 "k": [("FC", 9.6, 11.5, 1.5), ("FC", 14.4, 11.5, 1.5)]},

    "gWord":    {"s": [("R", 3.5, 5.5, 17, 13, 0)],
                 "t": ["M9.17 5.5 V18.5", "M14.83 5.5 V18.5"],
                 "f": [("FR", 4.6, 6.6, 3.4, 10.8)]},
}

# The marks that legitimately carry NO solid mass. They are not pictures of
# anything - they are the same class of glyph as the prototype's own window
# buttons, which are stroke-only.
CONTROLS = {"search", "close", "check", "chevron", "minimize", "maximize",
            "restore", "menu", "refresh", "plus", "folder_add", "download",
            "upload", "copy", "external", "back", "forward", "play", "pause"}


# ---- SVG path handling ----------------------------------------------------

_NUM = re.compile(r'[-+]?(?:\d*\.\d+|\d+\.?\d*)(?:[eE][-+]?\d+)?')


class _PathReader:
    """SVG path-data scanner.

    Not a general one - it covers exactly the commands the table uses
    (M L H V C S A Z and their relative forms) and raises on anything else,
    which is the point: a silently-ignored command would draw a wrong glyph.

    The one trap worth naming: an arc's large-arc and sweep parameters are
    FLAGS, single characters, and a minifier packs them against the next
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
    """path data -> [(points, closed), ...] in DESIGN units (the 24-unit box)

    Each point is (x, y, curved). `curved` marks a point emitted by a curve
    command (C/S/A), and therefore a segment the 45/90 armature check must
    skip: PRESSWORK permits a curve where the object is physically round, and
    a flattened arc is thousands of tiny off-axis chords. Points from
    M/L/H/V/Z are AUTHORED straight segments and are checked.
    """
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
            pts, prev_c2 = [(x, y, False)], None
        elif k == "L":
            x, y = r.num(), r.num()
            if rel:
                x, y = x + cx, y + cy
            cx, cy = x, y
            pts.append((x, y, False))
            prev_c2 = None
        elif k == "H":
            x = r.num()
            cx = x + cx if rel else x
            pts.append((cx, cy, False))
            prev_c2 = None
        elif k == "V":
            y = r.num()
            cy = y + cy if rel else y
            pts.append((cx, cy, False))
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
            pts.extend((px, py, True)
                       for px, py in _bezier(cx, cy, x1, y1, x2, y2, x, y))
            prev_c2, cx, cy = (x2, y2), x, y
        elif k == "A":
            rx, ry, rot = r.num(), r.num(), r.num()
            large, sweep = r.flag(), r.flag()
            x, y = r.num(), r.num()
            if rel:
                x, y = x + cx, y + cy
            pts.extend((px, py, True)
                       for px, py in _svg_arc(cx, cy, rx, ry, rot, large, sweep, x, y))
            cx, cy, prev_c2 = x, y, None
        elif k == "Z":
            if pts and len(pts) > 1:
                subs.append((pts, True))
            cx, cy = sx, sy                        # a relative command after z is
            pts, prev_c2 = [(sx, sy, False)], None  # relative to the SUBPATH start
        else:
            raise ValueError("unsupported SVG path command %r in %r" % (cmd, dstr))
    if pts and len(pts) > 1:
        subs.append((pts, False))
    return subs


def _armature_violations(item):
    """count authored straight segments in one table item that break 45/90"""
    if not isinstance(item, str):
        return 0                               # R/C/FR/FC are axis-aligned or round
    bad = 0
    for pts, closed in _flatten(item):
        ring_pts = pts + [pts[0]] if closed else pts
        for a, b in zip(ring_pts, ring_pts[1:]):
            if a[2] or b[2]:                   # a curve's chord, exempt by rule
                continue
            if _armature_bad(a, b):
                bad += 1
    return bad


# ---- turning one table entry into geometry --------------------------------

def _rect_box(x, y, w, h, grow=0.0):
    """design-space rect -> master bbox, expanded by `grow` master units

    PIL draws a rectangle outline INWARD from its bbox; SVG centres the stroke
    on the path. Growing the bbox by half the stroke width puts the two in
    agreement, which is what makes a 24-unit coordinate copied off the
    prototype land where the prototype puts it. `ring()` already did this for
    circles; this is the same correction for rectangles.
    """
    return [x * PWSC - grow, y * PWSC - grow,
            (x + w) * PWSC + grow, (y + h) * PWSC + grow]


def _draw_item(d, item, width):
    """stroke one table item at `width` master units"""
    if isinstance(item, str):
        for pts, closed in _flatten(item):
            p = [(px * PWSC, py * PWSC) for px, py, _c in pts]
            if closed:
                p.append(p[0])
            _stroke(d, p, width)
        return
    kind = item[0]
    if kind == "R":
        _, x, y, w, h, r = item
        _weights_seen.add(width)
        box = _rect_box(x, y, w, h, grow=width / 2.0)
        if r > 0:
            d.rounded_rectangle(box, radius=r * PWSC + width / 2.0,
                                outline=INK, width=width)
        else:
            d.rectangle(box, outline=INK, width=width)
        return
    if kind == "C":
        _, cx, cy, r = item
        ring(d, cx * PWSC, cy * PWSC, r * PWSC, width=width)
        return
    raise ValueError("%r is a fill, not a stroke" % (item,))


def _fill_item(d, item, colour):
    """fill one table item - the solid mass, or a knockout out of it"""
    if isinstance(item, str):
        for pts, _closed in _flatten(item):
            if len(pts) >= 3:
                d.polygon([(px * PWSC, py * PWSC) for px, py, _c in pts], fill=colour)
        return
    kind = item[0]
    if kind == "FR":
        _, x, y, w, h = item
        # PIL's rectangle bounds are INCLUSIVE, so the far edge is one master
        # unit short of the mathematical edge - otherwise the mass bleeds a
        # quarter-covered column past where the prototype puts it.
        box = _rect_box(x, y, w, h)
        d.rectangle([box[0], box[1], box[2] - 1, box[3] - 1], fill=colour)
        return
    if kind == "FC":
        _, cx, cy, r = item
        dot(d, cx * PWSC, cy * PWSC, r * PWSC, fill=colour)
        return
    raise ValueError("%r is a stroke, not a fill" % (item,))


def pw_icon(name):
    """build a draw function for one PW entry"""
    spec = PW[name]

    def draw(d):
        for it in spec.get("s", ()):
            _draw_item(d, it, W_MAIN)
        for it in spec.get("t", ()):
            _draw_item(d, it, W_THIN)
        for it in spec.get("f", ()):
            _fill_item(d, it, INK)
        for it in spec.get("k", ()):
            _fill_item(d, it, ERASE)
    draw.__name__ = "pw_" + name
    return draw


# ---- the atlas order ------------------------------------------------------
#
# THIS ORDER IS AN ABI. An atlas index is an integer burned into kernel.zl,
# apps_registry.zl and wmshot.c (wmshot.c:193 walks 0..n, kernel.zl's
# dock_icon() returns bare numbers), and fb_icon24 takes it as an int with only
# a bounds check. Reordering silently swaps every icon on screen. Append only -
# never insert, never reorder, never rename.
ORDER = [
    # app launchers
    "terminal", "snake", "paint", "editor", "monitor", "about", "cube3d",
    "files", "settings", "power",
    # the control set
    "search", "lock", "drive", "close", "check", "chevron", "clock",
    "network", "volume", "grid", "browser", "home", "minimize", "maximize",
    "restore", "menu", "refresh", "plus", "folder_add", "trash", "download",
    "upload", "copy", "paste", "warning", "bell", "back", "forward",
    "external", "user", "calendar", "camera", "play", "pause",
    # system apps
    "log", "render", "hex", "snap", "tty", "calcApp", "font",
    "disk", "svc", "binary", "imgv", "chip",
    # utilities
    "uClip", "uColor", "uRegex", "uBase", "uDiff",
    "uHash", "uUnit", "uNote", "uKeys", "uBench",
    # games
    "gTetris", "gPong", "gBrick", "gMine", "gNum", "gLife", "gRock", "gAlien",
    "gCross", "gTiles", "gDrop", "gDisc", "gBulb", "gWave", "gMaze", "gCrate",
    "gBird", "gTarget", "gTower", "gSticks", "gCard", "gFrog", "gWord",
]

ICONS = [(nm, pw_icon(nm)) for nm in ORDER]


# ---- the three checks -----------------------------------------------------

def check_table():
    """armature, mass and coverage checks over PW. Raises rather than warns.

    Run before anything is rasterised, so a rule violation costs a second
    rather than a full render. Returns the per-icon off-axis tally so main()
    can print the measured number instead of asserting one.
    """
    missing = [nm for nm in ORDER if nm not in PW]
    extra   = [nm for nm in PW if nm not in ORDER]
    if missing or extra:
        raise SystemExit("PW/ORDER disagree: missing %r, unused %r" % (missing, extra))

    off = {}
    for nm in ORDER:
        spec = PW[nm]
        bad = sum(_armature_violations(it)
                  for key in ("s", "t", "f", "k")
                  for it in spec.get(key, ()))
        allowed = OFF_AXIS_OK.get(nm, (0, ""))[0]
        if bad != allowed:
            raise SystemExit(
                "%s: %d off-axis segment(s), %d declared in OFF_AXIS_OK.\n"
                "PRESSWORK's armature is 45/90 only. Either straighten the "
                "segment or declare the exception WITH ITS REASON." % (nm, bad, allowed))
        if bad:
            off[nm] = bad

        masses = len(spec.get("f", ()))
        want = 0 if nm in CONTROLS else 1
        if masses != want:
            raise SystemExit(
                "%s: %d solid mass(es), expected %d. PRESSWORK allows exactly one "
                "per pictorial icon and none on a control mark; %s is listed as %s."
                % (nm, masses, want, nm,
                   "a control" if nm in CONTROLS else "pictorial"))
        if not spec.get("s") and not spec.get("t") and not spec.get("f"):
            raise SystemExit("%s: empty icon" % nm)

    stale = [nm for nm in OFF_AXIS_OK if nm not in off]
    if stale:
        raise SystemExit("OFF_AXIS_OK names icons that no longer break the "
                         "armature: %r. Delete the stale exemption." % stale)
    return off


# ---- render + emit --------------------------------------------------------

class ScaledDraw:
    """An ImageDraw that multiplies every length by `k` before it hits PIL.

    The icons above are written in ONE coordinate space - the 24-unit design
    box, scaled to the 96-unit master by PWSC. To get a 48x48 atlas we cannot
    simply box-filter the 96x96 master by 2x2: a 4x4 supersample gives 17
    coverage levels, a 2x2 gives FIVE, and five levels is a staircase again.
    The master has to grow with the output.

    So rather than parameterise every drawing call by size, the geometry stays
    in 96-unit space and this proxy scales it on the way through. Everything
    the icons call goes via a point list plus `width`/`radius`, all of which
    are lengths; `start`/`end` are ANGLES and must not be touched.
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
    # an exact integer ratio keeps every scaled literal an exact integer too
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
    off = check_table()

    # one atlas per output size, each rasterized from the geometry at that size
    atlases = [(n, [(nm, render(fn, n)) for nm, fn in ICONS]) for n in SIZES]

    # The weight rule, checked against what ACTUALLY reached PIL rather than
    # against what the table says. Every wide-line call goes through _stroke or
    # ring, both of which record their width, so a third weight cannot hide in
    # a helper. Note the 48x48 pass scales widths through ScaledDraw AFTER this
    # record is taken - the ladder is defined in master units, one ladder for
    # both sizes, which is the point.
    if _weights_seen != {W_MAIN, W_THIN}:
        raise SystemExit(
            "PRESSWORK allows exactly two stroke weights, %d and %d master units "
            "(2.0 and 1.5 design px). These reached PIL: %s"
            % (W_MAIN, W_THIN, sorted(_weights_seen)))

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
        f.write(" * PRESSWORK icon language, checked by the generator every run:\n")
        f.write(" *   - 45/90 armature: every authored straight segment is horizontal,\n")
        f.write(" *     vertical or exactly 45 degrees. Curves only where the object is\n")
        f.write(" *     physically round.\n")
        f.write(" *   - two stroke weights only: 2.0 design px silhouette, 1.5 interior.\n")
        f.write(" *   - exactly one solid mass per pictorial icon, none on a control mark.\n")
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

    # The PRESSWORK compliance report. Every number here is counted, not
    # claimed: the weights are what actually reached PIL, the off-axis tally is
    # what the armature check actually found.
    print("\n  PRESSWORK:")
    print("    stroke weights that reached PIL: %s (design px: %s)"
          % (sorted(_weights_seen),
             ", ".join("%.2f" % (w / float(SS)) for w in sorted(_weights_seen))))
    print("    transcribed verbatim from the prototype: %d of %d"
          % (len(PROTO), len(ORDER)))
    print("    control marks (no solid mass): %d; pictorial (one each): %d"
          % (len(CONTROLS), len(ORDER) - len(CONTROLS)))
    print("    off-axis segments, all declared: %d across %d icon(s) %s"
          % (sum(off.values()), len(off), sorted(off)))
    for nm in sorted(off):
        print("      %-10s %d  %s" % (nm, off[nm], OFF_AXIS_OK[nm][1]))

    for nm in ("terminal", "cube3d", "power", "chip", "uBench", "gTarget"):
        preview(nm, dict(atlases[0][1])[nm])


if __name__ == "__main__":
    main()
