# MP-00 decision, reversal and deprecation ledger — 2026-08-24

## Result

[`../decision-ledger.json`](../decision-ledger.json) is the first generated
EV-026 decision ledger. It contains 19 dated, attributable records and indexes
all 47 labels in the older [`DECISIONS.md`](DECISIONS.md). Five legacy labels
currently have normalized semantics; 42 remain explicitly open.

This is `PARTIAL_CURRENT`. It proves that the selected records point to exact
source text and that reversals retain their replaced decision. It does not prove
that all historical decisions across the repository have been normalized, that
an independent person approved them, or that a recorded decision took effect at
runtime.

## Contract

Every normalized record has:

- a stable `DEC-####` identity and ISO date;
- an action and current status;
- affected canonical feature IDs;
- a decision and rationale;
- an exact source path, source hash and unique text needle;
- reciprocal `supersedes` and `replaced_by` links;
- retained obsolete evidence when a decision is superseded;
- an evidence ceiling separating an attributable decision from runtime proof.

The generator rejects unknown statuses/actions, invalid dates, missing records,
invented source text, non-reciprocal replacement links, replacement cycles,
superseded records without obsolete-history evidence, drift in the 47-label
legacy inventory, and any attempt to hide the open coverage gap.

## Current coverage

The 19 records capture high-impact decisions and corrections that affect the
active master-program foundation: the C/zl mechanism boundary, framebuffer
clipping reversal, snapshot-drag retirement, bounded GPU strategy, shared
SOURCES migration, browser-scope reversal, the application false-green fix,
route-specific evidence, Intel modeset and I2C-HID evidence corrections,
release licensing, source custody, dependency locking, build-graph scope,
resource-contained gates, host-only structured-event staging, and the red
status of the interrupted complete gate.

The exact open boundary remains:

- 42 legacy decision labels have identities but no normalized semantics;
- repository-wide decisions outside `docs/DECISIONS.md` are not exhaustively
  inventoried;
- no signed or independent approval exists;
- source changes do not automatically create migration records;
- no user-visible history viewer exists.

## Commands

```sh
python3 kernel/gen-decision-ledger.py --write --selftest
python3 kernel/gen-decision-ledger.py --check --selftest
```

The landing gate runs both commands before joining the ledger into the evidence
registry. The current host may run these lightweight checks; the full landing
gate remains restricted to the resource-contained launcher on an idle host.

## How to extend it

Add a record to `RECORDS` in `gen-decision-ledger.py`, bind it to one unique
source needle, preserve the old record when reversing a decision, and update
the exact legacy-coverage accounting. Never delete a superseded decision to
make the current design look inevitable, and never use a decision record as a
substitute for the feature's runtime receipt.
