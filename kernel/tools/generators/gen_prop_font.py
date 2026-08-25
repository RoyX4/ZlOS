#!/usr/bin/env python3
# gen_prop_font.py - the PROPORTIONAL companion to gen_hd_font.py.
#
# docs/desktop/desktop-look.md's item 4, and it calls it "the one that stops the desktop
# reading as a terminal":
#
#     fb_text_aa advances by exactly cell_w per character, so EVERY string on
#     screen is monospace-positioned, including window titles and dock labels.
#     Uniform advance is the single strongest visual signal of "this is a
#     terminal", and it is applied to things that are not terminals.
#
# That document proposes emitting per-glyph advance widths from the existing
# atlas. THAT ALONE DOES NOTHING, and it is worth saying why before anyone
# tries it: the atlas font is DejaVu Sans **Mono**. Every advance FreeType
# reports for it is the same number, so a per-glyph advance table would be 95
# copies of one value and the screen would not change by a pixel.
#
# Real proportional text needs a proportional FACE. This uses DejaVu Sans - the
# proportional cut of the same superfamily, same designer, same shapes, drawn
# for exactly this purpose. That is a reading of "no new fonts" that keeps the
# rule's intent (do not introduce a second visual system) while getting the
# thing the rule was never about.
#
#   font8x16_prop[95][16][10]   +  prop8_adv[95]
#   font16x32_prop[95][32][20]  +  prop16_adv[95]
#
# THE CELL IS WIDER THAN THE ADVANCE, on purpose. A proportional glyph can be
# wider than the advance it hands on - 'm' and 'W' notably - and can carry a
# negative left bearing. The cell is sized to the widest INK box so nothing is
# ever cropped, while the pen still moves by the real advance. Cropping a glyph
# to its advance is the classic way proportional text ends up with clipped
# round letters.
#
# The console keeps its grid and keeps the mono atlas. Only titles and labels
# change, which is exactly what docs/desktop/desktop-look.md asks for.

import os
from PIL import Image, ImageDraw, ImageFont

KERNEL_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
FONT_DIR = os.path.join(KERNEL_ROOT, "src", "graphics", "fonts")
OUT = os.path.join(FONT_DIR, "font_prop.inc")
FONT      = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
FONT_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"

# THE TYPE SCALE, v10 SS6.8. Counted from the prototype rather than chosen:
# its font-size declarations cluster at 9.5-13 px for body text, 15 and 19 for
# headings, and it uses font-weight:700 fourteen times against a 400 default.
# So three sizes and two weights is what the source actually asks for.
#
# These are CELL HEIGHTS in pixels, not point sizes. 16 is the console cell at
# ui() == 1 and 32 is the console cell at ui() == 2, so those two already
# existed; 24 is the new middle step, and it is what makes a scale rather than
# a big-or-small switch.
CELLS = (16, 24, 32)
FIRST, LAST = 32, 126
CHARS = [chr(c) for c in range(FIRST, LAST + 1)]


def best_size(cell_h, path=FONT):
    """largest size whose line box fits the cell height"""
    best = 6
    for s in range(6, cell_h * 3):
        f = ImageFont.truetype(path, s)
        asc, desc = f.getmetrics()
        if asc + desc <= cell_h + 2:
            best = s
    return best


def measure(font):
    """(cell width, advances, ink widths). The cell must hold the widest INK
    box, including any part that sits left of the pen (a negative bearing).

    The per-glyph INK width is what the renderer actually has to blit. Drawing
    the full cell for every glyph is correct - coverage is zero outside the ink
    - but at title size the cell is 30px and a comma's ink is 4, so it is up to
    7x the pixels touched for nothing. Measured at 35% over the mono path
    before this was added."""
    wide, advs, boxes = 0, [], []
    for ch in CHARS:
        adv = font.getlength(ch)
        box = font.getbbox(ch)              # (x0, y0, x1, y1), ink relative to pen
        need = max(box[2], adv)             # ink may overhang the advance
        left = min(box[0], 0)               # ...and may start left of it
        wide = max(wide, int(need - left) + 1)
        advs.append(int(round(adv)))
        boxes.append(int(need - left) + 1)
    inks = [min(b, wide) for b in boxes]
    return wide, advs, inks


def build(cell_h, path=FONT):
    size = best_size(cell_h, path)
    font = ImageFont.truetype(path, size)
    cell_w, advs, inks = measure(font)
    asc, desc = font.getmetrics()
    baseline = (cell_h + asc - desc) // 2

    glyphs = []
    for ch in CHARS:
        img = Image.new("L", (cell_w, cell_h), 0)
        d = ImageDraw.Draw(img)
        # 'ls' anchors at the left end of the baseline, i.e. AT THE PEN - which
        # is what makes the advance below actually mean something.
        d.text((0, baseline), ch, font=font, fill=255, anchor="ls")
        glyphs.append([[img.getpixel((x, y)) for x in range(cell_w)]
                       for y in range(cell_h)])
    return glyphs, advs, inks, cell_w, size


