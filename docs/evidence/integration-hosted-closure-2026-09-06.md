# Process and sweep integration: hosted closure

This supersedes the pending-verification status in the
[integration candidate receipt](integration-sweep-process-2026-09-06.md).
It records this integration batch, not completion of the 906-feature program.
The complete hosted gate passed: **126 successful steps, zero failed steps,
`GATE GREEN`**, on implementation `dd1871a`.

## Published source and ownership

[Draft PR #15](https://github.com/RoyX4/ZlOS/pull/15) contains the scheduler,
bounded sleep, TODO recovery and committed whole-tree sweep in one branch:
`codex/integrate-sweep-process`. The tested implementation is
`dd1871a82c544f38692fd286d14d4923b7898fa1` in
`/home/roy/Documents/repos/zl-linux-integration-2026-09-06`.

The branch retains these committed inputs in its ancestry:

| Input | Source tip |
|---|---|
| Scheduler and sleep, PRs #12/#13 | `ddb79d14731d48b9e169a4d6afa6b1baf53769d0` |
| Claude whole-tree sweep | `921297911b340790cd8fd11635f28760cd89f67e` |
| TODO and installed-hook fixtures, PR #14 | `5cd58d7888187d1f88a371c3a097c8d24b40cb20` |

Main advanced concurrently from `8fb1425` to `9212979`; both the local main ref
and GitHub main were checked at 05:51 UTC on 2026-09-06. The integration already
contains that exact main tip. This task did not move main or merge/close any PR.
PRs #12–#15 remain open. The original candidate receipt describes the earlier
snapshot; its statement that main was unchanged is no longer a live status.

The integration adds reproduced repairs for kernel FP controls on syscall/timer
entry, missed ISR roots/deep calls, and deferred window-close requests crossing
into a replacement window. TODO generation now also rejects an unterminated
handwritten block before publication. The candidate receipt retains the
before/after tests and their limits. Review was bounded to the integration
points and selected changed code; this is not a second exhaustive audit of all
120 sweep paths.

## Hosted verification

The implementation head has 20 successful ordinary GitHub check results.
That count includes the optional second-opinion job, whose actual model review
was skipped because it was not configured. It is not independent review proof.

| Lane | Run | Measured result |
|---|---|---|
| BIOS/raw/ISO/native UEFI QEMU | [34014489039](https://github.com/RoyX4/ZlOS/actions/runs/34014489039) | Passed, including external-file fault, normal exit and bounded sleep routes. |
| Kali automatic host tests | [34014488881](https://github.com/RoyX4/ZlOS/actions/runs/34014488881) | 77 commands; 71 passed targets, zero failed, four hardware skips and 13 not-run targets, from 88 declared targets. |
| Complete sequential landing gate | [34014513857](https://github.com/RoyX4/ZlOS/actions/runs/34014513857) | Passed: 126 successful steps, zero failed steps, ending `GATE GREEN`. |

The ordinary boot receipts share build identity
`17f276aafff25b292c00cea59c5be48d11dea5e58de5bc0d9700053f43d703e2`.
The separate Kali host receipt has identity
`c50d19442a60f8b70c31fcac75c29e772821c27c7e1db1a437a49b0a592af17e`.
They are distinct environment-bound proof lanes; they do not replace the
complete run's own evidence join.

The completed Kali run independently regenerated and joined its feature,
performance, host and three native process receipts under the single identity
`c50d19442a60f8b70c31fcac75c29e772821c27c7e1db1a437a49b0a592af17e`.
The downloaded run metadata and build context both name exact head `dd1871a`.
All implementation hashes named by the three process receipts were checked
against the local source. Their common native image SHA-256 is
`ef4d17065cbad9b7f3001dcc18f7c436895eb6341810c24c1dd814ba7072f04a`.
The full run again executed 77 host commands with zero failures, retaining the
71 passed, four hardware-skipped and 13 not-run target classifications.

## Current program and performance result

The full-run feature ledger reports `PASS_WITH_OPEN_GAPS`. Its 906 individual
rows were recounted from the downloaded JSON and match the declared totals:

| Maturity | Features |
|---|---:|
| `PROVED_CURRENT` | 8 |
| `PARTIAL_CURRENT` | 55 |
| `PLANNED_UNPROVED` | 843 |
| Total | 906 |

This integration preserves the prior sleep capability and repairs its combined
foundation; it does not add another completed product feature. Public release
remains blocked by the recorded evidence and capability gaps.

Six of seven measured host performance categories passed their budgets.
The host-test build distribution remains over budget: p95 **89.60 seconds**
against **60.00 seconds**, across seven full host-test-suite builds with
75 verified executables. The performance registry reports
`OPEN_REGRESSIONS_AND_GAPS` with `build` as its open regression. The landing
gate verifies receipt integrity while retaining the failed budget.
Product build latency, native-target performance and physical
performance remain unmeasured.

The external-file routes report
`PASS_EXTERNAL_FILE_SPAWN_FAULT_OBSERVE_REAP`,
`PASS_EXTERNAL_FILE_SPAWN_EXIT_OBSERVE_REAP` and
`PASS_EXTERNAL_FILE_SLEEP_EXIT_OBSERVE_REAP`. The successful program emits
`R3!` and exits with 37; the sleep route observes at least five guest PIT ticks.
These are QEMU observations, not physical-machine qualification or a userspace
spawn/wait syscall.

Superseded complete runs `34014183287` and `34014376771` were deliberately
cancelled after source follow-ups. Neither is counted as passed.

## Concurrent worktree snapshot

Read-only snapshot at 05:51 UTC, 2026-09-06. Paths below are under
`/home/roy/Documents/repos/`. Dirty counts are status entries, not unfinished
feature counts or evidence of which agent is currently running.

| Checkout | Head | Dirty paths |
|---|---|---:|
| `zl-linux` (main) | `9212979` | 31 |
| `zl-linux-906-close` | `8fb1425` | 4 |
| `zl-linux-cleanup-todo` | `5cd58d7` | 3 |
| `zl-linux-integration-2026-09-06` | `dd1871a` | 0 |
| `zl-linux-process-lifecycle` | `8fb1425` | 24 |
| `zl-linux-process-sleep` | `ddb79d1` | 0 |
| `zl-linux-scheduler-service` | `54d3cf6` | 17 |
| `zl-linux/.claude/worktrees/fable-sweep` | `9212979` | 1 |
| `zlos-github-professional` | `8fb1425` | 13 |

All 17 saved scheduler dirty-file hashes still match the original backup.
Other worktrees were neither cleaned nor reset. Uncommitted changes in main
and the sweep worktree are outside PR #15. The visible Codex task list showed
this cleanup task active and its predecessor not loaded; it does not establish
whether a separate Claude or Cursor process is editing a checkout.
At 06:07 UTC, a process-name/working-directory scan found one matching `codex`
process in the language checkout and no matching Claude/Cursor process in these
OS paths. This filtered snapshot does not prove exclusive ownership. It read
neither process command lines nor environments; `agent-process-snapshot.json`
retains the observation.

## Evidence custody and remaining work

Local artifacts are retained under
`/home/roy/Documents/artifacts/zl-linux/integration-2026-09-06/`, including
`integration-inputs.json`, `ordinary-proof-summary.json`,
`worktree-closeout-snapshot.json`, the downloaded ordinary receipts and the
before/after regression logs. The complete downloaded artifact is in
`full-34014513857/`; `full-proof-summary.json` records counts, source binding
checks and locally measured receipt hashes.

The GitHub artifact is `hosted-full-closure-34014513857-1`, ID `9984178737`,
with GitHub-reported archive SHA-256
`058f7c4181a46f9e815f0c59b8a1cd03ecd92ee04c84287ce71c997f2a776976`.
Locally measured SHA-256 values include:

- Full log: `4ca4a939192ebade88c01244a49f0f9847e5c42e8f2b9aaad926ad56eda602f5`.
- Feature ledger: `7a5ca1d0bfbffa75f15654183c81f6b5d87945009c259d59b106e729b8ae4d15`.
- Host receipt: `b12a3290bf75e304e3dd603f3c40726ebcae6e8cd1d11deb9e7d1ba224d9e21c`.
- Sleep receipt: `9737e2b318cb00c49b83182dfc6115a60ecc35542088845a6cacacd72aff2271`.

Generated runtime evidence already tracked in Git remains a historical
snapshot. The downloaded complete run is the authority for the counts above;
receipts from different environments were not mixed into the checkout. The
closing publication changes documentation only, leaving the fully tested
implementation unchanged. No new runtime verification is claimed for that
documentation commit.

The next product dependency remains
[userspace process management](../design/userspace-process-management-next.md):
separate child image preparation from the active caller before exposing spawn
and wait, with rollback, parent/child custody and independent memory proof.
This batch adds no spawn/wait syscall. Physical ThinkPad qualification,
general scheduling/address-space scope and the documented certificate-policy
gap remain open. Shared live hooks, firmware and installed boot state were not
changed by this task.
