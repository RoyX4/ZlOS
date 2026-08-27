#!/usr/bin/env python3
"""check-appids.py - no two apps may share an id. Half a second, no QEMU.

WHY THIS EXISTS. apps_sys2.zl, apps_sys3.zl, apps_games3.zl and apps_games4.zl
have each carried this sentence in their header since they were written:

    a duplicate app id compiles clean and misroutes at RUNTIME

...and nobody wrote the check. It then happened, in the worst possible place:
apps_registry.zl gave the app catalog id 13, kernel.zl later gave APP_FILES the
same 13, and reg_open(APP_CATALOG) found the Files window the desktop opens at
boot, raised it, and returned success. The catalog - the only route to 47
registry apps - could not be opened by anyone, by any route, and there was no
error anywhere because nothing had failed. It also silently routed every event
aimed at the Files window into the catalog's event handler.

Two apps sharing an id is not a naming problem, it is a routing problem: wm.c
stores ONE integer per window and every dispatcher in the tree answers "which
app is this" with it.

WHAT COUNTS AS AN APP ID, precisely, because a loose scan is a guard that cries
wolf until somebody deletes it:

  1. every `APP_<NAME> = <int>` at the top level of any kernel .zl file. These
     are the original suite plus the registry's own.
  2. every id named by a `fn <slice>_name(id)` dispatch arm, resolved through
     the constants defined in that same file. This is how the game slices spell
     theirs (G3_TETRIS, G4_REVERSI), and it is deliberately NOT a `G3_*` prefix
     scan: G3_BAR is a row height of 30 and G3_TETB is a buffer address, and
     both would collide with something under a prefix rule.

Run it before committing anything that adds an app.
"""
import glob
import os
import re
import sys

KERNEL_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

CONST = re.compile(r"^([A-Z][A-Z0-9_]*)\s*=\s*(\d+)\s*(?:#.*)?$", re.M)
APP_CONST = re.compile(r"^(APP_[A-Z0-9_]+)\s*=\s*(\d+)\s*(?:#.*)?$", re.M)
NAME_FN = re.compile(r"fn (\w*_?name)\(id\)\s*\{(.*?)\n\}", re.S)
NAME_ARM = re.compile(r'if id == (\w+)\s*\{\s*return "([^"]+)"')


def collect(sources):
    """{id: {constant name: {file, ...}}} over the whole corpus.

    Keyed by NAME, not by name-and-file: zl has one flat namespace and a
    constant is routinely written down twice with the same value (APP_SETTINGS
    is declared in both kernel.zl and apps_registry.zl, deliberately, because
    each side opens one). That is one app, so it must not read as a collision -
    a guard that cries wolf on a legal pattern is a guard someone deletes.
    """
    ids = {}

    def note(const, value, where):
        ids.setdefault(value, {}).setdefault(const, set()).add(where)

    for path, src in sources.items():
        where = os.path.basename(path)
        for m in APP_CONST.finditer(src):
            note(m.group(1), int(m.group(2)), where)
        # A slice spells its ids as its own constants; resolve against the
        # constants in the SAME file, which is the scope zl's flat namespace
        # gives them anyway.
        local = {m.group(1): int(m.group(2)) for m in CONST.finditer(src)}
        for fn in NAME_FN.finditer(src):
            for arm in NAME_ARM.finditer(fn.group(2)):
                tok = arm.group(1)
                val = int(tok) if tok.isdigit() else local.get(tok)
                if val is not None:
                    note(tok, val, where)
    return ids


def duplicates(ids):
    out = {}
    for value, names in sorted(ids.items()):
        if len(names) > 1:
            out[value] = sorted(f"{n} ({', '.join(sorted(w))})"
                                for n, w in names.items())
    return out


def read_sources():
    paths = [os.path.join(KERNEL_ROOT, "src", "kernel.zl")]
    paths += sorted(glob.glob(os.path.join(KERNEL_ROOT, "apps", "apps_*.zl")))
    return {p: open(p, encoding="utf-8", errors="replace").read() for p in paths}


def report(dups, label):
    if not dups:
        print(f"{label}: no duplicate app ids")
        return 0
    print(f"{label}: DUPLICATE APP IDS", file=sys.stderr)
    for value, names in dups.items():
        print(f"  id {value} is claimed by {len(names)}: " + ", ".join(names),
              file=sys.stderr)
    print("  A window carries ONE app id. Two apps sharing it means every\n"
          "  dispatcher - app_draw, app_event, app_tick, task_label,\n"
          "  reg_find_win - answers for whichever arm it tests first, and the\n"
          "  other app is unreachable with no error anywhere.", file=sys.stderr)
    return 1


def main(argv):
    sources = read_sources()

    if "--selftest" in argv:
        # THE GUARD MUST CATCH THE BUG IT WAS WRITTEN FOR. Put APP_CATALOG back
        # on 13 - the real defect, verbatim - and require a failure. A check
        # that has never been shown to go red is a check nobody can trust; see
        # docs/GUARDS-THAT-DID-NOT-GUARD.md.
        reg = os.path.join(KERNEL_ROOT, "apps", "apps_registry.zl")
        broken = dict(sources)
        broken[reg] = re.sub(r"^APP_CATALOG(\s*)=\s*\d+", r"APP_CATALOG\g<1>= 13",
                             broken[reg], count=1, flags=re.M)
        if broken[reg] == sources[reg]:
            print("selftest could not plant the defect - APP_CATALOG's "
                  "assignment no longer matches", file=sys.stderr)
            return 2
        dups = duplicates(collect(broken))
        if 13 not in dups:
            print("SELFTEST FAILED: APP_CATALOG = 13 collides with APP_FILES = "
                  "13 and this check did not see it", file=sys.stderr)
            return 2
        print("selftest: planted APP_CATALOG = 13, caught as "
              + ", ".join(dups[13]))

    return report(duplicates(collect(sources)), "check-appids")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
