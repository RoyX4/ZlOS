#!/usr/bin/env python3
"""Join application identity and QEMU receipts into one fail-closed registry.

This does not promote a first compositor draw into workflow, image-membership,
package, or physical-hardware proof. It answers one narrower question exactly:
does every current application identity have fresh open/ready/close evidence on
the same tested ISO, with no unaccounted identity in the receipts?
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import sys


HERE = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.dirname(os.path.dirname(HERE))
REPO_ROOT = os.path.dirname(KERNEL_ROOT)
METADATA = os.path.join(KERNEL_ROOT, "metadata")
MANIFEST = os.path.join(METADATA, "app-manifest.json")
BUILD_IDENTITY = os.path.join(METADATA, "build-identity.json")
ISO = os.path.join(KERNEL_ROOT, "zlOS.iso")
OUTPUT = os.path.join(METADATA, "app-evidence.json")
RECEIPTS = {
    "routes": os.path.join(KERNEL_ROOT, "docs", "receipts", "app-routes-qemu-2026-08-22.json"),
    "catalogue": os.path.join(KERNEL_ROOT, "docs", "receipts", "app-lifecycle-qemu-2026-08-22.json"),
    "run": os.path.join(KERNEL_ROOT, "docs", "receipts", "run-qemu-2026-08-22.json"),
}
BOOT_RECEIPTS = {
    "raw-bios": os.path.join(KERNEL_ROOT, "docs", "receipts", "app-manifest-raw-bios-qemu-2026-08-22.json"),
    "native-uefi64": os.path.join(KERNEL_ROOT, "docs", "receipts", "app-manifest-native-uefi64-qemu-2026-08-22.json"),
    "grub-bios32": os.path.join(KERNEL_ROOT, "docs", "receipts", "app-manifest-grub-bios32-qemu-2026-08-22.json"),
    "grub-uefi32": os.path.join(KERNEL_ROOT, "docs", "receipts", "app-manifest-grub-uefi32-qemu-2026-08-22.json"),
    "grub-bios64": os.path.join(KERNEL_ROOT, "docs", "receipts", "app-manifest-grub-bios64-qemu-2026-08-22.json"),
    "grub-uefi64": os.path.join(KERNEL_ROOT, "docs", "receipts", "app-manifest-grub-uefi64-qemu-2026-08-22.json"),
}


def fail(message):
    raise ValueError(message)


def load(path):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def repo_path(relative, label):
    if os.path.isabs(relative) or relative == ".." or relative.startswith("../"):
        fail(f"{label} path escapes repository: {relative}")
    return os.path.join(REPO_ROOT, relative)


def check_source_hashes(receipt_name, receipt):
    hashes = receipt.get("source_files_sha256")
    if not isinstance(hashes, dict) or not hashes:
        fail(f"{receipt_name}: missing source hashes")
    for relative, expected in hashes.items():
        path = repo_path(relative, f"{receipt_name}: source")
        if not os.path.isfile(path):
            fail(f"{receipt_name}: source disappeared: {relative}")
        actual = sha256(path)
        if actual != expected:
            fail(f"{receipt_name}: stale source {relative}: {expected} != {actual}")


def cycle_errors(opened, ready, closed, app):
    errors = []
    for label, event in (("open", opened), ("ready", ready), ("close", closed)):
        if not isinstance(event, dict):
            errors.append(f"missing {label}")
            continue
        if event.get("event") != label:
            errors.append(f"{label} event is {event.get('event')!r}")
        if event.get("app") != app:
            errors.append(f"{label} app is {event.get('app')!r}")
    if errors:
        return errors
    identity = {(event["slot"], event["generation"])
                for event in (opened, ready, closed)}
    if len(identity) != 1:
        errors.append(f"slot/generation changed: {sorted(identity)}")
    if opened["live"] != ready["live"]:
        errors.append("live count changed before close")
    if closed["live"] != opened["live"] - 1:
        errors.append("close did not release exactly one live window")
    return errors


def add_record(evidence, known, item, route, require_close=True):
    app = item.get("id")
    name = item.get("name")
    if app not in known:
        fail(f"{route}: receipt names unknown app id {app}")
    if known[app]["name"] != name:
        fail(f"{route}: id {app} is {name!r}, expected {known[app]['name']!r}")
    opened, ready, closed = item.get("open"), item.get("ready"), item.get("close")
    if not isinstance(opened, dict) or opened.get("app") != app:
        fail(f"{route}: id {app} lacks its open event")
    if not isinstance(ready, dict) or ready.get("app") != app:
        fail(f"{route}: id {app} lacks its ready event")
    if ((opened.get("slot"), opened.get("generation")) !=
            (ready.get("slot"), ready.get("generation"))):
        fail(f"{route}: id {app} changed identity before first draw")
    if require_close:
        errors = cycle_errors(opened, ready, closed, app)
        if errors:
            fail(f"{route}: id {app}: " + "; ".join(errors))
    evidence[app].append({
        "route": route,
        "open": opened,
        "ready": ready,
        "close": closed,
    })


def build(manifest, receipts, boot_receipts, verify_files=True, verify_artifact=False):
    entries = manifest.get("entries", [])
    known = {entry.get("id"): entry for entry in entries}
    if len(entries) != 62 or len(known) != 62:
        fail(f"manifest must contain 61 named apps plus catalogue; got {len(entries)}")
    if len({entry.get("name") for entry in entries}) != 62:
        fail("manifest contains duplicate application names")

    artifact_hashes = []
    source_heads = []
    shipped_digests = []
    expected_manifest_digest = sha256(MANIFEST)
    expected_build = load(BUILD_IDENTITY)
    for name, receipt in receipts.items():
        artifact = receipt.get("artifact", {})
        digest = artifact.get("sha256")
        if not digest:
            fail(f"{name}: missing artifact hash")
        artifact_hashes.append(digest)
        source_heads.append(receipt.get("source_head"))
        shipped = receipt.get("shipped_manifest", {})
        shipped_value = (shipped.get("schema"), shipped.get("entries"),
                         shipped.get("sha256"))
        if shipped_value != (1, 62, expected_manifest_digest):
            fail(f"{name}: shipped manifest mismatch: {shipped_value!r}")
        shipped_digests.append(shipped.get("sha256"))
        shipped_build = receipt.get("shipped_build_identity", {})
        if (shipped_build.get("schema"), shipped_build.get("id"),
                shipped_build.get("head"), shipped_build.get("dirty")) != (
                1, expected_build["identity_sha256"], expected_build["git"]["head"],
                expected_build["git"]["dirty"]):
            fail(f"{name}: shipped build identity mismatch")
        if verify_files:
            check_source_hashes(name, receipt)
    if len(set(artifact_hashes)) != 1:
        fail(f"receipts cover different artifacts: {sorted(set(artifact_hashes))}")
    if len(set(source_heads)) != 1:
        fail(f"receipts name different source heads: {sorted(set(source_heads))}")
    if verify_artifact and sha256(ISO) != artifact_hashes[0]:
        fail("current zlOS.iso does not match the receipt artifact")

    runtime_boot_routes = []
    expected_origins = {
        "raw-bios": "our bootloader (raw_boot), no GRUB",
        "native-uefi64": "UEFI application - no GRUB, no bootloader",
        "grub-bios32": "multiboot handoff, 32-bit protected mode",
        "grub-uefi32": "multiboot handoff, 32-bit protected mode",
        "grub-bios64": "multiboot handoff, then OUR jump into 64-bit long mode",
        "grub-uefi64": "multiboot handoff, then OUR jump into 64-bit long mode",
    }
    if set(boot_receipts) != set(expected_origins):
        fail(f"boot receipt routes differ: {sorted(boot_receipts)}")
    for route, receipt in boot_receipts.items():
        if receipt.get("schema") != "zlos.application-manifest-boot-receipt.v1":
            fail(f"{route}: wrong boot-receipt schema")
        if receipt.get("route") != route or receipt.get("result") != "PASS":
            fail(f"{route}: boot receipt did not pass as its named route")
        if receipt.get("boot_origin") != expected_origins[route]:
            fail(f"{route}: boot origin mismatch")
        shipped = receipt.get("shipped_manifest", {})
        if (shipped.get("schema"), shipped.get("entries"), shipped.get("sha256")) != (
                1, 62, expected_manifest_digest):
            fail(f"{route}: booted artifact reported the wrong manifest")
        shipped_build = receipt.get("shipped_build_identity", {})
        if (shipped_build.get("schema"), shipped_build.get("id"),
                shipped_build.get("head"), shipped_build.get("dirty")) != (
                1, expected_build["identity_sha256"], expected_build["git"]["head"],
                expected_build["git"]["dirty"]):
            fail(f"{route}: booted artifact reported the wrong build identity")
        if verify_files:
            check_source_hashes(route, receipt)
        artifact = receipt.get("artifact", {})
        if not artifact.get("sha256") or not artifact.get("path"):
            fail(f"{route}: missing artifact identity")
        if verify_artifact:
            artifact_path = repo_path(artifact["path"], f"{route}: artifact")
            if sha256(artifact_path) != artifact["sha256"]:
                fail(f"{route}: current artifact does not match its runtime receipt")
        runtime_boot_routes.append({
            "route": route,
            "artifact": artifact,
            "shipped_manifest_sha256": expected_manifest_digest,
            "shipped_build_identity": expected_build["identity_sha256"],
            "evidence": receipt.get("evidence"),
        })

    evidence = {app: [] for app in known}
    route_result = receipts["routes"].get("result", {})
    for item in route_result.get("boot", []):
        add_record(evidence, known, item, "boot", require_close=False)
    for item in route_result.get("dock", []):
        add_record(evidence, known, item, f"dock:{item.get('slot')}")
    for item in route_result.get("shell_word", []):
        add_record(evidence, known, item, f"shell:{item.get('word')}")
    for item in route_result.get("surface", []):
        add_record(evidence, known, item, f"surface:{item.get('route')}")

    catalogue_result = receipts["catalogue"].get("result", {})
    cycles = catalogue_result.get("cycles", [])
    if catalogue_result.get("passed") != 47 or catalogue_result.get("failed") != 0:
        fail("catalogue receipt does not report 47 passed and 0 failed")
    for item in cycles:
        add_record(evidence, known, item, f"catalogue:{item.get('catalog_index')}")

    run_result = receipts["run"].get("result", {})
    run_app = run_result.get("app", {})
    run_lifecycle = run_result.get("lifecycle", {})
    run_item = {
        "id": run_app.get("id"),
        "name": run_app.get("name"),
        "open": run_lifecycle.get("open"),
        "ready": run_lifecycle.get("ready"),
        "close": run_lifecycle.get("close"),
    }
    add_record(evidence, known, run_item, "shell:run")
    duplicates = run_result.get("duplicate_open_events", {})
    if duplicates.get("before") != duplicates.get("after"):
        fail("Run receipt permits a duplicate window")
    if run_result.get("assertions_failed") != 0:
        fail("Run receipt contains failed assertions")

    output_entries = []
    for app in sorted(known):
        records = evidence[app]
        if not records:
            fail(f"{known[app]['name']} id {app} has no QEMU route evidence")
        if not any(record["ready"] for record in records):
            fail(f"{known[app]['name']} id {app} has no first-draw evidence")
        if not any(record["close"] for record in records):
            fail(f"{known[app]['name']} id {app} has no teardown evidence")
        output_entries.append({
            "id": app,
            "name": known[app]["name"],
            "kind": known[app]["kind"],
            "migration_phase": known[app]["migration_phase"],
            "routes": records,
            "claims": {
                "source_identity": True,
                "build_reachable": True,
                "qemu_open": True,
                "qemu_first_draw": True,
                "qemu_close_no_window_leak": True,
                "workflow_complete": False,
                "shipped_manifest_member": True,
                "physical_hardware": False,
            },
        })

    return {
        "schema": "zlos.application-evidence-registry.v1",
        "artifact": {"path": "kernel/zlOS.iso", "sha256": artifact_hashes[0]},
        "source_head": source_heads[0],
        "evidence_ceiling": "source/build plus boot-embedded manifest and QEMU route, first draw and teardown; no complete-workflow or physical proof",
        "shipped_manifest": {
            "schema": 1,
            "entries": 62,
            "sha256": shipped_digests[0],
        },
        "shipped_build_identity": {
            "schema": 1,
            "id": expected_build["identity_sha256"],
            "head": expected_build["git"]["head"],
            "dirty": expected_build["git"]["dirty"],
        },
        "runtime_manifest_boot_routes": runtime_boot_routes,
        "counts": {
            "identities": len(output_entries),
            "named_implementations": 61,
            "catalogue_surfaces": 1,
            "games": sum(entry["kind"] == "game" for entry in output_entries),
            "with_qemu_open_ready_close": len(output_entries),
            "runtime_manifest_boot_routes": len(runtime_boot_routes),
        },
        "entries": output_entries,
    }


def serialized(value):
    return json.dumps(value, indent=2, sort_keys=True, ensure_ascii=False) + "\n"


def selftest(manifest, receipts, boot_receipts):
    try:
        check_source_hashes("escape", {"source_files_sha256": {"../escape": "0" * 64}})
    except ValueError:
        print("app-evidence source selftest: caught path-escape")
    else:
        fail("selftest mutation escaped: path-escape")
    mutations = []
    mismatched = copy.deepcopy(receipts)
    mismatched["run"]["artifact"]["sha256"] = "0" * 64
    mutations.append(("artifact-mismatch", mismatched))
    missing_ready = copy.deepcopy(receipts)
    missing_ready["catalogue"]["result"]["cycles"][0]["ready"] = None
    mutations.append(("missing-ready", missing_ready))
    missing_identity = copy.deepcopy(receipts)
    missing_identity["catalogue"]["result"]["cycles"].pop()
    mutations.append(("unproved-identity", missing_identity))
    wrong_name = copy.deepcopy(receipts)
    wrong_name["routes"]["result"]["dock"][0]["name"] = "Not Terminal"
    mutations.append(("identity-drift", wrong_name))
    duplicate_run = copy.deepcopy(receipts)
    duplicate_run["run"]["result"]["duplicate_open_events"]["after"] += 1
    mutations.append(("duplicate-run", duplicate_run))
    missing_shipped = copy.deepcopy(receipts)
    del missing_shipped["routes"]["shipped_manifest"]
    mutations.append(("missing-shipped-manifest", missing_shipped))
    wrong_build = copy.deepcopy(receipts)
    wrong_build["routes"]["shipped_build_identity"]["id"] = "e" * 64
    mutations.append(("wrong-build-identity", wrong_build))
    caught = []
    for name, mutant in mutations:
        try:
            build(manifest, mutant, boot_receipts, verify_files=False)
        except ValueError:
            caught.append(name)
        else:
            fail(f"selftest mutation escaped: {name}")
    print("app-evidence selftest: caught " + ", ".join(caught))
    wrong_boot = copy.deepcopy(boot_receipts)
    wrong_boot["raw-bios"]["shipped_manifest"]["sha256"] = "f" * 64
    try:
        build(manifest, receipts, wrong_boot, verify_files=False)
    except ValueError:
        print("app-evidence boot selftest: caught wrong-boot-manifest")
    else:
        fail("selftest mutation escaped: wrong-boot-manifest")
    wrong_boot_build = copy.deepcopy(boot_receipts)
    wrong_boot_build["native-uefi64"]["shipped_build_identity"]["id"] = "d" * 64
    try:
        build(manifest, receipts, wrong_boot_build, verify_files=False)
    except ValueError:
        print("app-evidence boot selftest: caught wrong-boot-build-identity")
    else:
        fail("selftest mutation escaped: wrong-boot-build-identity")


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--verify-artifact", action="store_true",
                        help="also require the current zlOS.iso to be the tested artifact")
    args = parser.parse_args(argv)
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        manifest = load(MANIFEST)
        receipts = {name: load(path) for name, path in RECEIPTS.items()}
        boot_receipts = {name: load(path) for name, path in BOOT_RECEIPTS.items()}
        value = build(manifest, receipts, boot_receipts,
                      verify_artifact=args.verify_artifact)
        expected = serialized(value)
        if args.write:
            temp = OUTPUT + ".tmp"
            with open(temp, "w", encoding="utf-8") as handle:
                handle.write(expected)
            os.replace(temp, OUTPUT)
        else:
            with open(OUTPUT, encoding="utf-8") as handle:
                actual = handle.read()
            if actual != expected:
                fail("app-evidence.json is stale; regenerate and inspect the diff")
        if args.selftest:
            selftest(manifest, receipts, boot_receipts)
    except (OSError, ValueError, KeyError, TypeError) as error:
        print(f"app-evidence: FAIL: {error}", file=sys.stderr)
        return 1
    print("app-evidence: PASS: 62/62 identities have fresh QEMU open-ready-close evidence")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
