#!/usr/bin/env python3
"""Keep every shipped kernel route warning-strict and fail-fast."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parent
ROUTES = ("build.sh", "build64.sh", "mkdisk.sh", "buildefi.sh")
REQUIRED_FLAGS = ("-Wall", "-Wextra", "-Werror")


def route_failures(name: str, source: str) -> list[str]:
    errors = []
    for flag in REQUIRED_FLAGS:
        if flag not in source:
            errors.append(f"{name}: missing {flag}")
    code = "\n".join(
        line for line in source.splitlines() if not line.lstrip().startswith("#")
    )
    if re.search(r"(?<![A-Za-z0-9_-])-w(?![A-Za-z0-9_-])", code):
        errors.append(f"{name}: blanket -w suppression")
    if "set -e" not in source:
        errors.append(f"{name}: does not enable shell fail-fast mode")
    return errors


def all_failures(sources: dict[str, str]) -> list[str]:
    errors = []
    for name in ROUTES:
        if name not in sources:
            errors.append(f"missing route script: {name}")
            continue
        errors.extend(route_failures(name, sources[name]))
    return errors


def selftest(sources: dict[str, str]) -> None:
    removed = dict(sources)
    removed["build.sh"] = removed["build.sh"].replace("-Werror", "", 1)
    assert any("build.sh: missing -Werror" == item for item in all_failures(removed))

    silenced = dict(sources)
    silenced["buildefi.sh"] += "\nCF=\"$CF -w\"\n"
    assert any("buildefi.sh: blanket -w suppression" == item for item in all_failures(silenced))

    missing = dict(sources)
    del missing["mkdisk.sh"]
    assert any("missing route script: mkdisk.sh" == item for item in all_failures(missing))
    print("build-contract selftest: caught missing-Werror, blanket-suppression and missing-route")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    sources = {name: (ROOT / name).read_text() for name in ROUTES if (ROOT / name).is_file()}
    if args.selftest:
        selftest(sources)
    errors = all_failures(sources)
    if errors:
        print("build-contract: FAIL")
        for error in errors:
            print(f"  {error}")
        return 1
    print(f"build-contract: PASS: {len(ROUTES)} warning-strict fail-fast routes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
