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
5. `REMAINING-WORK.md` on pushed `main` is the post-integration H0-H3 map; it
   is not present in this older cleanup base.

## Repository State

| Repository or checkout | Commit state | Meaning |
|---|---|---|
| `RoyX4/zl-linux:main` | Pushed at `4b9883123682` | Final integration head. Every currently advertised `origin/*` topic branch is an ancestor of this commit. This is branch closure, not product completion. |
| This cleanup worktree | `codex/full-restructure`, based on `85fcdf542b9c` | Isolated at `/home/roy/Documents/repos/zl-linux-structure`. The source/docs/formatting cleanup is uncommitted and the base remains 32 commits behind pushed `main`. The original dirty checkout was not rewritten. |
| `kernel/docs/display-state-2026-08-25.md` | Untracked in this checkout | A concurrent read-only display audit. Preserve it, but do not describe it as committed or pushed. |
| `zl-linux-merge-all` | Clean at `4b9883123682` | Local checkout of the pushed integration head and the best place to inspect final merged state. |
| `RoyX4/zl:main` | Pushed at `6f53115` | Separate language repository. Its local `main` matches the remote, but the working directory has untracked local output. |
| `zl-merge-all` | Local head `5207d5c`, five commits ahead of `RoyX4/zl:main` | The raw GitHub/other-OS research is committed locally. Two repository-pattern commits are on `origin/codex/github-repo-patterns`; the complete merge head is not on remote `main`. |
| `zl-starred-sources` | Not a Git repository | Local source mirror used for research. It is evidence input, not a committed project snapshot. |

The old `zl-apps`, `zl-browser`, `zl-exec`, `zl-feel`, `zl-main`, `zl-system`,
and `zl-value16` directories are no longer present. Their historical worktree
roles remain documented under [`evidence/`](evidence/). The final integration
receipt is `MERGE-ALL-2026-08-24.md` on pushed `main`; it is not present in this
older cleanup base.

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
| `codex/full-restructure` | `85fcdf542b9c` base | This isolated uncommitted structure cleanup. |

## Whole Implementation Program

The other-repository research was distilled into [`program/`](program/). That
suite is committed and pushed in `zl-linux:main` and contains:

- 906 product feature atoms;
- 609 normalized implementation targets;
- 174 research contracts;
- phases MP-00 through MP-20;
- driver, service, application, language, agent, release, and proof-gate maps.

The recorded validator passed for inventory, identifiers, dependencies, links,
and generated-file freshness. That proves the plan is internally complete. It
does not prove the product is implemented.

## Implementation State

The codebase is substantial but the whole program is partial.

| Layer | Honest state |
|---|---|
| Research inventory and dependency plan | Complete and committed. |
| Branch/worktree integration discovered on 2026-08-24 | Complete and pushed. Historical refs/directories were not deleted. |
| Existing local implementation batch | Implemented across many host and QEMU lanes; see the dated receipt under `kernel/docs/evidence/`. |
| Full landing gate at final integration | Stopped after the explicit push request. Later host inventory, QEMU boots, and joined evidence refresh were not completed in that run. |
| Physical ThinkPad proof | Partial. QEMU and host evidence do not close panel, input feel, I219, USB power-cut, NVMe, suspend, or Intel promotion gates. |
| Complete MP-00 through MP-20 product | Not complete. The product-first order begins with Wave 1 performance/dependability and proceeds through boot, processes, IPC, services, hardware breadth, apps, browser, tooling, and release. |

## Raw Research Shelf

The detailed audits from the other chat are under:

`/home/roy/Documents/repos/zl-merge-all/docs/research/starred-repositories/`

Start with its `README.md`, then:

- `CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md`;
- `CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md`;
- `IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`;
- `DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`;
- `VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md`;
- `RESPONSIVENESS_RENDERING_AND_VISUAL_POLISH_DEEP_DIVE_2026-08-22.md`.

Those are research and architecture inputs. [`program/`](program/) is the
normalized execution authority for zlOS.

## Status Rule

Do not create `done/` and `half-done/` source folders. Completion changes as
evidence changes, while source ownership should remain stable. Use folders to
separate current docs, evidence, and archive; use the program ledgers and
receipts to record implementation maturity.
