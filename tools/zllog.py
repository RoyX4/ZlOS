#!/usr/bin/env python3
"""Build, inspect, and extract the zlOS persistent boot journal."""

from __future__ import annotations

import argparse
import binascii
import csv
import hashlib
import json
import os
import stat
import struct
import sys
import uuid
from dataclasses import dataclass
from pathlib import Path
from typing import BinaryIO, Iterable


TYPE_GUID = uuid.UUID("a2bc51d4-225d-4ad4-8db5-b0095953aa19")
PARTITION_LABEL = "ZLLOG"
PARTITION_BYTES = 64 * 1024 * 1024
PARTITION_MAX_BYTES = 512 * 1024 * 1024

SUPERBLOCK_BYTES = 4096
SUPER_A_OFFSET = 0
SUPER_B_OFFSET = SUPERBLOCK_BYTES
SLOTS_OFFSET = 2 * SUPERBLOCK_BYTES
SLOT_BYTES = 2 * 1024 * 1024
SLOT_HEADER_BYTES = 4096
RECORD_BYTES = 64

SUPER_MAGIC = b"ZLLOGV1\0"
SUPER_VERSION = 1
SUPER_HEADER_BYTES = 164
SUPER_COMMIT = b"ZLCOMMIT"
SUPER_CRC_OFFSET = 152
SUPER_STRUCT = struct.Struct("<8sHHIQIIQQQQQIIII16s16s32sI8s")

SLOT_MAGIC = b"ZLBOOT1\0"
SLOT_VERSION = 1
SLOT_HEADER_USED_BYTES = 132
SLOT_COMMIT = b"ZLSLOTOK"
SLOT_CRC_OFFSET = 120
SLOT_STRUCT = struct.Struct("<8sHHIIIQQQIIIII IQQ32sI8s".replace(" ", ""))

RECORD_STRUCT = struct.Struct("<QQHBBHHQQQ12sI")
ACTIVE_SLOT_NONE = 0xFFFFFFFF
SLOT_WRITING = 1
SLOT_COMPLETE = 2

GPT_SIGNATURE = b"EFI PART"
GPT_HEADER_MIN_BYTES = 92
GPT_HEADER_MAX_BYTES = 4096
GPT_ENTRY_LIMIT_BYTES = 16 * 1024 * 1024

SUB_INPUT = 2
SUB_STORAGE = 5
SUB_FRAME = 3
SUB_DISPLAY = 15
EV_STORAGE_REFUSED = 4
EV_STORAGE_XHCI = 5
EV_FRAME = 20
EV_FRAME_LATE = 21
EV_COUNTER = 50
EV_FRAME_EXT = 59
EV_FRAME_BURST = 60
EV_CHECKPOINT = 62
EV_POINTER = 63
EV_SNAPSHOT = 64
EV_FRAME_PAINT = 65
EV_LIFECYCLE = 66
EV_OPERATION_BEGIN = 67
EV_OPERATION_RESULT = 68
EV_DISPLAY_STATE = 58

SUBSYSTEM_NAMES = {
    1: "boot", 2: "input", 3: "frame", 4: "usb", 5: "storage",
    6: "kernel", 7: "cpu", 8: "irq", 9: "driver", 10: "fs",
    11: "syscall", 12: "sched", 13: "memory", 14: "net",
    15: "display", 16: "perf", 17: "lab",
}
EVENT_NAMES = {
    1: "boot-start", 2: "boot-milestone", 3: "storage-ready",
    4: "storage-refused", 5: "storage-xhci", 10: "input-batch",
    11: "input-event", 12: "input-drop", 20: "frame", 21: "frame-late",
    30: "flush-error", 31: "text", 40: "driver-state",
    41: "command-submit", 42: "command-complete", 43: "timeout",
    44: "fault", 45: "panic", 46: "syscall-enter", 47: "syscall-exit",
    48: "fs-mutation", 49: "drop", 50: "counter", 51: "span-begin",
    52: "span-end", 53: "trigger", 54: "burst", 55: "memory",
    56: "process", 57: "net-state", 58: "display-state",
    59: "frame-ext", 60: "frame-burst", 61: "recorder-drop",
    62: "checkpoint", 63: "pointer", 64: "snapshot",
    65: "frame-paint-phases", 66: "lifecycle",
    67: "operation-begin", 68: "operation-result",
}
OBJECT_NAMES = {
    1: "process", 2: "app", 3: "window", 4: "file", 5: "socket",
    6: "dns", 7: "http", 8: "kernel",
}
LIFECYCLE_NAMES = {
    1: "start", 2: "ready", 3: "exit", 4: "fault", 5: "refused",
}
OPERATION_NAMES = {
    0x2001: "file-mount", 0x2002: "file-format", 0x2003: "file-find",
    0x2004: "file-create", 0x2005: "file-open", 0x2006: "file-read",
    0x2007: "file-write", 0x2008: "file-close", 0x2009: "file-info",
    0x200A: "file-delete", 0x200B: "file-rename", 0x200C: "file-sync",
    0x3001: "net-connect", 0x3002: "net-send", 0x3003: "net-receive",
    0x3004: "net-close", 0x3005: "net-abort", 0x3006: "dns-resolve",
    0x3007: "http-request", 0x4001: "window-open",
    0x4002: "window-present", 0x4003: "window-poll",
    0x4004: "window-close", 0x5001: "ipc-send", 0x5002: "ipc-receive",
}
COUNTER_NAMES = {
    1: "irq-timer", 2: "irq-keyboard", 3: "irq-mouse", 4: "irq-stray",
    5: "xhci-event", 6: "xhci-command", 7: "xhci-transfer",
    8: "mmio-poll", 9: "nvme-command", 10: "nvme-poll",
    11: "net-rx", 12: "net-tx", 13: "net-drop", 14: "alloc",
    15: "free", 16: "alloc-bytes", 17: "heap-refuse",
    18: "sched-switch", 19: "sched-yield", 20: "syscall",
    21: "fs-read", 22: "fs-write", 23: "fs-mutation",
    24: "frame-paint", 25: "frame-late", 26: "frame-lost",
    27: "rendered-pixels", 28: "present-bytes", 29: "input-queue",
    30: "input-drop", 31: "recorder-overwrite", 32: "usb-write-bytes",
    33: "flush-us",
}
SNAPSHOT_NAMES = {
    1: "xhci-wait", 2: "nvme-csts", 3: "nvme-completion",
    4: "gpu-forcewake", 5: "gpu-ring", 6: "intel-vblank",
    7: "net-wait", 8: "tcp-timeout", 9: "xhci-command",
    10: "xhci-transfer",
}
MSC_INIT_STAGES = {
    0: "idle",
    1: "controller",
    2: "port",
    3: "enumerated",
    4: "config-header",
    5: "config-full",
    6: "interface",
    7: "set-config",
    8: "endpoints",
    9: "ready",
}
ZLLOG_ERRORS = {
    0: "none",
    1: "msc-init",
    2: "capacity",
    3: "block-size",
    4: "gpt",
    5: "superblock",
    6: "write-bounds",
    7: "write",
    8: "sync",
    9: "slot-full",
}
CACHE_TYPE_NAMES = {0: "unknown", 1: "uncacheable", 2: "write-combining", 3: "write-back"}


