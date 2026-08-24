#!/usr/bin/env python3
"""Host-only format and failure-recovery tests for tools/zllog.py."""

from __future__ import annotations

import hashlib
import json
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path

import zllog


DISK_BYTES = 132 * 1024 * 1024
ESP_START_LBA = 2048
ESP_END_LBA = 129023
LOG_START_LBA = 131072
LOG_END_LBA = 262143
BLOCK_BYTES = 512


def region_hash(path: Path, offset: int, count: int) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        stream.seek(offset)
        remaining = count
        while remaining:
            chunk = stream.read(min(1024 * 1024, remaining))
            if not chunk:
                raise AssertionError("short test image")
            digest.update(chunk)
            remaining -= len(chunk)
    return digest.hexdigest()


class ZllogTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        if shutil.which("sgdisk") is None:
            raise unittest.SkipTest("sgdisk is required")

    def setUp(self) -> None:
        self.tempdir = tempfile.TemporaryDirectory(prefix="zllog-test-")
        self.root = Path(self.tempdir.name)
        self.image = self.root / "disk.img"
        with self.image.open("wb") as stream:
            stream.truncate(DISK_BYTES)
        subprocess.run(
            [
                "sgdisk",
                "--clear",
                f"--new=1:{ESP_START_LBA}:{ESP_END_LBA}",
                "--typecode=1:ef00",
                "--change-name=1:zlOS EFI",
                f"--new=2:{LOG_START_LBA}:{LOG_END_LBA}",
                f"--typecode=2:{zllog.TYPE_GUID}",
                f"--change-name=2:{zllog.PARTITION_LABEL}",
                str(self.image),
            ],
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            text=True,
        )
        with self.image.open("r+b") as stream:
            stream.seek(ESP_START_LBA * BLOCK_BYTES)
            stream.write(b"ESP-MUST-NOT-CHANGE" * 4096)
            stream.seek((LOG_END_LBA + 1) * BLOCK_BYTES)
            stream.write(b"TAIL-MUST-NOT-CHANGE" * 4096)
        self.image_id = hashlib.sha256(b"test BOOTX64.EFI").digest()

    def tearDown(self) -> None:
        self.tempdir.cleanup()

    def init(self) -> zllog.Source:
        return zllog.initialize(self.image, self.image_id)

    def test_init_changes_only_exact_zllog_partition(self) -> None:
        log_offset = LOG_START_LBA * BLOCK_BYTES
        log_end = (LOG_END_LBA + 1) * BLOCK_BYTES
        prefix_before = region_hash(self.image, 0, log_offset)
        suffix_before = region_hash(self.image, log_end, DISK_BYTES - log_end)

        source = self.init()

        self.assertEqual(source.base_offset, log_offset)
        self.assertEqual(source.byte_count, zllog.PARTITION_BYTES)
        self.assertEqual(prefix_before, region_hash(self.image, 0, log_offset))
        self.assertEqual(suffix_before, region_hash(self.image, log_end, DISK_BYTES - log_end))
        supers, selected = zllog.read_supers(source)
        self.assertEqual([item["valid"] for item in supers], [True, True])
        self.assertEqual([item["generation"] for item in supers], [1, 0])
        self.assertEqual(selected["copy"], "A")
        self.assertEqual(selected["slot_count"], 31)
        self.assertEqual(selected["partition_start_lba"], LOG_START_LBA)
        self.assertEqual(selected["partition_blocks"], 131072)
        self.assertEqual(selected["image_id"], self.image_id.hex())

    def test_init_accepts_a_larger_bounded_history_partition(self) -> None:
        image = self.root / "larger.img"
        log_blocks = 128 * 1024 * 1024 // BLOCK_BYTES
        log_end_lba = LOG_START_LBA + log_blocks - 1
        disk_bytes = 196 * 1024 * 1024
        with image.open("wb") as stream:
            stream.truncate(disk_bytes)
        subprocess.run(
            [
                "sgdisk", "--clear",
                f"--new=1:{ESP_START_LBA}:{ESP_END_LBA}", "--typecode=1:ef00",
                "--change-name=1:zlOS EFI",
                f"--new=2:{LOG_START_LBA}:{log_end_lba}",
                f"--typecode=2:{zllog.TYPE_GUID}",
                f"--change-name=2:{zllog.PARTITION_LABEL}", str(image),
            ],
            check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True,
        )
        log_offset = LOG_START_LBA * BLOCK_BYTES
        log_end = (log_end_lba + 1) * BLOCK_BYTES
        prefix_before = region_hash(image, 0, log_offset)
        suffix_before = region_hash(image, log_end, disk_bytes - log_end)
        source = zllog.initialize(image, self.image_id)
        self.assertEqual(source.byte_count, 128 * 1024 * 1024)
        _, selected = zllog.read_supers(source)
        self.assertEqual(selected["slot_count"], 63)
        self.assertEqual(prefix_before, region_hash(image, 0, log_offset))
        self.assertEqual(suffix_before, region_hash(image, log_end, disk_bytes - log_end))

    def test_torn_newest_superblock_falls_back(self) -> None:
        source = self.init()
        with self.image.open("r+b", buffering=0) as stream:
            stream.seek(source.base_offset + 300)
            stream.write(b"torn")
        supers, selected = zllog.read_supers(source)
        self.assertFalse(supers[0]["valid"])
        self.assertEqual(supers[0]["error"], "CRC mismatch")
        self.assertTrue(supers[1]["valid"])
        self.assertEqual(selected["copy"], "B")
        self.assertEqual(selected["generation"], 0)

    def test_torn_replacement_does_not_hide_older_generation(self) -> None:
        source = self.init()
        replacement = bytearray(zllog.encode_super(source, 2, self.image_id))
        replacement[zllog.SUPER_CRC_OFFSET] ^= 0x80
        with self.image.open("r+b", buffering=0) as stream:
            stream.seek(source.base_offset + zllog.SUPER_B_OFFSET)
            stream.write(replacement)
        supers, selected = zllog.read_supers(source)
        self.assertTrue(supers[0]["valid"])
        self.assertFalse(supers[1]["valid"])
        self.assertEqual(selected["copy"], "A")
        self.assertEqual(selected["generation"], 1)

    def test_extract_valid_records_and_skip_corrupt_record(self) -> None:
        source = self.init()
        record0 = zllog.encode_record(
            sequence=1,
            tsc=100,
            cpu=0,
            severity=1,
            subsystem=2,
            event=3,
            value0=4,
            value1=5,
            value2=6,
            payload=b"boot",
        )
        record1 = bytearray(
            zllog.encode_record(sequence=2, tsc=200, cpu=1, severity=2, subsystem=4, event=8, payload=b"input")
        )
        record1[24] ^= 1
        header = zllog.encode_slot_header(
            slot_index=0,
            state_value=zllog.SLOT_WRITING,
            boot_id=7,
            first_sequence=1,
            last_sequence=2,
            record_count=2,
            dropped_records=0,
            boot_path=3,
            flags=0,
            started_tsc=90,
            ended_tsc=0,
            build_hash=self.image_id,
        )
        slot_offset = source.base_offset + zllog.SLOTS_OFFSET
        with self.image.open("r+b", buffering=0) as stream:
            stream.seek(slot_offset)
            stream.write(header)
            stream.write(record0)
            stream.write(record1)

        report = zllog.journal_report(source, latest=True)
        self.assertEqual(len(report["boots"]), 1)
        self.assertEqual(report["boots"][0]["state"], "writing")
        self.assertEqual(report["boots"][0]["records"][0]["payload_text"], "boot")
        self.assertEqual(len(report["boots"][0]["records"]), 1)
        self.assertIn("slot 0 record 1: CRC mismatch", report["warnings"])

        json_path = self.root / "boots.json"
        csv_path = self.root / "records.csv"
        text_path = self.root / "boots.txt"
        zllog.export_report(report, json_path, csv_path, text_path)
        exported = json.loads(json_path.read_text())
        self.assertEqual(exported["boots"][0]["boot_id"], 7)
        self.assertIn("boot_id,slot,state,sequence", csv_path.read_text())
        self.assertIn("boot 7: slot 0 writing", text_path.read_text())

    def test_storage_refusal_decodes_msc_failure_boundary(self) -> None:
        detail = 8 | (4 << 8) | (2 << 16) | (17 << 24)
        record = zllog.decode_record(
            zllog.encode_record(
                sequence=1,
                tsc=100,
                cpu=0,
                severity=1,
                subsystem=zllog.SUB_STORAGE,
                event=zllog.EV_STORAGE_REFUSED,
                value0=1,
                value1=0xFFFFFFFF,
                value2=detail,
            ),
            0,
        )
        self.assertTrue(record["valid"])
        self.assertEqual(
            record["diagnostic"],
            {
                "kind": "storage-refused",
                "reason": "msc-init",
                "msc_result": 0xFFFFFFFF,
                "msc_stage": 8,
                "msc_stage_name": "endpoints",
                "port": 4,
                "slot": 2,
                "xhci_completion_code": 17,
            },
        )

        controller = zllog.decode_record(
            zllog.encode_record(
                sequence=2,
                tsc=101,
                cpu=0,
                severity=1,
                subsystem=zllog.SUB_STORAGE,
                event=zllog.EV_STORAGE_XHCI,
                value0=0x00000C03,
                value1=0x00000000,
                value2=0x00000005,
            ),
            1,
        )
        self.assertEqual(
            controller["diagnostic"],
            {
                "kind": "xhci-state",
                "portsc": "0x00000c03",
                "usbsts": "0x00000000",
                "usbcmd": "0x00000005",
            },
        )

    def test_frame_record_decodes_timings_flags_and_damage(self) -> None:
        flags = 1 | 2 | 4
        meta = flags | (3 << 3) | (123456 << 7)
        frame = zllog.decode_record(
            zllog.encode_record(
                sequence=3,
                tsc=200,
                cpu=0,
                severity=1,
                subsystem=zllog.SUB_FRAME,
                event=zllog.EV_FRAME_LATE,
                value0=11,
                value1=22,
                value2=33 | (meta << 32),
                payload=struct.pack("<III", 44, 55, 16500),
            ),
            2,
        )
        diagnostic = frame["diagnostic"]
        self.assertEqual(diagnostic["compositor_us"], 33)
        self.assertEqual(diagnostic["damage_rects"], 3)
        self.assertEqual(diagnostic["damage_pixels"], 123456)
        self.assertTrue(diagnostic["late"])
        self.assertTrue(diagnostic["periodic_sample"])
        self.assertTrue(diagnostic["waited_vblank"])

    def test_pointer_record_decodes_pixels_queue_and_hardware_stamp(self) -> None:
        pointer = zllog.decode_record(
            zllog.encode_record(
                sequence=4, tsc=250, cpu=0, severity=1,
                subsystem=zllog.SUB_INPUT, event=zllog.EV_POINTER,
                value0=123 | (456 << 32),
                value1=1 | (2 << 32),
                value2=0x89ABCDEF | (77 << 32),
            ), 3,
        )
        self.assertEqual(pointer["event_name"], "pointer")
        self.assertEqual(pointer["diagnostic"], {
            "kind": "pointer",
            "x": 123,
            "y": 456,
            "buttons": 1,
            "queue_depth": 2,
            "source_tsc_low": 0x89ABCDEF,
            "input_sequence": 77,
        })

    def test_display_state_decodes_cache_mode_and_geometry(self) -> None:
        meta = 32 | (1 << 8) | (2 << 9) | (7680 << 16)
        display = zllog.decode_record(
            zllog.encode_record(
                sequence=5, tsc=260, cpu=0, severity=1,
                subsystem=zllog.SUB_DISPLAY, event=zllog.EV_DISPLAY_STATE,
                value0=1920, value1=1200, value2=meta,
            ), 4,
        )
        self.assertEqual(display["diagnostic"], {
            "kind": "display-state",
            "width": 1920,
            "height": 1200,
            "bits_per_pixel": 32,
            "back_buffer": True,
            "cache_type": 2,
            "cache_type_name": "write-combining",
            "pitch_bytes": 7680,
        })

    def test_timeout_snapshot_decodes_operation_phase_and_raw_words(self) -> None:
        snapshot = zllog.decode_record(
            zllog.encode_record(
                sequence=6, tsc=270, cpu=0, severity=0,
                subsystem=zllog.SUB_STORAGE, event=zllog.EV_SNAPSHOT,
                value0=(7 << 8) | 1,
                value1=0x11223344, value2=0xAABBCCDD,
            ), 5,
        )
        self.assertEqual(snapshot["event_name"], "snapshot")
        self.assertEqual(snapshot["diagnostic"], {
            "kind": "snapshot",
            "operation": 7,
            "operation_name": "net-wait",
            "phase": 1,
            "phase_name": "trigger",
            "raw0": "0x0000000011223344",
            "raw1": "0x00000000aabbccdd",
        })

    def test_telemetry_counter_and_frame_extensions_decode(self) -> None:
        counter = zllog.decode_record(
            zllog.encode_record(
                sequence=4, tsc=300, cpu=0, severity=0,
                subsystem=16, event=zllog.EV_COUNTER,
                value0=8, value1=123456, value2=9000,
                payload=struct.pack("<III", 17, 456, 2),
            ), 3,
        )
        self.assertEqual(counter["subsystem_name"], "perf")
        self.assertEqual(counter["event_name"], "counter")
        self.assertEqual(counter["diagnostic"]["counter_name"], "mmio-poll")
        self.assertEqual(counter["diagnostic"]["total_delta"], 456)

        ext = zllog.decode_record(
            zllog.encode_record(
                sequence=5, tsc=301, cpu=0, severity=1,
                subsystem=15, event=zllog.EV_FRAME_EXT,
                value0=77, value1=8321,
                value2=1920000 | (7680000 << 32),
                payload=struct.pack("<III", 3, 5, 7680000 | 0x80000000),
            ), 4,
        )
        self.assertEqual(ext["diagnostic"]["input_to_present_us"], 8321)
        self.assertEqual(ext["diagnostic"]["missed_deadlines"], 3)
        self.assertEqual(ext["diagnostic"]["present_bytes"], 7680000)
        self.assertTrue(ext["diagnostic"]["cursor_only"])

        paint = zllog.decode_record(
            zllog.encode_record(
                sequence=6, tsc=302, cpu=0, severity=1,
                subsystem=15, event=zllog.EV_FRAME_PAINT,
                value0=77 | (2 << 32),
                value1=110 | (220 << 32),
                value2=330 | (44 << 32),
                payload=struct.pack("<III", 1920000, 5, 3),
            ), 5,
        )
        self.assertEqual(paint["diagnostic"], {
            "kind": "frame-paint-phases",
            "frame_sequence": 77,
            "repaint_rects": 2,
            "desk_us": 110,
            "chrome_us": 220,
            "app_us": 330,
            "effects_us": 44,
            "repaint_pixels": 1920000,
            "window_visits": 5,
            "app_calls": 3,
        })

    def test_torn_active_slot_header_recovers_durable_record_prefix(self) -> None:
        source = self.init()
        records = [
            zllog.encode_record(sequence=10, tsc=100, cpu=0, severity=0,
                                subsystem=1, event=1, payload=b"early"),
            zllog.encode_record(sequence=11, tsc=200, cpu=0, severity=1,
                                subsystem=2, event=10, payload=b"input"),
        ]
        header = bytearray(
            zllog.encode_slot_header(
                slot_index=0,
                state_value=zllog.SLOT_WRITING,
                boot_id=9,
                first_sequence=10,
                last_sequence=11,
                record_count=2,
                dropped_records=0,
                boot_path=3,
                flags=0,
                started_tsc=90,
                ended_tsc=0,
                build_hash=self.image_id,
            )
        )
        header[300] ^= 0x80                 # torn mutable checkpoint header

        selected_super = bytearray(zllog.encode_super(source, 2, self.image_id))
        struct.pack_into("<I", selected_super, 24, 0)  # active_slot = 0
        struct.pack_into("<I", selected_super, zllog.SUPER_CRC_OFFSET, 0)
        struct.pack_into(
            "<I", selected_super, zllog.SUPER_CRC_OFFSET,
            zllog.crc32(bytes(selected_super)),
        )
        slot_offset = source.base_offset + zllog.SLOTS_OFFSET
        with self.image.open("r+b", buffering=0) as stream:
            stream.seek(source.base_offset + zllog.SUPER_B_OFFSET)
            stream.write(selected_super)
            stream.seek(slot_offset)
            stream.write(header)
            for record in records:
                stream.write(record)

        report = zllog.journal_report(source, latest=True)
        self.assertEqual(report["selected_superblock"], "B")
        self.assertEqual(len(report["boots"]), 1)
        boot = report["boots"][0]
        self.assertTrue(boot["recovered"])
        self.assertEqual(boot["state"], "recovered-writing")
        self.assertEqual([record["sequence"] for record in boot["records"]], [10, 11])
        self.assertIn("slot 0: CRC mismatch", report["warnings"])

    def test_cli_inspect_and_raw_partition_extract(self) -> None:
        source = self.init()
        command = [str(Path(zllog.__file__)), "inspect", str(self.image), "--json"]
        result = subprocess.run(command, check=True, text=True, capture_output=True)
        inspected = json.loads(result.stdout)
        self.assertEqual(inspected["partition"]["label"], zllog.PARTITION_LABEL)
        self.assertEqual(inspected["selected_superblock"], "A")

        # Exercise the public CLI wiring, not only the Python API. A Path must
        # be resolved to a Source before extract/export reaches journal_report.
        read_result = subprocess.run(
            [str(Path(zllog.__file__)), "read", str(self.image), "--json"],
            check=True, text=True, capture_output=True,
        )
        self.assertEqual(json.loads(read_result.stdout)["source_kind"], "gpt-disk")
        exported_json = self.root / "cli-export.json"
        subprocess.run(
            [str(Path(zllog.__file__)), "export", str(self.image), "--json", str(exported_json)],
            check=True, text=True, capture_output=True,
        )
        self.assertEqual(json.loads(exported_json.read_text())["source_kind"], "gpt-disk")

        raw = self.root / "zllog.raw"
        with self.image.open("rb") as source_stream, raw.open("wb") as raw_stream:
            source_stream.seek(source.base_offset)
            remaining = source.byte_count
            while remaining:
                chunk = source_stream.read(min(1024 * 1024, remaining))
                raw_stream.write(chunk)
                remaining -= len(chunk)
        raw_report = zllog.journal_report(zllog.locate_source(raw), latest=True)
        self.assertEqual(raw_report["source_kind"], "raw-partition")
        self.assertEqual(raw_report["selected_superblock"], "A")

        # A partition device has no GPT header to fall back to. Damage to A's
        # magic must still be recognized as a raw journal via valid copy B.
        with raw.open("r+b", buffering=0) as stream:
            stream.seek(0)
            stream.write(b"TORN")
        raw_report = zllog.journal_report(zllog.locate_source(raw), latest=True)
        self.assertEqual(raw_report["selected_superblock"], "B")

    def test_refuses_wrong_label(self) -> None:
        subprocess.run(
            ["sgdisk", "--change-name=2:NOT-ZLLOG", str(self.image)],
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            text=True,
        )
        with self.assertRaisesRegex(zllog.ZllogError, "found 0"):
            zllog.initialize(self.image, self.image_id)


if __name__ == "__main__":
    unittest.main(verbosity=2)
