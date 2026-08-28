#!/usr/bin/env python3
"""Every version string the user can see must be the same version string.

WHY. "0.3" is written out six times in kernel.zl - the rail, the boot banner,
the About window, the system pane, a registry slot - and settings.c had a
SEVENTH that said "zl 0.1". Nothing compared them, so the About pane sat two
releases behind the rail eighteen pixels to its left for as long as it took
someone to look at the two at once.

The architecture was the same shape but worse: settings.c stated "i386 - ring
0" as a constant, in a file compiled into BOTH the 32-bit and the 64-bit
kernel. The build that boots the laptop is the 64-bit one.

This is not a style check. A version the software reports about itself is a
fact, and seven copies of a fact is six chances to be wrong about it.

The single source is kernel/src/core/version.h. C reads it directly; kernel.zl
cannot include a C header, so its literals are checked against it here.
"""
import re
import sys
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
VERSION_H = ROOT / "src/core/version.h"
KERNEL_ZL = ROOT / "src/kernel.zl"

def strip_comment(line):
    """Everything before an unquoted '#'.

    The first version tested line.lstrip().startswith("#"), which drops
    WHOLE-LINE comments and keeps trailing ones - so a version mentioned in a
    note beside code ("was 0.1 before the rename") would have been read as a
    live claim and failed the build. Splitting on a bare '#' is wrong the other
    way round, because a zl string literal may contain one.
    """
    out, in_str = [], False
    for ch in line:
        if ch == '"':
            in_str = not in_str
        elif ch == "#" and not in_str:
            break
        out.append(ch)
    return "".join(out)


def main() -> int:
    text = VERSION_H.read_text()
    m = re.search(r'#define\s+ZLOS_VERSION_STR\s+"([^"]+)"', text)
    if not m:
        print("check-version: FAIL - ZLOS_VERSION_STR not found in version.h")
        return 1
    version = m.group(1)
    print(f"check-version: version.h declares {version}")

    src = KERNEL_ZL.read_text()
    bad = []
    checked = 0
    for lineno, line in enumerate(src.splitlines(), 1):
        code = strip_comment(line)
        for lit in re.findall(r'"([^"]*)"', code):
            found = re.findall(r'(\d+\.\d+)', lit)
            if not found:
                continue
            # WHICH DOTTED LITERALS ARE OURS - two rules, and the second exists
            # because the first missed a real one.
            #
            #   a) it NAMES the system: "zlOS 0.3", "0.3 x86_64",
            #      "zlos 0.3, x86_64, single image, ".
            #   b) it is a BARE version and nothing else: `return "0.3"` in the
            #      rail_sub registry slot table. Rule (a) skipped that
            #      one completely, so the guard would have sat green through a
            #      drift in a slot table - which is exactly the kind of place a
            #      stale copy of a version goes to hide.
            #
            # (b) is safe here, MEASURED rather than assumed: the only other
            # bare dotted literals in this file are "6.796", "6.47" and "0.00",
            # and all three are inside comments, which strip_comment removes.
            # If a real non-version bare decimal is ever added to code this
            # fails loudly instead of silently, which is the right direction.
            names_us = bool(re.search(r'zl', lit, re.I)) or "x86" in lit
            bare = re.fullmatch(r'\s*\d+\.\d+\s*', lit) is not None
            if not (names_us or bare):
                continue
            checked += 1
            for f in found:
                if f != version:
                    bad.append((lineno, lit, f))

    if bad:
        print(f"check-version: FAIL - {len(bad)} literal(s) disagree with version.h")
        for lineno, lit, f in bad:
            print(f"  kernel.zl:{lineno}  {f!r} in {lit!r}  (version.h says {version!r})")
        return 1

    print(f"check-version: PASS - {checked} version literal(s) in kernel.zl, all {version}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
