# When the code lies — detection, and review on push

Written 2026-08-19 by Claude (`source: claude`). **PLAN ONLY — nothing here is
built.** Extends [housekeeping-plan.md](housekeeping-plan.md).

---

## 1. A taxonomy of lies, every one of them real here

"The code lies" sounds abstract until you list the forms it takes. Every category
below has already happened in **this** repository. That is not a rhetorical
flourish — it is why the list is the right list.

| # | Lie | Real instance | Caught by |
|---|---|---|---|
| 1 | A gate reports green **without running** | every `verify-*.sh` does `echo "skip:"; exit 0` when qemu/OVMF is missing | script |
| 2 | A gate runs but tests the **wrong artefact** | three gates green while the 64-bit build was dead — all three booted the 32-bit kernel | thought, then a new gate |
| 3 | A guard is **configured but inert** | `-w` silences the four `-Werror=` flags placed after it; 34 truncation sites sit behind it | probe |
| 4 | A doc **contradicts the code** | `.ultra/STATE.md`: compositor "unreachable, nothing calls them" — `kernel.zl` calls it in six places | claim |
| 5 | A doc describes a file **not in the repo** | `verify-efi.sh`, `verify_fmt.sh`, `zlfmt.c`, `syntax_tour.zl` | script |
| 6 | A **measurement** measures the wrong thing | a gate "regression" that was really host load; the boot took 30 s instead of 12 s | A/B |
| 7 | Code **exists but never runs** | most of `intel.c`'s write paths, gated behind `lt_armed` | caller scan |
| 8 | A **name** promises what the code does not do | `verify-efi.sh` "verifies" and also silently skips | model |
| 9 | A **comment** contradicts its code | — (not yet found; the class is real) | model |
| 10 | A **test cannot fail** | assertions passed while `ui_toggle` drew a circle instead of a pill | mutation |

Note the right-hand column. **Six of ten need no model at all.** Only 8 and 9
genuinely require judgement. Building the model-based review first would be
solving the smallest part of the problem.

## 2. The central principle: prove the check can fail

The deepest lie is a **green check that cannot go red**. It is worse than no
check, because it actively buys confidence.

You have already solved this once, and the solution is in the repo:

> `verify-efi.sh` is validated in both directions: green on a good build, and red
> with the right diagnosis when the GOP lookup is forced to fail.

**That practice should be mandatory and mechanical for every gate.** Generalised:

```
tools/gate-mutation.sh
```

For each gate, a registered mutation it MUST detect:

| gate | mutation | must go red with |
|---|---|---|
| `verify.sh` | corrupt one byte of the golden transcript | "serial transcript changed" |
| `verify-raw.sh` | remove the bootloader's marker print | marker never appears |
| `verify-efi.sh` | force the GOP lookup to fail | "no framebuffer" |
| `verify_fmt.sh` | make `zlfmt` drop a trailing byte | token stream differs |
| `run_tests.sh` | flip a comparison in `interp.c` | some test disagrees |
| `hazard-scan` | add a `(unsigned long)&x` cast to an EFI file | count increases |
| `engine-parity` | remove a pin | unpinned divergence |
| `doc-check` | point a doc at a nonexistent file | stale reference |
| `wmshot` diff | shift the title bar by 2 px | pixel delta |

Run nightly, not per-push: it deliberately breaks things and reverts them, so it
wants an isolated worktree and no audience. **A gate with no registered mutation
is an untested test** and should be reported as such.

This single mechanism catches lies 1, 2, 3 and 10 — the four most dangerous —
and it needs no model.

## 3. Mutation testing, scoped to the diff

Section 2 asks "can this gate fail at all". Mutation testing asks the harder
question: **does anything notice when this specific line changes?**

Full mutation testing over 291k lines is absurd. Scoped to a PR it is cheap:

1. take the lines the PR touched
2. apply a small set of semantic mutations — `<` → `<=`, `+` → `-`, delete a
   statement, negate a condition, return early
3. run only the fast gates
4. **any mutation that survives means nothing tests that line**

Report as coverage-of-consequence, not a blocker. "You changed 40 lines; 12 of
them can be arbitrarily altered and every gate stays green" is a far more useful
sentence than a coverage percentage, and it directly targets the failure mode
this repo keeps hitting — 63 green assertions over 6 data-loss bugs.

Start with `fb.c`, `wm.c`, `ui.c`: they are freestanding C against memory, so a
mutation round costs a compile and a `wmtest` run, no boot.

## 4. Per-directory mandatory instruction files

**Verified today, on this machine:** Codex loads all three levels and they
*compose* — nothing is replaced.

```
cwd = <repo>/kernel, inside a git repo
  ~/.codex/AGENTS.md   (global)   loaded
  <repo>/AGENTS.md     (root)     loaded
  <repo>/kernel/AGENTS.md (nested) loaded
```

Cursor does the same: its rule loader globs `**/AGENTS.md`, `**/.cursor/rules/**`
and `**/.agents/skills/**` and has an explicit `nested_extensibility.agents`
counter. Outside a git repo the walk-up stops early — inside one, all three land.

So per-directory instruction files are natively supported by both, today, with no
tooling. Proposed set:

