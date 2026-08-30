# Housekeeping and documentation — plan

Written 2026-08-19 by Claude (`source: claude`). **PLAN ONLY — nothing here is
built.** Companion to [ci-and-agent-pipeline.md](ci-and-agent-pipeline.md), which
covers the gates that already exist, and to
[lie-detection-and-review.md](lie-detection-and-review.md), which covers proving
a check can fail, per-directory instruction files, and multi-agent review.

Status of the world as of writing:

- **On GitHub, green:** the six CI workflows, `AGENTS.md`, `hazard-scan`,
  `engine-parity`, `preflight`, the hooks installer.
- **Committed locally, deliberately NOT pushed:** `doc-check.sh`, `todo.sh`,
  `journal.sh`, `docs.yml`. Two commits. They are a first cut at section 3
  below and should be treated as a prototype to argue with, not a decision.

---

## 1. The problem, stated properly

Three different things get called "documentation" here, and they have different
lifetimes. Conflating them is why they rot:

| kind | lifetime | truth test | example |
|---|---|---|---|
| **State** | overwritten; only *now* is true | does it match the tree today? | `kernel/HANDOFF.md`, `.ultra/STATE.md` |
| **History** | append-only; never edited | was it true when written? | a journal, `.ultra/CHANGELOG.md` |
| **Explanation** | edited rarely; long-lived | is the reasoning still sound? | `docs/design/*`, `CLAUDE.md` |

`HANDOFF.md` is a **state** document. A journal is **history**. They must not be
the same file and must not be written by the same process — that was the mistake
in the prototype, and it is the separation you asked for.

The failure mode differs per kind, so the automation must differ per kind:

- State rots **silently** and is the dangerous one. `.ultra/STATE.md` said the
  compositor was unreachable while `kernel.zl` called it six times.
- History cannot rot; it can only be **missing**. Nothing writes it today.
- Explanation rots **slowly**, and mostly at its edges — the paths and counts it
  cites. 19 stale references are already baselined.

## 2. Doc taxonomy — what should exist, everywhere

The ask is "all docs, in every folder". Proposed set, with one owner each:

### 2.1 Per-directory orientation — `README.md` in every folder

~15 directories have none: `kernel/`, `kernel/tests/host/`, `kernel/docs/`,
`tools/`, `docs/`, `docs/design/`, `examples/`, `tests/`, `stdlib/`,
`freestanding/`, `editors/`, `learn/`, `bench/`, `interp/`, `nativegen/`.

Each gets three sections, split by lifetime:

```
# <folder> — one line on what lives here

<hand-written: why this folder exists, what to read first, what not to touch>

## Files                      <-- GENERATED, regenerated on demand
| file | lines | what it is |            (the "what it is" column comes from
                                          the file's own first comment block)

## Entry points               <-- GENERATED
<scripts here, and what each produces>
```

The generated half means the inventory can never be wrong. The hand-written half
is the part worth reading, and it is never touched by a tool.

**Bootstrapping honestly:** a generator can fill the inventory for all 15 today.
It cannot write the prose. Options are (a) leave the prose empty with a `TODO:`
marker and let a doc-coverage check nag, or (b) have an agent draft each one for
you to edit. Recommend (a) first — an empty section is honest, a hallucinated
paragraph about `nativegen/` is worse than nothing.

### 2.2 The four top-level documents, separated

| file | kind | written by | answers |
|---|---|---|---|
| `kernel/HANDOFF.md` | state | **human/agent, deliberately** | where is this right now |
| `docs/JOURNAL.md` | history | tool, append-only | what happened, and when |
| `TODO.md` | derived | tool, fully regenerated | what is outstanding |
| hazards doc | explanation | human, checked by tool | what will bite you |

The hazards doc is new and is worth pulling out of `CLAUDE.md`: the EFI/LLP64 rule,
the panel-power rule, the "code exists ≠ code works" rule, the gate-timing rule.
`CLAUDE.md` and `AGENTS.md` then *link* to it instead of restating it, so there
is one copy to keep true. Every hazard gets a registered claim (§4.3) so the doc
cannot quietly stop being accurate the way the `-Werror=` claim did.

