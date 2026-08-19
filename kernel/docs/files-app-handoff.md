# Files app + editor save-through — handoff (work lost, spec survives)

Status: **not started in this repo.** A Codex thread built it on 2026-08-19 and
the work was destroyed before it landed. This doc is the recovered spec so the
next agent does not re-derive it. Source: Codex thread
`01a0187a-43ac-7a62-8396-ff45ebff3b79`, rollout at
`~/.codex/sessions/2026/08/19/rollout-2026-08-19T15-24-21-01a0187a-*.jsonl`.

## What was asked

Persistent storage + a real file manager for the zlOS desktop — named files that
survive a power cycle, not numbered RAM slots.

## The design it settled on (keep this)

- **`zlfs` stays the one real filesystem.** Do not add a second one. It already
  exists here: `kernel/fs.c`, driven from `kernel/kernel.zl`, host-tested by
  `kernel/hosttest/fstest.c`.
- **Mount on Files-app open**, rather than requiring a manual mount step.
- **Files window**: create / open / delete by *name*.
- **Editor gains a disk-backed mode** — real save, Ctrl+S, copy/paste.
- **Numbered RAM slots stay as a compatibility path only**, not the primary
  store. Do not delete them in the same increment.

## The acceptance gate (this is the whole point)

Cold boot, not an in-process test:

1. create a file in the Files app
2. save it from the editor
3. **kill QEMU**
4. boot fresh from the *same* NVMe image
5. the file is there with its contents

An in-process mount/unmount round-trip does **not** satisfy this.

Codex reported the kernel build green (0 undefined symbols, valid multiboot
header) and the fs suite green incl. a separate-process cold mount — but see
below: none of that is verifiable, because the tree it built is gone.

## Why the work was lost — read this before isolating anything

Codex tried to make a `codex/storage-file-manager` **git worktree** and Git
refused. It reported the repo's Git metadata as read-only and fell back to a full
clone under `/tmp`. The app then refreshed `/tmp` mid-session and deleted the
clone. Its recovery attempt went to another `/tmp` path and died the same way.

Verified 2026-08-19 after the fact:

- `.git` here is `drwxrwxr-x roy:roy` — **writable**, and
  `git worktree add --detach` succeeds when run as `roy`.
- `~/.codex/config.toml` sets `sandbox_mode = "workspace-write"` with **no**
  `writable_roots` under `[sandbox_workspace_write]`, and
  `exclude_tmpdir_env_var = false`.

So the read-only `.git` was **Codex's own sandbox, not the filesystem** —
workspace-write leaves `.git` protected while leaving `TMPDIR` writable, which
is exactly why it picked the one directory that gets wiped. Nothing survived:
no branch, no stash, no `refs/codex/turn-diffs/*` checkpoint contains it (all
are clean 346-file bases), and `find /home/roy/Documents -iname '*zlfs*'`
returns nothing outside this repo.

**Rule for the next agent:** isolate with a real `git worktree` under
`~/.cursor/worktrees/` or `../zl-linux-<topic>`. Never use `/tmp` as a working
copy on this box. If a tool reports `.git` read-only, that is its sandbox — fix
the sandbox, do not route around it into `/tmp`.

## Repo hazards

- `zl` is the **language** repo and defers OS work. The kernel is **here**
  (`zl-linux`). Do not build this in `zl`.
- This checkout carries ~33 dirty files owned by other concurrent sessions
  (`kernel/kernel.zl`, `kernel/term.c`, `freestanding/runtime_kernel.c`, docs…).
  Branch from committed state; do not build on top of someone else's dirt.
- The ZL kernel subset has unusual global-variable and string rules — compile
  early and often rather than writing the whole feature first. Codex flagged this
  as the thing most likely to bite.
