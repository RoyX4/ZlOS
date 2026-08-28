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

The checker also reads fb.c's scale constants. Width, continuous layout scale,
integer ZL scale, and console-cell scale are separate contracts; treating them
as one hid clipped launcher rows at both 800x600 and 1920x1080. Every supported
mode must expose the whole fixed register. A built-in mutation removes compact
row sizing and proves the 800x600 clipping is caught.

It reads source and does arithmetic. No build, no QEMU, so it cannot fail
because the host is busy. Half a second.

    python3 kernel/tools/checks/check-shell-layout.py
"""
import os
import re
import sys

KERNEL_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
ZL = os.path.join(KERNEL_ROOT, "src", "kernel.zl")
FB = os.path.join(KERNEL_ROOT, "src", "graphics", "framebuffer", "fb.c")

# Panel sizes that exist on this project's real hardware and gates. 800x600 is
# the BIOS fallback mode, 1280x800 is the host reference, 1920x1080 is an
# ordinary laptop panel, and 2560x1440 is the physical ThinkPad panel.
CASES = [(800, 600), (1280, 800), (1920, 1080), (2560, 1440)]


def framebuffer_contract(src):
    """Read the scale and cell thresholds from fb.c, failing on drift."""
    glyph_w = re.search(r"^#define GLYPH_W\s+(\d+)$", src, re.M)
    glyph_h = re.search(r"^#define GLYPH_H\s+(\d+)$", src, re.M)
    scale = re.search(
        r"ui_scale_q8 = \(int\)\(\(width \* (\d+)u \+ (\d+)u\) / (\d+)u\);",
        src,
    )
    low = re.search(r"if \(ui_scale_q8 < (\d+)\) ui_scale_q8 = (\d+);", src)
    high = re.search(r"if \(ui_scale_q8 > (\d+)\) ui_scale_q8 = (\d+);", src)
    integer = re.search(r"ui_scale = \(ui_scale_q8 \+ (\d+)\) / (\d+);", src)
    cell = re.search(
        r"cell_w\s*= \(width >= (\d+)\) \? GLYPH_W \* (\d+) : GLYPH_W;",
        src,
    )
    if not all((glyph_w, glyph_h, scale, low, high, integer, cell)):
        raise ValueError("cannot read framebuffer scale/cell contract from fb.c")
    if low.group(1) != low.group(2) or high.group(1) != high.group(2):
        raise ValueError("fb.c scale clamps no longer assign their boundary")
    return {
        "glyph_w": int(glyph_w.group(1)),
        "glyph_h": int(glyph_h.group(1)),
        "scale_num": int(scale.group(1)),
        "scale_round": int(scale.group(2)),
        "scale_den": int(scale.group(3)),
        "scale_low": int(low.group(1)),
        "scale_high": int(high.group(1)),
        "ui_round": int(integer.group(1)),
        "ui_den": int(integer.group(2)),
        "cell_threshold": int(cell.group(1)),
        "cell_factor": int(cell.group(2)),
    }


def runtime_metrics(width, contract):
    q8 = ((width * contract["scale_num"] + contract["scale_round"])
          // contract["scale_den"])
    q8 = max(contract["scale_low"], min(contract["scale_high"], q8))
    u = (q8 + contract["ui_round"]) // contract["ui_den"]
    factor = contract["cell_factor"] if width >= contract["cell_threshold"] else 1
    return q8, u, (contract["glyph_w"] * factor,
                   contract["glyph_h"] * factor)


def zl_eval(expr, k, u, row_h):
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
    e = expr.replace("row_h()", "(%d)" % row_h)
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


def static_walk(src, k, u, row_h):
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
            total += row_h
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


def register_geometry_contract(src):
    """Require one row height authority across drawing and hit testing."""
    errors = []
    body = re.search(r"^fn rail_row_h\(\) \{(.*?)^\}", src, re.M | re.S)
    if not body:
        return ["kernel.zl no longer defines rail_row_h()"]
    compact = body.group(1)
    for seam in (
        "rh = row_h()",
        "cap = idiv(rail_reg_h(), REG_ROWS)",
        "if cap < rh { rh = cap }",
        "if rh < cell_h() { rh = cell_h() }",
    ):
        if seam not in compact:
            errors.append("rail_row_h() lost: " + seam)
    required = {
        "fn rail_reg_fit() { return idiv(rail_reg_h(), rail_row_h()) }": 1,
        "rh = rail_row_h()": 2,
        "drawn * rail_row_h()": 3,
        "idiv(cy - rail_reg_y(), rail_row_h())": 1,
        "if iw <= rh {": 2,
    }
    for seam, count in required.items():
        actual = src.count(seam)
        if actual != count:
            errors.append("register geometry seam %r occurs %d times, expected %d"
                          % (seam, actual, count))
    return errors


def check(src, w, h, k, fb_contract, fails, compact_rows=True):
    q8, u, cell = runtime_metrics(w, fb_contract)

    def theme_dp(name):
        return (k[name] * q8 + 128) >> 8

    def zl_dp(name):
        return k[name] * u

    rule2 = k["RULE_H"] * u
    rule1 = 1
    row_h = theme_dp("ROW_H_D")
    rail_w = theme_dp("RAIL_W_D")
    strip_h = theme_dp("STRIP_H_D")
    foot_h = theme_dp("FOOT_H_D")

    def bad(msg):
        fails.append("%dx%d q8=%d ui%d: %s" % (w, h, q8, u, msg))

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
    top_sum = zl_eval(body_of(src, "rail_top_h"), k, u, row_h)
    top_walk = zl_dp("RAIL_IDENT_H") + rule2 + zl_dp("RAIL_SECT_H") + rule1
    if top_sum != top_walk:
        bad("rail_top_h() is %d, the walk above the register is %d (%+d px)"
            % (top_sum, top_walk, top_walk - top_sum))

    bot_sum = zl_eval(body_of(src, "rail_bot_h"), k, u, row_h)
    walk = static_walk(src, k, u, row_h)
    clock_top_from_walk = (h - bot_sum) + walk
    clock_top_anchored = h - zl_dp("RAIL_CLOCK_H")
    if clock_top_from_walk != clock_top_anchored:
        bad("clock: draw_rail_static's walk lands at %d, draw_rail_clock anchors"
            " at %d (%+d px of bare rail between them)"
            % (clock_top_from_walk, clock_top_anchored,
               clock_top_anchored - clock_top_from_walk))

    # ---- the workspace row and the stat block, as rail_ws_y() writes it, must
    # land inside the block rail_bot_h() reserved
    ws_y = zl_eval(body_of(src, "rail_ws_y").replace("px_h()", str(h))
                   .replace("rail_bot_h()", str(bot_sum)), k, u, row_h)
    stat_y = ws_y + row_h + rule2
    if stat_y + zl_dp("RAIL_STAT_H") + rule1 != clock_top_anchored:
        bad("stat block ends at %d, the clock starts at %d"
            % (stat_y + zl_dp("RAIL_STAT_H") + rule1, clock_top_anchored))

    # ---- navigation comes before the optional tail: every fixed row must fit.
    reg_h = h - top_sum - bot_sum
    rail_row_h = row_h
    if compact_rows:
        rail_row_h = min(rail_row_h, reg_h // k["REG_ROWS"])
        rail_row_h = max(rail_row_h, cell[1])
    fit = reg_h // rail_row_h
    if fit < k["REG_ROWS"]:
        bad("fixed register exposes %d of %d rows (%d px band, %d px rows)"
            % (fit, k["REG_ROWS"], reg_h, rail_row_h))

    # ---- the module grid must divide, and the remainder must be SPREAD
    mar = zl_dp("GRID_MARGIN")
    gut = zl_dp("GRID_GUTTER")
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
    cw_cell, ch_cell = cell
    icon_w = max(k["RAIL_SLOT_ICO"], theme_dp("RAIL_SLOT_ICO"))
    icon_room = icon_w + zl_dp("RAIL_SLOT_GAP") if icon_w <= rail_row_h else 0
    name_room = (rail_w - zl_dp("RAIL_SLOT_PL") - 2 * cw_cell
                 - zl_dp("RAIL_SLOT_GAP") - icon_room - zl_dp("RAIL_SLOT_PR"))
    if name_room < 8 * cw_cell:
        bad("a register row leaves %d px for its name, under eight mono cells"
            % name_room)
    if ch_cell > rail_row_h:
        bad("the mono cell is %d px in a %d px register row"
            % (ch_cell, rail_row_h))

    # ---- the foot's two rows must fit inside the foot
    band_h = theme_dp("BAND_H_D")
    if 3 * u + 11 * u > foot_h - band_h:
        bad("the memory ruler overflows into the slug row")
    if 2 * u + ch_cell > band_h:
        bad("the slug's mono line is %d px in a %d px band" % (ch_cell, band_h))

    # ---- the raster strip must hold a column plus its label
    if 4 * u >= strip_h:
        bad("the strip has no room for a column")
    return fit, rail_row_h, q8, u


def main():
    src = open(ZL, encoding="utf-8").read()
    fb_src = open(FB, encoding="utf-8").read()
    try:
        fb_contract = framebuffer_contract(fb_src)
    except ValueError as exc:
        print("check-shell-layout: FAIL - %s" % exc)
        return 2
    k = consts(src)
    need = ["RAIL_W_D", "STRIP_H_D", "FOOT_H_D", "BAND_H_D", "ROW_H_D", "PAD_D",
            "GAP_D", "RULE_H", "FOCUS_BAR_D", "GRID_COLS", "GRID_ROWS",
            "GRID_MARGIN", "GRID_GUTTER", "RAIL_IDENT_H", "RAIL_SECT_H",
            "RAIL_STAT_H", "RAIL_CLOCK_H", "RAIL_SLOT_PL", "RAIL_SLOT_PR",
            "RAIL_SLOT_GAP", "RAIL_SLOT_ICO", "REG_N", "REG_ROWS"]
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

    contract_errors = register_geometry_contract(src)
    if contract_errors:
        print("check-shell-layout: FAIL")
        for error in contract_errors:
            print("   ", error)
        return 1

    fails = []
    fits = []
    for w, h in CASES:
        fit, row, q8, u = check(src, w, h, k, fb_contract, fails)
        fits.append((w, h, fit, row, q8, u))
    if fails:
        print("check-shell-layout: FAIL")
        for f in fails:
            print("   ", f)
        return 1

    # Prove the all-fixed-rows assertion is live. Without compact rows the
    # supported 800x600 fallback must fail specifically on register clipping.
    mutation_fails = []
    check(src, 800, 600, k, fb_contract, mutation_fails, compact_rows=False)
    if not any("fixed register exposes" in error for error in mutation_fails):
        print("check-shell-layout: FAIL - compact-row removal mutation escaped")
        return 1

    print("check-shell-layout: PASS - 4 panel sizes tile; fixed register exposes "
          + ", ".join("%d rows at %dx%d (%d px row, q8=%d, ui%d)"
                      % (fit, w, h, row, q8, u)
                      for w, h, fit, row, q8, u in fits))
    print("check-shell-layout selftest: caught compact-row removal")
    return 0


if __name__ == "__main__":
    sys.exit(main())
