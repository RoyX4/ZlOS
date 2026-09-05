# Persistent process sleep: 906-feature continuation

Date: 2026-09-05

This Wave 3 slice adds useful waiting to the existing two-slot Ring-3 service.
A program can call syscall 25 with a bounded relative delay, leave the runnable
queue, let its sibling run, and resume after its deadline. `userps` reports
sleeping processes. This advances KR-022 (scheduler) and the bounded part of
KR-024 (timer/deadline API); all 906 feature atoms remain in scope.

## Implementation and limits

The service reuses `scheduler_policy_sleep`; there is no second timer queue.
Only the exact running lifecycle handle may submit one pending request.
Reconciliation charges the completed turn once and records the deadline from
the request's tick. Waiting neither consumes dispatch turns nor accrues run
time. A sleeping process retains its existing lifecycle custody and cannot be
reaped as if it had exited.

The ABI accepts 1..`0x7fffffff` ticks and rejects zero, wider delays and nonzero
reserved arguments before yielding. The 32-bit 100 Hz tick domain is wrap-safe
within that bound. Absolute deadlines, cancellation, coalescing, suspend/resume,
maximum wake latency and physical qualification remain open. See the
[ABI contract](../../kernel/docs/architecture/system/user-process-abi.md).

## Evidence ledger

- Focused production-service harness: 144 checks, zero failures in native,
  32-bit and ASan/UBSan builds. These include wrap, deadline zero, maximum delay,
  exact accounting, sibling completion and stale/foreign/duplicate requests.
- Generated syscall admission: 36 checks pass for the 25-entry ABI.
- Changed service and architecture sources compile with warning-strict UEFI64
  flags. Compilation is not a target execution claim.
- New boot oracle: `LSW` must come from real Ring-3 execution with injected
  scheduler timestamps, no early dispatch, independent exit statuses 33/44,
  sleeping-state observation and exact physical-frame reclamation.
- New external-file route: `probe-user-process-exit.py --sleep` loads machine
  code from zlfs. The program reads guest PIT ticks, sleeps five ticks, and
  faults if sleep fails or elapsed guest time is less than five. Only afterward
  can it emit `R3!`, exit 37, be observed and be reaped. There is no user polling
  loop. This complements the exact-boundary boot oracle.
- Hosted QEMU execution for this change is pending. No physical test was run.

The feature generator may label KR-024 `PARTIAL_CURRENT` only after joining the
fresh host, boot and external-sleep receipts. Merely adding that rule does not
refresh or promote the checked-in ledger. The unchanged program validator
reports 906 feature atoms, 21 phases, 174 research contracts, 63 named current
implementations and 24 games.

Focused test logs and object files are retained locally at
`/home/roy/Documents/artifacts/zl-linux/process-sleep-2026-09-05/`.

## Work custody and next order

The isolated worktree is `/home/roy/Documents/repos/zl-linux-process-sleep`,
branch `codex/process-sleep`, based on `fc038d4`. That base is the allocator
feature-join repair on [PR #12](https://github.com/RoyX4/ZlOS/pull/12).
Its [full hosted run](https://github.com/RoyX4/ZlOS/actions/runs/33958363486)
verifies that base, not this new sleep implementation. All ordinary base PR
workflows passed; the complete sequential gate was still running when this
note was written. Optional second-opinion review was skipped, not performed.

Claude's active `fable/whole-tree-sweep` remains separately owned. Its
`usermode.c`, `kernel.zl` and landing-gate edits overlap this branch by file and
must be reconciled before integration. No worktree was deleted or reset. The
scheduler worktree's 17 pre-existing pending files remain separate and saved.

Next: finish hosted sleep proof and inspect its exact receipts; reconcile the
scheduler and active sweep; then continue userspace process management,
wait/termination custody and generalization beyond fixed slots in the
[product implementation order](../program/PRODUCT-IMPLEMENTATION-ORDER.md).
The complete destination also retains boot/recovery, IPC/services, drivers,
storage, desktop/apps, browser, language/tooling and release work.