### 2.3 Design docs — coverage, not just correctness

`docs/design/` has ~30 files. Two gaps worth closing:

- **An index.** A design-doc README listing each doc, its status
  (proposed / decided / landed / superseded), and its date. Several existing docs
  are decided-but-not-landed (`design_scoping_decision.md`), which a reader
  cannot tell without opening them.
- **A status header.** One frontmatter line per design doc:
  `status: decided | landed | superseded | proposed`. Then the index generates,
  and "decided but not landed" becomes a queryable state rather than folklore.

## 3. What happens on commit and on push

You asked for "whenever it commits or pushes, write everything down, check the
docs, make a new todo, check the code is good". All of that is right; putting
all of it in one hook is not. Tiering by cost:

| when | budget | does |
|---|---|---|
| **pre-commit** | <2 s | doc formatting, trailing whitespace, fence languages |
| **post-commit** | <5 s | append journal entry, regenerate TODO |
| **pre-push** | ~1 min | fast gates (`preflight.sh`) |
| **CI on PR** | ~2 min | everything above + four boot paths + desktop render |
| **nightly** | any | housekeeping sweep, dead-code scan, drift |
| **weekly** | any | doc coverage audit, orphan hunt |

The rule: **anything that can block a push must be fast and must be about
correctness.** Housekeeping is never blocking — it opens an issue or writes a
report. A hook that costs a minute for tidiness is a hook that gets uninstalled,
and you have already seen the pre-push hook block a legitimate push because of
one corrupted untracked file.

## 4. "Make sure the code is good and follows what I want"

This is the vaguest part of the ask and needs to become specific before it can be
built. Your conventions already exist in writing — the ladder, immutability, file
and function size, error handling, no magic numbers. Split them by what can
actually be checked:

### 4.1 Mechanical — a script, no model, blocking

- function longer than 50 lines
- file longer than 800 lines
- nesting deeper than 4
- new `TODO`/`FIXME`/`XXX` without an owner or issue
- `printf`/debug left in kernel code
- a new global in `kernel/*.c` outside the documented memory map
- new file added with no entry in its folder's README

All greppable or countable. Same design as `hazard-scan.sh`: baseline the
existing violations, fail only on an increase.

### 4.2 Judgement — a model, non-blocking, advisory

- does this follow the ladder, or is it a rewrite where a reuse would do
- is the error handling real or performative
- does the naming match the surrounding code
- is the comment explaining *why* or restating the code

This is `codex review` (already wired, needs a key) plus a prompt built from
`AGENTS.md`. It **comments**; it never blocks. A model gate that blocks merges is
a model gate you will learn to rubber-stamp.

### 4.3 Registered claims — the interesting middle

The prototype's best idea, worth keeping: a doc asserts something and names the
command that proves it.

```
CLAUDE.md | the EFI build is LLP64, unsigned long is 4 bytes | <compile a _Static_assert>
```

Nine of these already pass. This is how "follows what I want" becomes checkable
without a model: every convention you care about gets one line and one command,
once, and is then re-checked forever.

## 5. Housekeeping sweep — nightly, non-blocking

Each item below already has evidence in this repo:

| check | why | seen |
|---|---|---|
| tracked build outputs | gitignore does not apply to indexed files | `_genefi.c` |
| documented-but-untracked files | docs promising what the repo lacks | `verify-efi.sh` + 3 |
| untracked-but-important dirs | one disk failure from gone | `.ultra/`, 14 `kernel/docs/*.md` |
| stray files in glob'd dirs | `examples/*.zl` is a test input set | `Zaccoding.zl` breaks the suite |
| dead code | "exists" ≠ "runs" | `lt_armed` paths |
| stale branches | 14 remote branches, several merged | |
| large files | 628 MB working tree vs 13.5 MB packed | |
| duplicate docs | `docs/archive/backups/GAPS_REALWORLD-2026-08-03.backup.md` | |

