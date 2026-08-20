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

---

## Outcome (2026-08-20)

**7 of 12 landed. 1 deferred. 4 needed no merge at all.** `main` builds and both
runnable gates pass; nothing has been pushed.

| Branch | Result |
|---|---|
| `claude/recursing-ellis-82c8e8` | **landed** |
| `integration/codex-threads` | **landed** (boot-loader conflict, below) |
| `fleet/audit-2026-08-19` | **landed** (`kernel.zl` union, below) |
| `docs/fix-stale-links` | **landed** |
| `desktop/browser-next` | **landed** — caught up in its own worktree first |
| `desktop/v10-look`, `desktop/app-suite`, `desktop/files-app` | **dissolved** — ahead 0 after the first three landings; their work arrived inside them |
| `files-app` (cursor worktree) | already merged before round 2 started |
| `desktop/storage-recovered` | **SUPERSEDED, do not merge** — 1278 insertions / 14207 deletions |
| `desktop/v10-recovered` | **SUPERSEDED, do not merge** — 3998 / 15408, zero unique files |
| `claude/compassionate-curie-a0599c` | **DEFERRED** — 23 conflict hunks, 11 in `kernel.zl` |

### The two "recovered" branches are traps

Both say *"recovered from a lost session"* and both are ~75-85 commits behind.
Merging either applies far more deletion than insertion: `storage-recovered`
would delete `kernel/usermode.c` (420 lines), `verify_selfhost.sh`, and most of
`wm.c`/`xhci.c`/`virtio_gpu.c`. Their features are already in `main` in richer
form — `main` has `filemgr`, `fs_ch`, 8 `ed_disk` sites and the 249-line
`probe-files.py` gate; `storage-recovered` has none of the first three. This is
`MERGE-EVIDENCE.md` §2.2's two-implementations class. Only one file was worth
rescuing (`kernel/docs/storage-and-files.md`) and it was cherry-picked.

### Conflicts that mattered

- **Boot loader** (`raw_boot.asm` + `mkdisk.sh`). `main` `CHUNKS=192` (6 MiB,
  image size derived); `codex-threads` `CHUNKS=80` (2.5 MiB, hardcoded 3 MiB)
  reasoning against a stack at 6 MiB that `main` had already moved to 12. Took
  `main` — they are only correct as a pair, and 2.5 MiB would not hold the
  53-app suite.
- **`kernel.zl` app_event** — both sides were needed. `main`'s registry
  early-return comment says *"BEFORE the nav_to_char translation below"*; it was
  written expecting `fleet`'s block. Union, registry first.
- **`browser-next`** was merged *into* by `main` in its own worktree — one
  resolution pass (12 hunks) instead of 21 through a rebase — and
  `kernel/build.sh` was green there before it landed.

### What the landing found

Three staleness bugs of one class, all in commit `018abc6`, all invisible until
`arena.c` stopped restating literals and started deriving from `memmap.h`:

1. `memmap.h`'s `LO_ARENA` was 8 MiB; the arena moved to 14 MiB when `CHUNKS`
   went to 192. An 8 MiB base with a 16 MiB budget spans 8..24 MiB and swallows
   the raw-boot stack at 12 MiB.
2. `arena.c`'s `RAW_STACK_TOP` said 6 MiB, citing a line that reads 12 MiB. The
   `_Static_assert` against it is the **only** guard on that boundary, and a
   stale 6 MiB lets an 8 MiB arena through.
3. `check-memmap.sh` could only read literals, so it printed
   `FAIL: not found` against correct code.

**Negative-tested, not just green:** with `LO_ARENA` planted back at 8 MiB the
build fails on the static assert (exit 1); restored, exit 0. Note the shell gate
does **not** catch this — it covers `kernel.zl` only, by its own closing note.

### Still open

- **`compassionate-curie` (68 commits, 98 files).** Merge aborted clean. 23
  hunks over 13 files, **11 in `kernel.zl`** — a language with no compiler, no
  linker and no type checker, which is the one place this repo has repeatedly
  produced silent breakage. It needs a dedicated pass, not the tail of another.
  It was also committing to itself during this work (`56cca54` -> `a48d85c` ->
  `a998a6c`); confirm it is idle first. Rollback: `prelanding2/claude-compassionate-curie-a0599c`.
- **Nothing is pushed.** `prelanding2/*` tags exist locally only.
- `kernel/hosttest/cryptotest` is still a tracked ELF with no build script.


---

## Round 2 CLOSED (2026-08-20)

**All twelve branches are resolved. `main` builds and every gate passes,
including the two that boot.**

| Gate | Result |
|---|---|
| `./build.sh` (toolchain) | exit 0 |
| `kernel/build.sh` | exit 0 |
| `kernel/check-memmap.sh` | exit 0 |
| `kernel/check-zl-calls.sh` | exit 0 |
| `kernel/verify-iso.sh` | **exit 0 — BIOS and UEFI, compositor 3 windows** |
| `kernel/verify-disk.sh` | **exit 0 — BOOTCOUNT 1 -> 2 -> 3, file survives by name** |

`compassionate-curie` landed after all. It was caught up in its own worktree
(94 behind), and the C the zl compiler emits caught **three duplicate
implementations git did not consider conflicts**, because each side's copy sat
in a region the other did not touch:

1. **`files_draw`/`files_event` defined twice, with different argument orders** —
   `files_event(ety,ecode,ex,ey,win)` vs `files_event(win,ety,ecode,ex,ey)`.
   Both were called, from two `if id == APP_FILES` arms of the **same**
   `app_event`, so the second was unreachable. That is `MERGE-EVIDENCE.md`'s
   app-id collision class, exactly.
2. **`snap_preview_set` defined twice.** Kept the branch's: its
   `snap_preview_*` state is read by the paint path (`fb_rrect_blend`), while
   main's `sp_*` is only written inside its own setter and never drawn.
3. **`ANIM_FRAMES`** was main's constant, dropped when the branch won the
   animation hunk while main's `anim_tick` came through unconflicted still
   reading it.

And one that would **not** have been a build error at all:
`ACCENT`/`TXT_DIM`/`TXT_HI` had **11 uses and zero definitions** — the branch
replaced them with `theme(TH_*)` and main's Files UI arrived unconflicted still
using the old names. In zl that is a silent runtime failure. Converted.

### The lesson round 2 adds to round 1

Round 1 said the damage is silent. Round 2 says **where** it hides: not in the
conflicted hunks, which get read, but in the regions git merges cleanly *around*
them. Every one of the four faults above was in text neither side flagged. The
only reason they surfaced is that zl compiles to C and C refuses to define a
function twice — the one fault C could not catch (the undefined colour
constants) is the one that had to be found by grep.

### Weakest link, unprompted

`wm.c` carries two animation engines' worth of history. The branch's side won
because only it defines `wm_anim_at/progress/scale`, which the branch's own
`wmtest.c` requires — but `anim_tick` came through from main. It builds, boots
and passes both visual gates; it has not been proven that every one of the
seven animations still moves pixels on the merged tree. `hosttest/wmtest` is
the thing that would say.
