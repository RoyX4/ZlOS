#!/usr/bin/env python3
"""Derive stable GPT/FAT/time identities from the current build identity."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
import sys
import uuid


HERE = os.path.dirname(os.path.abspath(__file__))
BUILD_IDENTITY = os.path.join(HERE, "build-identity.json")
OUTPUT = os.path.join(HERE, "boot-media-ids.json")


def load(path):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


def derived_uuid(label, identity):
    value = bytearray(hashlib.sha256((label + "\0" + identity).encode()).digest()[:16])
    value[6] = (value[6] & 0x0F) | 0x50
    value[8] = (value[8] & 0x3F) | 0x80
    return str(uuid.UUID(bytes=bytes(value)))


def generate():
    build = load(BUILD_IDENTITY)
    identity = build.get("identity_sha256")
    head = build.get("git", {}).get("head")
    if not isinstance(identity, str) or len(identity) != 64:
        raise ValueError("invalid build identity")
    if not isinstance(head, str) or len(head) != 40:
        raise ValueError("invalid build Git head")
    epoch = int(subprocess.check_output(
        ["git", "show", "-s", "--format=%ct", head], cwd=os.path.dirname(HERE),
        text=True).strip())
    fat = hashlib.sha256(("zlos-usb-fat\0" + identity).encode()).hexdigest()[:8].upper()
    return {
        "schema": "zlos.boot-media-ids.v1",
        "build_identity": identity,
        "source_date_epoch": epoch,
        "usb_disk_guid": derived_uuid("zlos-usb-disk", identity),
        "usb_partition_guid": derived_uuid("zlos-usb-esp", identity),
        "usb_zllog_partition_guid": derived_uuid("zlos-usb-zllog", identity),
        "usb_fat_volume_id": fat,
        "derivation": "SHA-256 labels over build identity; UUID version/variant bits normalized",
    }


def validate(value):
    expected = generate()
    if value != expected:
        raise ValueError("boot-media-ids.json is stale")
    for key in ("usb_disk_guid", "usb_partition_guid", "usb_zllog_partition_guid"):
        if uuid.UUID(value[key]).version != 5:
            raise ValueError(f"{key} is not a normalized version-5 UUID")
    if len(value["usb_fat_volume_id"]) != 8:
        raise ValueError("invalid FAT volume ID")


def selftest(value):
    changed = dict(value)
    changed["build_identity"] = "0" * 64
    if (derived_uuid("zlos-usb-disk", changed["build_identity"]) ==
            value["usb_disk_guid"]):
        raise ValueError("identity mutation did not change disk GUID")
    if derived_uuid("zlos-usb-disk", value["build_identity"]) == value["usb_partition_guid"]:
        raise ValueError("domain labels did not separate disk and partition GUIDs")
    if value["usb_partition_guid"] == value["usb_zllog_partition_guid"]:
        raise ValueError("ESP and ZLLOG partition GUIDs are not separated")
    print("boot-media-ids selftest: identity and domain mutations change IDs")


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args(argv)
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        expected = generate()
        if args.write:
            temporary = OUTPUT + ".tmp"
            with open(temporary, "w", encoding="utf-8") as handle:
                json.dump(expected, handle, indent=2, sort_keys=True)
                handle.write("\n")
            os.replace(temporary, OUTPUT)
        else:
            validate(load(OUTPUT))
        if args.selftest:
            selftest(expected)
        print("boot-media-ids: PASS: " + expected["build_identity"])
    except (OSError, ValueError, json.JSONDecodeError,
            subprocess.CalledProcessError) as error:
        print(f"boot-media-ids: FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
