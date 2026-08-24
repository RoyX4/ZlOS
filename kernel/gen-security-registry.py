#!/usr/bin/env python3
"""Record current security claims, enforcement evidence and residual risk."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
OUTPUT = HERE / "security-registry.json"

CLAIMS = (
    ("non-rwx-kernel-images", "STATIC_PROVED", "kernel code/data pages", "writable executable LOAD segment", "linker PHDR flags + ELF verifier", (), "planted RWX mutation", "page-table enforcement after boot is not separately proved"),
    ("filesystem-integrity", "HOST_PROVED_LIMITED", "zlfs metadata and file contents", "corrupt media, arithmetic wrap, transient write", "zlfs mount validation and transactional metadata update", ("fstest",), "corrupt superblock/geometry/write rollback cases", "no power-cut target receipt, permissions or multi-process adversary"),
    ("network-parser-bounds", "HOST_PROVED_LIMITED", "kernel memory and network state", "malformed/truncated/looping packets", "bounded packet parsers and protocol state machines", ("fuzz", "dnstest", "nettest", "tcptest", "httptest"), "deterministic fuzz and scripted hostile packets", "host parsers are not process-isolated and not every protocol is covered"),
    ("tls-x509-authenticity", "HOST_PROVED_LIMITED", "peer identity and transport plaintext", "forged chain, wrong host/time, malformed certificate", "TLS record/handshake and X.509 chain/hostname validation", ("x509test", "tlstest", "tlscryptotest"), "forged root, truncation, hostname/time refusal", "root distribution, revocation, persistent secrets and production policy remain incomplete"),
    ("interpreter-resource-bounds", "HOST_PROVED_LIMITED", "kernel responsiveness and interpreter memory", "runaway steps/depth/memory access", "interpreter step/depth/confine checks", ("killtest.sh",), "budget/confine hostile programs", "this is an interpreter harness, not general process containment"),
    ("ring3-address-space-isolation", "QEMU_BOOT_REACHABLE_PARTIAL", "kernel and sibling process memory", "user process reads/writes privileged memory", "paging privilege bits and Ring-3 transition", (), "boot reaches the cooperative Ring-3 smoke stage", "no general preemptive process/usercopy hostile matrix or physical proof"),
    ("syscall-usercopy-boundary", "MISSING", "kernel memory and caller buffers", "negative syscall index, invalid input/output pointer, copy fault", "not implemented", (), "none", "no exhaustive syscall/usercopy hostile receipt"),
    ("capability-handles-revocation", "MISSING", "kernel objects and delegated authority", "forged/stale handle, over-delegation, incomplete revoke", "not implemented", (), "none", "capability handle table and derivation/revoke graph do not exist"),
    ("credential-transitions", "MISSING", "user/group identity and process control", "UID/GID escalation or corrupted fork credentials", "not implemented", (), "none", "credential model and transition matrix do not exist"),
    ("typed-ipc-authorization", "MISSING", "service endpoints, messages and transferred handles", "unauthorized sender, pointer smuggling, queue exhaustion", "not implemented", (), "none", "current PID IPC is not the bounded typed capability transport"),
    ("dma-iommu-isolation", "MISSING", "physical memory outside a device grant", "device DMA escapes granted pages", "not implemented", (), "none", "no enforced IOMMU domain; passthrough cannot be called isolated"),
    ("secret-storage-redaction", "MISSING", "passwords, keys, tokens and logs", "secret disclosure at rest/in memory/logs", "not implemented", (), "none", "no secrets service, zeroization or redaction receipt"),
    ("boot-authenticity-rollback", "MISSING", "selected kernel/configuration and boot policy", "tampered or rolled-back boot artifact", "not implemented", (), "none", "byte identity is not signature, measured boot or anti-rollback"),
    ("application-sandbox", "MISSING", "user files, services, devices and sibling apps", "compromised app exercises ambient authority", "not implemented", (), "none", "manifest/portal sandbox and complete mediation do not exist"),
    ("package-signature-install-atomicity", "MISSING", "installed files, services, grants and live processes", "malicious or partially installed package", "not implemented", (), "none", "no publisher trust, staged atomic install or uninstall revocation"),
    ("audit-integrity", "MISSING", "security event history and attribution", "event deletion, truncation, forgery or secret leak", "not implemented", (), "none", "no durable append/anchor/drop/redaction security audit service"),
    ("privileged-operation-mediation", "MISSING", "MMIO, mount, scanout/input, kill/debug and power", "ordinary process invokes privileged operation", "not implemented", (), "none", "no complete operation-rights matrix and denial suite"),
)


def sha256(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.security-claim-registry.v1":
        raise ValueError("wrong security registry schema")
    if value.get("result") != "PASS_WITH_OPEN_GAPS":
        raise ValueError("security registry overpromoted current claims")
    rows = value.get("claims", [])
    if [row.get("id") for row in rows] != [row[0] for row in CLAIMS]:
        raise ValueError("security claim set/order drift")
    if [row.get("status") for row in rows] != [row[1] for row in CLAIMS]:
        raise ValueError("security claim status drift")
    for row in rows:
        for field in ("protected_asset", "threat", "enforcement_point", "adversarial_proof", "residual_risk"):
            if not row.get(field):
                raise ValueError(f"{row.get('id')}: missing {field}")
        if row.get("status") == "MISSING" and row.get("adversarial_proof") != "none":
            raise ValueError(f"{row.get('id')}: missing claim has invented proof")
    expected_counts = {
        "claims": 17,
        "static_proved": 1,
        "host_proved_limited": 4,
        "qemu_boot_reachable_partial": 1,
        "missing": 11,
        "production_complete": 0,
        "native_hardware_complete": 0,
    }
    if value.get("counts") != expected_counts:
        raise ValueError("security claim counts drift")
    if value.get("open_claims") != [row[0] for row in CLAIMS]:
        raise ValueError("security residual-risk set was hidden")
    if len(value.get("build_identity", "")) != 64:
        raise ValueError("security registry build identity missing")


def build() -> dict:
    receipt_path = HERE / "hosttest/test-run-receipt.json"
    receipt = json.loads(receipt_path.read_text())
    passed = {row["name"] for row in receipt["results"] if row.get("status") == "passed"}
    artifact = json.loads((HERE / "artifact-registry.json").read_text())
    init = json.loads((HERE / "init-registry.json").read_text())
    if any(row.get("memory_permissions") != "NO_RWX_LOAD" for row in artifact["artifacts"].values() if row["path"].endswith(".elf")):
        raise ValueError("non-RWX source claim lost its artifact evidence")
    if not any(row["id"] == "INIT-013" and row["name"] == "ring3-smoke" for row in init["stages"]):
        raise ValueError("Ring-3 boot-reachability source disappeared")
    rows = []
    for claim_id, status, asset, threat, enforcement, targets, adversarial, residual in CLAIMS:
        missing = sorted(set(targets) - passed)
        if missing:
            raise ValueError(f"{claim_id}: mapped host proof is not passing: {missing}")
        rows.append({
            "id": claim_id,
            "status": status,
            "protected_asset": asset,
            "threat": threat,
            "enforcement_point": enforcement,
            "passed_host_targets": list(targets),
            "adversarial_proof": adversarial,
            "residual_risk": residual,
            "evidence_ceiling": (
                "host behavior only" if targets else
                ("QEMU boot reachability only" if status == "QEMU_BOOT_REACHABLE_PARTIAL" else
                 ("static artifact structure only" if status == "STATIC_PROVED" else "no current proof"))
            ),
        })
    value = {
        "schema": "zlos.security-claim-registry.v1",
        "result": "PASS_WITH_OPEN_GAPS",
        "build_identity": json.loads((HERE / "build-identity.json").read_text())["identity_sha256"],
        "input_receipts": {
            "kernel/artifact-registry.json": sha256(HERE / "artifact-registry.json"),
            "kernel/init-registry.json": sha256(HERE / "init-registry.json"),
            "kernel/hosttest/test-run-receipt.json": sha256(receipt_path),
        },
        "claims": rows,
        "counts": {
            "claims": len(rows),
            "static_proved": sum(row["status"] == "STATIC_PROVED" for row in rows),
            "host_proved_limited": sum(row["status"] == "HOST_PROVED_LIMITED" for row in rows),
            "qemu_boot_reachable_partial": sum(row["status"] == "QEMU_BOOT_REACHABLE_PARTIAL" for row in rows),
            "missing": sum(row["status"] == "MISSING" for row in rows),
            "production_complete": 0,
            "native_hardware_complete": 0,
        },
        "open_claims": [row["id"] for row in rows],
        "evidence_ceiling": "claim registry, static checks, host adversarial tests and QEMU boot reachability only",
        "weakest_link": "no claim is production-complete; capability/usercopy/DMA/credential/IPC/sandbox/boot/package/audit mediation is absent",
        "generator": {"path": "kernel/gen-security-registry.py", "sha256": sha256(Path(__file__).resolve())},
    }
    validate(value)
    return value


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["claims"].pop()
    mutations["missing-claim"] = missing
    complete = copy.deepcopy(value)
    complete["counts"]["production_complete"] = 1
    mutations["production-overclaim"] = complete
    hidden = copy.deepcopy(value)
    hidden["open_claims"].remove("dma-iommu-isolation")
    mutations["hidden-dma-risk"] = hidden
    dma = copy.deepcopy(value)
    next(row for row in dma["claims"] if row["id"] == "dma-iommu-isolation")["status"] = "HOST_PROVED_LIMITED"
    mutations["invented-dma-proof"] = dma
    threat = copy.deepcopy(value)
    threat["claims"][0]["protected_asset"] = ""
    mutations["missing-protected-asset"] = threat
    caught = []
    for name, mutated in mutations.items():
        try:
            validate(mutated)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"security-registry selftest mutation escaped: {name}")
    print("security-registry selftest: caught " + ", ".join(caught))


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
            raise ValueError("security-registry.json is missing or stale")
        print(
            "security-registry: PASS_WITH_OPEN_GAPS: "
            f"{value['counts']['static_proved']} static, {value['counts']['host_proved_limited']} host-limited, "
            f"{value['counts']['qemu_boot_reachable_partial']} QEMU-partial, {value['counts']['missing']} missing"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"security-registry: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
