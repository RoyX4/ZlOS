#!/usr/bin/env python3
"""Prove that two complete recipe executions produce identical zlOS artifacts."""

from __future__ import annotations

import argparse
import fcntl
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile


HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_RECEIPT = os.path.join(
    HERE, "docs", "receipts", "reproducible-build-2026-08-22.json")
BUILD_IDENTITY = os.path.join(HERE, "build-identity.json")
RECIPES = ("./mkiso.sh", "./mkiso64.sh", "./mkdisk.sh", "./mkusb.sh")
ARTIFACTS = (
    "kernel.elf",
    "kernel64.elf",
    "kernel_raw.elf",
    "BOOTX64.EFI",
    "zlOS.iso",
    "zlOS64.iso",
    "zlOS.img",
    "zlOS-usb.img",
    "boot-media-ids.json",
)


def digest(path):
    value = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def compare(first, second):
    failures = []
    for name in ARTIFACTS:
        left = os.path.join(first, name)
        right = os.path.join(second, name)
        if not os.path.isfile(left) or not os.path.isfile(right):
            failures.append(f"{name}: missing artifact")
            continue
        left_hash, right_hash = digest(left), digest(right)
        if left_hash != right_hash:
            failures.append(f"{name}: {left_hash} != {right_hash}")
    return failures


def artifact_records(directory):
    return {
        name: {
            "path": "kernel/" + name,
            "sha256": digest(os.path.join(directory, name)),
            "bytes": os.path.getsize(os.path.join(directory, name)),
        }
        for name in ARTIFACTS
    }


def snapshot_contract():
    identity = json.load(open(BUILD_IDENTITY, encoding="utf-8"))
    return {
        "schema": "zlos.reproducible-build-snapshot.v1",
        "build_identity": identity["identity_sha256"],
        "checker_sha256": digest(os.path.abspath(__file__)),
        "recipes": list(RECIPES),
        "artifact_names": list(ARTIFACTS),
    }


def marker_path(directory):
    return os.path.join(directory, ".complete.json")


def load_snapshot(directory, contract):
    marker = marker_path(directory)
    try:
        value = json.load(open(marker, encoding="utf-8"))
    except (OSError, ValueError):
        return False
    if value.get("contract") != contract:
        return False
    try:
        records = artifact_records(directory)
    except OSError:
        return False
    return value.get("artifacts") == records


def write_snapshot(directory, contract):
    value = {"contract": contract, "artifacts": artifact_records(directory)}
    marker = marker_path(directory)
    temp = marker + ".tmp"
    with open(temp, "w", encoding="utf-8") as handle:
        json.dump(value, handle, indent=2, sort_keys=True)
        handle.write("\n")
    os.replace(temp, marker)


def reset_snapshot(directory):
    if os.path.isdir(directory):
        shutil.rmtree(directory)
    os.makedirs(directory)


def build_or_resume(destination, contract, label):
    if load_snapshot(destination, contract):
        print(f"reproducible-build: resumed verified {label} snapshot")
        return
    reset_snapshot(destination)
    build(destination)
    write_snapshot(destination, contract)
    print(f"reproducible-build: checkpointed {label} snapshot")


def write_receipt(path, first, second):
    build_identity = json.load(open(BUILD_IDENTITY, encoding="utf-8"))
    first_records = artifact_records(first)
    second_records = artifact_records(second)
    if first_records != second_records:
        raise RuntimeError("refusing to receipt non-identical artifact records")
    receipt = {
        "schema": "zlos.reproducible-build-receipt.v1",
        "result": "PASS",
        "evidence": "two independent executions of every listed build recipe produced byte-identical artifacts",
        "build_identity": {
            "schema": build_identity["schema"],
            "id": build_identity["identity_sha256"],
            "head": build_identity["git"]["head"],
            "dirty": build_identity["git"]["dirty"],
        },
        "recipes": list(RECIPES),
        "independent_runs": 2,
        "artifacts": first_records,
        "source_files_sha256": {
            "check-reproducible-build.py": digest(os.path.abspath(__file__)),
            "build-identity.json": digest(BUILD_IDENTITY),
        },
        "weakest_link": "byte reproducibility does not by itself prove bootability, runtime behavior, compiler diversity, or physical-hardware behavior",
    }
    output = os.path.abspath(path)
    os.makedirs(os.path.dirname(output), exist_ok=True)
    temp = output + ".tmp"
    with open(temp, "w", encoding="utf-8") as handle:
        json.dump(receipt, handle, indent=2, sort_keys=True)
        handle.write("\n")
    os.replace(temp, output)