Output is a report, not a failure. Optionally opens one GitHub issue per class,
updated in place rather than duplicated.

## 6. Implementation options, with the tradeoffs

### 6.1 Where the generators live

- **(a) Shell in `tools/`** — matches everything here today, no dependencies,
  ugly for structured data. What the prototype does.
- **(b) A zl program.** `zl` is self-hosting and the repo's whole thesis is that
  it can do real work. Writing the housekeeping tooling *in zl* is dogfooding
  that finds language gaps — the `stdlib` gaps are already documented. Slower to
  build, and worth far more than the tool itself.
- **(c) Python.** Fastest to write, and a third language in a repo that is
  proudly C-and-zl.

**Recommendation: (a) now, (b) as a deliberate project.** A `zl` reimplementation
of `doc-check` is a genuinely good stress test of the language, and the failures
it hits are `STDLIB_PLAN.md` items.

### 6.2 Generated-vs-written separation

- **(a) Marker blocks** in one file (`<!-- BEGIN GENERATED -->`). One file per
  topic, but a careless edit inside the block is lost.
- **(b) Separate files** — `README.md` hand-written, `README.files.md` generated,
  linked. Nothing is ever lost; twice as many files.
- **(c) Frontmatter + include** — needs a build step.

**Recommendation: (a)**, with the generated block *last* in the file, and a
pre-commit check that fails if a generated block was hand-edited (compare against
a regeneration). That gets (b)'s safety without the file sprawl.

### 6.3 Who runs it on push

- **(a) Git hooks only** — instant, but local-only, bypassable, and must be
  installed per clone *and per worktree*.
- **(b) CI only** — authoritative and unbypassable, but feedback is 2 minutes
  away and it must push commits back.
- **(c) Both, with hooks as the fast mirror of CI.** One definition of green,
  two places to find out. This is what `preflight.sh` already does.

**Recommendation: (c).**

### 6.4 A harder idea worth considering

**Make the docs executable.** Registered claims are the thin end of this: a doc
does not merely *describe* the system, it *names commands that prove* what it
says. Taken further, every design doc's key assertions become a test file, and
`docs/design/design_scoping_decision.md` stops being a document that disagrees
with the code and becomes one that cannot.

This is the same move `verify_fmt.sh` makes for the formatter and
`examples/syntax_tour.zl` makes for the language reference — both already in the
repo, both the strongest artefacts in it. Generalising it is the highest-value
idea in this document.

## 7. Suggested order

1. **Hazards doc** extracted from `CLAUDE.md`, every hazard with a claim.
2. **Per-folder `README.md`** — generated inventory, empty prose with `TODO:`.
3. **Doc coverage check** — every directory has a README; every design doc has a
   status header.
4. **Doc formatter tool** — formatting only, pre-commit, fast.
5. **Code checker tool** (§4.1), baselined like `hazard-scan`.
6. **Journal + TODO** — the held prototype, after the taxonomy above is settled.
7. **Nightly housekeeping report.**
8. **`codex review`** with a key, advisory.
9. **The zl reimplementation**, as its own project.

## 8. Decisions needed before any of it is built

1. **The two held commits** — keep as the starting point, or revert and redo
   under this taxonomy? They conflate journal and TODO generation in ways §1
   argues against.
2. **`HANDOFF.md`** — who writes it? It is state, so it cannot be generated, but
   it is also the doc most likely to be stale. A "verify this is still true"
   checklist on every PR is possible; a generator is not.
3. **Per-folder README prose** — empty with a nag, or agent-drafted for you to
   edit?
4. **Blocking policy** — does a code-convention violation block a push, or only
   report? §4 recommends: mechanical blocks, judgement never does.
5. **`.ultra/` and `kernel/docs/*.md`** — commit them? They are untracked today,
   and `TENSIONS.md` alone is 672 lines of real record living on one machine.
