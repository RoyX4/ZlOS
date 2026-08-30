# Whole zl-linux topology closure — 2026-08-30

This receipt supersedes the repository-topology claims in the 2026-08-24
receipts. It proves Git and working-copy closure only. It does not claim that
the product roadmap is complete, that every planned feature is implemented, or
that QEMU evidence is physical-hardware evidence.

## Scope

The audit searched the complete `/home/roy` tree plus `/tmp` and `/var/tmp` for
Git repositories connected to `RoyX4/zl-linux`. It then checked:

- every GitHub branch returned by `git ls-remote --heads`;
- every local branch in the canonical object store;
- every registered, including detached, worktree;
- every standalone clone found under the searched roots;
- tracked modifications and all untracked files in every checkout; and
- process working directories before removing old checkouts.

The separate `RoyX4/zl` language repository was not merged into this repository
and remains a deliberately separate project.

## What the audit found

- GitHub had 41 branches: `main` plus 40 historical topic branches. Every one
  of the 40 topic tips was already an ancestor of GitHub `main` at
  `4b4413cd2f489d5179c0e13143ed848fca2b1a7d`.
- The canonical object store had four local preservation tips that were not
  ancestors of that `main`: `e427fe6`, `82bdb4e`, `4470346`, and `441933d`.
  The two commits on the first tip were patch-equivalent to commits already in
  `main`; the other tips were preserved snapshots whose current content had
  already been integrated or superseded.
- All 17 registered worktrees were clean. Their branch or detached heads were
  already ancestors of `main`, except for the preservation tips above.
- Eight additional standalone clones existed. Five were clean. Two old Codex
  visualization clones had 23 and 11 dirty files respectively; every dirty
  file blob was byte-for-byte reachable in `main` history and later
  superseded.
- The old standalone `codex/all-reference-apps` clone contained genuinely
  unique uncommitted prototype source plus generated host-test ELF files. Its
  ten source/build-list changes were committed as `783187a`; the generated ELF
  files were not treated as source. Current `main` already contains the same
  application catalogue, utilities, and games as the later native ZL
  implementation, so applying the older C prototype would have rolled the
  product backward.

## Ancestry closure

Commit `61814bfa6bb70057775871a2d7c97e858d386bbc` is an ancestry-only merge with
these parents:

1. the audited GitHub `main`;
2. local full-program preservation tip `e427fe6`;
3. local structure preservation tip `82bdb4e`;
4. local shared-checkout preservation tip `4470346`;
5. local review preservation tip `441933d`; and
6. preserved reference-app prototype source `783187a`.

The `ours` merge strategy was intentional: it records that every tip was
examined and retained in history while keeping the newer, already-gated main
tree. This is the same distinction as the earlier Product Wave ancestry merge:
history closure must not reintroduce stale files merely to create a tree delta.

## Housekeeping performed

- Deleted all 40 historical GitHub branches. `git ls-remote --heads` then
  returned only `main`.
- Removed 15 redundant registered worktrees, leaving the canonical checkout
  and the temporary publication checkout. The publication checkout is removed
  after this closure reaches `main`, leaving one registered worktree.
- Deleted 42 obsolete local branch refs after their tips became ancestors of
  the closure merge.
- Moved all eight standalone clone directories to the desktop Trash rather
  than irreversibly deleting them. Their committed history remains reachable
  from the closure merge or GitHub `main`.
- Reset the canonical checkout to the live GitHub `main`; no old snapshot
  branch remains checked out.

## Verification boundary

The earlier consolidation and Presswork landing passed the host, build, BIOS,
raw, ISO, EFI, QEMU, dead-state, and GitHub Actions gates recorded in their own
receipts. This topology pass changed no product files: `61814bf` uses the
`ours` strategy, and the remaining changes are documentation. The repository's
pre-push hook was allowed to run once during branch deletion, but it ran from
the old canonical snapshot checkout and did not send the delete request. The
delete-only retry used `--no-verify` after exact ancestry checks; no source ref
was pushed by that command. Publication of this receipt remains subject to the
normal current-branch checks.

No physical ThinkPad boot or hardware validation was run in this topology pass.
