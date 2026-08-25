# The working rule: everything lands on `main`, the same day

Zac's call, 2026-08-19, after the eleven-track merge. Written down because the
alternative was tried at length and the cost is measured.

## The rule

**Every session lands its work on `main` before it finishes.** Not "when it's
polished", not "when the track is complete" — the same day it is written.

## Why, with the numbers

Eight tracks ran in parallel for about a week. Each forked from a `main` that
had no compositor in it, so **each one built its own substrate**. The bill:

- `kernel/src/graphics/windowing/wm.c` existed on eight branches and on `main` not at all
- 55 files were touched by two or more tracks
- **eight** app-id collisions, every one silent — two dispatch arms matching the
  same id, the second unreachable
- two snake games, two resize grips, two frame timers, two names for the console
  mute, and `wm_focus` meaning *opposite things* on two branches
- a memory map where the framebuffer back buffer would have grown over the stack
  of every application processor

None of that is a merge problem. It is a **duration** problem. Every one of those
collisions exists because two people solved the same problem a week apart without
seeing each other. Land the same day and the second person sees the first one's
code and doesn't write it again.

`docs/evidence/MERGE-EVIDENCE.md` is the full account.

## What this does NOT mean

**It does not mean "never use a branch."** A branch that lives an hour costs
nothing and is invisible. Two agents editing the same file at the same moment
collide whether or not a branch exists — the branch only makes it recoverable.

So keep the shape that already works:

- one worktree per session, on its own short-lived branch
- **merge to `main` and gate before the session ends**
- delete the branch after

The difference from what went wrong is entirely the second line.

## The one thing that is actually forbidden

**Never `git worktree add -f` a branch that is already checked out somewhere.**

Git refuses this by default, and the refusal is correct. It was forced twice
during the merge, leaving three directories all holding `main`. The consequences
were real, not theoretical:

- one session's commit made the other two see a "staged" diff nobody staged
- that phantom index swept six unrelated files into a commit
- a session found its checkout stale with a 93-file phantom diff

If you need a clean tree, branch it. `git worktree list` should never show the
same branch twice.

## Before you commit, in a shared checkout

Other sessions work in this repo at the same time. Two habits, both cheap:

```bash
git status --porcelain          # is someone else's work sitting here?
git add <your paths>            # never `git commit -a`, never bare `git commit`
```

`git commit -a` in this repo takes whatever another session left in the tree and
puts your name on it. Stage explicit paths, always.

## And the gate

`gates/land-gate.sh` runs the builds, the ~26 host harnesses, both static
checkers and every boot gate. Run it before you land, backgrounded, on a quiet
box. It reported green for its whole life while running none of the harnesses —
that is fixed, and the lesson is the one below.

## The standard

A thing is done when a command says so and you read the output. Not when the
plan says it was going to be done. `docs/evidence/MERGE-EVIDENCE.md` had an entry claiming
an address collision was fixed; the fix had been written into the plan and never
applied to the code, and the document reported the intention as the outcome. The
audit caught it. That is the failure this project keeps having, and it is the one
worth building habits against.
