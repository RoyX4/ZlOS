#!/usr/bin/env python3
"""Write exact native-UEFI64 user-process boundary evidence from a boot log."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile


HERE = Path(__file__).resolve().parent
KERNEL_ROOT = HERE.parents[1]
IDENTITY = KERNEL_ROOT / "metadata/build-identity.json"
ARTIFACT = KERNEL_ROOT / "zlOS-usb.img"
HARNESS = KERNEL_ROOT / "tools/checks/verify-efi.sh"
USERMODE = KERNEL_ROOT / "src/arch/x86/usermode.c"
PROCESS_LIFECYCLE_HEADER = KERNEL_ROOT / "src/core/process_lifecycle.h"
PROCESS_LIFECYCLE = KERNEL_ROOT / "src/core/process_lifecycle.c"
PROCESS_LIFECYCLE_TEST = KERNEL_ROOT / "tests/host/processlifecycletest.c"
SCHEDULER_POLICY_HEADER = KERNEL_ROOT / "src/core/scheduler_policy.h"
SCHEDULER_POLICY = KERNEL_ROOT / "src/core/scheduler_policy.c"
SCHEDULER_POLICY_TEST = KERNEL_ROOT / "tests/host/schedulerpolicytest.c"
PROCESS_SERVICE_HEADER = KERNEL_ROOT / "src/core/user_process_service.h"
PROCESS_SERVICE = KERNEL_ROOT / "src/core/user_process_service.c"
PROCESS_SERVICE_TEST = KERNEL_ROOT / "tests/host/userprocessservicetest.c"
PROCESS_MEMORY_HEADER = KERNEL_ROOT / "src/core/process_memory.h"
PROCESS_MEMORY = KERNEL_ROOT / "src/core/process_memory.c"
PROCESS_MEMORY_TEST = KERNEL_ROOT / "tests/host/processmemorytest.c"
PMM_HEADER = KERNEL_ROOT / "src/core/pmm.h"
PMM = KERNEL_ROOT / "src/core/pmm.c"
PMM_TEST = KERNEL_ROOT / "tests/host/pmmtest.c"
ANON_MEMORY_HEADER = KERNEL_ROOT / "src/core/anon_memory.h"
ANON_MEMORY = KERNEL_ROOT / "src/core/anon_memory.c"
ANON_MEMORY_TEST = KERNEL_ROOT / "tests/host/anonmemorytest.c"
HOST_RECEIPT = KERNEL_ROOT / "tests/host/test-run-receipt.json"
SYSCALL_SPEC = KERNEL_ROOT / "src/arch/x86/user_syscalls.json"
SYSCALL_HEADER = KERNEL_ROOT / "src/arch/x86/user_syscalls_generated.h"
IDT = KERNEL_ROOT / "src/arch/x86/idt.c"
GDT = KERNEL_ROOT / "boot/gdt64.c"
ABI_DOC = KERNEL_ROOT / "docs/architecture/system/user-process-abi.md"
DEFAULT_OUTPUT = KERNEL_ROOT / "docs/receipts/user-process-native-uefi64-qemu-2026-08-29.json"

ASSERTIONS = (
    {
        "id": "ring3-syscall-lifecycle",
        "marker": "ring 3 64: u1 <- iretq/int80/iretq, 6 syscalls, process exited, kernel alive",
        "processes": 1,
        "syscalls": 6,
        "entry": "iretq",
        "syscall_gate": "int 0x80",
        "returned_to_kernel": True,
    },
    {
        "id": "unknown-syscall-admission",
        "marker": "syscall ABI: zero/gap/sign-bit/max refused with ENOSYS",
        "abi_version": 1,
        "probes": [0, 25, 1 << 63, (1 << 64) - 1],
        "result": "ENOSYS",
    },
    {
        "id": "generation-safe-process-identity",
        "marker": "<- process lifecycle generation reuse rejected the stale handle and retained exact exit custody",
        "handle_fields": ["slot", "generation"],
        "stale_handle_refused": True,
        "exit_status_retained": True,
        "observed_exit_status": -7,
    },
    {
        "id": "anonymous-reserve-commit-release",
        "marker": "anonymous memory: M <- reserve/commit zero-fill, cross-page copy and release passed",
        "pages": 2,
        "reserved_has_frame": False,
        "commit_zero_filled": True,
        "cross_page_copy_admitted": True,
        "released_copy_refused": True,
    },
    {
        "id": "reserved-anonymous-page-fault",
        "marker": "<- reserved anonymous page stayed absent; sibling V exited",
        "offender_vector": 14,
        "page_fault_error": 0x4,
        "fault_address": "anonymous_base",
        "reserved_pte": "absent",
        "sibling_trace": "V",
        "sibling_exited": True,
    },
    {
        "id": "released-anonymous-page-fault",
        "marker": "<- released anonymous page faulted exactly; sibling R exited",
        "offender_vector": 14,
        "page_fault_error": 0x4,
        "fault_address": "anonymous_base",
        "released_pte": "absent",
        "sibling_trace": "R",
        "sibling_exited": True,
        "frame_reclaimed": True,
    },
    {
        "id": "privilege-and-user-copy-boundary",
        "marker": "ring 3 hostile: cli GP, kernel/device PF, crossing pointer refused; kernel alive",
        "privileged_instruction_vector": 13,
        "kernel_mapping_vector": 14,
        "device_mapping_vector": 14,
        "crossing_pointer_refused_before_dereference": True,
        "kernel_survived": True,
    },
    {
        "id": "separate-address-space-resume",
        "marker": "<- two PML4 processes yielded/resumed AB12 and exited independently",
        "processes": 2,
        "trace": "AB12",
        "separate_pml4_roots": True,
        "separate_user_stacks": True,
        "separate_kernel_stacks": True,
    },
    {
        "id": "offender-only-fault-containment",
        "marker": "<- one process GP-faulted; its sibling ran and exited",
        "offender_vector": 13,
        "sibling_trace": "K",
        "sibling_exited": True,
    },
    {
        "id": "fault-and-exit-custody",
        "marker": "<- process lifecycle retained exact GP-fault custody and independent sibling exit",
        "fault_fields": ["vector", "error", "address"],
        "fault_vector": 13,
        "fault_error": 0,
        "fault_address": 0,
        "fault_exit_distinct": True,
        "sibling_exit_retained": True,
        "sibling_exit_status": 7,
    },
    {
        "id": "lower-stack-guard-fault-containment",
        "marker": "<- lower stack guard PF error 6 at exact address; sibling G exited",
        "offender_vector": 14,
        "page_fault_error": 0x6,
        "fault_address": "user_base+4096+2048",
        "guard_pte": "absent",
        "sibling_trace": "G",
        "sibling_exited": True,
        "kernel_survived": True,
    },
    {
        "id": "guarded-supervisor-tss-stacks",
        "marker": "<- two guarded supervisor TSS stacks bounded through syscall/preempt/fault paths",
        "processes": 2,
        "pages_per_stack": 2,
        "lower_guard_pte": "absent",
        "leaf_permissions": "supervisor-read-write-no-execute",
        "tss_rsp0_selected": True,
        "paths": ["syscall", "timer-preemption", "fault"],
        "high_water_limit_bytes": 8192,
    },
    {
        "id": "process-memory-accounting",
        "marker": "<- process memory accounting: fixed/anonymous quotas and owner totals passed",
        "fixed_owner_count": 2,
        "fixed_limit_pages_per_owner": 16,
        "anonymous_owner_count": 2,
        "anonymous_limit_pages_per_owner": 32,
        "owner_totals_after_release": 0,
        "owner_high_water_bounded": True,
        "owner_refusals": 0,
        "metadata_owner_sum_invariant": True,
    },
    {
        "id": "process-frame-reclamation",
        "marker": "<- process-owned page tables/code/stacks reclaimed; PMM baseline restored",
        "frames_per_process": 8,
        "owned_resources": ["PML4", "PDPT", "PD", "PT", "code", "user-stack",
                            "kernel-stack-low", "kernel-stack-high"],
        "failure_atomic_acquire": True,
        "two_processes_disjoint": True,
        "baseline_restored": True,
    },
    {
        "id": "process-identity-reclamation",
        "marker": "<- process lifecycle slots reaped with generation history retained",
        "live_slots_after_reap": 0,
        "generation_history_retained": True,
        "resource_release_before_reap": True,
    },
    {
        "id": "persistent-process-service",
        "marker": "<- persistent service scheduled ST12 across four kernel turns; exact exit custody reaped",
        "processes": 2,
        "kernel_turns": 4,
        "trace": "ST12",
        "selection": "round-robin exact lifecycle handle",
        "turn_boundary": "yield or exit, with timer preemption armed",
        "exit_statuses": [11, 22],
        "terminal_custody_observed": True,
        "scheduler_detached_before_identity_reap": True,
        "physical_frame_baseline_restored": True,
    },
)

SOURCE_CONTRACTS = {
    "process_slots": 2,
    "process_identity": "generation-tagged slot handle",
    "process_id_reuse": "allowed only after reap; stale generation rejected",
    "generation_exhaustion": "slot permanently retired before wrap",
    "termination_record": "distinct signed exit status or exact fault vector/error/address",
    "reap_order": "resources first, identity last; parent identity retained while children exist",
    "persistent_service": "one bounded preemptible Ring-3 turn per kernel work-loop call",
    "scheduler_owner": "exact generation-tagged lifecycle handle",
    "scheduler_policy": "fixed-capacity round robin with one running owner",
    "scheduler_accounting": "64-bit saturating run, dispatch, switch and work counters",
    "scheduler_failure": "lifecycle-policy disagreement fail-stops subsequent work",
    "user_code": "read-execute",
    "user_stack": "read-write-no-execute",
    "lower_stack_guard_pte": "absent",
    "kernel_stack_pages": 2,
    "kernel_stack": "supervisor-read-write-no-execute",
    "lower_kernel_stack_guard_pte": "absent",
    "tss_rsp0": "process-private virtual kernel-stack top",
    "return_order": "switch to permanent kernel stack before restoring kernel CR3",
    "inherited_kernel_and_device_mappings": "supervisor-only",
    "physical_frame_owner": "one typed PMM owner per process slot",
    "physical_frames_per_process": 8,
    "physical_frame_limit_per_process_owner": 16,
    "anonymous_frame_limit_per_process_owner": 32,
    "replacement": "allocate complete successor before releasing predecessor",
    "reclamation": "preflight all frame owners before release",
    "copy_validation": "complete fixed or committed-anonymous range before first byte",
    "anonymous_window_pages": 32,
    "anonymous_first_pte": 6,
    "anonymous_states": ["free", "reserved", "committed", "broken"],
    "anonymous_commit": "allocate and zero all frames before atomic PTE publication",
    "anonymous_reclamation": "atomic unmap before owner-checked PMM release",
    "hardware_pte_bits": "accessed and dirty accepted without weakening ownership checks",
    "syscall_numbers": {
        "abi_version": 1,
        "first": 1,
        "last": 24,
        "dispatch": "generated unsigned admission",
        "unknown_result": "ENOSYS",
    },
}


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def validate_log(log: str) -> None:
    for assertion in ASSERTIONS:
        marker = assertion["marker"]
        if log.count(marker) != 1:
            raise ValueError(
                f"user-process marker count for {assertion['id']} is {log.count(marker)}, expected 1"
            )
    high_water = re.findall(
        r"^  kernel stacks high-water: P0 ([0-9]+) P1 ([0-9]+) bytes$",
        log, re.MULTILINE,
    )
    if len(high_water) != 1 or any(
            not 0 < int(value) < 8192 for value in high_water[0]):
        raise ValueError("guarded kernel-stack high-water observation is absent or unbounded")
    failures = (
        "64-bit Ring 3 proof missing or incomplete",
        "syscall ABI: unknown-number ENOSYS gate FAILED",
        "anonymous reserve/commit/release FAILED",
        "reserved anonymous page fault containment FAILED",
        "released anonymous page fault containment FAILED",
        "ring 3 hostile: FAILED",
        "multi-process yield/resume FAILED",
        "sibling fault isolation FAILED",
        "lower stack guard fault containment FAILED",
        "guarded supervisor TSS stack proof FAILED",
        "process lifecycle generation/exit custody FAILED",
        "process lifecycle fault/exit custody FAILED",
        "process lifecycle final teardown FAILED",
        "persistent user-process service FAILED",
        "process frame reclamation FAILED",
        "process memory accounting FAILED",
    )
    present = [marker for marker in failures if marker in log]
    if present:
        raise ValueError("user-process failure marker present: " + ", ".join(present))


def expected_files() -> list[dict]:
    return [
        {"path": "kernel/src/arch/x86/usermode.c", "sha256": digest(USERMODE)},
        {"path": "kernel/src/core/process_lifecycle.h", "sha256": digest(PROCESS_LIFECYCLE_HEADER)},
        {"path": "kernel/src/core/process_lifecycle.c", "sha256": digest(PROCESS_LIFECYCLE)},
        {"path": "kernel/tests/host/processlifecycletest.c", "sha256": digest(PROCESS_LIFECYCLE_TEST)},
        {"path": "kernel/src/core/scheduler_policy.h", "sha256": digest(SCHEDULER_POLICY_HEADER)},
        {"path": "kernel/src/core/scheduler_policy.c", "sha256": digest(SCHEDULER_POLICY)},
        {"path": "kernel/tests/host/schedulerpolicytest.c", "sha256": digest(SCHEDULER_POLICY_TEST)},
        {"path": "kernel/src/core/user_process_service.h", "sha256": digest(PROCESS_SERVICE_HEADER)},
        {"path": "kernel/src/core/user_process_service.c", "sha256": digest(PROCESS_SERVICE)},
        {"path": "kernel/tests/host/userprocessservicetest.c", "sha256": digest(PROCESS_SERVICE_TEST)},
        {"path": "kernel/src/core/process_memory.h", "sha256": digest(PROCESS_MEMORY_HEADER)},
        {"path": "kernel/src/core/process_memory.c", "sha256": digest(PROCESS_MEMORY)},
        {"path": "kernel/tests/host/processmemorytest.c", "sha256": digest(PROCESS_MEMORY_TEST)},
        {"path": "kernel/src/core/pmm.h", "sha256": digest(PMM_HEADER)},
        {"path": "kernel/src/core/pmm.c", "sha256": digest(PMM)},
        {"path": "kernel/tests/host/pmmtest.c", "sha256": digest(PMM_TEST)},
        {"path": "kernel/src/core/anon_memory.h", "sha256": digest(ANON_MEMORY_HEADER)},
        {"path": "kernel/src/core/anon_memory.c", "sha256": digest(ANON_MEMORY)},
        {"path": "kernel/tests/host/anonmemorytest.c", "sha256": digest(ANON_MEMORY_TEST)},
        {"path": "kernel/src/arch/x86/user_syscalls.json", "sha256": digest(SYSCALL_SPEC)},
        {"path": "kernel/src/arch/x86/user_syscalls_generated.h", "sha256": digest(SYSCALL_HEADER)},
        {"path": "kernel/src/arch/x86/idt.c", "sha256": digest(IDT)},
        {"path": "kernel/boot/gdt64.c", "sha256": digest(GDT)},
        {"path": "kernel/docs/architecture/system/user-process-abi.md", "sha256": digest(ABI_DOC)},
    ]


def host_observation() -> dict:
    receipt = json.loads(HOST_RECEIPT.read_text())
    identity = json.loads(IDENTITY.read_text())["identity_sha256"]
    if receipt.get("build_identity") != identity:
        raise ValueError("user-process host receipt is from a foreign build")
    observed = {}
    for target, minimum in (("processlifecycletest", 80),
                            ("schedulerpolicytest", 100),
                            ("userprocessservicetest", 100),
                            ("processmemorytest", 100),
                            ("anonmemorytest", 240)):
        rows = [row for row in receipt.get("results", [])
                if row.get("name") == target]
        if len(rows) != 1 or rows[0].get("status") != "passed":
            raise ValueError(f"current host receipt has no passing {target}")
        commands = rows[0].get("commands", [])
        if len(commands) != 1 or commands[0].get("exit_code") != 0 \
                or commands[0].get("timed_out"):
            raise ValueError(f"{target} command did not pass exactly once")
        match = re.search(rf"{target}: ([1-9][0-9]*) checks, 0 failures",
                          commands[0].get("output_tail", ""))
        if not match or int(match.group(1)) < minimum:
            raise ValueError(f"{target} lost its lifecycle observations")
        observed[target] = {"checks": int(match.group(1))}
    return {
        "path": "kernel/tests/host/test-run-receipt.json",
        "sha256": digest(HOST_RECEIPT),
        "targets": observed,
    }


def build(log_path: Path) -> dict:
    log = log_path.read_text(encoding="latin-1").replace("\r", "")
    validate_log(log)
    high_water_match = re.search(
        r"^  kernel stacks high-water: P0 ([0-9]+) P1 ([0-9]+) bytes$",
        log, re.MULTILINE,
    )
    assertions = [dict(assertion) for assertion in ASSERTIONS]
    guarded = next(assertion for assertion in assertions
                   if assertion["id"] == "guarded-supervisor-tss-stacks")
    guarded["observed_high_water_bytes"] = {
        "slot0": int(high_water_match.group(1)),
        "slot1": int(high_water_match.group(2)),
    }
    value = {
        "schema": "zlos.user-process-native-uefi64-qemu-receipt.v1",
        "result": "PASS_BOUNDED_USER_PROCESS_BOUNDARY",
        "build_identity": json.loads(IDENTITY.read_text())["identity_sha256"],
        "route": "native-uefi64",
        "artifact": {"path": "kernel/zlOS-usb.img", "sha256": digest(ARTIFACT)},
        "boot_log_sha256": digest(log_path),
        "harness": {
            "path": "kernel/tools/checks/verify-efi.sh",
            "sha256": digest(HARNESS),
        },
        "implementation": expected_files(),
        "host_receipt": host_observation(),
        "assertions": assertions,
        "source_contracts": copy.deepcopy(SOURCE_CONTRACTS),
        "known_gaps": [
            "persistent service is kernel-owned; there is no userspace spawn/wait syscall or process-handle ABI",
            "no SMEP or SMAP enablement receipt",
            "kernel-stack guards are selected and use-observed but not overflow-fault-injected",
            "the emergency IST stack has no guard page",
            "copy admission covers the fixed code/stack layout rather than arbitrary mapped regions",
            "anonymous allocation is a fixed 32-page window without a virtual-area allocator or demand paging",
            "owner accounting is not yet unified across services, caches, pinned memory, DMA or surfaces",
            "the typed PMM owner is not a per-allocation origin or call-site provenance ledger",
            "ABI version 1 has no compatibility, deprecation or migration tooling",
            "no current physical-hardware user-process receipt",
        ],
        "evidence_ceiling": (
            "exact current native-UEFI64 QEMU artifact plus source-bound fixed-layout, "
            "bounded persistent-service and kernel-stack-use contracts; not a userspace-managed general process API, stable compatibility promise, or "
            "physical-hardware qualification"
        ),
        "generator": {
            "path": "kernel/tools/checks/write-user-process-receipt.py",
            "sha256": digest(Path(__file__).resolve()),
        },
    }
    validate(value)
    return value


def validate(value: dict) -> None:
    identity = json.loads(IDENTITY.read_text())["identity_sha256"]
    if value.get("schema") != "zlos.user-process-native-uefi64-qemu-receipt.v1" \
            or value.get("result") != "PASS_BOUNDED_USER_PROCESS_BOUNDARY" \
            or value.get("route") != "native-uefi64":
        raise ValueError("wrong user-process receipt schema/result/route")
    if value.get("build_identity") != identity:
        raise ValueError("user-process receipt is not bound to the current build identity")
    if value.get("artifact") != {
            "path": "kernel/zlOS-usb.img", "sha256": digest(ARTIFACT)}:
        raise ValueError("user-process receipt does not bind the exact current USB artifact")
    if value.get("harness") != {
            "path": "kernel/tools/checks/verify-efi.sh", "sha256": digest(HARNESS)}:
        raise ValueError("user-process receipt does not bind the current verifier")
    if value.get("implementation") != expected_files():
        raise ValueError("user-process implementation identity drifted")
    if value.get("host_receipt") != host_observation():
        raise ValueError("user-process host lifecycle observation drifted")
    assertions = copy.deepcopy(value.get("assertions", []))
    if not assertions:
        raise ValueError("user-process assertion set is absent")
    guarded = next((assertion for assertion in assertions
                    if assertion.get("id") == "guarded-supervisor-tss-stacks"), None)
    if not guarded:
        raise ValueError("guarded kernel-stack assertion is absent")
    high_water = guarded.pop("observed_high_water_bytes", {})
    if assertions != [dict(assertion) for assertion in ASSERTIONS] \
            or set(high_water) != {"slot0", "slot1"} \
            or any(not isinstance(high_water[key], int) or not 0 < high_water[key] < 8192
                   for key in ("slot0", "slot1")):
        raise ValueError("user-process assertion set drifted")
    if value.get("source_contracts") != SOURCE_CONTRACTS:
        raise ValueError("user-process source contract drifted")
    if len(value.get("boot_log_sha256", "")) != 64 \
            or len(value.get("generator", {}).get("sha256", "")) != 64:
        raise ValueError("user-process receipt evidence identity is missing")
    gaps = value.get("known_gaps", [])
    if len(gaps) != 10 or not any("physical" in gap for gap in gaps):
        raise ValueError("user-process receipt hides its known gaps")


def selftest(value: dict) -> None:
    mutations = {}
    identity = copy.deepcopy(value)
    identity["build_identity"] = "0" * 64
    mutations["foreign-build"] = identity
    mapping = copy.deepcopy(value)
    mapping["source_contracts"]["user_stack"] = "read-write-execute"
    mutations["invented-executable-stack"] = mapping
    generation = copy.deepcopy(value)
    generation["source_contracts"]["generation_exhaustion"] = \
        "generation wraps to one"
    mutations["lost-generation-exhaustion"] = generation
    boundary = copy.deepcopy(value)
    next(assertion for assertion in boundary["assertions"]
         if assertion["id"] == "privilege-and-user-copy-boundary")[
             "crossing_pointer_refused_before_dereference"] = False
    mutations["lost-full-span-refusal"] = boundary
    missing = copy.deepcopy(value)
    missing["assertions"].pop(4)
    mutations["missing-offender-containment"] = missing
    guard = copy.deepcopy(value)
    next(assertion for assertion in guard["assertions"]
         if assertion["id"] == "lower-stack-guard-fault-containment")[
             "page_fault_error"] = 0
    mutations["invented-guard-fault"] = guard
    kernel_stack = copy.deepcopy(value)
    next(assertion for assertion in kernel_stack["assertions"]
         if assertion["id"] == "guarded-supervisor-tss-stacks")[
             "observed_high_water_bytes"]["slot0"] = 8192
    mutations["invented-kernel-stack-headroom"] = kernel_stack
    lifecycle = copy.deepcopy(value)
    lifecycle["assertions"] = [assertion for assertion in lifecycle["assertions"]
                               if assertion["id"] != "process-frame-reclamation"]
    mutations["missing-process-reclamation"] = lifecycle
    identity_custody = copy.deepcopy(value)
    identity_custody["assertions"] = [
        assertion for assertion in identity_custody["assertions"]
        if assertion["id"] != "generation-safe-process-identity"]
    mutations["missing-generation-custody"] = identity_custody
    fault_custody = copy.deepcopy(value)
    next(assertion for assertion in fault_custody["assertions"]
         if assertion["id"] == "fault-and-exit-custody")[
             "fault_address"] = 0xdeadbeef
    mutations["invented-gp-fault-address"] = fault_custody
    identity_reap = copy.deepcopy(value)
    next(assertion for assertion in identity_reap["assertions"]
         if assertion["id"] == "process-identity-reclamation")[
             "resource_release_before_reap"] = False
    mutations["lost-resource-before-reap"] = identity_reap
    accounting = copy.deepcopy(value)
    next(assertion for assertion in accounting["assertions"]
         if assertion["id"] == "process-memory-accounting")[
             "owner_totals_after_release"] = 1
    mutations["invented-owner-total"] = accounting
    host = copy.deepcopy(value)
    host["host_receipt"]["targets"].pop("anonmemorytest")
    mutations["wrong-host-lifecycle-target"] = host
    gaps = copy.deepcopy(value)
    gaps["known_gaps"] = []
    mutations["hidden-gaps"] = gaps
    caught = []
    for name, mutant in mutations.items():
        try:
            validate(mutant)
        except (OSError, ValueError, TypeError, json.JSONDecodeError):
            caught.append(name)
        else:
            raise ValueError(f"user-process receipt mutation escaped: {name}")
    print("user-process receipt selftest: caught " + ", ".join(caught))


def write_atomic(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", dir=path.parent, delete=False,
                                     encoding="utf-8") as handle:
        json.dump(value, handle, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    os.replace(temporary, path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--log", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args()
    try:
        value = build(args.log)
        if args.selftest:
            selftest(value)
        output = args.output.resolve()
        write_atomic(output, value)
        print(f"user-process QEMU receipt: PASS -> {output.relative_to(KERNEL_ROOT)}")
        return 0
    except (OSError, ValueError, TypeError, json.JSONDecodeError) as error:
        print(f"user-process QEMU receipt: FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