class ZllogError(Exception):
    """A malformed or unsafe journal source."""


@dataclass(frozen=True)
class Partition:
    number: int
    start_lba: int
    blocks: int
    block_bytes: int
    type_guid_raw: bytes
    unique_guid_raw: bytes
    label: str

    @property
    def byte_offset(self) -> int:
        return self.start_lba * self.block_bytes

    @property
    def byte_count(self) -> int:
        return self.blocks * self.block_bytes


@dataclass(frozen=True)
class Source:
    path: Path
    base_offset: int
    byte_count: int
    partition: Partition | None
    disk_blocks: int | None
    logical_block_bytes: int | None


def crc32(data: bytes) -> int:
    return binascii.crc32(data) & 0xFFFFFFFF


def _crc_block(block: bytes, crc_offset: int) -> int:
    mutable = bytearray(block)
    mutable[crc_offset : crc_offset + 4] = b"\0" * 4
    return crc32(mutable)


def _path_size(path: Path) -> int:
    size = path.stat().st_size
    if size:
        return size
    if stat.S_ISBLK(path.stat().st_mode):
        import fcntl

        BLKGETSIZE64 = 0x80081272
        with path.open("rb", buffering=0) as stream:
            return struct.unpack("Q", fcntl.ioctl(stream.fileno(), BLKGETSIZE64, b"\0" * 8))[0]
    return size


def _read_exact(stream: BinaryIO, offset: int, count: int) -> bytes:
    stream.seek(offset)
    data = stream.read(count)
    if len(data) != count:
        raise ZllogError(f"short read at byte {offset}: wanted {count}, got {len(data)}")
    return data


def _decode_guid(raw: bytes) -> str:
    return str(uuid.UUID(bytes_le=raw))


def _parse_gpt_at(stream: BinaryIO, disk_bytes: int, block_bytes: int) -> tuple[int, list[Partition]]:
    header_block = _read_exact(stream, block_bytes, block_bytes)
    if header_block[:8] != GPT_SIGNATURE:
        raise ZllogError("no GPT signature")
    header_size = struct.unpack_from("<I", header_block, 12)[0]
    if not GPT_HEADER_MIN_BYTES <= header_size <= min(block_bytes, GPT_HEADER_MAX_BYTES):
        raise ZllogError(f"invalid GPT header size {header_size}")
    stored_header_crc = struct.unpack_from("<I", header_block, 16)[0]
    header_for_crc = bytearray(header_block[:header_size])
    header_for_crc[16:20] = b"\0" * 4
    if crc32(header_for_crc) != stored_header_crc:
        raise ZllogError("GPT header CRC mismatch")

    current_lba = struct.unpack_from("<Q", header_block, 24)[0]
    first_usable = struct.unpack_from("<Q", header_block, 40)[0]
    last_usable = struct.unpack_from("<Q", header_block, 48)[0]
    entries_lba = struct.unpack_from("<Q", header_block, 72)[0]
    entry_count = struct.unpack_from("<I", header_block, 80)[0]
    entry_bytes = struct.unpack_from("<I", header_block, 84)[0]
    stored_entries_crc = struct.unpack_from("<I", header_block, 88)[0]
    disk_blocks = disk_bytes // block_bytes
    if current_lba != 1 or disk_bytes % block_bytes or last_usable >= disk_blocks:
        raise ZllogError("GPT geometry is outside the source")
    if entry_bytes < 128 or entry_bytes % 8:
        raise ZllogError(f"invalid GPT entry size {entry_bytes}")
    entries_bytes = entry_count * entry_bytes
    if entries_bytes <= 0 or entries_bytes > GPT_ENTRY_LIMIT_BYTES:
        raise ZllogError(f"unsafe GPT entry array size {entries_bytes}")
    if entries_lba * block_bytes + entries_bytes > disk_bytes:
        raise ZllogError("GPT entry array is outside the source")
    entries = _read_exact(stream, entries_lba * block_bytes, entries_bytes)
    if crc32(entries) != stored_entries_crc:
        raise ZllogError("GPT entry array CRC mismatch")

    partitions: list[Partition] = []
    for index in range(entry_count):
        entry = entries[index * entry_bytes : (index + 1) * entry_bytes]
        type_guid = entry[:16]
        if type_guid == b"\0" * 16:
            continue
        unique_guid = entry[16:32]
        start_lba, end_lba = struct.unpack_from("<QQ", entry, 32)
        if start_lba > end_lba or start_lba < first_usable or end_lba > last_usable:
            raise ZllogError(f"GPT partition {index + 1} is outside usable disk bounds")
        label_bytes = entry[56 : min(entry_bytes, 128)]
        try:
            label = label_bytes.decode("utf-16-le").split("\0", 1)[0]
        except UnicodeDecodeError as error:
            raise ZllogError(f"GPT partition {index + 1} has an invalid label") from error
        partitions.append(
            Partition(
                number=index + 1,
                start_lba=start_lba,
                blocks=end_lba - start_lba + 1,
                block_bytes=block_bytes,
                type_guid_raw=type_guid,
                unique_guid_raw=unique_guid,
                label=label,
            )
        )
    return disk_blocks, partitions


def parse_gpt(stream: BinaryIO, disk_bytes: int) -> tuple[int, int, list[Partition]]:
    errors: list[str] = []
    for block_bytes in (512, 4096):
        if disk_bytes < 2 * block_bytes or disk_bytes % block_bytes:
            continue
        try:
            disk_blocks, partitions = _parse_gpt_at(stream, disk_bytes, block_bytes)
            return block_bytes, disk_blocks, partitions
        except ZllogError as error:
            errors.append(f"{block_bytes}: {error}")
    raise ZllogError("not a valid GPT disk (" + "; ".join(errors) + ")")


def locate_source(path: Path) -> Source:
    disk_bytes = _path_size(path)
    if disk_bytes < 2 * SUPERBLOCK_BYTES:
        raise ZllogError(f"source is only {disk_bytes} bytes")
    with path.open("rb", buffering=0) as stream:
        super_a_magic = _read_exact(stream, SUPER_A_OFFSET, 8)
        super_b_magic = _read_exact(stream, SUPER_B_OFFSET, 8)
        if super_a_magic == SUPER_MAGIC or super_b_magic == SUPER_MAGIC:
            return Source(path, 0, disk_bytes, None, None, None)
        block_bytes, disk_blocks, partitions = parse_gpt(stream, disk_bytes)
    matches = [
        partition
        for partition in partitions
        if partition.type_guid_raw == TYPE_GUID.bytes_le and partition.label == PARTITION_LABEL
    ]
    if len(matches) != 1:
        raise ZllogError(
            f"expected exactly one GPT partition with type {TYPE_GUID} and label {PARTITION_LABEL}, found {len(matches)}"
        )
    partition = matches[0]
    if partition.byte_offset + partition.byte_count > disk_bytes:
        raise ZllogError("ZLLOG partition extends beyond the disk")
    return Source(path, partition.byte_offset, partition.byte_count, partition, disk_blocks, block_bytes)


