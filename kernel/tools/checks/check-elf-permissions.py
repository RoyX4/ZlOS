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
            print("elf-permissions selftest: caught planted RWX on every image")
    except (OSError, ValueError, struct.error) as error:
        print(f"elf-permissions: FAIL: {error}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
