#!/usr/bin/env python3
"""Generate the route-neutral source/toolchain identity embedded in zlOS.

The exact final artifact hash is necessarily measured after linking and belongs
in a receipt. This identity covers the inputs shared by every route: active zl
imports, C/header/build inputs, compiler binary, tool versions, app manifest,
Git head/branch and whether the worktree is dirty. Target/loader/bits remain
runtime fields and are joined to this ID by each artifact receipt.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys


HERE = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.dirname(os.path.dirname(HERE))
ROOT = os.path.dirname(KERNEL_ROOT)
METADATA = os.path.join(KERNEL_ROOT, "metadata")
OUTPUT = os.path.join(METADATA, "build-identity.json")
EMBED = os.path.join(KERNEL_ROOT, "build_identity_embed.zl")
IMPORT_RE = re.compile(r"^\s*import\s+([A-Za-z_]\w*)\s*(?:#.*)?$", re.M)
ROUTE_INPUTS = (
    "SOURCES", "build.sh", "build64.sh", "buildefi.sh",
    "tools/images/mkdisk.sh", "tools/images/mkiso.sh",
    "tools/images/mkiso64.sh", "tools/images/mkusb.sh",
    "boot/link.ld", "boot/link64.ld", "boot/link-raw.ld",
    "boot/boot.S", "boot/boot64.S", "boot/raw_entry.S", "boot/raw_boot.asm",
    "boot/efi.c", "boot/gdt.c", "boot/gdt64.c",
    "boot/smp_trampoline.S", "boot/smp_trampoline64.S",
    "metadata/app-manifest.json", "app_manifest_embed.zl",
    "tools/generators/gen-app-manifest.py",
    "tools/generators/gen-build-identity.py",
    "tools/generators/gen-boot-media-ids.py",
    "tools/generators/gen-user-syscalls.py",
    "src/arch/x86/user_syscalls.json",
    "tools/reproducible_time.c",
)


def fail(message):
    raise ValueError(message)


def read_bytes(path):
    try:
        with open(path, "rb") as handle:
            return handle.read()
    except OSError as error:
        fail(f"cannot read {path}: {error}")


def sha_bytes(value):
    return hashlib.sha256(value).hexdigest()


def command(argv, cwd=ROOT):
    try:
        return subprocess.check_output(argv, cwd=cwd, text=True,
                                       stderr=subprocess.STDOUT).strip()
    except (OSError, subprocess.CalledProcessError) as error:
        fail(f"cannot identify tool {' '.join(argv)}: {error}")


def zl_imports():
    pending = [os.path.join(KERNEL_ROOT, "src", "kernel.zl")]
    result = set()
    while pending:
        path = os.path.abspath(pending.pop())
        if path in result:
            continue
        text = read_bytes(path).decode("utf-8", errors="strict")
        result.add(path)
        for module in IMPORT_RE.findall(text):
            # Excluded to break the deliberate identity -> embedded receipt
            # generation cycle. Every other generated/imported module is an
            # input, including the app-manifest embed.
            if module == "build_identity_embed":
                continue
            candidates = (
                os.path.join(KERNEL_ROOT, "src", module + ".zl"),
                os.path.join(KERNEL_ROOT, "apps", module + ".zl"),
                os.path.join(KERNEL_ROOT, module + ".zl"),
            )
            child = next((path for path in candidates if os.path.isfile(path)), None)
            if child is None:
                fail(f"kernel imports missing module {module}.zl")
            pending.append(child)
    return result


def input_paths():
    paths = set(zl_imports())
    sources_path = os.path.join(KERNEL_ROOT, "SOURCES")
    for line in read_bytes(sources_path).decode().splitlines():
        value = line.strip()
        if value and not value.startswith("#"):
            paths.add(os.path.join(KERNEL_ROOT, value))
    for name in ROUTE_INPUTS:
        paths.add(os.path.join(KERNEL_ROOT, name))
    for directory in (os.path.join(KERNEL_ROOT, "src"),
                      os.path.join(ROOT, "freestanding")):
        for current, _, names in os.walk(directory):
            for name in names:
                if name.endswith((".h", ".inc")):
                    paths.add(os.path.join(current, name))
    paths.add(os.path.join(ROOT, "freestanding", "runtime_kernel.c"))
    for path in paths:
        if not os.path.isfile(path):
            fail(f"declared build input is missing: {path}")
    return sorted(os.path.abspath(path) for path in paths)


def relative(path):
    return os.path.relpath(path, ROOT)


def identity_payload(value):
    """Return the route-neutral fields covered by identity_sha256.

    Git context is retained for provenance, but cannot be part of a checked-in
    identity: committing the generated file necessarily changes HEAD, and the
    generated files themselves necessarily change whole-worktree dirty state.
    """
    return {key: item for key, item in value.items()
            if key not in ("git", "identity_sha256")}


def source_git_context(paths):
    relative_paths = [relative(path) for path in paths]
    status = command([
        "git", "status", "--porcelain=v1", "--untracked-files=all", "--",
        *relative_paths,
    ])
    return {
        "head": command(["git", "rev-parse", "HEAD"]),
        "branch": command(["git", "branch", "--show-current"]),
        "dirty": bool(status),
        "evidence_ceiling": (
            "generator-observed context for the declared build-input closure; "
            "not part of identity_sha256 and not an artifact commit attestation"
        ),
    }


def build_identity():
    paths = input_paths()
    file_hashes = {relative(path): sha_bytes(read_bytes(path)) for path in paths}
    compiler = os.path.join(ROOT, "compile")
    if not os.path.isfile(compiler):
        fail("compiler binary is missing; build the zl toolchain first")
    toolchain = {
        "zl-compiler": {"path": "compile", "sha256": sha_bytes(read_bytes(compiler))},
        "gcc": command(["gcc", "--version"]).splitlines()[0],
        "clang": command(["clang", "--version"]).splitlines()[0],
        "ld": command(["ld", "--version"]).splitlines()[0],
        "lld-link": command(["lld-link", "--version"]).splitlines()[0],
        "nasm": command(["nasm", "-v"]).splitlines()[0],
        "grub-mkrescue": command(["grub-mkrescue", "--version"]).splitlines()[0],
    }
    value = {
        "schema": "zlos.build-input-identity.v1",
        "git": source_git_context(paths),
        "source_scope": "active zl import closure, declared C SOURCES, kernel/freestanding headers and all route scripts/entries/linkers/packaging helpers; conservative header superset",
        "source_files_sha256": file_hashes,
        "toolchain": toolchain,
        "routes": ["BIOS-multiboot32", "multiboot64", "native-UEFI64", "raw-BIOS-loader32"],
        "evidence_ceiling": "pre-link shared build-input identity; exact target, configuration, boot origin and final artifact hash require a route receipt",
    }
    canonical = json.dumps(identity_payload(value), sort_keys=True, separators=(",", ":"),
                           ensure_ascii=False).encode("utf-8")
    value["identity_sha256"] = sha_bytes(canonical)
    validate(value)
    return value


def validate(value):
    if value.get("schema") != "zlos.build-input-identity.v1":
        fail("wrong build identity schema")
    files = value.get("source_files_sha256")
    if not isinstance(files, dict) or len(files) < 40:
        fail("build input inventory is implausibly small")
    required = {"kernel/src/kernel.zl", "kernel/SOURCES", "kernel/metadata/app-manifest.json",
                "kernel/app_manifest_embed.zl", "kernel/tools/reproducible_time.c",
                "kernel/tools/generators/gen-boot-media-ids.py",
                "freestanding/runtime_kernel.c"}
    if not required.issubset(files):
        fail(f"build identity misses required files: {sorted(required - set(files))}")
    if value.get("routes") != ["BIOS-multiboot32", "multiboot64", "native-UEFI64", "raw-BIOS-loader32"]:
        fail("build route inventory drifted")
    tools = value.get("toolchain", {})
    for name in ("zl-compiler", "gcc", "clang", "ld", "lld-link", "nasm", "grub-mkrescue"):
        if not tools.get(name):
            fail(f"toolchain identity misses {name}")
    identity = value.get("identity_sha256")
    expected = sha_bytes(json.dumps(identity_payload(value), sort_keys=True, separators=(",", ":"),
                                     ensure_ascii=False).encode("utf-8"))
    if identity != expected:
        fail("build identity digest does not cover its canonical fields")


def serialized(value):
    return json.dumps(value, indent=2, sort_keys=True, ensure_ascii=False) + "\n"


def embedded(value):
    dirty = 1 if value["git"]["dirty"] else 0
    return (
        "# Generated by gen-build-identity.py. Do not edit.\n"
        "# Route-neutral input identity; target/artifact identity is a receipt.\n"
        "fn build_identity_report() {\n"
        f'    print("  build-identity: schema=1 id={value["identity_sha256"]}")\n'
        f'    print("  build-source: head={value["git"]["head"]} dirty={dirty}")\n'
        "    return 0\n"
        "}\n"
    )


def selftest(value):
    mutations = []
    for name, mutate in (
        ("missing-source", lambda x: x["source_files_sha256"].pop("kernel/src/kernel.zl")),
        ("missing-tool", lambda x: x["toolchain"].pop("gcc")),
        ("missing-route", lambda x: x["routes"].pop()),
        ("identity-drift", lambda x: x.__setitem__("identity_sha256", "0" * 64)),
    ):
        mutant = json.loads(json.dumps(value))
        mutate(mutant)
        try:
            validate(mutant)
        except ValueError:
            mutations.append(name)
        else:
            fail(f"selftest mutation escaped: {name}")
    print("build-identity selftest: caught " + ", ".join(mutations))
    context = json.loads(json.dumps(value))
    context["git"] = {
        "head": "0" * 40, "branch": "other", "dirty": not value["git"]["dirty"],
        "evidence_ceiling": value["git"]["evidence_ceiling"],
    }
    validate(context)
    if context["identity_sha256"] != value["identity_sha256"]:
        fail("Git generation context changed build-input identity")
    print("build-identity selftest: Git generation context is identity-neutral")


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args(argv)
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        value = build_identity()
        expected, expected_embed = serialized(value), embedded(value)
        if args.write:
            for path, content in ((OUTPUT, expected), (EMBED, expected_embed)):
                temp = path + ".tmp"
                with open(temp, "w", encoding="utf-8") as handle:
                    handle.write(content)
                os.replace(temp, path)
        else:
            current = json.loads(read_bytes(OUTPUT).decode())
            validate(current)
            if identity_payload(current) != identity_payload(value) \
                    or current.get("identity_sha256") != value.get("identity_sha256"):
                fail("build-identity.json is stale; regenerate and inspect the diff")
            if read_bytes(EMBED).decode() != embedded(current):
                fail("build_identity_embed.zl is stale; regenerate and inspect the diff")
        if args.selftest:
            selftest(value)
    except (OSError, ValueError) as error:
        print(f"build-identity: FAIL: {error}", file=sys.stderr)
        return 1
    print(f"build-identity: PASS: {len(value['source_files_sha256'])} inputs -> {value['identity_sha256']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
