# Allocator feature join repair

Date: 2026-09-05

The [last hosted full closure](https://github.com/RoyX4/ZlOS/actions/runs/33837793854)
failed both feature-status steps because the allocator validator searched its
seven limitations for the word `consumer`. The receipt writer now describes
the same limit as production accounting for the fixed two-slot diagnostic.
The validator rejected that valid declaration. Conversely, substituting
`consumer and physical proof are complete` preserved the count and keywords
and passed the old test.

`tools/gen_feature_status.py` now compares all limitations against the literal
`KNOWN_GAPS` tuple in the receipt's already hash-verified writer. It reads that
declaration with Python's AST parser; it never executes an evidence-directory
script. Existing artifact, implementation, owner-accounting, reclamation and
host-test checks remain in force.

## Reproduction and focused proof

Run `python3 tools/test_feature_status_pmm.py` from the repository root.
Before the repair, the valid receipt failed and all seven same-count false
completion substitutions escaped. Afterward, all six test methods pass,
including per-limitation removal/substitution, malformed gap data, writer hash
drift, and proof that writer code is not executed. The fixtures contain
synthetic evidence bytes; they make no boot or allocator execution claim.

The hosted full-closure workflow runs this regression before the complete
landing gate. Local validation also parsed the changed Python and workflow YAML
and checked whitespace. The full hosted result must be recorded separately;
this focused repair alone does not refresh the 906-feature maturity ledger or
prove physical hardware.

## Continuation custody

This repair resumes PR #12 at `4c5061e19c5c1cde36ec129264b523d56f977610`.
The 17 pre-existing dirty files in the scheduler worktree were saved under
`/home/roy/Documents/artifacts/zl-linux/scheduler-continuation-2026-09-05/`
and excluded from this change. Claude's active `fable/whole-tree-sweep` worktree
remains independently owned. Expensive verification belongs on the hosted
runner while that local work is active.
