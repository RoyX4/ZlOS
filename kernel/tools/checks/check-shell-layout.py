#!/usr/bin/env python3
"""check-shell-layout.py - PRESSWORK's four bands must TILE the screen.

WHY THIS EXISTS. The shell in kernel.zl is laid out by two kinds of code that
have to agree and cannot be made to agree by construction:

  * a BLOCK HEIGHT function - rail_top_h(), rail_bot_h() - which SUMS a list of
    terms, and
  * a WALK - draw_rail_static() - which DRAWS the same list one term at a time.

Nothing in zl checks that the sum equals the walk. The first draft of
rail_bot_h() counted three 2px rules where the walk draws two, and used ui() for
a rule that advances by exactly one physical pixel. At ui 2 that leaves five
pixels of bare rail between the stat block and the clock: no crash, no warning,
invisible in a screenshot at 100%, and wrong. The same class of mistake in the
other direction OVERLAPS the clock onto the stat block and eats a reading.

So both sides are READ OUT OF kernel.zl rather than restated here: the height
functions are parsed and evaluated as written (zl_eval / body_of), the walk is
recovered line by line out of draw_rail_static's body (static_walk), and every
named constant is parsed rather than copied. A check that transcribes the
arithmetic can only ever confirm its own copy of it.

VERIFIED IN BOTH DIRECTIONS, which is the standard docs/GUARDS-THAT-DID-NOT-GUARD.md
sets. Three defects were planted and each one turned this red with the right
diagnosis - rail_bot_h() gaining the extra rule (+2 px at ui 1, +5 at ui 2),
draw_rail_static losing one from its walk (+2 / +4), and RAIL_SLOT_PL grown
until a register row has no room for its name - and removing each returned it
to green.

It reads source and does arithmetic. No build, no QEMU, so it cannot fail
because the host is busy. Half a second.

    python3 kernel/tools/checks/check-shell-layout.py
"""
import os
import re
import sys

KERNEL_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
ZL = os.path.join(KERNEL_ROOT, "src", "kernel.zl")

# Panel sizes that exist on this project's real hardware and gates, with the ui
# scale fb.c derives for each. 800x600 is the BIOS fallback mode, 1280x800 is
# what wmshot and the reference render at, 1920x1080 is the ThinkPad's panel and
# 2560x1440 is the size the brief's own build command asks for.
CASES = [(800, 600, 1), (1280, 800, 1), (1920, 1080, 2), (2560, 1440, 2)]

# The console cell, which is what every mono advance in the shell is measured
# in. fb.c switches from 8x16 to 16x32 with the same threshold ui uses.
CELL = {1: (8, 16), 2: (16, 32)}


def zl_eval(expr, k, u):
    """Evaluate a zl height expression READ OUT OF kernel.zl.

    This is the part that makes the file a guard rather than a transcription. A
    check that restates the arithmetic can only ever confirm its own copy; this
    substitutes the two calls a height expression is allowed to contain, then
    the named constants, and evaluates what is left. If somebody adds a term to
    rail_bot_h() the number here changes with it.

    Only the shapes these expressions actually use are accepted - integers,
    named constants, ui(), row_h(), + and * - and anything else raises, so an
    expression that grows a construct this cannot read fails loudly instead of
    being half-understood."""
    e = expr.replace("row_h()", "(%d)" % (k["ROW_H_D"] * u))
    e = e.replace("ui()", "(%d)" % u)
    e = re.sub(r"[A-Z][A-Z_0-9]*", lambda m: str(k[m.group(0)]), e)
    if not re.fullmatch(r"[0-9()+*\-\s]+", e):
        raise ValueError("cannot read height expression: %r -> %r" % (expr, e))
    return eval(e)


def body_of(src, name):
    """The one-line body of `fn name() { return <expr> }`."""
    m = re.search(r"^fn\s+" + name + r"\(\)\s*\{\s*return\s+(.+?)\s*\}\s*$",
                  src, re.M)
    if not m:
        raise ValueError("kernel.zl has no one-line fn " + name)
    return m.group(1)


