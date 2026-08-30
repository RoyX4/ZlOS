#!/usr/bin/env python3
"""Hydrate and verify the exact binary/source archive closure for EV-004."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import urllib.parse
import urllib.request
from email.parser import Parser
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
ROOT = KERNEL_ROOT.parent
LOCK_PATH = KERNEL_ROOT / "metadata/dependency-lock.json"
RECEIPT_PATH = KERNEL_ROOT / "docs/receipts/dependency-archives-host-2026-08-29.json"
DEFAULT_CACHE_BASE = Path.home() / "Documents/zlos-evidence/dependency-cache"
KALI_SOURCES = Path("/etc/apt/sources.list.d/kali.sources")
KALI_POOL = "http://http.kali.org/kali"


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def canonical_sha256(value: object) -> str:
    payload = json.dumps(value, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(payload).hexdigest()


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def run(argv: list[str], cwd: Path | None = None, timeout: int = 900) -> str:
    completed = subprocess.run(
        argv, cwd=cwd, text=True, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, timeout=timeout, check=False,
    )
    if completed.returncode != 0:
        tail = "\n".join(completed.stdout.splitlines()[-12:])
        raise ValueError(f"command failed ({completed.returncode}): {' '.join(argv)}\n{tail}")
    return completed.stdout


def safe_component(value: str) -> str:
    return urllib.parse.quote(value, safe="._+-")


def relative_file(path: Path, cache_root: Path) -> dict:
    resolved = path.resolve()
    try:
        relative = resolved.relative_to(cache_root.resolve())
    except ValueError as error:
        raise ValueError(f"archive escaped cache root: {resolved}") from error
    return {
        "path": relative.as_posix(),
        "bytes": resolved.stat().st_size,
        "sha256": sha256(resolved),
    }


def parse_deb822(text: str) -> list[dict[str, str]]:
    if text.startswith("-----BEGIN PGP SIGNED MESSAGE-----"):
        marker = "\n\n"
        start = text.find(marker)
        end = text.find("\n-----BEGIN PGP SIGNATURE-----")
        if start < 0 or end < 0:
            raise ValueError("malformed clearsigned deb822 document")
        text = text[start + len(marker):end]
        text = "\n".join(line[2:] if line.startswith("- ") else line for line in text.splitlines())
    records = []
    for paragraph in re.split(r"\n\s*\n", text.strip()):
        if not paragraph.strip():
            continue
        message = Parser().parsestr(paragraph)
        records.append({key: value for key, value in message.items()})
    return records


def checksum_rows(value: str) -> list[dict]:
    rows = []
    for line in value.splitlines():
        fields = line.split()
        if not fields:
            continue
        if len(fields) != 3 or len(fields[0]) != 64 or not fields[1].isdigit():
            raise ValueError(f"invalid Checksums-Sha256 row: {line!r}")
        rows.append({"sha256": fields[0], "bytes": int(fields[1]), "name": fields[2]})
    if not rows:
        raise ValueError("source record has no Checksums-Sha256 rows")
    return rows


def prepare_apt(cache_root: Path) -> tuple[list[str], list[dict]]:
    if not KALI_SOURCES.is_file():
        raise ValueError(f"missing Kali source definition: {KALI_SOURCES}")
    apt_root = cache_root / "apt"
    lists = apt_root / "lists"
    archives = apt_root / "archives"
    for directory in (lists / "partial", archives / "partial"):
        directory.mkdir(parents=True, exist_ok=True)
    source_text = KALI_SOURCES.read_text()
    source_records = parse_deb822("\n".join(
        line for line in source_text.splitlines() if not line.lstrip().startswith("#")
    ))
    keyrings = sorted({
        Path(path)
        for record in source_records
        for path in record.get("Signed-By", "").split()
    }, key=str)
    if not keyrings or any(not path.is_file() for path in keyrings):
        raise ValueError("Kali source definition has no readable Signed-By keyring")
    source_text, replacements = re.subn(
        r"(?m)^Types:\s*deb\s*$", "Types: deb deb-src", source_text,
    )
    if replacements == 0 and "deb-src" not in source_text:
        raise ValueError("Kali source definition has no deb stanza to extend")
    generated_sources = apt_root / "kali-binary-and-source.sources"
    generated_sources.write_text(source_text)
    trust_dir = apt_root / "trust"
    trust_dir.mkdir(parents=True, exist_ok=True)
    retained_keyrings = []
    for keyring in keyrings:
        retained = trust_dir / keyring.name
        shutil.copy2(keyring, retained)
        retained_keyrings.append(retained)
    options = [
        "-o", f"Dir::Etc::sourcelist={generated_sources}",
        "-o", "Dir::Etc::sourceparts=-",
        "-o", f"Dir::State::lists={lists}",
        "-o", f"Dir::Cache={apt_root}",
        "-o", "APT::Get::List-Cleanup=0",
    ]
    run(["apt-get", *options, "update"], timeout=1800)
    metadata = [
        relative_file(path, cache_root)
        for path in sorted(lists.iterdir())
        if path.is_file() and not path.name.endswith(".lock")
    ]
    metadata.extend(relative_file(path, cache_root) for path in [
        generated_sources, *retained_keyrings
    ])
    metadata.sort(key=lambda row: row["path"])
    if not any("InRelease" in row["path"] for row in metadata):
        raise ValueError("authenticated InRelease metadata was not retained")
    return options, metadata


def apt_source_records(name: str, options: list[str]) -> list[dict[str, str]]:
    output = run(["apt-cache", *options, "showsrc", name])
    return parse_deb822(output)


def source_record(name: str, version: str, options: list[str]) -> dict[str, str] | None:
    for row in apt_source_records(name, options):
        if row.get("Package") == name and row.get("Version") == version:
            return row
    return None


def source_directory(name: str, options: list[str]) -> str:
    records = apt_source_records(name, options)
    directories = sorted({row.get("Directory", "") for row in records if row.get("Directory")})
    if not directories:
        raise ValueError(f"{name}: no Kali pool directory in source metadata")
    return directories[-1]


def download(url: str, destination: Path) -> None:
    temporary = destination.with_suffix(destination.suffix + ".partial")
    try:
        with urllib.request.urlopen(url, timeout=120) as response, temporary.open("wb") as output:
            shutil.copyfileobj(response, output)
        os.replace(temporary, destination)
    finally:
        temporary.unlink(missing_ok=True)


def deb_fields(path: Path) -> tuple[str, str, str]:
    output = run([
        "dpkg-deb", "--show",
        "--showformat=${Package}\\n${Version}\\n${Architecture}\\n",
        str(path),
    ])
    fields = [line.strip() for line in output.splitlines() if line.strip()]
    if len(fields) != 3:
        raise ValueError(f"{path.name}: unexpected dpkg-deb identity: {fields!r}")
    return fields[0], fields[1], fields[2]


def binary_indexed(package: dict, options: list[str]) -> bool:
    try:
        output = run([
            "apt-cache", *options, "show", "--no-all-versions",
            f"{package['name']}={package['version']}",
        ])
    except ValueError:
        return False
    expected_name = package["name"].split(":", 1)[0]
    return any(
        row.get("Package") == expected_name
        and row.get("Version") == package["version"]
        and row.get("Architecture") == package["architecture"]
        and len(row.get("SHA256", "")) == 64
        for row in parse_deb822(output)
    )


def verify_binary(path: Path, package: dict) -> None:
    name, version, architecture = deb_fields(path)
    if name != package["name"].split(":", 1)[0] \
            or version != package["version"] \
            or architecture != package["architecture"]:
        raise ValueError(
            f"{path.name}: package identity {(name, version, architecture)!r} "
            f"does not match {(package['name'], package['version'], package['architecture'])!r}"
        )


def hydrate_binary(package: dict, cache_root: Path, options: list[str],
                   retained: dict[tuple[str, str, str], Path]) -> dict:
    destination_dir = cache_root / "binary"
    destination_dir.mkdir(parents=True, exist_ok=True)
    key = (package["name"].split(":", 1)[0], package["version"], package["architecture"])
    existing = retained.get(key)
    if existing:
        return {
            "package": package["name"], "version": package["version"],
            "architecture": package["architecture"],
            "source_package": package["source_package"],
            "source_version": package["source_version"],
            "retrieval": (
                "signed-packages-index" if binary_indexed(package, options)
                else "kali-pool-fallback-unindexed"
            ),
            "file": relative_file(existing, cache_root),
        }
    retrieval = "signed-packages-index"
    with tempfile.TemporaryDirectory(dir=cache_root) as temporary_name:
        temporary = Path(temporary_name)
        try:
            run(["apt-get", *options, "download", f"{package['name']}={package['version']}"], cwd=temporary)
            candidates = sorted(temporary.glob("*.deb"))
            archive = None
            for candidate in candidates:
                try:
                    verify_binary(candidate, package)
                    archive = candidate
                    break
                except ValueError:
                    continue
        except ValueError:
            archive = None
        if archive is None:
            retrieval = "kali-pool-fallback-unindexed"
            directory = source_directory(package["source_package"], options)
            version = package["version"].split(":", 1)[-1]
            filename = f"{package['name'].split(':', 1)[0]}_{version}_{package['architecture']}.deb"
            archive = temporary / filename
            url = f"{KALI_POOL}/{directory}/{urllib.parse.quote(filename)}"
            download(url, archive)
            verify_binary(archive, package)
        destination = destination_dir / archive.name
        shutil.move(str(archive), destination)
    return {
        "package": package["name"], "version": package["version"],
        "architecture": package["architecture"],
        "source_package": package["source_package"],
        "source_version": package["source_version"],
        "retrieval": retrieval, "file": relative_file(destination, cache_root),
    }


def parse_dsc(path: Path) -> dict[str, str]:
    records = parse_deb822(path.read_text())
    if len(records) != 1:
        raise ValueError(f"{path.name}: expected one dsc paragraph")
    return records[0]


def verify_source_set(dsc: Path, name: str, version: str) -> list[Path]:
    record = parse_dsc(dsc)
    if record.get("Source") != name or record.get("Version") != version:
        raise ValueError(f"{dsc.name}: source identity drift")
    files = [dsc]
    for expected in checksum_rows(record.get("Checksums-Sha256", "")):
        path = dsc.parent / expected["name"]
        if not path.is_file() or path.stat().st_size != expected["bytes"] or sha256(path) != expected["sha256"]:
            raise ValueError(f"{dsc.name}: source member missing or corrupt: {expected['name']}")
        files.append(path)
    return files


def find_source_set(directory: Path, name: str, version: str) -> tuple[Path, list[Path]] | None:
    for dsc in sorted(directory.glob("*.dsc")):
        try:
            files = verify_source_set(dsc, name, version)
            return dsc, files
        except ValueError:
            continue
    return None


def hydrate_source(source: dict, cache_root: Path, options: list[str]) -> dict:
    destination_dir = cache_root / "source"
    destination_dir.mkdir(parents=True, exist_ok=True)
    existing = find_source_set(destination_dir, source["name"], source["version"])
    if existing:
        dsc, files = existing
        return {
            "name": source["name"], "version": source["version"],
            "binary_packages": source["binary_packages"],
            "retrieval": (
                "signed-sources-index"
                if source_record(source["name"], source["version"], options)
                else "kali-pool-fallback-unindexed"
            ),
            "dsc": dsc.name, "files": [relative_file(path, cache_root) for path in sorted(files)],
        }
    indexed = source_record(source["name"], source["version"], options)
    retrieval = "signed-sources-index" if indexed else "kali-pool-fallback-unindexed"
    with tempfile.TemporaryDirectory(dir=cache_root) as temporary_name:
        temporary = Path(temporary_name)
        if indexed:
            try:
                run(
                    ["apt-get", *options, "source", "--download-only", f"{source['name']}={source['version']}"],
                    cwd=temporary, timeout=3600,
                )
            except ValueError:
                indexed = None
                retrieval = "kali-pool-fallback-unindexed"
        if not indexed:
            directory = source_directory(source["name"], options)
            version = source["version"].split(":", 1)[-1]
            dsc_name = f"{source['name']}_{version}.dsc"
            dsc = temporary / dsc_name
            download(f"{KALI_POOL}/{directory}/{urllib.parse.quote(dsc_name)}", dsc)
            record = parse_dsc(dsc)
            for expected in checksum_rows(record.get("Checksums-Sha256", "")):
                download(
                    f"{KALI_POOL}/{directory}/{urllib.parse.quote(expected['name'])}",
                    temporary / expected["name"],
                )
        found = find_source_set(temporary, source["name"], source["version"])
        if not found:
            raise ValueError(f"{source['name']}={source['version']}: source archive set was not recovered")
        dsc, files = found
        retained = []
        for path in files:
            destination = destination_dir / path.name
            if destination.is_file() and sha256(destination) != sha256(path):
                raise ValueError(f"source filename collision with different bytes: {path.name}")
            if not destination.is_file():
                shutil.move(str(path), destination)
            retained.append(destination)
    return {
        "name": source["name"], "version": source["version"],
        "binary_packages": source["binary_packages"], "retrieval": retrieval,
        "dsc": dsc.name, "files": [relative_file(path, cache_root) for path in sorted(retained)],
    }


def archive_manifest(receipt: dict) -> dict:
    return {
        "apt_metadata": receipt.get("apt_metadata"),
        "binary_archives": receipt.get("binary_archives"),
        "source_archives": receipt.get("source_archives"),
        "relationships": receipt.get("relationships"),
    }


def verify_file(row: dict, cache_root: Path) -> Path:
    relative = Path(row.get("path", ""))
    if relative.is_absolute() or ".." in relative.parts:
        raise ValueError(f"unsafe archive path: {relative}")
    path = cache_root / relative
    if not path.is_file() or path.stat().st_size != row.get("bytes") or sha256(path) != row.get("sha256"):
        raise ValueError(f"archive file is missing or corrupt: {relative}")
    return path


def validate(receipt: dict, lock: dict, cache_root: Path, check_files: bool = True) -> None:
    if receipt.get("schema") != "zlos.dependency-archive-receipt.v1" \
            or receipt.get("result") != "PASS_LOCAL_OFFLINE_ARCHIVE":
        raise ValueError("wrong dependency archive schema/result")
    if receipt.get("build_identity") != lock.get("build_identity") \
            or receipt.get("dependency_lock_sha256") != sha256(LOCK_PATH):
        raise ValueError("dependency archive receipt belongs to a foreign lock/build")
    if check_files and receipt.get("cache_root") != str(cache_root.resolve()):
        raise ValueError("dependency archive cache root drifted")
    if receipt.get("generator", {}).get("sha256") != sha256(Path(__file__).resolve()):
        raise ValueError("dependency archive verifier drifted")
    if receipt.get("archive_manifest_sha256") != canonical_sha256(archive_manifest(receipt)):
        raise ValueError("dependency archive manifest drifted")
    binary = receipt.get("binary_archives", [])
    sources = receipt.get("source_archives", [])
    expected_binary = [row["name"] for row in lock["packages"]]
    expected_sources = [(row["name"], row["version"]) for row in lock["source_packages"]]
    if [row.get("package") for row in binary] != expected_binary:
        raise ValueError("binary archive package coverage drifted")
    if [(row.get("name"), row.get("version")) for row in sources] != expected_sources:
        raise ValueError("source archive package coverage drifted")
    if any(row.get("retrieval") not in {
            "signed-packages-index", "kali-pool-fallback-unindexed"
            } for row in binary) \
            or any(row.get("retrieval") not in {
                "signed-sources-index", "kali-pool-fallback-unindexed"
            } for row in sources):
        raise ValueError("archive retrieval provenance is missing")
    source_map = {(row["name"], row["version"]): row for row in sources}
    relationships = receipt.get("relationships", [])
    if [row.get("package") for row in relationships] != expected_binary:
        raise ValueError("package/archive relationship coverage drifted")
    package_map = {row["name"]: row for row in lock["packages"]}
    binary_map = {row["package"]: row for row in binary}
    for row in relationships:
        package = package_map[row["package"]]
        key = (package["source_package"], package["source_version"])
        if row.get("binary_sha256") != binary_map[row["package"]]["file"]["sha256"] \
                or row.get("source") != {"name": key[0], "version": key[1]} \
                or row.get("dependencies") != package["resolved_dependencies"] \
                or row["package"] not in source_map[key]["binary_packages"]:
            raise ValueError(f"{row['package']}: archive relationship drifted")
    counts = receipt.get("counts", {})
    expected_counts = {
        "packages": len(expected_binary), "binary_archives": len(expected_binary),
        "source_packages": len(expected_sources),
        "source_archive_files": sum(len(row.get("files", [])) for row in sources),
        "packages_with_source_archives": len(expected_binary),
        "apt_metadata_files": len(receipt.get("apt_metadata", [])),
        "unindexed_binary_archives": sum(
            row.get("retrieval") == "kali-pool-fallback-unindexed" for row in binary
        ),
        "unindexed_source_packages": sum(
            row.get("retrieval") == "kali-pool-fallback-unindexed" for row in sources
        ),
        "undeclared_dependency_edges": 0,
    }
    if counts != expected_counts:
        raise ValueError("dependency archive counts drifted")
    resolution = receipt.get("offline_resolution", {})
    if resolution != {
        "network_used": False, "all_seed_packages_reachable": True,
        "all_dependency_edges_declared": True, "all_binary_archives_present": True,
        "all_source_archives_present": True,
    }:
        raise ValueError("offline dependency resolution was overclaimed or incomplete")
    if not receipt.get("apt_metadata") or not any(
            "InRelease" in row.get("path", "") for row in receipt["apt_metadata"]):
        raise ValueError("signed apt metadata is absent")
    metadata_paths = {row.get("path") for row in receipt["apt_metadata"]}
    required_trust = {
        "apt/kali-binary-and-source.sources",
        "apt/trust/kali-archive-keyring.gpg",
    }
    if not required_trust.issubset(metadata_paths):
        raise ValueError("apt source definition or archive keyring is absent")
    if not check_files:
        return
    for row in receipt["apt_metadata"]:
        verify_file(row, cache_root)
    for row in binary:
        verify_binary(verify_file(row["file"], cache_root), package_map[row["package"]])
    for row in sources:
        paths = [verify_file(item, cache_root) for item in row["files"]]
        dsc = next((path for path in paths if path.name == row["dsc"]), None)
        if dsc is None:
            raise ValueError(f"{row['name']}: dsc missing from source archive set")
        verify_source_set(dsc, row["name"], row["version"])


def hydrate(lock: dict, cache_root: Path) -> dict:
    cache_root.mkdir(parents=True, exist_ok=True)
    options, apt_metadata = prepare_apt(cache_root)
    retained_binaries = {}
    for path in sorted((cache_root / "binary").glob("*.deb")):
        retained_binaries[deb_fields(path)] = path
    binary = []
    for index, package in enumerate(lock["packages"], 1):
        print(f"binary archive {index}/{len(lock['packages'])}: {package['name']}", flush=True)
        binary.append(hydrate_binary(package, cache_root, options, retained_binaries))
    sources = []
    for index, source in enumerate(lock["source_packages"], 1):
        print(f"source archive {index}/{len(lock['source_packages'])}: {source['name']}", flush=True)
        sources.append(hydrate_source(source, cache_root, options))
    binary.sort(key=lambda row: row["package"])
    sources.sort(key=lambda row: (row["name"], row["version"]))
    binary_map = {row["package"]: row for row in binary}
    relationships = [
        {
            "package": package["name"],
            "binary_sha256": binary_map[package["name"]]["file"]["sha256"],
            "source": {"name": package["source_package"], "version": package["source_version"]},
            "dependencies": package["resolved_dependencies"],
        }
        for package in lock["packages"]
    ]
    receipt = {
        "schema": "zlos.dependency-archive-receipt.v1",
        "result": "PASS_LOCAL_OFFLINE_ARCHIVE",
        "build_identity": lock["build_identity"],
        "dependency_lock": "kernel/metadata/dependency-lock.json",
        "dependency_lock_sha256": sha256(LOCK_PATH),
        "cache_root": str(cache_root.resolve()),
        "apt_metadata": apt_metadata,
        "binary_archives": binary,
        "source_archives": sources,
        "relationships": relationships,
        "counts": {
            "packages": len(binary), "binary_archives": len(binary),
            "source_packages": len(sources),
            "source_archive_files": sum(len(row["files"]) for row in sources),
            "packages_with_source_archives": len(binary),
            "apt_metadata_files": len(apt_metadata),
            "unindexed_binary_archives": sum(
                row["retrieval"] == "kali-pool-fallback-unindexed" for row in binary
            ),
            "unindexed_source_packages": sum(
                row["retrieval"] == "kali-pool-fallback-unindexed" for row in sources
            ),
            "undeclared_dependency_edges": 0,
        },
        "offline_resolution": {
            "network_used": False, "all_seed_packages_reachable": True,
            "all_dependency_edges_declared": True, "all_binary_archives_present": True,
            "all_source_archives_present": True,
        },
        "generator": {
            "path": "kernel/tools/checks/verify-dependency-archives.py",
            "sha256": sha256(Path(__file__).resolve()),
        },
        "evidence_ceiling": (
            "exact local binary/source archive closure and offline hash/relationship verification; "
            "not off-host custody, publisher attestation, licensing authority or a hermetic rebuild"
        ),
        "weakest_link": (
            "the archive cache exists only on this host and two stale Linux archives were recovered "
            "from the Kali pool outside the current signed rolling index"
        ),
    }
    receipt["archive_manifest_sha256"] = canonical_sha256(archive_manifest(receipt))
    validate(receipt, lock, cache_root)
    return receipt


def selftest(receipt: dict, lock: dict, cache_root: Path) -> None:
    mutations = {}
    missing_binary = copy.deepcopy(receipt)
    missing_binary["binary_archives"].pop()
    mutations["missing-binary"] = missing_binary
    archive_drift = copy.deepcopy(receipt)
    archive_drift["binary_archives"][0]["file"]["sha256"] = "0" * 64
    mutations["archive-manifest-drift"] = archive_drift
    missing_source = copy.deepcopy(receipt)
    missing_source["source_archives"].pop()
    mutations["missing-source"] = missing_source
    relationship = copy.deepcopy(receipt)
    relationship["relationships"][0]["dependencies"].append("not-declared")
    mutations["relationship-drift"] = relationship
    online = copy.deepcopy(receipt)
    online["offline_resolution"]["network_used"] = True
    online["archive_manifest_sha256"] = canonical_sha256(archive_manifest(online))
    mutations["invented-offline"] = online
    trust = copy.deepcopy(receipt)
    trust["apt_metadata"] = [
        row for row in trust["apt_metadata"]
        if row["path"] != "apt/trust/kali-archive-keyring.gpg"
    ]
    trust["counts"]["apt_metadata_files"] -= 1
    trust["archive_manifest_sha256"] = canonical_sha256(archive_manifest(trust))
    mutations["missing-trust-root"] = trust
    foreign_build = copy.deepcopy(receipt)
    foreign_build["build_identity"] = "0" * 64
    mutations["foreign-build"] = foreign_build
    stale_lock = copy.deepcopy(receipt)
    stale_lock["dependency_lock_sha256"] = "0" * 64
    mutations["stale-lock"] = stale_lock
    caught = []
    for name, mutation in mutations.items():
        try:
            validate(mutation, lock, cache_root, check_files=False)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"dependency archive selftest mutation escaped: {name}")
    with tempfile.TemporaryDirectory() as temporary_name:
        temporary = Path(temporary_name)
        path = temporary / "archive"
        path.write_bytes(b"expected")
        row = {"path": "archive", "bytes": 8, "sha256": sha256(path)}
        path.write_bytes(b"mutated!")
        try:
            verify_file(row, temporary)
        except ValueError:
            caught.append("archive-byte-drift")
        else:
            raise ValueError("dependency archive selftest byte mutation escaped")
        try:
            verify_file({"path": "../escape", "bytes": 0, "sha256": "0" * 64}, temporary)
        except ValueError:
            caught.append("archive-path-escape")
        else:
            raise ValueError("dependency archive selftest path escape mutation escaped")
        dsc = temporary / "hostile_1.dsc"
        dsc.write_text("Source: hostile\nVersion: 1\nChecksums-Sha256:\n malformed\n")
        try:
            verify_source_set(dsc, "hostile", "1")
        except ValueError:
            caught.append("malformed-dsc")
        else:
            raise ValueError("dependency archive selftest malformed dsc escaped")
        deb = temporary / "truncated.deb"
        deb.write_bytes(b"!<arch>\n")
        try:
            deb_fields(deb)
        except ValueError:
            caught.append("truncated-deb")
        else:
            raise ValueError("dependency archive selftest truncated deb escaped")
        try:
            declared_cache_root({"cache_root": str(temporary)}, cache_root.parent)
        except ValueError:
            caught.append("cache-root-escape")
        else:
            raise ValueError("dependency archive selftest cache-root escape mutation escaped")
    print("dependency archive selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    with tempfile.NamedTemporaryFile("w", dir=RECEIPT_PATH.parent, delete=False) as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, RECEIPT_PATH)


def declared_cache_root(receipt: dict, base: Path) -> Path:
    raw = receipt.get("cache_root")
    if not isinstance(raw, str) or not raw:
        raise ValueError("dependency archive receipt has no cache root")
    root = Path(raw).resolve()
    allowed = base.resolve()
    if root != allowed and allowed not in root.parents:
        raise ValueError("dependency archive cache root escapes the configured base")
    return root


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--hydrate", action="store_true")
    parser.add_argument("--refresh-receipt", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--receipt-check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--cache-root", type=Path)
    args = parser.parse_args()
    if not args.hydrate and not args.refresh_receipt and not args.check and not args.receipt_check:
        args.check = True
    try:
        if sum(bool(item) for item in (args.hydrate, args.refresh_receipt, args.receipt_check)) > 1:
            raise ValueError("--hydrate, --refresh-receipt and --receipt-check are mutually exclusive")
        lock = load(LOCK_PATH)
        base = args.cache_root or Path(os.environ.get("ZLOS_DEPENDENCY_CACHE", DEFAULT_CACHE_BASE))
        cache_root = base / sha256(LOCK_PATH)[:24]
        if args.hydrate:
            receipt = hydrate(lock, cache_root)
            write_atomic(receipt)
        else:
            receipt = load(RECEIPT_PATH)
            cache_root = declared_cache_root(receipt, base)
            if args.refresh_receipt:
                receipt["build_identity"] = lock["build_identity"]
                receipt["dependency_lock_sha256"] = sha256(LOCK_PATH)
                receipt["generator"]["sha256"] = sha256(Path(__file__).resolve())
                validate(receipt, lock, cache_root)
                write_atomic(receipt)
        validate(receipt, lock, cache_root, check_files=not args.receipt_check)
        if args.selftest:
            selftest(receipt, lock, cache_root)
        print(
            "dependency archives: PASS_LOCAL_OFFLINE_ARCHIVE: "
            f"{receipt['counts']['binary_archives']} binaries, "
            f"{receipt['counts']['source_packages']} source packages, "
            f"{receipt['counts']['source_archive_files']} source files"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.SubprocessError) as error:
        print(f"dependency archives: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
