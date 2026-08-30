#!/usr/bin/env python3
"""Write the joined host/QEMU receipt for the bounded physical allocator."""

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
USER_PROCESS_RECEIPT = (
    KERNEL / "docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"
)
IMPLEMENTATION = (
    KERNEL / "src/core/pmm.h",
    KERNEL / "src/core/pmm.c",
    ROOT / "freestanding/runtime_kernel.c",
    KERNEL / "src/kernel.zl",
    KERNEL / "tests/host/pmmtest.c",
)
DEFAULT_OUTPUT = (
    KERNEL / "docs/receipts/physical-page-allocator-native-uefi64-qemu-2026-08-30.json"
)
COUNT_PATTERN = re.compile(
    r"^  pmm: ([1-9][0-9]*)/([1-9][0-9]*) pages free in \[320, 1024\) MiB$",
    re.MULTILINE,
)
SELFTEST_MARKER = (
    "<- physical allocator reserved floor, owner quota/mismatch, double-free and "
    "zero/reuse passed; baseline restored"
)
ACCOUNT_MARKER = (
    "<- process memory accounting: fixed/anonymous quotas and owner totals passed"
)
PROCESS_MARKER = (
    "<- process-owned page tables/code/stacks reclaimed; PMM baseline restored"
)
HOST_MARKER = (
    "typed physical allocator enforces owner quotas, rejects reserved/foreign/"
    "double-free and zeroes reuse"
)
HEX64 = re.compile(r"[0-9a-f]{64}")
OWNER_ASSERTION = {
    "id": "typed-owner-release",
    "wrong_owner_refused_without_mutation": True,
    "reserved_release_refused": True,
    "double_free_refused": True,
    "unaligned_release_refused": True,
}
ACCOUNTING_ASSERTION = {
    "id": "exact-owner-accounting-and-quota",
    "exact_used_pages": True,
    "exact_high_water_pages": True,
    "exact_available_pages": True,
    "exact_refusal_count": True,
    "allocation_refused_at_limit": True,
    "limit_shrink_below_usage_refused_without_mutation": True,
    "metadata_owner_sum_invariant": True,
    "target_owner_totals_restored_to_zero": True,
}
ZERO_REUSE_ASSERTION = {
    "id": "zero-reuse-and-exhaustion",
    "zero_on_every_allocation": True,
    "deterministic_reuse": True,
    "host_exhaustion_observed": True,
    "target_baseline_restored": True,
}
KNOWN_GAPS = (
    "the allocator is deliberately bounded to firmware-declared usable pages from 320 MiB through the first 1 GiB",
    "production accounting covers only the fixed two-slot native-UEFI64 process diagnostic and its bounded anonymous window, not services, caches, pinned memory, DMA or surfaces",
    "the typed owner identifier is a bounded provenance key, not a per-allocation origin or call-site ledger",
    "there is no concurrent or interrupt-context locking contract",
    "there is no segmented metadata for physical memory above 1 GiB",
    "the saved live receipt is native-UEFI64 QEMU; Multiboot has an exact golden boot gate but no dedicated allocator receipt",
    "there is no current physical-hardware allocator receipt",
)
EVIDENCE_CEILING = (
    "current host exhaustion, exact owner accounting/quota and hostile ownership proof plus exact native-UEFI64 "
    "QEMU map admission, zero/reuse and bounded process/anonymous-frame reclamation; not unified consumers, SMP or physical hardware"
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def host_target(receipt: dict) -> dict:
    rows = [row for row in receipt.get("results", []) if row.get("name") == "pmmtest"]
    if len(rows) != 1 or rows[0].get("status") != "passed":
        raise ValueError("current host receipt has no passing pmmtest")
    commands = rows[0].get("commands", [])
    if len(commands) != 1 or commands[0].get("exit_code") != 0:
        raise ValueError("pmmtest command did not pass exactly once")
    output = commands[0].get("output_tail", "")
    match = re.search(r"pmmtest: ([1-9][0-9]*) checks, 0 failures", output)
    if not match or HOST_MARKER not in output:
        raise ValueError("pmmtest lost its ownership/exhaustion/zero-reuse observations")
    row = copy.deepcopy(rows[0])
    row["observed_checks"] = int(match.group(1))
    return row


def build(log_path: Path) -> dict:
    log = log_path.read_text(encoding="latin-1").replace("\r", "")
    matches = COUNT_PATTERN.findall(log)
    if len(matches) != 1:
        raise ValueError(f"physical allocator count marker appears {len(matches)} times")
    free_pages, total_pages = (int(value) for value in matches[0])
    if free_pages != total_pages or total_pages < 1:
        raise ValueError("physical allocator self-test did not restore its baseline")
    if log.count(SELFTEST_MARKER) != 1:
        raise ValueError("physical allocator self-test marker is absent or duplicated")
    if log.count(PROCESS_MARKER) != 1:
        raise ValueError("process-frame reclamation marker is absent or duplicated")
    if log.count(ACCOUNT_MARKER) != 1:
        raise ValueError("process-frame accounting marker is absent or duplicated")
    identity = json.loads(IDENTITY.read_text())["identity_sha256"]
    host = json.loads(HOST_RECEIPT.read_text())
    row = host_target(host)
    if host.get("build_identity") != identity or host.get("outcome") != "PASS":
        raise ValueError("physical allocator host receipt is failed or foreign")
    consumer = json.loads(USER_PROCESS_RECEIPT.read_text())
    if consumer.get("build_identity") != identity or \
            consumer.get("result") != "PASS_BOUNDED_USER_PROCESS_BOUNDARY":
        raise ValueError("physical allocator process-consumer receipt is failed or foreign")
    value = {
        "schema": "zlos.physical-page-allocator-native-uefi64-qemu-receipt.v1",
        "result": "PASS_BOUNDED_TYPED_ALLOCATOR_WITH_OPEN_GAPS",
        "build_identity": identity,
        "route": "native-uefi64",
        "artifact": {"path": "kernel/zlOS-usb.img", "sha256": digest(ARTIFACT)},
        "boot_log_sha256": digest(log_path),
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
            "checks": row["observed_checks"],
        },
        "consumer_receipt": {
            "path": "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json",
            "sha256": digest(USER_PROCESS_RECEIPT),
            "assertions": [
                "process-frame-reclamation",
                "anonymous-reserve-commit-release",
                "released-anonymous-page-fault",
                "process-memory-accounting",
            ],
        },
        "assertions": [
            {
                "id": "bounded-firmware-map-admission",
                "managed_floor_bytes": 320 * 1024 * 1024,
                "managed_limit_bytes": 1024 * 1024 * 1024,
                "admitted_pages": total_pages,
                "free_pages_after_selftest": free_pages,
                "firmware_conventional_memory_only": True,
                "fixed_zlos_region_excluded": True,
            },
            dict(OWNER_ASSERTION),
            dict(ACCOUNTING_ASSERTION),
            dict(ZERO_REUSE_ASSERTION),
            {
                "id": "typed-process-and-anonymous-consumer",
                "frames_per_process": 8,
                "two_processes_disjoint": True,
                "failure_atomic_acquire": True,
                "anonymous_page_limit": 32,
                "anonymous_zero_fill": True,
                "anonymous_failure_atomic": True,
                "anonymous_target_reclaimed": True,
                "target_baseline_restored": True,
            },
        ],
        "known_gaps": list(KNOWN_GAPS),
        "evidence_ceiling": EVIDENCE_CEILING,
        "generator": {
            "path": "kernel/tools/checks/write-pmm-receipt.py",
            "sha256": digest(Path(__file__).resolve()),
        },
    }
    validate(value)
    return value


