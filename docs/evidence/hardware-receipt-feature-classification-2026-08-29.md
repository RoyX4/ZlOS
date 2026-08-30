# Hardware-receipt feature classification - 2026-08-29

This receipt classifies EV-015 against the canonical 906-feature ledger. It
records current machinery for collecting physical proof while keeping a plan,
a template and a real observed hardware run as three different things.

## Result

EV-015 is `PARTIAL_CURRENT`. The exact current subject is the `build_identity`
field in generated `docs/program/FEATURE-STATUS.json` and the hardware plan;
this page does not duplicate that volatile value.

The generated hardware plan binds all six boot routes to their exact current
boot-medium and payload hashes. Each route requires seven lifecycle scenarios:
first use, reuse, warm reboot, cold boot, reset, teardown and recovery. Each
receipt also requires ten durable raw-evidence classes, including artifact
read-back, machine and firmware inventories, device topology, boot and
interaction logs, a screen capture, storage read-back, teardown and recovery
logs.

## Fail-closed boundary

The receipt validator rejects:

- a wrong artifact, route, build identity or boot-medium read-back;
- missing firmware, CPU, memory or display/storage/input/network topology;
- a missing or failed lifecycle scenario;
- absent, stale, duplicate, path-escaping or hash-mismatched raw evidence;
- a receipt not attested as physically observed by an operator;
- an overall physical pass with zero validated receipts; and
- any receipt changed after its canonical content digest was recorded.

The artifact registry now derives `PASS_EXACT_HASH` only from the validated
receipt set. Its existing physical-overclaim mutation still proves that editing
an artifact status by hand cannot promote it.

## Current physical result

```text
matrix routes:                    6
required scenarios per route:    7
required evidence kinds:        10
validated physical receipts:     0
physically passed routes:         0
physically passed artifacts:      0
```

No drive was flashed, no machine was rebooted and no physical observation was
made in this tranche. The generated template is deliberately `NOT_RUN` and is
not stored in the physical receipt directory. EV-015 advances from
`PARTIAL_HISTORICAL` to `PARTIAL_CURRENT` because its schema, matrix, validator,
landing-gate seams and artifact-consumer path are current; it is not
`PROVED_CURRENT` because physical execution is still absent.

## Remaining proof

Run each case on identified hardware, preserve all raw files inside the
repository evidence tree, validate the completed receipt, and regenerate the
plan and artifact registry. The primary H2 target is native UEFI64 on the
ThinkPad; the BIOS, UEFI32 and GRUB cases remain secondary compatibility
targets. Signing, independent custody and release authorization are separate
gates even after physical receipts exist.
