#!/usr/bin/env python3
"""Generate/check one machine-readable identity manifest for current zlOS apps.

The source authorities are kernel.zl::app_name(), apps_registry.zl::reg_name(),
the four slice name functions, reg_exists(), and ds-reference.html's category
arrays. The output covers 61 current named implementations plus the separate
All Applications catalogue surface. It is identity/static-route evidence, not
a runtime launch claim.
"""

from __future__ import annotations

import argparse
import glob
import hashlib
import json
import os
import re
import sys


HERE = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.dirname(os.path.dirname(HERE))
ROOT = os.path.dirname(KERNEL_ROOT)
METADATA = os.path.join(KERNEL_ROOT, "metadata")
REFERENCE = os.path.join(ROOT, "docs", "design", "ds-reference.html")
OUTPUT = os.path.join(METADATA, "app-manifest.json")
EMBED = os.path.join(KERNEL_ROOT, "app_manifest_embed.zl")

CONST_RE = re.compile(r"^([A-Z][A-Z0-9_]*)\s*=\s*(\d+)\s*(?:#|$)", re.M)
ARM_RE = re.compile(r'if (?:app|id) == (\w+)\s*\{\s*return "([^"]+)"')
FUNCTION_HEAD_RE = re.compile(r"fn (\w+)\((?:app|id)\)\s*\{")
IMPORT_RE = re.compile(r"^import\s+([A-Za-z_][A-Za-z0-9_]*)\s*(?:#.*)?$", re.M)


def fail(message):
    raise ValueError(message)


def read(path):
    try:
        with open(path, encoding="utf-8", errors="strict") as handle:
            return handle.read()
    except OSError as error:
        fail(f"cannot read {path}: {error}")


def constants(sources):
    values = {}
    for path, text in sources.items():
        for name, raw in CONST_RE.findall(text):
            value = int(raw)
            if name in values and values[name] != value:
                fail(f"constant {name} differs: {values[name]} vs {value} in {path}")
            values[name] = value
    return values


def function_body(text, name):
    match = re.search(r"fn %s\([A-Za-z_][A-Za-z0-9_]*\)\s*\{" % re.escape(name), text)
    if not match:
        fail(f"missing function {name}")
    start = match.end()
    depth = 1
    for index in range(start, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index]
    fail(f"unterminated function {name}")


def named_arms(body, consts, owner):
    result = {}
    for token, name in ARM_RE.findall(body):
        value = int(token) if token.isdigit() else consts.get(token)
        if value is None:
            fail(f"{owner}: unresolved app token {token}")
        previous = result.get(value)
        if previous is not None and previous != name:
            fail(f"{owner}: id {value} has names {previous!r} and {name!r}")
        result[value] = name
    return result


def reference_kinds(reference):
    kinds = {}
    for match in re.finditer(r"\['\w+','([^']+)','u\w+'\]", reference):
        kinds[match.group(1)] = "utility"
    for match in re.finditer(r"\['\w+',(?:\"([^\"]+)\"|'([^']+)'),'\w+',\d+,\d+,'g\w+'\]", reference):
        kinds[match.group(1) or match.group(2)] = "game"
    for match in re.finditer(r"\{id:'\w+',\s*name:'([^']+)'[^}]*?w:\d+,\s*h:\d+", reference):
        kinds[match.group(1)] = "system"
    if len(kinds) != 53:
        fail(f"reference category inventory has {len(kinds)} names, expected 53")
    return kinds


def dense_registry(registry, consts):
    first = consts.get("REG_FIRST")
    last = consts.get("REG_LAST")
    if first is None or last is None:
        fail("REG_FIRST/REG_LAST missing")
    body = function_body(registry, "reg_exists")
    holes = set()
    for lo, hi in re.findall(r"if id >= (\d+) \{ if id <= (\d+) \{ return 0", body):
        holes.update(range(int(lo), int(hi) + 1))
    holes.update(int(value) for value in re.findall(r"if id == (\d+) \{ return 0", body))
    return [value for value in range(first, last + 1) if value not in holes]


def source_digest(paths):
    digest = hashlib.sha256()
    for path in sorted(paths):
        digest.update(os.path.basename(path).encode("utf-8"))
        digest.update(b"\0")
        digest.update(read(path).encode("utf-8"))
        digest.update(b"\0")
    return digest.hexdigest()


def imported_app_paths(kernel_path, kernel):
    modules = IMPORT_RE.findall(kernel)
    if len(modules) != len(set(modules)):
        fail("kernel.zl imports an app module more than once")
    imported = []
    for module in modules:
        if not module.startswith("apps_"):
            continue
        path = os.path.join(KERNEL_ROOT, "apps", module + ".zl")
        if not os.path.isfile(path):
            fail(f"kernel.zl imports missing app module {module}.zl")
        imported.append(path)
    discovered = set(glob.glob(os.path.join(KERNEL_ROOT, "apps", "apps_*.zl")))
    imported_set = set(imported)
    if discovered != imported_set:
        missing = sorted(os.path.basename(path) for path in discovered - imported_set)
        unknown = sorted(os.path.basename(path) for path in imported_set - discovered)
        fail(f"app source/import drift: unimported={missing} missing={unknown}")
    return imported