def _super_values(source: Source, generation: int, image_id: bytes) -> tuple[object, ...]:
    if source.partition is None or source.disk_blocks is None or source.logical_block_bytes is None:
        raise ZllogError("initialization requires a whole GPT disk image, not a raw partition")
    partition = source.partition
    slot_count = (partition.byte_count - SLOTS_OFFSET) // SLOT_BYTES
    return (
        SUPER_MAGIC,
        SUPER_VERSION,
        SUPER_HEADER_BYTES,
        SUPERBLOCK_BYTES,
        generation,
        ACTIVE_SLOT_NONE,
        slot_count,
        1,
        0,
        source.disk_blocks,
        partition.start_lba,
        partition.blocks,
        source.logical_block_bytes,
        RECORD_BYTES,
        SLOT_BYTES,
        0,
        TYPE_GUID.bytes_le,
        partition.unique_guid_raw,
        image_id,
        0,
        SUPER_COMMIT,
    )


def encode_super(source: Source, generation: int, image_id: bytes) -> bytes:
    block = bytearray(SUPERBLOCK_BYTES)
    SUPER_STRUCT.pack_into(block, 0, *_super_values(source, generation, image_id))
    struct.pack_into("<I", block, SUPER_CRC_OFFSET, _crc_block(block, SUPER_CRC_OFFSET))
    return bytes(block)


def decode_super(block: bytes, copy_name: str) -> dict[str, object]:
    result: dict[str, object] = {"copy": copy_name, "valid": False}
    if len(block) != SUPERBLOCK_BYTES:
        result["error"] = "short superblock"
        return result
    values = SUPER_STRUCT.unpack_from(block)
    (
        magic,
        version,
        header_bytes,
        superblock_bytes,
        generation,
        active_slot,
        slot_count,
        next_sequence,
        completed_boots,
        disk_blocks,
        partition_start_lba,
        partition_blocks,
        logical_block_bytes,
        record_bytes,
        slot_bytes,
        flags,
        type_guid_raw,
        unique_guid_raw,
        image_id,
        stored_crc,
        commit,
    ) = values
    checks = [
        (magic == SUPER_MAGIC, "bad magic"),
        (version == SUPER_VERSION, f"unsupported version {version}"),
        (header_bytes == SUPER_HEADER_BYTES, f"bad header size {header_bytes}"),
        (superblock_bytes == SUPERBLOCK_BYTES, f"bad superblock size {superblock_bytes}"),
        (commit == SUPER_COMMIT, "missing commit marker"),
        (stored_crc == _crc_block(block, SUPER_CRC_OFFSET), "CRC mismatch"),
        (logical_block_bytes in (512, 4096), f"unsupported block size {logical_block_bytes}"),
        (record_bytes == RECORD_BYTES, f"bad record size {record_bytes}"),
        (slot_bytes == SLOT_BYTES, f"bad slot size {slot_bytes}"),
        (type_guid_raw == TYPE_GUID.bytes_le, "wrong partition type GUID"),
        (slot_count > 0, "no boot slots"),
        (active_slot == ACTIVE_SLOT_NONE or active_slot < slot_count, "active slot out of range"),
    ]
    failure = next((message for ok, message in checks if not ok), None)
    result.update(
        {
            "generation": generation,
            "active_slot": active_slot,
            "slot_count": slot_count,
            "next_sequence": next_sequence,
            "completed_boots": completed_boots,
            "disk_blocks": disk_blocks,
            "partition_start_lba": partition_start_lba,
            "partition_blocks": partition_blocks,
            "logical_block_bytes": logical_block_bytes,
            "record_bytes": record_bytes,
            "slot_bytes": slot_bytes,
            "flags": flags,
            "partition_type_guid": _decode_guid(type_guid_raw),
            "partition_unique_guid": _decode_guid(unique_guid_raw),
            "image_id": image_id.hex(),
            "stored_crc32": f"{stored_crc:08x}",
        }
    )
    if failure:
        result["error"] = failure
    else:
        result["valid"] = True
    return result


def read_supers(source: Source) -> tuple[list[dict[str, object]], dict[str, object]]:
    with source.path.open("rb", buffering=0) as stream:
        supers = [
            decode_super(_read_exact(stream, source.base_offset + SUPER_A_OFFSET, SUPERBLOCK_BYTES), "A"),
            decode_super(_read_exact(stream, source.base_offset + SUPER_B_OFFSET, SUPERBLOCK_BYTES), "B"),
        ]
    for superblock in supers:
        if not superblock["valid"]:
            continue
        try:
            _validate_super_against_source(source, superblock)
        except ZllogError as error:
            superblock["valid"] = False
            superblock["error"] = str(error)
    valid = [superblock for superblock in supers if superblock["valid"]]
    if not valid:
        errors = ", ".join(f"{item['copy']}: {item.get('error', 'invalid')}" for item in supers)
        raise ZllogError(f"no valid superblock ({errors})")
    selected = max(valid, key=lambda item: (int(item["generation"]), item["copy"] == "A"))
    return supers, selected


def _validate_super_against_source(source: Source, superblock: dict[str, object]) -> None:
    block_bytes = int(superblock["logical_block_bytes"])
    part_bytes = int(superblock["partition_blocks"]) * block_bytes
    expected_slots = (part_bytes - SLOTS_OFFSET) // SLOT_BYTES
    if int(superblock["slot_count"]) != expected_slots:
        raise ZllogError("superblock slot count does not match its partition bounds")
    if SLOTS_OFFSET + expected_slots * SLOT_BYTES > part_bytes:
        raise ZllogError("superblock slots extend beyond the recorded partition")
    if source.partition is None:
        if part_bytes > source.byte_count:
            raise ZllogError("recorded partition is larger than the raw source")
        return
    partition = source.partition
    actual = (
        source.disk_blocks,
        partition.start_lba,
        partition.blocks,
        partition.block_bytes,
        _decode_guid(partition.unique_guid_raw),
    )
    recorded = (
        int(superblock["disk_blocks"]),
        int(superblock["partition_start_lba"]),
        int(superblock["partition_blocks"]),
        block_bytes,
        superblock["partition_unique_guid"],
    )
    if actual != recorded:
        raise ZllogError("superblock disk/partition bounds or unique GUID do not match GPT")


