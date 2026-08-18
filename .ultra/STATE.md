# STATE — zl-main, the integration worktree

Updated 2026-08-18 ~21:10 local. (Box clock drifts; prefer
`gh api repos/RoyX4/zl-linux --jq .pushed_at` for real times.)

## Objective

Land eight parallel zlOS tracks onto `main` without silently losing work, and
without shipping the class of bug that merges clean and fails at runtime.

**Success metric:** `main` builds, links, and passes the boot gates, with a named
resolution recorded for all 55 multi-touch files.

## Where the increment stands

### Done and verified

- **Safety net complete.** 0 branches unpushed (was 10). 130 MB of per-worktree
  tars at `~/zlos-freeze-20260818-2054/`, a verified 13 MB bundle at
  `~/zl-freeze-20260818-2056.bundle`, 15 `prelanding/*` tags pushed, and 101
  uncommitted files captured as `refs/wip/*` commits (also pushed) **without
  touching any working tree**.
- **Evidence written.** `docs/MERGE-EVIDENCE.md` — 37 sessions re-read, every
  branch measured. Supersedes `docs/INTEGRATION-PLAN.md`.
- **L0 landed** — `claude/amazing-robinson-19793a`. Untracks `kernel/_genefi.c`.
- **L1 landed** — the 33-commit shared spine (`d61a481`). `main` now carries
  `kernel/wm.c`, `ui.c`, `ui.h`, `wmglue.c`, `term.c` for the first time.
- **L1 follow-up landed** — `da34635` cherry-picked. See TENSIONS T-1.

Effect on divergence, measured:

| Track | behind main's old base | behind main now |
|---|---:|---:|
| `lang/value-16` | 41 | 8 |
| `fix/dma-map-hid-arena` | 42 | 9 |
| `desktop/browser` | 44 | 11 |
| `desktop/apps-in-windows` | 43 | 13 |
| `desktop/overnight-compositor` | 48 | 15 |
| `desktop/feel-and-control` | 48 | 15 |
| `desktop/exec-track` (carries `system-track`) | 40 | 28 |

Most of the apparent 40–50 commit gap was shared history `main` did not have.

### In flight

- Boot gate running against `main @ 91bbad8` via `gates/land-gate.sh`. The
  compile steps pass; the QEMU steps are **waiting on the load guard** — the box
  is at loadavg 13.8, driven by Cursor, not by this work.

### Next, in order

1. Finish the gate on `main @ 91bbad8`. Do not advance on red.
2. Write the three checkers in `docs/MERGE-EVIDENCE.md` §3 (address table,
   symbol table, builtin CALL/REG table) **before** any further landing. They are
   the only detectors for the silent-failure class.
3. Rewrite `kernel/check-memmap.sh` — it iterates a hardcoded nine-name list with
   no `DISK_SCRATCH` and no discovery, so it cannot catch the collision it exists
   to catch.
4. **L2 — `desktop/apps-in-windows`.** Must precede the five tracks that append
   `gcc` lines to the build scripts, or their appends land and are then deleted.
   Requires adding 17 `.c` files to `kernel/SOURCES`.
5. L3 — `desktop/overnight-compositor`. The expensive one: `kernel.zl`, 13
   conflict hunks / 898 conflicted lines against apps.

## Constraints that bind this work

- 15 worktrees share one `.git`. Never run destructive git while another session
  holds a worktree.
- 8 cores / 15 GB, QEMU under TCG. Gates run sequentially and backgrounded,
  never alongside an agent fan-out. Guard on **available memory** as well as
  loadavg — the documented prior kill here was an OOM.
- `git add -A` in `zl-linux` would stage 226 MB of untracked `kernel/exercise-out`.
  Always use an explicit pathspec.
