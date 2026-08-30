#!/usr/bin/env python3
"""Power-loss and write-containment test for the real kernel zllog writer."""

from __future__ import annotations

import hashlib
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


REPO = Path(__file__).resolve().parents[3]
TOOLS = REPO / "tools"
sys.path.insert(0, str(TOOLS))

import zllog  # noqa: E402


DISK_BYTES = 132 * 1024 * 1024
BLOCK_BYTES = 512
ESP_START_LBA = 2048
ESP_END_LBA = 129023
LOG_START_LBA = 131072
LOG_END_LBA = 262143


def region_hash(path: Path, start: int, count: int) -> str:
    digest = hashlib.sha256()
    with path.open("rb", buffering=0) as stream:
        stream.seek(start)
        remaining = count
        while remaining:
            chunk = stream.read(min(1024 * 1024, remaining))
            if not chunk:
                raise AssertionError(f"short read hashing {path} at {start}")
            digest.update(chunk)
            remaining -= len(chunk)
    return digest.hexdigest()


class KernelZllogEndToEnd(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        if shutil.which("sgdisk") is None:
            raise unittest.SkipTest("sgdisk is required")
        cls.class_tmp = tempfile.TemporaryDirectory(prefix="zllog-e2e-bin-")
        cls.writer = Path(cls.class_tmp.name) / "zllogtest"
        build = subprocess.run(
            [
                "gcc",
                "-O1",
                "-g",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-Wno-unused-function",
                "-fsanitize=address,undefined",
                "-fno-sanitize-recover=all",
                "-o",
                str(cls.writer),
                str(Path(__file__).with_name("zllogtest.c")),
            ],
            text=True,
            capture_output=True,
        )
        if build.returncode:
            raise AssertionError(f"zllog fake-device harness did not build:\n{build.stdout}{build.stderr}")

    @classmethod
    def tearDownClass(cls) -> None:
        cls.class_tmp.cleanup()

    def setUp(self) -> None:
        self.tmp = tempfile.TemporaryDirectory(prefix="zllog-e2e-")
        self.root = Path(self.tmp.name)
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
            text=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
        with self.image.open("r+b", buffering=0) as stream:
            stream.seek(ESP_START_LBA * BLOCK_BYTES)
            stream.write(b"EFI-SYSTEM-PARTITION-MUST-NOT-CHANGE\n" * 8192)
            stream.seek((LOG_END_LBA + 1) * BLOCK_BYTES)
            stream.write(b"DISK-TAIL-MUST-NOT-CHANGE\n" * 8192)
        self.image_id = hashlib.sha256(b"zllog end-to-end BOOTX64.EFI").digest()
        self.source = zllog.initialize(self.image, self.image_id)
        self.log_start = self.source.base_offset
        self.log_end = self.log_start + self.source.byte_count

    def tearDown(self) -> None:
        self.tmp.cleanup()

    def outside_hashes(self, image: Path) -> tuple[str, str]:
        return (
            region_hash(image, 0, self.log_start),
            region_hash(image, self.log_end, DISK_BYTES - self.log_end),
        )

    def run_writer(self, image: Path, tag: int, tear: str = "none") -> str:
        result = subprocess.run(
            [str(self.writer), str(image), str(tag), tear],
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("outside=0", result.stdout)
        if tear in ("final-super", "complete-slot"):
            self.assertIn("torn=1", result.stdout)
        elif tear == "none":
            self.assertIn("torn=0", result.stdout)
        return result.stdout.strip()

    def test_recorder_hot_path_overflow_privacy_and_reusable_irq_lane(self) -> None:
        result = subprocess.run(
            [str(self.writer), str(self.image), "0", "selftest"],
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("recorder_core_selftest=1", result.stdout)

    def test_transient_msc_failure_retries_and_drains_without_a_command(self) -> None:
        outside_before = self.outside_hashes(self.image)
        output = self.run_writer(self.image, 0x5151, "retry")
        self.assertIn("auto_retry=1", output)
        self.assertIn("buffered=0", output)
        self.assertIn("error=0", output)
        self.assertEqual(outside_before, self.outside_hashes(self.image))

        report = zllog.journal_report(zllog.locate_source(self.image), latest=True)
        self.assertEqual(len(report["boots"]), 1)
        records = report["boots"][0]["records"]
        self.assertTrue(any(int(record["event"]) == 4 for record in records))
        controller = next(record for record in records if int(record["event"]) == 5)
        self.assertEqual(controller["diagnostic"]["kind"], "xhci-state")
        self.assertEqual(controller["diagnostic"]["portsc"], "0x00000c03")

    def test_full_slot_seals_and_continues_without_stopping_telemetry(self) -> None:
        outside_before = self.outside_hashes(self.image)
        output = self.run_writer(self.image, 0x7171, "rotate")
        self.assertIn("rotated=1", output)
        self.assertIn("completed=1", output)
        self.assertEqual(outside_before, self.outside_hashes(self.image))

        report = self.assert_records_are_crc_valid_and_sequential(self.image)
        self.assertEqual(len(report["boots"]), 2)
        self.assertTrue(all(boot["state"] == "complete" for boot in report["boots"]))
        self.assertEqual(
            [int(boot["slot_index"]) for boot in report["boots"]], [0, 1]
        )
        self.assertEqual(len({int(boot["boot_id"]) for boot in report["boots"]}), 1)
        latest = zllog.journal_report(zllog.locate_source(self.image), latest=True)
        self.assertEqual(
            len(latest["boots"]), 2,
            "latest must return every continuation segment from the live session",
        )
        records = [record for boot in report["boots"] for record in boot["records"]]
        self.assertTrue(
            any(int(record["event"]) == zllog.EV_CHECKPOINT for record in records),
            "the next segment must identify why it exists",
        )

    def assert_records_are_crc_valid_and_sequential(self, image: Path) -> dict[str, object]:
        report = zllog.journal_report(zllog.locate_source(image), latest=False)
        self.assertEqual(report["warnings"], [])
        records = [record for boot in report["boots"] for record in boot["records"]]
        self.assertGreater(len(records), 0)
        sequences = [int(record["sequence"]) for record in records]
        self.assertEqual(sequences, list(range(sequences[0], sequences[-1] + 1)))
        for record in records:
            self.assertTrue(record["valid"])
            self.assertRegex(str(record["stored_crc32"]), r"^[0-9a-f]{8}$")
        for boot in report["boots"]:
            boot_sequences = [int(record["sequence"]) for record in boot["records"]]
            self.assertEqual(int(boot["first_sequence"]), boot_sequences[0])
            self.assertEqual(int(boot["last_sequence"]), boot_sequences[-1])
            self.assertEqual(int(boot["record_count"]), len(boot_sequences))
        return report

    def test_real_writer_stays_in_partition_and_recovers_from_torn_metadata(self) -> None:
        outside_before = self.outside_hashes(self.image)
        normal_output = self.run_writer(self.image, 0x1111)
        self.assertIn("completed=1", normal_output)
        self.assertEqual(outside_before, self.outside_hashes(self.image))

        first_report = self.assert_records_are_crc_valid_and_sequential(self.image)
        self.assertEqual(len(first_report["boots"]), 1)
        first_boot = first_report["boots"][0]
        self.assertEqual(first_boot["state"], "complete")
        frame = next(record for record in first_boot["records"] if int(record["event"]) == 20)
        self.assertEqual(frame["diagnostic"]["damage_rects"], 2)
        self.assertEqual(frame["diagnostic"]["damage_pixels"], 33000)
        self.assertEqual(frame["diagnostic"]["total_us"], 150)
        frame_ext = next(
            record for record in first_boot["records"]
            if int(record["event"]) == zllog.EV_FRAME_EXT
        )
        self.assertEqual(frame_ext["diagnostic"]["input_to_present_us"], 90)
        self.assertEqual(frame_ext["diagnostic"]["input_sequence"], 44)
        self.assertEqual(frame_ext["diagnostic"]["present_bytes"], 132000)
        paint = next(
            record for record in first_boot["records"]
            if int(record["event"]) == zllog.EV_FRAME_PAINT
        )
        self.assertEqual(paint["diagnostic"], {
            "kind": "frame-paint-phases",
            "frame_sequence": 0,
            "repaint_rects": 2,
            "desk_us": 4,
            "chrome_us": 5,
            "app_us": 6,
            "effects_us": 7,
            "repaint_pixels": 32000,
            "window_visits": 3,
            "app_calls": 2,
        })
        counters = {
            record["diagnostic"]["counter_name"]: record["diagnostic"]
            for record in first_boot["records"]
            if int(record["event"]) == zllog.EV_COUNTER
        }
        self.assertEqual(counters["irq-timer"]["total"], 3)
        self.assertEqual(counters["mmio-poll"]["maximum"], 20)
        audit = zllog.audit_report(first_report)["boots"][0]
        self.assertEqual(
            [item["action_name"] for item in audit["lifecycle"]
             if item["object_kind_name"] == "process"],
            ["start", "exit"],
        )
        self.assertEqual(len(audit["transactions"]), 1)
        self.assertEqual(audit["transactions"][0]["operation_name"], "syscall-2")
        self.assertEqual(audit["transactions"][0]["result"], 0x1111)
        self.assertEqual(audit["incomplete_operations"], [])
        self.assertEqual(audit["unmatched_results"], [])
        first_boot_id = int(first_boot["boot_id"])

        # Power dies halfway through publishing the newest superblock. The
        # other copy must remain selected and the older completed boot remains.
        torn_super = self.root / "torn-super.img"
        shutil.copyfile(self.image, torn_super)
        super_outside_before = self.outside_hashes(torn_super)
        super_output = self.run_writer(torn_super, 0x2222, "final-super")
        self.assertIn("completed=0", super_output)
        self.assertEqual(super_outside_before, self.outside_hashes(torn_super))
        super_source = zllog.locate_source(torn_super)
        supers, selected = zllog.read_supers(super_source)
        self.assertEqual(sum(bool(item["valid"]) for item in supers), 1)
        self.assertLess(int(selected["generation"]), max(int(item["generation"]) for item in supers))
        super_report = zllog.journal_report(super_source, latest=False)
        self.assertIn(first_boot_id, [int(boot["boot_id"]) for boot in super_report["boots"]])
        super_sequences = [
            int(record["sequence"])
            for boot in super_report["boots"]
            for record in boot["records"]
        ]
        self.assertEqual(
            super_sequences,
            list(range(super_sequences[0], super_sequences[-1] + 1)),
            "sequence numbers must remain global and contiguous across an early-RAM second boot",
        )

        # Mixed recovery/clean rotation must not reuse the just-completed slot:
        # boot 2 ended with an active super but a completed slot; boots 3 and 4
        # must advance to slots 2 and 3, preserving slots 0 and 1.
        rotation_outside_before = self.outside_hashes(torn_super)
        self.run_writer(torn_super, 0x4444)
        self.run_writer(torn_super, 0x5555)
        self.assertEqual(rotation_outside_before, self.outside_hashes(torn_super))
        rotation_report = self.assert_records_are_crc_valid_and_sequential(torn_super)
        self.assertEqual(
            [int(boot["slot_index"]) for boot in rotation_report["boots"]],
            [0, 1, 2, 3],
            "an incomplete/clean mix must rotate forward instead of overwriting the newest completed slot",
        )

        # Power dies halfway through the COMPLETE slot-header checkpoint. The
        # new slot is rejected by CRC, while the prior committed slot survives.
        torn_slot = self.root / "torn-slot.img"
        shutil.copyfile(self.image, torn_slot)
        slot_outside_before = self.outside_hashes(torn_slot)
        slot_output = self.run_writer(torn_slot, 0x3333, "complete-slot")
        self.assertIn("completed=0", slot_output)
        self.assertEqual(slot_outside_before, self.outside_hashes(torn_slot))
        slot_report = zllog.journal_report(zllog.locate_source(torn_slot), latest=False)
        self.assertIn(first_boot_id, [int(boot["boot_id"]) for boot in slot_report["boots"]])
        recovered = [boot for boot in slot_report["boots"] if boot.get("recovered")]
        self.assertEqual(len(recovered), 1)
        recovered_sequences = [int(record["sequence"]) for record in recovered[0]["records"]]
        self.assertGreater(recovered_sequences[0], int(first_boot["last_sequence"]))
        self.assertEqual(
            recovered_sequences,
            list(range(recovered_sequences[0], recovered_sequences[-1] + 1)),
        )
        self.assertTrue(any("CRC mismatch" in warning for warning in slot_report["warnings"]))

    def assert_mount_refused_without_write(self, image: Path, expected_error: int) -> None:
        outside_before = self.outside_hashes(image)
        result = subprocess.run(
            [str(self.writer), str(image), "0x9999", "refuse"],
            text=True,
            capture_output=True,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("writes=0", result.stdout)
        self.assertIn("outside=0", result.stdout)
        self.assertIn("refused=1", result.stdout)
        self.assertIn(f"error={expected_error}", result.stdout)
        self.assertEqual(outside_before, self.outside_hashes(image))

    def test_kernel_refuses_mismatched_or_corrupt_identity_without_any_write(self) -> None:
        wrong_label = self.root / "wrong-label.img"
        shutil.copyfile(self.image, wrong_label)
        subprocess.run(
            ["sgdisk", "--change-name=2:NOT-ZLLOG", str(wrong_label)],
            check=True,
            text=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
        self.assert_mount_refused_without_write(wrong_label, expected_error=4)

        wrong_guid = self.root / "wrong-unique-guid.img"
        shutil.copyfile(self.image, wrong_guid)
        subprocess.run(
            [
                "sgdisk",
                "--partition-guid=2:11111111-2222-4333-8444-555555555555",
                str(wrong_guid),
            ],
            check=True,
            text=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
        self.assert_mount_refused_without_write(wrong_guid, expected_error=5)

        bad_gpt_crc = self.root / "bad-gpt-crc.img"
        shutil.copyfile(self.image, bad_gpt_crc)
        with bad_gpt_crc.open("r+b", buffering=0) as stream:
            stream.seek(BLOCK_BYTES + 16)
            stored_crc = bytearray(stream.read(4))
            stored_crc[0] ^= 0x80
            stream.seek(BLOCK_BYTES + 16)
            stream.write(stored_crc)
        self.assert_mount_refused_without_write(bad_gpt_crc, expected_error=4)

        bad_supers = self.root / "bad-super-magic.img"
        shutil.copyfile(self.image, bad_supers)
        with bad_supers.open("r+b", buffering=0) as stream:
            stream.seek(self.log_start)
            stream.write(b"BADSUPER")
            stream.seek(self.log_start + zllog.SUPERBLOCK_BYTES)
            stream.write(b"BADSUPER")
        self.assert_mount_refused_without_write(bad_supers, expected_error=5)


if __name__ == "__main__":
    unittest.main(verbosity=2)
