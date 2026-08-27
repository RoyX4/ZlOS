#!/usr/bin/env python3
"""Archive the exact current build-input closure without claiming custody.

The build identity detects input drift, but hashes alone cannot reconstruct a
dirty or untracked input after it changes. This generator writes a deterministic
ustar payload containing every file named by build-identity.json and a receipt
that independently checks every member, byte count, mode and digest.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import io
import json
import os
import stat
import subprocess
import sys
import tarfile
from pathlib import Path, PurePosixPath


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
ROOT = KERNEL_ROOT.parent
METADATA = KERNEL_ROOT / "metadata"
IDENTITY = METADATA / "build-identity.json"
ARCHIVE = KERNEL_ROOT / "docs/receipts/source-snapshot-build-inputs-2026-08-24.tar"
RECEIPT = KERNEL_ROOT / "docs/receipts/source-snapshot-2026-08-24.json"
ARCHIVE_REL = "kernel/docs/receipts/source-snapshot-build-inputs-2026-08-24.tar"
RECEIPT_REL = "kernel/docs/receipts/source-snapshot-2026-08-24.json"


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def read_json(path: Path) -> dict:
    return json.loads(path.read_text())


def command(*args: str) -> str:
    try:
        return subprocess.check_output(args, cwd=ROOT, text=True, stderr=subprocess.STDOUT).strip()
    except (OSError, subprocess.CalledProcessError) as error:
        raise ValueError(f"command failed: {' '.join(args)}: {error}") from error


def safe_input_path(relative: str) -> Path:
    pure = PurePosixPath(relative)
    if pure.is_absolute() or not pure.parts or any(part in ("", ".", "..") for part in pure.parts):
        raise ValueError(f"unsafe source path: {relative!r}")
    path = ROOT.joinpath(*pure.parts)
    info = path.lstat()
    if not stat.S_ISREG(info.st_mode):
        raise ValueError(f"build input is not a regular file: {relative}")
    return path


def source_rows(identity: dict) -> list[dict]:
    expected = identity.get("source_files_sha256")
    if not isinstance(expected, dict) or not expected:
        raise ValueError("build identity has no source-file closure")
    rows = []
    for relative, expected_hash in sorted(expected.items()):
        path = safe_input_path(relative)
        payload = path.read_bytes()
        actual = sha256_bytes(payload)
        if actual != expected_hash:
            raise ValueError(f"build identity is stale for {relative}")
        rows.append(
            {
                "path": relative,
                "bytes": len(payload),
                "mode": stat.S_IMODE(path.stat().st_mode),
                "sha256": actual,
            }
        )
    return rows


def archive_bytes(rows: list[dict]) -> bytes:
    output = io.BytesIO()
    with tarfile.open(fileobj=output, mode="w", format=tarfile.USTAR_FORMAT) as archive:
        for row in rows:
            payload = (ROOT / row["path"]).read_bytes()
            info = tarfile.TarInfo(row["path"])
            info.size = len(payload)
            info.mode = row["mode"]
            info.uid = 0
            info.gid = 0
            info.uname = ""
            info.gname = ""
            info.mtime = 0
            info.type = tarfile.REGTYPE
            archive.addfile(info, io.BytesIO(payload))
    return output.getvalue()


def canonical_status(identity: dict) -> str:
    paths = sorted(identity["source_files_sha256"])
    raw = command("git", "status", "--porcelain=v1", "--untracked-files=all", "--", *paths)
    return raw + ("\n" if raw else "")


def build(identity: dict, rows: list[dict], payload: bytes) -> dict:
    submodules = command("git", "submodule", "status", "--recursive")
    return {
        "schema": "zlos.source-snapshot-receipt.v1",
        "result": "PASS_WITH_OPEN_CUSTODY_GAP",
        "build_identity": identity["identity_sha256"],
        "git": {
            "head": command("git", "rev-parse", "HEAD"),
            "branch": command("git", "branch", "--show-current"),
            "dirty": identity["git"]["dirty"],
            "canonical_status_sha256": sha256_bytes(canonical_status(identity).encode()),
            "submodule_status": submodules.splitlines() if submodules else [],
        },
        "scope": identity["source_scope"],
        "archive": {
            "path": ARCHIVE_REL,
            "format": "deterministic POSIX ustar; sorted paths; uid/gid/mtime zero",
            "bytes": len(payload),
            "sha256": sha256_bytes(payload),
            "members": len(rows),
        },
        "counts": {"build_inputs": len(rows), "archived_inputs": len(rows)},
        "source_files": rows,
        "open_gaps": {
            "off_host_copies": 0,
            "signed_attestation": False,
            "whole_repository_snapshot": False,
        },
        "generator": {
            "path": "kernel/gen-source-snapshot.py",
            "sha256": sha256_bytes(Path(__file__).read_bytes()),
        },
        "evidence_ceiling": (
            "exact reconstructable current build-input closure in the same uncommitted worktree; "
            "not a signed, off-host, whole-repository or release-custody snapshot"
        ),
    }


def validate(value: dict, payload: bytes, identity: dict) -> None:
    if value.get("schema") != "zlos.source-snapshot-receipt.v1":
        raise ValueError("wrong source-snapshot schema")
    if value.get("result") != "PASS_WITH_OPEN_CUSTODY_GAP":
        raise ValueError("source snapshot hides its custody gap")
    if value.get("build_identity") != identity.get("identity_sha256"):
        raise ValueError("source snapshot build identity drifted")
    rows = value.get("source_files")
    if not isinstance(rows, list) or len(rows) != len(identity.get("source_files_sha256", {})):
        raise ValueError("source snapshot input count drifted")
    expected_paths = sorted(identity["source_files_sha256"])
    if [row.get("path") for row in rows] != expected_paths:
        raise ValueError("source snapshot member order/set drifted")
    counts = value.get("counts")
    if counts != {"build_inputs": len(rows), "archived_inputs": len(rows)}:
        raise ValueError("source snapshot counts drifted")
    archive = value.get("archive", {})
    if archive.get("path") != ARCHIVE_REL or archive.get("bytes") != len(payload) \
            or archive.get("sha256") != sha256_bytes(payload) \
            or archive.get("members") != len(rows):
        raise ValueError("source snapshot archive identity drifted")
    if value.get("open_gaps") != {
        "off_host_copies": 0,
        "signed_attestation": False,
        "whole_repository_snapshot": False,
    }:
        raise ValueError("source snapshot custody gaps were hidden")
    if len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("source snapshot generator identity is missing")

    with tarfile.open(fileobj=io.BytesIO(payload), mode="r:") as archive_file:
        members = archive_file.getmembers()
        if [member.name for member in members] != expected_paths:
            raise ValueError("archive member order/set drifted")
        for row, member in zip(rows, members, strict=True):
            if not member.isfile() or member.uid != 0 or member.gid != 0 or member.mtime != 0:
                raise ValueError(f"archive metadata is not normalized: {member.name}")
            if member.mode != row.get("mode") or member.size != row.get("bytes"):
                raise ValueError(f"archive mode/size drifted: {member.name}")
            extracted = archive_file.extractfile(member)
            if extracted is None or sha256_bytes(extracted.read()) != row.get("sha256"):
                raise ValueError(f"archive content drifted: {member.name}")
            if row.get("sha256") != identity["source_files_sha256"][member.name]:
                raise ValueError(f"receipt hash disagrees with build identity: {member.name}")


def selftest(value: dict, payload: bytes, identity: dict) -> None:
    mutations = []
    cases = []
    missing = copy.deepcopy(value)
    missing["source_files"].pop()
    cases.append(("missing-input", missing, payload))
    hidden = copy.deepcopy(value)
    hidden["open_gaps"]["off_host_copies"] = 1
    cases.append(("invented-custody", hidden, payload))
    wrong_hash = copy.deepcopy(value)
    wrong_hash["archive"]["sha256"] = "0" * 64
    cases.append(("archive-hash", wrong_hash, payload))
    corrupted = bytearray(payload)
    corrupted[512] ^= 1
    cases.append(("archive-content", value, bytes(corrupted)))
    for name, mutant, mutant_payload in cases:
        try:
            validate(mutant, mutant_payload, identity)
        except (ValueError, tarfile.TarError):
            mutations.append(name)
        else:
            raise ValueError(f"source-snapshot mutation escaped: {name}")
    print("source-snapshot selftest: caught " + ", ".join(mutations))


def atomic_write(path: Path, payload: bytes) -> None:
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_bytes(payload)
    os.replace(temporary, path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        identity = read_json(IDENTITY)
        rows = source_rows(identity)
        payload = archive_bytes(rows)
        value = build(identity, rows, payload)
        validate(value, payload, identity)
        if args.selftest:
            selftest(value, payload, identity)
        if args.write:
            ARCHIVE.parent.mkdir(parents=True, exist_ok=True)
            atomic_write(ARCHIVE, payload)
            atomic_write(RECEIPT, (json.dumps(value, indent=2) + "\n").encode())
        else:
            if not ARCHIVE.is_file() or ARCHIVE.read_bytes() != payload:
                raise ValueError("source snapshot archive is stale; run --write and inspect")
            if not RECEIPT.is_file() or read_json(RECEIPT) != value:
                raise ValueError("source snapshot receipt is stale; run --write and inspect")
        print(
            f"source-snapshot: PASS_WITH_OPEN_CUSTODY_GAP: {len(rows)} inputs, "
            f"{len(payload)} bytes, sha256={sha256_bytes(payload)}"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError, tarfile.TarError) as error:
        print(f"source-snapshot: FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
