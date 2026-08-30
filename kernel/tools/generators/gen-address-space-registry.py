#!/usr/bin/env python3
"""Generate and validate the joined physical and Ring 3 address-space map."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import re
import sys
from pathlib import Path


KERNEL = Path(__file__).resolve().parents[2]
ROOT = KERNEL.parent
CONTRACT = KERNEL / "src/arch/x86/address_space_contract.json"
IDENTITY = KERNEL / "metadata/build-identity.json"
OUTPUT = KERNEL / "metadata/address-space-registry.json"
REQUIRED_CATEGORIES = {
    "kernel", "user", "device", "stack", "heap", "shared", "guard", "dynamic"
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def number(value: str) -> int:
    if not isinstance(value, str) or not re.fullmatch(r"0x[0-9a-fA-F]+", value):
        raise ValueError(f"invalid hexadecimal address: {value!r}")
    return int(value, 16)


def normalized_region(row: dict, start_key: str, end_key: str) -> dict:
    start = number(row[start_key])
    end = number(row[end_key])
    return {
        **row,
        start_key: start,
        end_key: end,
        "bytes": end - start,
        "range": f"0x{start:016x}..0x{end:016x}",
    }


def validate_non_overlap(rows: list[dict], start_key: str, end_key: str,
                         label: str) -> None:
    ids = [row.get("id") for row in rows]
    if any(not value for value in ids) or len(ids) != len(set(ids)):
        raise ValueError(f"{label} has missing or duplicate region IDs")
    ordered = sorted(rows, key=lambda row: row[start_key])
    for row in ordered:
        if row[start_key] < 0 or row[end_key] <= row[start_key]:
            raise ValueError(f"{label} region is empty or inverted: {row['id']}")
    for left, right in zip(ordered, ordered[1:]):
        if left[end_key] > right[start_key]:
            raise ValueError(
                f"{label} overlap: {left['id']} ends at 0x{left[end_key]:x}, "
                f"{right['id']} starts at 0x{right[start_key]:x}"
            )


def validate_source_assertions(contract: dict, root: Path) -> list[dict]:
    result = []
    for assertion in contract.get("source_assertions", []):
        path = root / assertion["path"]
        text = path.read_text()
        if re.search(assertion["pattern"], text, re.MULTILINE) is None:
            raise ValueError(
                f"source address contract drifted: {assertion['path']} / "
                f"{assertion['pattern']}"
            )
        result.append({
            "path": assertion["path"],
            "sha256": digest(path),
            "asserted_pattern": assertion["pattern"],
        })
    if not result:
        raise ValueError("address contract has no source assertions")
    return result


def build(root: Path = ROOT) -> dict:
    contract = json.loads((root / CONTRACT.relative_to(ROOT)).read_text())
    identity = json.loads((root / IDENTITY.relative_to(ROOT)).read_text())
    if contract.get("schema") != "zlos.address-space-contract.v1":
        raise ValueError("wrong address-space contract schema")

    physical = [
        normalized_region(row, "start", "end")
        for row in contract.get("physical_regions", [])
    ]
    user = [
        normalized_region(row, "start_offset", "end_offset")
        for row in contract.get("user_slot_template", {}).get("regions", [])
    ]
    validate_non_overlap(physical, "start", "end", "physical map")
    validate_non_overlap(user, "start_offset", "end_offset", "user slot")

    top = number(contract["minimum_physical_bytes"])
    if any(row["end"] > top for row in physical):
        raise ValueError("physical region exceeds the minimum supported RAM")
    slots = contract["user_slot_template"]["pml4_slots"]
    if slots != {"first": 1, "last": 254, "stride": "0x8000000000"}:
        raise ValueError("dynamic user PML4 slot policy drifted")
    if [row["id"] for row in user] != [
            "user-code", "lower-stack-guard", "user-stack",
            "kernel-stack-guard", "kernel-stack", "anonymous-memory"]:
        raise ValueError("user slot no longer has exact guarded stack/anonymous order")
    categories = {row["category"] for row in physical + user}
    if categories != REQUIRED_CATEGORIES:
        raise ValueError(
            "address category coverage drifted: "
            f"missing={sorted(REQUIRED_CATEGORIES - categories)} "
            f"extra={sorted(categories - REQUIRED_CATEGORIES)}"
        )
    if user[1]["permissions"] != "absent" \
            or user[2]["permissions"] != "read-write-no-execute" \
            or user[3]["permissions"] != "absent" \
            or user[4]["permissions"] != "supervisor-read-write-no-execute" \
            or user[5]["permissions"] != \
            "reserved-absent-or-read-write-no-execute" \
            or user[0]["permissions"] != "read-execute":
        raise ValueError("user/kernel code/guard/stack/anonymous permission policy drifted")

    sources = validate_source_assertions(contract, root)
    return {
        "schema": "zlos.address-space-registry.v1",
        "result": "PASS_CURRENT_GENERATED_NON_OVERLAP_WITH_DYNAMIC_USER_TEMPLATE",
        "build_identity": identity["identity_sha256"],
        "contract": {
            "path": str(CONTRACT.relative_to(ROOT)),
            "sha256": digest(root / CONTRACT.relative_to(ROOT)),
        },
        "physical": {
            "minimum_bytes": top,
            "minimum_mib": top >> 20,
            "regions": physical,
        },
        "user_slot_template": {
            **contract["user_slot_template"],
            "pml4_slots": {**slots, "stride": number(slots["stride"])},
            "regions": user,
        },
        "counts": {
            "physical_regions": len(physical),
            "user_template_regions": len(user),
            "source_assertions": len(sources),
            "categories": len(categories),
        },
        "categories": sorted(categories),
        "source_assertions": sources,
        "known_gaps": [
            "the zl-low shared reservation is an envelope; check-memmap.sh owns internal buffer sizes",
            "dynamic user virtual bases depend on a free PML4 slot at runtime",
            "the registry does not prove page-table mutation rollback",
            "the registry is source/build evidence, not QEMU or physical execution",
        ],
        "generator": {
            "path": str(Path(__file__).resolve().relative_to(ROOT)),
            "sha256": digest(Path(__file__).resolve()),
        },
    }


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.address-space-registry.v1" \
            or value.get("result") != \
            "PASS_CURRENT_GENERATED_NON_OVERLAP_WITH_DYNAMIC_USER_TEMPLATE":
        raise ValueError("wrong address-space registry schema/result")
    physical = value.get("physical", {}).get("regions", [])
    user = value.get("user_slot_template", {}).get("regions", [])
    validate_non_overlap(physical, "start", "end", "physical map")
    validate_non_overlap(user, "start_offset", "end_offset", "user slot")
    for rows, start_key, end_key, label in (
            (physical, "start", "end", "physical map"),
            (user, "start_offset", "end_offset", "user slot")):
        for row in rows:
            expected_bytes = row[end_key] - row[start_key]
            expected_range = (
                f"0x{row[start_key]:016x}..0x{row[end_key]:016x}"
            )
            if row.get("bytes") != expected_bytes or row.get("range") != expected_range:
                raise ValueError(f"{label} derived fields drifted: {row['id']}")
    categories = {row.get("category") for row in physical + user}
    if set(value.get("categories", [])) != REQUIRED_CATEGORIES \
            or categories != REQUIRED_CATEGORIES:
        raise ValueError("address-space registry hides category coverage")
    expected_counts = {
        "physical_regions": len(physical),
        "user_template_regions": len(user),
        "source_assertions": len(value.get("source_assertions", [])),
        "categories": len(categories),
    }
    if value.get("counts") != expected_counts:
        raise ValueError("address-space registry count drift")
    if len(value.get("build_identity", "")) != 64:
        raise ValueError("address-space registry lacks a build identity")
    gaps = value.get("known_gaps", [])
    if len(gaps) != 4 or not any("physical" in gap for gap in gaps):
        raise ValueError("address-space registry hides its evidence ceiling")


def selftest(value: dict, root: Path) -> None:
    mutations = {}
    overlap = copy.deepcopy(value)
    overlap["physical"]["regions"][1]["start"] = \
        overlap["physical"]["regions"][0]["end"] - 1
    mutations["physical-overlap"] = overlap
    inverted = copy.deepcopy(value)
    inverted["user_slot_template"]["regions"][2]["end_offset"] = \
        inverted["user_slot_template"]["regions"][2]["start_offset"]
    mutations["inverted-user-stack"] = inverted
    category = copy.deepcopy(value)
    category["categories"].remove("device")
    mutations["missing-category"] = category
    identity = copy.deepcopy(value)
    identity["build_identity"] = "short"
    mutations["foreign-build"] = identity
    derived = copy.deepcopy(value)
    derived["physical"]["regions"][0]["bytes"] += 1
    mutations["derived-field-drift"] = derived
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except ValueError:
            pass
        else:
            raise ValueError(f"address-space selftest mutation escaped: {name}")

    contract = json.loads((root / CONTRACT.relative_to(ROOT)).read_text())
    source = contract["source_assertions"][0]
    contract["source_assertions"][0] = {**source, "pattern": "DELETED_ADDRESS_AUTHORITY"}
    try:
        validate_source_assertions(contract, root)
    except ValueError:
        pass
    else:
        raise ValueError("address-space selftest mutation escaped: source-drift")
    print(
        "address-space selftest: caught physical-overlap, inverted-user-stack, "
        "missing-category, foreign-build, derived-field-drift and source-drift"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        value = build()
        validate(value)
        if args.selftest:
            selftest(value, ROOT)
        if args.write:
            OUTPUT.write_text(json.dumps(value, indent=2) + "\n")
        elif json.loads(OUTPUT.read_text()) != value:
            raise ValueError("address-space-registry.json is missing or stale")
        print(
            "address-space-registry: PASS: "
            f"{value['counts']['physical_regions']} physical + "
            f"{value['counts']['user_template_regions']} user-template regions, "
            f"{value['counts']['categories']} categories"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"address-space-registry: FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
