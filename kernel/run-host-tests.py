#!/usr/bin/env python3
"""Run every automatic host gate from the generated inventory, fail closed."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
import platform
import subprocess
import sys
import time


HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
HOST = os.path.join(HERE, "hosttest")
INVENTORY = os.path.join(HOST, "test-inventory.json")
DEFAULT_RECEIPT = os.path.join(HOST, "test-run-receipt.json")


def fail(message):
    raise ValueError(message)


def load(path):
    with open(path, encoding="utf-8") as handle:
        return json.load(handle)


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def command(argv, cwd=ROOT):
    return subprocess.check_output(argv, cwd=cwd, text=True,
                                   stderr=subprocess.STDOUT).strip()


def tail(text, lines=20):
    return "\n".join(text.rstrip().splitlines()[-lines:])


def execute(row):
    result = {
        "id": row["id"],
        "name": row["name"],
        "kind": row["kind"],
        "auto_run": row["auto_run"],
        "commands": [],
    }
    target_path = os.path.join(HOST, row["name"])
    if not row["auto_run"]:
        result["status"] = "not-run"
        result["reason"] = row["evidence_ceiling"]
        return result
    if not os.path.isfile(target_path) or not os.access(target_path, os.X_OK):
        result["status"] = "unavailable" if row["optional_build"] else "failed"
        result["reason"] = "optional build unavailable" if row["optional_build"] else "executable missing"
        return result
    result["executable_sha256"] = sha256(target_path)
    exits = []
    for argv in row["commands"]:
        started = time.monotonic_ns()
        try:
            completed = subprocess.run(argv, cwd=HOST, text=True,
                                       stdout=subprocess.PIPE,
                                       stderr=subprocess.STDOUT,
                                       timeout=row["timeout_seconds"], check=False)
            output = completed.stdout
            code = completed.returncode
            timed_out = False
        except subprocess.TimeoutExpired as error:
            output = (error.stdout or "") + (error.stderr or "")
            if isinstance(output, bytes):
                output = output.decode(errors="replace")
            code = 124
            timed_out = True
        elapsed = time.monotonic_ns() - started
        exits.append(code)
        result["commands"].append({
            "argv": argv,
            "exit_code": code,
            "timed_out": timed_out,
            "elapsed_ns": elapsed,
            "output_bytes": len(output.encode()),
            "output_sha256": hashlib.sha256(output.encode()).hexdigest(),
            "output_tail": tail(output),
        })
    if any(code not in row["allowed_exit_codes"] for code in exits):
        result["status"] = "failed"
        result["reason"] = f"exit codes {exits}, allowed {row['allowed_exit_codes']}"
    elif all(code == 0 for code in exits):
        result["status"] = "passed"
    elif row["kind"] == "hardware-gate" and all(code == 77 for code in exits):
        result["status"] = "skipped-hardware"
        result["reason"] = "target reported hardware/device unavailable (77)"
    else:
        result["status"] = "failed"
        result["reason"] = f"inconsistent partial execution exits {exits}"
    return result


def summary(results):
    statuses = ("passed", "failed", "skipped-hardware", "unavailable", "not-run")
    counts = {status: sum(row["status"] == status for row in results)
              for status in statuses}
    counts["targets"] = len(results)
    counts["commands_executed"] = sum(len(row["commands"]) for row in results)
    return counts


def validate_receipt(receipt, inventory):
    if receipt.get("schema") != "zlos.host-test-run-receipt.v1":
        fail("wrong host-test receipt schema")
    if receipt.get("inventory_sha256") != sha256(INVENTORY):
        fail("receipt does not name the current inventory")
    results = receipt.get("results")
    targets = inventory.get("targets", [])
    if not isinstance(results, list) or len(results) != len(targets):
        fail("receipt target count mismatch")
    got = [(row.get("id"), row.get("name")) for row in results]
    wanted = [(row["id"], row["name"]) for row in targets]
    if got != wanted or len(got) != len(set(got)):
        fail("receipt target identity/order mismatch")
    by_name = {row["name"]: row for row in targets}
    for result in results:
        target = by_name[result["name"]]
        status = result.get("status")
        runs = result.get("commands")
        if not isinstance(runs, list):
            fail(f"{result['name']}: commands are missing")
        if not target["auto_run"]:
            if status != "not-run" or runs:
                fail(f"{result['name']}: manual/instrument target was promoted")
            continue
        if status == "unavailable":
            if not target["optional_build"] or runs:
                fail(f"{result['name']}: non-optional target is unavailable")
            continue
        if len(runs) != len(target["commands"]):
            fail(f"{result['name']}: command count mismatch")
        exits = [run.get("exit_code") for run in runs]
        if any(run.get("timed_out") for run in runs):
            expected = "failed"
        elif any(code not in target["allowed_exit_codes"] for code in exits):
            expected = "failed"
        elif all(code == 0 for code in exits):
            expected = "passed"
        elif target["kind"] == "hardware-gate" and all(code == 77 for code in exits):
            expected = "skipped-hardware"
        else:
            expected = "failed"
        if status != expected:
            fail(f"{result['name']}: status {status!r}, expected {expected!r}")
        if status in ("passed", "skipped-hardware") and not result.get("executable_sha256"):
            fail(f"{result['name']}: execution lacks binary identity")
    if receipt.get("counts") != summary(results):
        fail("receipt summary does not match results")
    expected_outcome = "PASS" if receipt["counts"]["failed"] == 0 else "FAIL"
    if receipt.get("outcome") != expected_outcome:
        fail("receipt outcome does not match failures")


def fake_receipt(inventory):
    results = []
    for row in inventory["targets"]:
        result = {"id": row["id"], "name": row["name"], "kind": row["kind"],
                  "auto_run": row["auto_run"], "commands": []}
        if row["auto_run"]:
            result["commands"] = [{"argv": argv, "exit_code": 0,
                                    "timed_out": False} for argv in row["commands"]]
            result["executable_sha256"] = "0" * 64
            result["status"] = "passed"
        else:
            result["status"] = "not-run"
        results.append(result)
    value = {"schema": "zlos.host-test-run-receipt.v1",
             "inventory_sha256": sha256(INVENTORY), "results": results}
    value["counts"] = summary(results)
    value["outcome"] = "PASS"
    return value


def selftest(inventory):
    base = fake_receipt(inventory)
    validate_receipt(base, inventory)
    caught = []
    mutations = (
        ("missing-target", lambda r: r["results"].pop()),
        ("promoted-instrument", lambda r: next(x for x in r["results"]
                                                if not x["auto_run"]).__setitem__("status", "passed")),
        ("exit-77-as-pass", lambda r: next(x for x in r["results"]
                                            if x["kind"] == "hardware-gate")["commands"][0].__setitem__("exit_code", 77)),
        ("hidden-failure", lambda r: next(x for x in r["results"]
                                           if x["kind"] == "gate")["commands"][0].__setitem__("exit_code", 1)),
        ("duplicate-target", lambda r: r["results"].__setitem__(1, copy.deepcopy(r["results"][0]))),
    )
    for name, mutate in mutations:
        mutant = copy.deepcopy(base)
        mutate(mutant)
        try:
            validate_receipt(mutant, inventory)
        except ValueError:
            caught.append(name)
        else:
            fail(f"runner selftest mutation escaped: {name}")
    print("host-test runner selftest: caught " + ", ".join(caught))


def run_all(inventory, receipt_path):
    subprocess.run([sys.executable, "gen-test-inventory.py", "--check"],
                   cwd=HERE, check=True)
    results = []
    for row in inventory["targets"]:
        result = execute(row)
        results.append(result)
        label = result["status"].upper()
        print(f"{row['id']} {row['name']:<22} {label}")
        if result["status"] == "failed" and result["commands"]:
            print(result["commands"][-1]["output_tail"])
    value = {
        "schema": "zlos.host-test-run-receipt.v1",
        "inventory_sha256": sha256(INVENTORY),
        "git": {
            "head": command(["git", "rev-parse", "HEAD"]),
            "branch": command(["git", "branch", "--show-current"]),
            "dirty": bool(command(["git", "status", "--porcelain=v1", "--untracked-files=all"])),
        },
        "host": {"platform": platform.platform(), "python": platform.python_version()},
        "results": results,
        "evidence_ceiling": "host execution only; instruments/manual hardware are explicit non-runs and exit 77 is not a pass",
    }
    value["counts"] = summary(results)
    value["outcome"] = "PASS" if value["counts"]["failed"] == 0 else "FAIL"
    validate_receipt(value, inventory)
    temporary = receipt_path + ".tmp"
    with open(temporary, "w", encoding="utf-8") as handle:
        json.dump(value, handle, indent=2, sort_keys=True)
        handle.write("\n")
    os.replace(temporary, receipt_path)
    print("host-test runner: " + value["outcome"] + ": " +
          json.dumps(value["counts"], sort_keys=True))
    return 0 if value["outcome"] == "PASS" else 1


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", action="store_true")
    parser.add_argument("--selftest", action="store_true")
    parser.add_argument("--receipt", default=DEFAULT_RECEIPT)
    args = parser.parse_args(argv)
    if not args.run and not args.selftest:
        parser.error("choose --run, --selftest, or both")
    try:
        inventory = load(INVENTORY)
        if args.selftest:
            selftest(inventory)
        if args.run:
            return run_all(inventory, args.receipt)
    except (OSError, ValueError, json.JSONDecodeError,
            subprocess.CalledProcessError) as error:
        print(f"host-test runner: FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