def emit(f, name, cw, ch, glyphs):
    f.write(f"const unsigned char {name}[95][{ch}][{cw}] = {{\n")
    for i, g in enumerate(glyphs):
        f.write("  { /* %d %r */\n" % (FIRST + i, CHARS[i]))
        for row in g:
            f.write("    {" + ",".join("%3d" % v for v in row) + "},\n")
        f.write("  },\n")
    f.write("};\n")


def emit_adv(f, name, advs):
    f.write(f"const unsigned char {name}[95] = {{\n  ")
    f.write(",".join("%3d" % a for a in advs))
    f.write("\n};\n")


def main():
    # six atlases: three cell heights, regular and bold in each
    built = {}
    for ch in CELLS:
        for wname, path in (("", FONT), ("b", FONT_BOLD)):
            built[(ch, wname)] = build(ch, path)

    total = 0
    with open(OUT, "w") as f:
        f.write("/* font_prop.inc - GENERATED by gen_prop_font.py. Do not edit by hand.\n")
        f.write(" *\n")
        f.write(" * INCLUDED BY fb.c, not compiled on its own. That is deliberate: fb.c is\n")
        f.write(" * in every build - 32-bit, 64-bit, the UEFI application and the raw disk\n")
        f.write(" * image - and each of those has its own source list. A new .c file would\n")
        f.write(" * have had to be added to four of them. Riding along with fb.c needs no\n")
        f.write(" * build change at all.\n")
        f.write(" *\n")
        f.write(" * DejaVu SANS - proportional, the companion to the Mono in font_aa.c.\n")
        f.write(" * UI labels and window titles draw from this; the CONSOLE keeps its\n")
        f.write(" * grid and keeps the mono atlas.\n")
        f.write(" *\n")
        f.write(" * THREE CELL HEIGHTS AND TWO WEIGHTS, v10 SS6.8. The prototype's sizes\n")
        f.write(" * cluster at 9.5-13px body, 15/19 heading, and it sets font-weight:700\n")
        f.write(" * fourteen times. 16 and 32 are the console cells at ui() 1 and 2; 24 is\n")
        f.write(" * the middle step that turns a big-or-small switch into a scale.\n")
        f.write(" *\n")
        f.write(" * The CELL IS WIDER THAN THE ADVANCE on purpose: a proportional glyph\n")
        f.write(" * can overhang the advance it hands on, and can start left of the pen.\n")
        f.write(" */\n")
        f.write('#include "font_prop.h"\n\n')
        for ch in CELLS:
            for wname in ("", "b"):
                glyphs, advs, inks, cw, sz = built[(ch, wname)]
                tag = f"{ch}{wname}"
                f.write(f"/* {ch}px cell, {'bold' if wname else 'regular'}, "
                        f"{sz}px face, advances {min(advs)}..{max(advs)} */\n")
                emit(f, f"prop{tag}", cw, ch, glyphs)
                f.write("\n")
                emit_adv(f, f"prop{tag}_adv", advs)
                f.write("\n")
                emit_adv(f, f"prop{tag}_ink", inks)
                f.write("\n")
                total += 95 * ch * cw + 190

    hdr = os.path.join(FONT_DIR, "font_prop.h")
    with open(hdr, "w") as f:
        f.write("/* font_prop.h - GENERATED by gen_prop_font.py. Do not edit.\n")
        f.write(" * The proportional atlas cell WIDTHS, so fb.c's extern declarations\n")
        f.write(" * cannot drift from the data. A mismatch here is not a compile\n")
        f.write(" * error, it is a silently misindexed array. */\n")
        f.write("#ifndef FONT_PROP_H\n#define FONT_PROP_H\n")
        for ch in CELLS:
            for wname in ("", "b"):
                cw = built[(ch, wname)][3]
                f.write(f"#define PROP{ch}{wname.upper()}_W {cw}\n")
        f.write("#endif\n")

    print(f"wrote {OUT} and {hdr}")
    for ch in CELLS:
        for wname in ("", "b"):
            glyphs, advs, inks, cw, sz = built[(ch, wname)]
            print(f"  {ch:2d}px cell {'bold  ' if wname else 'regular'}  "
                  f"face {sz:2d}px  cell {cw:2d}x{ch}  advances {min(advs)}..{max(advs)}  "
                  f"distinct {len(set(advs))}")
    print(f"  atlas bytes: {total}")


if __name__ == "__main__":
    main()
