# Documentation Coverage Audit - 2026-08-25

Status: static documentation and navigation audit passed in the isolated
restructure verification tree. This receipt does not claim implementation of
the complete product program.

## Answer In One Paragraph

The complete destination is documented. The repository contains a locally
verifiable 906-feature catalogue, MP-00 through MP-20, all 174 research
contracts, 609 normalized driver/service/application/platform targets, the 61
named current implementations, one All Applications catalogue surface, and 24
games. Most of that destination is not implemented: the checked-in status
snapshot labels 877 rows `PLANNED_UNPROVED`, 22 `PARTIAL_CURRENT`, and 7
`PROVED_CURRENT` for bounded contracts. That snapshot is dated and must not be
treated as fresh evidence after the restructure.

## Where A New Contributor Starts

1. [`../PROJECT-STATUS.md`](../PROJECT-STATUS.md) separates repository,
   implementation, plan, QEMU, and physical-hardware state.
2. [`../README.md`](../README.md) maps current docs, evidence, design work, and
   archives.
3. [`../program/README.md`](../program/README.md) explains the complete program
   and what each program artifact does and does not prove.
4. [`../program/PRODUCT-IMPLEMENTATION-ORDER.md`](../program/PRODUCT-IMPLEMENTATION-ORDER.md)
   gives the nine human product waves.
5. [`../program/PHASES.md`](../program/PHASES.md) is the MP-00 through MP-20
   dependency authority.
6. [`../program/FEATURE-MAP.md`](../program/FEATURE-MAP.md) maps every feature
   ID to one primary phase and workstream.
7. [`../program/research/README.md`](../program/research/README.md) opens the
   retained 33-repository research, matrices, deep dives, refutations, and
   canonical source catalogues.

## Measured Coverage

Command:

```sh
python3 tools/validate_master_program.py --self-test
```

Result:

```text
master-program self-test: PASS: missing-feature, duplicate-feature,
unknown-prefix, unknown-contract
master-program: PASS: features=906 prefixes=25 phases=21
research_contracts=174 registries=609 research_docs=41 repositories=33
current_named=61 catalogue=1 games=24
```

The validator now consumes `docs/program/research/` inside this repository.
It no longer requires a sibling `zl` or `zl-merge-all` checkout. The retained
research shelf contains 36 top-level documents plus four retained zl language
plan snapshots and their orientation page. It includes the canonical catalogue,
all 33-repository matrices, three contract backlogs, architecture plans,
focused and mature-system deep dives, source snapshot manifest, and refutation
reports. All local Markdown links across the 12 program documents and 41
retained research documents resolve within this repository.

The source manifest and the feature, driver/application, and visual/application
matrices each contain 33 unique repository rows. Their repository sets match
exactly; the validator fails if any one ledger drops, duplicates, or substitutes
a repository.

## Navigation Check

Command:

```sh
./tools/doc-check.sh --paths-only
```

Result: pass after staging the complete atomic migration in the isolated
verification tree. Every current referenced path exists and every described
source/script is part of the migration.

`docs/program/research/` is deliberately excluded from local-path resolution:
its path citations name files in the 33 donor repositories, not files promised
by zlOS. Archive folders are excluded because their paths preserve historical
state. Eleven proposal references remain in the existing documented baseline;
they name planned stdlib/tests/config files that do not exist and are not
current navigation promises.

## Status Boundaries

- `FEATURE-MAP.md` proves catalogue coverage and phase assignment only.
- `FEATURE-STATUS.json` is a dated evidence join, not a live implementation
  database and not a completion claim.
- `PARTIAL-CLOSURE.md` is the dated closure plan for a 23-row batch: 22 rows
  remained partial and one bounded row had been promoted.
- `PROOF-GATES.md` defines what evidence would be required; it is not itself
  evidence that a gate ran.
- Research `Keep`, `Deepen`, `Add`, `Later`, `Fixture`, and `Reject` states are
  research classifications, not implementation maturity.
- A moved file has changed ownership/navigation only. Its feature status does
  not improve because of the move.

## Verification Not Run

No full host suite, image build, QEMU run, graphical comparison, or physical
hardware gate was run for this documentation audit. A broad host build was
started while the earlier scope still appeared to include implementation and
was stopped immediately when the user clarified the request. Partial compiler
output is not a green receipt.

At the time of this audit the restructure was uncommitted and unpushed, and the
original dirty shared checkout was not rewritten. It was subsequently committed
and published; GitHub docs, repository gates and all four QEMU boot jobs passed
at `a5c6f4e4306d`. That later result does not retroactively turn this static
documentation audit into runtime or physical evidence.

## Weakest Link

The 906-row status snapshot still records the older evidence worktree and build
identity. Refreshing it would require rebuilding and rerunning the evidence it
joins; changing the path or identity by hand would fabricate freshness. Until
that evidence run is explicitly requested and completed, read it as a dated
snapshot and use this audit only for catalogue/documentation coverage.