def merge_name(target, owners, app_id, name, owner):
    previous = target.get(app_id)
    if previous is not None and previous != name:
        fail(
            f"app id {app_id} conflicts: {previous!r} from {owners[app_id]} "
            f"vs {name!r} from {owner}"
        )
    target[app_id] = name
    owners.setdefault(app_id, owner)


def build_manifest():
    kernel_path = os.path.join(KERNEL_ROOT, "src", "kernel.zl")
    registry_path = os.path.join(KERNEL_ROOT, "apps", "apps_registry.zl")
    kernel = read(kernel_path)
    app_paths = imported_app_paths(kernel_path, kernel)
    if registry_path not in app_paths:
        fail("kernel.zl does not import apps_registry.zl")
    slice_paths = [path for path in app_paths if path != registry_path]
    paths = [kernel_path] + app_paths
    sources = {path: read(path) for path in paths}
    consts = constants(sources)

    names = named_arms(function_body(sources[kernel_path], "app_name"), consts, "app_name")
    name_owners = {app_id: "kernel.zl::app_name" for app_id in names}
    registry_names = named_arms(function_body(sources[registry_path], "reg_name"), consts, "reg_name")
    registry_owners = {app_id: "apps_registry.zl::reg_name" for app_id in registry_names}
    for path in slice_paths:
        text = sources[path]
        for function in FUNCTION_HEAD_RE.findall(text):
            if function.endswith("_name"):
                body = function_body(text, function)
                for app_id, name in named_arms(body, consts, function).items():
                    owner = f"{os.path.basename(path)}::{function}"
                    merge_name(registry_names, registry_owners, app_id, name, owner)

    dense = dense_registry(sources[registry_path], consts)
    for app_id in dense:
        if app_id not in registry_names:
            fail(f"dense catalog id {app_id} has no name")
        merge_name(names, name_owners, app_id, registry_names[app_id], registry_owners[app_id])

    catalog_id = consts.get("APP_CATALOG")
    if catalog_id is None:
        fail("APP_CATALOG missing")
    merge_name(names, name_owners, catalog_id, "All Applications", "kernel.zl::APP_CATALOG")
    kinds = reference_kinds(read(REFERENCE))
    dense_index = {app_id: index for index, app_id in enumerate(dense)}

    entries = []
    for app_id, name in sorted(names.items()):
        if app_id == catalog_id:
            kind = "shell"
            family = "catalogue"
            route = "catalogue-surface-static"
        elif app_id in dense_index:
            kind = kinds.get(name)
            if kind is None:
                fail(f"registry app {name!r} is absent from reference category inventory")
            family = "registry"
            route = "catalog-static"
        else:
            kind = kinds.get(name, "shell" if name in {"Menu", "Run"} else "application")
            family = "legacy"
            route = "legacy-static"
        entries.append(
            {
                "id": app_id,
                "name": name,
                "kind": kind,
                "source_family": family,
                "identity_source": name_owners[app_id],
                "static_route": route,
                "catalog_index": dense_index.get(app_id),
                "migration_phase": "MP-12",
                "evidence": ["S", "R-build"],
            }
        )

    manifest = {
        "schema": "zlos.application-identity-manifest.v1",
        "source_sha256": source_digest(paths + [REFERENCE]),
        "source_files": [os.path.basename(path) for path in paths] + [os.path.relpath(REFERENCE, ROOT)],
        "build_routes": ["BIOS-multiboot32", "multiboot64", "native-UEFI64", "raw-BIOS-loader32"],
        "manifest_delivery": "host JSON plus generated boot-embedded schema/count/digest receipt; runtime proof is recorded separately",
        "evidence_ceiling": "static source and build inclusion only; not launch, runtime, QEMU, or physical proof",
        "generated_files": ["app-manifest.json", "app_manifest_embed.zl"],
        "named_implementation_count": len(entries) - 1,
        "catalogue_surface_count": 1,
        "entries": entries,
    }
    validate_manifest(manifest, dense, catalog_id)
    return manifest


