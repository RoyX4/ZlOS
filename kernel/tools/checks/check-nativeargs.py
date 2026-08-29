#!/usr/bin/env python3
"""check-nativeargs.py - a zl call must supply every slot its native reads.

THE DEFECT THIS EXISTS FOR, and it is verified rather than reasoned about.

freestanding/runtime_kernel.c dispatches every builtin through one function that
collects arguments into a fixed array:

    Value a[8] = { 0 };
    for (i = 0; i < n && i < 8; i++) a[i] = __builtin_va_arg(ap, Value);

Each builtin then reads a FIXED number of slots. `br_click` reads a[0], a[1] and
a[2]. Nothing checks that the caller supplied them, and `kernel.zl` called
`br_click(ex, ey)` with two - so the third slot was read anyway.

That array used to be a bare uninitialised local, which made the missing
argument STACK GARBAGE: a browser click was decided by whatever the call path
had left behind. It is `= { 0 }` now, so the value is at least determinate - but
a determinate wrong answer is still a wrong answer, and nothing else in the tree
notices the mismatch. zl has no signatures to check against.

WHAT IT DOES

  * Reads runtime_kernel.c and, for each `streq(name, "X")` dispatch line,
    records the highest a[N] that line reads. That is X's arity.
  * Reads kernel/src/kernel.zl and kernel/apps/*.zl, finds every call to a name
    it has an arity for, and counts the arguments actually passed - respecting
    nesting, so `idiv(a - b, c)` is two and `f(g(x, y), z)` is two.
  * Reports any call site passing FEWER than the native reads.

Passing MORE is not reported: the extra is collected into a slot nobody reads,
which is wasteful and harmless, and flagging it would bury the real thing.

WHAT IT CANNOT SEE, stated so nobody trusts it further than it goes:

  * A native whose dispatch line spans more than one physical line - the scan is
    per-line. Such a native simply gets no arity and no call of it is checked.
  * A call built across a line break. zl is one-call-per-line by convention, so
    this is rare, but it is a hole and not a guarantee.
  * Whether the arguments are the RIGHT ones. Only that there are enough.

RUN IT: python3 kernel/tools/checks/check-nativeargs.py
Self-test: check-nativeargs-selftest.sh plants four defects and two legitimate
calls, and requires exactly the four to be caught.
"""
import re
import sys
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
RUNTIME = ROOT.parent / "freestanding" / "runtime_kernel.c"

DISPATCH = re.compile(r'streq\(name,\s*"([A-Za-z_][A-Za-z0-9_]*)"\)')
SLOT = re.compile(r"\ba\[(\d+)\]")


def native_arity(runtime_text):
    """{name: highest slot index read + 1} from each one-line dispatch."""
    arity = {}
    for line in runtime_text.splitlines():
        m = DISPATCH.search(line)
        if not m:
            continue
        slots = [int(x) for x in SLOT.findall(line)]
        arity[m.group(1)] = (max(slots) + 1) if slots else 0
    return arity


def count_args(text, open_paren):
    """Arguments in the call whose '(' is at open_paren, or None if unbalanced.

    Depth-aware, so a nested call counts as one argument. Skips string literals
    so a comma inside "a, b" is not a separator.
    """
    depth = 0
    args = 0
    seen_any = False
    i = open_paren
    n = len(text)
    while i < n:
        c = text[i]
        if c == '"':
            # A STRING IS AN ARGUMENT. The first version skipped the literal
            # without marking that anything had been seen, so every call whose
            # only argument was a string counted as zero - ui_label("...") and
            # thirty others reported as under-supplied. Caught by running the
            # guard against the real tree before trusting a word of it.
            if depth == 1:
                seen_any = True
            i += 1
            while i < n and text[i] != '"':
                i += 1
        elif c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return (args + 1) if seen_any else 0
        elif c == "," and depth == 1:
            args += 1
        elif depth == 1 and not c.isspace():
            seen_any = True
        i += 1
    return None


def scan(path, arity):
    out = []
    for lineno, raw in enumerate(path.read_text().splitlines(), 1):
        line = raw.split("#", 1)[0]
        if not line.strip():
            continue
        for m in re.finditer(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(", line):
            name = m.group(1)
            want = arity.get(name)
            if want is None or want == 0:
                continue
            # `fn name(` is a definition, not a call
            before = line[:m.start()].rstrip()
            if before.endswith("fn"):
                continue
            got = count_args(line, m.end() - 1)
            if got is None:
                continue          # the call is split across lines; see the header
            if got < want:
                out.append((lineno, name, got, want, raw.strip()))
    return out


def main():
    if not RUNTIME.exists():
        print(f"check-nativeargs: runtime not found at {RUNTIME}")
        return 1
    arity = native_arity(RUNTIME.read_text())
    targets = [ROOT / "src/kernel.zl"] + sorted((ROOT / "apps").glob("*.zl"))
    targets = [t for t in targets if t.exists()]
    if not targets:
        print("check-nativeargs: no .zl sources found")
        return 1

    findings = []
    for t in targets:
        for f in scan(t, arity):
            findings.append((t.relative_to(ROOT),) + f)

    checked = sum(1 for v in arity.values() if v > 0)
    if not findings:
        print(f"check-nativeargs: OK - every call supplies what its native reads "
              f"({checked} natives with known arity, {len(targets)} zl sources)")
        return 0
    print("check-nativeargs: FAIL - these calls pass fewer arguments than the "
          "native reads, so it reads a slot the caller never filled")
    for path, lineno, name, got, want, text in findings:
        print(f"  {path}:{lineno}  {name} takes {want}, given {got}")
        print(f"      {text}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