def build(destination):
    for recipe in RECIPES:
        subprocess.run([recipe], cwd=HERE, check=True)
    for name in ARTIFACTS:
        source = os.path.join(HERE, name)
        if not os.path.isfile(source):
            raise RuntimeError(f"mkiso.sh did not produce {name}")
        shutil.copyfile(source, os.path.join(destination, name))


def selftest():
    with tempfile.TemporaryDirectory(prefix="zlos-repro-selftest-") as root:
        first = os.path.join(root, "first")
        second = os.path.join(root, "second")
        os.mkdir(first)
        os.mkdir(second)
        for directory in (first, second):
            for name in ARTIFACTS:
                with open(os.path.join(directory, name), "wb") as handle:
                    handle.write((name + "\n").encode())
        if compare(first, second):
            raise RuntimeError("selftest rejected identical artifacts")
        contract = {
            "schema": "selftest", "build_identity": "a" * 64,
            "checker_sha256": "b" * 64, "recipes": list(RECIPES),
            "artifact_names": list(ARTIFACTS),
        }
        write_snapshot(first, contract)
        if not load_snapshot(first, contract):
            raise RuntimeError("selftest rejected a valid resume snapshot")
        with open(os.path.join(first, ARTIFACTS[0]), "ab") as handle:
            handle.write(b"snapshot mutation")
        if load_snapshot(first, contract):
            raise RuntimeError("selftest accepted a mutated resume snapshot")
        with open(os.path.join(first, ARTIFACTS[0]), "wb") as handle:
            handle.write((ARTIFACTS[0] + "\n").encode())
        write_snapshot(first, contract)
        wrong_contract = dict(contract)
        wrong_contract["build_identity"] = "c" * 64
        if load_snapshot(first, wrong_contract):
            raise RuntimeError("selftest accepted a snapshot for another identity")
        for name in ARTIFACTS:
            path = os.path.join(second, name)
            with open(path, "ab") as handle:
                handle.write(b"mutation")
            failures = compare(first, second)
            if len(failures) != 1 or not failures[0].startswith(name + ":"):
                raise RuntimeError(f"selftest {name} mutation escaped comparison")
            with open(path, "wb") as handle:
                handle.write((name + "\n").encode())
        os.unlink(os.path.join(second, "kernel.elf"))
        failures = compare(first, second)
        if not any(item == "kernel.elf: missing artifact" for item in failures):
            raise RuntimeError("selftest missing artifact escaped comparison")
    print(f"reproducible-build selftest: caught byte drift in all {len(ARTIFACTS)} artifacts, a missing artifact, and stale resume snapshots")


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--receipt", default=DEFAULT_RECEIPT)
    parser.add_argument("--resume-dir",
                        help="persistent checkpoint directory; defaults to /tmp keyed by build identity")
    parser.add_argument("--keep-workspace", action="store_true")
    args = parser.parse_args(argv)
    if not args.check and not args.selftest:
        parser.error("choose --check, --selftest, or both")

    try:
        if args.selftest:
            selftest()
        if args.check:
            contract = snapshot_contract()
            root = args.resume_dir or os.path.join(
                tempfile.gettempdir(), "zlos-repro-build-" + contract["build_identity"][:16])
            os.makedirs(root, exist_ok=True)
            lock_path = os.path.join(root, ".lock")
            with open(lock_path, "w", encoding="utf-8") as lock:
                try:
                    fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
                except BlockingIOError as error:
                    raise RuntimeError(f"another reproducibility run owns {root}") from error
                first = os.path.join(root, "first")
                second = os.path.join(root, "second")
                build_or_resume(first, contract, "first")
                build_or_resume(second, contract, "second")
                failures = compare(first, second)
                if failures:
                    for failure in failures:
                        print("reproducible-build: FAIL: " + failure,
                              file=sys.stderr)
                    return 1
                for name in ARTIFACTS:
                    print(f"reproducible-build: {name} sha256="
                          f"{digest(os.path.join(first, name))}")
                write_receipt(args.receipt, first, second)
                print(f"reproducible-build: receipt={args.receipt}")
                print("reproducible-build: PASS: two independent recipe runs match")
                if not args.keep_workspace:
                    # Leave the lock file and root until after both snapshots
                    # have been checked and receipted. An interrupted process
                    # leaves them for the next invocation to validate/resume.
                    for directory in (first, second):
                        shutil.rmtree(directory)
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f"reproducible-build: FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
