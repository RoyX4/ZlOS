#!/usr/bin/env python3
"""Inventory visual artifacts without promoting unbound screenshots."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import struct
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
METADATA = KERNEL_ROOT / "metadata"
OUTPUT = METADATA / "visual-registry.json"
PATTERNS = (
    "docs/evidence/virtio-gpu-proof.png",
    "shots/*.png",
    "docs/evidence/exercises/2026-08-24/bios/*.ppm",
    "docs/evidence/exercises/2026-08-24/uefi/*.ppm",
    "tests/host/*.ppm",
)
REQUIRED_VARIANTS = ("scale", "theme", "locale", "accessibility", "ui-state", "backend")


def sha256(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def dimensions(path: Path) -> tuple[int, int, str]:
    data = path.read_bytes()
    if data.startswith(b"\x89PNG\r\n\x1a\n") and len(data) >= 24:
        width, height = struct.unpack(">II", data[16:24])
        return width, height, "PNG"
    if data.startswith(b"P6"):
        tokens = []
        index = 0
        while len(tokens) < 4 and index < len(data):
            while index < len(data) and chr(data[index]).isspace():
                index += 1
            if index < len(data) and data[index] == ord("#"):
                while index < len(data) and data[index] not in b"\r\n":
                    index += 1
                continue
            start = index
            while index < len(data) and not chr(data[index]).isspace():
                index += 1
            tokens.append(data[start:index])
        if len(tokens) == 4 and tokens[0] == b"P6" and tokens[3] == b"255":
            return int(tokens[1]), int(tokens[2]), "PPM-P6"
    raise ValueError(f"unsupported or malformed visual artifact: {path}")


def classification(relative: str) -> tuple[str, str]:
    if relative.startswith("tests/host/"):
        return "HOST_RENDER_UNBOUND", "host renderer output; no scenario/build sidecar"
    return "PRIOR_QEMU_SCREENSHOT_UNBOUND", "image name suggests prior QEMU evidence; no current build sidecar"


def discover() -> list[Path]:
    paths = []
    for pattern in PATTERNS:
        paths.extend(KERNEL_ROOT.glob(pattern))
    return sorted(set(paths), key=lambda path: path.relative_to(KERNEL_ROOT).as_posix())


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.visual-evidence-registry.v1":
        raise ValueError("wrong visual registry schema")
    if value.get("result") != "INVENTORY_WITH_OPEN_GAPS":
        raise ValueError("visual registry overpromoted unbound assets")
    assets = value.get("assets", [])
    expected_paths = [path.relative_to(KERNEL_ROOT).as_posix() for path in discover()]
    if [row.get("path") for row in assets] != expected_paths:
        raise ValueError("visual asset set/order drift")
    for row in assets:
        if row.get("current_build_bound") is not False:
            raise ValueError(f"{row.get('path')}: unearned current-build binding")
        if len(row.get("sha256", "")) != 64 or row.get("width", 0) <= 0 or row.get("height", 0) <= 0:
            raise ValueError(f"{row.get('path')}: missing artifact identity/dimensions")
    counts = value.get("counts")
    expected_counts = {
        "assets": len(assets),
        "prior_qemu_unbound": sum(row.get("evidence_class") == "PRIOR_QEMU_SCREENSHOT_UNBOUND" for row in assets),
        "host_render_unbound": sum(row.get("evidence_class") == "HOST_RENDER_UNBOUND" for row in assets),
        "current_build_bound": sum(bool(row.get("current_build_bound")) for row in assets),
        "bios_uefi_pairs": len(value.get("bios_uefi_pairs", [])),
        "variant_dimensions_complete": 0,
        "variant_dimensions_open": 6,
    }
    if counts != expected_counts:
        raise ValueError("visual registry counts drift")
    variants = value.get("variant_coverage", {})
    if list(variants) != list(REQUIRED_VARIANTS) or any(item.get("status") == "COMPLETE" for item in variants.values()):
        raise ValueError("visual variant gaps were hidden")
    if value.get("open_gaps") != list(REQUIRED_VARIANTS):
        raise ValueError("visual open-gap set/order drift")
    if len(value.get("build_identity", "")) != 64:
        raise ValueError("missing current build identity boundary")


def build() -> dict:
    identity = json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"]
    assets = []
    for path in discover():
        relative = path.relative_to(KERNEL_ROOT).as_posix()
        width, height, image_format = dimensions(path)
        evidence_class, reason = classification(relative)
        assets.append({
            "path": relative,
            "sha256": sha256(path),
            "bytes": path.stat().st_size,
            "format": image_format,
            "width": width,
            "height": height,
            "evidence_class": evidence_class,
            "current_build_bound": False,
            "build_identity": None,
            "route": "bios" if "/bios/" in relative else ("uefi" if "/uefi/" in relative else None),
            "reason": reason,
        })
    by_path = {row["path"]: row for row in assets}
    pairs = []
    exercise_prefix = "docs/evidence/exercises/2026-08-24"
    for bios in sorted(path for path in by_path if path.startswith(f"{exercise_prefix}/bios/")):
        uefi = bios.replace("/bios/", "/uefi/", 1)
        if uefi not in by_path:
            raise ValueError(f"missing UEFI visual pair: {bios}")
        left, right = by_path[bios], by_path[uefi]
        pairs.append({
            "scenario": Path(bios).name,
            "bios_path": bios,
            "uefi_path": uefi,
            "same_dimensions": (left["width"], left["height"]) == (right["width"], right["height"]),
            "byte_identical": left["sha256"] == right["sha256"],
            "evidence_ceiling": "unbound route comparison only",
        })
    value = {
        "schema": "zlos.visual-evidence-registry.v1",
        "result": "INVENTORY_WITH_OPEN_GAPS",
        "build_identity": identity,
        "assets": assets,
        "bios_uefi_pairs": pairs,
        "variant_coverage": {
            "scale": {"status": "MISSING_METADATA", "required": "1x/2x/fractional and multiple modes"},
            "theme": {"status": "MISSING_METADATA", "required": "light/dark/high-contrast and custom tokens"},
            "locale": {"status": "MISSING_METADATA", "required": "long strings, RTL and non-Latin"},
            "accessibility": {"status": "MISSING_METADATA", "required": "focus, semantics, magnification and reduced motion"},
            "ui-state": {"status": "MISSING_METADATA", "required": "empty/loading/error/disabled/focus/hover/pressed"},
            "backend": {"status": "PARTIAL_UNBOUND", "required": "exact artifact-bound BIOS/UEFI/GPU/fallback pairs"},
        },
        "counts": {
            "assets": len(assets),
            "prior_qemu_unbound": sum(row["evidence_class"] == "PRIOR_QEMU_SCREENSHOT_UNBOUND" for row in assets),
            "host_render_unbound": sum(row["evidence_class"] == "HOST_RENDER_UNBOUND" for row in assets),
            "current_build_bound": sum(row["current_build_bound"] for row in assets),
            "bios_uefi_pairs": len(pairs),
            "variant_dimensions_complete": 0,
            "variant_dimensions_open": len(REQUIRED_VARIANTS),
        },
        "open_gaps": list(REQUIRED_VARIANTS),
        "evidence_ceiling": "asset inventory and unbound BIOS/UEFI pairing only; no current visual-regression promotion",
        "weakest_link": f"none of {len(assets)} images names the current build identity; all six required variant dimensions remain open",
        "generator": {"path": "kernel/tools/generators/gen-visual-registry.py", "sha256": sha256(Path(__file__).resolve())},
    }
    validate(value)
    return value


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["assets"].pop()
    mutations["missing-asset"] = missing
    promoted = copy.deepcopy(value)
    promoted["assets"][0]["current_build_bound"] = True
    mutations["invented-build-binding"] = promoted
    dimensions_mutation = copy.deepcopy(value)
    dimensions_mutation["assets"][0]["width"] = 0
    mutations["missing-dimensions"] = dimensions_mutation
    hidden = copy.deepcopy(value)
    hidden["open_gaps"].remove("accessibility")
    mutations["hidden-variant-gap"] = hidden
    complete = copy.deepcopy(value)
    complete["variant_coverage"]["theme"]["status"] = "COMPLETE"
    mutations["unearned-variant-promotion"] = complete
    caught = []
    for name, mutated in mutations.items():
        try:
            validate(mutated)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"visual-registry selftest mutation escaped: {name}")
    print("visual-registry selftest: caught " + ", ".join(caught))


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
        if args.check and (not OUTPUT.is_file() or json.loads(OUTPUT.read_text()) != value):
            raise ValueError("visual-registry.json is missing or stale")
        print(
            "visual-registry: INVENTORY_WITH_OPEN_GAPS: "
            f"{value['counts']['assets']} assets, {value['counts']['bios_uefi_pairs']} route pairs, "
            f"{value['counts']['current_build_bound']} current-build-bound"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"visual-registry: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
