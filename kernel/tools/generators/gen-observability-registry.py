#!/usr/bin/env python3
"""Record current crash/log/event evidence and its missing guarantees."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
METADATA = KERNEL_ROOT / "metadata"
OUTPUT = METADATA / "observability-registry.json"
BOOT_RECEIPTS = (
    "docs/receipts/app-manifest-raw-bios-qemu-2026-08-22.json",
    "docs/receipts/app-manifest-native-uefi64-qemu-2026-08-22.json",
    "docs/receipts/app-manifest-grub-bios32-qemu-2026-08-22.json",
    "docs/receipts/app-manifest-grub-uefi32-qemu-2026-08-22.json",
    "docs/receipts/app-manifest-grub-bios64-qemu-2026-08-22.json",
    "docs/receipts/app-manifest-grub-uefi64-qemu-2026-08-22.json",
)
CRASH_RECEIPT = "docs/receipts/cpu-fault-invalid-opcode-qemu-2026-08-23.json"

CAPABILITIES = (
    ("hashed-boot-transcript", "QEMU_HASH_ONLY", "six route receipts carry boot-log SHA-256 but not durable raw logs"),
    ("heap-corruption-diagnostic", "HOST_PROVED_LIMITED", "heap refuses after detected corruption and prints once; no crash bundle"),
    ("cpu-fault-stop", "QEMU_PROVED", "real UD2 reaches vector 6, emits a complete record and remains halted"),
    ("fault-register-capture", "QEMU_PROVED_PARTIAL", "vector/error/IP/CS/FLAGS/SP/SS/CR2 control frame only; no general registers"),
    ("stack-symbolization", "MISSING", "no unwind table, symbols or stack trace"),
    ("durable-crash-dump", "MISSING", "no reserved recorder, disk dump, checksum or recovery reader"),
    ("crash-service", "MISSING", "no offender-only collection, UI or submission workflow"),
    ("panic-recovery-policy", "MISSING", "fault path halts forever; no bounded reboot/rescue policy"),
    ("typed-event-schema", "HOST_PROVED_CORE", "versioned 152-byte pointer-free feature/process/authority/correlation envelope; not target integrated"),
    ("monotonic-event-order", "HOST_PROVED_CORE", "accepted records receive strict sequence order and nondecreasing caller-supplied monotonic time; single-owner host proof only"),
    ("drop-and-truncation-evidence", "HOST_PROVED_CORE", "full queue never overwrites and next successful emit records exact pending drops; no target/service proof"),
    ("redaction-privacy-policy", "HOST_PROVED_CORE", "four privacy classes and per-word redaction commit zeroes; no target policy owner"),
    ("tamper-evident-anchor", "MISSING", "no append-only anchor, rotation identity or deletion evidence"),
    ("trace-export-import", "HOST_PROVED_PARTIAL", "stable little-endian checksum round-trip preserves boot/process/authority/correlation identity but carries no artifact identity"),
)


def sha256(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.observability-registry.v1":
        raise ValueError("wrong observability schema")
    if value.get("result") != "PASS_WITH_CURRENT_QEMU_AND_HOST_SCHEMA_EVIDENCE_AND_OPEN_GAPS":
        raise ValueError("observability registry overpromoted current evidence")
    rows = value.get("capabilities", [])
    if [row.get("id") for row in rows] != [row[0] for row in CAPABILITIES]:
        raise ValueError("observability capability set/order drift")
    if [row.get("status") for row in rows] != [row[1] for row in CAPABILITIES]:
        raise ValueError("observability status drift")
    if [row.get("detail") for row in rows] != [row[2] for row in CAPABILITIES]:
        raise ValueError("observability claim detail drift")
    expected_counts = {
        "capabilities": 14,
        "qemu_hash_only": 1,
        "qemu_proved": 1,
        "qemu_proved_partial": 1,
        "host_proved_limited": 1,
        "host_proved_core": 4,
        "host_proved_partial": 1,
        "source_only": 0,
        "missing": 5,
        "qemu_crash_receipts": 1,
        "durable_crash_receipts": 0,
        "typed_structured_event_fields": 28,
        "current_build_bound_qemu_receipts": 7,
        "historical_qemu_receipts": 0,
    }
    if value.get("counts") != expected_counts:
        raise ValueError("observability counts drift")
    expected_gaps = [row[0] for row in CAPABILITIES
                     if row[1] not in ("QEMU_HASH_ONLY", "QEMU_PROVED")]
    if value.get("open_gaps") != expected_gaps:
        raise ValueError("observability gaps were hidden")
    boot = value.get("boot_transcripts", [])
    if len(boot) != 6 or any(len(row.get("boot_log_sha256", "")) != 64 for row in boot):
        raise ValueError("boot transcript identities missing")
    if any(row.get("raw_log_preserved") is not False for row in boot):
        raise ValueError("unearned raw boot-log preservation claim")
    if any(row.get("current_build_bound") is not True \
           or len(row.get("subject_build_identity", "")) != 64 for row in boot):
        raise ValueError("current boot transcript binding was lost")
    if value.get("qemu_crash_receipt", {}).get("current_build_bound") is not True:
        raise ValueError("current crash receipt binding was lost")
    if len(value.get("build_identity", "")) != 64:
        raise ValueError("observability build identity missing")


def build() -> dict:
    identity = json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"]
    event_schema_path = METADATA / "event-schema.json"
    event_schema = json.loads(event_schema_path.read_text())
    if event_schema.get("result") != "PASS_CURRENT_SCHEMA_WITH_CURRENT_HOST_PROOF_TARGET_UNINTEGRATED" or \
            event_schema.get("build_identity") != identity or \
            event_schema.get("counts", {}).get("wire_fields") != 28 or \
            event_schema.get("counts", {}).get("target_emitters") != 0:
        raise ValueError("structured event schema is missing, stale or overpromoted")
    transcripts = []
    for relative in BOOT_RECEIPTS:
        path = KERNEL_ROOT / relative
        receipt = json.loads(path.read_text())
        subject_identity = receipt.get("shipped_build_identity", {}).get("id")
        if receipt.get("result") != "PASS" or len(subject_identity or "") != 64:
            raise ValueError(f"{relative}: invalid historical boot receipt")
        transcripts.append({
            "path": "kernel/" + relative,
            "receipt_sha256": sha256(path),
            "route": receipt["route"],
            "boot_log_sha256": receipt["boot_log_sha256"],
            "raw_log_preserved": False,
            "evidence_ceiling": receipt["evidence"],
            "subject_build_identity": subject_identity,
            "current_build_bound": subject_identity == identity,
        })
    host_receipt_path = KERNEL_ROOT / "tests/host/test-run-receipt.json"
    host_receipt = json.loads(host_receipt_path.read_text())
    passed = {row["name"] for row in host_receipt["results"] if row.get("status") == "passed"}
    if not {"heaptest", "crashtest"}.issubset(passed):
        raise ValueError("heap/crash host proof is not passing")
    crash_path = KERNEL_ROOT / CRASH_RECEIPT
    crash = json.loads(crash_path.read_text())
    if crash.get("result") != "PASS" or len(crash.get("build_identity", "")) != 64:
        raise ValueError("QEMU crash receipt is missing or invalid")
    record = crash.get("record", {})
    if record.get("vector") != 6 or record.get("has_error") != 0 \
            or record.get("ip") != crash.get("kernel_symbol_ip") \
            or crash.get("guest_halted_after_record") is not True:
        raise ValueError("QEMU crash receipt does not prove exact fault capture")
    idt_source = (KERNEL_ROOT / "src/arch/x86/idt.c").read_text()
    crash_source = (KERNEL_ROOT / "src/core/crash.c").read_text()
    if "static void fault_stop" not in idt_source or "FAULT_ERR(14)" not in idt_source \
            or "crash_capture" not in idt_source or "crash_report" not in idt_source \
            or "last_record.magic = CRASH_RECORD_MAGIC" not in crash_source:
        raise ValueError("fault-record source boundary drift")
    ceilings = {
        "QEMU_HASH_ONLY": "current-build QEMU receipt hash only",
        "QEMU_PROVED": "current-build exact QEMU target behavior",
        "QEMU_PROVED_PARTIAL": "current-build exact QEMU target behavior, explicitly partial fields",
        "HOST_PROVED_LIMITED": "historical host behavior only",
        "HOST_PROVED_CORE": "current-build host-proved core, explicitly target-unintegrated",
        "HOST_PROVED_PARTIAL": "historical host-proved partial behavior",
        "SOURCE_ONLY": "source inspection only",
        "MISSING": "no current evidence",
    }
    rows = [
        {"id": capability_id, "status": status, "detail": detail,
         "evidence_ceiling": ceilings[status]}
        for capability_id, status, detail in CAPABILITIES
    ]
    value = {
        "schema": "zlos.observability-registry.v1",
        "result": "PASS_WITH_CURRENT_QEMU_AND_HOST_SCHEMA_EVIDENCE_AND_OPEN_GAPS",
        "build_identity": identity,
        "boot_transcripts": transcripts,
        "host_test_receipt": {"path": "kernel/tests/host/test-run-receipt.json", "sha256": sha256(host_receipt_path),
                              "subject_head": host_receipt.get("git", {}).get("head"),
                              "current_build_bound": False},
        "qemu_crash_receipt": {"path": "kernel/" + CRASH_RECEIPT,
                               "sha256": sha256(crash_path),
                               "subject_build_identity": crash["build_identity"],
                               "current_build_bound": crash["build_identity"] == identity},
        "event_schema": {"path": "kernel/metadata/event-schema.json",
                         "sha256": sha256(event_schema_path),
                         "result": event_schema["result"],
                         "target_emitters": 0},
        "source_identities": {
            path: sha256(KERNEL_ROOT / path)
            for path in (
                "src/arch/x86/idt.c", "src/core/crash.c", "src/core/crash.h",
                "src/core/heap.c", "src/core/console.c", "src/arch/x86/support.c",
                "src/graphics/windowing/term.c", "tools/checks/verify-crash.py",
                "tests/host/trace_event.c", "tests/host/trace_event.h",
                "tools/generators/gen-event-schema.py",
                "tools/checks/verify-event-trace.py",
            )
        },
        "capabilities": rows,
        "counts": {
            "capabilities": len(rows),
            "qemu_hash_only": 1,
            "qemu_proved": 1,
            "qemu_proved_partial": 1,
            "host_proved_limited": 1,
            "host_proved_core": 4,
            "host_proved_partial": 1,
            "source_only": 0,
            "missing": 5,
            "qemu_crash_receipts": 1,
            "durable_crash_receipts": 0,
            "typed_structured_event_fields": 28,
            "current_build_bound_qemu_receipts": len(transcripts) + 1,
            "historical_qemu_receipts": 0,
        },
        "open_gaps": [row["id"] for row in rows
                      if row["status"] not in ("QEMU_HASH_ONLY", "QEMU_PROVED")],
        "evidence_ceiling": "current source inventory joined to current-build QEMU boot/crash and event-schema host evidence; no target event emitter or durable crash evidence",
        "weakest_link": "the structured core is externally serialized and target-unintegrated; general registers, stack symbols and durable recovery remain absent",
        "generator": {"path": "kernel/tools/generators/gen-observability-registry.py", "sha256": sha256(Path(__file__).resolve())},
    }
    validate(value)
    return value


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["capabilities"].pop()
    mutations["missing-capability"] = missing
    frame = copy.deepcopy(value)
    next(row for row in frame["capabilities"] if row["id"] == "fault-register-capture")["status"] = "QEMU_PROVED"
    mutations["overpromoted-fault-frame"] = frame
    detail = copy.deepcopy(value)
    next(row for row in detail["capabilities"] if row["id"] == "fault-register-capture")["detail"] = "all registers captured"
    mutations["invented-general-registers"] = detail
    raw = copy.deepcopy(value)
    raw["boot_transcripts"][0]["raw_log_preserved"] = True
    mutations["invented-raw-log"] = raw
    binding = copy.deepcopy(value)
    binding["boot_transcripts"][0]["current_build_bound"] = False
    mutations["lost-current-boot-proof"] = binding
    typed = copy.deepcopy(value)
    typed["counts"]["typed_structured_event_fields"] = 29
    mutations["event-schema-field-drift"] = typed
    hidden = copy.deepcopy(value)
    hidden["open_gaps"].remove("durable-crash-dump")
    mutations["hidden-crash-gap"] = hidden
    caught = []
    for name, mutated in mutations.items():
        try:
            validate(mutated)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"observability selftest mutation escaped: {name}")
    print("observability selftest: caught " + ", ".join(caught))


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
        if args.check and (not OUTPUT.is_file() or json.loads(OUTPUT.read_text()) != value):
            raise ValueError("observability-registry.json is missing or stale")
        print(
            f"observability: {value['result']}: "
            f"{value['counts']['qemu_proved']} QEMU-proved, "
            f"{value['counts']['qemu_proved_partial']} QEMU-partial, "
            f"{value['counts']['qemu_hash_only']} QEMU-hash, "
            f"{value['counts']['host_proved_limited']} host-limited, "
            f"{value['counts']['missing']} missing"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"observability: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
