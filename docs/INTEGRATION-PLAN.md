# Integration plan — landing the eight parallel tracks

Written 2026-08-18 against `main` = `44346d6`. Every number below was measured
with `git merge-tree` (read-only, no working tree touched); the commands are at
the bottom so you can re-run them after anything moves.

## The situation, in one picture

Eight sessions have been building eight different floors of the same building at
the same time — and each one poured its own foundation, because when they
started, the floor below didn't exist yet. Nobody is *behind*; everybody forked
from the same `main` and walked forward. The problem is not that the work
diverged. It's that seven of the eight foundations have to be demolished when
the building is assembled, and the demolition is done by whoever merges last.

## What is actually out there

All eight are worktrees of one repo (`zl-linux.git`). Merge-base for every one
of them is `main` (`44346d6`), and **every branch is 0 commits behind**.

| Branch | Worktree | Commits ahead `main` | Files changed | Uncommitted |
|---|---|---:|---:|---:|
| `desktop/feel-and-control` | `repos/zl-feel` | 47 | 50 | 1 |
| `desktop/system-track` | `repos/zl-system` | 46 | 63 | 5 |
| `fix/dma-map-hid-arena` | `zl-linux/.claude/worktrees/dma-map-fix` | 42 | 69 | — |
| `desktop/overnight-compositor` | `repos/zl-linux` | 41 | 61 | 69 |
| `lang/value-16` | `repos/zl-value16` | 40 | 62 | 0 |
| `desktop/apps-in-windows` | `repos/zl-apps` | 39 | 52 | 4 |
| `desktop/browser` | `repos/zl-browser` | 39 | 60 | 3 |
| `desktop/exec-track` | `repos/zl-exec` | 38 | 56 | 14 |

**332 commits across 128 distinct files, none of it pushed.** `main` itself is
24 commits ahead of `origin/main`. The only branch that exists on the remote is
`claude/quirky-pare-05454c`. Everything else lives on one disk.

## The measured cost of merging

Each branch merges into `main` cleanly *on its own* — that is trivially true,
since `main` is the merge-base and every branch is a fast-forward. It tells you
nothing. The real number is what the **second through eighth** merges cost.

Three orders simulated end to end:

| Order tried | Conflicted-file events |
|---|---:|
| deepest layer first (`value-16` → compositor → …) | **181** |
| compositor first | 185 |
| smallest branch first | 190 |

The spread is 5%. **There is no clever ordering that saves this** — stop looking
for one. What the order *does* buy is which track pays, and that matters, see
below.

Where the 181 events land:

| Category | Events | Character |
|---|---:|---|
| Source (`.c` / `.h` / `.zl`) | 116 | real, needs someone who knows the code |
| Build scripts, `.gitignore`, probe `.py` | 46 | mechanical, resolvable by policy |
| `docs/*.md` | 19 | pure noise — every track wrote its own HANDOFF |

The eleven files that blow up in 7 of the 7 merges:

```
kernel/wmglue.c   kernel/wm.c    kernel/ui.h      kernel/term.c
kernel/kernel.zl  kernel/input.c kernel/fb.c      freestanding/runtime_kernel.c
kernel/hosttest/{wmtest,wmshot,inputtest}.c
```

`kernel/wm.c`, `kernel/ui.c`, `kernel/ui.h`, `kernel/term.c` and
`kernel/wmglue.c` **do not exist on `main` at all.** Eight branches each created
their own copy — `wm.c` is 817 lines on three of them, 895 on another, 1017 on
two more, 1091 on `feel-and-control`. That is not a merge conflict, it's eight
independent implementations of the same file with a shared ancestor that was
never committed anywhere.

## Waiting makes it worse

The instinct to let everything finish and then reconcile once is wrong here, and
the repo already contains the proof. From `desktop/overnight-compositor`'s own
`.ultra/STATE.md`:

> "I did not do it because `kernel.zl` was mid-flight in another session all
> night — its `mouse_x` builtin was rewired underneath me *while I was
> measuring it*"

That session **could not finish its highest-leverage task** because another
track owned the file. Every extra day of parallel work adds commits to the same
eleven hot files, and the whole cost gets paid at the end by one session that
wrote none of them. Divergence compounds; it does not settle.

## The plan

### 0. Push everything. Today. Before anything else.

332 commits of work exist on exactly one disk with no remote copy. This is the
largest actual risk in the picture and it has nothing to do with merging.

```bash
cd ~/Documents/repos/zl-linux && git push origin main desktop/overnight-compositor desktop/apps-in-windows desktop/browser desktop/exec-track desktop/feel-and-control desktop/system-track lang/value-16 fix/dma-map-hid-arena
```

Do this even if the integration order below changes. It is additive and
reversible.

