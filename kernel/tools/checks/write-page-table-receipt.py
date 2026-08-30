#!/usr/bin/env python3
"""Write the joined host/QEMU receipt for transactional page-table construction."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import re
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL = HERE.parents[1]
ROOT = KERNEL.parent
IDENTITY = KERNEL / "metadata/build-identity.json"
ARTIFACT = KERNEL / "zlOS-usb.img"
HARNESS = KERNEL / "tools/checks/verify-efi.sh"
HOST_RECEIPT = KERNEL / "tests/host/test-run-receipt.json"
IMPLEMENTATION = (
    KERNEL / "src/arch/x86/page_table_txn.h",
    KERNEL / "src/arch/x86/page_table_txn.c",
    KERNEL / "src/arch/x86/paging.c",
)
DEFAULT_OUTPUT = KERNEL / "docs/receipts/page-table-native-uefi64-qemu-2026-08-29.json"
QEMU_PATTERN = re.compile(
    r"vmm: 64 MiB mapped: virtual [0-9]+ GiB -> physical 256 MiB"
)
HOST_MARKERS = (
    "26 checks, 0 failures",
    "full-range page-table transactions are exact under every injected failure",
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def host_target(receipt: dict) -> dict:
    rows = [row for row in receipt.get("results", [])
            if row.get("name") == "pagetxntest"]
    if len(rows) != 1 or rows[0].get("status") != "passed":
        raise ValueError("current host receipt has no passing pagetxntest")
    commands = rows[0].get("commands", [])
    if len(commands) != 1 or commands[0].get("exit_code") != 0 \
            or any(marker not in commands[0].get("output_tail", "")
                   for marker in HOST_MARKERS):
        raise ValueError("pagetxntest lost its full-range failure observations")
    return rows[0]


def build(log_path: Path, journal_path: Path) -> dict:
    log = log_path.read_text(encoding="latin-1").replace("\r", "")
    journal = journal_path.read_text(encoding="latin-1").replace("\r", "")
    matches = QEMU_PATTERN.findall(log)
    if len(matches) != 1:
        raise ValueError(f"transactional VMM marker count is {len(matches)}, expected 1")
    if "cache=write-combining" not in journal:
        raise ValueError("framebuffer write-combining journal marker is absent")
    identity = json.loads(IDENTITY.read_text())["identity_sha256"]
    host = json.loads(HOST_RECEIPT.read_text())
    row = host_target(host)
    if host.get("build_identity") != identity or host.get("outcome") != "PASS":
        raise ValueError("page-table host receipt is failed or foreign")
    value = {
        "schema": "zlos.page-table-native-uefi64-qemu-receipt.v1",
        "result": "PASS_TRANSACTION_CORE_AND_NATIVE_HEAP_WINDOW_WITH_OPEN_GAPS",
        "build_identity": identity,
        "route": "native-uefi64",
        "artifact": {"path": "kernel/zlOS-usb.img", "sha256": digest(ARTIFACT)},
        "boot_log_sha256": digest(log_path),
        "boot_journal_sha256": digest(journal_path),
        "harness": {
            "path": "kernel/tools/checks/verify-efi.sh",
            "sha256": digest(HARNESS),
        },
        "implementation": [
            {"path": path.relative_to(ROOT).as_posix(), "sha256": digest(path)}
            for path in IMPLEMENTATION
        ],
        "host_receipt": {
            "path": "kernel/tests/host/test-run-receipt.json",
            "sha256": digest(HOST_RECEIPT),
            "target_id": row["id"],
            "target": row["name"],
            "injected_write_positions": 512,
            "checks": 26,
        },
        "assertions": [
            {
                "id": "full-range-host-transaction",
                "entries": 512,
                "stages": ["reserve", "validate", "apply", "flush", "commit"],
                "exact_rollback": True,
                "nth_write_failures": 512,
                "flush_failure": True,
                "post_flush_corruption": True,
                "rollback_flush_failure_is_fatal": True,
            },
            {
                "id": "native-heap-window-commit",
                "marker": matches[0],
                "bytes": 64 * 1024 * 1024,
                "physical_base": 256 * 1024 * 1024,
                "alias_probe_passed_before_commit": True,
            },
            {
                "id": "native-framebuffer-cache-transaction",
                "journal_marker": "cache=write-combining",
                "full_range_validated_before_apply": True,
                "global_tlb_flush": True,
                "transaction_committed": True,
            },
        ],
        "known_gaps": [
            "callers must still provide exclusive page-table ownership",
            "there is no general page-table allocation or teardown service",
            "fixed Ring-3 private page tables are built before CR3 publication but do not use the journal",
            "future live page-table writers must adopt the transaction core before publication",
            "the live transaction route is x86-64; the 32-bit build has paging disabled",
            "there is no current physical-hardware page-table receipt",
        ],
        "evidence_ceiling": (
            "current host full-range mutation proof plus exact native-UEFI64 QEMU "
            "heap-window and framebuffer-cache commits; not all page-table users or physical hardware"
        ),
        "generator": {
            "path": "kernel/tools/checks/write-page-table-receipt.py",
            "sha256": digest(Path(__file__).resolve()),
        },
    }
    validate(value)
    return value


def validate(value: dict) -> None:
    identity = json.loads(IDENTITY.read_text())["identity_sha256"]
    if value.get("schema") != "zlos.page-table-native-uefi64-qemu-receipt.v1" \
            or value.get("result") != \
            "PASS_TRANSACTION_CORE_AND_NATIVE_HEAP_WINDOW_WITH_OPEN_GAPS" \
            or value.get("route") != "native-uefi64" \
            or value.get("build_identity") != identity:
        raise ValueError("wrong page-table receipt schema/result/route/build")
    if value.get("artifact") != {
            "path": "kernel/zlOS-usb.img", "sha256": digest(ARTIFACT)}:
        raise ValueError("page-table receipt artifact drifted")
    if value.get("harness") != {
            "path": "kernel/tools/checks/verify-efi.sh", "sha256": digest(HARNESS)}:
        raise ValueError("page-table receipt harness drifted")
    expected_implementation = [
        {"path": path.relative_to(ROOT).as_posix(), "sha256": digest(path)}
        for path in IMPLEMENTATION
    ]
    if value.get("implementation") != expected_implementation:
        raise ValueError("page-table receipt implementation drifted")
    assertions = value.get("assertions", [])
    if len(assertions) != 3 \
            or assertions[0].get("nth_write_failures") != 512 \
            or assertions[0].get("exact_rollback") is not True \
            or assertions[1].get("alias_probe_passed_before_commit") is not True \
            or assertions[2].get("transaction_committed") is not True \
            or assertions[2].get("journal_marker") != "cache=write-combining":
        raise ValueError("page-table assertion coverage drifted")
    host = value.get("host_receipt", {})
    if host.get("sha256") != digest(HOST_RECEIPT) \
            or host.get("target") != "pagetxntest" \
            or host.get("checks") != 26:
        raise ValueError("page-table host binding drifted")
    gaps = value.get("known_gaps", [])
    if len(gaps) != 6 or not any("physical" in gap for gap in gaps) \
            or not any("Ring-3" in gap for gap in gaps):
        raise ValueError("page-table receipt hides remaining users or proof gaps")
    if len(value.get("boot_log_sha256", "")) != 64 \
            or len(value.get("boot_journal_sha256", "")) != 64 \
            or len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("page-table receipt identities are incomplete")


def selftest(value: dict) -> None:
    mutations = {}
    for name, mutate in (
        ("foreign-build", lambda item: item.__setitem__("build_identity", "0" * 64)),
        ("foreign-artifact", lambda item: item["artifact"].__setitem__("sha256", "0" * 64)),
        ("missing-injection", lambda item: item["assertions"][0].__setitem__("nth_write_failures", 511)),
        ("invented-physical", lambda item: item.__setitem__("known_gaps", [])),
    ):
        mutant = copy.deepcopy(value)
        mutate(mutant)
        mutations[name] = mutant
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except (OSError, ValueError, TypeError, json.JSONDecodeError):
            caught.append(name)
        else:
            raise ValueError(f"page-table receipt mutation escaped: {name}")
    print("page-table receipt selftest: caught " + ", ".join(caught))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--log", type=Path)
    parser.add_argument("--journal", type=Path)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.check == bool(args.log) or (args.log and not args.journal) \
            or (args.journal and not args.log):
        parser.error("choose --check or both --log and --journal")
    try:
        value = (json.loads(args.output.read_text()) if args.check
                 else build(args.log, args.journal))
        validate(value)
        if args.selftest:
            selftest(value)
        if args.check:
            print(f"page-table QEMU receipt: PASS_CURRENT -> {args.output}")
            return 0
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.NamedTemporaryFile(
                "w", dir=args.output.parent, delete=False, encoding="utf-8") as handle:
            json.dump(value, handle, indent=2)
            handle.write("\n")
            temporary = Path(handle.name)
        os.replace(temporary, args.output)
        print(f"page-table QEMU receipt: PASS -> {args.output}")
        return 0
    except (OSError, ValueError, TypeError, json.JSONDecodeError) as error:
        print(f"page-table QEMU receipt: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
