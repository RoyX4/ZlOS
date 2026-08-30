#!/usr/bin/env python3
"""check-dead-state.py - module-level state that nothing reads.

WHY THIS EXISTS. A one-off search over kernel.zl and the app modules found
FIFTEEN globals and three constants whose only mention in the entire tree was
their own declaration:

    s_len s_dx s_dy s_fx s_fy s_score s_alive s_seed s_gw s_gh s_next s_died
    SNAKE_CELL SNAKE_RATE SNAKE_MAX          the predecessor Snake
    mon_open ab_open focus_win               the pre-compositor window model
    wm_live                                  a guard nothing ever asked

None of that was harmless. The Snake block sat under a paragraph describing
snake_draw / snake_key / snake_step, which read as documentation for the game
that ships - and the game that ships is sn_draw / sn_event on a complete
parallel set of names. `focus_win` shadowed a C static of the SAME NAME in wm.c
meaning a slot index, and a comment at kernel.zl:11236 cites that static. And
`wm_live` sat directly beneath a paragraph describing a real re-entrancy
hazard, which reads as "this variable is the guard" - it is not, the guard is
wm_run(), and telling those two apart took reading wm_session.

Dead state is not clutter. It is a comment that describes a system nobody is
running, attached to a name the next reader will assume is load-bearing.

THE SEARCH WAS A COMMAND SOMEBODY TYPED ONCE, which is the shape this repo's
GUARDS-THAT-DID-NOT-GUARD.md is about. This is that search, kept.

WHAT IT COUNTS, AND WHY THAT WAY.

Declarations come from column 0 of the zl sources - that is what module scope
looks like in this subset. Mentions are counted over zl CODE WITH COMMENTS
STRIPPED, because otherwise the very comment that records a deletion would keep
the deleted name looking alive; every removal above left such a comment behind.

A name is also counted across the C, header, Python and shell sources, without
stripping their comments. That direction is deliberately lenient: zl globals
ARE read from C (SNAKE_X is poked by the live game and read by
check-memmap.sh), and a false accusation costs more here than a miss. So the
rule is strict about zl and generous about everything else:

    DEAD  =  exactly one mention in zl code, and zero anywhere else.

Exactly one, not zero: the declaration itself is the one.

Run:  python3 tools/checks/check-dead-state.py
      python3 tools/checks/check-dead-state.py --list   (every count, ranked)
"""
import os
import re
import sys
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__))
KERNEL = os.path.abspath(os.path.join(HERE, "..", ".."))

ZL_SOURCES = [os.path.join(KERNEL, "src", "kernel.zl")]
APPS = os.path.join(KERNEL, "apps")
if os.path.isdir(APPS):
    ZL_SOURCES += sorted(os.path.join(APPS, f) for f in os.listdir(APPS)
                         if f.endswith(".zl"))

OTHER_EXT = (".c", ".h", ".py", ".sh")

# A declaration at module scope: a bare `name = value` starting at column 0.
# `==` is excluded so a top-level comparison can never be read as one.
DECL = re.compile(r"(?m)^([A-Za-z_][A-Za-z0-9_]*)\s*=(?!=)")


def strip_zl_comments(text):
    """Drop everything after an unquoted '#'. zl has no block comments and no
    escape inside its string literals worth modelling here, so tracking whether
    we are inside a double-quoted run is enough."""
    out = []
    for line in text.split("\n"):
        instr = False
        cut = len(line)
        for i, ch in enumerate(line):
            if ch == '"':
                instr = not instr
            elif ch == "#" and not instr:
                cut = i
                break
        out.append(line[:cut])
    return "\n".join(out)


# THIS FILE AND ITS BASELINE DO NOT COUNT AS READERS.
#
# The first run with a baseline reported FILES_BUF and C_BLACK as "alive
# again" - because the paragraph above naming them as examples is inside a .py
# file under kernel/, and gather_other() reads every .py under kernel/. A check
# that can be defeated by writing the name in its own source is the shape
# docs/GUARDS-THAT-DID-NOT-GUARD.md is about.
#
# ONLY these two are excluded. A checker mentioning a constant is normally a
# REAL reader - check-memmap.sh reads SNAKE_X and that is exactly why SNAKE_X
# is not dead - so the exclusion is by exact path, not by directory.
SELF = os.path.abspath(__file__)
BASELINE = os.path.join(HERE, "dead-state-baseline.txt")


def gather_other():
    blob = []
    for root, dirs, files in os.walk(KERNEL):
        dirs[:] = [d for d in dirs if d not in (".git", "__pycache__", "build")]
        for f in files:
            if not f.endswith(OTHER_EXT):
                continue
            full = os.path.abspath(os.path.join(root, f))
            if full in (SELF, BASELINE):
                continue
            try:
                blob.append(open(full, errors="replace").read())
            except OSError:
                pass
    return "\n".join(blob)


