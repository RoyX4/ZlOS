# Project Status

Checked 2026-08-27. This page separates repository state, planning state, and
implementation state. They are not the same thing.

## Read This First

1. [`program/README.md`](program/README.md) is the complete destination and
   dependency program.
2. [`program/PRODUCT-IMPLEMENTATION-ORDER.md`](program/PRODUCT-IMPLEMENTATION-ORDER.md)
   is the human delivery order.
3. [`EXECUTION-ROADMAP.md`](EXECUTION-ROADMAP.md) is the nearer-term systems and
   performance queue.
4. [`STATE-OF-THE-PROJECT-2026-08-19.md`](evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md)
   is evidence-rich historical audit material, not the current queue.
5. [`WHOLE-TREE-CODEX-AUDIT-2026-08-26.md`](evidence/status-audits/WHOLE-TREE-CODEX-AUDIT-2026-08-26.md)
   records the current source-audit repairs and their hardware boundary.
6. [`REMAINING-WORK.md`](REMAINING-WORK.md) is the post-integration H0-H3 map.

## Repository State

| Repository or checkout | Commit state | Meaning |
|---|---|---|
| `RoyX4/zl-linux:main` | Published structure baseline at `26c089947dfa`; current authority is the live remote ref | The structure and directory-documentation series plus the whole-tree repair/evidence closure are published on `main`. Host, build, reproducibility and all six retained QEMU boot routes are covered by current receipts. This is repository closure and QEMU evidence, not product or physical-hardware completion. |
| This reconciliation worktree | `codex/full-program`, based on `origin/main` at `26c089947dfa` before the closure series | Isolated at `/home/roy/Documents/repos/zl-linux-final`. The cleanup series repairs status and static checker contracts, preserves historical inputs, separates retained evidence from disposable output, groups kernel documentation by purpose/subsystem, and adds eight local documentation surfaces to all 113 repository directories. The final repair pass also closes accepted language/kernel audit findings and refreshes current build/QEMU evidence. [`evidence/DIRECTORY-DOCUMENTATION-2026-08-26.md`](evidence/DIRECTORY-DOCUMENTATION-2026-08-26.md) records the structure rollout. Exact live Git state remains the publication authority. No physical-hardware result is claimed. |
| Older structure worktree | Dirty `codex/full-restructure`, based on `85fcdf542b9c` | Preserved at `/home/roy/Documents/repos/zl-linux-structure` with 566 pending entries from the older migration. It is input history, not the active finalization tree. |
| Original shared checkout | Dirty `codex/local-zlos-snapshot-2026-08-24` at `/home/roy/Documents/repos/zl-linux` | Preserved with 183 pending entries. Its untracked display audit has now been recovered into this branch as explicitly historical evidence; the shared checkout itself was not rewritten. |
| Desktop redesign worktree | `design/desktop-redesign` at `66eadd0` with 143 pending entries on the 2026-08-26 recheck | Preserved at `/home/roy/Documents/repos/zl-linux-desktop-redesign`. The pending design/prototype/render work is concurrent input, was not edited by this reconciliation pass, and is not merged or pushed. No kernel implementation is claimed. |
| `zl-linux-merge-all` | Clean historical integration checkout at `4b9883123682`; 63 commits behind the fetched `origin/main` on 2026-08-26 | Preserved integration checkpoint, not the current publication checkout. Use this reconciliation worktree or `origin/main` for current state. |
| `RoyX4/zl:main` | Pushed at `6f53115` | Separate language repository. Its local `main` matches the remote, but the working directory has untracked local output. |
| `zl-merge-all` | Local head `5207d5c`, five commits ahead of `RoyX4/zl:main` | The raw GitHub/other-OS research is committed locally. Two repository-pattern commits are on `origin/codex/github-repo-patterns`; the complete merge head is not on remote `main`. |
| `zl-starred-sources` | Not a Git repository | Local source mirror used for research. It is evidence input, not a committed project snapshot. |

The old `zl-apps`, `zl-browser`, `zl-exec`, `zl-feel`, `zl-main`, `zl-system`,
and `zl-value16` directories are no longer present. Their historical worktree
roles remain documented under [`evidence/`](evidence/). The final integration
receipt is [`MERGE-ALL-2026-08-24.md`](evidence/integration/MERGE-ALL-2026-08-24.md).

### Registered zl-linux Worktrees

Checked 2026-08-26 against `origin/main` at `26c089947dfa`.
Published integration heads are ancestors of that ref; intentional local
preservation and cleanup branches are called out separately rather than folded
into the ancestry claim. Dirty and untracked content is also explicit. The
local `main` ref itself is 96 commits behind `origin/main` and is not
publication authority.

| Worktree branch | HEAD | Remote/working state |
|---|---|---|
| `files-app` | `6b40dfb61cee` | Clean; matches `origin/files-app`. |
| `secret/desktop-foundry` | `1c96bfebe361` | Clean; matches `origin/secret/desktop-foundry`. |
| `design/desktop-redesign` | `66eadd0` | Dirty with 143 pending entries; no upstream and no kernel implementation claimed. Preserved untouched. |
| `codex/master-program-foundation` | `11c711eab261` | Clean; matches `github/codex/master-program-foundation`. |
| Three detached Claude worktrees | `bb1d623b9b49`, `84cecae63019`, `b8a00ec45ac1` | Clean and preserved; their commits are already in `origin/main`. |
| `codex/local-zlos-snapshot-2026-08-24` | `85fcdf542b9c` | Dirty shared checkout containing the uncommitted cleanup inputs; do not overwrite it. |
| `codex/full-restructure` | `85fcdf542b9c` base | Preserved older isolated migration with 566 pending entries. |
| `codex/full-program` | Base `26c089947dfa` before the current closure edits | Active isolated publication worktree. Exact live status is reported by Git rather than frozen into this row. |

