#!/usr/bin/env python3
"""QEMU-prove open -> first draw -> close for every registry application.

This consumes app-manifest.json and the compositor's serial-only lifecycle
stream. It deliberately proves only the 47 applications reached through All
Applications; boot, dock, menu, Run, and shell-word routes are different
contracts and remain explicitly outside this receipt.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import time


HERE = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.dirname(os.path.dirname(HERE))
REPO_ROOT = os.path.dirname(KERNEL_ROOT)
METADATA = os.path.join(KERNEL_ROOT, "metadata")
sys.path.insert(0, os.path.join(KERNEL_ROOT, "tests", "oracle"))
import zlosboot as zb  # noqa: E402


MANIFEST = os.path.join(METADATA, "app-manifest.json")
BUILD_IDENTITY = os.path.join(METADATA, "build-identity.json")
DEFAULT_RECEIPT = os.path.join(
    KERNEL_ROOT, "tests", "oracle", "out", "app-lifecycle-qemu.json")
LIFECYCLE_RE = re.compile(
    r"wm:lifecycle v=1 event=(open|ready|close) slot=(\d+) app=(\d+) "
    r"generation=(\d+) live=(\d+)"
)
MANIFEST_RE = re.compile(
    r"app-manifest: schema=(\d+) entries=(\d+) sha256=([0-9a-f]{64})"
)
BUILD_IDENTITY_RE = re.compile(
    r"build-identity: schema=(\d+) id=([0-9a-f]{64})"
)
BUILD_SOURCE_RE = re.compile(r"build-source: head=([0-9a-f]{40}) dirty=([01])")
W, H = 1280, 800


def fail(message):
    raise ValueError(message)


def read_json(path):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


def events(text):
    return [
        {
            "event": match.group(1),
            "slot": int(match.group(2)),
            "app": int(match.group(3)),
            "generation": int(match.group(4)),
            "live": int(match.group(5)),
        }
        for match in LIFECYCLE_RE.finditer(text)
    ]


def await_event(serial, start, event, app, ceiling):
    deadline = time.monotonic() + ceiling
    while time.monotonic() < deadline:
        serial.pump()
        found = [item for item in events(serial.all[start:])
                 if item["event"] == event and item["app"] == app]
        if found:
            return found[-1]
        time.sleep(0.01)
    tail = serial.all[start:][-1200:]
    fail(f"timed out waiting for {event} app {app}; serial tail:\n{tail}")


def validate_cycle(opened, ready, closed, app, baseline):
    errors = []
    for label, item in (("open", opened), ("ready", ready), ("close", closed)):
        if item["event"] != label:
            errors.append(f"{label}: wrong event {item['event']}")
        if item["app"] != app:
            errors.append(f"{label}: wrong app {item['app']}")
    identity = {(item["slot"], item["generation"])
                for item in (opened, ready, closed)}
    if len(identity) != 1:
        errors.append(f"slot/generation changed across lifecycle: {sorted(identity)}")
    if opened["live"] != baseline + 1:
        errors.append(f"open live={opened['live']}, expected {baseline + 1}")
    if ready["live"] != baseline + 1:
        errors.append(f"ready live={ready['live']}, expected {baseline + 1}")
    if closed["live"] != baseline:
        errors.append(f"close live={closed['live']}, expected {baseline}")
    return errors


def selftest():
    good = (
        {"event": "open", "slot": 4, "app": 15, "generation": 3, "live": 5},
        {"event": "ready", "slot": 4, "app": 15, "generation": 3, "live": 5},
        {"event": "close", "slot": 4, "app": 15, "generation": 3, "live": 4},
    )
    if validate_cycle(*good, 15, 4):
        fail("selftest valid lifecycle was rejected")
    mutations = []
    wrong_event = [dict(item) for item in good]
    wrong_event[1]["event"] = "open"
    mutations.append(("missing-ready", wrong_event))
    wrong_generation = [dict(item) for item in good]
    wrong_generation[2]["generation"] = 4
    mutations.append(("generation-reuse", wrong_generation))
    wrong_app = [dict(item) for item in good]
    wrong_app[1]["app"] = 16
    mutations.append(("wrong-app", wrong_app))
    leaked = [dict(item) for item in good]
    leaked[2]["live"] = 5
    mutations.append(("close-leak", leaked))
    caught = []
    for name, mutant in mutations:
        if validate_cycle(*mutant, 15, 4):
            caught.append(name)
        else:
            fail(f"selftest mutation escaped: {name}")
    print("app-lifecycle selftest: caught " + ", ".join(caught))


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def shipped_manifest(text, manifest_path=MANIFEST):
    matches = MANIFEST_RE.findall(text)
    if len(matches) != 1:
        fail(f"guest emitted {len(matches)} app-manifest receipts, expected exactly one")
    schema, entries, digest = matches[0]
    expected = sha256(manifest_path)
    if (int(schema), int(entries), digest) != (1, 62, expected):
        fail(
            "guest app-manifest receipt mismatch: "
            f"schema={schema} entries={entries} sha256={digest}, expected 1/62/{expected}"
        )
    return {"schema": int(schema), "entries": int(entries), "sha256": digest}


def shipped_build_identity(text, identity_path=BUILD_IDENTITY):
    matches = BUILD_IDENTITY_RE.findall(text)
    if len(matches) != 1:
        fail(f"guest emitted {len(matches)} build-identity receipts, expected exactly one")
    schema, identity = matches[0]
    sources = BUILD_SOURCE_RE.findall(text)
    if len(sources) != 1:
        fail(f"guest emitted {len(sources)} build-source receipts, expected exactly one")
    head, dirty = sources[0]
    expected = read_json(identity_path)
    wanted = (1, expected["identity_sha256"], expected["git"]["head"],
              1 if expected["git"]["dirty"] else 0)
    got = (int(schema), identity, head, int(dirty))
    if got != wanted:
        fail(f"guest build-identity receipt mismatch: {got!r}, expected {wanted!r}")
    return {"schema": got[0], "id": identity, "head": head, "dirty": bool(got[3])}


def command_output(argv):
    return subprocess.check_output(argv, cwd=REPO_ROOT, text=True).strip()


def write_receipt(path, receipt):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    temp = path + ".tmp"
    with open(temp, "w", encoding="utf-8") as handle:
        json.dump(receipt, handle, indent=2, sort_keys=True)
        handle.write("\n")
    os.replace(temp, path)


def run(receipt_path, no_build, timeout):
    manifest = read_json(MANIFEST)
    registry = sorted(
        (entry for entry in manifest["entries"] if entry["source_family"] == "registry"),
        key=lambda entry: entry["catalog_index"],
    )
    if len(registry) != 47:
        fail(f"manifest has {len(registry)} registry apps, expected 47")
    catalog = zb.catalog_apps()
    for entry in registry:
        if catalog.get(entry["name"]) != entry["catalog_index"]:
            fail(f"catalog route drift for {entry['name']!r}")

    cycles = []
    with zb.Machine(W, H, do_build=not no_build, boot_timeout=timeout,
                    how="native") as machine:
        serial = machine.ser
        ui = zb.guest_ui(machine.w)
        start = len(serial.all)
        zb.type_line(serial, "apps", timeout)
        catalog_open = await_event(serial, start, "open", 70, timeout)
        catalog_ready = await_event(serial, start, "ready", 70, timeout)
        if (catalog_open["slot"], catalog_open["generation"]) != (
                catalog_ready["slot"], catalog_ready["generation"]):
            fail("catalog open/ready identity changed")
        baseline = catalog_ready["live"]
        rows = zb.WIN_REPORT.findall(serial.all[start:])
        if not rows:
            fail("catalog opened but wm_report emitted no client rectangle")
        _, cx, cy, cw, ch = (int(value) for value in rows[-1])
        cols = max(1, cw // (zb.CAT_TILE_W * ui))
        visible_rows = max(1, (ch - zb.CAT_HEADER * ui) // (zb.CAT_TILE_H * ui))
        scroll = 0

        for number, entry in enumerate(registry, 1):
            index = entry["catalog_index"]
            col, row = index % cols, index // cols
            wanted_scroll = max(0, row - visible_rows + 1)
            if wanted_scroll != scroll:
                zb.at(machine.qmp, cx + cw // 2, cy + ch // 2,
                      machine.w, machine.h)
                serial.drain(0.1)
                zb.wheel(machine.qmp, scroll - wanted_scroll)
                scroll = wanted_scroll
            x = cx + col * zb.CAT_TILE_W * ui + zb.CAT_TILE_W * ui // 2
            y = (cy + zb.CAT_HEADER * ui
                 + (row - scroll) * zb.CAT_TILE_H * ui
                 + zb.CAT_TILE_H * ui // 2)
            start = len(serial.all)
            zb.click(machine.qmp, x, y, machine.w, machine.h, serial.drain)
            opened = await_event(serial, start, "open", entry["id"], timeout)
            ready = await_event(serial, start, "ready", entry["id"], timeout)
            close_start = len(serial.all)
            serial.send("\x17")
            closed = await_event(serial, close_start, "close", entry["id"], timeout)
            errors = validate_cycle(opened, ready, closed, entry["id"], baseline)
            if errors:
                fail(f"{entry['name']}: " + "; ".join(errors))
            cycles.append({
                "id": entry["id"],
                "name": entry["name"],
                "catalog_index": index,
                "open": opened,
                "ready": ready,
                "close": closed,
            })
            print(f"[{number:02d}/47] {entry['name']:<20} "
                  f"slot {opened['slot']} gen {opened['generation']} "
                  f"live {opened['live']}->{closed['live']} PASS")

    iso = os.path.join(KERNEL_ROOT, "zlOS.iso")
    source_paths = [
        os.path.join(KERNEL_ROOT, "src", "graphics", "windowing", "wm.c"),
        os.path.join(KERNEL_ROOT, "src", "kernel.zl"),
        os.path.join(KERNEL_ROOT, "apps", "apps_registry.zl"),
        MANIFEST,
        BUILD_IDENTITY,
        os.path.join(KERNEL_ROOT, "build_identity_embed.zl"),
        os.path.abspath(__file__),
    ]
    receipt = {
        "schema": "zlos.application-lifecycle-qemu-receipt.v1",
        "evidence": "QEMU pointer/compositor lifecycle; not workflow or physical proof",
        "scope": "47 All Applications registry entries only",
        "excluded_routes": ["boot-open", "dock", "menu", "Run", "shell-word"],
        "source_head": command_output(["git", "rev-parse", "HEAD"]),
        "source_files_sha256": {
            os.path.relpath(path, REPO_ROOT): sha256(path) for path in source_paths
        },
        "artifact": {"path": "kernel/zlOS.iso", "sha256": sha256(iso)},
        "shipped_manifest": shipped_manifest(serial.all),
        "shipped_build_identity": shipped_build_identity(serial.all),
        "environment": {
            "qemu": subprocess.check_output(["qemu-system-x86_64", "--version"], text=True).splitlines()[0],
            "display": [machine.w, machine.h],
            "pointer": "QEMU absolute usb-tablet through zlOS input routing",
        },
        "catalogue": {"open": catalog_open, "ready": catalog_ready, "baseline_live": baseline},
        "result": {"passed": len(cycles), "failed": 0, "cycles": cycles},
        "weakest_link": "first compositor draw is readiness, not a successful user workflow; non-catalog routes remain unproven here",
    }
    write_receipt(receipt_path, receipt)
    print(f"app-lifecycle: PASS: {len(cycles)}/47 open-ready-close cycles -> {receipt_path}")


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--receipt", default=DEFAULT_RECEIPT)
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--timeout", type=float, default=300.0)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args(argv)
    try:
        if args.selftest:
            selftest()
            return 0
        run(args.receipt, args.no_build, args.timeout)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"app-lifecycle: FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
