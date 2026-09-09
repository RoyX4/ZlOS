# Roadmap coverage and validation

Status: planning validation, 2026-09-08. No new product implementation or runtime-test evidence is claimed.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Structured plan](FULL-SYSTEM-ROADMAP.json)

## Measured coverage

| Inventory | Retained count |
|---|---:|
| features | 906 |
| contracts | 174 |
| targets | 611 |
| packages | 1,691 |
| tasks | 11,528 |
| phases | 21 |
| decisions | 25 |
| near term process steps | 32 |

The 1,691 packages are 906 feature requirements, 174 original contracts and 611 target requirements. They overlap in implementation. The 11,528 entries include shared lifecycle checklists, source-derived acceptance obligations, authored subsystem milestones, decisions and the detailed next-process sequence. **They are not 11,528 independently designed features, an effort estimate or a progress percentage.** The original requirements remain attached so repeated scaffolding cannot replace specific behavior.

All 25 feature domains are represented. All 63 current named implementations, including the 24 games, and the All Applications surface remain covered by the original application inventory and 64 current/game target rows. Registry identifiers were taken from actual rows; missing numerical suffixes were not invented.

## Checks performed

The planning validator compared exact feature/contract/target ID sets against the canonical sources; rejected duplicate task/package IDs and missing source files; required action, acceptance, owner scope and PLANNED state; checked every explicit prerequisite; checked all nine source-file hashes; and topologically ordered all 11,528 steps without a cycle.

It also rejected deliberately corrupted variants:

- `missing-feature`
- `missing-contract`
- `missing-target`
- `duplicate-step`
- `unknown-prerequisite`
- `dependency-cycle`
- `false-completion`
- `source-drift`

These checks prove structural coverage and the stated explicit graph. They do not prove narrative dependencies are complete, external standards have been researched, the architecture is correct, the programme is implemented, or hardware works. Each package has a mandatory source-dependency and design review before implementation.

The original source bodies for all 174 detailed contracts are preserved, including the five contracts with plain prose instead of standardized fields. Their document-level global rules remain linked from the master guide. The review sampled generated entries across all 25 feature domains and all target families and checked exact original text retention; it did not independently redesign all 906 features.

Authoring and validation inputs are retained locally at `/home/roy/Documents/artifacts/zl-linux/full-roadmap-2026-09-08/`: `source-inventory.json`, `roadmap_spec.py`, `build_roadmap.py`, `finish_roadmap.py` and `validation.json`. The builder is an authoring utility which rewrites its generated draft outputs. It is not an implementation gate or a safe way to preserve later manual progress edits. Use the standalone read-only final check retained there for subsequent validation.

## Pinned source inputs

| Source | SHA-256 |
|---|---|
| [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md) | `d549b87987531e449eb3e76c4f87e3d81ef979bd51df6c26b29b13475d9208ee` |
| [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md) | `c88d5495306c35796bac7718fc7712ef9194f234307b00e0348b95a4677efe77` |
| [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md) | `46b08ee1b353e4c55727f9d16e58f6a3cc15c12da808262b88dc21a08b4fac71` |
| [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md) | `af72e2c1fcc260d1930a58527c3b49ca38ea87777294a276c1e2eb0afb41e896` |
| [docs/program/PHASES.md](../../docs/program/PHASES.md) | `1f4c7176a7a34165542821bcee5231208328bf67b1fd2a8c4c62c33fa5497d98` |
| [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md) | `4d1882bfaecdebcb88c493ee32d2530b0c55ce92c9aeb6c4c3c429195a7ecd30` |
| [docs/program/SERVICES.md](../../docs/program/SERVICES.md) | `6bea15e0101ef3addcf3dd44e424abb34bf7f736c2de8e808da458ff4345f197` |
| [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md) | `4ae4a661ae9d08a421c2bce0fa84b4342cf694da55a8e0a7f51fda9a913f162a` |
| [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md) | `1b3a8433570b3ea460cfc5dc36daed886dcd54eb22cfa71804eb5f0c59e4e9ad` |

Baseline feature-evidence JSON SHA-256: `7a5ca1d0bfbffa75f15654183c81f6b5d87945009c259d59b106e729b8ae4d15` from the retained full-run artifact, run `34014513857`. This is separate from older checked-in generated status snapshots.

## Document and source-scope verification

Final read-only checks are recorded in the retained `final-validation.json` artifact. They check actual saved documents, package coverage, original-text retention, links and source ownership, alongside the repository's existing programme and directory-documentation checks. Runtime code is not changed by this planning pass. Commit and push are outside this pass.