## Whole Implementation Program

The current self-contained documentation coverage receipt is
[`evidence/DOCUMENTATION-COVERAGE-AUDIT-2026-08-25.md`](evidence/DOCUMENTATION-COVERAGE-AUDIT-2026-08-25.md).
It proves catalogue, phase, contract, registry, and navigation coverage; it
does not promote implementation status.

The other-repository research was distilled into [`program/`](program/). The
normalized core program is committed and pushed in `zl-linux:main` and contains:

- 906 product feature atoms;
- 609 normalized implementation targets;
- 174 research contracts;
- phases MP-00 through MP-20;
- driver, service, application, language, agent, release, and proof-gate maps.

The structure reconciliation also publishes the complete local research shelf
and its documentation audit. Moving and publishing them does not change
implementation status.

The static validator currently passes for program inventory, repository
identity, identifiers, dependencies and links in the isolated verification
tree. All 17 generated registry/status checks now agree on the current
154-input build identity. Historical artifact and runtime receipts retain their
original subject identities and are explicitly not current-build-bound. T-8
and T-9 are closed in [`.ultra/TENSIONS.md`](../.ultra/TENSIONS.md): the
evidence chain regenerates without joining unlike builds, and the recovery
policy is owned under `kernel/src/core/boot/` with focused host and four-lane
compile proof. Ignored local build/test outputs are outside Git and are not
part of the published layout. These checks prove planning consistency, not
product implementation or current runtime behavior.

## Implementation State

The codebase is substantial but the whole program is partial.

| Layer | Honest state |
|---|---|
| Research inventory and dependency plan | Complete and committed. |
| Branch/worktree integration discovered on 2026-08-24 | Complete and pushed. Historical refs/directories were not deleted. |
| Existing local implementation batch | Implemented across many host and QEMU lanes; see the dated receipt under `kernel/docs/evidence/`. |
| `program/FEATURE-STATUS.json` | Current identity-safe join covers all 906 rows; the generated file is the exact maturity-count authority. EV-004 has a complete current-host offline archive closure for 160 binaries and 104 source-package sets; EV-014 binds seven current host frame distributions and preserves their exact budget outcome in the generated receipt while retaining target and physical performance gaps; EV-015 has a current six-route exact-hash hardware matrix and hostile receipt validator while retaining zero physical runs; EV-016/EV-022 bind four exact current-artifact BIOS/native-UEFI screenshots and strict stable-region goldens while retaining all six variant and physical-display gaps; EV-017/KR-032 bind four exact 240-byte QEMU crash records across BIOS32/native-UEFI64 UD2, native-UEFI64 error-code GP and an IST1-contained double fault while retaining broader vectors, durable storage and recovery gaps; EV-019 binds current 32-position heap and 512-write page-table failure sweeps; EV-020 joins current host rejection evidence for 7 of 9 hostile-input families; EV-021 joins all seven host performance categories while preserving current host regressions and product-build/product-latency gaps in the generated registry. EV-012 has a current-build-bound host receipt. Seven boot rows, seven storage rows, BT-029 RTC, KR-001 bounded typed physical-page allocation, KR-002 heap diagnostics, KR-003 generated address map, KR-004 transactional heap/framebuffer mapping, KR-009 exact lower user-stack guard containment, KR-015 bounded process/anonymous physical-page accounting and quotas, KR-017 guarded TSS stack management, KR-022 scheduler, bounded user-process rows and KR-036 generated syscall admission join exact current evidence while retaining recovery, provider, VFS, unified service/cache/pinned/DMA/surface/kernel accounting, pressure/reclaim, memory above 1 GiB, direct kernel-stack overflow injection, SMP ownership, synchronization, persistent lifecycle, ABI compatibility, per-CPU scheduling, migration, suspend and physical gaps. |
| Post-restructure static and QEMU verification | Language/build gates, two-build reproducibility, raw BIOS, GRUB32 BIOS/UEFI, GRUB64 BIOS/UEFI and native UEFI64 passed for the current source identity. The complete app route sweep, 47 lifecycle cycles and 64 application-identity open-ready-close checks also passed. No physical boot was run. |
| Physical ThinkPad proof | Partial. QEMU and host evidence do not close panel, input feel, I219, USB power-cut, NVMe, suspend, or Intel promotion gates. |
| Complete MP-00 through MP-20 product | Not complete. The product-first order begins with Wave 1 performance/dependability and proceeds through boot, processes, IPC, services, hardware breadth, apps, browser, tooling, and release. |

## Research Shelf

The detailed audits from the other chat are retained inside this repository at
[`program/research/`](program/research/). Start with its
[`README.md`](program/research/README.md), then:

- [`CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md`](program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md);
- [`CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md`](program/research/CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md);
- [`IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`](program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md);
- [`DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`](program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md);
- [`VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md`](program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md);
- [`RESPONSIVENESS_RENDERING_AND_VISUAL_POLISH_DEEP_DIVE_2026-08-22.md`](program/research/RESPONSIVENESS_RENDERING_AND_VISUAL_POLISH_DEEP_DIVE_2026-08-22.md).

Those are research and architecture inputs. [`program/`](program/) is the
normalized execution authority for zlOS.

## Status Rule

Do not create `done/` and `half-done/` source folders. Completion changes as
evidence changes, while source ownership should remain stable. Use folders to
separate current docs, evidence, and archive; use the program ledgers and
receipts to record implementation maturity.
