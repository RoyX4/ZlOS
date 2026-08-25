# Ideas for actually using Codex and Cursor here

Written 2026-08-19 by Claude (`source: claude`). **Ideas, not decisions.**
Companion to [lie-detection-and-review.md](lie-detection-and-review.md) and
[housekeeping-plan.md](housekeeping-plan.md).

Capabilities below were checked on this machine, not assumed. Where something is
unverified it says so.

---

## A. Fix the bottleneck first: none of them know zl

Every idea in sections B–D is worth less than these three, because a model that
pattern-matches zl to Python produces confident nonsense faster than you can read
it. That has already happened here.

### A1. Expose `./interp` as an MCP tool — the single highest-value item

Both CLIs take MCP servers (`codex mcp add`, `cursor-agent mcp`, `.cursor/mcp.json`).
A tiny stdio server offering two tools:

```
zl_run(source)   -> stdout/stderr from ./interp
zl_parity(source)-> {interp, compile, compilel, nativegen} outputs side by side
```

Then **any** agent can settle a zl question by running it instead of guessing —
before it files a finding, not after you correct it. `zl_parity` additionally
makes the engine divergence visible at the moment it matters.

This turns "the interpreter is ground truth" from a sentence in `AGENTS.md` into
something the model can *act on*. It is roughly 60 lines of glue and it removes
the single largest failure mode of putting three agents on this repo.

### A2. Test the brief itself, empirically

`AGENTS.md` is a hypothesis: *"reading this makes a model competent at zl."*
Nobody has tested it.

The test is cheap. Give a model **only** `AGENTS.md` + `examples/syntax_tour.zl`,
ask for ten small programs with known outputs, run them through `./interp`, count
passes. That number is the brief's quality score.

Then iterate on the brief and watch the number move. Run it across
`cursor-agent --model gpt-5`, `--model sonnet-4-thinking`, `--model claude-opus-4-8`
and you also learn which family to point at zl work — an empirical answer to a
question everyone else guesses at.

Failures are more valuable than the score: each one names a thing the brief does
not teach, which is exactly what the next paragraph of the brief should say.

### A3. A zl-specific eval set that lives in the repo

Formalise A2 as `tests/agent-eval/`: prompts, expected outputs, a runner. It
becomes a regression test **for the documentation**, which is a thing almost no
project has. When someone edits `AGENTS.md`, the score says whether they helped.

## B. Get work off the four-core box

### B1. `codex cloud exec` for the 34 truncation sites

Mechanical, fully specified, and spread over 5 files: replace `-w` with
`-Wno-everything`, then fix each `(unsigned long)` on a pointer to
`unsigned long long`. Perfect offload:

```bash
codex cloud exec "..."   # runs on OpenAI's infrastructure
codex cloud diff         # read what it did
codex cloud apply        # land it locally
```

Your cores stay free, and `verify-efi.sh` in CI is the check on the result.
`codex cloud` is flagged EXPERIMENTAL by OpenAI and I have not run it.

### B2. The ThinkPad as a self-hosted runner — unlocks the "must stay local" work

I said `intel.c` and `hosttest/` can never be CI, because no runner has a Gen9
GPU. That is true of *GitHub's* runners. It is not true of a self-hosted one.

Registering the ThinkPad X1 Carbon as a self-hosted runner with a `hardware`
label means:

```yaml
runs-on: [self-hosted, gen9]
```

and `gpu-dev.sh probe`, `modeset_test --survey`, `intel_probe` run **against the
real panel, in CI**, on a PR label or on demand. The one category I called
permanently manual becomes automatable.

Caveats worth taking seriously: the runner executes untrusted PR code on a
machine you care about, so restrict it to `workflow_dispatch` and branches you
own; `modeset-run.sh` blanks the screen, so the laptop must be idle; and the
panel-power hazards do not stop being hazards because CI is driving.

### B3. `cursor-agent worker`

A private cloud worker that runs agents in your environment, with
`/healthz` + `/readyz` probes. Same idea as B2 but for agent work rather than
gates. Only worth it if you end up with a spare machine — on the 4-core box it
is another process competing for the same cores.

## C. Compose the tools instead of alternating between them

### C1. `codex mcp-server` — Codex as a tool *inside* Claude Code