def initialize(path: Path, image_id: bytes) -> Source:
    if len(image_id) != 32:
        raise ZllogError("image identity must be exactly 32 bytes")
    source = locate_source(path)
    if source.partition is None:
        raise ZllogError("refusing to initialize a raw partition: exact GPT type, label, and bounds are required")
    if not PARTITION_BYTES <= source.partition.byte_count <= PARTITION_MAX_BYTES:
        raise ZllogError(
            f"refusing ZLLOG partition of {source.partition.byte_count} bytes; "
            f"v1 accepts {PARTITION_BYTES}..{PARTITION_MAX_BYTES} bytes"
        )
    if source.partition.byte_offset % (1024 * 1024):
        raise ZllogError("refusing an unaligned ZLLOG partition")

    zero = b"\0" * (1024 * 1024)
    with path.open("r+b", buffering=0) as stream:
        remaining = source.byte_count
        offset = source.base_offset
        while remaining:
            count = min(remaining, len(zero))
            stream.seek(offset)
            stream.write(zero[:count])
            offset += count
            remaining -= count
        stream.seek(source.base_offset + SUPER_A_OFFSET)
        stream.write(encode_super(source, 1, image_id))
        stream.seek(source.base_offset + SUPER_B_OFFSET)
        stream.write(encode_super(source, 0, image_id))
        stream.flush()
        os.fsync(stream.fileno())

    _, selected = read_supers(source)
    if int(selected["generation"]) != 1:
        raise ZllogError("journal verification selected the wrong generation")
    return source


