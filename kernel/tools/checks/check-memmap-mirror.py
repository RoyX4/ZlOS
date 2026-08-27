#!/usr/bin/env python3
"""Constants in kernel.zl that mirror memmap.h must still equal it.

THE HAZARD THIS CLOSES was recorded, in writing, and left unenforced:

    RULER_DMA / RULER_DMA_END in kernel.zl restate HI_IMG / HI_HEAP from
    memmap.h. Verified equal today, enforced by nothing - zl cannot include a C
    header. That is the same drift class as the window-manager reserves that
    said 48/72 while the shell said 30/46/170, which is exactly how that bug
    survived.
        - docs/evidence/presswork-first-boot.md

The reserves in that comparison went on to be found in ELEVEN places, none of
them agreeing (docs/evidence/one-fact-many-copies-2026-08-27.md). A hazard
someone wrote down and nobody wired up is a guess with a citation.

THE COMMENT IS THE DECLARATION. zl cannot include a C header, so the mirror can
only ever be a convention - and the convention already exists in the source:

    RULER_DMA     = 0x03000000         # memmap.h HI_IMG  - the first driver region

That comment says "this is HI_IMG". This reads it as a promise and checks it.
Anything written that way in future is covered the day it is written; nothing
has to be added here.

IT ALSO REPORTS UNDECLARED COINCIDENCES rather than failing on them. A kernel.zl
constant that happens to equal a memmap.h value might be a mirror nobody
documented, or might be deliberate adjacency - PAINT_BUF sits exactly at
ZL_LOW_END because it is placed immediately after that block, which is a
relationship, not a copy. Failing on those would be crying wolf; staying silent
about them would hide the next real one. So they are listed, and the check still
passes.
"""
import re
import sys
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
KERNEL_ZL = ROOT / "src/kernel.zl"
MEMMAP_H = ROOT / "src/arch/x86/memmap.h"
FLOOR = 0x01000000          # below this, small integers collide by chance


def memmap_defines(text):
    return {m.group(1): int(m.group(2), 16)
            for m in re.finditer(r'#define\s+(\w+)\s+(0x[0-9a-fA-F]+)UL?\b', text)}


def main() -> int:
    if not KERNEL_ZL.exists() or not MEMMAP_H.exists():
        print("check-memmap-mirror: FAIL - kernel.zl or memmap.h is missing")
        return 1
    defines = memmap_defines(MEMMAP_H.read_text())
    by_value = {}
    for name, val in defines.items():
        by_value.setdefault(val, []).append(name)

    bad, checked, coincidences = [], 0, []
    for lineno, line in enumerate(KERNEL_ZL.read_text().splitlines(), 1):
        m = re.match(r'\s*([A-Z_][A-Z0-9_]*)\s*=\s*(0x[0-9a-fA-F]+)\b(.*)', line)
        if not m:
            continue
        name, val, rest = m.group(1), int(m.group(2), 16), m.group(3)
        cite = re.search(r'#.*\bmemmap\.h\s+([A-Z_][A-Z0-9_]*)', rest)
        if cite:
            sym = cite.group(1)
            checked += 1
            if sym not in defines:
                bad.append((lineno, name, hex(val), sym,
                            "memmap.h has no such symbol"))
            elif defines[sym] != val:
                bad.append((lineno, name, hex(val), sym,
                            f"memmap.h says {hex(defines[sym])}"))
        elif val >= FLOOR and val in by_value:
            coincidences.append((lineno, name, hex(val), by_value[val]))

    print(f"check-memmap-mirror: {len(defines)} memmap.h symbols, "
          f"{checked} declared mirror(s) in kernel.zl")
    if bad:
        print(f"check-memmap-mirror: FAIL - {len(bad)} mirror(s) have drifted")
        for lineno, name, val, sym, why in bad:
            print(f"  kernel.zl:{lineno}  {name} = {val} claims to be {sym}; {why}")
        return 1
    for lineno, name, val, syms in coincidences:
        print(f"  note   kernel.zl:{lineno}  {name} = {val} equals "
              f"{', '.join(syms)} but does not say so "
              f"(adjacency or an undeclared mirror - say which)")
    print("check-memmap-mirror: PASS - every declared mirror still matches")
    return 0


if __name__ == "__main__":
    sys.exit(main())
