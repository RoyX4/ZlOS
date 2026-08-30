#!/usr/bin/env python3
"""check-zlcalls.py - every function kernel.zl calls must exist. No QEMU.

WHY. zl resolves a call BY NAME AT RUNTIME. A name that is neither a user `fn`
nor a kernel builtin falls off the end of the builtin table in
freestanding/runtime_kernel.c and lands on

    kfatal("builtin not available in the kernel subset");

which unmutes the console, prints, and HALTS THE MACHINE. There is no link
error: `build.sh` reports "undefined symbols: 0" because that is `nm` counting C
symbols, and a zl call site is a string.

It happened. `reg_mono()` in apps_registry.zl has forwarded to `sys2_mono(id)`
and `sys3_mono(id)` since the slice files were written, and neither function was
ever defined. sys2's six apps are the ones with no icon, so the catalog asks
them for a monogram - and the first time anyone scrolled "All Applications" far
enough to show Kernel Log, zlOS died. The tile drew its background and its
category stripe, the remaining 27 tiles never drew at all, and the desktop
froze. From outside it looked exactly like an empty window.

WHAT IT CHECKS. Every `name(` call site in every kernel .zl file, against:
  * every `fn name(` defined anywhere in the same corpus,
  * every builtin the kernel runtime answers for, read out of
    runtime_kernel.c's `streq(name, "...")` table rather than transcribed,
  * zl's keywords, which are not calls.

Run it with the build; it costs half a second.
"""
import glob
import os
import re
import sys

KERNEL_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
RUNTIME = os.path.join(KERNEL_ROOT, "..", "freestanding", "runtime_kernel.c")

# `if (x)`, `while (x)` and friends are not calls. zl's own keyword list is
# lexer.c's; these are the ones that can be followed by a parenthesis.
KEYWORDS = {"if", "while", "for", "return", "fn", "else", "import", "in",
            "true", "false", "and", "or", "not"}

DEF = re.compile(r"^\s*fn\s+([A-Za-z_]\w*)\s*\(", re.M)
CALL = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
BUILTIN = re.compile(r'streq\(name,\s*"([A-Za-z_]\w*)"\)')
# lexer.c:272 drops comments, so a call inside one is not a call.
COMMENT = re.compile(r"#.*$", re.M)
STRING = re.compile(r'"[^"\n]*"')


def sources():
    paths = [os.path.join(KERNEL_ROOT, "src", "kernel.zl")]
    paths += sorted(glob.glob(os.path.join(KERNEL_ROOT, "apps", "apps_*.zl")))
    return {p: open(p, encoding="utf-8", errors="replace").read() for p in paths}


def main():
    src = sources()
    defined = set()
    for text in src.values():
        defined |= set(DEF.findall(text))

    try:
        rt = open(RUNTIME, encoding="utf-8", errors="replace").read()
    except OSError as e:
        print(f"cannot read the runtime's builtin table: {e}", file=sys.stderr)
        return 2
    builtins = set(BUILTIN.findall(rt))
    if len(builtins) < 100:
        print(f"only {len(builtins)} builtins found in {RUNTIME} - the "
              f"`streq(name, \"...\")` shape changed and this check would "
              f"report the whole kernel as undefined", file=sys.stderr)
        return 2

    known = defined | builtins | KEYWORDS
    missing = {}
    for path, text in src.items():
        stripped = STRING.sub('""', COMMENT.sub("", text))
        for ln, line in enumerate(stripped.splitlines(), 1):
            for name in CALL.findall(line):
                if name not in known:
                    missing.setdefault(name, []).append(
                        f"{os.path.basename(path)}:{ln}")

    if not missing:
        print(f"check-zlcalls: {len(defined)} zl functions, {len(builtins)} "
              f"builtins, every call site resolves")
        return 0

    print("check-zlcalls: CALLS THAT NOTHING DEFINES", file=sys.stderr)
    for name in sorted(missing):
        where = ", ".join(missing[name][:4])
        more = "" if len(missing[name]) <= 4 else f" (+{len(missing[name]) - 4} more)"
        print(f"  {name}()  called at {where}{more}", file=sys.stderr)
    print("  zl resolves calls by NAME AT RUNTIME. Each of these reaches\n"
          "  kfatal(\"builtin not available in the kernel subset\") the moment\n"
          "  it executes, which HALTS THE MACHINE - it is not a link error and\n"
          "  build.sh's \"undefined symbols: 0\" does not cover it.",
          file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
