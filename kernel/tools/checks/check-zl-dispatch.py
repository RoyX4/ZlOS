#!/usr/bin/env python3
"""Reject generated dynamic dispatch to functions defined in zl source."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


KERNEL = Path(__file__).resolve().parents[2]
FN_RE = re.compile(r"^\s*fn\s+([a-z_][a-z_0-9]*)\s*\(", re.MULTILINE)
IMPORT_RE = re.compile(r"^\s*import\s+([a-zA-Z0-9_, ]+)", re.MULTILINE)
DYNAMIC_RE = re.compile(r'\bzl_calln\("([a-z_][a-z_0-9]*)"')


def source_files(root: Path) -> list[Path]:
    entry = root / "src/kernel.zl"
    source = entry.read_text()
    files = [entry]
    for group in IMPORT_RE.findall(source):
        for name in group.replace(" ", "").split(","):
            candidate = root / f"apps/{name}.zl"
            if candidate.is_file():
                files.append(candidate)
    return files


def leaked_dispatches(sources: str, generated: str) -> list[str]:
    defined = set(FN_RE.findall(sources))
    dynamic = set(DYNAMIC_RE.findall(generated))
    return sorted(defined & dynamic)


def selftest() -> None:
    sources = "fn direct() { return 1 }\nfn other(a) { return a }\n"
    generated = 'zl_fn_direct(); zl_calln("builtin", 0);'
    assert leaked_dispatches(sources, generated) == []
    mutated = generated.replace("zl_fn_direct()", 'zl_calln("direct", 0)')
    assert leaked_dispatches(sources, mutated) == ["direct"]
    print("zl dispatch selftest: caught a source function downgraded to zl_calln")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.selftest:
        selftest()

    generated = KERNEL / "out.c"
    if not generated.is_file():
        print("zl dispatch: FAIL: kernel/out.c is missing; build the kernel first")
        return 1

    files = source_files(KERNEL)
    sources = "\n".join(path.read_text() for path in files)
    leaked = leaked_dispatches(sources, generated.read_text())
    if leaked:
        print("zl dispatch: FAIL: source functions emitted as dynamic calls")
        for name in leaked:
            print(f"  {name}")
        return 1

    print(
        f"zl dispatch: PASS: {len(FN_RE.findall(sources))} source definitions "
        "remain direct in generated C"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
