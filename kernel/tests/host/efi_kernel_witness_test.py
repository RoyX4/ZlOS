#!/usr/bin/env python3
"""Structural gate for the real-kernel pre-ExitBootServices witness."""

from __future__ import annotations

import pathlib
import sys


KERNEL = pathlib.Path(__file__).resolve().parents[2]
SOURCE = KERNEL / "boot" / "efi.c"
FB_SOURCE = KERNEL / "src" / "graphics" / "framebuffer" / "fb.c"
GDT_SOURCE = KERNEL / "boot" / "gdt64.c"
IMAGE = KERNEL / "ZLOS.EFI"

MARKERS = (
    b"KERNEL_ENTRY",
    b"KERNEL_IMAGE base=",
    b"EFI_DIAG_ARM status=",
    b"ACPI_SCAN entries=",
    b"ACPI_RESULT rsdp=",
    b"GOP_RESULT status=",
    b"GOP_DETAILS width=",
    b"FRAMEBUFFER_WC locate=",
    b"BACK_BUFFER status=",
    b"MEMORY_MAP_ATTEMPT attempt=",
    b"MEMORY_MAP_RESULT status=",
    b"FIXED_MEMORY type=",
    b"BEFORE_EXIT_BOOT_SERVICES attempt=",
    b"EXIT_BOOT_SERVICES_FAILED status=",
)

# Clang deliberately scalarises short literals passed through witness_text(),
# so their bytes need not remain contiguous in an optimized PE image. These
# longer loop-copied markers do remain and prove this source reached ZLOS.EFI;
# OVMF below is the behavioural proof of the complete emitted lines.
BINARY_MARKERS = (
    b"MEMORY_MAP_ATTEMPT attempt=",
    b"MEMORY_MAP_RESULT status=",
    b"BEFORE_EXIT_BOOT_SERVICES attempt=",
    b"EXIT_BOOT_SERVICES_FAILED status=",
)


def main() -> int:
    source = SOURCE.read_text()
    fb_source = FB_SOURCE.read_text()
    gdt_source = GDT_SOURCE.read_text()
    image = IMAGE.read_bytes()

    for marker in MARKERS:
        assert marker.decode() in source, f"source missing {marker!r}"
    for marker in BINARY_MARKERS:
        assert marker in image, f"ZLOS.EFI missing {marker!r}"

    assert "#define WITNESS_SIZE_LIMIT (64ULL * 1024ULL)" in source
    assert "#define FIXED_ARENA_START 0x00800000ULL" in source
    assert "#define FIXED_ARENA_END   0x0A800000ULL" in source
    assert "#define HI_BACK_START     0x08000000ULL" in source
    assert "size == line->length" in source, "short EFI writes are not rejected"

    entry = source.index("MS efi_status efi_main")
    init = source.index("witness_init(image, st);", entry)
    acpi = source.index('witness_begin("ACPI_SCAN entries=")', entry)
    diag_arm = source.index('witness_begin("EFI_DIAG_ARM status=")', entry)
    gop = source.index('witness_begin("GOP_RESULT status=")', entry)
    wc = source.index('witness_begin("FRAMEBUFFER_WC locate=")', gop)
    back = source.index('witness_begin("BACK_BUFFER status=")', gop)
    attempt = source.index('witness_begin("MEMORY_MAP_ATTEMPT attempt=")', entry)
    result = source.index('witness_begin("MEMORY_MAP_RESULT status=")', entry)
    fixed = source.index("witness_fixed_memory(", result)
    before = source.index('witness_begin("BEFORE_EXIT_BOOT_SERVICES attempt=")', result)
    refresh = source.index("map_status = bs->get_memory_map(", before)
    exit_call = source.index("bs->exit_boot_services(image, map_key)", refresh)
    assert init < diag_arm < acpi < gop < wc < back < attempt < result < fixed < before < refresh < exit_call

    assert "EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS |" in source
    assert "EFI_VARIABLE_RUNTIME_ACCESS" in source
    assert "efi_persist_storage_diag" in source
    assert "DIAG_MAX_PORTS 31U" in source
    assert "DIAG_HEADER_BYTES 440U" in source
    assert "DIAG_BUFFER_BYTES <= 4096U" in source
    assert "runtime_diag[6] = '6'" in source
    assert "DIAG_ENTRY_BYTES 104U" in source
    assert "xhci_msc_port_stage" in source
    assert "xhci_msc_port_ep0_trace" in source
    assert "xhci_ep0_first_device_probe" in source
    assert "xhci_ep0_first_config_diag" in source
    assert "xhci_ep0_first_device_slot_context" in source
    assert "xhci_lifecycle_diag" in source
    assert "xhci_msc_last_opcode" in source
    assert "diag_fnv32(runtime_diag, total - 4U)" in source

    allocation = source.index("bs->allocate_pages(", gop)
    handoff = source.index("fb_set_back_buffer(fb_back_addr, fb_back_bytes);", allocation)
    assert allocation < back < handoff < attempt
    assert "loader_back = (unsigned int *)(fb_uptr)addr;" in fb_source
    assert "back = loader_back;" in fb_source
    assert "0x00CF92000000FFFFULL" in gdt_source
    assert "0x00AF92000000FFFFULL" not in gdt_source

    success = source.index("if (exit_status == EFI_SUCCESS)", exit_call)
    failure = source.index("} else {", success)
    success_arm = source[success:failure]
    assert "witness_" not in success_arm, (
        "successful ExitBootServices path must not call the filesystem"
    )
    assert "EXIT_BOOT_SERVICES_FAILED" in source[failure:]

    path_utf16 = "\\EFI\\ZLOS\\WITNESS.LOG".encode("utf-16-le") + b"\0\0"
    assert path_utf16 in image, "ZLOS.EFI has no UTF-16 witness path"
    variable_utf16 = "ZlBootDiag".encode("utf-16-le") + b"\0\0"
    assert variable_utf16 in image, "ZLOS.EFI has no UTF-16 fallback variable name"

    print("  ok    kernel EFI witness ordering, bounds and no-post-exit rule")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except AssertionError as exc:
        print(f"  FAIL  kernel EFI witness structure: {exc}", file=sys.stderr)
        raise SystemExit(1)
