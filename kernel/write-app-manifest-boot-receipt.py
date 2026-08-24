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
MANIFEST = os.path.join(HERE, "app-manifest.json")
EMBED = os.path.join(HERE, "app_manifest_embed.zl")
BUILD_IDENTITY = os.path.join(HERE, "build-identity.json")
BUILD_EMBED = os.path.join(HERE, "build_identity_embed.zl")
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


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--route", required=True,
                        choices=("raw-bios", "native-uefi64", "grub-bios32", "grub-uefi32",
                                 "grub-bios64", "grub-uefi64"))
    parser.add_argument("--artifact", required=True)
    parser.add_argument("--log", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--boot-origin", required=True)
    parser.add_argument("--harness", required=True)
    parser.add_argument("--source-file", action="append", default=[])
    args = parser.parse_args(argv)
    try:
        log = open(args.log, encoding="latin-1").read().replace("\r", "")
        found = MARKER.findall(log)
        expected = sha256(MANIFEST)
        if found != [("1", "62", expected)]:
            raise ValueError(f"manifest marker mismatch: {found!r}, expected 1/62/{expected}")
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
            "app-manifest.json": expected,
            "app_manifest_embed.zl": sha256(EMBED),
            "build-identity.json": sha256(BUILD_IDENTITY),
            "build_identity_embed.zl": sha256(BUILD_EMBED),
            "kernel.zl": sha256(os.path.join(HERE, "kernel.zl")),
            os.path.basename(args.harness): sha256(os.path.abspath(args.harness)),
            "write-app-manifest-boot-receipt.py": sha256(os.path.abspath(__file__)),
        }
        for source in args.source_file:
            source_path = os.path.abspath(source)
            if not os.path.isfile(source_path):
                raise ValueError(f"extra receipt source is missing: {source}")
            source_files[os.path.relpath(source_path, HERE)] = sha256(source_path)
        receipt = {
            "schema": "zlos.application-manifest-boot-receipt.v1",
            "route": args.route,
            "evidence": "QEMU runtime boot and embedded manifest; not native physical hardware",
            "source_head": subprocess.check_output(
                ["git", "rev-parse", "HEAD"], cwd=os.path.dirname(HERE), text=True).strip(),
            "source_files_sha256": source_files,
            "artifact": {
                "path": os.path.relpath(artifact, os.path.dirname(HERE)),
                "sha256": sha256(artifact),
                "bytes": os.path.getsize(artifact),
            },
            "boot_log_sha256": sha256(args.log),
            "boot_origin": args.boot_origin,
            "qemu": subprocess.check_output(
                ["qemu-system-x86_64" if "uefi" in args.route or args.route.endswith("64")
                 else "qemu-system-i386",
                 "--version"], text=True).splitlines()[0],
            "shipped_manifest": {"schema": 1, "entries": 62, "sha256": expected},
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