def decode_slot_header(block: bytes) -> dict[str, object]:
    result: dict[str, object] = {"valid": False}
    if len(block) != SLOT_HEADER_BYTES:
        result["error"] = "short slot header"
        return result
    if block == b"\0" * SLOT_HEADER_BYTES:
        result["empty"] = True
        return result
    values = SLOT_STRUCT.unpack_from(block)
    (
        magic,
        version,
        header_bytes,
        slot_bytes,
        state_value,
        slot_index,
        boot_id,
        first_sequence,
        last_sequence,
        record_count,
        record_capacity,
        dropped_records,
        boot_path,
        flags,
        reserved,
        started_tsc,
        ended_tsc,
        build_hash,
        stored_crc,
        commit,
    ) = values
    checks = [
        (magic == SLOT_MAGIC, "bad magic"),
        (version == SLOT_VERSION, f"unsupported version {version}"),
        (header_bytes == SLOT_HEADER_USED_BYTES, f"bad header size {header_bytes}"),
        (slot_bytes == SLOT_BYTES, f"bad slot size {slot_bytes}"),
        (state_value in (SLOT_WRITING, SLOT_COMPLETE), f"bad state {state_value}"),
        (record_capacity == (SLOT_BYTES - SLOT_HEADER_BYTES) // RECORD_BYTES, "bad record capacity"),
        (record_count <= record_capacity, "record count exceeds capacity"),
        (reserved == 0, "reserved field is nonzero"),
        (commit == SLOT_COMMIT, "missing commit marker"),
        (stored_crc == _crc_block(block, SLOT_CRC_OFFSET), "CRC mismatch"),
    ]
    failure = next((message for ok, message in checks if not ok), None)
    result.update(
        {
            "slot_index": slot_index,
            "state": "complete" if state_value == SLOT_COMPLETE else "writing",
            "state_value": state_value,
            "boot_id": boot_id,
            "first_sequence": first_sequence,
            "last_sequence": last_sequence,
            "record_count": record_count,
            "record_capacity": record_capacity,
            "dropped_records": dropped_records,
            "boot_path": boot_path,
            "flags": flags,
            "started_tsc": started_tsc,
            "ended_tsc": ended_tsc,
            "build_hash": build_hash.hex(),
            "stored_crc32": f"{stored_crc:08x}",
        }
    )
    if failure:
        result["error"] = failure
    else:
        result["valid"] = True
    return result


def encode_slot_header(
    *,
    slot_index: int,
    state_value: int,
    boot_id: int,
    first_sequence: int,
    last_sequence: int,
    record_count: int,
    dropped_records: int,
    boot_path: int,
    flags: int,
    started_tsc: int,
    ended_tsc: int,
    build_hash: bytes,
) -> bytes:
    if state_value not in (SLOT_WRITING, SLOT_COMPLETE):
        raise ZllogError(f"invalid slot state {state_value}")
    if len(build_hash) != 32:
        raise ZllogError("slot build hash must be exactly 32 bytes")
    record_capacity = (SLOT_BYTES - SLOT_HEADER_BYTES) // RECORD_BYTES
    if not 0 <= record_count <= record_capacity:
        raise ZllogError(f"record count {record_count} exceeds capacity {record_capacity}")
    block = bytearray(SLOT_HEADER_BYTES)
    SLOT_STRUCT.pack_into(
        block,
        0,
        SLOT_MAGIC,
        SLOT_VERSION,
        SLOT_HEADER_USED_BYTES,
        SLOT_BYTES,
        state_value,
        slot_index,
        boot_id,
        first_sequence,
        last_sequence,
        record_count,
        record_capacity,
        dropped_records,
        boot_path,
        flags,
        0,
        started_tsc,
        ended_tsc,
        build_hash,
        0,
        SLOT_COMMIT,
    )
    struct.pack_into("<I", block, SLOT_CRC_OFFSET, _crc_block(block, SLOT_CRC_OFFSET))
    return bytes(block)


def encode_record(
    *,
    sequence: int,
    tsc: int,
    cpu: int,
    severity: int,
    subsystem: int,
    event: int,
    value0: int = 0,
    value1: int = 0,
    value2: int = 0,
    payload: bytes = b"",
) -> bytes:
    if len(payload) > 12:
        raise ZllogError("record payload cannot exceed 12 bytes")
    block = bytearray(RECORD_BYTES)
    RECORD_STRUCT.pack_into(
        block,
        0,
        sequence,
        tsc,
        cpu,
        severity,
        subsystem,
        event,
        len(payload),
        value0,
        value1,
        value2,
        payload.ljust(12, b"\0"),
        0,
    )
    struct.pack_into("<I", block, 60, crc32(block[:60]))
    return bytes(block)


def decode_record(block: bytes, index: int) -> dict[str, object]:
    if len(block) != RECORD_BYTES:
        return {"index": index, "valid": False, "error": "short record"}
    if block == b"\0" * RECORD_BYTES:
        return {"index": index, "valid": False, "error": "empty record"}
    values = RECORD_STRUCT.unpack(block)
    sequence, tsc, cpu, severity, subsystem, event, payload_len, value0, value1, value2, payload, stored_crc = values
    result: dict[str, object] = {
        "index": index,
        "valid": False,
        "sequence": sequence,
        "tsc": tsc,
        "cpu": cpu,
        "severity": severity,
        "subsystem": subsystem,
        "event": event,
        "subsystem_name": SUBSYSTEM_NAMES.get(subsystem, f"subsystem-{subsystem}"),
        "event_name": EVENT_NAMES.get(event, f"event-{event}"),
        "payload_len": payload_len,
        "value0": value0,
        "value1": value1,
        "value2": value2,
        "stored_crc32": f"{stored_crc:08x}",
    }
    if payload_len > 12:
        result["error"] = f"payload length {payload_len} exceeds 12"
        return result
    if crc32(block[:60]) != stored_crc:
        result["error"] = "CRC mismatch"
        return result
    actual_payload = payload[:payload_len]
    result["payload_hex"] = actual_payload.hex()
    result["payload_text"] = actual_payload.decode("utf-8", errors="replace")
    if event == EV_LIFECYCLE:
        object_kind = value0 >> 32
        action = value1 >> 32
        result["diagnostic"] = {
            "kind": "lifecycle",
            "object_kind": object_kind,
            "object_kind_name": OBJECT_NAMES.get(object_kind, f"object-{object_kind}"),
            "object_id": value0 & 0xFFFFFFFF,
            "action": action,
            "action_name": LIFECYCLE_NAMES.get(action, f"action-{action}"),
            "parent_id": value1 & 0xFFFFFFFF,
            "detail": value2,
        }
    elif event == EV_OPERATION_BEGIN:
        operation = value2 >> 32
        actor_kind = value1 >> 32
        result["diagnostic"] = {
            "kind": "operation-begin",
            "operation_id": value0,
            "operation": operation,
            "operation_name": (
                f"syscall-{operation - 0x1000}" if 0x1000 <= operation < 0x2000
                else OPERATION_NAMES.get(operation, f"operation-{operation}")
            ),
            "actor_kind": actor_kind,
            "actor_kind_name": OBJECT_NAMES.get(actor_kind, f"object-{actor_kind}"),
            "actor_id": value1 & 0xFFFFFFFF,
            "object_id": value2 & 0xFFFFFFFF,
        }
    elif event == EV_OPERATION_RESULT:
        operation = value2 & 0xFFFFFFFF
        signed_result = value1 if value1 < (1 << 63) else value1 - (1 << 64)
        detail = struct.unpack("<I", actual_payload[:4])[0] if payload_len == 4 else 0
        result["diagnostic"] = {
            "kind": "operation-result",
            "operation_id": value0,
            "operation": operation,
            "operation_name": (
                f"syscall-{operation - 0x1000}" if 0x1000 <= operation < 0x2000
                else OPERATION_NAMES.get(operation, f"operation-{operation}")
            ),
            "result": signed_result,
            "error": value2 >> 32,
            "detail": detail,
        }
    elif subsystem == SUB_STORAGE and event == EV_STORAGE_REFUSED:
        detail = value2 & 0xFFFFFFFF
        stage = detail & 0xFF
        result["diagnostic"] = {
            "kind": "storage-refused",
            "reason": ZLLOG_ERRORS.get(value0, f"unknown-{value0}"),
            "msc_result": value1,
            "msc_stage": stage,
            "msc_stage_name": MSC_INIT_STAGES.get(stage, f"unknown-{stage}"),
            "port": (detail >> 8) & 0xFF,
            "slot": (detail >> 16) & 0xFF,
            "xhci_completion_code": (detail >> 24) & 0xFF,
        }
    elif subsystem == SUB_STORAGE and event == EV_STORAGE_XHCI:
        result["diagnostic"] = {
            "kind": "xhci-state",
            "portsc": f"0x{value0 & 0xFFFFFFFF:08x}",
            "usbsts": f"0x{value1 & 0xFFFFFFFF:08x}",
            "usbcmd": f"0x{value2 & 0xFFFFFFFF:08x}",
        }
    elif subsystem == SUB_INPUT and event == EV_POINTER:
        result["diagnostic"] = {
            "kind": "pointer",
            "x": value0 & 0xFFFFFFFF,
            "y": value0 >> 32,
            "buttons": value1 & 0xFFFFFFFF,
            "queue_depth": value1 >> 32,
            "source_tsc_low": value2 & 0xFFFFFFFF,
            "input_sequence": value2 >> 32,
        }
    elif event == EV_SNAPSHOT:
        phase = value0 & 0xFF
        operation = value0 >> 8
        result["diagnostic"] = {
            "kind": "snapshot",
            "operation": operation,
            "operation_name": SNAPSHOT_NAMES.get(operation, f"operation-{operation}"),
            "phase": phase,
            "phase_name": {0: "before", 1: "trigger", 2: "after"}.get(
                phase, f"phase-{phase}"
            ),
            "raw0": f"0x{value1 & 0xFFFFFFFFFFFFFFFF:016x}",
            "raw1": f"0x{value2 & 0xFFFFFFFFFFFFFFFF:016x}",
        }
    elif subsystem == SUB_DISPLAY and event == EV_DISPLAY_STATE:
        cache_type = (value2 >> 9) & 3
        result["diagnostic"] = {
            "kind": "display-state",
            "width": value0,
            "height": value1,
            "bits_per_pixel": value2 & 0xFF,
            "back_buffer": bool(value2 & 0x100),
            "cache_type": cache_type,
            "cache_type_name": CACHE_TYPE_NAMES[cache_type],
            "pitch_bytes": (value2 >> 16) & 0xFFFF,
        }
    elif subsystem == SUB_FRAME and event in (EV_FRAME, EV_FRAME_LATE) and payload_len == 12:
        vblank_us, present_us, total_us = struct.unpack("<III", actual_payload)
        meta = (value2 >> 32) & 0xFFFFFFFF
        result["diagnostic"] = {
            "kind": "frame",
            "input_us": value0,
            "tick_us": value1,
            "compositor_us": value2 & 0xFFFFFFFF,
            "vblank_us": vblank_us,
            "present_us": present_us,
            "total_us": total_us,
            "late": bool(meta & 1),
            "periodic_sample": bool(meta & 2),
            "waited_vblank": bool(meta & 4),
            "damage_rects": (meta >> 3) & 0xF,
            "damage_pixels": (meta >> 7) & 0x01FFFFFF,
        }
    elif event == EV_COUNTER and payload_len == 12:
        samples, total_delta, sample_delta = struct.unpack("<III", actual_payload)
        result["diagnostic"] = {
            "kind": "counter",
            "counter_id": value0,
            "counter_name": COUNTER_NAMES.get(value0, f"counter-{value0}"),
            "total": value1,
            "maximum": value2,
            "samples": samples,
            "total_delta": total_delta,
            "sample_delta": sample_delta,
        }
    elif subsystem == 15 and event == EV_FRAME_EXT and payload_len == 12:
        missed, queue_depth, present_bytes = struct.unpack("<III", actual_payload)
        cursor_only = bool(present_bytes & 0x80000000)
        present_bytes &= 0x7FFFFFFF
        result["diagnostic"] = {
            "kind": "frame-ext",
            "frame_sequence": value0 & 0xFFFFFFFF,
            "input_sequence": value0 >> 32,
            "input_to_present_us": value1,
            "damage_pixels": value2 & 0xFFFFFFFF,
            "present_bytes": present_bytes,
            "cursor_only": cursor_only,
            "missed_deadlines": missed,
            "input_queue_depth": queue_depth,
        }
    elif subsystem == 15 and event == EV_FRAME_BURST and payload_len == 12:
        damage, present_bytes, flags = struct.unpack("<III", actual_payload)
        result["diagnostic"] = {
            "kind": "frame-burst",
            "frame_sequence": value0 & 0xFFFFFFFF,
            "input_sequence": value0 >> 32,
            "total_us": value1 & 0xFFFFFFFF,
            "present_us": value1 >> 32,
            "compositor_us": value2 & 0xFFFFFFFF,
            "input_to_present_us": value2 >> 32,
            "damage_pixels": damage,
            "present_bytes": present_bytes,
            "flags": flags,
        }
    elif subsystem == 15 and event == EV_FRAME_PAINT and payload_len == 12:
        repaint_pixels, window_visits, app_calls = struct.unpack("<III", actual_payload)
        result["diagnostic"] = {
            "kind": "frame-paint-phases",
            "frame_sequence": value0 & 0xFFFFFFFF,
            "repaint_rects": value0 >> 32,
            "desk_us": value1 & 0xFFFFFFFF,
            "chrome_us": value1 >> 32,
            "app_us": value2 & 0xFFFFFFFF,
            "effects_us": value2 >> 32,
            "repaint_pixels": repaint_pixels,
            "window_visits": window_visits,
            "app_calls": app_calls,
        }
    result["valid"] = True
    return result


def read_boots(source: Source, selected: dict[str, object], latest: bool) -> tuple[list[dict[str, object]], list[str]]:
    boots: list[dict[str, object]] = []
    warnings: list[str] = []
    slot_count = int(selected["slot_count"])
    with source.path.open("rb", buffering=0) as stream:
        for slot_index in range(slot_count):
            slot_offset = source.base_offset + SLOTS_OFFSET + slot_index * SLOT_BYTES
            header = decode_slot_header(_read_exact(stream, slot_offset, SLOT_HEADER_BYTES))
            if header.get("empty"):
                continue
            if not header["valid"]:
                warnings.append(f"slot {slot_index}: {header.get('error', 'invalid header')}")
                # A power cut can tear the one mutable WRITING header after
                # its records were flushed. The selected superblock still
                # names that active slot, and every record has its own CRC and
                # monotonic sequence, so recover the durable prefix instead
                # of hiding the most useful failure evidence.
                if int(selected["active_slot"]) != slot_index:
                    continue
                records: list[dict[str, object]] = []
                previous_sequence = -1
                record_capacity = (SLOT_BYTES - SLOT_HEADER_BYTES) // RECORD_BYTES
                for record_index in range(record_capacity):
                    record_offset = slot_offset + SLOT_HEADER_BYTES + record_index * RECORD_BYTES
                    raw = _read_exact(stream, record_offset, RECORD_BYTES)
                    if raw == b"\0" * RECORD_BYTES:
                        break
                    record = decode_record(raw, record_index)
                    if not record["valid"]:
                        warnings.append(
                            f"slot {slot_index} recovered record {record_index}: "
                            f"{record.get('error', 'invalid')}"
                        )
                        break
                    sequence = int(record["sequence"])
                    if sequence <= previous_sequence:
                        warnings.append(
                            f"slot {slot_index}: recovery stopped at stale/non-monotonic "
                            f"sequence {sequence}"
                        )
                        break
                    previous_sequence = sequence
                    records.append(record)
                if records:
                    boot_id = int(header.get("boot_id", selected["generation"]))
                    if boot_id <= 0:
                        boot_id = int(selected["generation"])
                    boots.append(
                        {
                            "valid": True,
                            "recovered": True,
                            "slot_index": slot_index,
                            "state": "recovered-writing",
                            "state_value": SLOT_WRITING,
                            "boot_id": boot_id,
                            "first_sequence": int(records[0]["sequence"]),
                            "last_sequence": int(records[-1]["sequence"]),
                            "record_count": len(records),
                            "record_capacity": record_capacity,
                            "dropped_records": int(header.get("dropped_records", 0)),
                            "boot_path": int(header.get("boot_path", 0)),
                            "flags": int(header.get("flags", 0)),
                            "started_tsc": int(header.get("started_tsc", 0)),
                            "ended_tsc": 0,
                            "build_hash": header.get("build_hash", ""),
                            "records": records,
                        }
                    )
                continue
            if int(header["slot_index"]) != slot_index:
                warnings.append(f"slot {slot_index}: header claims slot {header['slot_index']}")
                continue
            records: list[dict[str, object]] = []
            for record_index in range(int(header["record_count"])):
                record_offset = slot_offset + SLOT_HEADER_BYTES + record_index * RECORD_BYTES
                record = decode_record(_read_exact(stream, record_offset, RECORD_BYTES), record_index)
                if not record["valid"]:
                    warnings.append(f"slot {slot_index} record {record_index}: {record.get('error', 'invalid')}")
                    continue
                records.append(record)
            boot = dict(header)
            boot["records"] = records
            boots.append(boot)
    boots.sort(key=lambda item: (int(item["boot_id"]),
                                 int(item.get("first_sequence", 0))))
    if latest and boots:
        latest_id = int(boots[-1]["boot_id"])
        boots = [boot for boot in boots if int(boot["boot_id"]) == latest_id]
    return boots, warnings


def journal_report(source: Source, latest: bool) -> dict[str, object]:
    supers, selected = read_supers(source)
    boots, warnings = read_boots(source, selected, latest)
    return {
        "source": str(source.path),
        "source_kind": "gpt-disk" if source.partition else "raw-partition",
        "partition_offset": source.base_offset,
        "partition_bytes": source.byte_count,
        "superblocks": supers,
        "selected_superblock": selected["copy"],
        "boots": boots,
        "warnings": warnings,
    }


def audit_report(report: dict[str, object]) -> dict[str, object]:
    """Return only typed lifecycle/operation records, correlated per boot."""
    audited_boots: list[dict[str, object]] = []
    for boot in report["boots"]:
        begins: dict[int, dict[str, object]] = {}
        transactions: list[dict[str, object]] = []
        lifecycle: list[dict[str, object]] = []
        unmatched_results: list[dict[str, object]] = []
        for record in boot["records"]:
            diagnostic = record.get("diagnostic", {})
            kind = diagnostic.get("kind")
            if kind == "lifecycle":
                lifecycle.append({
                    "sequence": record["sequence"], "tsc": record["tsc"],
                    **diagnostic,
                })
            elif kind == "operation-begin":
                begins[int(diagnostic["operation_id"])] = {
                    "begin_sequence": record["sequence"],
                    "begin_tsc": record["tsc"], **diagnostic,
                }
            elif kind == "operation-result":
                operation_id = int(diagnostic["operation_id"])
                begin = begins.pop(operation_id, None)
                completed = {
                    **(begin or {}),
                    "result_sequence": record["sequence"],
                    "result_tsc": record["tsc"],
                    "result": diagnostic["result"],
                    "error": diagnostic["error"],
                    "detail": diagnostic["detail"],
                    "operation_id": operation_id,
                    "operation": diagnostic["operation"],
                    "operation_name": diagnostic["operation_name"],
                }
                if begin is None:
                    unmatched_results.append(completed)
                else:
                    transactions.append(completed)
        audited_boots.append({
            "boot_id": boot["boot_id"], "slot": boot["slot_index"],
            "state": boot["state"], "lifecycle": lifecycle,
            "transactions": transactions,
            "incomplete_operations": list(begins.values()),
            "unmatched_results": unmatched_results,
        })
    return {
        "source": report["source"], "boots": audited_boots,
        "warnings": report["warnings"],
    }


def _human_audit(report: dict[str, object]) -> str:
    lines = [f"source: {report['source']}"]
    if not report["boots"]:
        return "\n".join(lines + ["audit boots: none recorded"])
    for boot in report["boots"]:
        lines.append(
            f"boot {boot['boot_id']}: {len(boot['lifecycle'])} lifecycle, "
            f"{len(boot['transactions'])} completed operation(s), "
            f"{len(boot['incomplete_operations'])} incomplete, "
            f"{len(boot['unmatched_results'])} unmatched result(s)"
        )
        items: list[tuple[int, str]] = []
        for life in boot["lifecycle"]:
            items.append((int(life["sequence"]),
                f"  #{life['sequence']} {life['object_kind_name']}#"
                f"{life['object_id']} {life['action_name']} "
                f"parent={life['parent_id']} detail={life['detail']}"))
        for operation in boot["transactions"]:
            items.append((int(operation["begin_sequence"]),
                f"  #{operation['begin_sequence']}..#{operation['result_sequence']} "
                f"audit {operation['operation_id']} {operation['operation_name']} "
                f"{operation['actor_kind_name']}#{operation['actor_id']} "
                f"object={operation['object_id']} -> result={operation['result']} "
                f"error={operation['error']} detail={operation['detail']}"))
        for operation in boot["incomplete_operations"]:
            items.append((int(operation["begin_sequence"]),
                f"  #{operation['begin_sequence']} audit {operation['operation_id']} "
                f"{operation['operation_name']} INCOMPLETE (power cut, crash, or drop)"))
        for operation in boot["unmatched_results"]:
            items.append((int(operation["result_sequence"]),
                f"  #{operation['result_sequence']} audit {operation['operation_id']} "
                f"{operation['operation_name']} result={operation['result']} "
                f"error={operation['error']} (begin missing or dropped)"))
        lines.extend(text for _, text in sorted(items))
    lines.extend(f"WARNING: {warning}" for warning in report["warnings"])
    return "\n".join(lines)


def _human_report(report: dict[str, object]) -> str:
    selected = next(item for item in report["superblocks"] if item["copy"] == report["selected_superblock"])
    lines = [
        f"source: {report['source']} ({report['source_kind']})",
        f"journal: generation {selected['generation']}, superblock {selected['copy']}, "
        f"{selected['completed_boots']} completed boot(s)",
        f"partition: LBA {selected['partition_start_lba']} + {selected['partition_blocks']} "
        f"at {selected['logical_block_bytes']}-byte blocks, GUID {selected['partition_unique_guid']}",
        f"image: {selected['image_id']}",
    ]
    boots = report["boots"]
    if not boots:
        lines.append("boots: none recorded")
    for boot in boots:
        valid_records = len(boot["records"])
        lines.append(
            f"boot {boot['boot_id']}: slot {boot['slot_index']} {boot['state']}, "
            f"records {valid_records}/{boot['record_count']}, dropped {boot['dropped_records']}, "
            f"sequence {boot['first_sequence']}..{boot['last_sequence']}"
        )
        for record in boot["records"]:
            payload = record.get("payload_text", "")
            suffix = f" payload={payload!r}" if payload else ""
            diagnostic = record.get("diagnostic")
            if diagnostic:
                kind = diagnostic["kind"]
                if kind == "storage-refused":
                    suffix += (
                        f" diagnostic={diagnostic['reason']}"
                        f" msc-result={diagnostic['msc_result']}"
                        f" stage={diagnostic['msc_stage']}({diagnostic['msc_stage_name']})"
                        f" port={diagnostic['port']} slot={diagnostic['slot']}"
                        f" xhci-cc={diagnostic['xhci_completion_code']}"
                    )
                elif kind == "xhci-state":
                    suffix += (
                        f" xhci-state portsc={diagnostic['portsc']}"
                        f" usbsts={diagnostic['usbsts']} usbcmd={diagnostic['usbcmd']}"
                    )
                elif kind == "frame":
                    suffix += (
                        f" frame-us={diagnostic['total_us']}"
                        f" phases={diagnostic['input_us']}/{diagnostic['tick_us']}/"
                        f"{diagnostic['compositor_us']}/{diagnostic['vblank_us']}/"
                        f"{diagnostic['present_us']}"
                        f" damage={diagnostic['damage_rects']}r/{diagnostic['damage_pixels']}px"
                    )
                elif kind == "pointer":
                    suffix += (
                        f" pointer={diagnostic['x']},{diagnostic['y']}"
                        f" buttons={diagnostic['buttons']}"
                        f" depth={diagnostic['queue_depth']}"
                        f" input-seq={diagnostic['input_sequence']}"
                    )
                elif kind == "display-state":
                    suffix += (
                        f" mode={diagnostic['width']}x{diagnostic['height']}x"
                        f"{diagnostic['bits_per_pixel']}"
                        f" pitch={diagnostic['pitch_bytes']}"
                        f" cache={diagnostic['cache_type_name']}"
                        f" back-buffer={int(diagnostic['back_buffer'])}"
                    )
                elif kind == "snapshot":
                    suffix += (
                        f" snapshot={diagnostic['operation_name']}/"
                        f"{diagnostic['phase_name']}"
                        f" raw={diagnostic['raw0']},{diagnostic['raw1']}"
                    )
                elif kind == "lifecycle":
                    suffix += (
                        f" lifecycle={diagnostic['object_kind_name']}#"
                        f"{diagnostic['object_id']}:{diagnostic['action_name']}"
                        f" parent={diagnostic['parent_id']} detail={diagnostic['detail']}"
                    )
                elif kind == "operation-begin":
                    suffix += (
                        f" audit={diagnostic['operation_id']}:{diagnostic['operation_name']}"
                        f" actor={diagnostic['actor_kind_name']}#{diagnostic['actor_id']}"
                        f" object={diagnostic['object_id']}"
                    )
                elif kind == "operation-result":
                    suffix += (
                        f" audit={diagnostic['operation_id']}:{diagnostic['operation_name']}"
                        f" result={diagnostic['result']} error={diagnostic['error']}"
                        f" detail={diagnostic['detail']}"
                    )
            lines.append(
                f"  #{record['sequence']} tsc={record['tsc']} cpu={record['cpu']} "
                f"sev={record['severity']} "
                f"sub={record['subsystem']}({record['subsystem_name']}) "
                f"event={record['event']}({record['event_name']}) "
                f"values={record['value0']},{record['value1']},{record['value2']}{suffix}"
            )
    for warning in report["warnings"]:
        lines.append(f"WARNING: {warning}")
    return "\n".join(lines)


def _record_rows(boots: Iterable[dict[str, object]]) -> Iterable[dict[str, object]]:
    for boot in boots:
        for record in boot["records"]:
            yield {
                "boot_id": boot["boot_id"],
                "slot": boot["slot_index"],
                "state": boot["state"],
                "sequence": record["sequence"],
                "tsc": record["tsc"],
                "cpu": record["cpu"],
                "severity": record["severity"],
                "subsystem": record["subsystem"],
                "subsystem_name": record["subsystem_name"],
                "event": record["event"],
                "event_name": record["event_name"],
                "value0": record["value0"],
                "value1": record["value1"],
                "value2": record["value2"],
                "diagnostic": json.dumps(record.get("diagnostic", {}), sort_keys=True),
                "payload_hex": record.get("payload_hex", ""),
                "payload_text": record.get("payload_text", ""),
            }


def export_report(report: dict[str, object], json_path: Path | None, csv_path: Path | None, text_path: Path | None) -> None:
    if not any((json_path, csv_path, text_path)):
        raise ZllogError("export requires at least one of --json, --csv, or --text")
    if json_path:
        json_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if csv_path:
        rows = list(_record_rows(report["boots"]))
        fieldnames = [
            "boot_id", "slot", "state", "sequence", "tsc", "cpu", "severity",
            "subsystem", "subsystem_name", "event", "event_name",
            "value0", "value1", "value2", "diagnostic", "payload_hex", "payload_text",
        ]
        with csv_path.open("w", encoding="utf-8", newline="") as stream:
            writer = csv.DictWriter(stream, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(rows)
    if text_path:
        text_path.write_text(_human_report(report) + "\n", encoding="utf-8")


def inspect_source(path: Path) -> dict[str, object]:
    source = locate_source(path)
    supers, selected = read_supers(source)
    partition = source.partition
    return {
        "source": str(path),
        "source_kind": "gpt-disk" if partition else "raw-partition",
        "partition": {
            "number": partition.number if partition else None,
            "byte_offset": source.base_offset,
            "byte_count": source.byte_count,
            "start_lba": partition.start_lba if partition else selected["partition_start_lba"],
            "blocks": partition.blocks if partition else selected["partition_blocks"],
            "logical_block_bytes": partition.block_bytes if partition else selected["logical_block_bytes"],
            "type_guid": _decode_guid(partition.type_guid_raw) if partition else selected["partition_type_guid"],
            "unique_guid": _decode_guid(partition.unique_guid_raw) if partition else selected["partition_unique_guid"],
            "label": partition.label if partition else PARTITION_LABEL,
        },
        "superblocks": supers,
        "selected_superblock": selected["copy"],
    }


def _image_identity(args: argparse.Namespace) -> bytes:
    if args.image_file:
        digest = hashlib.sha256()
        with args.image_file.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
        return digest.digest()
    try:
        identity = bytes.fromhex(args.image_id)
    except ValueError as error:
        raise ZllogError("--image-id must be 64 hexadecimal characters") from error
    if len(identity) != 32:
        raise ZllogError("--image-id must be 64 hexadecimal characters")
    return identity


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    init_parser = subparsers.add_parser("init", help="initialize the exact ZLLOG partition in a GPT disk image")
    init_parser.add_argument("source", type=Path)
    identity = init_parser.add_mutually_exclusive_group(required=True)
    identity.add_argument("--image-file", type=Path, help="file whose SHA-256 identifies the zlOS build")
    identity.add_argument("--image-id", help="64-character hexadecimal zlOS build identity")

    inspect_parser = subparsers.add_parser("inspect", help="validate GPT bounds and both journal superblocks")
    inspect_parser.add_argument("source", type=Path)
    inspect_parser.add_argument("--json", action="store_true", help="print JSON")

    for name in ("extract", "read", "audit"):
        extract_parser = subparsers.add_parser(name, help="read boot records (read is an extract alias)")
        extract_parser.add_argument("source", type=Path)
        group = extract_parser.add_mutually_exclusive_group()
        group.add_argument("--latest", action="store_true", help="read only the newest boot (default)")
        group.add_argument("--all", action="store_true", help="read every valid boot")
        extract_parser.add_argument("--json", action="store_true", help="print JSON instead of text")

    export_parser = subparsers.add_parser("export", help="write JSON, CSV, and/or text reports")
    export_parser.add_argument("source", type=Path)
    group = export_parser.add_mutually_exclusive_group()
    group.add_argument("--latest", action="store_true", help="export only the newest boot (default)")
    group.add_argument("--all", action="store_true", help="export every valid boot")
    export_parser.add_argument("--json", type=Path, dest="json_path")
    export_parser.add_argument("--csv", type=Path, dest="csv_path")
    export_parser.add_argument("--text", type=Path, dest="text_path")
    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        if args.command == "init":
            source = initialize(args.source, _image_identity(args))
            print(
                f"initialized {PARTITION_LABEL}: {source.byte_count} bytes at disk byte {source.base_offset}; "
                f"type {TYPE_GUID}"
            )
            return 0
        if args.command == "inspect":
            result = inspect_source(args.source)
            if args.json:
                print(json.dumps(result, indent=2))
            else:
                selected = next(
                    item for item in result["superblocks"] if item["copy"] == result["selected_superblock"]
                )
                partition = result["partition"]
                print(
                    f"{partition['label']}: {partition['byte_count']} bytes, LBA {partition['start_lba']} "
                    f"+ {partition['blocks']}, GUID {partition['unique_guid']}"
                )
                for superblock in result["superblocks"]:
                    status = "valid" if superblock["valid"] else f"invalid ({superblock.get('error')})"
                    print(f"super {superblock['copy']}: {status}, generation {superblock.get('generation', '?')}")
                print(f"selected superblock: {selected['copy']} generation {selected['generation']}")
            return 0
        latest = not args.all
        report = journal_report(locate_source(args.source), latest=latest)
        if args.command in ("extract", "read"):
            print(json.dumps(report, indent=2) if args.json else _human_report(report))
            return 0
        if args.command == "audit":
            audit = audit_report(report)
            print(json.dumps(audit, indent=2) if args.json else _human_audit(audit))
            return 0
        export_report(report, args.json_path, args.csv_path, args.text_path)
        return 0
    except (OSError, ZllogError) as error:
        print(f"zllog: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
