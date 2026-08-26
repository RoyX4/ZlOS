<!-- Managed by tools/directory-docs.py. Edit only the LOCAL block. -->
<!-- BEGIN GENERATED: directory-docs -->
# Local agent contract: `src/tools/`

## Scope

Language-aware developer tools such as zlfmt. This file adds local constraints to every ancestor `AGENTS.md`;
it does not replace them.

## Read first

- Local [README](README.md), [status](STATUS.md), [tasks](TODO.md),
  [rules](RULES.md), and [SOP](SOP.md).
- Canonical [project status](../../docs/PROJECT-STATUS.md) and [code map](../../docs/CODE-MAP.md).

## Local contract

- Do not infer completion from a file, symbol, target, or document existing.
- Keep host, build, QEMU, graphical, and physical evidence explicitly separate.
- Keep tools deterministic, bounded, and explicit about skipped or unavailable checks.

Before reporting completion, re-read [STATUS.md](STATUS.md), run the smallest
relevant deterministic checks, and state every unrun evidence lane.
<!-- END GENERATED: directory-docs -->

<!-- BEGIN LOCAL: directory-docs -->
<!-- Add verified directory-specific notes here. -->
<!-- END LOCAL: directory-docs -->
