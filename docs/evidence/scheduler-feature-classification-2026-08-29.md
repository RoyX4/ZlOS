# Current scheduler feature classification

Date: 2026-08-29

This receipt maps the bounded 64-bit process scheduler evidence into KR-022.
It does not promote the two-process diagnostic into a general scheduler or
claim physical-hardware behavior.

## Current implementation

The native UEFI64 path has two fixed Ring 3 process objects. Each object owns a
PML4, guarded user stack, guarded two-page supervisor/NX TSS-selected kernel
stack and saved privilege-return frame.
The PIT interrupt path saves the complete interrupted frame, returns through
the kernel, selects the sibling's CR3 and kernel stack, and later resumes the
interrupted instruction.

This is distinct from `kernel/src/core/sched.c`. That older opt-in kernel-task
demo has at most eight fixed tasks, cooperative switches and bounded tick
counters. It deliberately does not preserve FPU/SSE state and is not evidence
for the general per-CPU scheduler contract.

## Current evidence

The exact current subject is the `build_identity` field in generated
`docs/program/FEATURE-STATUS.json` and the scheduler receipt; this page does not
duplicate that volatile value.

`scheduler-native-uefi64-qemu-2026-08-29.json` binds the exact current
`zlOS-usb.img`, `verify-efi.sh`, `usermode.c`, `idt.c`, boot log and receipt
generator. A fresh native UEFI64 QEMU run records three assertions:

| Assertion | Observed result |
|---|---|
| cooperative context resume | two separate PML4/kernel-stack contexts produce `AB12` and exit independently |
| timer preemption | two non-yielding Ring 3 loops are interrupted at least twice and produce `PQ` |
| fault containment | one process takes vector 13 while its sibling produces `K` and exits |

The receipt and 906-ledger validators reject a foreign build or artifact,
changed verifier or implementation, missing assertion, invented cooperative
preemption, and hidden gaps.

## Classification

KR-022 moves from `PLANNED_UNPROVED` to `PARTIAL_CURRENT`. The current artifact
proves a bounded two-process scheduling oracle on one QEMU route. It remains
partial because it has no general priority, fairness or deadline contract, no
per-CPU run-queue ownership or migration, no persistent spawn/reap/cancellation
service, no general resource reclamation, and no current physical-hardware
scheduler receipt.

Exact current maturity counts remain in generated `program/FEATURE-STATUS.json`.

## Verification

```sh
ZLOS_SKIP_BUILD=1 kernel/tools/checks/verify-efi.sh
python3 tools/gen_feature_status.py --write --selftest
python3 tools/gen_feature_status.py --check --selftest
python3 tools/validate_master_program.py --self-test
tools/doc-check.sh
```
