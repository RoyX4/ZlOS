# Directory documentation rollout receipt

Date: 2026-08-26

Checkout: `/home/roy/Documents/repos/zl-linux-final`

Branch: `codex/full-program`

Publication state: staged locally, not committed, not pushed

## Outcome

Every repository directory containing tracked project content, plus the root,
now has the full local documentation capsule:

- `README.md`
- `CLAUDE.md`
- `AGENTS.md`
- `STATUS.md`
- `TODO.md`
- `RULES.md`
- `VALUES.md`
- `SOP.md`

The measured scope is 113 directories and 904 capsule surfaces. All eight names
resolve in all 113 directories. The 112 nested `AGENTS.md` entries are relative
symlinks to their sibling `CLAUDE.md`, matching the root convention and keeping
one instruction source per directory.

The final staged diff contains 875 files: 868 additions and seven deliberate
updates to existing front doors or checks. There are no unstaged or untracked
files in this checkout.

Existing hand-written READMEs were preserved by the generator. The root README,
`docs/design/README.md`, `docs/README.md`, and `tools/README.md` received small
deliberate navigation or maintenance additions outside generator ownership.

## Architecture

[`../design/directory-documentation-system.md`](../design/directory-documentation-system.md)
is the governing design. [`../DIRECTORY-CAPSULE-INDEX.md`](../DIRECTORY-CAPSULE-INDEX.md)
is the generated directory map. `tools/directory-docs.py` creates missing
surfaces, refreshes deterministic blocks, preserves active-area LOCAL blocks,
and checks coverage without writing in `--check` mode.

Directory discovery ignores the capsule files themselves. Removing the last
real project file therefore removes a leaf from the required set instead of
letting its generated documentation keep an orphan alive forever.

Archive, evidence, generated, and vendor capsules have immutable LOCAL blocks.
They cannot become shadow task queues or rewrite dated observations. Their TODOs
explicitly say that they have no default active implementation queue.

## Truth boundary

The local files do not replace the project authorities:

- `docs/PROJECT-STATUS.md` remains current repository and implementation truth;
- `docs/REMAINING-WORK.md` and `docs/EXECUTION-ROADMAP.md` remain current order;
- `docs/program/` remains the complete destination;
- dated evidence remains bounded to its recorded lane;
- archive material remains historical.

Every generated `STATUS.md` says that product completion is not determined by
the directory. Every generated `TODO.md` says an empty local queue is not a
completion claim. A scan found zero active checkboxes in nested generated TODOs
and zero positive physical-completion phrases in generated status/task files.

## Independent challenge

Two read-only agent reviews challenged the design. Their useful findings were
resolved as follows:

- reversed instruction symlinks were corrected to `AGENTS.md -> CLAUDE.md`;
- filesystem-only coverage was rejected; every capsule must also be in Git;
- discovery now excludes capsule files to prevent self-sustaining orphan trees;
- `kernel/docs/receipts/` and display assets receive evidence/generated handling;
- evidence, archive, generated, and vendor LOCAL blocks are immutable;
- the root index link was corrected;
- the documentation wrapper no longer calls reporting helpers before defining
  them;
- React vendor provenance and its missing tracked license text are explicit in
  `kernel/tests/refrender/vendor/PROVENANCE.md`.

The recommendation to collapse the system to one README per directory was not
adopted because the repository owner explicitly requested every individual
folder to expose each named surface. Repetition is bounded in deterministic
generated blocks, while local authority remains centralized.

## Static evidence

The following checks passed:

```text
python3 tools/directory-docs.py
  updated 0 path(s) on the idempotence pass

python3 tools/directory-docs.py --check
  PASS (113 directories, 8 surfaces each)

tools/doc-check.sh --paths-only
  directory capsules complete and current
  every referenced path exists
  every local Markdown link resolves
  every described file is tracked

git diff --cached --check
  exit 0
```

Measured structural assertions:

```text
README.md   113
AGENTS.md   113
CLAUDE.md   113
STATUS.md   113
TODO.md     113
RULES.md    113
VALUES.md   113
SOP.md      113
index rows  113
nested AGENTS.md symlinks 112
```

A temporary-index mutation planted a new tracked directory named
`.directory-docs-selftest/` without a purpose or capsule. The checker exited 1
and named that exact directory. The temporary file and index were removed; the
real index was never changed by the mutation.

## Open limits

- The complete eight-surface system is intentionally large. Its value depends on
  keeping common content generated and putting only verified local facts in
  editable blocks.
- The React runtime files identify an MIT license but the referenced license text
  is not tracked. Publication-ready vendor provenance remains open.
- This documentation rollout does not refresh dated product implementation
  evidence or close any product-program feature.

## Evidence not run

No language or kernel compilation, sanitizer, test suite, QEMU boot, graphical
exercise, physical boot, hardware test, flash, commit, remote update, or GitHub
push was performed for this rollout. The evidence above is repository structure,
deterministic generation, Git tracking, and documentation-link validation only.