def validate_manifest(manifest, dense, catalog_id):
    entries = manifest.get("entries", [])
    ids = [entry.get("id") for entry in entries]
    names = [entry.get("name") for entry in entries]
    if len(entries) != 62:
        fail(f"manifest has {len(entries)} entries, expected 62")
    if len(set(ids)) != len(ids):
        fail("manifest has duplicate app ids")
    if len(set(names)) != len(names):
        fail("manifest has duplicate app names")
    if manifest.get("named_implementation_count") != 61:
        fail("named implementation count is not 61")
    source_files = manifest.get("source_files")
    if not isinstance(source_files, list) or "kernel.zl" not in source_files or "apps_registry.zl" not in source_files:
        fail("manifest source_files does not name the kernel and app registry")
    if manifest.get("build_routes") != ["BIOS-multiboot32", "multiboot64", "native-UEFI64", "raw-BIOS-loader32"]:
        fail("manifest build_routes is incomplete or reordered")
    if manifest.get("generated_files") != ["app-manifest.json", "app_manifest_embed.zl"]:
        fail("manifest generated-file inventory is incomplete")
    if ids.count(catalog_id) != 1 or names.count("All Applications") != 1:
        fail("All Applications catalogue identity is missing or duplicated")
    registry_ids = {entry["id"] for entry in entries if entry.get("source_family") == "registry"}
    if registry_ids != set(dense):
        fail(f"registry entries differ from dense ids: missing={sorted(set(dense)-registry_ids)} extra={sorted(registry_ids-set(dense))}")
    game_count = sum(entry.get("kind") == "game" for entry in entries)
    if game_count != 24:
        fail(f"manifest has {game_count} games, expected 24")
    required = {"id", "name", "kind", "source_family", "identity_source", "static_route", "catalog_index", "migration_phase", "evidence"}
    for entry in entries:
        missing = required - set(entry)
        if missing:
            fail(f"entry {entry.get('id')} missing fields: {sorted(missing)}")


def serialized(manifest):
    return json.dumps(manifest, indent=2, sort_keys=True, ensure_ascii=False) + "\n"


def embedded(serialized_manifest):
    digest = hashlib.sha256(serialized_manifest.encode("utf-8")).hexdigest()
    return (
        "# Generated by gen-app-manifest.py. Do not edit.\n"
        "# This receipt is compiled into every kernel build route.\n"
        "fn app_manifest_report() {\n"
        f'    print("  app-manifest: schema=1 entries=62 sha256={digest}")\n'
        "    return 0\n"
        "}\n"
    )


def run_selftest(manifest):
    mutations = []
    missing = json.loads(json.dumps(manifest))
    missing["entries"].pop()
    mutations.append(("missing-entry", missing))
    duplicate_id = json.loads(json.dumps(manifest))
    duplicate_id["entries"][1]["id"] = duplicate_id["entries"][0]["id"]
    mutations.append(("duplicate-id", duplicate_id))
    duplicate_name = json.loads(json.dumps(manifest))
    duplicate_name["entries"][1]["name"] = duplicate_name["entries"][0]["name"]
    mutations.append(("duplicate-name", duplicate_name))
    missing_field = json.loads(json.dumps(manifest))
    del missing_field["entries"][0]["static_route"]
    mutations.append(("missing-field", missing_field))
    missing_source = json.loads(json.dumps(manifest))
    missing_source["source_files"].remove("apps_registry.zl")
    mutations.append(("missing-source", missing_source))
    caught = []
    dense = [entry["id"] for entry in manifest["entries"] if entry["source_family"] == "registry"]
    catalog_id = next(entry["id"] for entry in manifest["entries"] if entry["name"] == "All Applications")
    for name, mutant in mutations:
        try:
            validate_manifest(mutant, dense, catalog_id)
        except ValueError:
            caught.append(name)
        else:
            fail(f"selftest mutation escaped: {name}")
    conflict_names = {1: "one"}
    conflict_owners = {1: "first"}
    try:
        merge_name(conflict_names, conflict_owners, 1, "other", "second")
    except ValueError:
        caught.append("identity-conflict")
    else:
        fail("selftest mutation escaped: identity-conflict")
    print("app-manifest selftest: caught " + ", ".join(caught))


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args(argv)
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        manifest = build_manifest()
        expected = serialized(manifest)
        expected_embed = embedded(expected)
        if args.write:
            with open(OUTPUT, "w", encoding="utf-8") as handle:
                handle.write(expected)
            with open(EMBED, "w", encoding="utf-8") as handle:
                handle.write(expected_embed)
        else:
            actual = read(OUTPUT)
            if actual != expected:
                fail("app-manifest.json is stale; regenerate with --write and inspect the diff")
            actual_embed = read(EMBED)
            if actual_embed != expected_embed:
                fail("app_manifest_embed.zl is stale; regenerate with --write and inspect the diff")
        if args.selftest:
            run_selftest(manifest)
    except ValueError as error:
        print(f"app-manifest: FAIL: {error}", file=sys.stderr)
        return 1
    print("app-manifest: PASS: 61 named implementations + 1 catalogue surface, 24 games")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
