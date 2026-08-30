# Performance-registry feature classification - 2026-08-29

This receipt classifies EV-021 against the canonical 906-feature ledger. It
joins current machine-checked host budgets while keeping gate runtime, product
latency and target execution as separate claims.

## Result

EV-021 is `PARTIAL_CURRENT`. The exact current subject is the `build_identity`
field in generated `docs/program/FEATURE-STATUS.json` and the bound performance
registry; this page does not duplicate that volatile value.

`performance-registry.json` binds the current benchmark receipt, 76-target host
receipt, host-build benchmark and test inventory by exact hash. The generated
registry is the numerical authority because each full gate refreshes the timing
receipts. All seven required categories are measured; their current pass/fail
counts and open regressions are read only from that registry.

## Current categories

- **Frame:** seven raw-sample p50/p95/peak distributions are measured against
  the 16.667 ms host guard. Exact values and current pass/fail state remain in
  the bound benchmark receipt.
- **Input:** the current shipping input logic gate is below its host runaway
  budget.
- **I/O:** current block and zlfs gates are jointly below their host runaway
  budget.
- **Network:** current network and TCP gates are jointly below their host
  runaway budget.
- **Launch:** the current executable-loader gate is below its host runaway
  budget.
- **Memory:** the 2,162-check heap gate is below its host runaway budget.
- **Build:** seven full host-test-suite rebuilds preserve all 64 current
  executable hashes and are measured against the declared 60-second host guard.
  Each sample has a separate 600-second runaway ceiling so an over-budget result
  is retained as evidence instead of being discarded by the old 180-second
  cutoff. Exact values and current pass/fail state remain in the bound receipt.

The generator recomputes every category from the bound receipts. Its self-test
rejects a missing category, hidden product-build gap, hidden regression, changed frame
summary, foreign build identity, invented product latency and invented target
proof.

## Evidence ceiling

Frame and host-test build timing have seven-sample distributions. The other five
categories are single current host-gate process runtimes intended to catch a
gross regression or runaway; they are not input-to-present, I/O operation,
packet, application-launch or allocator product latency.

The build distribution covers 67 host-test compilations, not the product
kernel, images or all nine artifacts. The latest contained full-gate rerun
confirmed that it remains over budget after the quiet-host admission guard.
No cause is claimed from this timing receipt alone.

There are zero native-target and zero physical categories. EV-021 therefore
advances from `PARTIAL_HISTORICAL` to `PARTIAL_CURRENT`, but the red host-build category,
product-build timing, contained-target distributions, backend separation and
identified physical performance remain open.
