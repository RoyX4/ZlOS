# MP-00 release notes and changelog — 2026-08-24

## Result

[`../release-notes.json`](../release-notes.json) and the generated
[`UNRELEASED-CHANGELOG.md`](UNRELEASED-CHANGELOG.md) implement the first EV-027
release-note contract. They record 17 current change candidates, six with
potential user-visible impact, three migrations, 12 current defects/blockers,
three recovery paths, and two retained superseded decisions. Published entries
remain exactly zero.

This is `PARTIAL_CURRENT`, not a release. There is no version, release date,
channel, signature, public redistribution authority, complete migration
inventory or previous signed generation to roll back to.

## Dependency boundary

The generator reads lower-level authorities directly:

- exact build identity, artifacts, boot routes and application evidence;
- source snapshot, toolchain, build graph and license state;
- decision/reversal history;
- host-test, benchmark, visual, accessibility, security and observability
  registries;
- the source-recovery and contained-gate operator scripts.

It deliberately does **not** read `evidence-registry.json`. The joined evidence
registry reads the finished release notes instead. This one-way dependency
prevents a release note from becoming evidence for itself.

## Contract

Every current change candidate carries its decision ID, date, type, canonical
feature IDs, audience, user-visible classification, rationale, compatibility
statement and exact decision-ledger identity. Superseded decisions stay in a
separate history instead of disappearing.

The unreleased batch also contains:

- exact artifact, route, ABI-lane and application compatibility facts;
- build-route, compositor and host-gate migration instructions;
- known issues sourced from current machine registries rather than prose;
- source-transaction and contained-gate recovery paths;
- an explicit unavailable previous-release rollback path.

The generated Markdown begins with an unreleased warning and is never edited by
hand. The JSON is the machine authority; the Markdown is its human view.

## Rejection tests

The self-test proves that validation rejects:

- a missing current change;
- an invented released state;
- a hidden public-release block;
- a deleted known issue;
- a migration without recovery;
- erased superseded history;
- a hidden historical-series gap.

The master 906-row join additionally rejects any release-note feature ID that
does not exist in the canonical catalogue.

## Commands

```sh
python3 kernel/tools/generators/gen-release-notes.py --write --selftest
python3 kernel/tools/generators/gen-release-notes.py --check --selftest
```

The full landing gate runs both only after all lower-level inputs have been
refreshed and immediately before the final evidence join. Running these host-
light checks does not upgrade the still-unrun complete contained gate.

## Open completion gates

EV-027 cannot become complete until a real release generation has an admitted
version and channel, signed notes, redistribution authority, exhaustive user-
data migrations, a previous-generation rollback artifact, publication proof
and user-visible entries derived from the exact released image rather than an
uncommitted development worktree.
