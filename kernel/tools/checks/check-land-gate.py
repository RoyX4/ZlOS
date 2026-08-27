#!/usr/bin/env python3
"""Prove that deleting or conditionally hiding a landing authority is red."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


GATE = Path(__file__).resolve().parents[3] / "gates" / "land-gate.sh"

REQUIRED_SNIPPETS = (
    'run "mandatory boot prerequisites"',
    'run "contained gate launcher contract"',
    'python3 tools/checks/check-land-gate.py --selftest',
    'run "QEMU crash classifier"',
    'run "wrapper inventory write"',
    'run "wrapper inventory check"',
    'python3 tools/checks/check-build-contract.py --selftest',
    'python3 tools/generators/gen-dependency-lock.py --check --selftest',
    'python3 tools/generators/gen-license-registry.py --check --selftest',
    'run "build input identity"',
    'run "toolchain manifest write"',
    'run "toolchain manifest check"',
    'run "build graph write"',
    'run "build graph check"',
    'run "source snapshot write"',
    'run "source snapshot check"',
    'run "kernel ELF permissions"',
    'run "SOURCES recovery selftest"',
    'run "SOURCES coverage"',
    'run "host test inventory"',
    'run "host tests execute"',
    'run "host benchmark receipt"',
    'run "zl call sites"',
    'run "zl generated dispatch"',
    'run "memory map"',
    'run "memory map mutation"',
    'run "memory-map mirrors"',
    'run "memory-map mirror canary"',
    'run "UI scale contract"',
    'run "unique app ids"',
    'run "app registry coverage"',
    'run "application manifest"',
    'run "app lifecycle verifier"',
    'run "reproducible artifact verifier"',
    'run "high-RAM map"',
    'FAIL (reverse SOURCES: kernel/SOURCES is missing)',
    'src/graphics/fonts/font_big.c|src/graphics/icons/icons_rgb.c)',
    'run "reproducible kernel and ISO"',
    'tools/images/mkiso.sh verify.sh tools/checks/verify-iso.sh',
    'tools/checks/verify-64.sh tools/checks/verify-efi.sh',
    'tools/checks/verify-raw.sh tools/checks/verify-disk.sh',
    'tools/checks/verify-clock.sh tools/checks/verify-net.sh',
    'run "final canonical ISO"',
    'run "CPU fault capture QEMU"',
    'run "app routes QEMU"',
    'run "47-app lifecycle QEMU"',
    'run "Run route QEMU"',
    'python3 tools/probes/probe-run.py --no-build',
    'run "application evidence registry write"',
    'run "application evidence registry check"',
    'run "artifact and boot-route registry write"',
    'run "artifact and boot-route registry check"',
    'run "initialization registry write"',
    'run "initialization registry check"',
    'run "adversarial registry write"',
    'run "adversarial registry check"',
    'run "host benchmark receipt check"',
    'run "visual evidence registry write"',
    'run "visual evidence registry check"',
    'run "accessibility proof registry write"',
    'run "accessibility proof registry check"',
    'run "security claim registry write"',
    'run "security claim registry check"',
    'run "decision ledger write"',
    'run "decision ledger check"',
    'run "event trace host receipt write"',
    'run "event trace host receipt check"',
    'run "event schema registry write"',
    'run "event schema registry check"',
    'run "observability registry write"',
    'run "observability registry check"',
    'run "release notes write"',
    'run "release notes check"',
    'run "provenance viewer write"',
    'run "provenance viewer check"',
    'run "joined evidence registry write"',
    'run "joined evidence registry check"',
)

OPTIONAL_AUTHORITY = re.compile(
    r'\[ -[fx] "\$WT/kernel/(?!SOURCES)[^"\n]+" \](?:\s*\\\s*)?(?:&&)?'
)
OPTIONAL_BOOT = re.compile(r'\[ -x "\$WT/kernel/\$g" \] \|\| continue')


def failures(source: str) -> list[str]:
    errors = [
        f"missing mandatory invocation: {snippet}"
        for snippet in REQUIRED_SNIPPETS
        if snippet not in source
    ]
    if OPTIONAL_AUTHORITY.search(source):
        errors.append("kernel authority is hidden behind an existence guard")
    if OPTIONAL_BOOT.search(source):
        errors.append("named boot script can be skipped when absent")
    for required in (
        "out=$( cd \"$dir\" && \"$@\" 2>&1 ); rc=$?",
        'echo ">>> FAIL ($label) exit=$rc"\n    FAIL=$((FAIL+1))',
        "exit $FAIL",
    ):
        if required not in source:
            errors.append(f"landing aggregator does not preserve child failure: {required}")
    return errors


def expect_failure(source: str, label: str) -> None:
    if not failures(source):
        raise AssertionError(f"mutation escaped: {label}")


def selftest(source: str) -> None:
    expect_failure(
        source.replace('run "build input identity"', '# removed identity gate', 1),
        "deleted-verifier",
    )
    expect_failure(
        source.replace(
            'run "app routes QEMU"',
            '[ -f "$WT/kernel/probe-app-routes.py" ] && run "app routes QEMU"',
            1,
        ),
        "optional-verifier",
    )
    expect_failure(
        source.replace(
            'FAIL (reverse SOURCES: kernel/SOURCES is missing)',
            'missing SOURCES ignored',
            1,
        ),
        "missing-sources-manifest",
    )
    expect_failure(
        source.replace(
            'src/graphics/fonts/font_big.c|src/graphics/icons/icons_rgb.c)',
            'src/graphics/fonts/font_big.c)',
            1,
        ),
        "deleted-generated-data-classification",
    )
    expect_failure(
        source.replace(
            'tools/checks/verify-64.sh tools/checks/verify-efi.sh',
            'tools/checks/verify-efi.sh',
            1,
        ),
        "deleted-boot-route",
    )
    expect_failure(source.replace("exit $FAIL", "exit 0", 1), "masked-final-exit")
    expect_failure(
        source.replace(
            'echo ">>> FAIL ($label) exit=$rc"\n    FAIL=$((FAIL+1))',
            'echo ">>> FAIL ($label) exit=$rc"\n    : # failure ignored',
            1,
        ),
        "masked-child-failure",
    )
    print(
        "land-gate selftest: caught deleted-verifier, optional-verifier, "
        "missing-SOURCES, deleted-generated-data-classification, deleted-boot-route, "
        "masked-final-exit and masked-child-failure"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    source = GATE.read_text()
    if args.selftest:
        selftest(source)
    errors = failures(source)
    if errors:
        print("land-gate closure: FAIL")
        for error in errors:
            print(f"  {error}")
        return 1
    print(f"land-gate closure: PASS: {len(REQUIRED_SNIPPETS)} mandatory seams")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
