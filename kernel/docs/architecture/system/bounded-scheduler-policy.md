# Bounded scheduler policy

## Status and evidence boundary

This document describes the current `scheduler_policy.c` contract and its
bounded Ring-3 integration. The policy is host-proved, listed in the kernel
source manifest, compiled under the BIOS 32-bit, BIOS 64-bit and UEFI 64-bit
warning-strict flags, linked into fresh target artifacts, and exercised by the
native-UEFI QEMU gate. `user_process_service.c` now combines it with exact
generation-tagged lifecycle handles and runs one preemptible Ring-3 turn from
each ordinary kernel work-loop call.

This is still not a general scheduler. The older scheduler in `core/sched.c`
continues to own its separate opt-in kernel-task context switching. There is no
per-CPU process run queue, migration, priority/deadline policy, cancellation API
or physical-hardware receipt. Those gates must not be inferred from the bounded
two-slot service.

## Why this seam exists

zlOS has two scheduling mechanisms with deliberately separate ownership:

- `sched.c` switches kernel task stacks after an explicit `sched_go` request.
- `user_process_service.c` selects at most two persistent Ring-3 processes and
  delegates the privileged step/resume operation to `usermode.c`.

The policy module owns only decisions that do not require privileged CPU state:

- exact owner admission and duplicate rejection
- one-running-owner enforcement
- round-robin selection
- wrap-safe sleep deadlines
- yield, exit and reap transitions
- per-owner run and dispatch accounting
- table-wide dispatch and context-switch accounting
- fail-closed structural validation

Stack pointers, page tables, saved registers, TSS stacks, lifecycle custody and
actual context switching remain outside this module.

## Identity contract

An owner is an exact nonzero 64-bit token. Ring-3 integration must use the
generation-safe `process_lifecycle_handle`, not a bare PID or reusable slot
index. A stale process handle must fail lifecycle validation before the policy
is changed. Kernel-task integration may use a separate nonzero task token, but
must not confuse that token with a process handle.

The caller supplies a fixed slot array and its capacity. There is no allocator,
unbounded queue or hidden fallback. Admission checks duplicates before capacity,
so a full table still reports an existing owner as a duplicate.

## State machine

The policy owns five states:

```text
EMPTY -> RUNNABLE -> RUNNING -> RUNNABLE
                     |   |
                     |   +-> SLEEPING -> RUNNABLE
                     +-----> EXITED -> EMPTY
```

Only `dispatch` creates `RUNNING`. Only the exact running owner may yield, sleep
or exit. An exited owner retains its accounting until reap; reap clears the slot
for a new exact owner. At most one slot may be running, and the table's running
index must name it exactly.

Every public read or mutation validates the complete table first. Corrupt state
is rejected without trying to repair or continue through it. Duplicate owners
are reported distinctly by the checker; operations surface global corruption as
`SCHEDULER_POLICY_E_STATE`.

## Time and accounting

Sleep uses unsigned 32-bit ticks and subtraction-based deadline comparison, so a
deadline crossing tick zero remains ordered. Delays are clamped to
`0x7fffffff`, the largest unambiguous forward distance. Zero delay is rejected;
yield represents that operation without a sentinel collision.

The caller charges elapsed run ticks when the current owner yields, sleeps or
exits. Run ticks, dispatches and switches saturate at 64-bit maximum instead of
wrapping. A dispatch of the same sole owner is not counted as a context switch.

## Current proof

`tests/host/schedulerpolicytest.c` currently reports 126 checks and zero
failures. It covers admission, duplicate/capacity precedence, fairness, exact
accounting, sleep across tick wrap, maximum-delay clamping, exit/reap/reuse,
counter saturation, unchanged outputs on idle, and corruption rejection through
all public operation classes.

The same test passes as a native and 32-bit executable. AddressSanitizer and
UndefinedBehaviorSanitizer report no defect. GCC `-fanalyzer` and Clang static
analysis report no defect. Measured source coverage is 100% of 169 lines, 100%
of 170 branches executed, and 86.47% of branch outcomes taken.

The Kali-local host receipt contains 84 targets and 74 executed commands: 69
passed, zero failed, three hardware skips and 12 explicit non-runs. The
[GitHub-hosted Ubuntu run](../../../../docs/evidence/hosted-user-process-gate-2026-09-03.md)
repeated the same inventory with 68 passes, zero failures, four hardware skips
and 12 explicit non-runs; the difference is one unavailable hardware target,
not a promoted skip.
`processlifecycletest` is `HT-058` with 91 checks, `schedulerpolicytest` is
`HT-064` with 126 checks, and `userprocessservicetest` is `HT-076` with 105
checks; all report zero failures.

The native-UEFI QEMU receipt binds the exact boot image, verifier, implementation
sources and host receipt. In addition to the older `AB12`, non-yielding `PQ` and
faulted-sibling `K` observations, it runs two service-owned processes across four
separate kernel work-loop calls. Round robin produces `ST12`, retains exit
statuses 11 and 22, detaches scheduler ownership before identity reap, and
restores the exact physical-frame baseline.

The desktop command probe independently creates a four-byte external
`/system/user.bin` through Files and the disk editor. `userexec` admits PID 1000;
the intentionally invalid 64-bit instruction faults only that process;
`userps` reports the exact terminal slot; `userreap 1` releases it; and a final
`userps` reports an empty table. This proves the real command route and contained
fault workflow, not a successful application workload. GitHub Actions run
`33712768713` repeated that route under Ubuntu QEMU TCG and retained the exact
six-assertion JSON receipt.

## Remaining integration order

1. Add a userspace-facing process-handle and spawn/wait/cancellation ABI with
   explicit authority; the current controls are kernel commands.
2. Generalize fixed process slots and resource reclamation without weakening
   generation or termination custody.
3. Add priority/deadline policy only with bounded admission and inversion tests.
4. Add per-CPU run-queue ownership, synchronization and migration receipts.
5. Integrate or deliberately retain `sched.c` only after target proof covers its
   architecture switcher and FPU/SSE state.
6. Run the same bounded service and command workflow on physical hardware.
