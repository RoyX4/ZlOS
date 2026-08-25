# Project Status

Checked 2026-08-25. This page separates repository state, planning state, and
implementation state. They are not the same thing.

## Read This First

1. [`program/README.md`](program/README.md) is the complete destination and
   dependency program.
2. [`program/PRODUCT-IMPLEMENTATION-ORDER.md`](program/PRODUCT-IMPLEMENTATION-ORDER.md)
   is the human delivery order.
3. [`EXECUTION-ROADMAP.md`](EXECUTION-ROADMAP.md) is the nearer-term systems and
   performance queue.
4. [`STATE-OF-THE-PROJECT.md`](STATE-OF-THE-PROJECT.md) is a detailed 2026-08-19
   audit. It is evidence-rich but older than the final integration.
5. [`REMAINING-WORK.md`](REMAINING-WORK.md) is the post-integration H0-H3 map.

## Repository State

| Repository or checkout | Commit state | Meaning |
|---|---|---|
| `RoyX4/zl-linux:main` | Structure and verifier checkpoint `a5c6f4e4306d`; current docs refresh `b8e5f8a45164` | The complete restructure is published. GitHub docs, repository gates and all four QEMU boot jobs passed at the verifier checkpoint. This is repository closure and QEMU evidence, not product or physical-hardware completion. |
| This reconciliation worktree | Clean `codex/full-program`; published baseline `b8e5f8a45164` plus local documentation/checker cleanup | Isolated at `/home/roy/Documents/repos/zl-linux-final`. The structure migration, research shelf and verifier repairs are pushed. The later documentation-only cleanup is intentionally withheld from `main` while boot CI is deferred. The original dirty checkout was not rewritten. |
| Older structure worktree | `codex/full-restructure`, based on `85fcdf542b9c` | Preserved at `/home/roy/Documents/repos/zl-linux-structure` as the older migration input. It is not the active finalization tree. |
| Original shared checkout | Dirty `codex/local-zlos-snapshot-2026-08-24` at `/home/roy/Documents/repos/zl-linux` | Preserved migration inputs and a concurrent untracked display-state audit; it is not the publication authority and was not rewritten. |
| `zl-linux-merge-all` | Clean at `4b9883123682` | Local checkout of the pushed integration head and the best place to inspect final merged state. |
| `RoyX4/zl:main` | Pushed at `6f53115` | Separate language repository. Its local `main` matches the remote, but the working directory has untracked local output. |
| `zl-merge-all` | Local head `5207d5c`, five commits ahead of `RoyX4/zl:main` | The raw GitHub/other-OS research is committed locally. Two repository-pattern commits are on `origin/codex/github-repo-patterns`; the complete merge head is not on remote `main`. |
| `zl-starred-sources` | Not a Git repository | Local source mirror used for research. It is evidence input, not a committed project snapshot. |

The old `zl-apps`, `zl-browser`, `zl-exec`, `zl-feel`, `zl-main`, `zl-system`,
and `zl-value16` directories are no longer present. Their historical worktree
roles remain documented under [`evidence/`](evidence/). The final integration
receipt is [`MERGE-ALL-2026-08-24.md`](MERGE-ALL-2026-08-24.md).

### Registered zl-linux Worktrees

Checked 2026-08-25. These trees are clean unless noted, and every listed HEAD is
already an ancestor of `origin/main`; none contains a unique unmerged commit.

| Worktree branch | HEAD | Remote/working state |
|---|---|---|
| `files-app` | `6b40dfb61cee` | Clean; matches `origin/files-app`. |
| `secret/desktop-foundry` | `1c96bfebe361` | Clean; matches `origin/secret/desktop-foundry`. |
| `design/desktop-redesign` | `85fcdf542b9c` | Clean; no upstream; points at the older preserved roadmap commit. |
| `codex/master-program-foundation` | `11c711eab261` | Clean; matches `github/codex/master-program-foundation`. |
| Three detached Claude worktrees | `bb1d623b9b49`, `84cecae63019`, `b8a00ec45ac1` | Clean and preserved; their commits are already in `origin/main`. |
| `codex/local-zlos-snapshot-2026-08-24` | `85fcdf542b9c` | Dirty shared checkout containing the uncommitted cleanup inputs; do not overwrite it. |
| `codex/full-restructure` | `85fcdf542b9c` base | Preserved older isolated structure migration. |
| `codex/full-program` | Published through `b8e5f8a45164`; local documentation/checker commits ahead | Clean. The unpublished commits do not claim runtime or hardware evidence and remain local solely to avoid triggering automatic boot CI. |

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

The static validator currently passes for inventory, repository identity,
identifiers, dependencies, links, and generated-file freshness in the isolated
verification tree. That proves the plan is internally complete. It does not
prove the product is implemented.

## Implementation State

The codebase is substantial but the whole program is partial.

| Layer | Honest state |
|---|---|
| Research inventory and dependency plan | Complete and committed. |
| Branch/worktree integration discovered on 2026-08-24 | Complete and pushed. Historical refs/directories were not deleted. |
| Existing local implementation batch | Implemented across many host and QEMU lanes; see the dated receipt under `kernel/docs/evidence/`. |
| Post-restructure static and QEMU verification | GitHub docs, repository gates, BIOS32, raw BIOS, GRUB BIOS/UEFI and native UEFI all passed at `a5c6f4e4306d`. No physical boot was run. |
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
