# MP-00 host-test inventory and execution receipt — 2026-08-22

This closes the filename-guessed host-test gate in the isolated
`codex/master-program-foundation` implementation worktree.

## Reproduced gate defects

`gates/land-gate.sh` used to execute every extensionless executable except a
small hardcoded list. That policy disagreed with the test sources:

- it ran `parsestat` with no arguments even though `hosttest/build.sh` marks it
  **NOT A GATE**, so it always exited 2;
- it skipped the valid 32-bit `jmptest32` binary by name;
- it skipped every executable shell test through the blanket `*.*` rule,
  including the adversarial interpreter-budget and memory-map mutation gates;
- visual/benchmark instruments could exit zero and look like test passes;
- an exit-77 hardware absence was distinguishable only inside the loop, not in
  a durable receipt.

The first complete run also exposed two stale test oracles instead of hiding
them:

1. `toasttest` still assumed a 32 dp header, 64 dp dock and an 80 ms settle.
   The current design contract is 48 dp, 72 dp and a 200 ms zwin animation.
   Its covering window therefore left a measured 560 by 12 pixel title strip
   inside the toast rectangle. The test now consumes the theme title height,
   uses the current furniture dimensions and settles from `EASE_MS_WIN`.
2. `memmap-guard-test.sh` had four canaries aimed at old constants: the former
   HID/blur ordering message, the retired 256 MiB RAM ceiling, the arena's old
   8..24 MiB extent and its old literal-identity expectations. Each mutation
   now perturbs the live 14..30 MiB/1 GiB map and names the current owning
   assertion. The guard passes 20 mutation/baseline checks.

## Implementation

- `hosttest/test-policy.json` explicitly classifies every compiled target and
  executable script as a gate, hardware gate, instrument, manual hardware
  action, optional instrument or builder.
- `gen-test-inventory.py` parses all 53 `gcc -o` products from the real build
  script, scans all 7 executable shell scripts and requires exact one-to-one
  policy coverage. It emits `hosttest/test-inventory.json` with source hashes,
  exact commands, timeouts, allowed exits and evidence ceilings.
- its mutation suite proves missing compiled targets, duplicate classification,
  unknown policy kinds, unclassified scripts and foreign command overrides all
  fail nonzero.
- `run-host-tests.py` executes only automatic gates, hashes every executable,
  records command/output/elapsed identities and writes
  `hosttest/test-run-receipt.json`. Exit 77 is `skipped-hardware`, never pass.
  Instruments and manual hardware actions are `not-run`, never pass.
- its receipt validator mutation-proves missing targets, promoted instruments,
  exit-77-as-pass, hidden failures and duplicate identities.
- the landing gate now consumes the generated inventory and runner instead of
  inferring semantics from filenames.

## Fresh result

```text
test-inventory: PASS: 55 compiled + 7 scripts; 53 automatic commands
host-test runner: PASS: {
  "commands_executed": 53,
  "failed": 0,
  "not-run": 11,
  "passed": 48,
  "skipped-hardware": 3,
  "targets": 62,
  "unavailable": 0
}
```

The three hardware skips were `gpu_aperture`, `gpu_planes` and `gpu_ring` on
this unprivileged host run. `gpu_blt` and its negative control both executed.
The 11 non-runs are the builder, four visual/measurement instruments, three
manual Intel tools and three manual hardware recovery wrappers.

## Evidence ceiling

This is current host execution evidence. It does not promote skipped or manual
GPU paths to hardware proof, does not replace QEMU boot receipts and does not
claim that visual artifacts were reviewed merely because their generators
exist. The complete landing gate and physical ThinkPad suite remain separate.
