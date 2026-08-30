#!/usr/bin/env python3
"""Capture current-artifact QEMU frames and issue a fail-closed visual receipt."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
REPO_ROOT = KERNEL_ROOT.parent
METADATA = KERNEL_ROOT / "metadata"
ARTIFACT_REGISTRY = METADATA / "artifact-registry.json"
BUILD_IDENTITY = METADATA / "build-identity.json"
OUTPUT = KERNEL_ROOT / "docs/receipts/visual-qemu-2026-08-29.json"
EVIDENCE_DIR = KERNEL_ROOT / "docs/evidence/current-visuals-2026-08-29"
ORACLE = KERNEL_ROOT / "tests/oracle"
ZLOSBOOT = ORACLE / "zlosboot.py"
EXERCISE = KERNEL_ROOT / "tools/probes/exercise.py"
sys.path.insert(0, str(ORACLE))
from zlosboot import Machine, open_app  # noqa: E402


ROUTES = (
    ("grub-bios32", "zlOS.iso", False, "qemu-system-i386"),
    ("native-uefi64", "zlOS-usb.img", True, "qemu-system-x86_64"),
)
STATES = ("desktop-ready", "paint-open")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def canonical_sha256(value: dict) -> str:
    subject = copy.deepcopy(value)
    subject.pop("content_sha256", None)
    return hashlib.sha256(json.dumps(
        subject, sort_keys=True, separators=(",", ":")
    ).encode()).hexdigest()


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def relative(path: Path) -> str:
    return path.resolve().relative_to(REPO_ROOT.resolve()).as_posix()


def checked_path(raw: str) -> Path:
    path = Path(raw)
    if path.is_absolute() or ".." in path.parts:
        raise ValueError(f"visual evidence path escapes repository: {raw}")
    resolved = (REPO_ROOT / path).resolve()
    try:
        resolved.relative_to(REPO_ROOT.resolve())
    except ValueError:
        raise ValueError(f"visual evidence path escapes repository: {raw}")
    if not resolved.is_file():
        raise ValueError(f"visual evidence is absent: {raw}")
    return resolved


def validate_artifact(artifact: dict, context: str) -> Path:
    path = checked_path(artifact["path"])
    if sha256(path) != artifact["sha256"] \
            or path.stat().st_size != artifact["bytes"]:
        raise ValueError(f"{context}: artifact differs from registry")
    return path


def qemu_identity(binary: str) -> dict:
    path = shutil.which(binary)
    if path is None:
        raise ValueError(f"missing QEMU binary: {binary}")
    version = subprocess.run(
        [path, "--version"], check=True, capture_output=True, text=True
    ).stdout.splitlines()[0]
    return {"path": path, "sha256": sha256(Path(path)), "version": version}


def png_identity(path: Path) -> dict:
    from PIL import Image
    with Image.open(path) as image:
        rgb = image.convert("RGB")
        width, height = rgb.size
        colors = len(rgb.getcolors(maxcolors=1 << 24) or [])
        pixels_sha256 = hashlib.sha256(rgb.tobytes()).hexdigest()
    if width <= 0 or height <= 0 or colors < 16:
        raise ValueError(f"blank or invalid visual capture: {path}")
    return {
        "path": relative(path), "sha256": sha256(path),
        "pixels_sha256": pixels_sha256, "bytes": path.stat().st_size,
        "format": "PNG", "width": width, "height": height,
        "distinct_colors": colors,
    }


def save_frame(machine: Machine, route: str, state: str) -> dict:
    from PIL import Image
    ppm = Path(machine.tmp) / f"{route}-{state}.ppm"
    if not machine.qmp.screendump(str(ppm)):
        raise ValueError(f"{route}/{state}: QMP screendump failed")
    EVIDENCE_DIR.mkdir(parents=True, exist_ok=True)
    output = EVIDENCE_DIR / f"{route}-{state}.png"
    with Image.open(ppm) as image:
        rgb = image.convert("RGB")
        temporary = output.with_suffix(".png.tmp")
        rgb.save(temporary, format="PNG", compress_level=9)
        os.replace(temporary, output)
    return png_identity(output)


def capture_route(route: str, artifact_name: str, uefi: bool,
                  qemu_binary: str, registry: dict, build_id: str) -> dict:
    artifact = registry["artifacts"][artifact_name]
    validate_artifact(artifact, route)
    route_record = registry["boot_routes"][route]
    frames = []
    with Machine(do_build=False, how="native", uefi=uefi,
                 boot_timeout=420.0, cmd_timeout=90.0) as machine:
        machine.ser.drain(2.0)
        frames.append({
            "state": "desktop-ready", "interaction": "boot to ready and shell prompt",
            **save_frame(machine, route, "desktop-ready"),
        })
        open_app(machine.ser, machine.qmp, "paint", machine.w, machine.h, 90.0)
        machine.ser.drain(2.0)
        frames.append({
            "state": "paint-open", "interaction": "serial shell command paint; compositor reported a new window",
            **save_frame(machine, route, "paint-open"),
        })
        log = machine.ser.all
        log_path = EVIDENCE_DIR / f"{route}-serial.log"
        temporary = log_path.with_suffix(".log.tmp")
        temporary.write_text(log)
        os.replace(temporary, log_path)
        marker = f"build-identity: schema=1 id={build_id}"
        markers = {
            "build_identity": marker in log,
            "ready": "ready." in log,
            "shell_prompt": "zl>" in log,
            "paint_window": "wm: win " in log and "paint\n" in log,
        }
        if not all(markers.values()):
            missing = [name for name, present in markers.items() if not present]
            raise ValueError(f"{route}: serial markers missing: {', '.join(missing)}")
    # Re-read the host artifact after QEMU exits. A visual receipt is invalid if
    # the observation itself changed the bytes it claims to identify.
    validate_artifact(artifact, f"{route} after capture")
    return {
        "route": route,
        "firmware": "UEFI" if uefi else "BIOS",
        "artifact": {
            "name": artifact_name, "path": artifact["path"],
            "sha256": artifact["sha256"], "bytes": artifact["bytes"],
        },
        "boot_receipt": {
            "path": route_record["receipt"],
            "sha256": route_record["receipt_sha256"],
        },
        "qemu": qemu_identity(qemu_binary),
        "serial_log": {
            "path": relative(log_path), "sha256": sha256(log_path),
            "bytes": log_path.stat().st_size, "markers": markers,
        },
        "display": {"width": frames[0]["width"], "height": frames[0]["height"]},
        "captures": frames,
    }


def run() -> dict:
    registry = load(ARTIFACT_REGISTRY)
    identity = load(BUILD_IDENTITY)
    build_id = identity["identity_sha256"]
    if registry.get("schema") != "zlos.artifact-boot-registry.v1" \
            or registry.get("result") != "PASS" \
            or registry.get("build_identity", {}).get("id") != build_id:
        raise ValueError("artifact registry is absent, failed or foreign")
    routes = [capture_route(*spec, registry, build_id) for spec in ROUTES]
    receipt = {
        "schema": "zlos.visual-qemu-receipt.v1",
        "result": "PASS_CURRENT_ARTIFACT_SCREENSHOTS",
        "build_identity": build_id,
        "artifact_registry": {"path": relative(ARTIFACT_REGISTRY), "sha256": sha256(ARTIFACT_REGISTRY)},
        "routes": routes,
        "counts": {
            "routes": len(routes),
            "captures": sum(len(row["captures"]) for row in routes),
            "states": len(STATES),
            "blank_captures": 0,
        },
        "variant_metadata": {
            "scale": "current automatic scale for observed mode",
            "theme": "current default theme",
            "locale": "current built-in English strings",
            "accessibility": "default motion/contrast path; no assistive-provider claim",
            "ui_state": list(STATES),
            "backend": [row["route"] for row in routes],
        },
        "sources": {
            "runner": {"path": relative(Path(__file__).resolve()), "sha256": sha256(Path(__file__).resolve())},
            "machine": {"path": relative(ZLOSBOOT), "sha256": sha256(ZLOSBOOT)},
            "qemu_harness": {"path": relative(EXERCISE), "sha256": sha256(EXERCISE)},
        },
        "evidence_ceiling": "four current-artifact QEMU screenshots across BIOS and native UEFI in two states; not complete variant coverage, video, physical display or design approval",
        "weakest_link": "theme, locale, accessibility, scale, full UI-state and all-backend matrices remain incomplete",
    }
    receipt["content_sha256"] = canonical_sha256(receipt)
    validate(receipt)
    return receipt


def validate(value: dict) -> None:
    registry = load(ARTIFACT_REGISTRY)
    build_id = load(BUILD_IDENTITY)["identity_sha256"]
    if value.get("schema") != "zlos.visual-qemu-receipt.v1" \
            or value.get("result") != "PASS_CURRENT_ARTIFACT_SCREENSHOTS" \
            or value.get("build_identity") != build_id:
        raise ValueError("visual receipt is absent, failed or foreign")
    if value.get("artifact_registry") != {
            "path": relative(ARTIFACT_REGISTRY), "sha256": sha256(ARTIFACT_REGISTRY)}:
        raise ValueError("visual receipt artifact registry binding drift")
    routes = value.get("routes", [])
    if [(row.get("route"), row.get("artifact", {}).get("name")) for row in routes] \
            != [(route, artifact) for route, artifact, _, _ in ROUTES]:
        raise ValueError("visual receipt route/artifact matrix drift")
    capture_count = 0
    for row, (_, artifact_name, _, qemu_binary) in zip(routes, ROUTES):
        artifact = registry["artifacts"][artifact_name]
        validate_artifact(artifact, row.get("route", "unknown route"))
        if row.get("artifact") != {
                "name": artifact_name, "path": artifact["path"],
                "sha256": artifact["sha256"], "bytes": artifact["bytes"]}:
            raise ValueError(f"{row.get('route')}: visual artifact identity mismatch")
        expected_boot = registry["boot_routes"][row["route"]]
        if row.get("boot_receipt") != {
                "path": expected_boot["receipt"], "sha256": expected_boot["receipt_sha256"]}:
            raise ValueError(f"{row['route']}: boot receipt binding mismatch")
        if row.get("qemu") != qemu_identity(qemu_binary):
            raise ValueError(f"{row['route']}: QEMU identity drift")
        serial = row.get("serial_log", {})
        serial_path = checked_path(serial.get("path", ""))
        if serial.get("sha256") != sha256(serial_path) \
                or serial.get("bytes") != serial_path.stat().st_size \
                or not all(serial.get("markers", {}).values()):
            raise ValueError(f"{row['route']}: serial evidence drift")
        captures = row.get("captures", [])
        if [capture.get("state") for capture in captures] != list(STATES):
            raise ValueError(f"{row['route']}: visual state set/order drift")
        for capture in captures:
            path = checked_path(capture.get("path", ""))
            identity = png_identity(path)
            if any(capture.get(key) != identity[key] for key in identity):
                raise ValueError(f"{row['route']}/{capture.get('state')}: capture identity drift")
            if capture.get("width") != row.get("display", {}).get("width") \
                    or capture.get("height") != row.get("display", {}).get("height"):
                raise ValueError(f"{row['route']}: capture/display dimensions drift")
            capture_count += 1
    if value.get("counts") != {
            "routes": 2, "captures": 4, "states": 2, "blank_captures": 0} \
            or capture_count != 4:
        raise ValueError("visual receipt counts drift")
    expected_sources = {
        "runner": {"path": relative(Path(__file__).resolve()), "sha256": sha256(Path(__file__).resolve())},
        "machine": {"path": relative(ZLOSBOOT), "sha256": sha256(ZLOSBOOT)},
        "qemu_harness": {"path": relative(EXERCISE), "sha256": sha256(EXERCISE)},
    }
    if value.get("sources") != expected_sources:
        raise ValueError("visual receipt source identities drift")
    if value.get("content_sha256") != canonical_sha256(value):
        raise ValueError("visual receipt content digest mismatch")


def selftest(value: dict) -> None:
    mutations = []
    missing_route = copy.deepcopy(value)
    missing_route["routes"].pop()
    mutations.append(("missing-route", missing_route))
    foreign = copy.deepcopy(value)
    foreign["build_identity"] = "f" * 64
    mutations.append(("foreign-build", foreign))
    artifact = copy.deepcopy(value)
    artifact["routes"][0]["artifact"]["sha256"] = "0" * 64
    mutations.append(("wrong-artifact", artifact))
    state = copy.deepcopy(value)
    state["routes"][0]["captures"].pop()
    mutations.append(("missing-state", state))
    image = copy.deepcopy(value)
    image["routes"][0]["captures"][0]["sha256"] = "0" * 64
    mutations.append(("image-byte-drift", image))
    pixels = copy.deepcopy(value)
    pixels["routes"][0]["captures"][0]["pixels_sha256"] = "0" * 64
    mutations.append(("pixel-drift", pixels))
    blank = copy.deepcopy(value)
    blank["routes"][0]["captures"][0]["distinct_colors"] = 1
    mutations.append(("blank-capture", blank))
    serial = copy.deepcopy(value)
    serial["routes"][0]["serial_log"]["markers"]["build_identity"] = False
    mutations.append(("missing-serial-identity", serial))
    dimensions = copy.deepcopy(value)
    dimensions["routes"][0]["display"]["width"] += 1
    mutations.append(("dimension-drift", dimensions))
    digest = copy.deepcopy(value)
    digest["variant_metadata"]["theme"] = "changed after digest"
    mutations.append(("content-digest-drift", digest))
    caught = []
    for name, mutation in mutations:
        try:
            validate(mutation)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"visual receipt selftest mutation escaped: {name}")
    print("visual receipt selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", dir=OUTPUT.parent, delete=False,
                                     encoding="utf-8") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, OUTPUT)


def main() -> int:
    parser = argparse.ArgumentParser()
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--run", action="store_true")
    mode.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    try:
        if args.run:
            value = run()
            write_atomic(value)
        else:
            value = load(OUTPUT)
            validate(value)
        if args.selftest:
            selftest(value)
        print(
            "visual receipt: PASS_CURRENT_ARTIFACT_SCREENSHOTS: "
            f"{value['counts']['routes']} routes, {value['counts']['captures']} captures"
        )
        return 0
    except (OSError, ValueError, KeyError, json.JSONDecodeError,
            subprocess.CalledProcessError) as error:
        print(f"visual receipt: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
