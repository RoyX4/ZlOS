#!/usr/bin/env python3
"""Read zlOS's one-record UEFI fallback diagnostic."""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path


GUID = "8b1e7a9f-5e2d-4d39-9a3c-cb6f92e4d5a1"
DEFAULT_PATH = Path(f"/sys/firmware/efi/efivars/ZlBootDiag-{GUID}")
V1_PAYLOAD_BYTES = 64
V2_HEADER_BYTES = 64
V3_HEADER_BYTES = 96
V4_HEADER_BYTES = 96
V5_HEADER_BYTES = 312
V6_HEADER_BYTES = 440
V2_ENTRY_BYTES = 16
V4_ENTRY_BYTES = 104
MAGIC_V1 = b"ZLDIAG1\0"
MAGIC_V2 = b"ZLDIAG2\0"
MAGIC_V3 = b"ZLDIAG3\0"
MAGIC_V4 = b"ZLDIAG4\0"
MAGIC_V5 = b"ZLDIAG5\0"
MAGIC_V6 = b"ZLDIAG6\0"
MAGIC = MAGIC_V6
STAGES = {
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
ERRORS = {
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
ENUM_STAGES = {
    0: "idle",
    1: "connected",
    2: "port-ready",
    3: "slot-enabled",
    4: "addressed",
    5: "ep0-ready",
    6: "device-descriptor",
}
MSC_RESULTS = {
    0: "ok",
    1: "bad-argument",
    2: "not-ready",
    3: "cbw-transfer",
    4: "data-transfer",
    5: "csw-transfer",
    6: "csw-signature",
    7: "csw-tag",
    8: "csw-failed",
    9: "csw-phase",
    10: "capacity-unsupported",
}
EP0_EVENT_STAGES = {0: "unknown", 1: "setup", 2: "data", 3: "status"}


def fnv32(data: bytes) -> int:
    value = 2166136261
    for byte in data:
        value = ((value ^ byte) * 16777619) & 0xFFFFFFFF
    return value


def _cc(value: int) -> int | str:
    if value in (0xFF, 0xFFFFFFFF):
        return "not-attempted"
    if value == 0:
        return "timeout"
    return value


def payload_size(data: bytes, offset: int = 0) -> int:
    """Return one embedded payload's exact size without trusting its body."""
    magic = data[offset : offset + 8]
    if magic == MAGIC_V1:
        return V1_PAYLOAD_BYTES
    if magic in (MAGIC_V2, MAGIC_V3, MAGIC_V4, MAGIC_V5, MAGIC_V6):
        if len(data) < offset + 16:
            raise ValueError("short v2 diagnostic header")
        return struct.unpack_from("<I", data, offset + 12)[0]
    raise ValueError("bad ZlBootDiag magic")


def parse(data: bytes, *, efivarfs: bool = True) -> dict[str, object]:
    if efivarfs:
        if len(data) < 4:
            raise ValueError("short efivarfs attribute header")
        attributes = struct.unpack_from("<I", data)[0]
        data = data[4:]
    else:
        attributes = 7
    if len(data) < 8 or data[:8] not in (
        MAGIC_V1, MAGIC_V2, MAGIC_V3, MAGIC_V4, MAGIC_V5, MAGIC_V6
    ):
        raise ValueError("bad ZlBootDiag magic")
    if data[:8] == MAGIC_V1:
        return _parse_v1(data, attributes)
    return _parse_v2_or_v3(data, attributes)


def _base(attributes: int, state: int, reason: int, result: int) -> dict[str, object]:
    return {
        "attributes": f"0x{attributes:08x}",
        "state": state,
        "state_name": {
            1: "armed-before-exit",
            2: "storage-refused",
            3: "storage-ready",
        }.get(state, f"unknown-{state}"),
        "reason": reason,
        "reason_name": ERRORS.get(reason, f"unknown-{reason}"),
        "msc_result": result,
        "msc_result_name": MSC_RESULTS.get(result, f"unknown-{result}"),
    }


def _parse_v1(data: bytes, attributes: int) -> dict[str, object]:
    if len(data) != V1_PAYLOAD_BYTES:
        raise ValueError(f"expected {V1_PAYLOAD_BYTES} v1 payload bytes, got {len(data)}")
    version, size = struct.unpack_from("<HH", data, 8)
    if version != 1 or size != V1_PAYLOAD_BYTES:
        raise ValueError(f"unsupported diagnostic version/size {version}/{size}")
    stored_hash = struct.unpack_from("<I", data, 60)[0]
    actual_hash = fnv32(data[:60])
    if stored_hash != actual_hash:
        raise ValueError(
            f"diagnostic checksum mismatch: stored {stored_hash:08x}, actual {actual_hash:08x}"
        )
    fields = struct.unpack_from("<12I", data, 12)
    state, reason, result, stage, port, slot, cc, portsc, usbsts, usbcmd, vidpid, ticks = fields
    report = _base(attributes, state, reason, result)
    report.update({
        "version": 1,
        "msc_stage": stage,
        "msc_stage_name": STAGES.get(stage, f"unknown-{stage}"),
        "port": port,
        "slot": slot,
        "xhci_completion_code": _cc(cc),
        "portsc": f"0x{portsc:08x}",
        "usbsts": f"0x{usbsts:08x}",
        "usbcmd": f"0x{usbcmd:08x}",
        "usb_id": f"{(vidpid >> 16) & 0xffff:04x}:{vidpid & 0xffff:04x}",
        "ticks": ticks,
        "checksum": f"0x{stored_hash:08x}",
        "ports": [],
    })
    return report


def _decode_ep0_trace(trace: tuple[int, ...]) -> dict[str, object]:
    setup = trace[0:4]
    data_trb = trace[4:8]
    status_trb = trace[8:12]
    event = trace[12:16]
    context_dw0, context_dw2, context_dw3, meta = trace[16:20]
    setup_bytes = struct.pack("<II", setup[0], setup[1])
    probes = trace[20]
    return {
        "setup_trb": [f"0x{word:08x}" for word in setup],
        "data_trb": [f"0x{word:08x}" for word in data_trb],
        "status_trb": [f"0x{word:08x}" for word in status_trb],
        "setup_bytes": setup_bytes.hex(" "),
        "request_type": setup_bytes[0],
        "request": setup_bytes[1],
        "value": struct.unpack_from("<H", setup_bytes, 2)[0],
        "index": struct.unpack_from("<H", setup_bytes, 4)[0],
        "length": struct.unpack_from("<H", setup_bytes, 6)[0],
        "event": [f"0x{word:08x}" for word in event],
        "event_pointer": f"0x{((event[1] << 32) | event[0]):016x}",
        "event_completion_code": (event[2] >> 24) & 0xFF,
        "event_residual": event[2] & 0xFFFFFF,
        "event_slot": (event[3] >> 24) & 0xFF,
        "event_endpoint": (event[3] >> 16) & 0x1F,
        "ep_context_dw0": f"0x{context_dw0:08x}",
        "ep_context_dw2": f"0x{context_dw2:08x}",
        "ep_context_dw3": f"0x{context_dw3:08x}",
        "ep_state": context_dw0 & 7,
        "ep_dequeue": f"0x{((context_dw3 << 32) | (context_dw2 & ~0xF)):016x}",
        "ep_dcs": context_dw2 & 1,
        "software_enqueue": meta & 0xFF,
        "software_cycle": (meta >> 8) & 1,
        "valid": bool(meta & (1 << 9)),
        "data_present": bool(meta & (1 << 10)),
        "event_valid": bool(meta & (1 << 11)),
        "context_valid": bool(meta & (1 << 12)),
        "timeout": bool(meta & (1 << 13)),
        "context_size": (meta >> 16) & 0xFF,
        "attempt": (meta >> 24) & 0xFF,
        "probe_completion_codes": [
            (probes >> shift) & 0xFF for shift in (0, 8, 16, 24)
        ],
        "buffer_first4": f"0x{trace[21]:08x}",
    }


def _decode_probe_meta(value: int) -> dict[str, object]:
    recovery = (value >> 24) & 3
    return {
        "valid": bool(value & 0x80000000),
        "port": value & 0xFF,
        "slot": (value >> 8) & 0xFF,
        "completion_code": _cc((value >> 16) & 0xFF),
        "recovery": {0: "not-attempted", 1: "failed", 2: "completed"}.get(
            recovery, f"unknown-{recovery}"
        ),
    }


def _parse_v2_or_v3(data: bytes, attributes: int) -> dict[str, object]:
    if len(data) < V2_HEADER_BYTES + 4:
        raise ValueError("short v2 diagnostic payload")
    version, header_bytes = struct.unpack_from("<HH", data, 8)
    total_bytes = struct.unpack_from("<I", data, 12)[0]
    expected_header = {
        2: V2_HEADER_BYTES,
        3: V3_HEADER_BYTES,
        4: V4_HEADER_BYTES,
        5: V5_HEADER_BYTES,
        6: V6_HEADER_BYTES,
    }.get(version)
    if expected_header is None or header_bytes != expected_header:
        raise ValueError(f"unsupported diagnostic version/header {version}/{header_bytes}")
    if total_bytes != len(data):
        raise ValueError(f"v2 size says {total_bytes}, got {len(data)}")
    count, entry_bytes = struct.unpack_from("<HH", data, 60)
    expected = header_bytes + count * entry_bytes + 4
    expected_entry = V4_ENTRY_BYTES if version in (4, 5, 6) else V2_ENTRY_BYTES
    if entry_bytes != expected_entry or expected != total_bytes:
        raise ValueError(
            f"bad v2 port table: count {count}, entry {entry_bytes}, total {total_bytes}"
        )
    stored_hash = struct.unpack_from("<I", data, total_bytes - 4)[0]
    actual_hash = fnv32(data[: total_bytes - 4])
    if stored_hash != actual_hash:
        raise ValueError(
            f"diagnostic checksum mismatch: stored {stored_hash:08x}, actual {actual_hash:08x}"
        )
    state, reason, result, stage, port, slot, cc, usbsts, usbcmd, ticks, vidpid = (
        struct.unpack_from("<11I", data, 16)
    )
    ports: list[dict[str, object]] = []
    for index in range(count):
        off = header_bytes + index * entry_bytes
        p, mstage, pslot, mcc, estage, ecc, speed, flags, vid, pid, portsc = (
            struct.unpack_from("<8BHHI", data, off)
        )
        item: dict[str, object] = {
            "port": p,
            "usb_id": f"{vid:04x}:{pid:04x}",
            "msc_stage": mstage,
            "msc_stage_name": STAGES.get(mstage, f"unknown-{mstage}"),
            "msc_candidate": bool(flags & 4),
            "ep0_event_stage": (flags >> 3) & 3,
            "ep0_event_stage_name": EP0_EVENT_STAGES[(flags >> 3) & 3],
            "ep0_attempts": (flags >> 5) & 3,
            "ep0_recovery": {
                0: "not-attempted", 1: "failed", 2: "completed"
            }.get((speed >> 4) & 3, f"unknown-{(speed >> 4) & 3}"),
            "slot": pslot,
            "msc_completion_code": _cc(mcc),
            "enum_stage": estage,
            "enum_stage_name": ENUM_STAGES.get(estage, f"unknown-{estage}"),
            "enum_completion_code": _cc(ecc),
            "speed_id": speed & 0x0f,
            "connected": bool(flags & 1),
            "enabled": bool(flags & 2),
            "portsc": f"0x{portsc:08x}",
        }
        if version in (4, 5, 6):
            trace = struct.unpack_from("<22I", data, off + 16)
            item["ep0_trace"] = _decode_ep0_trace(trace)
        ports.append(item)
    selected = next((item for item in ports if item["port"] == port), None)
    report = _base(attributes, state, reason, result)
    report.update({
        "version": version,
        "msc_stage": stage,
        "msc_stage_name": STAGES.get(stage, f"unknown-{stage}"),
        "port": port,
        "slot": slot,
        "xhci_completion_code": _cc(cc),
        "portsc": selected["portsc"] if selected else "0x00000000",
        "usbsts": f"0x{usbsts:08x}",
        "usbcmd": f"0x{usbcmd:08x}",
        "usb_id": f"{(vidpid >> 16) & 0xffff:04x}:{vidpid & 0xffff:04x}",
        "ticks": ticks,
        "checksum": f"0x{stored_hash:08x}",
        "ports": ports,
    })
    if version in (3, 4, 5, 6):
        last_cc, csw_status, residue, recovery, opcode, sense_valid = (
            struct.unpack_from("<6I", data, 64)
        )
        sense_key, sense_asc, sense_ascq = struct.unpack_from("<3B", data, 88)
        report.update({
            "transport_xhci_completion_code": _cc(last_cc),
            "csw_status": "not-received" if csw_status == 0xFFFFFFFF else csw_status,
            "csw_residue": residue,
            "recovery": {0xFFFFFFFF: "not-attempted", 0: "failed", 1: "completed"}.get(
                recovery, f"unknown-{recovery}"
            ),
            "scsi_opcode": f"0x{opcode & 0xff:02x}",
            "sense_valid": bool(sense_valid),
            "sense_key": sense_key,
            "sense_asc": sense_asc,
            "sense_ascq": sense_ascq,
        })
    if version in (4, 5, 6):
        report["xhci_version"] = f"0x{struct.unpack_from('<I', data, 92)[0]:04x}"
    if version in (5, 6):
        first_raw = struct.unpack_from("<22I", data, 96)
        second_raw = struct.unpack_from("<22I", data, 184)
        first_meta = _decode_probe_meta(first_raw[20])
        second_meta = _decode_probe_meta(second_raw[20])
        report["first_device_probe"] = {
            "valid": first_meta["valid"] and second_meta["valid"],
            "first_meta": first_meta,
            "second_meta": second_meta,
            "first": _decode_ep0_trace(first_raw),
            "second": _decode_ep0_trace(second_raw),
        }
        legsup_before, legsup_after, legctl_before, legctl_after, flags, xecp, pre, post = (
            struct.unpack_from("<8I", data, 272)
        )
        report["xhci_lifecycle"] = {
            "legacy_found": bool(flags & 1),
            "firmware_cooperative": bool(flags & 2),
            "firmware_forced": bool(flags & 4),
            "xecp": xecp,
            "legsup_before": f"0x{legsup_before:08x}",
            "legsup_after": f"0x{legsup_after:08x}",
            "legctlsts_before": f"0x{legctl_before:08x}",
            "legctlsts_after": f"0x{legctl_after:08x}",
            "usbsts_before_reset": f"0x{pre:08x}",
            "usbsts_after_reset": f"0x{post:08x}",
        }
        first_slot, second_slot = struct.unpack_from("<2I", data, 304)
        report["first_device_probe"]["first_slot_context_dw3"] = (
            f"0x{first_slot:08x}"
        )
        report["first_device_probe"]["second_slot_context_dw3"] = (
            f"0x{second_slot:08x}"
        )
        report["first_device_probe"]["first_device_address"] = first_slot & 0xFF
        report["first_device_probe"]["second_device_address"] = second_slot & 0xFF
        report["first_device_probe"]["first_slot_state"] = (first_slot >> 27) & 0x1F
        report["first_device_probe"]["second_slot_state"] = (second_slot >> 27) & 0x1F
    if version == 6:
        config_raw = struct.unpack_from("<32I", data, 312)
        config_meta = _decode_probe_meta(config_raw[22])
        retry_word = config_raw[30]
        report["first_config_probe"] = {
            "valid": config_meta["valid"],
            "meta": config_meta,
            "trace": _decode_ep0_trace(config_raw[:22]),
            "slot_context_dw3": f"0x{config_raw[23]:08x}",
            "device_address": config_raw[23] & 0xFF,
            "slot_state": (config_raw[23] >> 27) & 0x1F,
            "recovery_kind": {0: "none", 1: "reset-endpoint", 2: "stop-endpoint"}.get(
                config_raw[24], f"unknown-{config_raw[24]}"
            ),
            "recovery_command_cc": _cc(config_raw[25]),
            "set_dequeue_cc": _cc(config_raw[26]),
            "post_ep_context_dw0": f"0x{config_raw[27]:08x}",
            "post_ep_context_dw2": f"0x{config_raw[28]:08x}",
            "post_ep_context_dw3": f"0x{config_raw[29]:08x}",
            "post_ep_state": config_raw[27] & 7,
            "post_ep_dequeue": f"0x{((config_raw[29] << 32) | (config_raw[28] & ~0xF)):016x}",
            "fresh_old_scheme_cc": _cc(retry_word & 0xFF),
            "fresh_preaddress_scheme_cc": _cc((retry_word >> 8) & 0xFF),
            "winning_scheme": {
                0: "none", 1: "fresh-old", 2: "pre-address"
            }.get((retry_word >> 16) & 3, f"unknown-{(retry_word >> 16) & 3}"),
        }
    return report


def human(report: dict[str, object]) -> str:
    text = (
        f"state: {report['state_name']}\n"
        f"reason: {report['reason']} ({report['reason_name']})\n"
        f"MSC: result {report['msc_result']} ({report['msc_result_name']}), "
        f"summary stage {report['msc_stage']} "
        f"({report['msc_stage_name']}), port {report['port']}, slot {report['slot']}, "
        f"xHCI cc {report['xhci_completion_code']}\n"
        f"USB: {report['usb_id']}, PORTSC {report['portsc']}\n"
        f"controller: USBSTS {report['usbsts']}, USBCMD {report['usbcmd']}\n"
        f"ticks: {report['ticks']}"
    )
    ports = report.get("ports", [])
    if ports:
        text += "\nports:"
        for item in ports:
            candidate = " MSC" if item["msc_candidate"] else ""
            text += (
                f"\n  {item['port']:2}: {item['usb_id']} slot {item['slot']} "
                f"enum {item['enum_stage_name']}/{item['enum_completion_code']} "
                f"msc {item['msc_stage_name']}/{item['msc_completion_code']} "
                f"ep0 {item['ep0_event_stage_name']} "
                f"tries {item['ep0_attempts']} recovery {item['ep0_recovery']}"
                f"{candidate} PORTSC {item['portsc']}"
            )
    if report.get("version") in (3, 4, 5, 6):
        text += (
            f"\ntransport: xHCI {report['transport_xhci_completion_code']}, "
            f"opcode {report['scsi_opcode']}, CSW {report['csw_status']}, "
            f"residue {report['csw_residue']}, recovery {report['recovery']}"
        )
        if report["sense_valid"]:
            text += (
                f", sense {report['sense_key']:02x}/"
                f"{report['sense_asc']:02x}/{report['sense_ascq']:02x}"
            )
    if report.get("version") in (4, 5, 6):
        text += f"\nxHCI version: {report['xhci_version']}"
        for item in ports:
            trace = item.get("ep0_trace")
            if not trace or not trace["valid"]:
                continue
            text += (
                f"\n  port {item['port']} trace: setup {trace['setup_bytes']}; "
                f"event {trace['event_pointer']} cc {trace['event_completion_code']} "
                f"residual {trace['event_residual']}; EP state {trace['ep_state']} "
                f"dequeue {trace['ep_dequeue']} dcs {trace['ep_dcs']}; "
                f"software {trace['software_enqueue']}/{trace['software_cycle']} "
                f"attempt {trace['attempt']}; probes "
                f"{'/'.join(str(code) for code in trace['probe_completion_codes'])}"
            )
    if report.get("version") in (5, 6):
        probe = report["first_device_probe"]
        if probe["valid"]:
            for label in ("first", "second"):
                trace = probe[label]
                meta = probe[f"{label}_meta"]
                text += (
                    f"\nfirst-device {label}: port {meta['port']} slot {meta['slot']} "
                    f"setup {trace['setup_bytes']}; event {trace['event_pointer']} "
                    f"cc {trace['event_completion_code']} residual {trace['event_residual']}; "
                    f"EP state {trace['ep_state']} dequeue {trace['ep_dequeue']}; "
                    f"software {trace['software_enqueue']}/{trace['software_cycle']} "
                    f"recovery {meta['recovery']}"
                )
            text += (
                f"\nfirst-device slot context: "
                f"{probe['first_slot_context_dw3']} addr {probe['first_device_address']} "
                f"state {probe['first_slot_state']} -> "
                f"{probe['second_slot_context_dw3']} addr {probe['second_device_address']} "
                f"state {probe['second_slot_state']}"
            )
        life = report["xhci_lifecycle"]
        text += (
            f"\nxHCI lifecycle: xECP {life['xecp']}, legacy {life['legacy_found']}, "
            f"cooperative {life['firmware_cooperative']}, forced {life['firmware_forced']}; "
            f"LEGSUP {life['legsup_before']}->{life['legsup_after']}; "
            f"LEGCTLSTS {life['legctlsts_before']}->{life['legctlsts_after']}; "
            f"USBSTS {life['usbsts_before_reset']}->{life['usbsts_after_reset']}"
        )
    if report.get("version") == 6:
        probe = report["first_config_probe"]
        if probe["valid"]:
            trace = probe["trace"]
            meta = probe["meta"]
            text += (
                f"\nfirst-config: port {meta['port']} slot {meta['slot']} "
                f"setup {trace['setup_bytes']}; event {trace['event_pointer']} "
                f"cc {trace['event_completion_code']} residual {trace['event_residual']}; "
                f"software {trace['software_enqueue']}/{trace['software_cycle']}; "
                f"recovery {probe['recovery_kind']} cc {probe['recovery_command_cc']} "
                f"set-dequeue {probe['set_dequeue_cc']}; "
                f"fresh retries {probe['fresh_old_scheme_cc']}/"
                f"{probe['fresh_preaddress_scheme_cc']} winner {probe['winning_scheme']}"
            )
    return text


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("path", nargs="?", type=Path, default=DEFAULT_PATH)
    parser.add_argument("--raw", action="store_true", help="input omits efivarfs's 4-byte attributes")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    report = parse(args.path.read_bytes(), efivarfs=not args.raw)
    print(json.dumps(report, indent=2) if args.json else human(report))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
