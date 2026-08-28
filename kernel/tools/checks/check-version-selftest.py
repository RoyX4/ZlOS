#!/usr/bin/env python3
"""Prove check-version.py can still FAIL, and still refuses to false-positive.

WHY A SELFTEST FOR A GUARD. docs/GUARDS-THAT-DID-NOT-GUARD.md is a list of five
checks in this tree that reported green while checking nothing. A guard is only
worth its runtime if something establishes that it can go red, and "I ran it
against a planted defect once, by hand, before I committed it" is not that -
nobody re-runs it, and the guard rots exactly like the thing it was watching.

THE FIRST VERSION OF check-version.py FAILED CASE B. It only examined literals
that NAMED the system - "zlOS 0.3", "0.3 x86_64" - which skipped rail_sub's
bare `if slot == 9 { return "0.3" }` entirely. A registry slot table is close to
the ideal hiding place for a stale version: it is a column of unrelated short
strings, and nothing about "0.3" sitting between "wire" and "design.h" looks
like a claim about the system. The guard was written, run, seen to pass, and was
blind to one of the six literals it existed to check.

Cases C and D exist because the fix for B - "a bare dotted literal is a version"
- widens what gets inspected, and widening is how a guard starts crying wolf.

Runs against the REAL module with synthetic inputs; it never modifies the tree,
so it is safe to run while a boot gate has the checkout.
"""
import importlib.util
import pathlib
import sys
import tempfile

HERE = pathlib.Path(__file__).resolve().parent
GUARD = HERE / "check-version.py"
KERNEL_ZL = HERE.parents[1] / "src/kernel.zl"

SLOT = 'if slot == 9 { return "0.3" }'


def load():
    spec = importlib.util.spec_from_file_location("check_version", GUARD)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def main() -> int:
    if not GUARD.exists():
        print("check-version-selftest: FAIL - check-version.py is missing")
        return 1
    cv = load()
    real = KERNEL_ZL.read_text()
    if SLOT not in real:
        print(f"check-version-selftest: FAIL - kernel.zl no longer contains "
              f"{SLOT!r}; cases B/C are anchored to it and must be re-pointed")
        return 1

    tmpdir = pathlib.Path(tempfile.mkdtemp())
    probe = tmpdir / "k.zl"

    def run(text):
        probe.write_text(text)
        cv.KERNEL_ZL = probe
        try:
            return cv.main()
        finally:
            cv.KERNEL_ZL = KERNEL_ZL

    cases = [
        ("A  the real tree passes",
         real, 0),
        ("B  drift in the BARE slot-table literal is caught",
         real.replace(SLOT, 'if slot == 9 { return "0.5" }', 1), 1),
        ("C  a version named in a trailing comment is NOT a claim",
         real.replace(SLOT, SLOT + '   # was "0.1" before the rename', 1), 0),
        ("D  a '#' inside a string does not truncate the line",
         real.replace('if slot == 10 { return "design.h" }',
                      'if slot == 10 { return "design.h # tokens" }', 1), 0),
    ]

    fails = 0
    out = []
    for label, text, want in cases:
        # the guard prints; keep the selftest's own output readable
        import io
        import contextlib
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            got = run(text)
        mark = "ok  " if got == want else "FAIL"
        if got != want:
            fails += 1
        out.append(f"  {mark} {label}   (exit {got}, wanted {want})")

    print("\n".join(out))
    if fails:
        print(f"check-version-selftest: FAILED ({fails} case(s))")
        return 1
    print("check-version-selftest: PASS - the guard can fail, and does not cry wolf")
    return 0


if __name__ == "__main__":
    sys.exit(main())
