#!/usr/bin/env python3
"""Generate the fail-closed zlOS artifact and boot-route registry."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import struct
import subprocess
import sys
import tempfile


HERE = os.path.dirname(os.path.abspath(__file__))
OUTPUT = os.path.join(HERE, "artifact-registry.json")
BUILD_IDENTITY = os.path.join(HERE, "build-identity.json")
REPRO_RECEIPT = os.path.join(
    HERE, "docs", "receipts", "reproducible-build-2026-08-22.json")
APP_EVIDENCE = os.path.join(HERE, "app-evidence.json")
ELF_PERMISSION_CHECKER = os.path.join(HERE, "check-elf-permissions.py")
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
BOOT_RECEIPTS = {
    "raw-bios": "app-manifest-raw-bios-qemu-2026-08-22.json",
    "native-uefi64": "app-manifest-native-uefi64-qemu-2026-08-22.json",
    "grub-bios32": "app-manifest-grub-bios32-qemu-2026-08-22.json",
    "grub-uefi32": "app-manifest-grub-uefi32-qemu-2026-08-22.json",
    "grub-bios64": "app-manifest-grub-bios64-qemu-2026-08-22.json",
    "grub-uefi64": "app-manifest-grub-uefi64-qemu-2026-08-22.json",
}
ROUTE_ARTIFACT = {
    "raw-bios": "zlOS.img",
    "native-uefi64": "zlOS-usb.img",
    "grub-bios32": "zlOS.iso",
    "grub-uefi32": "zlOS.iso",
    "grub-bios64": "zlOS64.iso",
    "grub-uefi64": "zlOS64.iso",
}
ORIGINS = {
    "raw-bios": "our bootloader (raw_boot), no GRUB",
    "native-uefi64": "UEFI application - no GRUB, no bootloader",
    "grub-bios32": "multiboot handoff, 32-bit protected mode",
    "grub-uefi32": "multiboot handoff, 32-bit protected mode",
    "grub-bios64": "multiboot handoff, then OUR jump into 64-bit long mode",
    "grub-uefi64": "multiboot handoff, then OUR jump into 64-bit long mode",
}


def fail(message):
    raise ValueError(message)


def load(path):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


def sha256(path):
    value = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def sha256_bytes(value):
    return hashlib.sha256(value).hexdigest()


def check_source_hashes(name, receipt):
    sources = receipt.get("source_files_sha256")
    if not isinstance(sources, dict) or not sources:
        fail(f"{name}: missing source hashes")
    for relative, expected in sources.items():
        path = os.path.join(HERE, relative)
        if not os.path.isfile(path):
            fail(f"{name}: source disappeared: {relative}")
        if sha256(path) != expected:
            fail(f"{name}: stale source: {relative}")


def format_checks():
    values = {name: open(os.path.join(HERE, name), "rb").read(4096)
              for name in ARTIFACTS if name != "boot-media-ids.json"}
    for name, elf_class in (("kernel.elf", 1), ("kernel64.elf", 2),
                            ("kernel_raw.elf", 1)):
        if values[name][:5] != b"\x7fELF" + bytes([elf_class]):
            fail(f"{name}: wrong ELF class or magic")
    try:
        subprocess.run(
            [sys.executable, ELF_PERMISSION_CHECKER,
             "kernel.elf", "kernel64.elf", "kernel_raw.elf"],
            cwd=HERE, check=True, stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )
    except (OSError, subprocess.CalledProcessError):
        fail("kernel ELF memory-permission contract failed")
    pe = values["BOOTX64.EFI"]
    if pe[:2] != b"MZ" or len(pe) < 64:
        fail("BOOTX64.EFI: missing DOS/PE header")
    pe_offset = struct.unpack_from("<I", pe, 0x3C)[0]
    with open(os.path.join(HERE, "BOOTX64.EFI"), "rb") as handle:
        handle.seek(pe_offset)
        if handle.read(4) != b"PE\0\0":
            fail("BOOTX64.EFI: missing PE signature")
    with open(os.path.join(HERE, "zlOS.iso"), "rb") as handle:
        handle.seek(16 * 2048 + 1)
        if handle.read(5) != b"CD001":
            fail("zlOS.iso: missing ISO9660 primary volume descriptor")
    with open(os.path.join(HERE, "zlOS64.iso"), "rb") as handle:
        handle.seek(16 * 2048 + 1)
        if handle.read(5) != b"CD001":
            fail("zlOS64.iso: missing ISO9660 primary volume descriptor")
    if values["zlOS.img"][510:512] != b"\x55\xaa":
        fail("zlOS.img: missing BIOS boot signature")
    with open(os.path.join(HERE, "zlOS-usb.img"), "rb") as handle:
        handle.seek(512)
        if handle.read(8) != b"EFI PART":
            fail("zlOS-usb.img: missing GPT header")


def extract_relations():
    with tempfile.TemporaryDirectory(prefix="zlos-artifact-registry-") as root:
        iso_kernel = os.path.join(root, "iso-kernel.elf")
        subprocess.run([
            "xorriso", "-osirrox", "on", "-indev", os.path.join(HERE, "zlOS.iso"),
            "-extract", "/boot/kernel.elf", iso_kernel,
        ], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if sha256(iso_kernel) != sha256(os.path.join(HERE, "kernel.elf")):
            fail("zlOS.iso does not contain the current kernel.elf")

        iso64_kernel = os.path.join(root, "iso-kernel64.elf")
        subprocess.run([
            "xorriso", "-osirrox", "on", "-indev", os.path.join(HERE, "zlOS64.iso"),
            "-extract", "/boot/kernel64.elf", iso64_kernel,
        ], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if sha256(iso64_kernel) != sha256(os.path.join(HERE, "kernel64.elf")):
            fail("zlOS64.iso does not contain the current kernel64.elf")

        usb_efi = os.path.join(root, "usb-BOOTX64.EFI")
        subprocess.run([
            "mcopy", "-n", "-i", os.path.join(HERE, "zlOS-usb.img") + "@@1M",
            "::/EFI/BOOT/BOOTX64.EFI", usb_efi,
        ], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if sha256(usb_efi) != sha256(os.path.join(HERE, "BOOTX64.EFI")):
            fail("zlOS-usb.img does not contain the current BOOTX64.EFI")

        raw_binary = os.path.join(root, "kernel_raw.bin")
        subprocess.run([
            "objcopy", "-O", "binary", os.path.join(HERE, "kernel_raw.elf"), raw_binary,
        ], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        binary = open(raw_binary, "rb").read()
        disk = open(os.path.join(HERE, "zlOS.img"), "rb").read()
        boot = open(os.path.join(HERE, "raw_boot.bin"), "rb").read()
        if len(boot) != 512 or disk[:512] != boot:
            fail("zlOS.img does not begin with the current 512-byte raw boot sector")
        if disk[512:512 + len(binary)] != binary:
            fail("zlOS.img does not contain kernel_raw.elf's current binary payload")
        if any(disk[512 + len(binary):]):
            fail("zlOS.img padding after kernel payload is not zero-filled")
        return {
            "iso_kernel": {
                "container": "zlOS.iso", "member": "/boot/kernel.elf",
                "source": "kernel.elf", "relation": "byte-exact",
            },
            "iso64_kernel": {
                "container": "zlOS64.iso", "member": "/boot/kernel64.elf",
                "source": "kernel64.elf", "relation": "byte-exact",
            },
            "usb_efi": {
                "container": "zlOS-usb.img", "member": "/EFI/BOOT/BOOTX64.EFI",
                "source": "BOOTX64.EFI", "relation": "byte-exact",
            },
            "raw_disk": {
                "container": "zlOS.img", "offset": 512,
                "source": "kernel_raw.elf", "relation": "objcopy-binary",
                "payload_bytes": len(binary), "payload_sha256": sha256_bytes(binary),
                "boot_sector_sha256": sha256_bytes(boot),
            },
        }


def validate_registry(value):
    if value.get("schema") != "zlos.artifact-boot-registry.v1":
        fail("wrong registry schema")
    if len(value.get("generator", {}).get("sha256", "")) != 64:
        fail("missing generator identity")
    if len(value.get("format_verifier", {}).get("sha256", "")) != 64:
        fail("missing format-verifier identity")
    artifacts = value.get("artifacts", {})
    if tuple(artifacts) != ARTIFACTS:
        fail("artifact order/set drift")
    routes = value.get("boot_routes", {})
    if set(routes) != set(BOOT_RECEIPTS):
        fail("boot route set drift")
    for route, record in routes.items():
        if record.get("result") != "PASS" or record.get("artifact") != ROUTE_ARTIFACT[route]:
            fail(f"{route}: invalid promoted route")
    for name, record in artifacts.items():
        if record.get("physical_hardware") != "UNVERIFIED_FOR_EXACT_HASH":
            fail(f"{name}: physical evidence overclaim")
        if record.get("sha256") is None or record.get("bytes") is None:
            fail(f"{name}: missing identity")
    for name in ("kernel.elf", "kernel64.elf", "kernel_raw.elf"):
        if artifacts[name].get("memory_permissions") != "NO_RWX_LOAD":
            fail(f"{name}: missing non-RWX memory-permission proof")
    if artifacts["kernel64.elf"].get("proof_state") != "PARENT_BYTE_EXACT":
        fail("kernel64.elf must retain its exact parent-payload runtime proof")


def build(verify_files=True):
    identity = load(BUILD_IDENTITY)
    repro = load(REPRO_RECEIPT)
    if repro.get("schema") != "zlos.reproducible-build-receipt.v1" \
            or repro.get("result") != "PASS" or repro.get("independent_runs") != 2:
        fail("reproducibility receipt did not prove two passing runs")
    wanted_identity = identity["identity_sha256"]
    if repro.get("build_identity", {}).get("id") != wanted_identity:
        fail("reproducibility receipt build identity mismatch")
    if verify_files:
        check_source_hashes("reproducibility", repro)
    repro_artifacts = repro.get("artifacts", {})
    if set(repro_artifacts) != set(ARTIFACTS) or len(repro_artifacts) != len(ARTIFACTS):
        fail("reproducibility artifact set drift")
    for name, record in repro_artifacts.items():
        path = os.path.join(HERE, name)
        if record.get("sha256") != sha256(path) or record.get("bytes") != os.path.getsize(path):
            fail(f"{name}: current artifact differs from reproducibility receipt")
    format_checks()
    relations = extract_relations()

    routes = {}
    receipt_dir = os.path.join(HERE, "docs", "receipts")
    for route, filename in BOOT_RECEIPTS.items():
        receipt = load(os.path.join(receipt_dir, filename))
        artifact = ROUTE_ARTIFACT[route]
        if receipt.get("route") != route or receipt.get("result") != "PASS":
            fail(f"{route}: route receipt did not pass")
        if receipt.get("boot_origin") != ORIGINS[route]:
            fail(f"{route}: boot origin mismatch")
        if receipt.get("shipped_build_identity", {}).get("id") != wanted_identity:
            fail(f"{route}: build identity mismatch")
        expected = repro_artifacts[artifact]
        if receipt.get("artifact", {}).get("sha256") != expected["sha256"]:
            fail(f"{route}: artifact hash mismatch")
        if verify_files:
            check_source_hashes(route, receipt)
        routes[route] = {
            "artifact": artifact,
            "firmware": "UEFI" if "uefi" in route else "BIOS",
            "entry_bits": 64 if route.endswith("64") else 32,
            "loader": "zlOS EFI application" if route == "native-uefi64"
                      else ("zlOS 512-byte boot sector" if route == "raw-bios" else "GRUB 2"),
            "boot_origin": ORIGINS[route],
            "receipt": "kernel/docs/receipts/" + filename,
            "receipt_sha256": sha256(os.path.join(receipt_dir, filename)),
            "result": "PASS",
            "evidence": "QEMU_RUNTIME",
            "physical_hardware": "UNVERIFIED_FOR_EXACT_HASH",
        }

    app_evidence = load(APP_EVIDENCE)
    if app_evidence.get("artifact", {}).get("sha256") != repro_artifacts["zlOS.iso"]["sha256"]:
        fail("application evidence does not cover the reproducible ISO")
    if app_evidence.get("shipped_build_identity", {}).get("id") != wanted_identity:
        fail("application evidence build identity mismatch")

    definitions = {
        "kernel.elf": ("ELF32 multiboot kernel", "build.sh", "PARENT_BYTE_EXACT",
                       ["grub-bios32", "grub-uefi32"]),
        "kernel64.elf": ("ELF64 freestanding kernel", "build64.sh", "PARENT_BYTE_EXACT",
                         ["grub-bios64", "grub-uefi64"]),
        "kernel_raw.elf": ("ELF32 raw-loader kernel", "mkdisk.sh", "PARENT_DERIVED_PAYLOAD",
                           ["raw-bios"]),
        "BOOTX64.EFI": ("PE32+ native UEFI application", "buildefi.sh", "PARENT_BYTE_EXACT",
                        ["native-uefi64"]),
        "zlOS.iso": ("hybrid ISO with GRUB", "mkiso.sh", "QEMU_DIRECT_ARTIFACT",
                     ["grub-bios32", "grub-uefi32"]),
        "zlOS64.iso": ("hybrid GRUB ISO with 64-bit multiboot payload", "mkiso64.sh",
                       "QEMU_DIRECT_ARTIFACT", ["grub-bios64", "grub-uefi64"]),
        "zlOS.img": ("raw BIOS disk image", "mkdisk.sh", "QEMU_DIRECT_ARTIFACT", ["raw-bios"]),
        "zlOS-usb.img": ("GPT/FAT UEFI USB disk image", "mkusb.sh", "QEMU_DIRECT_ARTIFACT",
                         ["native-uefi64"]),
        "boot-media-ids.json": ("deterministic disk/FAT identity metadata", "gen-boot-media-ids.py",
                                "METADATA_ONLY", []),
    }
    artifacts = {}
    for name in ARTIFACTS:
        description, producer, state, boot_routes = definitions[name]
        artifacts[name] = {
            **repro_artifacts[name],
            "description": description,
            "producer": producer,
            "proof_state": state,
            "boot_routes": boot_routes,
            "reproducibility": "TWO_INDEPENDENT_RUNS_BYTE_IDENTICAL",
            "physical_hardware": "UNVERIFIED_FOR_EXACT_HASH",
        }
        if name in ("kernel.elf", "kernel64.elf", "kernel_raw.elf"):
            artifacts[name]["memory_permissions"] = "NO_RWX_LOAD"

    value = {
        "schema": "zlos.artifact-boot-registry.v1",
        "result": "PASS",
        "generator": {
            "path": "kernel/gen-artifact-registry.py",
            "sha256": sha256(os.path.abspath(__file__)),
        },
        "format_verifier": {
            "path": "kernel/check-elf-permissions.py",
            "sha256": sha256(ELF_PERMISSION_CHECKER),
        },
        "build_identity": {
            "schema": identity["schema"], "id": wanted_identity,
            "head": identity["git"]["head"], "dirty": identity["git"]["dirty"],
        },
        "evidence_vocabulary": {
            "BUILD_ONLY": "format/build checks only; no runtime promotion",
            "PARENT_BYTE_EXACT": "exact bytes occur inside a QEMU-booted parent artifact",
            "PARENT_DERIVED_PAYLOAD": "a checked deterministic transform occurs inside a QEMU-booted parent",
            "QEMU_DIRECT_ARTIFACT": "this exact artifact hash booted in QEMU",
            "METADATA_ONLY": "generated identity input; not executable",
            "UNVERIFIED_FOR_EXACT_HASH": "no physical receipt is bound to this exact artifact hash",
        },
        "reproducibility_receipt": {
            "path": "kernel/docs/receipts/reproducible-build-2026-08-22.json",
            "sha256": sha256(REPRO_RECEIPT),
        },
        "relations": relations,
        "boot_routes": routes,
        "application_evidence": {
            "path": "kernel/app-evidence.json", "sha256": sha256(APP_EVIDENCE),
            "identities": app_evidence.get("counts", {}).get("identities"),
            "artifact": "zlOS.iso",
            "claim_ceiling": app_evidence.get("evidence_ceiling"),
        },
        "artifacts": artifacts,
        "weakest_link": "all runtime proof is QEMU and no artifact hash has current physical-hardware proof",
    }
    validate_registry(value)
    return value


def serialized(value):
    return json.dumps(value, indent=2, sort_keys=False) + "\n"


def selftest(value):
    mutations = []
    missing = copy.deepcopy(value)
    del missing["artifacts"]["kernel.elf"]
    mutations.append(("missing-artifact", missing))
    fake_route = copy.deepcopy(value)
    fake_route["boot_routes"]["raw-bios"]["result"] = "FAIL"
    mutations.append(("failed-route", fake_route))
    physical = copy.deepcopy(value)
    physical["artifacts"]["zlOS-usb.img"]["physical_hardware"] = "PASS"
    mutations.append(("physical-overclaim", physical))
    downgraded64 = copy.deepcopy(value)
    downgraded64["artifacts"]["kernel64.elf"]["proof_state"] = "BUILD_ONLY"
    mutations.append(("lost-kernel64-runtime-proof", downgraded64))
    caught = []
    for name, mutation in mutations:
        try:
            validate_registry(mutation)
        except ValueError:
            caught.append(name)
        else:
            fail(f"selftest mutation escaped: {name}")
    print("artifact-registry selftest: caught " + ", ".join(caught))


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args(argv)
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        value = build()
        expected = serialized(value)
        if args.write:
            temp = OUTPUT + ".tmp"
            with open(temp, "w", encoding="utf-8") as handle:
                handle.write(expected)
            os.replace(temp, OUTPUT)
        else:
            if open(OUTPUT, encoding="utf-8").read() != expected:
                fail("artifact-registry.json is stale")
        if args.selftest:
            selftest(value)
        print(f"artifact-registry: PASS: {len(ARTIFACTS)} artifacts, {len(BOOT_RECEIPTS)} QEMU boot routes")
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"artifact-registry: FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
