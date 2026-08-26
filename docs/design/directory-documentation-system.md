# Directory documentation system

Status: implemented locally on 2026-08-26; not committed or pushed. The
acceptance checks in this document define the maintained contract.

## Intent

Every tracked directory in `zl-linux` should explain itself at the point where a
person or coding agent enters it. A reader should not need to infer whether a
folder is active source, generated metadata, retained evidence, archived history,
or third-party code from its name alone.

This system deliberately favors abundant local documentation. It also prevents
that abundance from creating hundreds of independent and contradictory sources
of project truth.

## Scope

The rollout covers the repository root and every directory that contains a
tracked file directly or indirectly. At the planning snapshot there are 112
nested tracked directories and 977 tracked files.

The following are not project directories and are excluded:

- `.git/` internals;
- ignored compiler, test, image, screenshot, and QEMU output;
- transient caches and editor state;
- directories outside this checkout.

A newly tracked directory enters scope automatically. A static check must fail
until its documentation capsule exists.

## The capsule

Every in-scope directory has these local surfaces:

| File | Question answered | Authority |
|---|---|---|
| `README.md` | What is here, and where do I start? | Local orientation. Existing hand-written files are preserved. |
| `CLAUDE.md` | What must an editing agent know here? | Local instructions composed with ancestor instructions. |
| `AGENTS.md` | What must Codex and compatible tools know here? | A relative symlink to the same local `CLAUDE.md` contract. |
| `STATUS.md` | What kind of directory is this, and what can honestly be claimed? | Local classification and evidence boundary, not product completion authority. |
| `TODO.md` | What work is explicitly assigned here? | Local queue only. An empty queue never means the subsystem is complete. |
| `RULES.md` | What placement, editing, and validation rules apply? | Local rules plus inherited repository policy. |
| `VALUES.md` | Which project values control trade-offs here? | Stable decision guidance, not a status ledger. |
| `SOP.md` | What sequence should a contributor follow when changing this area? | Local operating procedure and verification handoff. |

The root already has hand-written `README.md`, `AGENTS.md`, `CLAUDE.md`, and
`TODO.md`; those remain authoritative and are not replaced. Root-level
`STATUS.md`, `RULES.md`, `VALUES.md`, and `SOP.md` complete its capsule.

## Truth hierarchy

The local files are navigation and operating context. They do not replace the
canonical project authorities:

1. `docs/PROJECT-STATUS.md` for current repository and implementation truth;
2. `docs/REMAINING-WORK.md` and `docs/EXECUTION-ROADMAP.md` for current delivery
   order;
3. `docs/program/` for the complete product program;
4. dated receipts under `docs/evidence/` and `kernel/docs/evidence/` for measured
   claims;
5. `docs/archive/` and `kernel/docs/archive/` for history only.

No local `STATUS.md` may promote a feature from planned to implemented, from
implemented to tested, from host-tested to booted, or from QEMU-tested to
physical-hardware proven. Those transitions require their canonical evidence.

## Directory classes

Every directory receives one class. Class drives the wording of its status,
rules, values, and SOP.

| Class | Meaning | Local queue policy |
|---|---|---|
| `source` | Active language or zlOS implementation | May hold explicit tasks; existence is never completion. |
| `proof` | Bounded self-hosting, freestanding, learning, benchmark, or example lane | Tasks must preserve the lane's stated limits. |
| `tests` | Tests, fixtures, or independent oracles | Tasks describe coverage gaps, never product implementation. |
| `tooling` | Build, gate, generator, probe, editor, or automation support | Changes must preserve a runnable deterministic check. |
| `documentation` | Current orientation, design, program, reference, or research docs | Status labels and authority boundaries are mandatory. |
| `evidence` | Dated or measured retained proof | Append or supersede; do not rewrite historical observations. |
| `archive` | Superseded material retained for provenance | No active TODOs; promote a copy to a current area before reuse. |
| `generated` | Machine-readable generated truth or metadata | Change the owner/generator, then regenerate. |
| `vendor` | Third-party material | Do not edit except for a deliberate vendor update with provenance. |
| `support` | Repository configuration or small supporting metadata | Keep changes narrow and compatible with the owning tool. |