def main():
    want_list = "--list" in sys.argv

    zl_raw = {}
    for p in ZL_SOURCES:
        try:
            zl_raw[p] = open(p, errors="replace").read()
        except OSError:
            print("FAIL: cannot read %s" % p)
            return 1
    if not zl_raw:
        print("FAIL: no zl sources found under %s" % KERNEL)
        return 1

    zl_code = {p: strip_zl_comments(s) for p, s in zl_raw.items()}
    zl_all = "\n".join(zl_code.values())
    other = gather_other()

    # declaration -> the file it is declared in, first wins
    decls = {}
    for p, code in zl_code.items():
        for m in DECL.finditer(code):
            decls.setdefault(m.group(1), p)

    # ONE PASS OVER EACH CORPUS, NOT ONE PER NAME. The first version compiled a
    # \bname\b regex for every declaration and searched a multi-megabyte blob
    # with it - several hundred names times several megabytes, and it did not
    # finish inside two minutes. Tokenising once and counting is the same
    # answer in a second, because \b...\b over an identifier is exactly "this
    # token appears".
    IDENT = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
    zl_count = Counter(IDENT.findall(zl_all))
    other_count = Counter(IDENT.findall(other))

    rows = []
    for name, where in decls.items():
        n_zl = zl_count.get(name, 0)
        n_other = other_count.get(name, 0)
        rows.append((n_zl + n_other, n_zl, n_other, name, where))
    rows.sort()

    if want_list:
        for total, n_zl, n_other, name, where in rows[:60]:
            print("  %-24s %3d zl  %3d other   %s"
                  % (name, n_zl, n_other, os.path.basename(where)))
        print("\n  %d module-level names" % len(rows))
        return 0

    dead = [(name, where) for total, n_zl, n_other, name, where in rows
            if n_zl == 1 and n_other == 0]

    # THE BASELINE, AND WHY THERE IS ONE.
    #
    # This check went green on nothing: its first run found 28 names, and a
    # gate that always fails is a gate nobody runs. Deleting 28 things in one
    # sweep is also wrong - several are members of coherent families where the
    # unused member is arguable (C_BLACK beside four colours that ARE used),
    # and one, FILES_BUF, is a memmap-registered PAGE whose absence of a reader
    # is itself a finding rather than a tidy-up.
    #
    # So the shape is check-memmap.sh's, which says of the constants it cannot
    # size: "Not a failure: a new address is not automatically wrong, it is
    # automatically unexamined." The known 28 are listed and reported; anything
    # NOT in that list fails, because a newly dead name is a name that died
    # today and someone can still remember why.
    base_path = os.path.join(HERE, "dead-state-baseline.txt")
    baseline = set()
    if os.path.exists(base_path):
        for l in open(base_path):
            l = l.split("#", 1)[0].strip()
            if l:
                baseline.add(l)

    def at(name, where):
        for i, l in enumerate(zl_code[where].split("\n"), 1):
            if re.match(r"^%s\s*=(?!=)" % re.escape(name), l):
                return i
        return 0

    fresh = [(n, w) for n, w in dead if n not in baseline]
    known = [(n, w) for n, w in dead if n in baseline]
    # A BASELINE ENTRY GOES STALE TWO WAYS, and they are not the same news.
    # The name was DELETED - the good outcome, and what happened to the six
    # KL_* constants when the kernel log's dead island went - or it gained a
    # READER, which is also good but means somebody wired it up. Saying "alive
    # again" about a deleted name sends the reader looking for a caller that
    # does not exist.
    gone = sorted(baseline - {n for n, w in dead})
    gone_deleted = [n for n in gone if n not in decls]
    gone_wired = [n for n in gone if n in decls]

    for name, where in known:
        print("  known-dead  %s:%d  %s"
              % (os.path.relpath(where, KERNEL), at(name, where), name))
    if gone_deleted:
        print("\n  BASELINE IS STALE - these no longer exist at all. Deleted, "
              "which is the point.\n  Drop them from dead-state-baseline.txt:")
        for n in gone_deleted:
            print("    %s" % n)
    if gone_wired:
        print("\n  BASELINE IS STALE - these are declared AND read now, so "
              "somebody wired them up.\n  Drop them from "
              "dead-state-baseline.txt:")
        for n in gone_wired:
            print("    %s" % n)

    if fresh:
        print("\ndead-state: FAIL - %d name(s) died since the baseline" % len(fresh))
        for name, where in fresh:
            print("  %s:%d  %s"
                  % (os.path.relpath(where, KERNEL), at(name, where), name))
        print("\n  Delete them, or say out loud what reads them. A name that "
              "nothing reads is\n  a comment describing a system nobody is "
              "running - which is exactly how\n  kernel.zl kept a paragraph "
              "documenting a Snake that had been replaced.")
        return 1

    # A stale baseline is a real defect in the check itself, and is fatal for
    # the same reason a stale golden.txt would be: it means this file is
    # describing a tree that no longer exists.
    if gone:
        print("\ndead-state: FAIL - the baseline names %d thing(s) that are "
              "no longer dead (%d deleted, %d wired up)"
              % (len(gone), len(gone_deleted), len(gone_wired)))
        return 1

    print("\ndead-state: PASS - %d module-level names, %d known dead and "
          "listed, 0 new" % (len(rows), len(known)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
