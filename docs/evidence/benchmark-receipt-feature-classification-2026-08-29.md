# Benchmark-receipt feature classification - 2026-08-29

This receipt classifies EV-014 against the canonical 906-feature ledger. It
records a current host benchmark distribution without promoting host RAM or
host compositor execution to QEMU or native zlOS performance.

## Result

EV-014 is `PARTIAL_CURRENT`. The exact current subject is the `build_identity`
field in generated `docs/program/FEATURE-STATUS.json` and the bound benchmark
receipt; this page does not duplicate that volatile value.

`benchmark-host-2026-08-23.json` identifies the host, kernel, CPU, logical CPU
count and starting load. It binds exact hashes for `wmbench`, `fbbench`, both
source files and the benchmark runner.

## Current proof

The receipt retains seven independent process samples for each registered
metric. It records p50, nearest-rank p95 and peak values and uses p95 for the
60 Hz budget decision.

The receipt measures seven host metrics against the 16,667 microsecond frame
budget:

- one shipping-compositor drag frame at 1920x1200;
- the shipping serial framebuffer desktop frame at 1920x1200, 2560x1440 and
  3840x2160; and
- all three four-band host experiments, explicitly labelled as
  non-native routes.

The receipt's `within_budget`, p50, p95 and peak fields are the current numerical
authority. A later gate may legitimately change those host timings without
making this classification page stale. They remain host guard results, not
target performance proof.

The self-test rejects a hidden regression, target overclaim, distribution
summary drift, missing executable identity and raw-output drift. The evidence
registry separately rejects a foreign build identity or changed receipt hash.

## Evidence ceiling

The measurements came from one contended Linux desktop. `fbbench` presents to
ordinary host RAM, not PCIe write-combining VRAM, and the four-band cases are
experiments rather than a shipped zlOS worker route. There are zero native
target measurements and no QEMU or physical performance distribution.

EV-014 therefore advances from `PARTIAL_HISTORICAL` to `PARTIAL_CURRENT` but is
not a complete performance qualification. EV-021 still owns the broader
cross-subsystem regression registry, and physical release promotion remains
gated on identified hardware distributions.