## Generated and local content

The capsule generator owns a bounded generated block in each managed Markdown
file. A `BEGIN LOCAL` / `END LOCAL` block is preserved verbatim so directory-
specific knowledge can accumulate without forking the common contract.

Rules for local blocks:

- record only facts specific to that directory;
- cite a receipt, source path, gate, or failure when adding a hazard;
- never copy root-wide policy into every child;
- never mark work complete from file existence alone;
- remove or supersede stale instructions when their evidence changes.

Existing hand-written `README.md` files are not rewritten. Missing READMEs are
created with purpose, classification, evidence boundary, child directories, and
direct-file inventory.

## TODO semantics

Every directory receives `TODO.md`, but not every directory receives invented
tasks.

- An empty local queue says only that no task is assigned in that file.
- Product work remains in the canonical roadmaps and program ledgers.
- Evidence, archive, generated, and vendor directories default to no active local
  queue and explain where changes must originate.
- A task may be checked only when its requested outcome and required evidence
  both exist.
- Host, build, QEMU, graphical, and physical evidence remain separate.

## Agent semantics

Nested `AGENTS.md` files compose with the root contract. They contain only local
purpose, class rules, evidence limits, and links back to canonical authority.
They do not duplicate the large root hazard manual.

`CLAUDE.md` is the regular local instruction file and `AGENTS.md` is a relative
symlink to it in nested directories. This matches the existing root convention
and prevents Codex and Claude from receiving divergent local rules. The root
pair remains hand-written and outside generator ownership.

## Maintenance tooling

`tools/directory-docs.py` owns generation and checking.

- Default mode creates missing capsule files and refreshes generated blocks.
- `--check` is read-only and fails on missing files, broken Claude links,
  malformed generated/local markers, uncovered tracked directories, or stale
  generated content.
- The tool emits `docs/DIRECTORY-CAPSULE-INDEX.md`, the complete directory,
  class, state, purpose, and coverage table.
- The check reads tracked paths from Git so ignored output cannot become policy.
- Generated output is deterministic and must be byte-identical on a second run.

The repository documentation check should invoke `directory-docs.py --check` so
new folders cannot silently escape the system.

## Implementation order

1. Inventory every tracked directory and preserve all existing hand-written
   orientation files.
2. Implement deterministic classification, local purpose descriptions, relative
   authority links, generated/local blocks, and Claude symlinks.
3. Generate all directory capsules and the central index.
4. Add the static coverage check to the existing documentation gate.
5. Run the generator twice, require a clean second diff, then run the static
   documentation checks.
6. Audit every `STATUS.md` and `TODO.md` class for false completion language.
7. Record exact file counts and exclusions in a dated evidence receipt.

## Acceptance criteria

The rollout is implemented only when all of these are true:

- every tracked directory and the root have all eight capsule surfaces;
- every capsule surface is tracked by Git; filesystem presence alone is not
  coverage;
- every nested `AGENTS.md` resolves to its sibling `CLAUDE.md`;
- existing hand-written READMEs are byte-preserved;
- every directory appears once in the central index;
- archive, evidence, generated, and vendor areas cannot imply active completion;
- every TODO explains that an empty local queue is not completion;
- a newly tracked undocumented directory makes `--check` fail;
- generation is deterministic and idempotent;
- repository doc checks pass;
- the final receipt states that no compile, QEMU boot, or hardware test was run.

## Non-goals

- This rollout does not implement any of the 906 product features.
- It does not refresh dated implementation evidence.
- It does not boot, compile, flash hardware, publish, or push.
- It does not reorganize source ownership again.
- It does not turn archive or research material into current authority.
