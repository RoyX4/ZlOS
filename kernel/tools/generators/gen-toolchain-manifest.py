#!/usr/bin/env python3
"""Generate the exact current zlOS compiler/ABI/linker contract."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
ROOT = KERNEL_ROOT.parent
METADATA = KERNEL_ROOT / "metadata"
OUTPUT = METADATA / "toolchain-manifest.json"
IDENTITY = METADATA / "build-identity.json"
DEPENDENCIES = METADATA / "dependency-lock.json"
FORBIDDEN_ENVIRONMENT = (
    "CPATH",
    "C_INCLUDE_PATH",
    "CPLUS_INCLUDE_PATH",
    "LIBRARY_PATH",
    "GCC_EXEC_PREFIX",
    "COMPILER_PATH",
    "LD_PRELOAD",
)


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256(path: Path) -> str:
    return sha256_bytes(path.read_bytes())


def load(path: Path) -> dict:
    return json.loads(path.read_text())


def run(argv: list[str], cwd: Path = KERNEL_ROOT) -> str:
    try:
        return subprocess.check_output(argv, cwd=cwd, text=True, stderr=subprocess.STDOUT).strip()
    except (OSError, subprocess.CalledProcessError) as error:
        raise ValueError(f"command failed: {shlex.join(argv)}: {error}") from error


def shell_flags(script: Path, variable: str) -> list[str]:
    text = script.read_text()
    match = re.search(rf"^{re.escape(variable)}=\"(.*?)\"", text, re.MULTILINE | re.DOTALL)
    if not match:
        raise ValueError(f"{script.name}: cannot find {variable}")
    includes = " ".join(
        f"-I{path.relative_to(KERNEL_ROOT)}"
        for root in (KERNEL_ROOT / "src", KERNEL_ROOT / "boot")
        for path in sorted((item for item in root.rglob("*") if item.is_dir()), key=str)
    )
    includes = f"-Isrc -Iboot {includes}"
    raw = match.group(1).replace("\\\n", " ").replace("$INCLUDES", includes)
    return shlex.split(raw)


def macro_probe(tool: str, flags: list[str]) -> dict[str, str]:
    output = run([tool, *flags, "-dM", "-E", "-x", "c", "/dev/null"])
    macros = {}
    for line in output.splitlines():
        match = re.match(r"#define\s+(\S+)\s+(.*)", line)
        if match:
            macros[match.group(1)] = match.group(2)
    wanted = (
        "__i386__",
        "__x86_64__",
        "_WIN64",
        "__SIZEOF_POINTER__",
        "__SIZEOF_LONG__",
        "__SIZEOF_WCHAR_T__",
        "__STDC_HOSTED__",
        "__BYTE_ORDER__",
        "__ORDER_LITTLE_ENDIAN__",
    )
    return {name: macros[name] for name in wanted if name in macros}


def dependency_paths(tool: str, flags: list[str], source: str) -> list[Path]:
    output = run([tool, *flags, "-M", source])
    flattened = output.replace("\\\n", " ")
    if ":" not in flattened:
        raise ValueError(f"dependency output has no target: {source}")
    values = shlex.split(flattened.split(":", 1)[1])
    result = []
    for value in values:
        path = (KERNEL_ROOT / value).resolve() if not Path(value).is_absolute() else Path(value).resolve()
        try:
            path.relative_to(ROOT)
        except ValueError:
            if path.is_file():
                result.append(path)
    return result


def required_script_shape(path: Path, snippets: tuple[str, ...]) -> None:
    text = path.read_text()
    missing = [snippet for snippet in snippets if snippet not in text]
    if missing:
        raise ValueError(f"{path.name}: missing build contract: {missing}")


def build() -> dict:
    identity = load(IDENTITY)
    dependency = load(DEPENDENCIES)
    if dependency.get("build_identity") != identity.get("identity_sha256"):
        raise ValueError("dependency lock and build identity disagree")

    forbidden_present = sorted(name for name in FORBIDDEN_ENVIRONMENT if os.environ.get(name))
    if forbidden_present:
        raise ValueError("build-injecting environment is set: " + ", ".join(forbidden_present))
    cc_value = os.environ.get("CC", "")
    if cc_value not in ("", "gcc"):
        raise ValueError(f"CC={cc_value!r} would replace the locked host compiler")

    scripts = {
        "bios32": KERNEL_ROOT / "build.sh",
        "bios64": KERNEL_ROOT / "build64.sh",
        "uefi64": KERNEL_ROOT / "buildefi.sh",
        "raw32": KERNEL_ROOT / "tools" / "images" / "mkdisk.sh",
    }
    required_script_shape(scripts["bios32"], ("-mgeneral-regs-only", "ld -m elf_i386 -T boot/link.ld"))
    required_script_shape(scripts["bios64"], ("-mgeneral-regs-only", "ld -m elf_x86_64 -T boot/link64.ld"))
    required_script_shape(scripts["uefi64"], ("-mgeneral-regs-only", "/Brepro /lldignoreenv", "-subsystem:efi_application"))
    required_script_shape(scripts["raw32"], ("-mgeneral-regs-only", "ld -m elf_i386 -T boot/link-raw.ld", "objcopy -O binary"))

    gcc32 = shell_flags(scripts["bios32"], "CFLAGS")
    gcc64 = shell_flags(scripts["bios64"], "CFLAGS")
    clang_uefi = shell_flags(scripts["uefi64"], "CF")
    raw32 = shell_flags(scripts["raw32"], "CFLAGS")
    if gcc32 != raw32:
        raise ValueError("BIOS32 and raw32 compiler contracts drifted")

    lanes = [
        {
            "id": "bios-multiboot32",
            "script": "kernel/build.sh",
            "script_sha256": sha256(scripts["bios32"]),
            "compiler": "gcc",
            "compiler_flags": gcc32,
            "linker": "ld",
            "linker_flags": ["-m", "elf_i386", "-T", "boot/link.ld"],
            "format": "ELF32-i386",
            "abi": "ILP32 SysV freestanding",
            "macros": macro_probe("gcc", ["-m32", "-ffreestanding"]),
        },
        {
            "id": "multiboot64",
            "script": "kernel/build64.sh",
            "script_sha256": sha256(scripts["bios64"]),
            "compiler": "gcc",
            "compiler_flags": gcc64,
            "linker": "ld",
            "linker_flags": ["-m", "elf_x86_64", "-T", "boot/link64.ld"],
            "format": "ELF64-x86-64",
            "abi": "LP64 SysV freestanding, no red zone, large code model",
            "macros": macro_probe("gcc", ["-m64", "-ffreestanding"]),
        },
        {
            "id": "native-uefi64",
            "script": "kernel/buildefi.sh",
            "script_sha256": sha256(scripts["uefi64"]),
            "compiler": "clang",
            "compiler_flags": clang_uefi,
            "linker": "lld-link",
            "linker_flags": [
                "-subsystem:efi_application",
                "-nodefaultlib",
                "-dll",
                "/Brepro",
                "/lldignoreenv",
                "-entry:efi_main",
            ],
            "format": "PE32+ EFI application",
            "abi": "LLP64 Microsoft x86-64, UTF-16 wchar, no red zone",
            "macros": macro_probe(
                "clang",
                ["-target", "x86_64-unknown-windows", "-ffreestanding", "-fshort-wchar"],
            ),
        },
        {
            "id": "raw-bios-loader32",
            "script": "kernel/tools/images/mkdisk.sh",
            "script_sha256": sha256(scripts["raw32"]),
            "compiler": "gcc",
            "compiler_flags": raw32,
            "linker": "ld+objcopy+nasm",
            "linker_flags": ["-m", "elf_i386", "-T", "boot/link-raw.ld", "-O", "binary"],
            "format": "ELF32 intermediate plus flat kernel and 512-byte boot sector",
            "abi": "ILP32 SysV freestanding",
            "macros": macro_probe("gcc", ["-m32", "-ffreestanding"]),
        },
    ]

    expected_macros = {
        "bios-multiboot32": {"__i386__": "1", "__SIZEOF_POINTER__": "4", "__SIZEOF_LONG__": "4", "__STDC_HOSTED__": "0"},
        "multiboot64": {"__x86_64__": "1", "__SIZEOF_POINTER__": "8", "__SIZEOF_LONG__": "8", "__STDC_HOSTED__": "0"},
        "native-uefi64": {"__x86_64__": "1", "_WIN64": "1", "__SIZEOF_POINTER__": "8", "__SIZEOF_LONG__": "4", "__SIZEOF_WCHAR_T__": "2", "__STDC_HOSTED__": "0"},
        "raw-bios-loader32": {"__i386__": "1", "__SIZEOF_POINTER__": "4", "__SIZEOF_LONG__": "4", "__STDC_HOSTED__": "0"},
    }
    for lane in lanes:
        expected = expected_macros[lane["id"]]
        if any(lane["macros"].get(name) != value for name, value in expected.items()):
            raise ValueError(f"{lane['id']}: ABI macro probe disagrees")
        if not {"-ffreestanding", "-fno-stack-protector", "-Wall", "-Wextra", "-Werror"}.issubset(lane["compiler_flags"]):
            raise ValueError(f"{lane['id']}: mandatory compiler policy is missing")

    header_consumers: dict[Path, set[str]] = {}
    probes = (
        ("bios32-fb", "gcc", gcc32, "src/graphics/framebuffer/fb.c"),
        ("bios64-fb", "gcc", gcc64, "src/graphics/framebuffer/fb.c"),
        ("uefi64-fb", "clang", clang_uefi, "src/graphics/framebuffer/fb.c"),
        (
            "reproducible-media-helper",
            "gcc",
            ["-shared", "-fPIC", "-O2", "-Wall", "-Wextra", "-Werror"],
            "tools/reproducible_time.c",
        ),
    )
    for label, tool, flags, source in probes:
        for path in dependency_paths(tool, flags, source):
            header_consumers.setdefault(path, set()).add(label)
    external_headers = [
        {
            "path": str(path),
            "bytes": path.stat().st_size,
            "sha256": sha256(path),
            "consumers": sorted(consumers),
        }
        for path, consumers in sorted(header_consumers.items(), key=lambda item: str(item[0]))
    ]
    if not external_headers:
        raise ValueError("external compiler/sysroot header closure is implausibly empty")

    command_rows = {row["name"]: row for row in dependency["commands"]}
    tools = []
    for name in ("gcc", "clang", "ld", "lld-link", "objcopy", "nasm"):
        row = command_rows.get(name)
        if not row:
            raise ValueError(f"dependency lock misses toolchain command {name}")
        tools.append(
            {
                "name": name,
                "invoked_path": row["path"],
                "resolved_path": row["resolved_path"],
                "bytes": row["bytes"],
                "sha256": row["sha256"],
                "version": row["version"],
            }
        )
    tools.insert(
        0,
        {
            "name": "zl-compiler",
            "invoked_path": str((ROOT / identity["toolchain"]["zl-compiler"]["path"]).resolve()),
            "resolved_path": str((ROOT / identity["toolchain"]["zl-compiler"]["path"]).resolve()),
            "bytes": (ROOT / identity["toolchain"]["zl-compiler"]["path"]).stat().st_size,
            "sha256": identity["toolchain"]["zl-compiler"]["sha256"],
            "version": "repository-built archived zl-to-C compiler; byte identity is authoritative",
        },
    )

    return {
        "schema": "zlos.toolchain-manifest.v1",
        "result": "PASS_WITH_OPEN_PORTABILITY_GAPS",
        "build_identity": identity["identity_sha256"],
        "dependency_lock_sha256": sha256(DEPENDENCIES),
        "environment": {
            "CC": cc_value or "UNSET_EFFECTIVE_GCC",
            "forbidden_variables": list(FORBIDDEN_ENVIRONMENT),
            "forbidden_present": forbidden_present,
            "path_resolution_bound_by_dependency_lock": True,
        },
        "tools": tools,
        "target_lanes": lanes,
        "external_headers": external_headers,
        "counts": {
            "tools": len(tools),
            "target_lanes": len(lanes),
            "external_headers": len(external_headers),
        },
        "open_gaps": [
            "tool binaries and external headers are local distribution bytes without publisher attestation",
            "compiler/sysroot source packages are not archived",
            "no portable hermetic build image exists",
            "the top-level host-tool build still permits an explicit CC=gcc alias",
        ],
        "generator": {
            "path": "kernel/gen-toolchain-manifest.py",
            "sha256": sha256(Path(__file__).resolve()),
        },
        "evidence_ceiling": (
            "exact current compiler/linker/header bytes and ABI policy for four kernel routes; "
            "not a hermetic, signed, source-reproducible or cross-host toolchain"
        ),
    }


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.toolchain-manifest.v1" \
            or value.get("result") != "PASS_WITH_OPEN_PORTABILITY_GAPS":
        raise ValueError("wrong toolchain manifest schema/result")
    if len(value.get("build_identity", "")) != 64 or len(value.get("dependency_lock_sha256", "")) != 64:
        raise ValueError("toolchain manifest identity is missing")
    lanes = value.get("target_lanes")
    expected_ids = ["bios-multiboot32", "multiboot64", "native-uefi64", "raw-bios-loader32"]
    if not isinstance(lanes, list) or [lane.get("id") for lane in lanes] != expected_ids:
        raise ValueError("toolchain target lane set/order drifted")
    for lane in lanes:
        if "-Werror" not in lane.get("compiler_flags", []) or not lane.get("macros"):
            raise ValueError(f"{lane.get('id')}: warning/ABI policy is missing")
    tools = value.get("tools")
    expected_tools = ["zl-compiler", "gcc", "clang", "ld", "lld-link", "objcopy", "nasm"]
    if not isinstance(tools, list) or [row.get("name") for row in tools] != expected_tools:
        raise ValueError("toolchain tool set/order drifted")
    for row in tools:
        if len(row.get("sha256", "")) != 64 or not Path(row.get("invoked_path", "")).is_absolute():
            raise ValueError(f"{row.get('name')}: tool identity is incomplete")
    headers = value.get("external_headers")
    if not isinstance(headers, list) or not headers:
        raise ValueError("external header closure is missing")
    for row in headers:
        if len(row.get("sha256", "")) != 64 or row.get("bytes", 0) <= 0 or not row.get("consumers"):
            raise ValueError(f"external header identity is incomplete: {row.get('path')}")
    counts = value.get("counts")
    if counts != {"tools": len(tools), "target_lanes": len(lanes), "external_headers": len(headers)}:
        raise ValueError("toolchain manifest counts drifted")
    environment = value.get("environment", {})
    if environment.get("forbidden_present") != [] or environment.get("path_resolution_bound_by_dependency_lock") is not True:
        raise ValueError("toolchain environment injection is hidden")
    if len(value.get("open_gaps", [])) < 4 or len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("toolchain portability gaps/generator identity are missing")


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value)
    missing["target_lanes"].pop()
    mutations["missing-lane"] = missing
    abi = copy.deepcopy(value)
    abi["target_lanes"][2]["macros"]["__SIZEOF_LONG__"] = "8"
    mutations["wrong-llp64"] = abi
    warning = copy.deepcopy(value)
    warning["target_lanes"][0]["compiler_flags"].remove("-Werror")
    mutations["missing-warning-policy"] = warning
    header = copy.deepcopy(value)
    header["external_headers"][0]["sha256"] = "0" * 64
    mutations["header-drift"] = header
    environment = copy.deepcopy(value)
    environment["environment"]["forbidden_present"] = ["CPATH"]
    mutations["environment-injection"] = environment
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
            if mutant != value:
                raise ValueError("toolchain manifest differs from measured value")
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"toolchain mutation escaped: {name}")
    print("toolchain-manifest selftest: caught " + ", ".join(caught))


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
        validate(value)
        if args.selftest:
            selftest(value)
        if args.write:
            write_atomic(value)
        if args.check and (not OUTPUT.is_file() or load(OUTPUT) != value):
            raise ValueError("toolchain-manifest.json is missing or stale")
        print(
            "toolchain-manifest: PASS_WITH_OPEN_PORTABILITY_GAPS: "
            f"{value['counts']['tools']} tools, {value['counts']['target_lanes']} lanes, "
            f"{value['counts']['external_headers']} external headers"
        )
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.SubprocessError) as error:
        print(f"toolchain-manifest: FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
