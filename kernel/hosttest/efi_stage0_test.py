#!/usr/bin/env python3
"""Structural gate for the removable-media stage-0 EFI chain."""

from __future__ import annotations

import hashlib
import json
import pathlib
import struct
import subprocess
import sys
import tempfile


ROOT = pathlib.Path(__file__).resolve().parents[1]


def u16(data: bytes, off: int) -> int:
    return struct.unpack_from("<H", data, off)[0]


def u32(data: bytes, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


def u64(data: bytes, off: int) -> int:
    return struct.unpack_from("<Q", data, off)[0]


def check_pe(path: pathlib.Path, *, stage0: bool) -> None:
    data = path.read_bytes()
    assert data[:2] == b"MZ", f"{path.name}: missing DOS header"
    pe = u32(data, 0x3C)
    assert data[pe : pe + 4] == b"PE\0\0", f"{path.name}: missing PE header"
    coff = pe + 4
    assert u16(data, coff) == 0x8664, f"{path.name}: not x86-64"
    section_count = u16(data, coff + 2)
    characteristics = u16(data, coff + 18)
    assert characteristics & 0x0002, f"{path.name}: not executable"
    assert not characteristics & 0x2000, f"{path.name}: incorrectly marked DLL"

    optional = coff + 20
    assert u16(data, optional) == 0x20B, f"{path.name}: not PE32+"
    assert u32(data, optional + 16) != 0, f"{path.name}: zero entry point"
    assert u64(data, optional + 24) == 0, f"{path.name}: preferred base is not zero"
    assert u32(data, optional + 32) == 4096, f"{path.name}: section alignment != 4K"
    assert u32(data, optional + 36) == 4096, f"{path.name}: file alignment != 4K"
    assert u16(data, optional + 68) == 10, f"{path.name}: not an EFI application"

    directory_count = u32(data, optional + 108)
    assert directory_count > 5, f"{path.name}: no relocation directory slot"
    reloc_rva, reloc_size = struct.unpack_from("<II", data, optional + 112 + 5 * 8)
    assert reloc_rva and reloc_size, f"{path.name}: empty relocation directory"

    section_table = optional + u16(data, coff + 16)
    section_names = {
        data[section_table + i * 40 : section_table + i * 40 + 8].rstrip(b"\0")
        for i in range(section_count)
    }
    assert b".reloc" in section_names, f"{path.name}: no .reloc section"

    if stage0:
        assert len(data) <= 128 * 1024, "stage 0 unexpectedly contains the kernel"
        for marker in (b"STAGE0 ENTER", b"LOAD_IMAGE 0x", b"START_IMAGE CALL"):
            assert marker in data, f"{path.name}: missing witness marker {marker!r}"


def copy_from_image(image: pathlib.Path, source: str, target: pathlib.Path) -> None:
    subprocess.run(
        ["mcopy", "-o", "-i", f"{image}@@1M", source, str(target)],
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
    )


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_image(image: pathlib.Path, stage0: pathlib.Path, kernel: pathlib.Path) -> None:
    with image.open("rb") as f:
        f.seek(512)
        gpt = f.read(512)
        assert gpt[:8] == b"EFI PART", "disk has no primary GPT"
        entries_lba = u64(gpt, 72)
        entry_size = u32(gpt, 84)
        f.seek(entries_lba * 512)
        first = f.read(entry_size)
        esp_start = u64(first, 32)
        assert esp_start == 2048, f"ESP starts at unexpected LBA {esp_start}"
        f.seek(esp_start * 512)
        bpb = f.read(512)

    assert bpb[82:90] == b"FAT32   ", "ESP is not FAT32"
    assert u32(bpb, 28) == esp_start, (
        f"FAT BPB_HiddSec={u32(bpb, 28)}, GPT start={esp_start}"
    )

    with tempfile.TemporaryDirectory() as tmp:
        tmpdir = pathlib.Path(tmp)
        image_stage0 = tmpdir / "BOOTX64.EFI"
        image_kernel = tmpdir / "ZLOS.EFI"
        copy_from_image(image, "::/EFI/BOOT/BOOTX64.EFI", image_stage0)
        copy_from_image(image, "::/EFI/ZLOS/ZLOS.EFI", image_kernel)
        assert sha256(image_stage0) == sha256(stage0), "ESP stage 0 differs from build"
        assert sha256(image_kernel) == sha256(kernel), "ESP kernel differs from build"

    inspected = json.loads(
        subprocess.run(
            [sys.executable, str(ROOT.parent / "tools" / "zllog.py"),
             "inspect", str(image), "--json"],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        ).stdout
    )
    selected_name = inspected["selected_superblock"]
    selected = next(s for s in inspected["superblocks"] if s["copy"] == selected_name)
    assert selected["image_id"] == sha256(kernel), (
        "ZLLOG identifies stage 0 instead of the real ZLOS.EFI kernel"
    )


def main() -> int:
    stage0 = ROOT / "BOOTX64.EFI"
    kernel = ROOT / "ZLOS.EFI"
    image = ROOT / "zlOS-usb.img"
    for path in (stage0, kernel, image):
        assert path.is_file(), f"missing build artifact: {path}"
    check_pe(stage0, stage0=True)
    check_pe(kernel, stage0=False)
    check_image(image, stage0, kernel)
    print("  ok    EFI stage 0/kernel PE layout and FAT handoff structure")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, subprocess.CalledProcessError) as exc:
        print(f"  FAIL  EFI stage 0 structure: {exc}", file=sys.stderr)
        raise SystemExit(1)
