#!/usr/bin/env python3

from __future__ import annotations

import struct
import unittest

import zlbootdiag


class BootDiagTests(unittest.TestCase):
    def make_v1_payload(self) -> bytes:
        data = bytearray(64)
        data[:8] = zlbootdiag.MAGIC_V1
        struct.pack_into("<HH", data, 8, 1, 64)
        struct.pack_into(
            "<12I", data, 12,
            2, 1, 3, 8, 4, 2, 17,
            0x00000C03, 0, 5, (0x0718 << 16) | 0x067D, 1234,
        )
        struct.pack_into("<I", data, 60, zlbootdiag.fnv32(data[:60]))
        return bytes(data)

    def make_v2_payload(self) -> bytes:
        entries = (
            struct.pack(
                "<8BHHI", 4, 4, 2, 0, 6, 1, 3, 3 | (2 << 3),
                0x0718, 0x067D, 0x00000C03,
            )
            + struct.pack(
                "<8BHHI", 10, 5, 4, 0xFF, 6, 1, 1, 3,
                0x8087, 0x0026, 0x00000603,
            )
        )
        total = 64 + len(entries) + 4
        data = bytearray(total)
        data[:8] = zlbootdiag.MAGIC_V2
        struct.pack_into("<HHI", data, 8, 2, 64, total)
        struct.pack_into(
            "<11I", data, 16,
            2, 1, 2, 5, 10, 4, 0xFFFFFFFF,
            0x18, 1, 189, (0x8087 << 16) | 0x0026,
        )
        struct.pack_into("<HH", data, 60, 2, 16)
        data[64:-4] = entries
        struct.pack_into("<I", data, total - 4, zlbootdiag.fnv32(data[:-4]))
        return bytes(data)

    def make_v3_payload(self) -> bytes:
        entry = struct.pack(
            "<8BHHI", 4, 9, 1, 1, 6, 1, 3 | (2 << 4),
            7 | (3 << 3) | (3 << 5),
            0x0718, 0x067D, 0x00000E03,
        )
        total = 96 + len(entry) + 4
        data = bytearray(total)
        data[:8] = zlbootdiag.MAGIC_V3
        struct.pack_into("<HHI", data, 8, 3, 96, total)
        struct.pack_into(
            "<11I", data, 16,
            2, 2, 8, 9, 4, 1, 1, 0x18, 1, 250,
            (0x0718 << 16) | 0x067D,
        )
        struct.pack_into("<HH", data, 60, 1, 16)
        struct.pack_into("<6I", data, 64, 4, 1, 512, 1, 0x25, 1)
        struct.pack_into("<3B", data, 88, 0x02, 0x3A, 0)
        data[96:-4] = entry
        struct.pack_into("<I", data, total - 4, zlbootdiag.fnv32(data[:-4]))
        return bytes(data)

    def make_v4_payload(self) -> bytes:
        prefix = struct.pack(
            "<8BHHI", 4, 4, 1, 4, 6, 1, 3 | (2 << 4),
            3 | (1 << 3) | (3 << 5),
            0x0718, 0x067D, 0x00000E03,
        )
        meta = (
            6 | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) |
            (1 << 12) | (64 << 16) | (3 << 24)
        )
        probes = 1 | (1 << 8) | (4 << 16) | (4 << 24)
        trace = (
            (0x02000680, 0x00090000, 8, 0x00030841)
            + (0x0E020100, 0, 9, 0x00010C01)
            + (0, 0, 0, 0x00001021)
            + (0x0E011030, 0, 0x04000000, 0x01018001)
            + (2, 0x0E011031, 0, meta, probes, 0)
        )
        entry = prefix + struct.pack("<22I", *trace)
        total = 96 + len(entry) + 4
        data = bytearray(total)
        data[:8] = zlbootdiag.MAGIC_V4
        struct.pack_into("<HHI", data, 8, 4, 96, total)
        struct.pack_into(
            "<11I", data, 16,
            2, 1, 2, 4, 4, 1, 4, 0x18, 1, 300,
            (0x0718 << 16) | 0x067D,
        )
        struct.pack_into("<HH", data, 60, 1, 104)
        struct.pack_into("<6I", data, 64, 0xFFFFFFFF, 0xFFFFFFFF, 0,
                         0xFFFFFFFF, 0, 0)
        struct.pack_into("<I", data, 92, 0x0110)
        data[96:-4] = entry
        struct.pack_into("<I", data, total - 4, zlbootdiag.fnv32(data[:-4]))
        return bytes(data)

    def make_v5_payload(self) -> bytes:
        v4_entry = self.make_v4_payload()[96:-4]
        total = 312 + len(v4_entry) + 4
        data = bytearray(total)
        data[:8] = zlbootdiag.MAGIC_V5
        struct.pack_into("<HHI", data, 8, 5, 312, total)
        struct.pack_into(
            "<11I", data, 16,
            2, 1, 2, 4, 4, 1, 4, 0x18, 1, 400,
            (0x0718 << 16) | 0x067D,
        )
        struct.pack_into("<HH", data, 60, 1, 104)
        struct.pack_into("<6I", data, 64, 0xFFFFFFFF, 0xFFFFFFFF, 0,
                         0xFFFFFFFF, 0, 0)
        struct.pack_into("<I", data, 92, 0x0110)

        first_trace_meta = (
            3 | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) |
            (1 << 12) | (32 << 16) | (1 << 24)
        )
        first_probe_meta = 0x80000000 | 4 | (1 << 8) | (1 << 16)
        first = (
            (0x01000680, 0x00120000, 8, 0x00030841)
            + (0x0E020100, 0, 18, 0x00010C01)
            + (0, 0, 0, 0x00001021)
            + (0x0E011020, 0, 0x01000000, 0x01018001)
            + (1, 0x0E011031, 0, first_trace_meta, first_probe_meta, 0x02000112)
        )
        second_trace_meta = first_trace_meta | 6
        second_probe_meta = (
            0x80000000 | 4 | (1 << 8) | (4 << 16) | (2 << 24)
        )
        second = (
            (0x01000680, 0x00120000, 8, 0x00030841)
            + (0x0E020100, 0, 18, 0x00010C01)
            + (0, 0, 0, 0x00001021)
            + (0x0E011030, 0, 0x04000008, 0x01018001)
            + (2, 0x0E011031, 0, second_trace_meta, second_probe_meta, 0x02000112)
        )
        struct.pack_into("<22I", data, 96, *first)
        struct.pack_into("<22I", data, 184, *second)
        struct.pack_into(
            "<8I", data, 272,
            0x00010001, 0x01000000, 0x00002000, 0xE0000000,
            3, 0x846C // 4, 1, 1,
        )
        struct.pack_into("<2I", data, 304, 0x10000005, 0x10000005)
        data[312:-4] = v4_entry
        struct.pack_into("<I", data, total - 4, zlbootdiag.fnv32(data[:-4]))
        return bytes(data)

    def make_v6_payload(self) -> bytes:
        v5 = self.make_v5_payload()
        entry = v5[312:-4]
        total = 440 + len(entry) + 4
        data = bytearray(total)
        data[:312] = v5[:312]
        data[:8] = zlbootdiag.MAGIC_V6
        struct.pack_into("<HHI", data, 8, 6, 440, total)

        meta = (
            9 | (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) |
            (1 << 12) | (32 << 16) | (1 << 24)
        )
        trace = (
            (0x02000680, 0x00090000, 8, 0x00030841)
            + (0x0E020100, 0, 9, 0x00010C05)
            + (0, 0, 0, 0x00001021)
            + (0x0E011060, 0, 0x04000008, 0x01018001)
            + (2, 0x0E011061, 0, meta, 0, 0)
        )
        config_meta = 0x80000000 | 4 | (1 << 8) | (4 << 16) | (2 << 24)
        words = trace + (
            config_meta, 0x10000001, 1, 1, 1,
            3, 0x0E011001, 0,
            4 | (1 << 8) | (2 << 16), 0,
        )
        struct.pack_into("<32I", data, 312, *words)
        data[440:-4] = entry
        struct.pack_into("<I", data, total - 4, zlbootdiag.fnv32(data[:-4]))
        return bytes(data)

    def test_decodes_storage_failure(self) -> None:
        report = zlbootdiag.parse(struct.pack("<I", 7) + self.make_v1_payload())
        self.assertEqual(report["state_name"], "storage-refused")
        self.assertEqual(report["reason_name"], "msc-init")
        self.assertEqual(report["msc_stage_name"], "endpoints")
        self.assertEqual(report["usb_id"], "0718:067d")
        self.assertEqual(report["portsc"], "0x00000c03")

    def test_decodes_recovered_storage_state(self) -> None:
        payload = bytearray(self.make_v6_payload())
        struct.pack_into("<4I", payload, 16, 3, 0, 0, 9)
        struct.pack_into("<I", payload, len(payload) - 4,
                         zlbootdiag.fnv32(payload[:-4]))
        report = zlbootdiag.parse(bytes(payload), efivarfs=False)
        self.assertEqual(report["state_name"], "storage-ready")
        self.assertEqual(report["reason_name"], "none")
        self.assertEqual(report["msc_stage_name"], "ready")

    def test_decodes_every_v2_port_without_overwrite(self) -> None:
        payload = self.make_v2_payload()
        report = zlbootdiag.parse(struct.pack("<I", 7) + payload)
        self.assertEqual(report["version"], 2)
        self.assertEqual(report["msc_result_name"], "not-ready")
        self.assertEqual(report["xhci_completion_code"], "not-attempted")
        self.assertEqual(len(report["ports"]), 2)
        self.assertEqual(report["ports"][0]["usb_id"], "0718:067d")
        self.assertEqual(report["ports"][0]["msc_stage_name"], "config-header")
        self.assertEqual(report["ports"][0]["ep0_event_stage_name"], "data")
        self.assertEqual(report["ports"][1]["usb_id"], "8087:0026")
        self.assertEqual(report["ports"][1]["msc_completion_code"], "not-attempted")
        self.assertEqual(zlbootdiag.payload_size(b"xxxx" + payload, 4), len(payload))

    def test_v3_retains_transport_and_sense_failure(self) -> None:
        payload = self.make_v3_payload()
        report = zlbootdiag.parse(payload, efivarfs=False)
        self.assertEqual(report["version"], 3)
        self.assertEqual(report["reason_name"], "capacity")
        self.assertEqual(report["transport_xhci_completion_code"], 4)
        self.assertEqual(report["csw_status"], 1)
        self.assertEqual(report["csw_residue"], 512)
        self.assertEqual(report["recovery"], "completed")
        self.assertEqual(report["scsi_opcode"], "0x25")
        self.assertTrue(report["sense_valid"])
        self.assertEqual((report["sense_key"], report["sense_asc"]), (2, 0x3A))
        self.assertEqual(report["ports"][0]["ep0_event_stage_name"], "status")
        self.assertEqual(report["ports"][0]["ep0_attempts"], 3)
        self.assertEqual(report["ports"][0]["ep0_recovery"], "completed")
        self.assertEqual(report["ports"][0]["speed_id"], 3)

    def test_refuses_corrupt_payload(self) -> None:
        data = bytearray(self.make_v3_payload())
        data[40] ^= 1
        with self.assertRaisesRegex(ValueError, "checksum mismatch"):
            zlbootdiag.parse(bytes(data), efivarfs=False)

    def test_v4_decodes_exact_ep0_td_event_context_and_probes(self) -> None:
        payload = self.make_v4_payload()
        report = zlbootdiag.parse(payload, efivarfs=False)
        self.assertEqual(report["version"], 4)
        self.assertEqual(report["xhci_version"], "0x0110")
        trace = report["ports"][0]["ep0_trace"]
        self.assertEqual(trace["setup_bytes"], "80 06 00 02 00 00 09 00")
        self.assertEqual((trace["request_type"], trace["request"]), (0x80, 6))
        self.assertEqual((trace["value"], trace["length"]), (0x0200, 9))
        self.assertEqual(trace["event_pointer"], "0x000000000e011030")
        self.assertEqual(trace["event_completion_code"], 4)
        self.assertEqual((trace["event_slot"], trace["event_endpoint"]), (1, 1))
        self.assertEqual(trace["ep_state"], 2)
        self.assertEqual(trace["ep_dequeue"], "0x000000000e011030")
        self.assertEqual(trace["ep_dcs"], 1)
        self.assertEqual((trace["software_enqueue"], trace["software_cycle"]), (6, 1))
        self.assertEqual((trace["context_size"], trace["attempt"]), (64, 3))
        self.assertEqual(trace["probe_completion_codes"], [1, 1, 4, 4])
        self.assertEqual(zlbootdiag.payload_size(b"xxxx" + payload, 4), len(payload))

    def test_v5_decodes_immediate_identical_device_pair_and_lifecycle(self) -> None:
        payload = self.make_v5_payload()
        report = zlbootdiag.parse(payload, efivarfs=False)
        self.assertEqual(report["version"], 5)
        probe = report["first_device_probe"]
        self.assertTrue(probe["valid"])
        self.assertEqual(probe["first"]["setup_bytes"], "80 06 00 01 00 00 12 00")
        self.assertEqual(probe["second"]["setup_bytes"], probe["first"]["setup_bytes"])
        self.assertEqual(probe["first"]["event_pointer"], "0x000000000e011020")
        self.assertEqual(probe["second"]["event_pointer"], "0x000000000e011030")
        self.assertEqual(probe["first"]["event_completion_code"], 1)
        self.assertEqual(probe["second"]["event_completion_code"], 4)
        self.assertEqual(probe["second"]["event_residual"], 8)
        self.assertEqual((probe["second_meta"]["port"], probe["second_meta"]["slot"]),
                         (4, 1))
        self.assertEqual(probe["second_meta"]["recovery"], "completed")
        self.assertEqual(probe["first_device_address"], 5)
        self.assertEqual(probe["second_device_address"], 5)
        self.assertEqual(probe["first_slot_state"], 2)
        self.assertEqual(probe["second_slot_state"], 2)
        life = report["xhci_lifecycle"]
        self.assertTrue(life["legacy_found"])
        self.assertTrue(life["firmware_cooperative"])
        self.assertFalse(life["firmware_forced"])
        self.assertEqual(life["xecp"], 0x846C // 4)
        self.assertEqual(life["legctlsts_after"], "0xe0000000")
        self.assertEqual(zlbootdiag.payload_size(b"xxxx" + payload, 4), len(payload))

    def test_refuses_corrupt_v5_payload(self) -> None:
        data = bytearray(self.make_v5_payload())
        data[190] ^= 1
        with self.assertRaisesRegex(ValueError, "checksum mismatch"):
            zlbootdiag.parse(bytes(data), efivarfs=False)

    def test_v6_retains_first_config_and_clean_reenumeration_results(self) -> None:
        payload = self.make_v6_payload()
        report = zlbootdiag.parse(payload, efivarfs=False)
        self.assertEqual(report["version"], 6)
        probe = report["first_config_probe"]
        self.assertTrue(probe["valid"])
        self.assertEqual(probe["trace"]["setup_bytes"],
                         "80 06 00 02 00 00 09 00")
        self.assertEqual(probe["trace"]["event_pointer"],
                         "0x000000000e011060")
        self.assertEqual(probe["trace"]["event_completion_code"], 4)
        self.assertEqual(probe["trace"]["software_enqueue"], 9)
        self.assertEqual(probe["recovery_kind"], "reset-endpoint")
        self.assertEqual(probe["recovery_command_cc"], 1)
        self.assertEqual(probe["set_dequeue_cc"], 1)
        self.assertEqual(probe["post_ep_state"], 3)
        self.assertEqual(probe["fresh_old_scheme_cc"], 4)
        self.assertEqual(probe["fresh_preaddress_scheme_cc"], 1)
        self.assertEqual(probe["winning_scheme"], "pre-address")
        self.assertEqual(zlbootdiag.payload_size(b"xxxx" + payload, 4), len(payload))


if __name__ == "__main__":
    unittest.main(verbosity=2)
