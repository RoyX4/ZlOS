# DOCS-RECONCILE-PROMPT — turn 21 plan documents into one honest "what's left"

Written 2026-08-19, for a fresh session. Everything you need is in the repo;
nothing depends on a chat you cannot read.

## The job

There are **21 planning documents totalling ~392 KB**. They were written by
different sessions across several weeks, most of them *before* eleven parallel
branches were merged into `main` on 2026-08-19. They contradict each other, they
contain items that are long since done, and no single one of them says where the
project actually is.

Produce **one ranked document of what is genuinely still open**, with evidence,
and mark the 21 as superseded rather than deleting them.

## Why this is not just "read the docs"

**They are stale by construction.** `desktop-TODO.md` alone is 44 KB of items
written before six of the tracks landed. Reading it tells you what someone
*wanted*, not what is *true*. Every claim has to be checked against the merged
tree.

Three failure modes to expect, all observed already:

1. **Items already done that nobody knows are done.** The merge implemented a
   great deal incidentally. `docs/evidence/MERGE-EVIDENCE.md` §Outcome lists what landed.
2. **Contradictions between documents.** Eight parallel tracks were started
   because two docs disagreed about what mattered. Where two docs conflict, say
   so explicitly rather than picking one silently.
3. **Items superseded by a design change.** e.g. anything describing the static
   desktop, `bg_snapshot`/`grab`/`stamp` drag, or the full-screen demos: the
   compositor replaced all of it and the code is deleted. A doc that still plans
   against it is not "open", it is obsolete.

## The corpus

All 21 are on `main` as of this commit - the six that were orphaned in
`refs/wip/*` snapshots have been rescued into `kernel/docs/`, so you do not need
any archaeology.

```
kernel/docs/desktop-TODO.md                    44 KB
kernel/docs/gen9-modeset-plan.txt              59 KB   (JSON; text in .result.plan)
kernel/docs/archive/prompts/OVERNIGHT-PROMPT.md                35 KB   <- produced 19 compositor commits
kernel/docs/evidence/desktop-v10-plan.md       16 KB
kernel/docs/display-roadmap.md                 14 KB
kernel/docs/desktop-northstar-feasibility.md   11 KB
kernel/docs/desktop-plan.md                    20 KB
kernel/docs/feature-catalogue.md               20 KB
kernel/docs/os-landscape.md                    15 KB
kernel/docs/what-is-actually-impossible.md      9 KB
kernel/docs/wireless-plan.md                    9 KB
kernel/docs/archive/prompts/BROWSER-PROMPT.md                  13 KB
kernel/docs/archive/prompts/EXEC-PROMPT.md                     12 KB
kernel/docs/archive/prompts/FEEL-PROMPT.md                     13 KB
kernel/docs/archive/prompts/PLATFORM-PROMPT.md                 15 KB
kernel/docs/archive/prompts/SYSTEM-PROMPT.md                   12 KB
kernel/docs/POINTER-PROMPT.md                   9 KB   <- CURRENT, do not audit, see below
docs/archive/superseded/INTEGRATION-PLAN.md                       16 KB   <- superseded by MERGE-EVIDENCE.md
docs/design/PLAN_unboxing.md                   31 KB
docs/archive/superseded/GRAPHICS_PLAN.md                    8 KB
docs/design/STDLIB_PLAN.md                      8 KB
```

Read first, before any of them: `docs/evidence/MERGE-EVIDENCE.md` (what landed and what
the merge cost), `kernel/HANDOFF.md` (verified state), `docs/CODE-MAP.md` (where
the code actually is - the layout is not what the directory names imply).

## Method

One reader per document. Each reader **must verify, not summarise**:

- an item claiming a function should exist -> `grep` for it, report the line or
  its absence
- an item claiming a gate should pass -> check the gate exists in
  `kernel/verify*.sh` or `kernel/hosttest/`, and whether it is in
  `gates/land-gate.sh`
- an item about a file -> check the file still exists and is in `kernel/SOURCES`
  if it is meant to be compiled
- an item about zl -> `kernel/check-zl-calls.sh` proves whether a builtin is
  actually reachable

Return per item, structured:

```
doc, item, what it asked for, status, evidence
status ∈ done | partial | not-started | superseded | contradicted | unverifiable
```

`unverifiable` is a legitimate answer and is better than a guess. Say what would
settle it.

Then a **cross-check pass**: with all readers' output in hand, find (a) the same
item claimed by two docs with different status, (b) direct contradictions, (c)
items no doc owns but the code clearly needs.

Then **one synthesis**: a single ranked file, `docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`,
grouped by subsystem, ordered by leverage, each entry carrying its evidence and
its source doc. Add a header to each of the 21 pointing at it.

## Scale it properly

~392 KB of documents plus the code checks does not fit one context alongside the
verification. Fan out - one agent per document, a cross-check stage, then
synthesis. This is the same shape as the merge analysis in
`docs/evidence/MERGE-EVIDENCE.md`, which found eight silent id collisions and a memory
map that would have corrupted every AP stack; the pattern earns its keep.

Respect the box: 8 cores, 15 GB. Gates run backgrounded and sequentially and
never alongside a fan-out - `CLAUDE.md` has the measured consequences.

## Do NOT audit these two

- **`kernel/docs/POINTER-PROMPT.md`** is live work, not history. The pointer is
  visibly broken in `./try.sh` and the cause is measured (two drainers of one
  xHCI event ring, `xhci.c:1775` and `xhci.c:1789`). It also carries a confirmed
  visual regression, the dock readout leaving digit debris. Both are OPEN. Fold
  them into the final document as open items; do not re-diagnose them.
- **`docs/archive/superseded/INTEGRATION-PLAN.md`** is superseded by `docs/evidence/MERGE-EVIDENCE.md` and
  known to contain wrong figures (it says 332 commits were at risk; the whole
  repo is 186). Record it as superseded and move on.

## Known-open, so the audit does not "discover" them again

From `docs/evidence/MERGE-EVIDENCE.md` §Still open:

- `key()` is called in `kernel.zl` on the panel-handover path and defined
  nowhere, on every branch, since `b55f3f9`.
- `font_big.c` and `icons_rgb.c` are referenced by nothing.
- `ci/gates-and-agent-brief` is 9 commits unmerged and carries the fix that
  makes `build.sh` build `zlfmt`.
- 49 further files exist only in `refs/wip/*` and on no branch - the whole
  `learn/` course (23 files), `crypto.c` + its test, `zlfmt.c`, 7 probes.
  `git fetch origin 'refs/wip/*:refs/wip/*'` to see them.
- `intel.c`'s `edid_buf` is hardcoded at `0x0C980000`, inside `HI_BLUR`.
  `intel_set_edid_buffer()` exists; the fix is one call.

## What done looks like

`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md` exists, every entry has evidence, every one of
the 21 docs has a superseded header pointing at it, and a person can read one
page and know what to do next. If an item cannot be verified, it says so.
