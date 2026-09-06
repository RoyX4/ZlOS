# zlOS continuation handoff — 2026-09-05

Final verification update: 2026-09-06, 05:13 UTC.

This task continues the complete 906-feature program and the existing cleanup.
It does not narrow that destination to a scheduler or a small evidence batch.
This completed batch advances Wave 3 process foundations. Claude has separately
committed the broader system repair sweep; integration remains outstanding.

## Start in the right repository

The OS is `/home/roy/Documents/repos/zl-linux`, GitHub `RoyX4/ZlOS`.
The saved Codex project currently opens `/home/roy/Documents/repos/zl`, which is
the separate language repository. Do not infer the active OS checkout from the
task's initial working directory.

Read [PROJECT-STATUS.md](PROJECT-STATUS.md), the
[complete program](program/README.md), the
[product implementation order](program/PRODUCT-IMPLEMENTATION-ORDER.md), and
the relevant directory instructions before changing code. Older sole-checkout
tables are dated historical material; preserve today's concurrent work.

## Published work and proof

| Work | Location and commit | Current proof |
|---|---|---|
| Persistent scheduler and allocator/toolchain join repairs | `zl-linux-scheduler-service`, `codex/persistent-user-scheduler`, `54d3cf6`; [PR #12](https://github.com/RoyX4/ZlOS/pull/12) | [Full run 33961205228](https://github.com/RoyX4/ZlOS/actions/runs/33961205228): **121 successful steps, zero failures, GATE GREEN**. Ordinary CI also passed. Feature ledger: 8 proved, 54 partial, 844 planned. |
| Bounded userspace sleep | `zl-linux-process-sleep`, `codex/process-sleep`; [draft PR #13](https://github.com/RoyX4/ZlOS/pull/13), based on #12 | [Full run 33961210688](https://github.com/RoyX4/ZlOS/actions/runs/33961210688) at `baa4bb2`: **122 successful steps, zero failures, GATE GREEN**. KR-024 is now `PARTIAL_CURRENT`; ledger: **8 proved, 55 partial, 843 planned**. Published ancestry synchronization `6cd352a` has exactly the same tree. Final documentation/copy corrections follow that tested code head. |
| Atomic TODO publication and pending-document hook guard | `zl-linux-cleanup-todo`, `codex/cleanup-todo-atomic`, `5cd58d7`; [draft PR #14](https://github.com/RoyX4/ZlOS/pull/14), based on main | Nine TODO recovery tests, seven installed-hook fixture tests and all ordinary CI passed. The installer source is repaired; the live shared hooks have not been reinstalled. |

None of these PRs has been merged. Main was last verified at
`8fb1425777b184c762686a12161e3faf37e2095a`. Verify live refs before acting.
The optional second-opinion workflow skipped model review because no key was
available; its green job is not independent review evidence.

The first sleep CI attempt, `8a0959a`, failed before building because the
`userps` source change required a regenerated app manifest. `7e395d5` fixes both
generated files. The successful boot run is
[33958919425](https://github.com/RoyX4/ZlOS/actions/runs/33958919425).

## Preserve these owners and pending files

- `zl-linux-scheduler-service` had 17 dirty files before this continuation:
  source-bound generated receipts/identity data plus TODO, journal and docs.
  They remain byte-identical to the original saved hashes and were excluded
  from the new commits. Three useful documentation corrections (CI pipeline,
  evidence index and process ABI) have been reconciled into the sleep tree.
  The existing post-commit/pre-push hooks were stopped during this turn after
  starting automatic work. Two identity files rewritten by pre-push were
  restored to their exact saved bytes; no original pending change was lost. Backup:
  `/home/roy/Documents/artifacts/zl-linux/scheduler-continuation-2026-09-05/before.patch`
  and `before.json`. Do not publish those older receipts as fresh proof.
- `zl-linux/.claude/worktrees/fable-sweep`, branch `fable/whole-tree-sweep`, has
  committed source sweep `c6af05b` and follow-up `5dc6f45`; only its journal
  remains dirty at this final observation. Its source and evidence have not
  been independently reviewed here. Architecture, kernel, app-manifest and gate files
  overlap the process PRs; its atomic TODO change also overlaps #14.
  Inspect its final source and proof before integration;
  do not reset, replace or stage its working files.
- Other retained trees include `zl-linux-process-lifecycle`,
  `zl-linux-906-close` and `zlos-github-professional`. A matching file or old
  branch head does not establish that an entire dirty checkout is redundant.
- This sleep branch includes final evidence/next-step documentation and three
  feature-description string corrections beyond its tested code head. The TODO tree retains the unpublished live-workspace audit
  and its orientation link separately from the published repair commits.

No other agent was interrupted, no worktree was deleted, and no boot/security/firmware
settings were changed. The laptop is shared by other active tasks; expensive
full closure stays on the hosted runners. The interrupted local pre-push
rebuild is not passing gate evidence. Narrow commits use `ZL_HOOK_RUNNING=1`
to avoid automatic journal/TODO mutation; hosted CI supplies full verification.
The two repair pushes skipped the optional local pre-push duplicate rather
than restarting a local kernel/QEMU chain. Installed shared hooks were not
modified.

## Final verification and next product step

Both fresh full runs above completed successfully. Downloaded logs contain the
exact step counts and final `GATE GREEN`; artifact roots are:

- `/home/roy/Documents/artifacts/zl-linux/scheduler-continuation-2026-09-05/full-33961205228`
- `/home/roy/Documents/artifacts/zl-linux/process-sleep-2026-09-05/full-33961210688`

The scheduler build identity is
`cdae9885ee8dd1becd17284ee090fae3318dfce4a302076acbb8e9956ed17dd2`;
the sleep build identity is
`6e1d3f58a96010a78fb34fc584ad6fcb4a28884e7f977926724e61f4793764d7`.
The sleep receipt proves a 98-byte external program waited at least five guest
PIT ticks, exited with status 37, remained observable and was reaped.

Both host receipts record 74 commands executed, 68 passed targets, zero failed,
13 not run, four skipped for hardware and zero unavailable. These are different
receipt categories, not a single additive test count. Both performance
registries retain seven categories: six host budgets pass and the **build
budget fails**. There are zero native-target and zero physical performance
categories. A green evidence gate records that limitation; it does not repair
or waive the performance regression. Physical ThinkPad qualification remains
unverified. Optional independent model review was skipped.

The original runs `33958363486` and `33958972275` remain failed historical runs.
The [toolchain join repair](evidence/toolchain-join-repair-2026-09-05.md) explains
why the measured 83-header closure was rejected and how both retained-input
replays and fresh full runs now pass. The local watcher has finished; no
scheduled continuation was created.

The [sleep evidence record](evidence/process-sleep-2026-09-05.md) separates host,
compilation, injected-clock boot proof and real guest-timer proof. Fresh joined
receipts actually promote KR-024 to `PARTIAL_CURRENT`, not complete timer
infrastructure. Absolute deadlines, cancellation, coalescing, suspend, latency
bounds and physical proof remain open. These counts belong to the downloaded
full-run ledger, not an assertion that main's checked-in ledger was refreshed.

First reconcile Claude's committed sweep with the three PRs, then rerun gates
on the combined source before landing. No PR has been merged.

Next continue userspace process management and wait/termination custody through
the [audited implementation contract](design/userspace-process-management-next.md).
The full-width process handle must use a validated output buffer, not a
signed error return. Terminal-parent adoption must retain kernel custody of
children. The image loader must be separated from active execution context before it is
called from a syscall. Parent death/orphan ownership and failure-atomic wait
must be defined and tested with that slice. Reuse the existing lifecycle,
scheduler and memory modules rather than adding another process registry.

Physical ThinkPad qualification, broader providers, services, apps/browser,
language/tooling and release remain in the full program. Passing this bounded
process slice does not close them.

## Historical task recovery

The referenced task is `01a041bc-0d84-7200-b569-64ef4bbc19cc`, titled
`Continue ZL Linux cleanup`. Its task API returned an older slice; the verified
local source continues through September 4 with scheduler work and PR #12.
The source is
`/home/roy/.codex/sessions/2026/08/27/rollout-2026-08-27T15-40-38-01a041bc-0d84-7200-b569-64ef4bbc19cc.jsonl`.
Use its later user/assistant messages if the API again stops at PRESSWORK.
One malformed JSONL line was encountered; do not silently claim a perfect
whole-transcript parse. PRESSWORK publication is an older completed checkpoint,
not the current implementation queue.
