#!/usr/bin/env python3
"""Lock every host command and firmware blob used by current zlOS build/boot gates."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
METADATA = KERNEL_ROOT / "metadata"
OUTPUT = METADATA / "dependency-lock.json"
BUILD_IDENTITY = METADATA / "build-identity.json"

COMMANDS = (
    ("apt-cache", ("--version",)),
    ("apt-get", ("--version",)),
    ("dpkg-deb", ("--version",)),
    ("dpkg-query", ("--version",)),
    ("gcc", ("--version",)),
    ("clang", ("--version",)),
    ("ld", ("--version",)),
    ("lld-link", ("--version",)),
    ("objcopy", ("--version",)),
    ("nasm", ("-v",)),
    ("grub-mkrescue", ("--version",)),
    ("xorriso", ("-version",)),
    ("sgdisk", ("--version",)),
    ("mkfs.vfat", ("--help",)),
    ("mmd", ("-V",)),
    ("mcopy", ("-V",)),
    ("mdir", ("-V",)),
    ("qemu-system-i386", ("--version",)),
    ("qemu-system-x86_64", ("--version",)),
    ("ldd", ("--version",)),
)

FIRMWARE = (
    Path("/usr/share/OVMF/OVMF_CODE_4M.fd"),
    Path("/usr/share/OVMF/OVMF_VARS_4M.fd"),
)

PACKAGE_FIELDS = (
    "${binary:Package}\t${Version}\t${Architecture}\t${Status}\t"
    "${source:Package}\t${source:Version}\t${Depends}\t${Pre-Depends}\t${Provides}\n"
)
PACKAGE_CACHE: dict[str, dict] = {}


def sha256(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def version_line(path: Path, argv: tuple[str, ...]) -> str:
    completed = subprocess.run(
        [str(path), *argv], text=True, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, timeout=15, check=False,
    )
    lines = [line.strip() for line in completed.stdout.splitlines() if line.strip()]
    if not lines:
        raise ValueError(f"{path.name}: version command produced no output")
    return lines[0]


def command_output(argv: list[str]) -> str:
    completed = subprocess.run(
        argv, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        timeout=30, check=False,
    )
    if completed.returncode != 0:
        raise ValueError(f"command failed ({completed.returncode}): {' '.join(argv)}: {completed.stderr.strip()}")
    return completed.stdout


def package_owners(paths: list[Path]) -> dict[str, str]:
    exact = {str(path.resolve()): path.resolve() for path in paths}
    output = command_output(["dpkg-query", "-S", *sorted(exact)])
    matches: dict[str, list[str]] = {path: [] for path in exact}
    for line in output.splitlines():
        if ": " not in line:
            continue
        package, owned = line.split(": ", 1)
        resolved = str(Path(owned).resolve())
        if resolved in matches:
            matches[resolved].append(package)
    missing = sorted(path for path, owners in matches.items() if not owners)
    if missing:
        raise ValueError(f"no exact package owner for: {missing!r}")
    return {path: sorted(owners)[0] for path, owners in matches.items()}


def package_row(name: str) -> dict:
    if not PACKAGE_CACHE:
        output = command_output(["dpkg-query", "-W", "-f=" + PACKAGE_FIELDS])
        for line in output.splitlines():
            fields = line.split("\t")
            if len(fields) != 9:
                raise ValueError(f"unexpected installed package metadata: {fields!r}")
            binary, version, architecture, status, source, source_version, depends, pre_depends, provides = fields
            # dpkg keeps packages installed while deferred trigger processing
            # reports "triggers-pending" or "triggers-awaited".
            if not status.startswith("install ok "):
                continue
            row = {
                "name": binary,
                "version": version,
                "architecture": architecture,
                "status": status,
                "source_package": source or binary.split(":", 1)[0],
                "source_version": source_version or version,
                "depends_raw": depends,
                "pre_depends_raw": pre_depends,
                "provides_raw": provides,
            }
            PACKAGE_CACHE[binary] = row
            PACKAGE_CACHE.setdefault(binary.split(":", 1)[0], row)
        for row in list(PACKAGE_CACHE.values()):
            for provided in row["provides_raw"].split(","):
                provided_name = re.sub(r"\s*\([^)]*\)", "", provided).strip()
                if provided_name:
                    PACKAGE_CACHE.setdefault(provided_name, row)
    cached = PACKAGE_CACHE.get(name)
    if cached is None:
        raise ValueError(f"package is not installed: {name}")
    row = copy.deepcopy(cached)
    binary = row["name"]
    version = row["version"]
    architecture = row["architecture"]
    archive_name = f"{binary.split(':', 1)[0]}_{version.replace(':', '%3a')}_{architecture}.deb"
    archive = Path("/var/cache/apt/archives") / archive_name
    row.update({
        "binary_archive": str(archive),
        "binary_archive_retained": archive.is_file(),
        "binary_archive_sha256": sha256(archive) if archive.is_file() else None,
        "source_archive_retained": False,
    })
    return row


def installed_choice(expression: str) -> str | None:
    for alternative in expression.split("|"):
        candidate = re.sub(r"\s*\([^)]*\)", "", alternative).strip()
        candidate = re.sub(r"\[[^]]*\]", "", candidate).strip()
        candidate = re.sub(r"<[^>]*>", "", candidate).strip()
        if not candidate:
            continue
        try:
            return package_row(candidate)["name"]
        except ValueError:
            continue
    return None


def package_dependencies(row: dict) -> list[str]:
    raw = ",".join(value for value in (row["pre_depends_raw"], row["depends_raw"]) if value)
    dependencies = []
    for expression in raw.split(","):
        if not expression.strip():
            continue
        chosen = installed_choice(expression)
        if chosen is None:
            raise ValueError(f"{row['name']}: no installed dependency satisfies {expression!r}")
        if chosen != row["name"] and chosen not in dependencies:
            dependencies.append(chosen)
    return sorted(dependencies)


def runtime_files(path: Path) -> list[Path]:
    result = {path.resolve()}
    prefix = path.read_bytes()[:256]
    if prefix.startswith(b"#!"):
        first = prefix.splitlines()[0][2:].decode("utf-8", errors="strict").strip().split()
        if first:
            interpreter = Path(first[0]).resolve()
            if interpreter.is_file():
                result.add(interpreter)
    completed = subprocess.run(
        ["ldd", str(path)], text=True, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, timeout=30, check=False,
    )
    if completed.returncode == 0:
        for line in completed.stdout.splitlines():
            match = re.search(r"=>\s+(/\S+)", line) or re.match(r"\s*(/\S+)", line)
            if match:
                candidate = Path(match.group(1)).resolve()
                if candidate.is_file():
                    result.add(candidate)
    return sorted(result, key=str)


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.host-dependency-lock.v3" or value.get("result") != "PASS_WITH_OPEN_SUPPLY_GAPS":
        raise ValueError("wrong dependency-lock schema/result")
    commands = value.get("commands")
    expected = [name for name, _ in COMMANDS]
    if not isinstance(commands, list) or [row.get("name") for row in commands] != expected:
        raise ValueError("dependency command set/order drift")
    for row in commands:
        if not Path(row.get("path", "")).is_absolute():
            raise ValueError(f"{row.get('name')}: command path is not absolute")
        if Path(row.get("path", "")).name != row.get("name"):
            raise ValueError(f"{row.get('name')}: invocation alias was lost")
        if not Path(row.get("resolved_path", "")).is_absolute():
            raise ValueError(f"{row.get('name')}: resolved command path is not absolute")
        if len(row.get("sha256", "")) != 64 or not row.get("version"):
            raise ValueError(f"{row.get('name')}: missing binary/version identity")
    firmware = value.get("firmware")
    if not isinstance(firmware, list) or [row.get("path") for row in firmware] != [str(p) for p in FIRMWARE]:
        raise ValueError("firmware set/order drift")
    for row in firmware:
        if row.get("bytes", 0) <= 0 or len(row.get("sha256", "")) != 64:
            raise ValueError(f"{row.get('path')}: missing firmware identity")
    runtime = value.get("runtime_files", [])
    if not runtime or len({row.get("path") for row in runtime}) != len(runtime):
        raise ValueError("runtime-file closure is empty or duplicated")
    packages = value.get("packages", [])
    package_names = [row.get("name") for row in packages]
    if not packages or package_names != sorted(set(package_names)):
        raise ValueError("package closure is empty, duplicated or unordered")
    known = set(package_names)
    for row in packages:
        if not row.get("version") or not row.get("source_package") or not row.get("source_version"):
            raise ValueError(f"{row.get('name')}: incomplete package/source identity")
        if any(dependency not in known for dependency in row.get("resolved_dependencies", [])):
            raise ValueError(f"{row.get('name')}: unresolved package dependency")
        if row.get("source_archive_retained") is not False:
            raise ValueError(f"{row.get('name')}: invented source-archive custody")
        if row.get("reverse_dependencies") != sorted(set(row.get("reverse_dependencies", []))):
            raise ValueError(f"{row.get('name')}: reverse dependency drift")
        if any(parent not in known for parent in row["reverse_dependencies"]):
            raise ValueError(f"{row.get('name')}: unknown reverse dependency")
    for row in commands + firmware + runtime:
        if row.get("package") not in known:
            raise ValueError(f"{row.get('path', row.get('name'))}: package relationship is unresolved")
    source_packages = value.get("source_packages", [])
    source_keys = [(row.get("name"), row.get("version")) for row in source_packages]
    if not source_packages or source_keys != sorted(set(source_keys)):
        raise ValueError("source-package closure is empty, duplicated or unordered")
    expected_source_binaries: dict[tuple[str, str], list[str]] = {}
    for row in packages:
        key = (row["source_package"], row["source_version"])
        expected_source_binaries.setdefault(key, []).append(row["name"])
    for binaries in expected_source_binaries.values():
        binaries.sort()
    measured_source_binaries = {
        (row["name"], row["version"]): row.get("binary_packages")
        for row in source_packages
    }
    if measured_source_binaries != expected_source_binaries:
        raise ValueError("binary-to-source package relationships drifted")
    seed_packages = value.get("closure", {}).get("seed_packages", [])
    if seed_packages != sorted(set(seed_packages)) or any(name not in known for name in seed_packages):
        raise ValueError("seed-package closure is missing or invalid")
    reachable = set(seed_packages)
    pending = list(seed_packages)
    by_name = {row["name"]: row for row in packages}
    while pending:
        name = pending.pop()
        for dependency in by_name[name]["resolved_dependencies"]:
            if dependency not in reachable:
                reachable.add(dependency)
                pending.append(dependency)
    if reachable != known:
        raise ValueError("package closure contains an unreachable package")
    closure = value.get("closure", {})
    if closure.get("all_runtime_files_owned") is not True \
            or closure.get("all_package_dependencies_resolved") is not True \
            or closure.get("all_binary_source_relationships_resolved") is not True \
            or closure.get("all_source_archives_retained") is not False:
        raise ValueError("dependency closure hides its supply gaps")
    if len(value.get("build_identity", "")) != 64:
        raise ValueError("missing build identity")
    if len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("missing generator identity")


def build() -> dict:
    identity = load(BUILD_IDENTITY)
    commands = []
    all_runtime: dict[str, dict] = {}

    def add_runtime(path: Path, reason: str) -> str:
        resolved = path.resolve()
        key = str(resolved)
        row = all_runtime.setdefault(key, {
            "path": key,
            "bytes": resolved.stat().st_size,
            "sha256": sha256(resolved),
            "reasons": [],
        })
        if reason not in row["reasons"]:
            row["reasons"].append(reason)
        return key

    for name, argv in COMMANDS:
        found = shutil.which(name)
        if not found:
            raise ValueError(f"missing dependency command: {name}")
        invoked = Path(found).absolute()
        resolved = invoked.resolve()
        dependencies = [add_runtime(path, f"command:{name}") for path in runtime_files(resolved)]
        commands.append({
            "name": name,
            "path": str(invoked),
            "resolved_path": str(resolved),
            "bytes": resolved.stat().st_size,
            "sha256": sha256(resolved),
            "version_argv": [name, *argv],
            "version": version_line(invoked, argv),
            "runtime_files": dependencies,
        })
    firmware = []
    for configured in FIRMWARE:
        if not configured.is_file():
            raise ValueError(f"missing firmware dependency: {configured}")
        resolved = configured.resolve()
        firmware.append({
            "path": str(configured),
            "resolved_path": str(resolved),
            "bytes": resolved.stat().st_size,
            "sha256": sha256(resolved),
        })
    python_path = Path(sys.executable).resolve()
    for path in runtime_files(python_path):
        add_runtime(path, "python-generator-runtime")

    ownership_paths = [Path(path) for path in all_runtime]
    ownership_paths.extend(Path(row["resolved_path"]) for row in firmware)
    owners = package_owners(ownership_paths)
    for row in all_runtime.values():
        row["package"] = owners[row["path"]]
    for row in commands:
        row["package"] = owners[str(Path(row["resolved_path"]).resolve())]
    for row in firmware:
        row["package"] = owners[str(Path(row["resolved_path"]).resolve())]
    canonical_owner = {
        path: package_row(owner)["name"] for path, owner in owners.items()
    }
    for row in all_runtime.values():
        row["package"] = canonical_owner[row["path"]]
    for row in commands:
        row["package"] = canonical_owner[str(Path(row["resolved_path"]).resolve())]
    for row in firmware:
        row["package"] = canonical_owner[str(Path(row["resolved_path"]).resolve())]
    seed_packages = set(canonical_owner.values())

    package_map: dict[str, dict] = {}
    pending = sorted(seed_packages)
    while pending:
        requested = pending.pop(0)
        measured = package_row(requested)
        name = measured["name"]
        if name in package_map:
            continue
        dependencies = package_dependencies(measured)
        measured["resolved_dependencies"] = dependencies
        package_map[name] = measured
        pending.extend(dep for dep in dependencies if dep not in package_map and dep not in pending)
        pending.sort()
    reverse_dependencies: dict[str, list[str]] = {name: [] for name in package_map}
    for name, row in package_map.items():
        for dependency in row["resolved_dependencies"]:
            reverse_dependencies[dependency].append(name)
    for name, row in package_map.items():
        row["reverse_dependencies"] = sorted(reverse_dependencies[name])
    packages = [package_map[name] for name in sorted(package_map)]
    source_map: dict[tuple[str, str], list[str]] = {}
    for row in packages:
        key = (row["source_package"], row["source_version"])
        source_map.setdefault(key, []).append(row["name"])
    source_packages = [
        {
            "name": name,
            "version": version,
            "binary_packages": sorted(source_map[(name, version)]),
            "archive_files": [],
            "archives_retained": False,
        }
        for name, version in sorted(source_map)
    ]
    binary_archives = sum(bool(row["binary_archive_retained"]) for row in packages)
    value = {
        "schema": "zlos.host-dependency-lock.v3",
        "result": "PASS_WITH_OPEN_SUPPLY_GAPS",
        "build_identity": identity["identity_sha256"],
        "host": {
            "platform": platform.platform(),
            "python": platform.python_version(),
            "python_executable": str(Path(sys.executable).resolve()),
            "python_sha256": sha256(Path(sys.executable).resolve()),
        },
        "commands": commands,
        "firmware": firmware,
        "runtime_files": sorted(all_runtime.values(), key=lambda row: row["path"]),
        "packages": packages,
        "source_packages": source_packages,
        "closure": {
            "seed_packages": sorted(seed_packages),
            "seed_package_count": len(seed_packages),
            "transitive_packages": len(packages),
            "source_packages": len(source_packages),
            "runtime_files": len(all_runtime),
            "binary_archives_retained": binary_archives,
            "source_archives_retained": 0,
            "all_runtime_files_owned": True,
            "all_package_dependencies_resolved": True,
            "all_binary_source_relationships_resolved": True,
            "all_binary_archives_retained": binary_archives == len(packages),
            "all_source_archives_retained": False,
            "offline_rebuild_proved": False,
        },
        "generator": {
            "path": "kernel/tools/generators/gen-dependency-lock.py",
            "sha256": sha256(Path(__file__).resolve()),
        },
        "evidence_ceiling": (
            "exact installed binary, runtime-library, firmware and transitive package SBOM; "
            "not an offline rebuild, source archive, signature or supply-chain attestation"
        ),
        "weakest_link": (
            "zero package source archives are retained, so package identities cannot yet reconstruct the toolchain offline"
        ),
    }
    validate(value)
    return value


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["commands"].pop(4)
    mutations["missing-command"] = missing
    drift = copy.deepcopy(value)
    drift["commands"][0]["sha256"] = "0" * 64
    mutations["binary-drift"] = drift
    alias = copy.deepcopy(value)
    lld = next(row for row in alias["commands"] if row["name"] == "lld-link")
    lld["path"] = lld["resolved_path"]
    mutations["wrong-invocation-alias"] = alias
    firmware = copy.deepcopy(value)
    firmware["firmware"][0]["bytes"] = 0
    mutations["missing-firmware-identity"] = firmware
    package = copy.deepcopy(value)
    package["packages"].pop()
    mutations["missing-package"] = package
    dependency = copy.deepcopy(value)
    dependency["packages"][0]["resolved_dependencies"].append("not-installed")
    mutations["unresolved-package-dependency"] = dependency
    custody = copy.deepcopy(value)
    custody["packages"][0]["source_archive_retained"] = True
    mutations["invented-source-archive"] = custody
    source = copy.deepcopy(value)
    source["source_packages"].pop()
    mutations["missing-source-relationship"] = source
    reverse = copy.deepcopy(value)
    reverse["packages"][0]["reverse_dependencies"].append("not-installed")
    mutations["unknown-reverse-dependency"] = reverse
    root = copy.deepcopy(value)
    root["closure"]["seed_packages"].append("not-installed")
    mutations["unknown-seed-package"] = root
    identity = copy.deepcopy(value)
    identity["build_identity"] = "short"
    mutations["missing-build-identity"] = identity
    caught = []
    for name, mutated in mutations.items():
        try:
            validate(mutated)
            if mutated != value:
                raise ValueError("lock differs from current measured dependencies")
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"dependency-lock selftest mutation escaped: {name}")
    print("dependency-lock selftest: caught " + ", ".join(caught))


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
                raise ValueError("dependency-lock.json is missing or stale")
        print(
            f"dependency-lock: PASS_WITH_OPEN_SUPPLY_GAPS: {len(value['commands'])} commands, "
            f"{len(value['firmware'])} firmware blobs, {len(value['runtime_files'])} runtime files, "
            f"{len(value['packages'])} transitive packages, {len(value['source_packages'])} source packages"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.SubprocessError) as error:
        print(f"dependency-lock: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
