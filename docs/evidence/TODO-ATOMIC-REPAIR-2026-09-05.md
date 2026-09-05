# TODO generator recovery — 2026-09-05

The interrupted "Continue ZL Linux cleanup" task identified direct truncation in
`tools/todo.sh`. It remained present at `8fb1425777b184c762686a12161e3faf37e2095a`.
The regression test interrupted generation after its header was emitted: the old
script replaced the previous TODO and its hand-written task with only that header.

The repair generates into a unique temporary file beside the destination, checks
generation and publication results, preserves existing permissions, and renames
only after successful generation. HUP, INT and TERM terminate without publishing;
the exit trap removes the temporary file. SIGKILL cannot run traps and may leave
a temporary file, but the previous TODO remains intact. This does not add locking
against simultaneous human edits or concurrent successful generators.

## Verification

`python3 tools/test_todo.py` exercises interruption, hard kill, failed temporary
creation, a file-size-limited generation write failure, failed permission copying,
failed rename, successful publication with
hand-written text and mode preservation, first creation, and read-only stdout.
The original script failed the interruption/backlog regression. The repaired
script passes all nine tests. Fixtures stub GitHub, Git and the hazard scanner;
they do not build or touch the real backlog. The docs workflow now runs the test.

Additional local checks passed: Bash syntax, ShellCheck at warning severity,
`git diff --check`, the tension-heading self-test, real `--stdout` generation,
and `tools/doc-check.sh --paths-only` (118 directory capsules plus source paths
and local links). Default ShellCheck also reports two pre-existing SC2126 style
suggestions on grep/wc counting. Full executable documentation claims were not
run; the path-only check does not establish them.

## Continuation state

Live remote main was checked at the base SHA above. PRESSWORK PR #6 is merged
(2026-08-27). Persistent-process PR #12 and its bounded-sleep follow-up #13 are
separate from this maintenance repair. The latest listed main nightly runs
succeeded; those results do not verify this patch. Its focused tests run in
the docs workflow after publication.

Work is isolated in `/home/roy/Documents/repos/zl-linux-cleanup-todo`, branch
`codex/cleanup-todo-atomic`. The canonical checkout had concurrent edits to
`.ultra/TENSIONS.md`, `docs/PROJECT-STATUS.md` and the ThinkPad first-boot guide,
plus an untracked driver-oracle plan. Those files were not changed by this repair.
Current worktrees supersede the front door's historical sole-checkout statement.
The repair is published separately from the scheduler and sleep changes. Local
verification did not run a kernel build, QEMU or physical test; those are not
needed to exercise isolated TODO-file publication.

## Preserve pending documentation at commit time

The installed post-commit hook previously regenerated both managed documents
even when they already contained pending work. The installer now emits a guard
that preserves both if either is dirty, untracked or ignored, and skips refresh if Git
cannot report their state. Clean managed documents retain automatic refresh.
The guard checks the calling checkout, including when a linked worktree uses
the common hook installation. This closes the pre-existing-change case; it
does not add locking against an edit beginning after the status check.

`python3 tools/test_hooks.py` passes seven checks using disposable repositories.
Before the guard, five refusal cases failed, including overwriting pending
documentation in a linked worktree. A later ignored-file regression also
reproduced an overwrite and now passes. All seven checks pass, and the nine
TODO publication checks remain green. Both suites run in the docs workflow.
The installer also now accurately describes pre-push as including build/QEMU
work; its preflight behavior is unchanged.

The live shared hooks were not reinstalled while other agents were active.
This record proves the source-generated hooks in the fixtures. PR #14 retains
that installation boundary and the independent generator's interruption proof.
