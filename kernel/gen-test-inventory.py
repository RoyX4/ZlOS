#!/usr/bin/env python3
"""Generate and verify the complete host-test execution inventory."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import re
import sys


HERE = os.path.dirname(os.path.abspath(__file__))
HOST = os.path.join(HERE, "hosttest")
BUILD = os.path.join(HOST, "build.sh")
POLICY = os.path.join(HOST, "test-policy.json")
OUTPUT = os.path.join(HOST, "test-inventory.json")
KINDS = {"gate", "hardware-gate", "instrument", "manual-hardware",
         "optional-instrument", "builder"}
AUTO_KINDS = {"gate", "hardware-gate"}
OUTPUT_RE = re.compile(r"(?:^|\s)-o\s+([A-Za-z0-9_.-]+)(?:\s|$)")


def fail(message):
    raise ValueError(message)


def read(path):
    with open(path, encoding="utf-8") as handle:
        return handle.read()


def load(path):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def compiled_targets(build_text=None):
    text = read(BUILD) if build_text is None else build_text
    text = re.sub(r"\\\n\s*", " ", text)
    result = []
    for line in text.splitlines():
        if not re.match(r"^\s*gcc(?:\s|$)", line):
            continue
        match = OUTPUT_RE.search(line)
        if match:
            result.append(match.group(1))
    if len(result) != len(set(result)):
        fail("hosttest/build.sh compiles a target more than once")
    return sorted(result)


def executable_scripts():
    result = []
    for name in os.listdir(HOST):
        path = os.path.join(HOST, name)
        if name.endswith(".sh") and os.path.isfile(path) and os.access(path, os.X_OK):
            result.append(name)
    return sorted(result)


def flatten(section, label):
    result = {}
    if not isinstance(section, dict):
        fail(f"{label} policy is not an object")
    for kind, names in section.items():
        if kind not in KINDS:
            fail(f"{label} has unknown kind {kind}")
        if not isinstance(names, list) or not names:
            fail(f"{label}.{kind} must be a non-empty list")
        for name in names:
            if not isinstance(name, str) or not name:
                fail(f"{label}.{kind} has an invalid name")
            if name in result:
                fail(f"{label} classifies {name} more than once")
            result[name] = kind
    return result


def validate_policy(policy, compiled=None, scripts=None):
    if policy.get("schema") != "zlos.host-test-policy.v1":
        fail("wrong host-test policy schema")
    compiled = compiled_targets() if compiled is None else sorted(compiled)
    scripts = executable_scripts() if scripts is None else sorted(scripts)
    declared_compiled = flatten(policy.get("compiled"), "compiled")
    declared_scripts = flatten(policy.get("scripts"), "scripts")
    if set(declared_compiled) != set(compiled):
        fail("compiled policy drift: missing=" +
             repr(sorted(set(compiled) - set(declared_compiled))) +
             " extra=" + repr(sorted(set(declared_compiled) - set(compiled))))
    if set(declared_scripts) != set(scripts):
        fail("script policy drift: missing=" +
             repr(sorted(set(scripts) - set(declared_scripts))) +
             " extra=" + repr(sorted(set(declared_scripts) - set(scripts))))
    known = set(declared_compiled) | set(declared_scripts)
    commands = policy.get("commands", {})
    if not isinstance(commands, dict) or not set(commands).issubset(known):
        fail("command overrides name an unknown target")
    for name, runs in commands.items():
        if (not isinstance(runs, list) or not runs or
                any(not isinstance(run, list) or not run or
                    any(not isinstance(arg, str) or not arg for arg in run)
                    for run in runs)):
            fail(f"invalid command override for {name}")
        if any(run[0] != "./" + name for run in runs):
            fail(f"command override for {name} runs a different target")
    optional = policy.get("optional_build", [])
    if not isinstance(optional, list) or not set(optional).issubset(declared_compiled):
        fail("optional_build names an unknown compiled target")
    overrides = policy.get("source_overrides", {})
    if not isinstance(overrides, dict) or not set(overrides).issubset(declared_compiled):
        fail("source_overrides names an unknown compiled target")
    timeout = policy.get("default_timeout_seconds")
    if not isinstance(timeout, int) or timeout < 1 or timeout > 3600:
        fail("invalid default timeout")
    return declared_compiled, declared_scripts


def source_for(name, policy, is_script):
    if is_script:
        return name
    return policy.get("source_overrides", {}).get(name, name + ".c")


def generate(policy):
    compiled, scripts = validate_policy(policy)
    rows = []
    for name, kind, is_script in sorted(
            [(name, kind, False) for name, kind in compiled.items()] +
            [(name, kind, True) for name, kind in scripts.items()]):
        source = source_for(name, policy, is_script)
        source_path = os.path.join(HOST, source)
        source_present = os.path.isfile(source_path)
        optional = name in policy.get("optional_build", [])
        if not source_present and not optional:
            fail(f"{name} declares missing source {source}")
        commands = policy.get("commands", {}).get(name, [["./" + name]])
        auto = kind in AUTO_KINDS
        allowed = [0, 77] if kind == "hardware-gate" else [0]
        rows.append({
            "name": name,
            "kind": kind,
            "form": "script" if is_script else "compiled",
            "source": source,
            "source_present": source_present,
            "source_sha256": sha256(source_path) if source_present else None,
            "optional_build": optional,
            "auto_run": auto,
            "commands": commands,
            "allowed_exit_codes": allowed,
            "timeout_seconds": policy["default_timeout_seconds"],
            "evidence_ceiling": (
                "manual physical-hardware action" if kind == "manual-hardware" else
                "measurement/artifact only; not a pass" if "instrument" in kind else
                "host behavior; exit 77 is an explicit hardware absence" if kind == "hardware-gate" else
                "host behavior only"
            ),
        })
    for index, row in enumerate(rows, 1):
        row["id"] = f"HT-{index:03d}"
    value = {
        "schema": "zlos.host-test-inventory.v1",
        "policy_sha256": sha256(POLICY),
        "build_script_sha256": sha256(BUILD),
        "counts": {
            "compiled": len(compiled),
            "scripts": len(scripts),
            "total": len(rows),
            "auto_run_targets": sum(row["auto_run"] for row in rows),
            "manual_or_instrument_targets": sum(not row["auto_run"] for row in rows),
            "auto_run_commands": sum(len(row["commands"]) for row in rows if row["auto_run"]),
        },
        "targets": rows,
    }
    validate_inventory(value, policy)
    return value


def validate_inventory(value, policy):
    if value.get("schema") != "zlos.host-test-inventory.v1":
        fail("wrong host-test inventory schema")
    expected = generate_without_validation(policy)
    if value != expected:
        fail("host-test inventory is stale")


def generate_without_validation(policy):
    original = globals()["validate_inventory"]
    try:
        globals()["validate_inventory"] = lambda value, policy: None
        return generate(policy)
    finally:
        globals()["validate_inventory"] = original


def selftest(policy):
    caught = []
    mutations = (
        ("missing-compiled", lambda p: p["compiled"]["gate"].pop()),
        ("duplicate-target", lambda p: p["compiled"]["instrument"].append(
            p["compiled"]["gate"][0])),
        ("unknown-kind", lambda p: p["compiled"].__setitem__("maybe", ["fake"])),
        ("unclassified-script", lambda p: p["scripts"]["gate"].pop()),
        ("foreign-command", lambda p: p["commands"].__setitem__(
            p["compiled"]["gate"][0], [["./different-target"]])),
    )
    for name, mutate in mutations:
        mutant = copy.deepcopy(policy)
        mutate(mutant)
        try:
            validate_policy(mutant)
        except ValueError:
            caught.append(name)
        else:
            fail(f"selftest mutation escaped: {name}")
    print("test-inventory selftest: caught " + ", ".join(caught))


def serialized(value):
    return json.dumps(value, indent=2, sort_keys=True) + "\n"


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args(argv)
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    try:
        policy = load(POLICY)
        value = generate(policy)
        if args.write:
            temporary = OUTPUT + ".tmp"
            with open(temporary, "w", encoding="utf-8") as handle:
                handle.write(serialized(value))
            os.replace(temporary, OUTPUT)
        else:
            actual = load(OUTPUT)
            if actual != value:
                fail("test-inventory.json is stale; run --write")
        if args.selftest:
            selftest(policy)
        counts = value["counts"]
        print("test-inventory: PASS: "
              f"{counts['compiled']} compiled + {counts['scripts']} scripts; "
              f"{counts['auto_run_commands']} automatic commands")
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"test-inventory: FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
