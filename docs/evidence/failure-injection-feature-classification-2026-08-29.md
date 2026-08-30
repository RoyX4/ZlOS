# Failure-injection feature classification - 2026-08-29

This receipt classifies EV-019 against the canonical 906-feature ledger. It
binds current bounded rollback evidence without calling any broad failure
family exhaustive.

## Result

EV-019 is `PARTIAL_CURRENT`. The exact current subject is the `build_identity`
field in generated `docs/program/FEATURE-STATUS.json` and the adversarial
registry; this page does not duplicate that volatile value.

The adversarial registry binds two current exhaustive-within-bounds sweeps to
their implementation, harness, executable, output and receipt hashes.

## Heap allocation sweep

`HT-030 heaptest` forces one refusal at each of the 32 positions in a fixed
allocation transaction. Every iteration proves:

- only the selected allocation is refused;
- the failed call changes neither used-byte nor live-block accounting;
- every later allocation succeeds after the one-shot refusal;
- all surviving payloads remain intact; and
- releasing successful allocations returns the heap to zero use with a valid
  invariant.

The full heap target now records 2,162 checks and zero failures. It still
retains the 4,000-operation payload/invariant stress, measured constant step
bounds, tag preservation and freed-block poison probe.

## Page-table transaction sweep

`HT-051 pagetxntest` forces failure at every one of 512 page-table writes and
requires exact restoration of the original table. Separate cases prove apply
flush failure, post-flush corruption detection and a fatal rollback-flush
failure. The current native UEFI64 receipt joins that host result to committed
heap-window and framebuffer cache transactions in QEMU.

## Evidence ceiling

These are exhaustive sweeps only inside two bounded transactions. They do not
cover every allocation caller, queue, I/O path, provider admission, service
lifecycle, package transaction or application lifecycle seam. Provider,
service and package injectors remain absent; the existing queue, I/O and
lifecycle evidence remains partial. No physical-hardware failure-injection
receipt exists.

EV-019 therefore advances from `PARTIAL_HISTORICAL` to `PARTIAL_CURRENT`, while
all seven broad failure families remain open for release-complete proof.
