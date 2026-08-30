#!/usr/bin/env python3
"""Create and enforce strict current-build visual goldens outside live regions."""

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
REPO_ROOT = KERNEL_ROOT.parent
METADATA = KERNEL_ROOT / "metadata"
OUTPUT = METADATA / "visual-golden-registry.json"
RECEIPT = KERNEL_ROOT / "docs/receipts/visual-qemu-2026-08-29.json"
BUILD_IDENTITY = METADATA / "build-identity.json"

MASKS = (
    {"id": "top-live-telemetry", "rect": [0, 0, 1920, 42],
     "reason": "frame timing, wakeup and budget counters advance while running"},
    {"id": "system-monitor-live-surface", "rect": [790, 54, 440, 388],
     "reason": "the booted System Monitor graph and counters sample live state"},
    {"id": "rail-clock-and-power", "rect": [0, 1018, 170, 182],
     "reason": "wall clock, uptime and emulated power state are live"},
    {"id": "bottom-live-telemetry", "rect": [170, 1144, 1750, 56],
     "reason": "memory, frame and uptime telemetry changes between captures"},
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def relative(path: Path) -> str:
    return path.resolve().relative_to(REPO_ROOT.resolve()).as_posix()


def checked_path(raw: str) -> Path:
    path = Path(raw)
    if path.is_absolute() or ".." in path.parts:
        raise ValueError(f"golden path escapes repository: {raw}")
    result = (REPO_ROOT / path).resolve()
    try:
        result.relative_to(REPO_ROOT.resolve())
    except ValueError:
        raise ValueError(f"golden path escapes repository: {raw}")
    if not result.is_file():
        raise ValueError(f"golden is absent: {raw}")
    return result


def stable_pixels(path: Path, masks: tuple[dict, ...] | list[dict]) -> dict:
    from PIL import Image
    with Image.open(path) as source:
        image = source.convert("RGB")
    width, height = image.size
    pixels = bytearray(image.tobytes())
    masked = bytearray(width * height)
    for mask in masks:
        x, y, w, h = mask["rect"]
        if min(x, y, w, h) < 0 or w == 0 or h == 0 \
                or x + w > width or y + h > height:
            raise ValueError(f"mask is outside {width}x{height}: {mask.get('id')}")
        for row in range(y, y + h):
            start = row * width + x
            masked[start:start + w] = b"\x01" * w
    stable = hashlib.sha256()
    stable_count = 0
    masked_count = 0
    for index, is_masked in enumerate(masked):
        start = index * 3
        if is_masked:
            masked_count += 1
            continue
        stable.update(pixels[start:start + 3])
        stable_count += 1
    return {
        "width": width,
        "height": height,
        "stable_pixels": stable_count,
        "masked_pixels": masked_count,
        "stable_pixels_sha256": stable.hexdigest(),
    }


def receipt_captures(receipt: dict) -> list[dict]:
    rows = []
    for route in receipt.get("routes", []):
        for capture in route.get("captures", []):
            rows.append({
                "route": route["route"], "state": capture["state"],
                "path": capture["path"], "sha256": capture["sha256"],
                "pixels_sha256": capture["pixels_sha256"],
                "width": capture["width"], "height": capture["height"],
            })
    return rows


def build() -> dict:
    receipt = load(RECEIPT)
    identity = load(BUILD_IDENTITY)["identity_sha256"]
    if receipt.get("schema") != "zlos.visual-qemu-receipt.v1" \
            or receipt.get("result") != "PASS_CURRENT_ARTIFACT_SCREENSHOTS" \
            or receipt.get("build_identity") != identity:
        raise ValueError("visual receipt is absent, failed or foreign")
    goldens = []
    for capture in receipt_captures(receipt):
        path = checked_path(capture["path"])
        if sha256(path) != capture["sha256"]:
            raise ValueError(f"golden bytes differ from visual receipt: {capture['path']}")
        stable = stable_pixels(path, MASKS)
        if (stable["width"], stable["height"]) != (capture["width"], capture["height"]):
            raise ValueError(f"golden dimensions differ from visual receipt: {capture['path']}")
        goldens.append({**capture, **stable})
    return {
        "schema": "zlos.visual-golden-registry.v1",
        "result": "PASS_CURRENT_STRICT_GOLDENS_WITH_OPEN_MATRIX",
        "build_identity": identity,
        "visual_receipt": {"path": relative(RECEIPT), "sha256": sha256(RECEIPT)},
        "comparison_policy": {
            "stable_region": "exact RGB pixel equality",
            "dynamic_region": "ignored only inside the named rectangles below",
            "dimension_change": "always fail",
            "missing_or_extra_capture": "always fail",
            "thresholds": {"changed_stable_pixels": 0},
        },
        "dynamic_masks": list(MASKS),
        "goldens": goldens,
        "counts": {
            "goldens": len(goldens),
            "routes": len({row["route"] for row in goldens}),
            "states": len({row["state"] for row in goldens}),
            "dynamic_masks": len(MASKS),
            "complete_variant_dimensions": 0,
            "open_variant_dimensions": 6,
        },
        "open_gaps": ["scale", "theme", "locale", "accessibility", "ui-state", "backend"],
        "generator": {"path": relative(Path(__file__).resolve()), "sha256": sha256(Path(__file__).resolve())},
        "evidence_ceiling": "strict current-build BIOS/native-UEFI pixel goldens for desktop-ready and Paint-open outside four named live regions; not a complete variant, physical or design-approval matrix",
        "weakest_link": "four default-path frames cannot cover the six required variant dimensions",
    }


def validate(value: dict) -> None:
    expected = build()
    if value != expected:
        raise ValueError("visual golden registry is stale or drifted")


def compare_one(candidate: Path, golden: dict, masks: list[dict]) -> dict:
    try:
        observed = stable_pixels(candidate, masks)
    except (OSError, ValueError):
        return {
            "candidate": str(candidate),
            "dimensions_match": False,
            "stable_pixels_match": False,
            "result": "FAIL",
        }
    dimensions_match = (observed["width"], observed["height"]) == (
        golden["width"], golden["height"]
    )
    stable_match = dimensions_match and (
        observed["stable_pixels_sha256"] == golden["stable_pixels_sha256"]
    )
    return {
        "candidate": str(candidate),
        "dimensions_match": dimensions_match,
        "stable_pixels_match": stable_match,
        "result": "PASS" if stable_match else "FAIL",
    }


def compare_directory(value: dict, candidate_dir: Path) -> list[dict]:
    results = []
    expected_names = {Path(row["path"]).name for row in value["goldens"]}
    actual_names = {path.name for path in candidate_dir.glob("*.png")}
    if actual_names != expected_names:
        missing = sorted(expected_names - actual_names)
        extra = sorted(actual_names - expected_names)
        raise ValueError(f"candidate capture set drift: missing={missing} extra={extra}")
    for golden in value["goldens"]:
        result = compare_one(candidate_dir / Path(golden["path"]).name,
                             golden, value["dynamic_masks"])
        results.append(result)
    if any(row["result"] != "PASS" for row in results):
        raise ValueError("visual candidate differs outside declared live regions")
    return results


def selftest(value: dict) -> None:
    from PIL import Image
    caught = []
    missing = copy.deepcopy(value)
    missing["goldens"].pop()
    try:
        validate(missing)
    except ValueError:
        caught.append("missing-golden")
    else:
        raise ValueError("visual golden selftest mutation escaped: missing-golden")
    hidden_gap = copy.deepcopy(value)
    hidden_gap["open_gaps"].remove("accessibility")
    try:
        validate(hidden_gap)
    except ValueError:
        caught.append("hidden-variant-gap")
    else:
        raise ValueError("visual golden selftest mutation escaped: hidden-variant-gap")

    golden = value["goldens"][0]
    source = checked_path(golden["path"])
    with tempfile.TemporaryDirectory(prefix="zlos-visual-golden-") as raw:
        temp = Path(raw)
        with Image.open(source) as opened:
            base = opened.convert("RGB")
        stable_mutation = base.copy()
        for y in range(500, 564):
            for x in range(1400, 1464):
                stable_mutation.putpixel((x, y), (255, 0, 255))
        stable_path = temp / "stable.png"
        stable_mutation.save(stable_path)
        if compare_one(stable_path, golden, value["dynamic_masks"])["result"] != "FAIL":
            raise ValueError("visual golden selftest mutation escaped: stable-region-defect")
        caught.append("stable-region-defect")

        masked_mutation = base.copy()
        for y in range(4, 36):
            for x in range(400, 464):
                masked_mutation.putpixel((x, y), (255, 0, 255))
        masked_path = temp / "masked.png"
        masked_mutation.save(masked_path)
        if compare_one(masked_path, golden, value["dynamic_masks"])["result"] != "PASS":
            raise ValueError("declared live-region mutation created visual noise")
        caught.append("declared-live-region-noise")

        wrong_size = base.crop((0, 0, base.width - 1, base.height))
        size_path = temp / "wrong-size.png"
        wrong_size.save(size_path)
        if compare_one(size_path, golden, value["dynamic_masks"])["result"] != "FAIL":
            raise ValueError("visual golden selftest mutation escaped: dimension-drift")
        caught.append("dimension-drift")
    print("visual golden selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    with tempfile.NamedTemporaryFile("w", dir=OUTPUT.parent, delete=False,
                                     encoding="utf-8") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, OUTPUT)


def main() -> int:
    parser = argparse.ArgumentParser()
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true")
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--compare-dir", type=Path)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    try:
        value = build()
        if args.write:
            write_atomic(value)
        elif args.check:
            if not OUTPUT.is_file():
                raise ValueError("visual golden registry is absent")
            validate(load(OUTPUT))
        else:
            if not OUTPUT.is_file():
                raise ValueError("visual golden registry is absent")
            stored = load(OUTPUT)
            validate(stored)
            results = compare_directory(stored, args.compare_dir)
            print(f"visual candidate comparison: PASS: {len(results)} captures")
        if args.selftest:
            selftest(value)
        print(
            "visual golden registry: PASS_CURRENT_STRICT_GOLDENS_WITH_OPEN_MATRIX: "
            f"{value['counts']['goldens']} goldens, {value['counts']['dynamic_masks']} live masks"
        )
        return 0
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as error:
        print(f"visual golden registry: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
