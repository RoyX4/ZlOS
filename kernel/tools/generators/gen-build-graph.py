#!/usr/bin/env python3
"""Generate the current source -> object -> artifact graph for all zlOS lanes."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import re
import tempfile
from pathlib import Path, PurePosixPath


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
ROOT = KERNEL_ROOT.parent
METADATA = KERNEL_ROOT / "metadata"
OUTPUT = METADATA / "build-graph.json"
IDENTITY = METADATA / "build-identity.json"
ARTIFACTS = METADATA / "artifact-registry.json"
TOOLCHAIN = METADATA / "toolchain-manifest.json"
IMPORT_RE = re.compile(r"^\s*import\s+([A-Za-z_]\w*)\s*(?:#.*)?$", re.MULTILINE)
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.MULTILINE)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def safe_relative(value: str) -> str:
    pure = PurePosixPath(value)
    if pure.is_absolute() or not pure.parts or any(part in ("", ".", "..") for part in pure.parts):
        raise ValueError(f"unsafe graph path: {value!r}")
    return pure.as_posix()


def object_name(lane: str, source: str) -> str:
    stem = Path(source).stem
    if lane in ("bios-multiboot32", "raw-bios-loader32"):
        return f"_{stem}.o"
    if lane == "multiboot64":
        return f"_{stem}64.o"
    if lane == "native-uefi64":
        return f"_efi_{stem}.o"
    raise ValueError(f"unknown lane: {lane}")


def build() -> dict:
    identity = load(IDENTITY)
    artifacts = load(ARTIFACTS)
    toolchain = load(TOOLCHAIN)
    build_id = identity["identity_sha256"]
    if toolchain.get("build_identity") != build_id:
        raise ValueError("build graph toolchain has stale build identity")
    artifact_build_id = artifacts.get("build_identity", {}).get("id")
    if len(artifact_build_id or "") != 64:
        raise ValueError("artifact registry has no subject build identity")
    artifact_current = artifact_build_id == build_id
    source_hashes = identity["source_files_sha256"]
    source_paths = sorted(source_hashes)
    sources_list = [
        line.strip()
        for line in (KERNEL_ROOT / "SOURCES").read_text().splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    ]
    if len(sources_list) != len(set(sources_list)):
        raise ValueError("SOURCES contains duplicate compilation units")

    nodes: dict[str, dict] = {}
    edges: set[tuple[str, str, str]] = set()

    def node(identifier: str, kind: str, **fields: object) -> str:
        value = {"id": identifier, "kind": kind, **fields}
        previous = nodes.get(identifier)
        if previous is not None and previous != value:
            raise ValueError(f"node identity collision: {identifier}")
        nodes[identifier] = value
        return identifier

    def edge(source: str, target: str, relation: str) -> None:
        if source not in nodes or target not in nodes:
            raise ValueError(f"edge references missing node: {source} -> {target}")
        edges.add((source, target, relation))

    for path in source_paths:
        safe_relative(path)
        node(
            "source:" + path,
            "source-input",
            path=path,
            sha256=source_hashes[path],
            suffix=Path(path).suffix,
        )

    node("scope:declared-build-input", "scope", claim="declared input without a more precise consumer edge")
    node("meta:build-identity", "generated-metadata", path="kernel/metadata/build-identity.json")
    node("meta:app-manifest", "generated-metadata", path="kernel/metadata/app-manifest.json")
    node("generated:zl-c", "generated-source", path="kernel/out.c")

    # Exact zl import closure. Each imported module feeds the generated C once;
    # the generated C is then separately compiled for every target lane.
    zl_sources = sorted(path for path in source_paths if path.endswith(".zl"))
    for path in zl_sources:
        edge("source:" + path, "generated:zl-c", "zl-frontend-input")
    for path in zl_sources:
        text = (ROOT / path).read_text()
        for module in IMPORT_RE.findall(text):
            candidates = (
                f"kernel/src/{module}.zl",
                f"kernel/apps/{module}.zl",
                f"kernel/{module}.zl",
            )
            imported = next((candidate for candidate in candidates
                             if candidate in source_hashes), None)
            if imported:
                edge("source:" + imported, "source:" + path, "zl-imported-by")

    # Local include graph. Angle-bracket compiler/sysroot dependencies belong
    # to toolchain-manifest.json and are not falsely relabelled repository input.
    for path in source_paths:
        if Path(path).suffix not in (".c", ".h"):
            continue
        text = (ROOT / path).read_text(errors="strict")
        including_dir = (ROOT / path).parent
        for include in INCLUDE_RE.findall(text):
            candidates = ((including_dir / include).resolve(), (ROOT / include).resolve())
            for candidate in candidates:
                try:
                    relative = candidate.relative_to(ROOT).as_posix()
                except ValueError:
                    continue
                if relative in source_hashes:
                    edge("source:" + relative, "source:" + path, "included-by")
                    break

    lanes = (
        {
            "id": "bios-multiboot32",
            "script": "kernel/build.sh",
            "artifact": "kernel.elf",
            "special": (
                ("freestanding/runtime_kernel.c", "_rt.o"),
                ("kernel/boot/gdt.c", "_gdt.o"),
                ("kernel/boot/smp_trampoline.S", "_smptr.o"),
                ("kernel/boot/boot.S", "_boot.o"),
            ),
            "linker_script": "kernel/boot/link.ld",
        },
        {
            "id": "multiboot64",
            "script": "kernel/build64.sh",
            "artifact": "kernel64.elf",
            "special": (
                ("freestanding/runtime_kernel.c", "_rt64.o"),
                ("kernel/boot/gdt64.c", "_gdt64.o"),
                ("kernel/boot/smp_trampoline64.S", "_smptr64.o"),
                ("kernel/boot/boot64.S", "_boot64.o"),
            ),
            "linker_script": "kernel/boot/link64.ld",
        },
        {
            "id": "native-uefi64",
            "script": "kernel/buildefi.sh",
            "artifact": "BOOTX64.EFI",
            "special": (
                ("kernel/boot/efi.c", "_efi_efi.o"),
                ("freestanding/runtime_kernel.c", "_efi_runtime_kernel.o"),
                ("kernel/boot/gdt64.c", "_efi_gdt64.o"),
                ("kernel/boot/smp_trampoline64.S", "_efi_smptr.o"),
            ),
            "linker_script": None,
        },
        {
            "id": "raw-bios-loader32",
            "script": "kernel/tools/images/mkdisk.sh",
            "artifact": "kernel_raw.elf",
            "special": (
                ("freestanding/runtime_kernel.c", "_rt.o"),
                ("kernel/boot/gdt.c", "_gdt.o"),
                ("kernel/boot/smp_trampoline.S", "_smptr.o"),
                ("kernel/boot/raw_entry.S", "_rawentry.o"),
            ),
            "linker_script": "kernel/boot/link-raw.ld",
        },
    )

    lane_rows = []
    for lane in lanes:
        lane_id = lane["id"]
        artifact_id = "artifact:" + lane["artifact"]
        artifact_row = artifacts["artifacts"][lane["artifact"]]
        node(
            artifact_id,
            "artifact" if artifact_current else "historical-artifact",
            path=artifact_row["path"],
            sha256=artifact_row["sha256"],
            bytes=artifact_row["bytes"],
            current_build_bound=artifact_current,
        )
        script_id = "source:" + lane["script"]
        edge(script_id, artifact_id, "build-script-produces")
        if lane["linker_script"]:
            edge("source:" + lane["linker_script"], artifact_id, "linker-script-produces")
        edge("source:kernel/SOURCES", artifact_id, "declares-compiled-units")

        generated_object = node(
            f"object:{lane_id}:generated-zl.o",
            "logical-object",
            lane=lane_id,
            physical_path={
                "bios-multiboot32": "kernel/_gen.o",
                "multiboot64": "kernel/_gen64.o",
                "native-uefi64": "kernel/_efi__genefi.o",
                "raw-bios-loader32": "kernel/_gen.o",
            }[lane_id],
        )
        edge("generated:zl-c", generated_object, "compiled-to")
        edge(generated_object, artifact_id, "linked-into")

        object_count = 1
        for source in sources_list:
            relative = "kernel/" + source
            if relative not in source_hashes:
                raise ValueError(f"SOURCES unit is absent from build identity: {relative}")
            physical = "kernel/" + object_name(lane_id, source)
            object_id = node(
                f"object:{lane_id}:{source}",
                "logical-object",
                lane=lane_id,
                physical_path=physical,
            )
            edge("source:" + relative, object_id, "compiled-to")
            edge(object_id, artifact_id, "linked-into")
            object_count += 1
        for source, physical_name in lane["special"]:
            if source not in source_hashes:
                raise ValueError(f"special lane source is absent from build identity: {source}")
            object_id = node(
                f"object:{lane_id}:special:{Path(source).name}",
                "logical-object",
                lane=lane_id,
                physical_path="kernel/" + physical_name,
            )
            edge("source:" + source, object_id, "compiled-to")
            edge(object_id, artifact_id, "linked-into")
            object_count += 1
        lane_rows.append(
            {
                "id": lane_id,
                "primary_artifact": lane["artifact"],
                "logical_objects": object_count,
                "declared_c_units": len(sources_list),
            }
        )

    # Packaging/media derivations for every final artifact in the registry.
    package_edges = (
        ("kernel.elf", "zlOS.iso", "embedded-payload"),
        ("kernel64.elf", "zlOS64.iso", "embedded-payload"),
        ("kernel_raw.elf", "zlOS.img", "flattened-and-boot-wrapped"),
        ("BOOTX64.EFI", "zlOS-usb.img", "efi-system-partition-payload"),
        ("boot-media-ids.json", "zlOS-usb.img", "media-identity-input"),
    )
    for name, row in artifacts["artifacts"].items():
        artifact_id = "artifact:" + name
        node(artifact_id, "artifact" if artifact_current else "historical-artifact",
             path=row["path"], sha256=row["sha256"], bytes=row["bytes"],
             current_build_bound=artifact_current)
        producer = "source:kernel/" + row["producer"]
        if producer in nodes:
            edge(producer, artifact_id, "producer-script")
    for source, target, relation in package_edges:
        edge("artifact:" + source, "artifact:" + target, relation)
    if "source:kernel/boot/raw_boot.asm" in nodes:
        edge("source:kernel/boot/raw_boot.asm", "artifact:zlOS.img", "boot-sector-input")
    if "source:kernel/tools/reproducible_time.c" in nodes:
        edge("source:kernel/tools/reproducible_time.c", "artifact:zlOS.iso", "reproducible-metadata-helper")
        edge("source:kernel/tools/reproducible_time.c", "artifact:zlOS64.iso", "reproducible-metadata-helper")
        edge("source:kernel/tools/reproducible_time.c", "artifact:zlOS-usb.img", "reproducible-metadata-helper")
    if "source:kernel/tools/generators/gen-app-manifest.py" in nodes:
        edge("source:kernel/tools/generators/gen-app-manifest.py", "meta:app-manifest", "generates")
    if "source:kernel/metadata/app-manifest.json" in nodes:
        edge("source:kernel/metadata/app-manifest.json", "meta:app-manifest", "manifest-payload")
    if "source:kernel/tools/generators/gen-build-identity.py" in nodes:
        edge("source:kernel/tools/generators/gen-build-identity.py", "meta:build-identity", "generates")

    outgoing = {source for source, _, _ in edges}
    scope_only = []
    for path in source_paths:
        identifier = "source:" + path
        if identifier not in outgoing:
            edge(identifier, "scope:declared-build-input", "declared-scope-only")
            scope_only.append(path)

    rows = [nodes[key] for key in sorted(nodes)]
    edge_rows = [
        {"from": source, "to": target, "relation": relation}
        for source, target, relation in sorted(edges)
    ]
    artifact_names = sorted(artifacts["artifacts"])
    artifact_incoming = {name: 0 for name in artifact_names}
    for row in edge_rows:
        if row["to"].startswith("artifact:"):
            name = row["to"].split(":", 1)[1]
            if name in artifact_incoming:
                artifact_incoming[name] += 1
    if artifact_current and any(count == 0 for count in artifact_incoming.values()):
        raise ValueError("one or more artifacts has no producing edge")

    result = ("PASS_CURRENT_ARTIFACTS" if artifact_current
              else "PASS_RECIPE_WITH_HISTORICAL_ARTIFACT_SNAPSHOT")
    value = {
        "schema": "zlos.build-graph.v1",
        "result": result,
        "build_identity": build_id,
        "artifact_snapshot": {
            "subject_build_identity": artifact_build_id,
            "current_build_bound": artifact_current,
            "evidence_ceiling": (
                "exact current artifact/QEMU registry bound to this build identity"
                if artifact_current else
                "exact historical artifact/QEMU registry; recipe edges do not promote "
                "those bytes as outputs of the current source identity"
            ),
        },
        "artifact_registry_sha256": sha256(ARTIFACTS),
        "toolchain_manifest_sha256": sha256(TOOLCHAIN),
        "source_inputs": source_paths,
        "lanes": lane_rows,
        "nodes": rows,
        "edges": edge_rows,
        "counts": {
            "source_inputs": len(source_paths),
            "declared_c_units": len(sources_list),
            "target_lanes": len(lane_rows),
            "artifacts": len(artifact_names),
            "current_artifacts": len(artifact_names) if artifact_current else 0,
            "historical_artifacts": 0 if artifact_current else len(artifact_names),
            "nodes": len(rows),
            "edges": len(edge_rows),
            "orphan_source_inputs": 0,
            "scope_only_inputs": len(scope_only),
        },
        "scope_only_inputs": scope_only,
        "open_gaps": [
            "logical object graph is source-derived; object hashes are represented by final reproducibility receipts, not per-object receipts",
            "conservative header-superset inputs may be scope-only rather than active includes",
            "archive/package/service outputs beyond the current nine-artifact registry are not yet graph nodes",
        ] + ([] if artifact_current else [
            "the exact artifact snapshot is historical and no artifact hash is bound to the current source identity",
        ]),
        "generator": {"path": "kernel/tools/generators/gen-build-graph.py", "sha256": sha256(Path(__file__).resolve())},
        "evidence_ceiling": (
            "complete declared-input build recipe for four lanes joined to the current reproducible "
            "nine-artifact snapshot; not a compiler-emitted per-object attestation"
            if artifact_current else
            "complete declared-input build recipe for four lanes joined to a clearly historical "
            "nine-artifact snapshot; not current artifact provenance or a compiler-emitted per-object attestation"
        ),
    }
    validate(value, source_hashes, artifact_names)
    return value


def validate(value: dict, source_hashes: dict[str, str], artifact_names: list[str]) -> None:
    if value.get("schema") != "zlos.build-graph.v1":
        raise ValueError("wrong build-graph schema/result")
    if value.get("source_inputs") != sorted(source_hashes):
        raise ValueError("build graph source-input set/order drifted")
    lanes = value.get("lanes")
    expected_lanes = ["bios-multiboot32", "multiboot64", "native-uefi64", "raw-bios-loader32"]
    if not isinstance(lanes, list) or [lane.get("id") for lane in lanes] != expected_lanes:
        raise ValueError("build graph lane set/order drifted")
    nodes = value.get("nodes")
    edges = value.get("edges")
    if not isinstance(nodes, list) or not isinstance(edges, list):
        raise ValueError("build graph nodes/edges are missing")
    node_ids = [node.get("id") for node in nodes]
    if node_ids != sorted(node_ids) or len(node_ids) != len(set(node_ids)):
        raise ValueError("build graph node identity/order drifted")
    if any(edge.get("from") not in set(node_ids) or edge.get("to") not in set(node_ids) for edge in edges):
        raise ValueError("build graph edge references a missing node")
    outgoing = {edge["from"] for edge in edges}
    missing = ["source:" + path for path in sorted(source_hashes) if "source:" + path not in outgoing]
    if missing:
        raise ValueError(f"build graph has orphan source inputs: {missing[:3]}")
    graph_artifacts = sorted(node["id"].split(":", 1)[1] for node in nodes
                             if node.get("kind") in ("artifact", "historical-artifact"))
    if graph_artifacts != artifact_names:
        raise ValueError("build graph artifact set drifted")
    counts = value.get("counts", {})
    expected_counts = {
        "source_inputs": len(source_hashes),
        "declared_c_units": counts.get("declared_c_units"),
        "target_lanes": len(lanes),
        "artifacts": len(artifact_names),
        "current_artifacts": counts.get("current_artifacts"),
        "historical_artifacts": counts.get("historical_artifacts"),
        "nodes": len(nodes),
        "edges": len(edges),
        "orphan_source_inputs": 0,
        "scope_only_inputs": len(value.get("scope_only_inputs", [])),
    }
    if counts != expected_counts or counts.get("declared_c_units", 0) <= 0:
        raise ValueError("build graph counts drifted")
    snapshot = value.get("artifact_snapshot", {})
    current = snapshot.get("current_build_bound") is True
    expected_result = ("PASS_CURRENT_ARTIFACTS" if current
                       else "PASS_RECIPE_WITH_HISTORICAL_ARTIFACT_SNAPSHOT")
    expected_current = len(artifact_names) if current else 0
    expected_historical = 0 if current else len(artifact_names)
    if value.get("result") != expected_result \
            or counts.get("current_artifacts") != expected_current \
            or counts.get("historical_artifacts") != expected_historical \
            or len(snapshot.get("subject_build_identity", "")) != 64:
        raise ValueError("artifact snapshot binding/counts disagree")
    if len(value.get("artifact_registry_sha256", "")) != 64 \
            or len(value.get("toolchain_manifest_sha256", "")) != 64 \
            or len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("build graph provenance identity is missing")
    if len(value.get("open_gaps", [])) < 3:
        raise ValueError("build graph evidence ceiling is hidden")


def selftest(value: dict, source_hashes: dict[str, str], artifact_names: list[str]) -> None:
    mutations = {}
    source = copy.deepcopy(value)
    source["source_inputs"].pop()
    mutations["missing-source"] = source
    lane = copy.deepcopy(value)
    lane["lanes"].pop()
    mutations["missing-lane"] = lane
    edge = copy.deepcopy(value)
    victim = "source:" + value["source_inputs"][0]
    edge["edges"] = [row for row in edge["edges"] if row["from"] != victim]
    edge["counts"]["edges"] = len(edge["edges"])
    mutations["orphan-source"] = edge
    artifact = copy.deepcopy(value)
    artifact["nodes"] = [row for row in artifact["nodes"] if row.get("id") != "artifact:zlOS.iso"]
    artifact["counts"]["nodes"] = len(artifact["nodes"])
    mutations["missing-artifact"] = artifact
    binding = copy.deepcopy(value)
    binding["artifact_snapshot"]["current_build_bound"] = not binding["artifact_snapshot"]["current_build_bound"]
    mutations["artifact-binding-count-drift"] = binding
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant, source_hashes, artifact_names)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"build-graph mutation escaped: {name}")
    print("build-graph selftest: caught " + ", ".join(caught))


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
        source_hashes = load(IDENTITY)["source_files_sha256"]
        artifact_names = sorted(load(ARTIFACTS)["artifacts"])
        if args.selftest:
            selftest(value, source_hashes, artifact_names)
        if args.write:
            write_atomic(value)
        if args.check and (not OUTPUT.is_file() or load(OUTPUT) != value):
            raise ValueError("build-graph.json is missing or stale")
        print(
            f"build-graph: {value['result']}: "
            f"{value['counts']['source_inputs']} inputs, {value['counts']['target_lanes']} lanes, "
            f"{value['counts']['artifacts']} artifacts, {value['counts']['scope_only_inputs']} scope-only"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"build-graph: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