`codex mcp-server` runs Codex as an MCP stdio server. Wired into Claude Code,
a second opinion becomes an inline tool call rather than a separate session:
mid-task, on one function, without losing context.

That is the cheapest possible version of "two models fail differently" — no PR,
no workflow, no context reload.

### C2. Structured verdicts with `--output-schema`

`codex exec --output-schema <FILE>` forces JSON conforming to a schema. So the
refuter in the review plan returns:

```json
{ "refuted": true, "reason": "...", "ran": ["./interp x.zl"], "confidence": "high" }
```

Machine-consumable, so surviving findings can feed `TODO.md` automatically
instead of being pasted by hand.

### C3. Vision review of the desktop render — **verified capability**

`codex exec -i/--image FILE` attaches images. `wmshot` already renders the
compositor to a PNG in milliseconds.

So: render the desktop, hand the image to a model, ask *"what looks wrong?"*

This is the exact class `wmshot` exists for — the pill that rendered as a circle
passed all 47 assertions and was caught by a human looking. A vision model looks
on every PR. Alignment, spacing, contrast, a title bar four pixels too tall.

Strongest version: attach **both** the base and head renders and ask what
changed and whether the change is intentional.

I have verified the `-i` flag exists; I have not run this end to end.

### C4. Cursor's editor where it is actually unbeatable

Register tables. `intel.c`, `pci.c`, `xhci.c` are full of repetitive, patterned
declarations — offsets, bitfields, device IDs. That is precisely where next-edit
prediction wins and where an agent round-trip is pure overhead.

Also: reviewing the giant diffs this project produces, hunk by hunk, in place.

## D. Point them at the actual backlog

### D1. The zl stdlib — the best delegation target in the repo

`docs/design/STDLIB_PLAN.md` and `GAPS_REALWORLD.md` already specify it. Stdlib
work is well-specified, pure zl, testable through `run_tests.sh`, and touches no
hardware and no boot path — the lowest-hazard, highest-value work available to an
agent here.

It also unblocks writing the housekeeping tooling *in zl*, which is the dogfooding
idea from the housekeeping plan.

### D2. A devil's advocate on open design decisions

`design_scoping_decision.md` is decided but not landed, and both unboxed backends
already implement the other side. Have a different model argue **against** the
adopted option, with the corpus numbers in hand (168 assignment sites, 24 of 111
files, `src/selfhost/compiler.zl` needs 8).

A design decision that survives an adversarial read from a model that did not
make it is worth more than one that was never challenged.

### D3. `codex fork` for two approaches from one context

```bash
codex fork --last "now do it with a ring buffer instead"
```

Explore both branches of a decision without re-explaining the problem twice.

### D4. `cursor-agent generate-rule` to bootstrap the per-directory files

Cursor ships a rule generator. Use it as the first draft of the seven
`AGENTS.md` files in the review plan, then edit — faster than a blank page, and
the editing is where the real content comes from anyway.

## E. Guardrails that should apply to all of the above

- **`codex sandbox` for anything the zl toolchain just emitted.** A compiler under
  development producing native code is the definition of untrusted. Default is
  read-only with no network — verified.
- **Never delegate panel power.** T12, AUX, `lt_armed` write paths. A cloud agent
  told to "make the display driver work" has no way to know the panel is real.
- **Nothing merges on a model's say-so.** Findings are hypotheses until a gate or
  a refuter agrees.
- **Every delegated task names its check.** If you cannot say which gate proves it
  worked, it is not ready to delegate.

## Rough ranking

| | idea | cost | why |
|---|---|---|---|
| 1 | **A1** `interp` as an MCP tool | low | removes the largest failure mode of using any of them here |
| 2 | **C3** vision review of `wmshot` | low | verified capability, catches what assertions structurally cannot |
| 3 | **A2** test the brief | low | makes doc quality a number instead of an opinion |
| 4 | **C1** `codex mcp-server` in Claude Code | low | inline second opinion, no context reload |
| 5 | **B2** ThinkPad as a runner | medium | converts the "permanently manual" category |
| 6 | **D1** stdlib delegation | medium | the safest real work, and it unblocks zl tooling |
| 7 | **B1** cloud-exec the 34 fixes | medium | experimental, but the task is perfectly specified |
| 8 | **D2** devil's advocate on scoping | low | the decision is load-bearing and unchallenged |