def static_walk(src, k, u):
    """The height draw_rail_static() actually walks for the BOTTOM block.

    Read out of the function body between `by = px_h() - rail_bot_h()` and its
    return, one advance per line, in source order. rail_rule1 advances by one
    PHYSICAL pixel and rail_rule2 by RULE_H * ui(); rail_sect advances by
    RAIL_SECT_H * ui(). Any line in that range this cannot classify raises."""
    m = re.search(r"by = px_h\(\) - rail_bot_h\(\)(.*?)\n    return 0",
                  src, re.S)
    if not m:
        raise ValueError("draw_rail_static no longer walks up from rail_bot_h()")
    total = 0
    for line in m.group(1).split("\n"):
        line = line.split("#")[0].strip()
        if not line:
            continue
        if "rail_rule2(" in line:
            total += k["RULE_H"] * u
        elif "rail_rule1(" in line:
            total += 1
        elif "rail_sect(" in line:
            total += k["RAIL_SECT_H"] * u
        elif line.startswith("by = by + row_h()"):
            total += k["ROW_H_D"] * u
        elif line.startswith("by = by + RAIL_STAT_H * u"):
            total += k["RAIL_STAT_H"] * u
        else:
            raise ValueError("unreadable line in draw_rail_static's walk: " + line)
    return total


def consts(src):
    """Pull `NAME = <int>` out of kernel.zl. Only the plain-integer form, so a
    computed constant cannot be silently half-read."""
    out = {}
    for m in re.finditer(r"^([A-Z][A-Z_0-9]*)\s*=\s*(\d+)\s*(#.*)?$", src, re.M):
        out[m.group(1)] = int(m.group(2))
    return out


def check(src, w, h, u, k, fails):
    def dp(name):
        return k[name] * u

    rule2 = k["RULE_H"] * u
    rule1 = 1
    row_h = dp("ROW_H_D")
    rail_w = dp("RAIL_W_D")
    strip_h = dp("STRIP_H_D")
    foot_h = dp("FOOT_H_D")

    def bad(msg):
        fails.append("%dx%d ui%d: %s" % (w, h, u, msg))

    # ---- the four bands must tile the screen vertically, on the field side
    field_y = strip_h + rule2
    field_h = h - field_y - foot_h - rule2
    foot_y = h - foot_h
    if field_y + field_h + rule2 != foot_y:
        bad("field bottom %d + rule != foot top %d" % (field_y + field_h, foot_y))
    if field_h <= 0:
        bad("field has no height (%d)" % field_h)
    if rail_w >= w:
        bad("rail %d is wider than the screen" % rail_w)

    # ---- rail_top_h / rail_bot_h, AS kernel.zl WRITES THEM, must equal the
    # walk draw_rail_static makes. Both sides are read out of the source.
    top_sum = zl_eval(body_of(src, "rail_top_h"), k, u)
    top_walk = dp("RAIL_IDENT_H") + rule2 + dp("RAIL_SECT_H") + rule1
    if top_sum != top_walk:
        bad("rail_top_h() is %d, the walk above the register is %d (%+d px)"
            % (top_sum, top_walk, top_walk - top_sum))

    bot_sum = zl_eval(body_of(src, "rail_bot_h"), k, u)
    walk = static_walk(src, k, u)
    clock_top_from_walk = (h - bot_sum) + walk
    clock_top_anchored = h - dp("RAIL_CLOCK_H")
    if clock_top_from_walk != clock_top_anchored:
        bad("clock: draw_rail_static's walk lands at %d, draw_rail_clock anchors"
            " at %d (%+d px of bare rail between them)"
            % (clock_top_from_walk, clock_top_anchored,
               clock_top_anchored - clock_top_from_walk))

    # ---- the workspace row and the stat block, as rail_ws_y() writes it, must
    # land inside the block rail_bot_h() reserved
    ws_y = zl_eval(body_of(src, "rail_ws_y").replace("px_h()", str(h))
                   .replace("rail_bot_h()", str(bot_sum)), k, u)
    stat_y = ws_y + row_h + rule2
    if stat_y + dp("RAIL_STAT_H") + rule1 != clock_top_anchored:
        bad("stat block ends at %d, the clock starts at %d"
            % (stat_y + dp("RAIL_STAT_H") + rule1, clock_top_anchored))

    # ---- the register band must have room for at least one row
    reg_h = h - top_sum - bot_sum
    if reg_h < row_h:
        bad("register band is %d px, one row is %d - no row fits" % (reg_h, row_h))
    fit = reg_h // row_h
    if fit < 1:
        bad("register fits %d rows" % fit)

    # ---- the module grid must divide, and the remainder must be SPREAD
    mar = dp("GRID_MARGIN")
    gut = dp("GRID_GUTTER")
    inw = (w - rail_w) - 2 * mar
    inh = field_h - 2 * mar
    cols, rows = k["GRID_COLS"], k["GRID_ROWS"]
    cw = (inw - gut * (cols - 1)) // cols
    cwr = (inw - gut * (cols - 1)) - cw * cols
    rh = (inh - gut * (rows - 1)) // rows
    rhr = (inh - gut * (rows - 1)) - rh * rows
    if cw < 1 or rh < 1:
        bad("module is %dx%d px - the grid does not divide here" % (cw, rh))
    else:
        # the spread must reach exactly the far margin: sum of widths plus every
        # gutter must equal the inner width, or the right margin is not a margin
        if cw * cols + cwr + gut * (cols - 1) != inw:
            bad("columns do not sum to the inner width")
        if rh * rows + rhr + gut * (rows - 1) != inh:
            bad("rows do not sum to the inner height")
        if cwr >= cols or rhr >= rows:
            bad("remainder %d/%d is not smaller than the count" % (cwr, rhr))

    # ---- the register row must fit its own contents
    cw_cell, ch_cell = CELL[u]
    name_room = (rail_w - dp("RAIL_SLOT_PL") - 2 * cw_cell
                 - dp("RAIL_SLOT_GAP") - dp("RAIL_SLOT_PR"))
    if name_room < 8 * cw_cell:
        bad("a register row leaves %d px for its name, under eight mono cells"
            % name_room)
    if ch_cell > row_h:
        bad("the mono cell is %d px in a %d px row" % (ch_cell, row_h))

    # ---- the foot's two rows must fit inside the foot
    band_h = dp("BAND_H_D")
    if 3 * u + 11 * u > foot_h - band_h:
        bad("the memory ruler overflows into the slug row")
    if 2 * u + ch_cell > band_h:
        bad("the slug's mono line is %d px in a %d px band" % (ch_cell, band_h))

    # ---- the raster strip must hold a column plus its label
    if 4 * u >= strip_h:
        bad("the strip has no room for a column")
    return fit


