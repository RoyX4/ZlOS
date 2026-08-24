#!/usr/bin/env python3
"""Exercise every current non-catalogue application launch surface in QEMU.

The registry-app sweep owns IDs 15..69. This probe owns boot-open identities,
all nine dock slots, every shell-word application, Menu, and All Applications.
Run/exec has its own deeper probe and remains a named external prerequisite.
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import os
import re
import subprocess
import sys


HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "oracle"))
import zlosboot as zb  # noqa: E402

_spec = importlib.util.spec_from_file_location(
    "app_lifecycle", os.path.join(HERE, "probe-app-lifecycle.py"))
lc = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(lc)
_gen_spec = importlib.util.spec_from_file_location(
    "app_manifest", os.path.join(HERE, "gen-app-manifest.py"))
gen = importlib.util.module_from_spec(_gen_spec)
_gen_spec.loader.exec_module(gen)

MANIFEST = os.path.join(HERE, "app-manifest.json")
DEFAULT_RECEIPT = os.path.join(HERE, "oracle", "out", "app-routes-qemu.json")
W, H = 1280, 800

DOCK = [
    (0, 0, "Terminal"),
    (1, 5, "Browser"),
    (2, 12, "Text Editor"),
    (3, 8, "Paint"),
    (4, 3, "Snake"),
    (5, 9, "3D"),
    (6, 1, "System Monitor"),
    (7, 2, "About"),
    (8, 6, "Settings"),
]
WORDS = [
    ("snake", 3, "Snake"),
    ("paint", 8, "Paint"),
    ("cube", 9, "3D"),
    ("anim", 10, "zlOS animation"),
    ("mouse", 11, "Pointer"),
    ("edit", 12, "Text Editor"),
    ("files", 13, "Files"),
]
BOOT_OPEN = [(0, "Terminal"), (13, "Files"), (1, "System Monitor")]


def fail(message):
    raise ValueError(message)


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def current_live(serial):
    parsed = lc.events(serial.all)
    if not parsed:
        fail("guest emitted no lifecycle event")
    return parsed[-1]["live"]


def is_open(serial, app):
    relevant = [event for event in lc.events(serial.all)
                if event["app"] == app and event["event"] in {"open", "close"}]
    return bool(relevant and relevant[-1]["event"] == "open")


def click(machine, point):
    zb.click(machine.qmp, point[0], point[1], machine.w, machine.h,
             machine.ser.drain)


def close_after_focus(machine, app, timeout):
    start = len(machine.ser.all)
    machine.ser.send("\x17")
    return lc.await_event(machine.ser, start, "close", app, timeout)


def dock_cycle(machine, point, app, timeout):
    if is_open(machine.ser, app):
        click(machine, point)
        close_after_focus(machine, app, timeout)
    baseline = current_live(machine.ser)
    start = len(machine.ser.all)
    click(machine, point)
    opened = lc.await_event(machine.ser, start, "open", app, timeout)
    ready = lc.await_event(machine.ser, start, "ready", app, timeout)
    # Some shell-backed dock launchers restore Terminal focus. A second dock
    # click must raise/focus the already-open app without creating a duplicate.
    click(machine, point)
    closed = close_after_focus(machine, app, timeout)
    errors = lc.validate_cycle(opened, ready, closed, app, baseline)
    if errors:
        fail("; ".join(errors))
    return {"open": opened, "ready": ready, "close": closed}


def word_cycle(machine, word, app, timeout):
    if is_open(machine.ser, app):
        # Files is part of the boot composition. Reach the existing instance
        # through the same word under test, focus it, then establish the closed
        # prestate. Refusing boot-open apps here would leave their reopen path
        # permanently untestable.
        zb.type_line(machine.ser, word, timeout)
        close_after_focus(machine, app, timeout)
    baseline = current_live(machine.ser)
    start = len(machine.ser.all)
    zb.type_line(machine.ser, word, timeout)
    opened = lc.await_event(machine.ser, start, "open", app, timeout)
    ready = lc.await_event(machine.ser, start, "ready", app, timeout)
    # These five commands restore focus to Terminal after opening. Invoking the
    # existing app again is the keyboard route that focuses it; no new open is
    # allowed on that second invocation.
    if word in {"snake", "paint", "cube", "anim", "mouse"}:
        before = len([e for e in lc.events(machine.ser.all) if e["event"] == "open"])
        zb.type_line(machine.ser, word, timeout)
        after = len([e for e in lc.events(machine.ser.all) if e["event"] == "open"])
        if after != before:
            fail(f"{word}: focusing an existing app opened a duplicate")
    closed = close_after_focus(machine, app, timeout)
    errors = lc.validate_cycle(opened, ready, closed, app, baseline)
    if errors:
        fail("; ".join(errors))
    return {"open": opened, "ready": ready, "close": closed}


def simple_pointer_cycle(machine, point, app, timeout, close_with_pointer=False):
    baseline = current_live(machine.ser)
    start = len(machine.ser.all)
    click(machine, point)
    opened = lc.await_event(machine.ser, start, "open", app, timeout)
    ready = lc.await_event(machine.ser, start, "ready", app, timeout)
    if close_with_pointer:
        close_start = len(machine.ser.all)
        click(machine, point)
        closed = lc.await_event(machine.ser, close_start, "close", app, timeout)
    else:
        closed = close_after_focus(machine, app, timeout)
    errors = lc.validate_cycle(opened, ready, closed, app, baseline)
    if errors:
        fail("; ".join(errors))
    return {"open": opened, "ready": ready, "close": closed}


def write_receipt(path, value):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    temp = path + ".tmp"
    with open(temp, "w", encoding="utf-8") as handle:
        json.dump(value, handle, indent=2, sort_keys=True)
        handle.write("\n")
    os.replace(temp, path)


def source_dock_routes():
    kernel_path = os.path.join(HERE, "kernel.zl")
    kernel = gen.read(kernel_path)
    paths = [kernel_path] + gen.imported_app_paths(kernel_path, kernel)
    sources = {path: gen.read(path) for path in paths}
    constants = gen.constants(sources)
    body = gen.function_body(kernel, "dock_app")
    routes = []
    for slot, token in re.findall(r"if slot == (\d+)\s*\{\s*return (\w+)", body):
        app = int(token) if token.isdigit() else constants.get(token)
        if app is None:
            fail(f"dock_app has unresolved token {token}")
        routes.append((int(slot), app))
    if len(routes) != 9 or len({slot for slot, _ in routes}) != 9:
        fail(f"dock_app defines {len(routes)} non-unique routes, expected 9")
    return routes


def run(receipt_path, no_build, timeout):
    manifest = json.load(open(MANIFEST, encoding="utf-8"))
    identities = {entry["id"]: entry["name"] for entry in manifest["entries"]}
    source_routes = source_dock_routes()
    expected_routes = [(slot, app) for slot, app, _ in DOCK]
    if source_routes != expected_routes:
        fail(f"dock route/source drift: source={source_routes} probe={expected_routes}")
    for _, app, name in DOCK:
        if identities.get(app) != name:
            fail(f"dock route identity drift: {app} is {identities.get(app)!r}, expected {name!r}")
    for _, app, name in WORDS:
        if identities.get(app) != name:
            fail(f"word route identity drift: {app} is {identities.get(app)!r}, expected {name!r}")

    result = {"boot": [], "dock": [], "shell_word": [], "surface": []}
    with zb.Machine(W, H, do_build=not no_build, boot_timeout=timeout,
                    how="native") as machine:
        geometry = zb.dock_geometry(machine.w, machine.h)
        boot_events = lc.events(machine.ser.all)
        for app, name in BOOT_OPEN:
            opened = [event for event in boot_events if event["event"] == "open" and event["app"] == app]
            if len(opened) != 1:
                fail(f"boot identity {name} lacks exactly one open event")
            # Machine returns as soon as the guest says ready and a framebuffer
            # can be captured. The compositor's first repaint may follow that
            # marker, so readiness is awaited rather than sampled immediately.
            ready = lc.await_event(machine.ser, 0, "ready", app, timeout)
            if ready["event"] != "ready":
                fail(f"boot identity {name} lacks exactly one open/ready pair")
            if (opened[0]["slot"], opened[0]["generation"]) != (ready["slot"], ready["generation"]):
                fail(f"boot identity changed before readiness for {name}")
            result["boot"].append({"id": app, "name": name, "open": opened[0], "ready": ready})
            print(f"[boot] {name:<20} open-ready PASS")

        for slot, app, name in DOCK:
            cycle = dock_cycle(machine, geometry["slots"][slot], app, timeout)
            result["dock"].append({"slot": slot, "id": app, "name": name, **cycle})
            print(f"[dock {slot}] {name:<20} open-ready-close PASS")
            if app == 0:
                # Keep a Terminal alive for the shell-word routes below.
                start = len(machine.ser.all)
                click(machine, geometry["slots"][slot])
                lc.await_event(machine.ser, start, "open", app, timeout)
                lc.await_event(machine.ser, start, "ready", app, timeout)

        for word, app, name in WORDS:
            cycle = word_cycle(machine, word, app, timeout)
            result["shell_word"].append({"word": word, "id": app, "name": name, **cycle})
            print(f"[word {word:<5}] {name:<20} open-ready-close PASS")

        menu = simple_pointer_cycle(machine, geometry["topbar_corner"], 4, timeout,
                                    close_with_pointer=True)
        result["surface"].append({"route": "topbar", "id": 4, "name": "Menu", **menu})
        print("[topbar] Menu                 open-ready-close PASS")
        catalogue = simple_pointer_cycle(machine, geometry["grid"], 70, timeout)
        result["surface"].append({"route": "grid", "id": 70, "name": "All Applications", **catalogue})
        print("[grid] All Applications     open-ready-close PASS")

    iso = os.path.join(HERE, "zlOS.iso")
    receipt = {
        "schema": "zlos.application-route-qemu-receipt.v1",
        "evidence": "QEMU input/compositor routes; not workflow or physical proof",
        "source_head": subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=os.path.dirname(HERE), text=True).strip(),
        "source_files_sha256": {
            path: sha256(os.path.join(HERE, path))
            for path in ("kernel.zl", "wm.c", "app-manifest.json",
                         "build-identity.json", "build_identity_embed.zl",
                         "probe-app-routes.py", "probe-app-lifecycle.py",
                         "oracle/zlosboot.py")
        },
        "artifact": {"path": "kernel/zlOS.iso", "sha256": sha256(iso)},
        "shipped_manifest": lc.shipped_manifest(machine.ser.all),
        "shipped_build_identity": lc.shipped_build_identity(machine.ser.all),
        "environment": {
            "qemu": subprocess.check_output(["qemu-system-x86_64", "--version"], text=True).splitlines()[0],
            "display": [machine.w, machine.h],
        },
        "result": result,
        "counts": {key: len(value) for key, value in result.items()},
        "external_prerequisite": "APP_RUN id 7 is exercised by probe-run.py, not this route sweep",
        "weakest_link": "route and first draw are not complete workflows; native physical input is unproven here",
    }
    write_receipt(receipt_path, receipt)
    print(f"app-routes: PASS -> {receipt_path}")


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--receipt", default=DEFAULT_RECEIPT)
    parser.add_argument("--no-build", action="store_true")
    parser.add_argument("--timeout", type=float, default=30.0)
    args = parser.parse_args(argv)
    try:
        run(args.receipt, args.no_build, args.timeout)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"app-routes: FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
