#!/usr/bin/env python3
"""Record exact shipped-source license evidence without inventing a grant."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
ROOT = KERNEL_ROOT.parent
METADATA = KERNEL_ROOT / "metadata"
BUILD_IDENTITY = METADATA / "build-identity.json"
OUTPUT = METADATA / "license-registry.json"
LICENSE_NAMES = {"license", "license.md", "license.txt", "copying", "copying.md", "notice", "notice.md"}


def sha256(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def license_files() -> list[Path]:
    found = []
    for path in ROOT.rglob("*"):
        if not path.is_file() or path.name.lower() not in LICENSE_NAMES:
            continue
        if any(part in {".git", "build", "build-audit", "__pycache__"} for part in path.parts):
            continue
        found.append(path)
    return sorted(found)


def validate(value: dict, expected_paths: list[str]) -> None:
    if value.get("schema") != "zlos.license-provenance-registry.v1":
        raise ValueError("wrong license registry schema")
    if value.get("result") != "PASS_WITH_RELEASE_BLOCK":
        raise ValueError("license gap must remain an explicit release block")
    if value.get("public_release_blocked") is not True:
        raise ValueError("public release is not blocked despite missing per-input grants")
    inputs = value.get("inputs")
    if not isinstance(inputs, list) or [row.get("path") for row in inputs] != expected_paths:
        raise ValueError("license input set/order drift")
    for row in inputs:
        if len(row.get("sha256", "")) != 64:
            raise ValueError(f"{row.get('path')}: missing source identity")
        if row.get("license_status") not in {"UNDECLARED_IN_REPOSITORY", "REVIEW_REQUIRED"}:
            raise ValueError(f"{row.get('path')}: unsupported license conclusion")
        if row.get("redistribution_grant") != "NOT_ESTABLISHED":
            raise ValueError(f"{row.get('path')}: unproved redistribution grant")
    if len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("missing license generator identity")


def build() -> dict:
    identity = load(BUILD_IDENTITY)
    source_hashes = identity.get("source_files_sha256")
    if not isinstance(source_hashes, dict) or not source_hashes:
        raise ValueError("build identity has no source inventory")
    declared = license_files()
    status = "REVIEW_REQUIRED" if declared else "UNDECLARED_IN_REPOSITORY"
    inputs = [
        {
            "path": path,
            "sha256": source_hash,
            "origin_status": "REPOSITORY_LOCAL_INPUT_AUTHORSHIP_NOT_INFERRED",
            "license_status": status,
            "redistribution_grant": "NOT_ESTABLISHED",
        }
        for path, source_hash in source_hashes.items()
    ]
    value = {
        "schema": "zlos.license-provenance-registry.v1",
        "result": "PASS_WITH_RELEASE_BLOCK",
        "build_identity": identity["identity_sha256"],
        "license_files": [
            {
                "path": str(path.relative_to(ROOT)),
                "bytes": path.stat().st_size,
                "sha256": sha256(path),
                "mapping_status": "NOT_MAPPED_TO_BUILD_INPUTS",
            }
            for path in declared
        ],
        "inputs": inputs,
        "counts": {
            "build_inputs": len(inputs),
            "declared_license_files": len(declared),
            "inputs_with_established_redistribution_grant": 0,
        },
        "public_release_blocked": True,
        "release_block_reason": (
            "no build input has a repository-evidenced license and redistribution mapping"
        ),
        "generator": {
            "path": "kernel/gen-license-registry.py",
            "sha256": sha256(Path(__file__).resolve()),
        },
        "evidence_ceiling": (
            "repository-local source/hash and license-file presence only; authorship and legal rights are not inferred"
        ),
        "weakest_link": (
            "the repository currently contains no license file and no per-input third-party provenance mapping"
        ),
    }
    validate(value, list(source_hashes))
    return value


def selftest(value: dict) -> None:
    expected = [row["path"] for row in value["inputs"]]
    mutations = {}
    missing = copy.deepcopy(value)
    missing["inputs"].pop()
    mutations["missing-input"] = missing
    grant = copy.deepcopy(value)
    grant["inputs"][0]["redistribution_grant"] = "ESTABLISHED"
    mutations["invented-grant"] = grant
    release = copy.deepcopy(value)
    release["public_release_blocked"] = False
    mutations["false-release-green"] = release
    identity = copy.deepcopy(value)
    identity["inputs"][0]["sha256"] = "short"
    mutations["missing-source-identity"] = identity
    caught = []
    for name, mutated in mutations.items():
        try:
            validate(mutated, expected)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"license-registry selftest mutation escaped: {name}")
    print("license-registry selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    with tempfile.NamedTemporaryFile("w", dir=HERE, delete=False, encoding="utf-8") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, OUTPUT)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if not args.write and not args.check:
        args.check = True
    try:
        value = build()
        if args.selftest:
            selftest(value)
        if args.write:
            write_atomic(value)
        if args.check:
            if not OUTPUT.is_file() or load(OUTPUT) != value:
                raise ValueError("license-registry.json is missing or stale")
        print(
            f"license-registry: PASS_WITH_RELEASE_BLOCK: {len(value['inputs'])} inputs, "
            f"{len(value['license_files'])} license files"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"license-registry: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
