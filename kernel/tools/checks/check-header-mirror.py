#!/usr/bin/env python3
"""Any kernel.zl constant that cites a C header must still equal it.

check-memmap-mirror.py closes this hazard for ONE header. This closes it for
every header, because the hazard was never about memmap.h - it is about zl
being unable to include a C header at all, so that every C constant zl needs
exists twice, and the second copy is maintained by hope.

WHAT IT COST, MEASURED. desk_key in kernel.zl handled Escape and F1. It compared
Escape against 27 and F1 against 0x13B. This kernel's keycodes.h says:

    KEY_ESC   0x101
    KEY_F1    0x120
    KEY_SUPER 0x11A

27 is the serial byte, not the keycode. 0x13B is 0x100|0x3B - the LINUX keycode
for F1, from a scheme nothing in this tree emits. Of the three keys desk_key
bound, the only constant that was right was KEY_SUPER - and Super was the only
one of the three that was ever routed to desk_key at all. A key nobody can press
cannot be observed to hold the wrong number, so the two bugs kept each other
invisible while the overlay printed ESC TO DISMISS on screen.

wm.c had already hit the identical bug from the C side. Its comment:

    whoever wrote the Alt+Tab test had no KEY_TAB in scope because nobody had
    copied that line in, reached for '\\t' instead, and Alt+Tab has never fired.
    ... A partial copy of a table cannot carry a rule.

Its fix was to delete the copies and #include the header. That move does not
exist on the zl side. This check is the substitute: a copy a tool can compare
beats a copy only a reader can.

THE COMMENT IS THE DECLARATION, the same convention check-memmap-mirror.py
established:

    KEY_ESC_K  = 0x101      # keycodes.h KEY_ESC
    SPARK_MAX  = 64         # design.h ZD_SPARK_MAX

Anything written that way is covered the day it is written. Decimal and hex both
count, because design.h's tokens are decimal and a hex-only reader would have
silently skipped every one of them.
"""
import re
import sys
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
KERNEL_ZL = ROOT / "src/kernel.zl"
SRC = ROOT / "src"


def parse_int(tok):
    try:
        return int(tok, 16) if tok.lower().startswith("0x") else int(tok, 10)
    except ValueError:
        return None


def defines_in(path):
    """#define NAME <int literal>. Expressions are skipped deliberately - a
    mirror of a computed value is a different and worse idea than a mirror of a
    literal, and silently 'resolving' one would invent a number."""
    out = {}
    for m in re.finditer(r'#define\s+(\w+)\s+(0[xX][0-9a-fA-F]+|\d+)\s*(?:UL?|L)?\s*(?:/\*|//|$)',
                         path.read_text(errors="replace"), re.M):
        v = parse_int(m.group(2))
        if v is not None:
            out[m.group(1)] = v
    return out


def find_header(name):
    hits = sorted(SRC.rglob(name))
    return hits[0] if hits else None


def main() -> int:
    if not KERNEL_ZL.exists():
        print("check-header-mirror: FAIL - kernel.zl is missing")
        return 1

    cache, bad, checked = {}, [], 0
    for lineno, line in enumerate(KERNEL_ZL.read_text().splitlines(), 1):
        m = re.match(r'\s*([A-Z_][A-Z0-9_]*)\s*=\s*(0[xX][0-9a-fA-F]+|\d+)\b(.*)', line)
        if not m:
            continue
        # .c AS WELL AS .h, AND THIS WAS A REAL MISS. Six FS_WHY_* mirrors were
        # added citing "fs.c FS_WHY_DAMAGED" - fs.c holds those #defines because
        # there is no fs.h - and this pattern skipped every one of them while
        # still reporting OK. A checker that silently matches nothing passes
        # exactly as loudly as one that matches everything, which is the failure
        # this file's own selftest case D exists to catch. It caught it here
        # only because the mirror count did not move.
        cite = re.search(r'#.*?\b([A-Za-z0-9_]+\.[hc])\s+([A-Z_][A-Z0-9_]*)', m.group(3))
        if not cite:
            continue
        name, val = m.group(1), parse_int(m.group(2))
        header, sym = cite.group(1), cite.group(2)
        checked += 1

        if header not in cache:
            p = find_header(header)
            cache[header] = defines_in(p) if p else None
        table = cache[header]

        if table is None:
            bad.append((lineno, name, val, header, sym, "no such header under src/"))
        elif sym not in table:
            bad.append((lineno, name, val, header, sym, "header has no such symbol"))
        elif table[sym] != val:
            bad.append((lineno, name, val, header, sym,
                        "header says %s (0x%X)" % (table[sym], table[sym])))

    if bad:
        print("check-header-mirror: FAIL - %d mirrored constant(s) disagree" % len(bad))
        for lineno, name, val, header, sym, why in bad:
            print("  kernel.zl:%d  %s = %s  cites %s %s - %s"
                  % (lineno, name, val, header, sym, why))
        return 1

    if checked == 0:
        print("check-header-mirror: FAIL - no citations found at all.")
        print("  kernel.zl mirrors C constants and always has. Zero matches means")
        print("  the convention was renamed or this pattern broke, not that the")
        print("  mirrors went away - a checker that checks nothing passes loudly.")
        return 1

    print("check-header-mirror: OK - %d mirrored constant(s) match their header" % checked)
    return 0


if __name__ == "__main__":
    sys.exit(main())
