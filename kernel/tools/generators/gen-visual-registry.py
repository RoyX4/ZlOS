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
RECEIPT = KERNEL_ROOT / "docs/receipts/visual-qemu-2026-08-29.json"
ARTIFACT_REGISTRY = METADATA / "artifact-registry.json"
GOLDEN_REGISTRY = METADATA / "visual-golden-registry.json"
PATTERNS = (
    "docs/evidence/virtio-gpu-proof.png",
    "shots/*.png",
    "docs/evidence/exercises/2026-08-24/bios/*.ppm",
    "docs/evidence/exercises/2026-08-24/uefi/*.ppm",
    "tests/host/*.ppm",
    "docs/evidence/current-visuals-2026-08-29/*.png",
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


def classification(relative: str, current: dict[str, dict]) -> tuple[str, str]:
    if relative in current:
        return "CURRENT_QEMU_SCREENSHOT", "exact current artifact/build/route/state receipt"
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
    if value.get("result") != "PARTIAL_CURRENT_VISUAL_EVIDENCE":
        raise ValueError("visual registry result drift")
    assets = value.get("assets", [])
    expected_paths = [path.relative_to(KERNEL_ROOT).as_posix() for path in discover()]
    if [row.get("path") for row in assets] != expected_paths:
        raise ValueError("visual asset set/order drift")
    for row in assets:
        current = row.get("evidence_class") == "CURRENT_QEMU_SCREENSHOT"
        if row.get("current_build_bound") is not current:
            raise ValueError(f"{row.get('path')}: current-build binding drift")
        if current and (not row.get("route") or not row.get("state")
                        or row.get("build_identity") != value.get("build_identity")):
            raise ValueError(f"{row.get('path')}: current visual metadata incomplete")
        if len(row.get("sha256", "")) != 64 or row.get("width", 0) <= 0 or row.get("height", 0) <= 0:
            raise ValueError(f"{row.get('path')}: missing artifact identity/dimensions")
    counts = value.get("counts")
    expected_counts = {
        "assets": len(assets),
        "prior_qemu_unbound": sum(row.get("evidence_class") == "PRIOR_QEMU_SCREENSHOT_UNBOUND" for row in assets),
        "host_render_unbound": sum(row.get("evidence_class") == "HOST_RENDER_UNBOUND" for row in assets),
        "current_build_bound": sum(bool(row.get("current_build_bound")) for row in assets),
        "current_qemu": sum(row.get("evidence_class") == "CURRENT_QEMU_SCREENSHOT" for row in assets),
        "current_routes": len({row.get("route") for row in assets if row.get("current_build_bound")}),
        "current_states": len({row.get("state") for row in assets if row.get("current_build_bound")}),
        "strict_current_goldens": 4,
        "declared_dynamic_masks": 4,
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
    receipt = json.loads(RECEIPT.read_text())
    artifact_registry = json.loads(ARTIFACT_REGISTRY.read_text())
    golden_registry = json.loads(GOLDEN_REGISTRY.read_text())
    if receipt.get("schema") != "zlos.visual-qemu-receipt.v1" \
            or receipt.get("result") != "PASS_CURRENT_ARTIFACT_SCREENSHOTS" \
            or receipt.get("build_identity") != identity \
            or receipt.get("artifact_registry", {}).get("sha256") != sha256(ARTIFACT_REGISTRY) \
            or receipt.get("counts") != {
                "routes": 2, "captures": 4, "states": 2, "blank_captures": 0,
            }:
        raise ValueError("current visual receipt is absent, failed or foreign")
    if golden_registry.get("schema") != "zlos.visual-golden-registry.v1" \
            or golden_registry.get("result") != "PASS_CURRENT_STRICT_GOLDENS_WITH_OPEN_MATRIX" \
            or golden_registry.get("build_identity") != identity \
            or golden_registry.get("visual_receipt", {}).get("sha256") != sha256(RECEIPT) \
            or golden_registry.get("counts", {}).get("goldens") != 4 \
            or golden_registry.get("counts", {}).get("dynamic_masks") != 4:
        raise ValueError("current visual golden registry is absent, failed or foreign")
    current = {}
    for route in receipt.get("routes", []):
        artifact = artifact_registry.get("artifacts", {}).get(
            route.get("artifact", {}).get("name"), {})
        if route.get("artifact", {}).get("sha256") != artifact.get("sha256"):
            raise ValueError(f"{route.get('route')}: current visual artifact mismatch")
        for capture in route.get("captures", []):
            relative = capture.get("path", "")
            prefix = "kernel/"
            if not relative.startswith(prefix):
                raise ValueError("current visual path is outside kernel")
            relative = relative[len(prefix):]
            if relative in current:
                raise ValueError(f"duplicate current visual capture: {relative}")
            current[relative] = {
                **capture,
                "route": route["route"],
                "artifact": route["artifact"],
            }
    if len(current) != 4:
        raise ValueError("current visual receipt does not bind four captures")
    assets = []
    for path in discover():
        relative = path.relative_to(KERNEL_ROOT).as_posix()
        width, height, image_format = dimensions(path)
        evidence_class, reason = classification(relative, current)
        binding = current.get(relative)
        if binding and (binding.get("sha256") != sha256(path)
                        or binding.get("bytes") != path.stat().st_size
                        or binding.get("width") != width
                        or binding.get("height") != height):
            raise ValueError(f"{relative}: visual receipt identity mismatch")
        assets.append({
            "path": relative,
            "sha256": sha256(path),
            "bytes": path.stat().st_size,
            "format": image_format,
            "width": width,
            "height": height,
            "evidence_class": evidence_class,
            "current_build_bound": binding is not None,
            "build_identity": identity if binding else None,
            "route": (binding["route"] if binding else
                      ("bios" if "/bios/" in relative else ("uefi" if "/uefi/" in relative else None))),
            "state": binding.get("state") if binding else None,
            "artifact": binding.get("artifact") if binding else None,
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
        "result": "PARTIAL_CURRENT_VISUAL_EVIDENCE",
        "build_identity": identity,
        "current_receipt": {
            "path": "kernel/docs/receipts/visual-qemu-2026-08-29.json",
            "sha256": sha256(RECEIPT),
        },
        "current_golden_registry": {
            "path": "kernel/metadata/visual-golden-registry.json",
            "sha256": sha256(GOLDEN_REGISTRY),
            "comparison": golden_registry["comparison_policy"],
        },
        "assets": assets,
        "bios_uefi_pairs": pairs,
        "variant_coverage": {
            "scale": {"status": "PARTIAL_CURRENT", "observed": ["1920x1200 automatic scale"], "required": "1x/2x/fractional and multiple modes"},
            "theme": {"status": "PARTIAL_CURRENT", "observed": ["current default"], "required": "light/dark/high-contrast and custom tokens"},
            "locale": {"status": "PARTIAL_CURRENT", "observed": ["built-in English"], "required": "long strings, RTL and non-Latin"},
            "accessibility": {"status": "PARTIAL_CURRENT", "observed": ["default motion and contrast"], "required": "focus, semantics, magnification and reduced motion"},
            "ui-state": {"status": "PARTIAL_CURRENT", "observed": ["desktop-ready", "paint-open"], "required": "empty/loading/error/disabled/focus/hover/pressed"},
            "backend": {"status": "PARTIAL_CURRENT", "observed": ["grub-bios32", "native-uefi64"], "required": "exact artifact-bound BIOS/UEFI/GPU/fallback pairs"},
        },
        "counts": {
            "assets": len(assets),
            "prior_qemu_unbound": sum(row["evidence_class"] == "PRIOR_QEMU_SCREENSHOT_UNBOUND" for row in assets),
            "host_render_unbound": sum(row["evidence_class"] == "HOST_RENDER_UNBOUND" for row in assets),
            "current_build_bound": sum(row["current_build_bound"] for row in assets),
            "current_qemu": sum(row["evidence_class"] == "CURRENT_QEMU_SCREENSHOT" for row in assets),
            "current_routes": len({row["route"] for row in assets if row["current_build_bound"]}),
            "current_states": len({row["state"] for row in assets if row["current_build_bound"]}),
            "strict_current_goldens": golden_registry["counts"]["goldens"],
            "declared_dynamic_masks": golden_registry["counts"]["dynamic_masks"],
            "bios_uefi_pairs": len(pairs),
            "variant_dimensions_complete": 0,
            "variant_dimensions_open": len(REQUIRED_VARIANTS),
        },
        "open_gaps": list(REQUIRED_VARIANTS),
        "evidence_ceiling": "four current-artifact QEMU screenshots and strict stable-region goldens plus historical asset inventory; no video, complete variant matrix, physical display or design approval",
        "weakest_link": f"only 4 of {len(assets)} images are current and all six required variant dimensions remain incomplete",
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
    promoted["assets"][0]["current_build_bound"] = not promoted["assets"][0]["current_build_bound"]
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
    golden = copy.deepcopy(value)
    golden["counts"]["strict_current_goldens"] = 0
    mutations["hidden-current-golden"] = golden
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
            "visual-registry: PARTIAL_CURRENT_VISUAL_EVIDENCE: "
            f"{value['counts']['assets']} assets, {value['counts']['bios_uefi_pairs']} route pairs, "
            f"{value['counts']['current_build_bound']} current-build-bound"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"visual-registry: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
