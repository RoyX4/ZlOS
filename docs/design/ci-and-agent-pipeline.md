# CI and the agent pipeline — design, not yet built

Written 2026-08-18 by Claude (`source: claude`). **Nothing here is implemented.**
This is the thinking-first pass; every claim below was checked against the tree.

Related: [../../CLAUDE.md](../../CLAUDE.md) (hazards and gates),
`~/Documents/agent-when-to-use.md` (which agent for what).

---

## 0. The reframe: the bottleneck is the box, not the brains

Every gate in this project is a QEMU boot. Every QEMU boot competes for the same
4 cores as the agent writing the code. Your own notes record both failure modes:

- load average hit 6, a 12 s boot took 30 s, **and the gate reported a regression
  that did not exist**
- the OOM killer took the agent itself at 7.9 GB, taking the session with it

So the scarce resource here is not model quality. It's **CPU-seconds on this
laptop**. That single fact should drive the whole integration:

> **Every unit of work moved off this box is capacity gained. Work that stays on
> the box should be work that can only happen here.**

Three destinations exist:

| Destination | What belongs there |
|---|---|
| **GitHub Actions** | every QEMU gate, every host-test, every build |
| **Cloud agents** (Codex Cloud, `cursor-agent worker`) | long autonomous jobs |
| **This box, unavoidably** | `intel.c` / `hosttest` against the real GPU, the ThinkPad boot, anything touching panel power |

The third row is the interesting one — it's what makes this project *not* fully
cloud-able, and it's exactly where your hardware-damage hazards live.

## 1. Current state, measured

```
.github/workflows           does not exist — zero CI
open PRs                    3, all authored by Claude Code, none independently reviewed
remote branches             14 (desktop/*, claude/*, fix/*, lang/*)
repo                        private, 13.5 MB packed
gh auth                     RoyX4, scopes include repo + workflow + copilot
uncommitted in worktree     57 files
```

Every gate runs only when a human remembers to run it, on the one machine that
also has to think.

**`.ultra/STATE.md` is stale.** It says the compositor is "built, tested, and
unreachable — nothing calls them." `kernel/src/kernel.zl` calls `wm_open_p` at lines
293–296, 2315, 2511, 2518, plus `wm_us`, `wm_peak`, `ui_scale`. It was wired up in
the commits after that summary was written. Worth fixing before it misleads an
agent into re-doing finished work.

## 2. Layer 1 — move the gates to GitHub Actions

All four boot gates and both host gates are CI-compatible today, unmodified:

- `verify.sh` already runs headless — `-display none -serial stdio`, polls with a
  ceiling, diffs a golden transcript. That is exactly a CI job.
- Dependencies are all apt-installable on `ubuntu-latest`:
  `gcc clang nasm qemu-system-i386 qemu-system-x86_64 ovmf mtools xorriso grub-pc-bin`

**Matrix the four boot paths so they run in parallel:**

| job | script | catches |
|---|---|---|
| `bios32` | `kernel/verify.sh` | 32-bit multiboot regressions |
| `raw` | `kernel/tools/checks/verify-raw.sh` | our own bootloader |
| `iso` | `kernel/tools/checks/verify-iso.sh` | GRUB BIOS + UEFI |
| `efi` | `kernel/tools/checks/verify-efi.sh` | **zlOS as its own UEFI app — the ThinkPad path** |
| `lang` | `run_tests.sh` | interpreter vs C backend vs native, byte-identical |
| `fmt` | `verify_fmt.sh` | token stream identical before/after |

Four runners in parallel ≈ the wall-clock of the slowest gate (~1.5 min) instead
of ~5 minutes serial — **and none of it on your cores.**

### Why this specifically matters here

Your own doc: *"Three gates were green while the 64-bit build was dead."* The
cause was that nobody ran `verify-efi.sh`, because running all four by hand costs
five minutes of a four-core box.

CI does not get bored and does not forget. It runs `verify-efi.sh` on every push,
every time, for free capacity. **That entire bug class is a scheduling problem,
and CI is the fix — no AI involved.**

Second bonus: your notes say a gate failed on an *unchanged* kernel because host
load pushed the boot past a fixed timeout. A CI runner is an idle dedicated box.
The timing-sensitivity class gets quieter simply by moving.

**Constraint to respect:** this repo is private, so Actions draws on the account's
included minutes rather than the unlimited public-repo pool. That argues for
matrix jobs that fail fast and for keeping the heavy ISO/EFI gates on PR and push
to `main` rather than on every commit to every branch.

## 3. Layer 2 — the cool one: a picture of the desktop on every PR

`kernel/tests/host/wmshot.c` renders **one frame of the compositor to a PPM, from
Linux, with no boot, no GPU, and no display** — it's C against mmap'd memory, so
it finishes in milliseconds. Its own header says it best:

> *wmtest asserts 47 properties and nobody has LOOKED at any of them. Both matter
> and they catch different things: assertions catch a click landing on the wrong
> window, eyes catch a title bar that is four pixels too tall.*