def main():
    src = open(ZL).read()
    k = consts(src)
    need = ["RAIL_W_D", "STRIP_H_D", "FOOT_H_D", "BAND_H_D", "ROW_H_D", "PAD_D",
            "GAP_D", "RULE_H", "FOCUS_BAR_D", "GRID_COLS", "GRID_ROWS",
            "GRID_MARGIN", "GRID_GUTTER", "RAIL_IDENT_H", "RAIL_SECT_H",
            "RAIL_STAT_H", "RAIL_CLOCK_H", "RAIL_SLOT_PL", "RAIL_SLOT_PR",
            "RAIL_SLOT_GAP", "REG_N", "REG_ROWS"]
    missing = [n for n in need if n not in k]
    if missing:
        print("check-shell-layout: FAIL - kernel.zl no longer defines %s"
              % ", ".join(missing))
        return 2

    # the register table and the row count have to agree, or rail_launch routes
    # a fixed row into the tail
    if k["REG_ROWS"] != k["REG_N"] + 1:
        print("check-shell-layout: FAIL - REG_ROWS %d is not REG_N %d plus the "
              "catalog row" % (k["REG_ROWS"], k["REG_N"]))
        return 1

    fails = []
    fits = []
    for w, h, u in CASES:
        fits.append((w, h, check(src, w, h, u, k, fails)))
    if fails:
        print("check-shell-layout: FAIL")
        for f in fails:
            print("   ", f)
        return 1
    print("check-shell-layout: PASS - 4 panel sizes tile; register fits "
          + ", ".join("%d rows at %dx%d" % (f, w, h) for w, h, f in fits))
    return 0


if __name__ == "__main__":
    sys.exit(main())
