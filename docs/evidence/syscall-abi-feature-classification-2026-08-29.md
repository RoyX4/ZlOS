# Current syscall ABI feature classification

Date: 2026-08-30

This receipt records the KR-036 generated syscall-number repair. It promotes a
bounded x86-64 admission contract, not a complete typed or cross-version ABI.

## Regression and repair

The previous dispatcher hardcoded constants 1 through 21 in `usermode.c` and
returned `-1` for every unknown value. A Ring 3 regression image was added
first. At that repair boundary it issued unsigned syscall IDs 0, 22, `2^63` and `2^64-1`, required
`-ENOSYS`, and made the native UEFI gate fail while every other process check
remained green.

The repair adds `user_syscalls.json` as ABI version 1 authority and
`gen-user-syscalls.py` as the deterministic header generator. The generator
requires unique, ordered, positive IDs below the sign bit, rejects an
undeclared gap. The current contract extends contiguously through 24 for
anonymous reserve/commit/release and preserves zero as invalid; the first-gap
target probe is now 25.
`usermode.c` now admits numbers through the generated unsigned switch before
dispatch and returns `-ENOSYS` outside the set.

## Current evidence

The exact current shipped identity is the `build_identity` field in generated
`docs/program/FEATURE-STATUS.json` and the bound receipt; this page does not
duplicate that volatile value.

- `user-syscalls-test.sh` passes 34 host checks: ABI/version bounds, the three
  exact anonymous-memory numbers, all 24
  admitted IDs, zero, first gap, sign bit and all bits set.
- Generator mutations reject a duplicate, undeclared gap, sign-bit ID, syscall
  zero, malformed constant and unadmitted ABI version.
- The rebuilt native UEFI64 artifact runs 0, 25, the sign bit and all bits set from Ring 3 and
  observes `-ENOSYS` for all four.
- The complete host receipt passes 79 targets with 3 hardware skips, 12
  declared non-runs and no failures.
- Two independent full recipe runs produce byte-identical copies of all nine
  shipped artifacts; all six retained QEMU boot routes pass on this identity.

## Classification

KR-036 moves from `PLANNED_UNPROVED` to `PARTIAL_CURRENT`. ABI version 1 now has
generated number admission and explicit unsigned unknown behavior. It remains
partial because dispatch bodies and argument types are hand-written, there is
no generated argument/layout/error/handle manifest, no compatibility
translation layer, and no current physical-hardware receipt.

Exact current maturity counts remain in generated `program/FEATURE-STATUS.json`.

## Verification

```sh
python3 kernel/tools/generators/gen-user-syscalls.py --check --selftest
kernel/tests/host/user-syscalls-test.sh
python3 kernel/tools/run/run-host-tests.py --run --selftest
ZLOS_SKIP_BUILD=1 kernel/tools/checks/verify-efi.sh
python3 kernel/tools/checks/check-reproducible-build.py --check --selftest
python3 tools/gen_feature_status.py --check --selftest
```
