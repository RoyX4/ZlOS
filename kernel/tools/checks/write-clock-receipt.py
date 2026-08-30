#!/usr/bin/env python3
"""Validate controlled RTC observations and write a build-bound QEMU receipt."""

from __future__ import annotations

import argparse
import copy
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import tempfile


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
IDENTITY = KERNEL_ROOT / "metadata/build-identity.json"
KERNEL = KERNEL_ROOT / "kernel.elf"
DEFAULT_OUTPUT = KERNEL_ROOT / "docs/receipts/rtc-clock-qemu-2026-08-29.json"
EXPECTED = (
    ("2026-08-18T14:37:05", 1787063825),
    ("2026-12-31T23:59:07", 1798761547),
    ("2000-02-29T12:00:00", 951825600),
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def parse_epoch(value: str) -> int:
    parsed = datetime.strptime(value, "%Y-%m-%d %H:%M:%S")
    return int(parsed.replace(tzinfo=timezone.utc).timestamp())


def load_observations(path: Path) -> list[dict]:
    rows = []
    for line in path.read_text().splitlines():
        fields = line.split("\t")
        if len(fields) != 4:
            raise ValueError("clock observation is not four tab-separated fields")
        base, rtc, epoch, offset = fields
        rows.append({
            "base": base,
            "observed_rtc": rtc,
            "observed_epoch": int(epoch),
            "boot_offset_seconds": int(offset),
        })
    return rows


def build(observations: Path, max_boot_seconds: int) -> dict:
    identity = json.loads(IDENTITY.read_text())["identity_sha256"]
    value = {
        "schema": "zlos.rtc-clock-qemu-receipt.v1",
        "result": "PASS",
        "build_identity": identity,
        "artifact": {
            "path": "kernel/kernel.elf",
            "sha256": digest(KERNEL),
        },
        "harness": {
            "path": "kernel/tools/checks/verify-clock.sh",
            "sha256": digest(KERNEL_ROOT / "tools/checks/verify-clock.sh"),
        },
        "cases": load_observations(observations),
        "max_boot_seconds": max_boot_seconds,
        "evidence_ceiling": (
            "controlled QEMU MC146818/CMOS decoding on the exact current kernel.elf; "
            "not physical RTC accuracy, timezone, synchronization or suspend proof"
        ),
        "generator": {
            "path": "kernel/tools/checks/write-clock-receipt.py",
            "sha256": digest(Path(__file__).resolve()),
        },
    }
    validate(value)
    return value


def validate(value: dict) -> None:
    current_identity = json.loads(IDENTITY.read_text())["identity_sha256"]
    if value.get("schema") != "zlos.rtc-clock-qemu-receipt.v1" \
            or value.get("result") != "PASS":
        raise ValueError("wrong RTC receipt schema/result")
    if value.get("build_identity") != current_identity:
        raise ValueError("RTC receipt is not bound to the current build identity")
    artifact = value.get("artifact", {})
    if artifact.get("path") != "kernel/kernel.elf" \
            or artifact.get("sha256") != digest(KERNEL):
        raise ValueError("RTC receipt does not bind the exact current kernel")
    harness = value.get("harness", {})
    if harness.get("sha256") != digest(KERNEL_ROOT / "tools/checks/verify-clock.sh"):
        raise ValueError("RTC receipt does not bind the current harness")
    limit = value.get("max_boot_seconds")
    if not isinstance(limit, int) or limit < 0 or limit > 120:
        raise ValueError("invalid RTC boot-offset ceiling")
    cases = value.get("cases", [])
    if [row.get("base") for row in cases] != [base for base, _ in EXPECTED]:
        raise ValueError("RTC case set/order drift")
    for row, (_, start_epoch) in zip(cases, EXPECTED):
        observed_epoch = row.get("observed_epoch")
        offset = row.get("boot_offset_seconds")
        if parse_epoch(row.get("observed_rtc", "")) != observed_epoch:
            raise ValueError("printed RTC fields disagree with the guest epoch")
        if offset != observed_epoch - start_epoch or offset < 0 or offset > limit:
            raise ValueError("RTC boot offset is outside the admitted range")
    generator = value.get("generator", {})
    if len(generator.get("sha256", "")) != 64:
        raise ValueError("RTC receipt generator identity is missing")


def selftest(value: dict) -> None:
    mutations = {}
    identity = copy.deepcopy(value)
    identity["build_identity"] = "0" * 64
    mutations["foreign-build"] = identity
    missing = copy.deepcopy(value)
    missing["cases"].pop()
    mutations["missing-case"] = missing
    epoch = copy.deepcopy(value)
    epoch["cases"][0]["observed_epoch"] += 1
    mutations["field-epoch-disagreement"] = epoch
    offset = copy.deepcopy(value)
    offset["cases"][0]["boot_offset_seconds"] = value["max_boot_seconds"] + 1
    mutations["excessive-offset"] = offset
    artifact = copy.deepcopy(value)
    artifact["artifact"]["sha256"] = "0" * 64
    mutations["foreign-artifact"] = artifact
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except (ValueError, TypeError):
            caught.append(name)
        else:
            raise ValueError(f"RTC receipt mutation escaped: {name}")
    print("RTC receipt selftest: caught " + ", ".join(caught))


def write_atomic(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", dir=path.parent, delete=False,
                                     encoding="utf-8") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--observations", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--max-boot-seconds", type=int, default=30)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    try:
        value = build(args.observations, args.max_boot_seconds)
        if args.selftest:
            selftest(value)
        write_atomic(args.output, value)
        print(f"RTC QEMU receipt: PASS -> {args.output.relative_to(KERNEL_ROOT)}")
        return 0
    except (OSError, ValueError, TypeError, json.JSONDecodeError) as error:
        print(f"RTC QEMU receipt: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