### 1. Freeze the noise categories by policy, not by merging

65 of the 181 conflict events are docs and build scripts. Kill them with a rule
instead of a resolution:

- `main` owns `kernel/HANDOFF.md`, `docs/`, `kernel/docs/*.md`, `build*.sh`,
  `mkdisk.sh`, `mkiso.sh`, `.gitignore`.
- Tracks keep their notes in `kernel/docs/<track>.md` only — one file per track,
  which nobody else touches.
- On every landing, resolve those paths with `-X ours` from the integration side
  and re-apply intended build-script changes by hand.

That is a 36% reduction in conflict volume before a single merge runs.

### 2. Land one at a time, and rebase the rest each time

Serialize the queue instead of doing one eight-way merge. After each landing,
every unlanded track rebases onto the new `main`. Each track then resolves
conflicts **in its own code, in its own session, with the context that wrote
it** — instead of one final session resolving 116 source conflicts across eight
codebases it has never read.

Land order, deepest layer first — this is both the lowest measured total (181)
and the right architectural direction, since everything above compiles against
what's below:

| # | Track | Why here |
|---|---|---|
| 1 | `lang/value-16` | changes `Value` representation; every track compiles against it |
| 2 | `desktop/overnight-compositor` | creates `wm.c` / `ui.c` / `ui.h` — the substrate the other six each cloned |
| 3 | `fix/dma-map-hid-arena` | **1 conflict** against #2 (`kernel/fb.c`); same lineage, land as a pair |
| 4 | `desktop/feel-and-control` | input sits directly on the compositor |
| 5 | `desktop/system-track` | clipboard / zlfs on top of the window manager |
| 6 | `desktop/apps-in-windows` | apps need windows to be in |
| 7 | `desktop/browser` | an app, plus its own TCP work |
| 8 | `desktop/exec-track` | loads code the kernel wasn't built with — wants everything else stable |

Steps 2 and 3 are effectively one landing: measured at **one conflicted file**.

### 3. The gate is a boot, not a merge

A clean merge proves nothing here. `kernel.zl`, `wm.c` and `runtime_kernel.c`
from three different tracks can merge without a single conflict marker and still
not compile — and if they compile, still not boot.

After each landing, before the next track rebases:

```bash
./build.sh && kernel/hosttest/build.sh   # toolchain + assertions
kernel/verify.sh                         # ~1 min, BIOS vs golden transcript
kernel/verify-iso.sh                     # ~1.5 min, BIOS *and* UEFI
kernel/verify-raw.sh                     # 1–3 min, our own bootloader
```

Per `CLAUDE.md`: start these in the background and don't run them alongside an
agent fan-out. **A landing that doesn't boot gets reverted, not patched
forward.** Land #N+1 only against a green `main`.

### 4. Integrate in a dedicated worktree

`~/Documents/repos/zl-main` is checked out on `main` for exactly this. Do the
landings there. Leave the eight track worktrees alone so a still-running session
never finds its tree rearranged underneath it — which is the failure this whole
document exists to stop.

## What has not been verified

Stated plainly so nobody reads more confidence into this than it earned:

- **No merged combination has been built or booted.** Every number above is
  `git merge-tree` — text-level conflicts only. Semantic breakage (two tracks
  both adding a `wm_focus()` with different signatures, `Value` shrinking to 16
  bytes under code written for 64) will not appear in any of these counts and is
  the real cost.
- The 181/185/190 figures count *conflicted files per merge step*, not lines and
  not difficulty. One `kernel.zl` conflict may outweigh twenty in `bench/`.
- Uncommitted work in the eight worktrees (69 dirty files in `zl-linux` alone)
  is not in any of these measurements.

**The weakest link is step 2's assumption that a track can rebase itself.** A
track that cloned `wm.c` wholesale is not rebasing — it is deleting its own file
and re-applying its intent onto someone else's. For `feel-and-control` (1091
lines) and `system-track` (895) that is closer to a port than a merge, and it
should be budgeted as one.

## Re-running the measurements

```bash
cd ~/Documents/repos/zl-linux
# divergence
for b in $(git branch --format='%(refname:short)'); do \
  echo "$b $(git rev-list --count main..$b) ahead, $(git rev-list --count $b..main) behind"; done
# sequential conflict simulation, read-only
base=main; for b in lang/value-16 desktop/overnight-compositor fix/dma-map-hid-arena \
  desktop/feel-and-control desktop/system-track desktop/apps-in-windows \
  desktop/browser desktop/exec-track; do \
  out=$(git merge-tree --write-tree --name-only $base $b); \
  echo "$b -> $(echo "$out" | tail -n +2 | grep -c .) conflicts"; \
  base=$(git commit-tree $(echo "$out" | head -1) -p $base -m sim); done
```