| file | says |
|---|---|
| `kernel/AGENTS.md` | the LLP64 rule, `-mgeneral-regs-only` boundary, the memory map, which gates cover this dir, **panel power can damage hardware** |
| `kernel/hosttest/AGENTS.md` | these run on Linux against the real GPU; `--survey` is read-only; `modeset-run.sh` blanks the screen and recovers from an EXIT trap |
| `tools/AGENTS.md` | every check is mechanical; a check that needs judgement belongs in review, not here; every gate needs a registered mutation |
| `docs/design/AGENTS.md` | status header required; a decision is not landed until the code agrees |
| `examples/AGENTS.md` | **this directory is a test input set** — `run_tests.sh` globs `*.zl`; scratch files break the suite |
| `stdlib/AGENTS.md` | names leak into global scope; prefix them |
| `freestanding/AGENTS.md` | no libc; the binary must have 0 undefined symbols |

`examples/AGENTS.md` alone would have prevented the `Zaccoding.zl` breakage.

**Coverage check:** every directory containing source must have one. Same shape
as the README check — mechanical, baselined, fails on a new uncovered directory.

### 4.1 Letting agents edit them — the learning loop

The valuable half of your idea. Today, a hazard is learned once and written by
hand, or lost. Proposed loop:

1. a gate fails, or review finds something real
2. the fixing agent must answer: **what instruction would have prevented this?**
3. it appends a candidate line to the relevant `AGENTS.md`, marked `<!-- proposed -->`
4. you accept or delete it in the PR

Rules that keep it from rotting into noise:
- **A proposal must cite the failure that motivated it** — commit sha or gate name.
  No sha, no proposal.
- **Additions only, never silent edits.** An agent rewriting a hazard it does not
  understand is the failure mode to design against.
- **A cap.** If `kernel/AGENTS.md` exceeds ~200 lines it stops being read; over
  the cap, a proposal must *replace* something and say what.
- **Every hazard gets a registered claim** where possible, so the instruction and
  its proof travel together.

## 5. Review on push — several agents, different lenses

### 5.1 Lenses, derived from section 1

Generic "review this diff" produces generic findings. Each agent gets one lens,
matched to a way this repo actually breaks:

| lens | asks |
|---|---|
| **truncation** | any pointer through a 32-bit type in an EFI translation unit? |
| **reachability** | does new code have a caller; is any flag arming it ever set? |
| **gate coverage** | which gate would have caught this? if none, say so |
| **hardware** | does this touch panel power, T12, AUX, or the ThinkPad path? |
| **zl semantics** | any claim about zl behaviour — was `./interp` actually run? |
| **name/comment truth** | does the name promise what the body does; does the comment match? |
| **measurement** | is any number here measured or asserted? under what load? |

The hardware lens should be able to **block**. The rest comment.

### 5.2 Adversarial verification, not consensus

A finding from one agent is a hypothesis. Before it reaches you it must survive a
second agent whose instruction is to **refute** it, defaulting to refuted when
uncertain.

This is not theoretical for you: a review once asserted zl's scoping was broken,
when it is frame-scoped for parameters and works. A refuter with `./interp` in
hand kills that in one run. Findings that survive arrive with the refutation
attempt attached, so you can see what was tried.

### 5.3 Cost control

Reviewing every push is too much. Trigger:

- on PR open and on pushes that change `kernel/**` or `*.c`
- skipped entirely for docs-only diffs
- lenses selected by what the diff touches — no hardware lens on a `tools/` change
- diffs over ~2000 lines get split by file rather than truncated, and the report
  says which files were covered

**Never silently reduce coverage.** A review that quietly skipped half the diff
reads as "reviewed" — which is lie #1 wearing a different hat.

## 6. What "it writes what it does" means concretely

On push, one report, appended to history and attached to the PR:

```
- commit range, files, lines
- gates: which ran, which passed, which SKIPPED (skip is a failure)
- hazard deltas: truncation sites, parity pins, doc rot — as a diff, not a total
- mutation survivors on the changed lines
- review findings that survived refutation, with the refutation shown
- instruction proposals raised, and their motivating sha
- what was NOT covered, and why
```

The last line matters most. Every one of the ten lies in section 1 is ultimately
a report that omitted its own limits.

## 7. Order, and the honest cost

1. **`gate-mutation.sh`** — highest value, no model, catches the four worst lies.
   Also the only item that tells you whether anything else you build works.
2. **Per-directory `AGENTS.md`** — verified to work, pure text, immediate.
3. **Reachability scan** — a caller check for new symbols; pure script.
4. **Mutation testing on `fb.c`/`wm.c`/`ui.c`** — cheap, no boot.
5. **Multi-agent review with lenses + refutation** — needs a key.
6. **The instruction learning loop** — last; it needs the rest to generate signal.

Items 1–4 need no model and cover eight of the ten lies. Item 5 is the expensive
one and covers two. That ordering is the opposite of the intuitive one, and it is
the right one.

## 8. Open questions

1. **Where does the mutation harness run?** It deliberately breaks the tree, so
   it wants a throwaway worktree. Nightly, or on demand?
2. **Does the hardware lens block, or only shout?** It is the one place where
   being wrong costs a physical panel.
3. **Do instruction proposals land as PR comments, or as `<!-- proposed -->` lines
   in the file?** The second is easier to act on and easier to ignore.
4. **How many refuters per finding?** One is cheap and catches the obvious; three
   with different lenses catches more and costs three times as much.
5. **What happens when a gate has no registered mutation?** Report only, or fail?
   Recommend report until every existing gate has one, then fail.