And it has already paid: it caught `ui_toggle` drawing a circle instead of a pill —
**a bug every assertion passed.**

That is a complete visual-regression harness sitting in the tree with no CI around
it. Wrapped in a workflow it becomes:

1. build `wmshot`, render `desktop.ppm` at a fixed size
2. convert to PNG, upload as a PR artifact
3. diff against the PNG from the base commit
4. if pixels changed, post the before/after into the PR

**Every pull request gets a screenshot of the operating system it changes.** Any
rendering regression becomes visible instead of theoretical, and it costs one
runner and no boot.

I have not seen another OS project do this per-PR. It is possible here only
because `fb.c`/`wm.c`/`ui.c` are freestanding C against memory — the same design
choice that makes `fbbench` measurable is what makes this cheap.

Natural extensions once it exists:
- render at 1024×768 **and** 2560×1440 — the resolution-cliff class you already hit
- a contact sheet of several app windows, not just the desktop
- fail the job on >N% pixel delta unless the PR body says `visual-change:`

## 4. Layer 3 — a second brain on every PR

Three PRs are open. All three were written by Claude Code. None has been read by
anything that isn't Claude.

Your own evidence for why that matters: an adversarial reviewer found **6
data-loss bugs underneath 63 passing assertions**, and the `intel.c` audit found
**86 verified defects by reading code that no gate executes**.

A workflow step running `codex exec --json` over the PR diff, posting a review
comment, gives you a different model's blind spots on every change. The argument
isn't "GPT is smarter than Claude" — it's that **two models fail differently**,
and reading is the only verification available for the large parts of this kernel
that no gate can reach.

This is the highest-value AI integration in the list, and it's third because the
two above it are more reliable and cheaper.

## 5. Layer 4 — the grep gates, which need no AI at all

Ladder rung 6: *can it be one line?* Several of your recorded hazards are regexes,
not judgment calls. These belong in a fast CI job and a pre-commit hook, **not** in
a review prompt — a regex is exact, free, and cannot hallucinate:

| Hazard from CLAUDE.md | Mechanical check |
|---|---|
| pointer through `unsigned long` in the EFI build | grep `(unsigned long)` in the 28 `buildefi.sh` files |
| shift-by-32 on a 32-bit value | already covered — assert the four `-Werror=` flags stay *after* `-w` |
| gate waits on wall-clock, not a marker | grep `timeout` near `qemu-system` in `kernel/verify*.sh` |
| build outputs tracked in git | fail if a new `kernel/_gen*.c` or ELF appears in the diff |
| code that exists but has no caller | grep new `wm_*`/`ui_*` symbols for at least one call site |

The last one is your `lt_armed` rule generalized: *"the code exists" is not "the
code works"* — check for an actual caller. That is a script, not a model.

## 6. Layer 5 — the prerequisite: teach every agent what zl is

**This one gates the value of everything above involving a model.**

zl is 291,626 lines across 1,099 files, and **no model has been trained on it.**
Worse, it reads like Python — `fn`, `#` comments, `and`/`or`/`not`, no type
annotations, no declaration keyword. Every model will pattern-match it to Python
or C and be confidently wrong.

That is not hypothetical. It already happened: a review asserted zl's scoping was
broken, when in fact params are frame-scoped and recursion works — only same-named
free locals collide. The reviewer applied C intuitions to a language that doesn't
share them.

**Adding two more models without fixing this makes the problem worse, not better** —
three sources of confident wrong findings instead of one.

The raw material exists and is good:

- `docs/REFERENCE.md` — 207 lines, the syntax and builtins
- `examples/syntax_tour.zl` — and REFERENCE.md already names it the **tiebreaker**:
  *"it either runs or it does not."* That is a runnable oracle, which is worth more
  than any prose spec.
- `docs/design/design_scoping_decision.md`, `KEYWORDS_MAP.md`, `C_CPP_PARITY.md`

So the artifact to build is a **short** zl brief — one page, not the full reference
— covering the traps specifically:

1. it looks like Python; here is where the resemblance stops
2. scoping is frame-scoped, recursion works, here is the one real collision case
3. lists are the AST representation; `["bin","+",L,R]`
4. the interpreter is ground truth; `./interp examples/syntax_tour.zl` settles arguments
5. "if you are about to report a language-semantics bug, run it first"

Point 5 is the important one. Every model gets an executable way to check itself
before it files a finding, which is the only thing that reliably stops this class.

That brief goes in `AGENTS.md` at the repo root, where **all three** read it —
Codex natively, Cursor natively, Claude Code via `CLAUDE.md`.

## 7. What must never leave this box

- **`kernel/src/drivers/display/intel.c` and `kernel/tests/host/`** — needs the real Gen9 GPU. No runner
  has one. The `gpu-dev.sh` / `modeset_test` loop stays local, permanently.
- **The ThinkPad boot path.** Physical hardware, no serial port, screen is the only
  diagnostic.
