#!/usr/bin/env python3
"""Reject writable-executable LOAD segments in every freestanding kernel ELF."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path


PT_LOAD = 1
PF_X = 1
PF_W = 2
KERNEL_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_IMAGES = tuple(
    KERNEL_ROOT / name for name in ("kernel.elf", "kernel64.elf", "kernel_raw.elf")
)


def load_flags(blob: bytes) -> list[int]:
    if len(blob) < 64 or blob[:4] != b"\x7fELF":
        raise ValueError("not an ELF image")
    elf_class = blob[4]
    endian = blob[5]
    if endian != 1:
        raise ValueError("only little-endian ELF is supported")
    if elf_class == 1:
        phoff = struct.unpack_from("<I", blob, 28)[0]
        phentsize, phnum = struct.unpack_from("<HH", blob, 42)
        minimum = 32
        type_offset, flags_offset = 0, 24
    elif elf_class == 2:
        phoff = struct.unpack_from("<Q", blob, 32)[0]
        phentsize, phnum = struct.unpack_from("<HH", blob, 54)
        minimum = 56
        type_offset, flags_offset = 0, 4
    else:
        raise ValueError(f"unknown ELF class {elf_class}")
    if phentsize < minimum:
        raise ValueError(f"program-header entry too small: {phentsize}")
    if phnum == 0 or phoff + phentsize * phnum > len(blob):
        raise ValueError("program-header table is absent or out of bounds")
    result = []
    for index in range(phnum):
        offset = phoff + index * phentsize
        p_type = struct.unpack_from("<I", blob, offset + type_offset)[0]
        p_flags = struct.unpack_from("<I", blob, offset + flags_offset)[0]
        if p_type == PT_LOAD:
            result.append(p_flags)
    if not result:
        raise ValueError("ELF contains no LOAD segments")
    return result


def check_blob(blob: bytes) -> list[int]:
    flags = load_flags(blob)
    if any((value & (PF_W | PF_X)) == (PF_W | PF_X) for value in flags):
        raise ValueError("writable-executable LOAD segment")
    return flags


def mutated_rwx(blob: bytes) -> bytes:
    out = bytearray(blob)
    elf_class = out[4]
    if elf_class == 1:
        phoff = struct.unpack_from("<I", out, 28)[0]
        phentsize, phnum = struct.unpack_from("<HH", out, 42)
        flags_offset = 24
    else:
        phoff = struct.unpack_from("<Q", out, 32)[0]
        phentsize, phnum = struct.unpack_from("<HH", out, 54)
        flags_offset = 4
    for index in range(phnum):
        offset = phoff + index * phentsize
        if struct.unpack_from("<I", out, offset)[0] == PT_LOAD:
            flags = struct.unpack_from("<I", out, offset + flags_offset)[0]
            struct.pack_into("<I", out, offset + flags_offset, flags | PF_W | PF_X)
            return bytes(out)
    raise AssertionError("selftest fixture has no LOAD segment")


def minimal_elf(elf_class: int, segment_type: int = PT_LOAD,
                flags: int = PF_X) -> bytes:
    blob = bytearray(128)
    blob[:7] = b"\x7fELF" + bytes((elf_class, 1, 1))
    if elf_class == 1:
        phoff, phentsize = 52, 32
        struct.pack_into("<I", blob, 28, phoff)
        struct.pack_into("<HH", blob, 42, phentsize, 1)
        struct.pack_into("<I", blob, phoff, segment_type)
        struct.pack_into("<I", blob, phoff + 24, flags)
    elif elf_class == 2:
        phoff, phentsize = 64, 56
        struct.pack_into("<Q", blob, 32, phoff)
        struct.pack_into("<HH", blob, 54, phentsize, 1)
        struct.pack_into("<I", blob, phoff, segment_type)
        struct.pack_into("<I", blob, phoff + 4, flags)
    else:
        raise ValueError(f"unsupported fixture class {elf_class}")
    return bytes(blob)


def hostile_selftest() -> int:
    elf32 = minimal_elf(1)
    elf64 = minimal_elf(2)
    if check_blob(elf32) != [PF_X] or check_blob(elf64) != [PF_X]:
        raise ValueError("minimized valid ELF fixtures were rejected")
    cases = {
        "empty": (b"", "not an ELF"),
        "bad-magic": (b"BAD!" + elf64[4:], "not an ELF"),
        "unknown-class": (elf64[:4] + b"\x03" + elf64[5:], "unknown ELF class"),
        "big-endian": (elf64[:5] + b"\x02" + elf64[6:], "little-endian"),
        "small-phdr": (elf64[:54] + struct.pack("<H", 8) + elf64[56:], "entry too small"),
        "zero-phnum": (elf64[:56] + b"\x00\x00" + elf64[58:], "absent or out of bounds"),
        "table-out-of-bounds": (
            elf64[:32] + struct.pack("<Q", len(elf64) - 4) + elf64[40:],
            "absent or out of bounds",
        ),
        "no-load-segment": (minimal_elf(2, segment_type=2), "no LOAD segments"),
        "truncated-table": (elf32[:70], "absent or out of bounds"),
        "writable-executable": (minimal_elf(2, flags=PF_W | PF_X), "writable-executable"),
    }
    caught = 0
    for name, (blob, marker) in cases.items():
        try:
            check_blob(blob)
        except (ValueError, struct.error) as error:
            if marker not in str(error):
                raise ValueError(f"{name}: wrong rejection: {error}") from error
            caught += 1
        else:
            raise ValueError(f"{name}: hostile ELF escaped")
    return caught


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("images", nargs="*", default=list(DEFAULT_IMAGES))
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    try:
        for name in args.images:
            blob = Path(name).read_bytes()
            flags = check_blob(blob)
            print(f"elf-permissions: {name}: PASS: LOAD flags {flags}")
            if args.selftest:
                try:
                    check_blob(mutated_rwx(blob))
                except ValueError as error:
                    if "writable-executable" not in str(error):
                        raise
                else:
                    raise ValueError(f"{name}: planted RWX mutation escaped")
        if args.selftest:
            hostile = hostile_selftest()
            print(
                "elf-permissions selftest: caught planted RWX on every image "
                f"and {hostile} minimized malformed ELF cases"
            )
    except (OSError, ValueError, struct.error) as error:
        print(f"elf-permissions: FAIL: {error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
