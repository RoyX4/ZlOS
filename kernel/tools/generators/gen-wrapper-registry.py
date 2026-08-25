#!/usr/bin/env python3
"""Inventory every repository command wrapper and its landing-authority status."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import subprocess
import tempfile
from pathlib import Path


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
ROOT = KERNEL_ROOT.parent
METADATA = KERNEL_ROOT / "metadata"
OUTPUT = METADATA / "wrapper-registry.json"
LAND_GATE = ROOT / "gates/land-gate.sh"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def repository_paths() -> list[Path]:
    completed = subprocess.run(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"],
        cwd=ROOT, stdout=subprocess.PIPE, check=True,
    )
    paths = []
    for raw in completed.stdout.split(b"\0"):
        if not raw:
            continue
        relative = raw.decode("utf-8", errors="strict")
        path = ROOT / relative
        if path.is_file():
            paths.append(path)
    return sorted(paths)


def language(first: str) -> str:
    if "python" in first:
        return "python"
    if "node" in first:
        return "node"
    if "sh" in first or "bash" in first:
        return "shell"
    return "other"


def role(relative: str) -> str:
    name = Path(relative).name
    if relative in {"gates/land-gate.sh", "gates/run-land-gate-contained.sh"}:
        return "LANDING_AUTHORITY"
    if name.startswith(("check-", "verify-")) or name in {"run_tests.sh", "preflight.sh"}:
        return "VERIFIER"
    if name.startswith(("build", "gen-", "mkiso", "mkdisk", "mkusb", "buildefi")):
        return "BUILD_OR_GENERATOR"
    if name.startswith(("probe-", "shot-", "score-", "diff-")):
        return "PROBE_OR_INSTRUMENT"
    if name.startswith(("install", "write-", "run-")):
        return "ACTION_OR_LAUNCHER"
    return "UTILITY_OR_LEGACY"


def exit_policy(text: str, lang: str) -> str:
    if lang == "shell":
        if "exit $FAIL" in text or "exit \"$FAIL\"" in text or "exit $fail" in text or "exit \"$fail\"" in text:
            return "AGGREGATED_EXPLICIT_EXIT"
        if "set -e" in text[:1000] or "set -eu" in text[:1000]:
            return "SHELL_ERREXIT"
        if "|| exit" in text or "return 1" in text or "exit 1" in text:
            return "MANUAL_EXPLICIT_FAILURES"
        return "NO_STATIC_FAILURE_POLICY"
    if lang == "python":
        if "raise SystemExit(main())" in text:
            return "PYTHON_MAIN_EXIT"
        if "sys.exit(" in text or "raise SystemExit(" in text:
            return "PYTHON_EXPLICIT_EXIT"
        return "PYTHON_UNPROVED_TOP_LEVEL"
    if lang == "node":
        return "NODE_TOP_LEVEL"
    return "UNKNOWN"


def build() -> dict:
    gate = LAND_GATE.read_text(encoding="utf-8")
    rows = []
    for path in repository_paths():
        data = path.read_bytes()
        if not data.startswith(b"#!"):
            continue
        text = data.decode("utf-8", errors="strict")
        relative = path.relative_to(ROOT).as_posix()
        first = text.splitlines()[0]
        rows.append({
            "path": relative,
            "sha256": hashlib.sha256(data).hexdigest(),
            "bytes": len(data),
            "executable": bool(path.stat().st_mode & 0o111),
            "interpreter": first[2:].strip(),
            "language": language(first),
            "role": role(relative),
            "exit_policy": exit_policy(text, language(first)),
            "named_by_landing_gate": Path(relative).name in gate or relative in gate,
        })
    counts = {
        "wrappers": len(rows),
        "executable": sum(row["executable"] for row in rows),
        "named_by_landing_gate": sum(row["named_by_landing_gate"] for row in rows),
        "no_static_failure_policy": sum(row["exit_policy"] in {"NO_STATIC_FAILURE_POLICY", "PYTHON_UNPROVED_TOP_LEVEL", "UNKNOWN"} for row in rows),
    }
    return {
        "schema": "zlos.wrapper-registry.v1",
        "result": "PASS_INVENTORY_WITH_LEGACY_POLICY_GAPS",
        "build_identity": json.loads((METADATA / "build-identity.json").read_text())["identity_sha256"],
        "landing_gate_sha256": sha256(LAND_GATE),
        "wrappers": rows,
        "counts": counts,
        "authority_contract": {
            "only_supported_landing_entry": "gates/run-land-gate-contained.sh start",
            "landing_aggregator": "gates/land-gate.sh",
            "child_failures_captured": "run() records each child exit and final process exits with FAIL count",
            "legacy_policy_gaps_are_not_landing_authority": True,
        },
        "evidence_ceiling": "complete current shebang-wrapper inventory plus static policy classification; dynamic child-exit proof belongs to check-land-gate.py",
        "weakest_link": "legacy/action/probe wrappers can retain manual or unproved policies but cannot become landing authorities without an explicit gate mutation",
        "generator": {"path": "kernel/gen-wrapper-registry.py", "sha256": sha256(Path(__file__).resolve())},
    }


def validate(value: dict) -> None:
    if value.get("schema") != "zlos.wrapper-registry.v1" or value.get("result") != "PASS_INVENTORY_WITH_LEGACY_POLICY_GAPS":
        raise ValueError("wrong wrapper-registry schema/result")
    current = build()
    rows = value.get("wrappers", [])
    if [row.get("path") for row in rows] != [row["path"] for row in current["wrappers"]]:
        raise ValueError("wrapper inventory is missing, extra or reordered")
    if len({row.get("path") for row in rows}) != len(rows):
        raise ValueError("duplicate wrapper path")
    if any(len(row.get("sha256", "")) != 64 or not row.get("exit_policy") for row in rows):
        raise ValueError("wrapper identity/policy missing")
    if value.get("counts") != current["counts"]:
        raise ValueError("wrapper counts drift")
    authority = value.get("authority_contract", {})
    if authority.get("only_supported_landing_entry") != "gates/run-land-gate-contained.sh start" \
            or authority.get("legacy_policy_gaps_are_not_landing_authority") is not True:
        raise ValueError("landing authority boundary was hidden")
    if len(value.get("landing_gate_sha256", "")) != 64 or len(value.get("build_identity", "")) != 64:
        raise ValueError("wrapper registry lacks build/gate identity")


def selftest(value: dict) -> None:
    mutations = {}
    missing = copy.deepcopy(value); missing["wrappers"].pop(); mutations["missing-wrapper"] = missing
    duplicate = copy.deepcopy(value); duplicate["wrappers"].append(copy.deepcopy(duplicate["wrappers"][0])); mutations["duplicate-wrapper"] = duplicate
    policy = copy.deepcopy(value); policy["wrappers"][0]["exit_policy"] = ""; mutations["missing-policy"] = policy
    authority = copy.deepcopy(value); authority["authority_contract"]["legacy_policy_gaps_are_not_landing_authority"] = False; mutations["hidden-authority-boundary"] = authority
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except ValueError:
            caught.append(name)
        else:
            raise ValueError(f"wrapper-registry selftest mutation escaped: {name}")
    print("wrapper-registry selftest: caught " + ", ".join(caught))


def write_atomic(value: dict) -> None:
    with tempfile.NamedTemporaryFile("w", dir=HERE, delete=False, encoding="utf-8") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, OUTPUT)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    if not args.write and not args.check:
        args.check = True
    try:
        value = build()
        if args.selftest:
            selftest(value)
        if args.write:
            write_atomic(value)
        if args.check and (not OUTPUT.is_file() or json.loads(OUTPUT.read_text()) != value):
            raise ValueError("wrapper-registry.json is missing or stale")
        print(f"wrapper-registry: PASS_INVENTORY_WITH_LEGACY_POLICY_GAPS: {value['counts']['wrappers']} wrappers, {value['counts']['named_by_landing_gate']} named by landing gate")
        return 0
    except (OSError, ValueError, json.JSONDecodeError, subprocess.SubprocessError) as error:
        print(f"wrapper-registry: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