- **Anything touching panel power.** T12 delay violations and AUX-into-unpowered-panel
  can damage hardware. "Try it and see" is not acceptable, so neither is "let an
  agent try it and see."

Any agent instructions written for this repo should say so explicitly, because a
cloud agent cheerfully told to "make the display driver work" has no way to know
the panel is real.

## 8. Build order, by leverage per unit of effort

1. **The zl brief in `AGENTS.md`** — cheap, and everything else involving a model
   depends on it. Without it the reviews are noise.
2. **CI for `run_tests.sh` + `verify_fmt.sh`** — fast, no QEMU, proves the pipeline.
3. **CI matrix for the four boot gates** — kills the "green gates, dead build" class.
4. **`wmshot` screenshot artifact per PR** — the cool one, and it already caught a
   real bug once.
5. **Grep gates for the recorded hazards** — exact, free, no model.
6. **`codex review` on PRs** — highest AI value, lands after the brief exists.
7. **Cloud agents** — only once CI can tell you whether their output is good.

Note that 6 of the 7 are GitHub and shell, not AI. The agents get more useful
*because* the verification around them got stronger, not instead of it.

## 8b. What building the hazard scanner found: the EFI guard is inert

This is the most important thing in this document and it was not the goal —
it fell out of writing check 1 of `tools/hazard-scan.sh`.

`CLAUDE.md` says, of the four warning flags in `kernel/buildefi.sh`:

> They are now re-enabled after `-w` and fatal... Clang applies flags left to
> right, so these must stay *after* the `-w`. The whole 28-file EFI build is
> clean under them; a reintroduction now fails the build instead of failing on
> the laptop.

**The first half is false, so the second half is meaningless.** `-w` is not an
ordinary warning group — it is a blanket suppression that wins regardless of
position. A later `-Werror=` does not survive it. Measured, same compiler, same
target, same file:

```
clang --target=x86_64-unknown-windows ... -w              -Werror=pointer-to-int-cast   → exit 0
clang --target=x86_64-unknown-windows ... -Wno-everything -Werror=pointer-to-int-cast   → exit 1
clang --target=x86_64-unknown-windows ...                 -Werror=pointer-to-int-cast   → exit 1
```

on this two-line probe, which must be rejected under LLP64 and is not:

```c
int g;
unsigned long trunc(void) { return (unsigned long)&g; }
```

`unsigned long` really is 4 bytes for that target — confirmed with a
`_Static_assert`. So the build is "clean under them" only because they never
fire. The guard against the class that shipped twice is decorative.

### What it is currently hiding

Forcing the guard on (`-w` → `-Wno-everything`, nothing else changed) and
compiling the real EFI translation units from `kernel/`:

| file | truncation sites |
|---|---|
| `xhci.c` | 12 |
| `freestanding/runtime_kernel.c` | 11 |
| `fb.c` | 8 |
| `smp.c` | 2 |
| `console.c` | 1 |
| **total** | **34 across 5 files** |

`xhci.c` is the largest single group, and address truncation is already on the
record here as one of the bugs that only appears outside QEMU.

Every one of these is harmless while the addresses involved stay below 4 GiB,
which is exactly why nothing has ever shown them — and exactly the reasoning
that made the last two instances of this class ship.

### The fix, and why it is not applied here

One word in `kernel/buildefi.sh`:

```diff
-    -fshort-wchar -mno-red-zone -O2 -DZL_64 -DZL_EFI -w -I.. \
+    -fshort-wchar -mno-red-zone -O2 -DZL_64 -DZL_EFI -Wno-everything -I.. \
```

That is verified to make the guard fire. It is **deliberately not applied in
this commit**, because the moment it lands the EFI build stops compiling until
all 34 sites are repaired — that is a real change to boot-path code and it
wants its own branch and its own `verify-efi.sh` run, not a drive-by inside a
CI commit.

`tools/hazard-scan.sh` check 1 now fails until it is done, so it cannot be
quietly forgotten. Check 2 prints the current count.

### The transferable lesson

The original fix was correct about the bug and correct about the remedy, and
still produced no protection, because nobody pressed the test button. A guard
that is *configured* and a guard that *fires* are different claims, and only
one of them is worth anything.

Every mechanical check in `hazard-scan.sh` is therefore written to prove the
behaviour, not to inspect the configuration.

## 9. Open questions for Roy

- Actions minutes are shared across the whole account and this repo is private —
  do the heavy ISO/EFI gates run on every push, or only on PR and `main`?
- The 9 sibling repos (`zl-apps`, `zl-browser`, `zl-exec`, `zl-feel`, `zl-merge`,
  `zl-system`, `zl-value16`, `zl-main`, `zl`) are parallel tracks on their own
  branches. Do they get the same CI, or does it live only in `zl-linux` until the
  tracks merge?
- 57 files are uncommitted in the worktree right now, including `CLAUDE.md`,
  `build.sh` and `docs/REFERENCE.md`. Some of that may be another session's work.
  Worth a `git status` review before any of this lands.
