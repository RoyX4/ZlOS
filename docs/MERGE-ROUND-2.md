# Merge round 2 — the twelve worktrees standing on 2026-08-20

Written 2026-08-20. Every number here was measured by a command in this repo on
that date; the commands are in § Re-measure. Nothing below is inferred from the
round-1 documents — those are cited only where they already answered a question.

Round 1 (the eleven-track landing of 2026-08-19) is in
[`MERGE-EVIDENCE.md`](MERGE-EVIDENCE.md). [`INTEGRATION-PLAN.md`](INTEGRATION-PLAN.md)
is superseded and should not be planned from. **This file is round 2.**

---

## The question this answers

> "I have a ton of worktrees and other chats with different commits — should I
> ask them all to open a PR?"

**No.** Four measured reasons, then what to do instead.

### 1. There is nothing to open a PR *against*

```
main (local)  3f00366        origin/main  55e0226        45 commits apart
```

Local `main` is **45 commits ahead of `origin/main`**, and **none of the twelve
branches exist on the remote at all**. `git ls-remote --heads origin` returns 17
names; every one is from round 1 or earlier. A PR opened today would diff a
branch against a `main` that is 45 commits stale — which is precisely the error
that got `INTEGRATION-PLAN.md` marked SUPERSEDED ("written against the wrong
base").

### 2. A PR is a review surface, and there is no reviewer

One-owner repo. Twelve PRs means self-approving **219 distinct files**. The
green checkmark would be produced by the same party that wrote the diff.

### 3. There is nobody left to ask

Of 40 sessions, exactly **one is running** (`Zlos overnight watchdog`, cwd
`~/Documents` — not a worktree). Only two sessions are bound to any of these
twelve branches (`compassionate-curie`, `recursing-ellis`) and **both are
stopped**. Ten of the twelve worktrees have no session behind them.

### 4. PRs assume the branches are independent. These are not.

**63 of 219 files are touched by two or more branches.** The top of the
collision set:

| File | Branches touching it |
|---|---:|
| `kernel/kernel.zl` | **9** |
| `docs/JOURNAL.md` | 7 |
| `TODO.md` | 7 |
| `kernel/wm.c` | 6 |
| `kernel/fb.c` | 5 |
| `freestanding/runtime_kernel.c` | 5 |
| `kernel/term.c`, `raw_boot.asm`, `browser.c`, `mkdisk.sh`, `check-memmap.sh`, `probe-files.py`, `HANDOFF.md` | 4 |

Twelve PRs open against `main` means merging the first invalidates the other
eleven. You would rebase twelve times and resolve `kernel.zl` nine times.

### 5. And round 1 already measured the answer

`MERGE-EVIDENCE.md` § 4, from the eleven-track landing:

> Ordering was measured four ways: 163 / 181 / 185 / 190 conflicted-file events.
> **Ordering buys ~10%.** It is not the lever.

The lever was resolving the memory map and the build contract as their own
steps. And the damage that actually landed was **silent** — merges that came out
clean and then failed: two snake games, `wm_focus` as a getter on one side and a
setter on the other, eight app-id collisions, `LINE_BUF`/`DISK_SCRATCH` on one
address. A PR's green tick catches none of that class.

---

## The twelve, measured

`git fetch` run first. `AHEAD`/`BEHIND` are vs local `main` (`3f00366`).
`DIRTY` is `git status --porcelain | wc -l` in that worktree.

| Branch | Worktree | Ahead | Behind | Files | Dirty |
|---|---|---:|---:|---:|---:|
| `claude/compassionate-curie-a0599c` | `.claude/worktrees/…` | 68 | 0 | 98 | 11 |
| `fleet/audit-2026-08-19` | `zl-linux-fleet` | 31 | 0 | 35 | 11 |
| `desktop/browser-next` | `zl-browser2` | 21 | **83** | 61 | 27 |
| `integration/codex-threads` | `zl-linux-integration` | 20 | 0 | 29 | 7 |
| `claude/recursing-ellis-82c8e8` | `.claude/worktrees/…` | 7 | 0 | 47 | 1 |
| `docs/fix-stale-links` | `zl-linux-docfix` | 5 | 5 | 6 | 2 |
| `desktop/v10-look` | `zl-linux-v10look` | 5 | 4 | 8 | 7 |
| `desktop/app-suite` | `zl-linux-apps` | 3 | 4 | 14 | 7 |
| `desktop/files-app` | `zl-linux-files-app` | 2 | 7 | 7 | 7 |
| `desktop/v10-recovered` | `zl-linux-v10rec` | 2 | 18 | 23 | 2 |
| `desktop/storage-recovered` | `zl-linux-storerec` | 1 | 8 | 11 | 2 |
| `files-app` | `~/.cursor/worktrees/…` | **0** | 7 | 0 | 11 |

Plus `main`'s own worktree: **42 dirty**, of which 8 are `.cache/clangd/` and
`.cursor/` noise and **34 are real** (`CLAUDE.md`, `README.md`,
`docs/STATE-OF-THE-PROJECT.md`, `kernel/kernel.zl`, `kernel/term.c`,
`freestanding/runtime_kernel.c`, the `probe-*.py` set, `run_tests.sh`, and
untracked `docs/ROAD-TO-TEN.md`, `verify_selfhost.sh`, four `kernel/hosttest/`
binaries).

**Totals: 165 commits, 219 distinct files, 129 uncommitted files, 0 pushed.**

Two facts fall straight out of that table:

- **`files-app` is already in `main`** (0 ahead; `git branch --merged main` lists
  it). Nothing to land. Remove the worktree.
- **`desktop/browser-next` is 83 behind.** It forked before round 1 landed. It
  is the only branch that predates the eleven-track merge, and it is the one
  that will hurt.

---

## What to do instead

One integration branch, landed sequentially, gated after each step. Not twelve
PRs.

### Step 0 — get a restore point off this disk (do this first, alone)

Right now 165 commits exist on exactly one SSD. Round 1's rollback mechanism was
the `prelanding/*` tags **and they were pushed**; that is why round 1 was
recoverable. Repeat it:

1. `.gitignore` the noise: `.cache/clangd/`, `.cursor/`, and the four untracked
   `kernel/hosttest/` binaries (`fbtext`, `palette`, `termwrap`, `walltest` —
   build outputs; `kernel/hosttest/inputtest_feel` and `wmtest_feel` are already
   tracked and dirty, which is its own bug).
2. Review and commit the 34 real changes on `main`.
3. `git push origin main` — closes the 45-commit gap.
4. Tag and push a rollback point per branch: `prelanding2/<branch>`.

Until step 0 is done, every later step is unrecoverable.

### Step 1 — drop the dead worktree

`git worktree remove ~/.cursor/worktrees/zl-linux/files-app` — 0 ahead, already
merged. (Its 11 dirty files are Cursor scratch; check before removing.)

### Step 2 — land the four that are 0 behind, no rebase needed

In this order, smallest first, because each one shrinks the surface the next has
to fight:

1. `claude/recursing-ellis-82c8e8` — 7 commits, memory-model work
2. `integration/codex-threads` — 20 commits, wm snap preview
3. `fleet/audit-2026-08-19` — 31 commits, the http overflow/deadlock fixes
4. `claude/compassionate-curie-a0599c` — 68 commits, 98 files, the ds.html clone

**#4 last, and read it before merging.** Its own tip commit says
*"checkpoint: the wave's work so far, again uncommitted and ag…"* — that is a
session admitting it stopped mid-flight. It is also the only branch big enough
to be a round-1-scale landing on its own.

### Step 3 — rebase, then land the seven that are behind

Ascending by `BEHIND`, so each rebase is onto a `main` that already contains its
neighbours: `v10-look` (4) → `app-suite` (4) → `fix-stale-links` (5) →
`files-app` (7) → `storage-recovered` (8) → `v10-recovered` (18) →
**`browser-next` (83, last)**.

### The gate to run after every single merge

Round 1's lesson was that the breakage is silent, so a merge is not landed until
these pass. All three exist and were confirmed present on 2026-08-20:

```bash
kernel/check-memmap.sh     # address collisions
kernel/check-zl-calls.sh   # zl call sites with no builtin — zl has no linker
kernel/verify-iso.sh       # boots zlOS.iso, not kernel.elf
```

`check-memmap.sh` is in better shape than `MERGE-EVIDENCE.md` § 3 says: that
document calls the address sweep "specified, not written", but the file now has
a real discovery sweep at line 57
(`grep -oP '^\K[A-Z_]+(?=\s*=\s*0x0[0-9A-Fa-f]{5,})'`) *alongside* the legacy
nine-name list at line 55. **§ 3 item 1 is stale in the pessimistic direction.**
Likewise `DISK_SCRATCH` is genuinely `0x02040000` now (`kernel.zl:1363`) — the
round-1 correction was applied, not just written.

**`land-gate.sh` no longer exists** anywhere in the tree, under either the
`kernel/` or root path `MERGE-EVIDENCE.md` refers to. Round 1 found and fixed a
defect in it (the `tail` pipe that made `$?` always zero) and the fixed script is
now gone. Either recover it from a round-1 commit or accept that the three
scripts above are run by hand.

---

## Weakest link

Same as round 1, and worse: **`kernel/kernel.zl`, now contested by nine
branches** instead of eight. zl has no compiler diagnostics, no linker, no type
checker and no static asserts, so the entire mechanical backstop is
`check-zl-calls.sh` plus booting the ISO. Everything else is a person reading
carefully.

Second weakest: `desktop/browser-next` at 83 behind is the one branch that never
saw round 1. Its 61 files were written against a tree that no longer exists.
Landing it last is right; expect it to cost more than the other eleven combined.

## Re-measure

```bash
cd ~/Documents/repos/zl-linux && git fetch origin
git worktree list
# ahead/behind/pushed per branch
git worktree list --porcelain | grep '^branch ' | sed 's|^branch refs/heads/||' |
while read -r b; do
  printf "%-34s %4s %4s %4s\n" "$b" \
    "$(git rev-list --count main.."$b")" \
    "$(git rev-list --count "$b"..main)" \
    "$(git diff --name-only main..."$b" | wc -l)"
done
# collision set
for b in $(git worktree list --porcelain | grep '^branch ' | sed 's|^branch refs/heads/||'); do
  git diff --name-only main..."$b"; done | sort | uniq -c | sort -rn | awk '$1>1'
```
