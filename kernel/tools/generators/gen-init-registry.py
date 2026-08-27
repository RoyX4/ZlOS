#!/usr/bin/env python3
"""Generate the ordered zlOS boot-initialization registry from kernel.zl."""

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
METADATA = KERNEL_ROOT / "metadata"
SOURCE = KERNEL_ROOT / "src/kernel.zl"
ARTIFACT_REGISTRY = METADATA / "artifact-registry.json"
BUILD_IDENTITY = METADATA / "build-identity.json"
OUTPUT = METADATA / "init-registry.json"
BOOT_MARKER = "# ---- boot"

STAGE_SPECS = (
    ("INIT-001", "gdt", "setup_gdt()", (), "cpu", "required-success",
     "Failure is fatal before protected execution can continue."),
    ("INIT-002", "idt-pic-pit-ps2", "setup_idt()", ("INIT-001",), "interrupts", "required-success",
     "Installs the IDT, remaps PIC, starts PIT and baseline PS/2 handling."),
    ("INIT-003", "framebuffer-write-combining", "fb_wc()", ("INIT-002",), "display", "optional-provider",
     "Applies the framebuffer cache policy when the current mapping supports it."),
    ("INIT-004", "apic-routing", "boot_apic = apic_up()", ("INIT-002",), "interrupts", "optional-provider",
     "A failed APIC admission explicitly retains the legacy 8259 PIC path."),
    ("INIT-005", "usb-bootstrap", "boot_usb = usb_boot()", ("INIT-002",), "usb", "required-attempt",
     "Controller/class admission is bounded; unsupported hardware leaves fallback input paths."),
    ("INIT-006", "persistent-observer", "diag_boot = diag_up()", ("INIT-005",), "storage", "optional-provider",
     "Admits only the exact ZLLOG target; otherwise evidence remains RAM-only."),
    ("INIT-007", "keyboard-state", "kbd_init()", ("INIT-002",), "input", "required-attempt",
     "Initializes shared keyboard translation state after interrupt ownership is established."),
    ("INIT-008", "settings-load", "set_load()", ("INIT-006", "INIT-007"), "settings", "optional-data",
     "Missing or invalid persisted settings retain defaults and report why."),
    ("INIT-009", "filesystem-mount", "fs_try()", ("INIT-006", "INIT-008"), "storage", "optional-data",
     "Mounts an existing zlfs volume without formatting unknown media."),
    ("INIT-010", "program-arena", "arena_up()", ("INIT-009",), "memory", "required-attempt",
     "Publishes exact bounds and refuses program execution if the fixed arena is unavailable."),
    ("INIT-011", "kernel-heap", "heap_up()", ("INIT-010",), "memory", "required-attempt",
     "Publishes capacity and refuses allocations when the mapped heap is unavailable."),
    ("INIT-012", "virtual-memory-report", "vmm_up()", ("INIT-011",), "memory", "required-attempt",
     "Reports the paging window established by heap initialization."),
    ("INIT-013", "display-layout", "if px_w() > 0 { layout() }", ("INIT-012",), "display", "conditional",
     "Framebuffer routes negotiate a mode and layout; text-only routes retain VGA geometry."),
    ("INIT-014", "compositor-bootstrap", "wm_boot = wm_boot_start()", ("INIT-013",), "window-system", "conditional",
     "Framebuffer routes require a compositor; no-framebuffer routes keep the text shell fallback."),
    ("INIT-015", "ring3-smoke", "user_up()", ("INIT-002", "INIT-012", "INIT-014"), "process", "required-proof",
     "Runs the current ring-3 syscall smoke path; failure prevents the later ready marker."),
    ("INIT-016", "application-manifest", "app_manifest_report()", ("INIT-015",), "product-identity", "required-proof",
     "The running image must report the exact 62-surface manifest digest."),
    ("INIT-017", "build-identity", "build_identity_report()", ("INIT-016",), "artifact-identity", "required-proof",
     "The running image must report the exact build-input identity and source state."),
    ("INIT-018", "ready-publication", 'color(C_GREEN)  print("  ready.")', ("INIT-017",), "boot", "required-proof",
     "Published only after every earlier required attempt/proof; all promoted QEMU routes require it."),
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def source_lines() -> tuple[list[str], int]:
    lines = SOURCE.read_text().splitlines()
    starts = [index for index, line in enumerate(lines) if line.startswith(BOOT_MARKER)]
    if len(starts) != 1:
        raise ValueError(f"expected one boot marker, found {len(starts)}")
    return lines, starts[0]


def locate(lines: list[str], start: int, marker: str) -> int:
    matches = []
    for index in range(start + 1, len(lines)):
        stripped = lines[index].lstrip()
        if stripped.startswith("#"):
            continue
        if stripped == marker:
            matches.append(index + 1)
    if len(matches) != 1:
        raise ValueError(f"init marker {marker!r}: expected one source line, found {len(matches)}")
    return matches[0]


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.init-registry.v1" or value.get("result") != "PASS":
        raise ValueError("wrong init registry schema/result")
    expected_ids = [spec[0] for spec in STAGE_SPECS]
    stages = value.get("stages")
    if not isinstance(stages, list) or [stage.get("id") for stage in stages] != expected_ids:
        raise ValueError("init stage set/order drift")
    if len(set(expected_ids)) != len(expected_ids):
        raise ValueError("duplicate canonical init ID")
    routes = value.get("routes")
    if not isinstance(routes, list) or len(routes) != 6 or len(set(routes)) != 6:
        raise ValueError("init route coverage must contain six unique routes")
    seen = set()
    previous_line = 0
    for stage in stages:
        stage_id = stage["id"]
        line = stage.get("source_line", 0)
        if not isinstance(line, int) or line <= previous_line:
            raise ValueError(f"{stage_id}: source order is not strictly increasing")
        previous_line = line
        dependencies = stage.get("dependencies")
        if not isinstance(dependencies, list) or any(dep not in seen for dep in dependencies):
            raise ValueError(f"{stage_id}: dependency is unknown or not earlier")
        if stage.get("routes") != routes:
            raise ValueError(f"{stage_id}: route coverage drift")
        seen.add(stage_id)
    source = value.get("source", {})
    if source.get("path") != "kernel/src/kernel.zl" or len(source.get("sha256", "")) != 64:
        raise ValueError("missing init source identity")
    if len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("missing init generator identity")


def build() -> dict:
    lines, boot_start = source_lines()
    artifact = load(ARTIFACT_REGISTRY)
    identity = load(BUILD_IDENTITY)
    if artifact.get("result") != "PASS" or len(artifact.get("boot_routes", {})) != 6:
        raise ValueError("artifact registry does not supply six passing routes")
    if artifact.get("build_identity", {}).get("id") != identity.get("identity_sha256"):
        raise ValueError("artifact/build identity mismatch")
    routes = list(artifact["boot_routes"])
    stages = []
    for stage_id, name, marker, dependencies, owner, obligation, failure in STAGE_SPECS:
        stages.append({
            "id": stage_id,
            "name": name,
            "owner": owner,
            "source_anchor": marker,
            "source_line": locate(lines, boot_start, marker),
            "dependencies": list(dependencies),
            "obligation": obligation,
            "failure_behavior": failure,
            "routes": routes,
            "evidence": "SOURCE_ORDER_AND_LATER_READY_ON_ALL_PROMOTED_QEMU_ROUTES",
        })
    value = {
        "schema": "zlos.init-registry.v1",
        "result": "PASS",
        "build_identity": identity["identity_sha256"],
        "source": {
            "path": "kernel/src/kernel.zl",
            "sha256": digest(SOURCE),
            "boot_marker_line": boot_start + 1,
        },
        "generator": {
            "path": "kernel/tools/generators/gen-init-registry.py",
            "sha256": digest(Path(__file__).resolve()),
        },
        "routes": routes,
        "stages": stages,
        "evidence_ceiling": (
            "source order plus six QEMU routes reaching later ready/identity markers; "
            "not stage-specific physical-hardware proof"
        ),
        "weakest_link": (
            "several legacy init calls expose no typed status and keep policy in the monolithic kernel"
        ),
    }
    validate(value)
    return value


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["stages"].pop(8)
    mutations["missing-stage"] = missing
    reordered = copy.deepcopy(value)
    reordered["stages"][5], reordered["stages"][6] = reordered["stages"][6], reordered["stages"][5]
    mutations["reordered-stage"] = reordered
    unknown = copy.deepcopy(value)
    unknown["stages"][4]["dependencies"] = ["INIT-999"]
    mutations["unknown-dependency"] = unknown
    routes = copy.deepcopy(value)
    routes["routes"].pop()
    mutations["missing-route"] = routes
    caught = []
    for name, mutated in mutations.items():
        try:
            validate(mutated)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"init registry selftest mutation escaped: {name}")
    print("init-registry selftest: caught " + ", ".join(caught))


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
                raise ValueError("init-registry.json is missing or stale")
        print(f"init-registry: PASS: {len(value['stages'])} ordered stages, {len(value['routes'])} routes")
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"init-registry: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
