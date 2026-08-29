#!/usr/bin/env python3
"""check-intdiv.py - `/` is FLOAT division in zl, and indices must not be float.

WHY THIS EXISTS. In the week before this was written, NINETEEN separate defects
in this tree came from the same line shape:

    row = (ey - y0) / rowh          # float, so `row` is 3.5, not 3

Every one of them was invisible in review, because that line is integer division
in every language the reader has used. zl's `/` is not: every value is a double
and zl_binop returns a double, so the result carries a fraction. What happened
next depended on where the value went, and all three outcomes looked like
something else:

  * as a LOOP or ARRAY index, it matched nothing. The Files list ignored
    nineteen clicks in twenty; the calculator pressed '+' for every key; the
    kernel log highlighted no row ever.
  * as a SELECTION, it fell through to the default branch. The network pane
    reported eth0's driver, address and counters under the loopback's name.
  * as a GUARD, it silently never held. foot_tick damaged the whole foot band
    sixty times a second instead of once.

None of those reads as "wrong operator". They read as a dead control, a wrong
label, and a performance problem.

WHAT IS FLAGGED, AND WHY IT IS DELIBERATELY NARROW.

There are over two hundred bare `/` in these files and most are harmless -
centring offsets, halves of a width, anything whose result is a pixel that C
truncates on the way into a fill. Flagging all of them would produce a guard
nobody reads, which is worse than no guard.

So this flags an assignment `NAME = ... / ...` only where NAME looks like an
index or a count: row, col, cell, idx, slot, sel, line, page, fit, top, cols,
rows, n, i, r, c. That is the population the nineteen came from.

TO SILENCE ONE, use idiv() - which is what the fix always is. If a genuinely
fractional value is wanted under one of these names, put `# float ok` on the
line and say why; the guard honours it and the next reader gets the reason.

RUN IT: python3 kernel/tools/checks/check-intdiv.py
Its self-test is check-intdiv-selftest.sh, which plants four defects and one
legitimate float and requires the guard to catch exactly the four.
"""
import re
import sys
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]

# The names whose values are indices, counts or loop bounds. A fraction in any
# of these is a defect; a fraction in `cx`, `pad` or `half` usually is not.
# `sec` and `seen` are here because the once-a-second guards are the same
# class: `fsec = ticks() / 100` never held, so foot_tick damaged the whole band
# every frame instead of once a second. `cell` is deliberately NOT here - in
# this tree `cell` almost always holds a SIZE, and flagging every one of those
# would be the noise that gets a guard switched off.
# MATCHED AS A SUFFIX, SINGLE LETTERS INCLUDED, AND THE TRADE IS DELIBERATE.
#
# An intermediate version required single letters to be the whole name or to
# follow an underscore, so that `browser_win` would not match on the "n". That
# removed one false positive and lost THREE real findings with it - `edr` (a
# row count), `sb_shown` (a shown-row count) and `pi` (Nim's pile index), none
# of which has an underscore before its last letter.
#
# A false positive costs one `# float ok` comment and a sentence saying why. A
# false negative costs a dead control that looks like a design decision. So the
# match is broad and the escape hatch is the pressure valve.
#
# `cell` is still excluded: in this tree it almost always holds a SIZE, and that
# one WOULD be the noise that gets a guard switched off.
INDEXY = (
    r"(?:row|col|idx|index|slot|sel|line|page|fit|top|first|"
    r"rows|cols|count|n|i|j|k|r|c|cut|hit|want|step|band|sec|seen)"
)

# NAME = <anything> / <anything>
#   - NAME ends with an index-ish word (files_mrow, kl_fit, tw_i, cc, crow...)
#   - the divide is a real one: not `//`, not `/=`, not inside a string
ASSIGN = re.compile(
    r"^\s*(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*=\s*(?P<rhs>[^#\n]*?/[^#\n/=]*)$"
)
# Matched as a SUFFIX with no underscore required: the first version demanded
# `(?:^|_)`, so `files_mcol` - which is exactly the shape of the Files-list bug
# this guard was written for - slipped straight past it. Its own self-test
# caught that, which is the whole reason the self-test plants that name.
NAME_OK = re.compile(rf"{INDEXY}[0-9_]*$", re.IGNORECASE)


def strip_strings(s):
    """Remove "..." so a slash inside a literal is not a divide."""
    return re.sub(r'"[^"]*"', '""', s)


def scan(path):
    out = []
    for lineno, raw in enumerate(path.read_text().splitlines(), 1):
        if "# float ok" in raw:
            continue
        line = strip_strings(raw)
        if "//" in line:
            continue
        m = ASSIGN.match(line)
        if not m:
            continue
        name = m.group("name")
        if not NAME_OK.search(name):
            continue
        rhs = m.group("rhs")
        # idiv() on the right-hand side is the fix; a `/` elsewhere on the same
        # line (inside the idiv call's own arguments) is not a finding.
        without_idiv = re.sub(r"idiv\s*\([^()]*(?:\([^()]*\)[^()]*)*\)", "", rhs)
        if "/" not in without_idiv:
            continue
        out.append((lineno, name, raw.strip()))
    return out


def main():
    # ROOT is kernel/, matching check-header-mirror.py beside it - the sources
    # are src/kernel.zl and apps/*.zl relative to that, not to the repo root.
    targets = [ROOT / "src/kernel.zl"] + sorted((ROOT / "apps").glob("*.zl"))
    targets = [t for t in targets if t.exists()]
    if not targets:
        print("check-intdiv: no .zl sources found - is the tree laid out as expected?")
        return 1
    findings = []
    for t in targets:
        for lineno, name, text in scan(t):
            findings.append((t.relative_to(ROOT), lineno, name, text))
    if not findings:
        print(f"check-intdiv: OK - no float division into an index across "
              f"{len(targets)} zl source(s)")
        return 0
    print("check-intdiv: FAIL - `/` is float division in zl; these assign a "
          "fraction to an index-ish name")
    for path, lineno, name, text in findings:
        print(f"  {path}:{lineno}  {name}")
        print(f"      {text}")
    print()
    print("  Use idiv(a, b). If the fraction is deliberate, add `# float ok` "
          "and say why.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
