# Everyday ergonomics — the daily loop, not the architecture

Written 2026-08-19 by Claude (`source: claude`). **Ideas, not decisions.**
Companion to [agent-ideas.md](agent-ideas.md).

Measurements here were taken on this machine today.

---

## 1. The headline: your fast loop is ~600× faster than your slow one

Measured:

```
kernel/hosttest/wmtest      0.136 s     47 assertions, no boot
kernel/verify.sh           ~50    s     QEMU, 32-bit BIOS
kernel/verify-efi.sh     1 m 27  s      QEMU + OVMF
```

`wmtest`, `fbbench`, `inputtest`, `wmshot` are C against mmap'd memory. They do
not boot anything. **A change to `wm.c`, `ui.c` or `fb.c` can be checked in a
sixth of a second**, and most of the compositor work you are doing lives in
exactly those files.

The QEMU gates answer "does it still boot". They are the wrong tool for "does
this button land in the right place", and they cost 600 times more.

### 1.1 Watch loop — save the file, see the result

`entr` is already installed. Roughly:

```bash
ls kernel/*.c kernel/*.h | entr -c ./future-fastloop
```

where the future fast loop rebuilds the hosttest binaries, runs `wmtest`, and re-renders
`wmshot` to a PNG. Sub-second, on every save.

Point an image viewer at that PNG and it refreshes as you type. That is a live
view of your own operating system's desktop, on a Linux desktop, with no VM.

For a project whose current work is a compositor, this is probably the single
biggest day-to-day improvement available — bigger than anything involving an
agent.

### 1.2 Make the fast loop the default and the boot loop deliberate

Right now the reflex is to boot. It should be: fast loop constantly, boot before
a commit, full matrix in CI. `tools/preflight.sh` already encodes the last two;
the first needs `watch.sh`.

## 2. One entry point instead of fourteen scripts

Today: `build.sh`, `run_tests.sh`, `verify_fmt.sh`, `kernel/verify*.sh` (×4),
`kernel/mk*.sh` (×4), `hosttest/build.sh`, `hosttest/gpu-dev.sh`,
`tools/*.sh` (×6 and growing). Nobody remembers fourteen names, including you in
three weeks.

```
./zl build          ./zl test          ./zl watch
./zl boot [efi]     ./zl shot          ./zl check [--boot|--all]
./zl gpu probe      ./zl status        ./zl journal
```

A thin dispatcher over what exists — no new behaviour, just discoverability.
`./zl` with no arguments prints the list, which is the actual feature: the tools
become findable instead of remembered.

## 3. `./zl status` — machine state in one glance

Cheap to build, answers the questions you actually ask at the start of a session:

```
branch      ci/gates-and-agent-brief   (9 ahead of main)
tree        57 modified, 41 untracked      <-- 4 of them never committed
worktrees   5 active: zl-apps, zl-browser, zl-exec, zl-feel, +2 claude/
gates       fast: clean (2 min ago)   boot: not run since HEAD~3
hazards     34 truncation sites, 2 parity pins, 0 doc rot
PRs         #4 open, all checks green   #1 #2 #3 open, no checks
untracked   verify-efi.sh is documented but not in git   <-- would have caught it
```

The last line is the point. That specific condition existed for weeks and cost a
CI debugging round to discover.

## 4. Never lose work again

You had **four files that existed only on this laptop**, all of them named in
committed documentation, one of them the gate CLAUDE.md calls essential.

- **`./zl wip`** — commit everything to a `wip/<branch>-<date>` branch and push
  it, without touching your working tree or current branch. A safety net that
  costs one command and makes "the disk died" survivable.
- **A daily `wip` push from cron.** Uncommitted work is the only thing in this
  project with no backup — git has everything else.
- **`./zl status` flags documented-but-untracked files** (section 3), which is the
  detection half.

## 5. Bisect, using the gates you now have

You have deterministic, scriptable gates. That makes `git bisect run` a
superpower rather than a chore:

```bash
./zl bisect kernel/verify-efi.sh good=v0.2 bad=HEAD
```

A wrapper over `git bisect run` that knows how to build first and which exit
codes mean "skip" (build failed) versus "bad". With ~1 minute per boot, a
20-commit range is about 5 boots — five minutes to find the exact commit,
unattended.

Worth building **before** you need it, because you build it badly under pressure.

## 6. zl in Cursor — verified possible, one word

`editors/vscode-zl/install.sh` requires `code` on PATH and calls
`code --install-extension`. Cursor is a VS Code fork with the same manifest at
`~/.cursor/extensions/extensions.json`, and **`cursor --install-extension`
exists** — checked today.

So the installer should take the editor as an argument:

```bash
editors/vscode-zl/install.sh          # VS Code
editors/vscode-zl/install.sh cursor   # Cursor
```

Syntax highlighting, `Ctrl+F5` to run through the interpreter, and the formatter
— in the editor whose tab-completion is the reason to open Cursor at all. Without
this, zl files are plain text there, which wastes the one thing Cursor is best at.

**Note the trap the extension's own comments record:** copying the folder into
the extensions directory does nothing, because the editor only loads what is
listed in `extensions.json`. It fails silently. Same trap applies to Cursor.

## 7. Tell me when the slow thing finished

`notify-send` is installed. Background gates already print to a file; nothing
tells you they are done, so you either block on them or forget them.

```bash
./zl boot efi --notify
```

→ desktop notification with pass/fail and the elapsed time. Small, and it changes
the ergonomics of a 90-second gate from "wait" to "start it and forget it", which
is the difference between running it and skipping it.

## 8. A worktree helper

You use worktrees heavily — five active plus the `.claude/worktrees/` set. A new
one currently needs: create it, install hooks, build the toolchain, build
hosttest.

```bash
./zl worktree new fix/truncation-xhci
```

→ worktree created, hooks installed, toolchain built, ready. Removes the reason
to work in a dirty main checkout, which is where the concurrent-session collisions
come from.

## 9. Keep the golden artifacts over time

`verify.sh` diffs against one golden transcript. `wmshot` renders one frame.
Neither keeps history.

Store both per tagged release under `artifacts/`:

- the serial transcript, so boot output changes are readable over months
- the desktop PNG, so you can see the desktop evolve release by release

The second is also, incidentally, the best possible progress record for a project
whose thesis is one person doing what normally takes thousands. A contact sheet
of every zlOS desktop from v0.1 to now is worth more than a changelog.

## 10. Shell polish

- **Completions** for `./zl` — one file, and the whole surface becomes
  discoverable by tab.
- **A `zlcd` alias set** for jumping between the five worktrees.
- **`direnv`** so entering the repo puts `./zl` and the built binaries on PATH.
- **`./zl cheat`** — the ten commands you actually use, printed. Faster than
  grepping CLAUDE.md, and it is the thing a new agent session should read too.

## Ranking, by daily value per hour spent

| | idea | why |
|---|---|---|
| 1 | **`watch.sh`** — 0.136 s loop | 600× faster than booting; your current work is exactly the files it covers |
| 2 | **`./zl` dispatcher + `status`** | fourteen scripts become one discoverable surface |
| 3 | **zl extension in Cursor** | verified one-word fix; without it Cursor's best feature is wasted on your main language |
| 4 | **`./zl wip` + cron** | four critical files were one disk failure from gone |
| 5 | **`bisect` wrapper** | you have the gates; build it before you need it |
| 6 | **notify on long gates** | turns a 90 s gate from "wait" into "forget" |
| 7 | **worktree helper** | removes the reason to work in a dirty checkout |
| 8 | **golden artifact history** | costs nothing, and it is the progress record |
