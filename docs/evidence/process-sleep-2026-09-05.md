# Persistent process sleep: 906-feature continuation

Date: 2026-09-05; final hosted evidence verified 2026-09-06.

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
- [Hosted run 33958919425](https://github.com/RoyX4/ZlOS/actions/runs/33958919425)
  passed all four boot jobs at `7e395d5`, including the boot sleep oracle and
  separate external fault, normal-exit and real-timer sleep routes. The retained
  98-byte sleep fixture has SHA-256
  `4eca0e58d4c191c8d340ac9b27c7648d4c8b564b42300b6df42a9166533a5c41`;
  its receipt is `PASS_EXTERNAL_FILE_SLEEP_EXIT_OBSERVE_REAP`, with build identity
  `0f713684ce2a62890150a0fbe7158e964e53b0bfdadf1087ff179793764d8d59`.
  All ordinary PR workflows passed. No physical test was run.
- The first [complete hosted sleep closure](https://github.com/RoyX4/ZlOS/actions/runs/33958972275)
  passed its runtime steps but failed the final toolchain/evidence joins. The
  runner measured 83 headers while the join required 82. The
  [shared repair](toolchain-join-repair-2026-09-05.md) reuses the manifest
  validator and passes replay of both failed runs' retained registry inputs.
  [Fresh full run 33961210688](https://github.com/RoyX4/ZlOS/actions/runs/33961210688)
  passed at `baa4bb2`: **122 successful steps, zero failing steps, GATE GREEN**.
  Base synchronization `6cd352a` preserves its exact committed tree,
  `47f693b713c6fe0867868ac70c706b4c850d29a3`. The full-run sleep receipt is
  `PASS_EXTERNAL_FILE_SLEEP_EXIT_OBSERVE_REAP`, 98 bytes, minimum five guest
  ticks and expected exit status 37, bound to build identity
  `6e1d3f58a96010a78fb34fc584ad6fcb4a28884e7f977926724e61f4793764d7`.
  Downloaded artifacts are retained under `full-33961210688/` below the local
  evidence root. Final documentation and feature-description copy corrections
  follow the tested code head; no runtime behavior changed in that follow-up.

The fresh full-run feature ledger now labels KR-024 `PARTIAL_CURRENT` after
joining its current host, boot and external-sleep receipts. Its measured counts
are **906 total: 8 `PROVED_CURRENT`, 55 `PARTIAL_CURRENT`, 843
`PLANNED_UNPROVED`**. This is the downloaded full-run ledger; main has not been
merged or refreshed. Both final full runs retain a failed host build-performance
budget: six of seven host categories pass, with no native-target or physical
performance categories. Green evidence closure does not close that regression. The unchanged program validator
reports 906 feature atoms, 21 phases, 174 research contracts, 63 named current
implementations and 24 games.

Focused test logs and object files are retained locally at
`/home/roy/Documents/artifacts/zl-linux/process-sleep-2026-09-05/`.

## Work custody and next order

The isolated worktree is `/home/roy/Documents/repos/zl-linux-process-sleep`,
branch `codex/process-sleep`, based on `fc038d4`. That base is the allocator
feature-join repair on [PR #12](https://github.com/RoyX4/ZlOS/pull/12).
Its first [full hosted run](https://github.com/RoyX4/ZlOS/actions/runs/33958363486)
failed the toolchain join described above. The base repair is `54d3cf6`, under
[full run 33961205228](https://github.com/RoyX4/ZlOS/actions/runs/33961205228),
which passed 121 steps with zero failures and final `GATE GREEN`. Its scheduler-only
feature counts are 8 proved, 54 partial and 844 planned.
The sleep branch contains that base in its ancestry. Its target process code
remains distinct from the scheduler-only branch. Optional second-opinion review was skipped, not performed.

Claude's `fable/whole-tree-sweep` now contains committed sweep `c6af05b` and
follow-up `5dc6f45`; its journal remains dirty. The sweep was not independently
reviewed in this task. Its
`usermode.c`, `kernel.zl` and landing-gate edits overlap this branch by file and
must be reconciled before integration. No worktree was deleted or reset. The
scheduler worktree's 17 pre-existing pending files remain separate and saved.

Next: reconcile the scheduler, sleep and cleanup PRs with the committed sweep
and verify their combined source; then continue userspace process management,
wait/termination custody and generalization beyond fixed slots in the
[product implementation order](../program/PRODUCT-IMPLEMENTATION-ORDER.md).
The [next process-management contract](../design/userspace-process-management-next.md)
records the audited loader/context dependency and the required spawn/wait
transaction, orphan policy and proof cases.
The complete destination also retains boot/recovery, IPC/services, drivers,
storage, desktop/apps, browser, language/tooling and release work.
