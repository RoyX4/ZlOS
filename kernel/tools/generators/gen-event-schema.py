#!/usr/bin/env python3
"""Generate EV-018's canonical logical/wire contract and reject overclaims."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import tempfile


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
METADATA = KERNEL_ROOT / "metadata"
OUTPUT = METADATA / "event-schema.json"
RECEIPT = KERNEL_ROOT / "docs/receipts/event-trace-host-2026-08-24.json"

FIELDS = (
    ("magic", 0, 4, "u32"),
    ("version", 4, 2, "u16"),
    ("bytes", 6, 2, "u16"),
    ("sequence", 8, 8, "u64"),
    ("monotonic_ns", 16, 8, "u64"),
    ("boot_id", 24, 8, "u64"),
    ("correlation_id", 32, 8, "u64"),
    ("parent_sequence", 40, 8, "u64"),
    ("process_id", 48, 4, "u32"),
    ("process_generation", 52, 4, "u32"),
    ("authority_id", 56, 4, "u32"),
    ("authority_generation", 60, 4, "u32"),
    ("event_code", 64, 4, "u32"),
    ("outcome", 68, 4, "i32"),
    ("severity", 72, 1, "u8-enum"),
    ("privacy", 73, 1, "u8-enum"),
    ("kind", 74, 1, "u8-enum"),
    ("flags", 75, 1, "u8-bits"),
    ("redaction_mask", 76, 1, "u8-bits"),
    ("payload_words", 77, 1, "u8"),
    ("reserved", 78, 2, "u16"),
    ("feature_id", 80, 16, "nul-token[16]"),
    ("component_id", 96, 16, "nul-token[16]"),
    ("payload[0]", 112, 8, "u64"),
    ("payload[1]", 120, 8, "u64"),
    ("payload[2]", 128, 8, "u64"),
    ("payload[3]", 136, 8, "u64"),
    ("checksum", 144, 8, "u64-fnv1a"),
)

ENUMS = {
    "severity": ["DEBUG", "INFO", "NOTICE", "WARNING", "ERROR", "CRITICAL"],
    "privacy": ["PUBLIC", "INTERNAL", "SENSITIVE", "SECRET"],
    "kind": ["STATE", "EFFECT", "SECURITY", "PERFORMANCE", "LIFECYCLE", "DROP"],
}

INVARIANTS = (
    "magic=ZLEV, version=1 and bytes=152",
    "sequence is nonzero and strictly increases for every committed record",
    "monotonic_ns never decreases within one trace",
    "boot_id and correlation_id are nonzero",
    "parent_sequence is zero or names an earlier committed sequence",
    "process and authority identities are absent as 0/0 or present as nonzero id/generation pairs",
    "feature_id and component_id are bounded NUL-terminated tokens",
    "unknown enum values, flags, reserved fields and redaction bits fail closed",
    "SECRET requires all four payload words redacted",
    "every redacted payload word is zero before checksum and commit",
    "checksum covers every field except itself in canonical little-endian value order",
    "a corrupt head is retained and cannot be consumed as valid evidence",
)

OPEN_GAPS = [
    "single-owner-only",
    "not-compiled-into-shipped-kernel",
    "no-target-emitter",
    "no-QEMU-receipt",
    "no-native-hardware-receipt",
    "no-durable-anchor",
    "no-service-export-policy",
]


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.event-schema-registry.v1":
        raise ValueError("wrong event-schema registry version")
    if value.get("result") != "PASS_CURRENT_SCHEMA_WITH_CURRENT_HOST_PROOF_TARGET_UNINTEGRATED":
        raise ValueError("event schema was failed or overpromoted")
    expected_identity = json.loads((METADATA / "build-identity.json").read_text())[
        "identity_sha256"]
    if value.get("build_identity") != expected_identity:
        raise ValueError("event schema has stale build context")
    contract = value.get("contract", {})
    if contract.get("wire") != {
            "endianness": "little", "bytes": 152, "pointer_fields": 0,
            "variable_length_fields": 0, "payload_words": 4}:
        raise ValueError("event wire contract drift")
    fields = contract.get("fields", [])
    expected = [{"name": name, "offset": offset, "bytes": size, "type": kind}
                for name, offset, size, kind in FIELDS]
    if fields != expected:
        raise ValueError("event wire field/order/offset drift")
    if any(fields[i]["offset"] + fields[i]["bytes"] != fields[i + 1]["offset"]
           for i in range(len(fields) - 1)) or \
            fields[-1]["offset"] + fields[-1]["bytes"] != 152:
        raise ValueError("event fields overlap or leave undeclared bytes")
    if contract.get("enums") != ENUMS:
        raise ValueError("event enum contract drift")
    if contract.get("invariants") != list(INVARIANTS):
        raise ValueError("event invariants drift")
    queue = value.get("queue", {})
    if queue != {
            "production_capacity": 64,
            "allocation": "fixed caller-owned storage",
            "ownership": "single serialized owner; no SMP/IRQ safety claim",
            "full_policy": "drop new record; never overwrite committed evidence",
            "drop_evidence": "next successful emit first commits one DROP record with exact pending count",
            "corruption_policy": "fail closed and retain corrupt head for inspection",
    }:
        raise ValueError("event queue semantics drift")
    proof = value.get("proof", {})
    receipt = json.loads(RECEIPT.read_text())
    if proof.get("receipt_path") != "kernel/docs/receipts/event-trace-host-2026-08-24.json" or \
            proof.get("receipt_sha256") != sha256(RECEIPT) or \
            proof.get("host_checks") != 37 or proof.get("host_failures") != 0 or \
            proof.get("compile_lanes") != 3:
        raise ValueError("event-schema host proof is stale")
    if receipt.get("result") != "PASS_HOST_CORE_TARGET_UNINTEGRATED" or \
            receipt.get("target_integration", {}).get("target_emitters") != 0:
        raise ValueError("event-schema receipt overclaims target reachability")
    if proof.get("subject_build_identity") != receipt.get("build_identity") \
            or proof.get("current_build_bound") is not True:
        raise ValueError("current event-trace receipt binding was lost")
    if value.get("counts") != {
            "wire_fields": 28,
            "wire_bytes": 152,
            "payload_words": 4,
            "production_capacity": 64,
            "host_checks": 37,
            "compile_lanes": 3,
            "target_emitters": 0,
            "qemu_routes": 0,
            "native_hardware_routes": 0,
    }:
        raise ValueError("event-schema counts drift")
    expected_sources = {
        path: sha256(KERNEL_ROOT / path) for path in
        ("tests/host/trace_event.h", "tests/host/trace_event.c",
         "tests/host/eventtracetest.c", "tools/checks/verify-event-trace.py")
    }
    if value.get("source_identities") != expected_sources:
        raise ValueError("event-schema source identities drift")
    if value.get("open_gaps") != OPEN_GAPS:
        raise ValueError("event-schema gaps were hidden")


def build() -> dict:
    receipt = json.loads(RECEIPT.read_text())
    value = {
        "schema": "zlos.event-schema-registry.v1",
        "result": "PASS_CURRENT_SCHEMA_WITH_CURRENT_HOST_PROOF_TARGET_UNINTEGRATED",
        "feature_id": "EV-018",
        "build_identity": json.loads((METADATA / "build-identity.json").read_text())[
            "identity_sha256"],
        "contract": {
            "wire": {"endianness": "little", "bytes": 152,
                     "pointer_fields": 0, "variable_length_fields": 0,
                     "payload_words": 4},
            "fields": [
                {"name": name, "offset": offset, "bytes": size, "type": kind}
                for name, offset, size, kind in FIELDS
            ],
            "enums": ENUMS,
            "invariants": list(INVARIANTS),
        },
        "queue": {
            "production_capacity": 64,
            "allocation": "fixed caller-owned storage",
            "ownership": "single serialized owner; no SMP/IRQ safety claim",
            "full_policy": "drop new record; never overwrite committed evidence",
            "drop_evidence": "next successful emit first commits one DROP record with exact pending count",
            "corruption_policy": "fail closed and retain corrupt head for inspection",
        },
        "proof": {
            "receipt_path": "kernel/docs/receipts/event-trace-host-2026-08-24.json",
            "receipt_sha256": sha256(RECEIPT),
            "host_checks": receipt["proof"]["host_execution"]["checks"],
            "host_failures": receipt["proof"]["host_execution"]["failures"],
            "compile_lanes": len(receipt["proof"]["compile_lanes"]),
            "subject_build_identity": receipt["build_identity"],
            "current_build_bound": receipt["build_identity"] == json.loads(
                (METADATA / "build-identity.json").read_text())["identity_sha256"],
            "evidence_ceiling": "current-build host execution and ABI compile receipt; target remains unintegrated",
        },
        "source_identities": {
            path: sha256(KERNEL_ROOT / path) for path in
            ("tests/host/trace_event.h", "tests/host/trace_event.c",
             "tests/host/eventtracetest.c", "tools/checks/verify-event-trace.py")
        },
        "counts": {
            "wire_fields": len(FIELDS),
            "wire_bytes": 152,
            "payload_words": 4,
            "production_capacity": 64,
            "host_checks": receipt["proof"]["host_execution"]["checks"],
            "compile_lanes": len(receipt["proof"]["compile_lanes"]),
            "target_emitters": receipt["target_integration"]["target_emitters"],
            "qemu_routes": receipt["target_integration"]["qemu_routes"],
            "native_hardware_routes": receipt["target_integration"]["native_hardware_routes"],
        },
        "open_gaps": OPEN_GAPS,
        "evidence_ceiling": "current static schema joined to historical host proof; no current-build or shipped target integration",
        "weakest_link": "the core is externally serialized and has zero emitters in a booted zlOS artifact",
        "generator": {"path": "kernel/gen-event-schema.py",
                      "sha256": sha256(Path(__file__).resolve())},
    }
    validate(value)
    return value


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["contract"]["fields"].pop()
    mutations["missing-field"] = missing
    offset = copy.deepcopy(value)
    offset["contract"]["fields"][4]["offset"] -= 1
    mutations["overlapping-field"] = offset
    secret = copy.deepcopy(value)
    secret["contract"]["enums"]["privacy"].remove("SECRET")
    mutations["missing-secret-class"] = secret
    overwrite = copy.deepcopy(value)
    overwrite["queue"]["full_policy"] = "overwrite oldest"
    mutations["hidden-overwrite"] = overwrite
    owner = copy.deepcopy(value)
    owner["queue"]["ownership"] = "lock-free SMP safe"
    mutations["invented-concurrency"] = owner
    promoted = copy.deepcopy(value)
    promoted["counts"]["target_emitters"] = 1
    mutations["invented-target-emitter"] = promoted
    binding = copy.deepcopy(value)
    binding["proof"]["current_build_bound"] = False
    mutations["lost-current-host-proof"] = binding
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"event-schema mutation escaped: {name}")
    print("event-schema selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    with tempfile.NamedTemporaryFile("w", dir=HERE, delete=False,
                                     encoding="utf-8") as handle:
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
        if args.write:
            value = build()
            write_atomic(value)
        else:
            value = json.loads(OUTPUT.read_text())
            validate(value)
            if value != build():
                raise ValueError("event schema is stale")
        if args.selftest:
            selftest(value)
        print(f"event-schema: {value['result']}: "
              f"{value['counts']['wire_fields']} fields, "
              f"{value['counts']['host_checks']} host checks")
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"event-schema: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