def validate(value: dict) -> None:
    identity = json.loads(IDENTITY.read_text())["identity_sha256"]
    if value.get("schema") != \
            "zlos.physical-page-allocator-native-uefi64-qemu-receipt.v1" or \
            value.get("result") != "PASS_BOUNDED_TYPED_ALLOCATOR_WITH_OPEN_GAPS" or \
            value.get("route") != "native-uefi64" or \
            value.get("build_identity") != identity:
        raise ValueError("wrong physical allocator receipt schema/result/route/build")
    if value.get("artifact") != {
            "path": "kernel/zlOS-usb.img", "sha256": digest(ARTIFACT)}:
        raise ValueError("physical allocator artifact identity drifted")
    if value.get("harness") != {
            "path": "kernel/tools/checks/verify-efi.sh", "sha256": digest(HARNESS)}:
        raise ValueError("physical allocator harness identity drifted")
    expected_impl = [
        {"path": path.relative_to(ROOT).as_posix(), "sha256": digest(path)}
        for path in IMPLEMENTATION
    ]
    if value.get("implementation") != expected_impl:
        raise ValueError("physical allocator implementation identity drifted")
    log_digest = value.get("boot_log_sha256")
    if not isinstance(log_digest, str) or not HEX64.fullmatch(log_digest) or \
            log_digest == "0" * 64:
        raise ValueError("physical allocator boot-log identity is absent")
    assertions = value.get("assertions", [])
    if [row.get("id") for row in assertions] != [
            "bounded-firmware-map-admission", "typed-owner-release",
            "exact-owner-accounting-and-quota", "zero-reuse-and-exhaustion",
            "typed-process-and-anonymous-consumer"]:
        raise ValueError("physical allocator assertion set drifted")
    bounded = assertions[0]
    if bounded.get("managed_floor_bytes") != 320 * 1024 * 1024 or \
            bounded.get("managed_limit_bytes") != 1024 * 1024 * 1024 or \
            bounded.get("admitted_pages", 0) < 1 or \
            bounded.get("free_pages_after_selftest") != bounded.get("admitted_pages") or \
            bounded.get("firmware_conventional_memory_only") is not True or \
            bounded.get("fixed_zlos_region_excluded") is not True:
        raise ValueError("physical allocator bounded-map assertion is false")
    if assertions[1] != OWNER_ASSERTION or assertions[2] != ACCOUNTING_ASSERTION or \
            assertions[3] != ZERO_REUSE_ASSERTION:
        raise ValueError("physical allocator ownership, accounting or zero/reuse assertion is false")
    if assertions[4] != {
            "id": "typed-process-and-anonymous-consumer", "frames_per_process": 8,
            "two_processes_disjoint": True, "failure_atomic_acquire": True,
            "anonymous_page_limit": 32, "anonymous_zero_fill": True,
            "anonymous_failure_atomic": True,
            "anonymous_target_reclaimed": True,
            "target_baseline_restored": True}:
        raise ValueError("physical allocator process-consumer assertion is false")
    host_value = json.loads(HOST_RECEIPT.read_text())
    row = host_target(host_value)
    expected_host = {
        "path": "kernel/tests/host/test-run-receipt.json",
        "sha256": digest(HOST_RECEIPT),
        "target_id": row["id"],
        "target": row["name"],
        "checks": row["observed_checks"],
    }
    if host_value.get("build_identity") != identity or \
            host_value.get("outcome") != "PASS" or \
            value.get("host_receipt") != expected_host:
        raise ValueError("physical allocator host proof is missing or stale")
    consumer_value = json.loads(USER_PROCESS_RECEIPT.read_text())
    expected_consumer = {
        "path": "kernel/docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json",
        "sha256": digest(USER_PROCESS_RECEIPT),
        "assertions": [
            "process-frame-reclamation",
                "anonymous-reserve-commit-release",
                "released-anonymous-page-fault",
                "process-memory-accounting",
        ],
    }
    consumer_assertions = {
        assertion.get("id") for assertion in consumer_value.get("assertions", [])
    }
    if consumer_value.get("build_identity") != identity or \
            not set(expected_consumer["assertions"]).issubset(consumer_assertions) or \
            value.get("consumer_receipt") != expected_consumer:
        raise ValueError("physical allocator process-consumer proof is missing or stale")
    if value.get("known_gaps") != list(KNOWN_GAPS) or \
            value.get("evidence_ceiling") != EVIDENCE_CEILING:
        raise ValueError("physical allocator receipt hides its open gaps")
    if value.get("generator") != {
            "path": "kernel/tools/checks/write-pmm-receipt.py",
            "sha256": digest(Path(__file__).resolve())}:
        raise ValueError("physical allocator receipt generator drifted")


