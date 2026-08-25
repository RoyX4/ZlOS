# Repository structure cleanup receipt — 2026-08-25

## Scope

This receipt covers the static cleanup series from remote checkpoint
`b8e5f8a45164` through local structure checkpoint `c8644791bea1` on
`codex/full-program`. The user explicitly excluded compilation, QEMU, physical
booting, and pushing during this run.

## Measured outcome

- 40 local commits after the remote checkpoint;
- 293 tracked files changed across the full cleanup series;
- 119 tracked relocations recorded by Git;
- 40 Python entry scripts changed from mode `100644` to `100755`, with no
  content changes;
- top-level `kernel/docs/` files reduced from 83 to 25;
- 12 purpose-based first-level documentation directories now separate
  architecture, concepts, desktop, drivers, evidence, features, guides, plans,
  receipts, references, research, and archive;
- zero non-ignored untracked files at the checkpoint;
- zero dirty tracked files at the checkpoint.

The canonical layout is documented in [`REPOSITORY-STRUCTURE.md`](../REPOSITORY-STRUCTURE.md),
the source ownership map is [`CODE-MAP.md`](../CODE-MAP.md), and the detailed
kernel documentation taxonomy is [`kernel/docs/README.md`](../../kernel/docs/README.md).

## Truth-state cleanup

- Superseded plans, prompts, TODOs, handoffs, feasibility percentages, and the
  contradicted display roadmap moved under `archive/`.
- Completed desktop, physical-input, MP-00 execution, and Gen9 display records
  moved under `evidence/`.
- Current plans now carry explicit status boundaries; old task lists are not
  presented as the current queue.
- Research, guides, architecture contracts, stable references, driver notes,
  desktop design notes, and feature contracts each have a named home.
- Every moved path was updated in source comments, tests, tools, handoffs,
  historical audits, and orientation maps; no compatibility stubs were left.

## Static verification

The following checks passed after the final moves:

```text
git diff --check
tools/doc-check.sh --paths-only
tools/doc-check.sh --selftest-links
tools/todo.sh --selftest-tensions
jq empty kernel/docs/evidence/display/gen9-modeset-plan.json
```

The documentation checker confirmed that every referenced path exists, every
local Markdown link resolves, and every file described as tracked is in Git.
The two self-tests also proved they still catch planted failures.

## Deliberately held boundaries

- `.ultra/TENSIONS.md` T-8 remains open. Release notes, provenance viewer,
  evidence registry, and decision ledger inputs disagree on build identity.
  Generator-coupled root documents were not moved or regenerated piecemeal.
- `kernel/docs/mp00-source-snapshot-2026-08-24.md` and
  `kernel/docs/host-freeze-and-gate-containment-2026-08-24.md` remain at their
  hashed decision-ledger paths until that chain can be regenerated atomically.
- `kernel/boot_state.c` and `kernel/boot_state.h` remain the documented root
  source exception. Moving them requires the host compile gate, which this run
  was explicitly not allowed to execute.
- `kernel/docs/visual-speed-northstar.md` remains at its established path because
  `docs/JOURNAL.md` links to it and journals are not edited without explicit
  approval.
- Ignored build products in the repository root and kernel tree were not moved
  or deleted. Their placement and generators are documented in `CODE-MAP.md`.

## Evidence boundary

No compilation, sanitizer run, QEMU boot, physical boot, or hardware test was
performed. No remote branch was updated. This receipt proves repository and
documentation structure only; it does not promote implementation or runtime
claims.
