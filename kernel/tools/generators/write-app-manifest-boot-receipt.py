#!/usr/bin/env python3
"""Validate a boot log and atomically preserve its app-manifest receipt."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys


HERE = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.dirname(os.path.dirname(HERE))
REPO_ROOT = os.path.dirname(KERNEL_ROOT)
METADATA = os.path.join(KERNEL_ROOT, "metadata")
MANIFEST = os.path.join(METADATA, "app-manifest.json")
EMBED = os.path.join(KERNEL_ROOT, "app_manifest_embed.zl")
BUILD_IDENTITY = os.path.join(METADATA, "build-identity.json")
BUILD_EMBED = os.path.join(KERNEL_ROOT, "build_identity_embed.zl")
MARKER = re.compile(r"app-manifest: schema=(\d+) entries=(\d+) sha256=([0-9a-f]{64})")
BUILD_MARKER = re.compile(
    r"build-identity: schema=(\d+) id=([0-9a-f]{64})"
)
BUILD_SOURCE_MARKER = re.compile(r"build-source: head=([0-9a-f]{40}) dirty=([01])")


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def repo_relative(path):
    relative = os.path.relpath(os.path.abspath(path), REPO_ROOT)
    if relative == ".." or relative.startswith("../"):
        raise ValueError(f"receipt source escapes repository: {path}")
    return relative


def expected_manifest_marker(embedded=None):
    expected_digest = sha256(MANIFEST)
    manifest = json.load(open(MANIFEST, encoding="utf-8"))
    schema_match = re.fullmatch(
        r"zlos\.application-identity-manifest\.v(\d+)",
        manifest.get("schema", ""),
    )
    entries_value = manifest.get("entries")
    if schema_match is None or not isinstance(entries_value, list):
        raise ValueError("application manifest schema or entries are invalid")
    expected_schema = schema_match.group(1)
    expected_entries = str(len(entries_value))
    if embedded is None:
        embedded = open(EMBED, encoding="utf-8").read()
    found = MARKER.findall(embedded)
    if len(found) != 1:
        raise ValueError(f"embedded manifest marker count is {len(found)}, expected 1")
    schema, entries, digest = found[0]
    if (schema, entries, digest) != (expected_schema, expected_entries, expected_digest):
        raise ValueError(
            "embedded manifest marker is "
            f"{schema}/{entries}/{digest}, expected "
            f"{expected_schema}/{expected_entries}/{expected_digest}"
        )
    return schema, entries, digest


def run_selftest():
    schema, entries, digest = expected_manifest_marker()
    embedded = open(EMBED, encoding="utf-8").read()
    altered = embedded.replace(
        f"entries={entries}", f"entries={int(entries) + 1}", 1
    )
    try:
        expected_manifest_marker(altered)
    except ValueError:
        pass
    else:
        raise ValueError("manifest count mutation escaped")
    print(f"app-manifest boot receipt selftest: caught count mutation at {entries} entries")


def main(argv):
    parser = argparse.ArgumentParser()
    selftest_only = "--selftest" in argv
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--route", required=not selftest_only,
                        choices=("raw-bios", "native-uefi64", "grub-bios32", "grub-uefi32",
                                 "grub-bios64", "grub-uefi64"))
    parser.add_argument("--artifact", required=not selftest_only)
    parser.add_argument("--log", required=not selftest_only)
    parser.add_argument("--output", required=not selftest_only)
    parser.add_argument("--boot-origin", required=not selftest_only)
    parser.add_argument("--harness", required=not selftest_only)
    parser.add_argument("--source-file", action="append", default=[])
    args = parser.parse_args(argv)
    try:
        if args.selftest:
            run_selftest()
            return 0
        log = open(args.log, encoding="latin-1").read().replace("\r", "")
        found = MARKER.findall(log)
        schema, entries, expected = expected_manifest_marker()
        wanted = [(schema, entries, expected)]
        if found != wanted:
            raise ValueError(f"manifest marker mismatch: {found!r}, expected {wanted!r}")
        if args.boot_origin not in log:
            raise ValueError(f"boot-origin marker absent: {args.boot_origin!r}")
        build_identity = json.load(open(BUILD_IDENTITY, encoding="utf-8"))
        build_found = BUILD_MARKER.findall(log)
        build_wanted = [("1", build_identity["identity_sha256"])]
        if build_found != build_wanted:
            raise ValueError(f"build identity mismatch: {build_found!r}, expected {build_wanted!r}")
        source_found = BUILD_SOURCE_MARKER.findall(log)
        source_wanted = [(
            build_identity["git"]["head"],
            "1" if build_identity["git"]["dirty"] else "0",
        )]
        if source_found != source_wanted:
            raise ValueError(f"build source mismatch: {source_found!r}, expected {source_wanted!r}")
        artifact = os.path.abspath(args.artifact)
        source_files = {
            repo_relative(MANIFEST): expected,
            repo_relative(EMBED): sha256(EMBED),
            repo_relative(BUILD_IDENTITY): sha256(BUILD_IDENTITY),
            repo_relative(BUILD_EMBED): sha256(BUILD_EMBED),
            repo_relative(os.path.join(KERNEL_ROOT, "src", "kernel.zl")):
                sha256(os.path.join(KERNEL_ROOT, "src", "kernel.zl")),
            repo_relative(args.harness): sha256(os.path.abspath(args.harness)),
            repo_relative(__file__): sha256(os.path.abspath(__file__)),
        }
        for source in args.source_file:
            source_path = os.path.abspath(source)
            if not os.path.isfile(source_path):
                raise ValueError(f"extra receipt source is missing: {source}")
            source_files[repo_relative(source_path)] = sha256(source_path)
        receipt = {
            "schema": "zlos.application-manifest-boot-receipt.v1",
            "route": args.route,
            "evidence": "QEMU runtime boot and embedded manifest; not native physical hardware",
            "source_head": subprocess.check_output(
                ["git", "rev-parse", "HEAD"], cwd=REPO_ROOT, text=True).strip(),
            "source_files_sha256": source_files,
            "artifact": {
                "path": os.path.relpath(artifact, REPO_ROOT),
                "sha256": sha256(artifact),
                "bytes": os.path.getsize(artifact),
            },
            "boot_log_sha256": sha256(args.log),
            "boot_origin": args.boot_origin,
            "qemu": subprocess.check_output(
                ["qemu-system-x86_64" if "uefi" in args.route or args.route.endswith("64")
                 else "qemu-system-i386",
                 "--version"], text=True).splitlines()[0],
            "shipped_manifest": {
                "schema": int(schema),
                "entries": int(entries),
                "sha256": expected,
            },
            "shipped_build_identity": {
                "schema": 1,
                "id": build_identity["identity_sha256"],
                "head": build_identity["git"]["head"],
                "dirty": build_identity["git"]["dirty"],
            },
            "result": "PASS",
            "weakest_link": "QEMU proves this boot artifact and manifest marker, not physical firmware/display/input behavior",
        }
        output = os.path.abspath(args.output)
        os.makedirs(os.path.dirname(output), exist_ok=True)
        temp = output + ".tmp"
        with open(temp, "w", encoding="utf-8") as handle:
            json.dump(receipt, handle, indent=2, sort_keys=True)
            handle.write("\n")
        os.replace(temp, output)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"app-manifest boot receipt: FAIL: {error}", file=sys.stderr)
        return 1
    print(f"app-manifest boot receipt: PASS -> {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