def selftest(value: dict) -> None:
    mutations = {}
    foreign = copy.deepcopy(value)
    foreign["build_identity"] = "0" * 64
    mutations["foreign-build"] = foreign
    artifact = copy.deepcopy(value)
    artifact["artifact"]["sha256"] = "0" * 64
    mutations["foreign-artifact"] = artifact
    missing = copy.deepcopy(value)
    missing["assertions"].pop()
    mutations["missing-assertion"] = missing
    baseline = copy.deepcopy(value)
    baseline["assertions"][0]["free_pages_after_selftest"] -= 1
    mutations["invented-baseline"] = baseline
    floor = copy.deepcopy(value)
    floor["assertions"][0]["managed_floor_bytes"] = 0
    mutations["invented-reserved-floor"] = floor
    host = copy.deepcopy(value)
    host["host_receipt"]["checks"] = 0
    mutations["lost-host-exhaustion"] = host
    gaps = copy.deepcopy(value)
    gaps["known_gaps"] = []
    mutations["hidden-gaps"] = gaps
    owner = copy.deepcopy(value)
    owner["assertions"][1].pop("wrong_owner_refused_without_mutation")
    mutations["missing-owner-claim"] = owner
    boot_log = copy.deepcopy(value)
    boot_log["boot_log_sha256"] = "0" * 64
    mutations["missing-boot-log-identity"] = boot_log
    generator = copy.deepcopy(value)
    generator["generator"]["sha256"] = "0" * 64
    mutations["foreign-generator"] = generator
    target = copy.deepcopy(value)
    target["host_receipt"]["target_id"] = "HT-000"
    mutations["wrong-host-target"] = target
    consumer = copy.deepcopy(value)
    consumer["consumer_receipt"]["sha256"] = "0" * 64
    mutations["foreign-consumer-receipt"] = consumer
    process_claim = copy.deepcopy(value)
    process_claim["assertions"][4]["target_baseline_restored"] = False
    mutations["lost-process-baseline"] = process_claim
    anonymous_claim = copy.deepcopy(value)
    anonymous_claim["assertions"][4]["anonymous_target_reclaimed"] = False
    mutations["lost-anonymous-reclaim"] = anonymous_claim
    accounting = copy.deepcopy(value)
    accounting["assertions"][2]["metadata_owner_sum_invariant"] = False
    mutations["invented-owner-accounting"] = accounting
    substituted_gap = copy.deepcopy(value)
    substituted_gap["known_gaps"][0] = "all memory is supported"
    mutations["substituted-gap"] = substituted_gap
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except (OSError, ValueError, TypeError, json.JSONDecodeError):
            caught.append(name)
        else:
            raise ValueError(f"physical allocator receipt mutation escaped: {name}")
    print("physical allocator receipt selftest: caught " + ", ".join(caught))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--log", type=Path)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if args.check == bool(args.log):
        parser.error("choose exactly one of --check or --log")
    try:
        value = (json.loads(args.output.read_text()) if args.check
                 else build(args.log))
        validate(value)
        if args.selftest:
            selftest(value)
        if args.check:
            print(f"physical allocator QEMU receipt: PASS_CURRENT -> {args.output}")
            return 0
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.NamedTemporaryFile(
                "w", dir=args.output.parent, delete=False, encoding="utf-8") as handle:
            json.dump(value, handle, indent=2)
            handle.write("\n")
            temporary = Path(handle.name)
        os.replace(temporary, args.output)
        print(f"physical allocator QEMU receipt: PASS -> {args.output}")
        return 0
    except (OSError, ValueError, TypeError, json.JSONDecodeError) as error:
        print(f"physical allocator QEMU receipt: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
