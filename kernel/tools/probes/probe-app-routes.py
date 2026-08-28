#!/usr/bin/env python3
"""Exercise every current non-catalogue application launch surface in QEMU.

The registry-app sweep owns IDs 15..69. This probe owns boot-open identities,
all fourteen register slots, every shell-word application, Menu, System, Type,
and All Applications. Run/exec also has its own deeper workflow probe.
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
KERNEL_ROOT = os.path.dirname(os.path.dirname(HERE))
REPO_ROOT = os.path.dirname(KERNEL_ROOT)
METADATA = os.path.join(KERNEL_ROOT, "metadata")
sys.path.insert(0, os.path.join(KERNEL_ROOT, "tests", "oracle"))
import zlosboot as zb  # noqa: E402

_spec = importlib.util.spec_from_file_location(
    "app_lifecycle", os.path.join(HERE, "probe-app-lifecycle.py"))
lc = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(lc)
_gen_spec = importlib.util.spec_from_file_location(
    "app_manifest", os.path.join(KERNEL_ROOT, "tools", "generators", "gen-app-manifest.py"))
gen = importlib.util.module_from_spec(_gen_spec)
_gen_spec.loader.exec_module(gen)

MANIFEST = os.path.join(METADATA, "app-manifest.json")
DEFAULT_RECEIPT = os.path.join(
    KERNEL_ROOT, "tests", "oracle", "out", "app-routes-qemu.json")
W, H = 1280, 800

REGISTER = [
    (0, 0, "Terminal"),
    (1, 13, "Files"),
    (2, 1, "System Monitor"),
    (3, 12, "Text Editor"),
    (4, 40, "Kernel Log"),
    (5, 42, "Hex Viewer"),
    (6, 33, "Calculator"),
    (7, 50, "Network"),
    (8, 31, "Clocks & Timers"),
    (9, 71, "System"),
    (10, 6, "Settings"),
    (11, 46, "Disk Usage"),
    (12, 32, "System Info"),
    (13, 72, "Type"),
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
MENU_SURFACES = [
    (3, 2, "About"),
    (8, 71, "System"),
    (9, 72, "Type"),
    (11, 5, "Browser"),
]
MENU_HANDLERS = {
    3: "reopen_about",
    8: "open_syspane",
    9: "open_typepane",
    11: "reopen_browser",
}
TITLE_REPORT = re.compile(r"wm: win \d+ title (\d+),(\d+) (\d+)x(\d+)")


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


def focus_reported_window(machine, start):
    reports = TITLE_REPORT.findall(machine.ser.all[start:])
    if not reports:
        fail("opened window emitted no pointer-addressable title rectangle")
    tx, ty, tw, th = (int(value) for value in reports[-1])
    click(machine, (tx + max(8, tw // 3), ty + max(4, th // 2)))


def register_cycle(machine, point, app, timeout):
    if is_open(machine.ser, app):
        click(machine, point)
        close_after_focus(machine, app, timeout)
    baseline = current_live(machine.ser)
    start = len(machine.ser.all)
    click(machine, point)
    opened = lc.await_event(machine.ser, start, "open", app, timeout)
    ready = lc.await_event(machine.ser, start, "ready", app, timeout)
    # Some shell-backed launchers restore Terminal focus. A second register
    # click normally raises/focuses the existing app without a duplicate. Run
    # deliberately restores Terminal focus even on that path, so its title is
    # the only honest focus target before the unified Ctrl+W teardown.
    if app == 7:
        focus_reported_window(machine, start)
    else:
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


def super_cycle(machine, app, timeout):
    baseline = current_live(machine.ser)
    start = len(machine.ser.all)
    zb.tap_key(machine.qmp, "meta_l", machine.ser.drain)
    opened = lc.await_event(machine.ser, start, "open", app, timeout)
    ready = lc.await_event(machine.ser, start, "ready", app, timeout)
    close_start = len(machine.ser.all)
    zb.tap_key(machine.qmp, "meta_l", machine.ser.drain)
    closed = lc.await_event(machine.ser, close_start, "close", app, timeout)
    errors = lc.validate_cycle(opened, ready, closed, app, baseline)
    if errors:
        fail("; ".join(errors))
    return {"open": opened, "ready": ready, "close": closed}


def menu_row_cycle(machine, row, app, timeout):
    baseline = current_live(machine.ser)
    start = len(machine.ser.all)
    zb.tap_key(machine.qmp, "meta_l", machine.ser.drain)
    lc.await_event(machine.ser, start, "open", 4, timeout)
    lc.await_event(machine.ser, start, "ready", 4, timeout)
    rows = zb.WIN_REPORT.findall(machine.ser.all[start:])
    if not rows:
        fail("menu opened but wm_report emitted no client rectangle")
    point = zb.menu_row_point(tuple(int(value) for value in rows[-1]), row, machine.w)
    click(machine, point)
    lc.await_event(machine.ser, start, "close", 4, timeout)
    opened = lc.await_event(machine.ser, start, "open", app, timeout)
    ready = lc.await_event(machine.ser, start, "ready", app, timeout)
    closed = close_after_focus(machine, app, timeout)
    errors = lc.validate_cycle(opened, ready, closed, app, baseline)
    if errors:
        fail("; ".join(errors))
    return {"menu_row": row, "open": opened, "ready": ready, "close": closed}


def write_receipt(path, value):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    temp = path + ".tmp"
    with open(temp, "w", encoding="utf-8") as handle:
        json.dump(value, handle, indent=2, sort_keys=True)
        handle.write("\n")
    os.replace(temp, path)


def source_register_routes():
    kernel_path = os.path.join(KERNEL_ROOT, "src", "kernel.zl")
    kernel = gen.read(kernel_path)
    paths = [kernel_path] + gen.imported_app_paths(kernel_path, kernel)
    sources = {path: gen.read(path) for path in paths}
    constants = gen.constants(sources)
    body = gen.function_body(kernel, "rail_app")
    routes = []
    for slot, token in re.findall(r"if slot == (\d+)\s*\{\s*return (\w+)", body):
        app = int(token) if token.isdigit() else constants.get(token)
        if app is None:
            fail(f"rail_app has unresolved token {token}")
        routes.append((int(slot), app))
    if len(routes) != len(REGISTER) or len({slot for slot, _ in routes}) != len(REGISTER):
        fail(f"rail_app defines {len(routes)} non-unique routes, expected {len(REGISTER)}")
    launch = gen.function_body(kernel, "rail_launch")
    for app, owner in (("APP_SYSPANE", "open_syspane"),
                       ("APP_TYPEPANE", "open_typepane")):
        seam = f"if da == {app} {{ return {owner}() }}"
        if seam not in launch:
            fail(f"rail_launch does not route kernel-owned {app} through {owner}")
    cards = gen.function_body(kernel, "sys_cards")
    if "wm_painted()" not in cards or "wm_frame()" in cards:
        fail("System cards must read wm_painted(), never recursively drive wm_frame()")
    menu = gen.function_body(kernel, "menu_pick")
    for row, _, name in MENU_SURFACES:
        seam = f"if idx == {row} {{ return {MENU_HANDLERS[row]}() }}"
        if seam not in menu:
            fail(f"menu_pick no longer routes tested {name} row {row} through "
                 f"{MENU_HANDLERS[row]}")
    return routes


def run(receipt_path, no_build, timeout):
    manifest = json.load(open(MANIFEST, encoding="utf-8"))
    identities = {entry["id"]: entry["name"] for entry in manifest["entries"]}
    source_routes = source_register_routes()
    expected_routes = [(slot, app) for slot, app, _ in REGISTER]
    if source_routes != expected_routes:
        fail(f"register route/source drift: source={source_routes} probe={expected_routes}")
    for _, app, name in REGISTER:
        if identities.get(app) != name:
            fail(f"register route identity drift: {app} is {identities.get(app)!r}, expected {name!r}")
    for _, app, name in WORDS:
        if identities.get(app) != name:
            fail(f"word route identity drift: {app} is {identities.get(app)!r}, expected {name!r}")

    for app, name in ((2, "About"), (4, "Menu"), (5, "Browser"),
                      (70, "All Applications"), (71, "System"), (72, "Type")):
        if identities.get(app) != name:
            fail(f"surface route identity drift: {app} is {identities.get(app)!r}, expected {name!r}")

    result = {"boot": [], "register": [], "shell_word": [], "surface": []}
    with zb.Machine(W, H, do_build=not no_build, boot_timeout=timeout,
                    how="native") as machine:
        geometry = zb.rail_geometry(machine.w, machine.h)
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

        for slot, app, name in REGISTER:
            cycle = register_cycle(machine, geometry["slots"][slot], app, timeout)
            result["register"].append({"slot": slot, "id": app, "name": name, **cycle})
            print(f"[register {slot:02d}] {name:<20} open-ready-close PASS")
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

        menu = super_cycle(machine, 4, timeout)
        result["surface"].append({"route": "super", "id": 4, "name": "Menu", **menu})
        print("[super] Menu                 open-ready-close PASS")
        for row, app, name in MENU_SURFACES:
            cycle = menu_row_cycle(machine, row, app, timeout)
            result["surface"].append({"route": f"menu:{row}", "id": app,
                                      "name": name, **cycle})
            print(f"[menu {row:02d}] {name:<20} open-ready-close PASS")
        catalogue = simple_pointer_cycle(machine, geometry["catalogue"], 70, timeout)
        result["surface"].append({"route": "rail:all-apps", "id": 70,
                                  "name": "All Applications", **catalogue})
        print(f"[rail {len(REGISTER):02d}] All Applications  open-ready-close PASS")

    iso = os.path.join(KERNEL_ROOT, "zlOS.iso")
    receipt = {
        "schema": "zlos.application-route-qemu-receipt.v2",
        "evidence": "QEMU input/compositor routes; not workflow or physical proof",
        "source_head": subprocess.check_output(
            ["git", "rev-parse", "HEAD"], cwd=REPO_ROOT, text=True).strip(),
        "source_files_sha256": {
            os.path.relpath(path, REPO_ROOT): sha256(path)
            for path in (
                os.path.join(KERNEL_ROOT, "src", "kernel.zl"),
                os.path.join(KERNEL_ROOT, "src", "graphics", "windowing", "wm.c"),
                MANIFEST,
                os.path.join(METADATA, "build-identity.json"),
                os.path.join(KERNEL_ROOT, "build_identity_embed.zl"),
                os.path.join(HERE, "probe-app-routes.py"),
                os.path.join(HERE, "probe-app-lifecycle.py"),
                os.path.join(KERNEL_ROOT, "tests", "oracle", "zlosboot.py"),
            )
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
        "external_prerequisite": "APP_RUN id 7 route is covered here; probe-run.py owns its deeper execution workflow",
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
