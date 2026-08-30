# Current-build host-test receipt

Date: 2026-08-28

This receipt closes the stale-host-test gap in the 906-feature status join. It
does not promote host execution to QEMU or physical-hardware proof.

## Problem

`kernel/tests/host/test-run-receipt.json` named a Git head and exact executable
hashes, but it did not name the route-neutral zlOS build-input identity or the
runner which interpreted every exit. The joined evidence registry therefore
hard-coded the receipt as historical and kept
`current_host_test_receipt_missing` open even after a fresh passing run.

## Repair

- `kernel/tools/run/run-host-tests.py` now records the current
  `kernel/metadata/build-identity.json` identity and its own SHA-256.
- Receipt validation rejects a foreign build identity, foreign runner, missing
  or duplicate target, promoted instrument, hidden failure, and exit 77
  mislabeled as a pass.
- `kernel/tools/generators/gen-evidence-registry.py` joins the host receipt as a
  ninth current-build evidence binding and derives the host-test gap from the
  receipt identity instead of hard-coding it open.
- `tools/gen_feature_status.py` promotes only EV-012, test inventory parity, to
  `PROVED_CURRENT`. No product, QEMU, or hardware feature is promoted by this
  receipt.

## Execution

The first fresh host execution was red: 58 targets passed and `killtest.sh`
failed because the isolated worktree did not yet contain the root `interp`
binary. That result was retained as a real dependency failure rather than
discarded as noise.

After `./build.sh` built the root language tools, the initial repair used this
route-neutral identity:

```text
54162c053eab3b13b1731f1f955ef293700b6a95944c9d89438a47e450665a3d
```

The complete inventory then reported:

| Measure | Result |
|---|---:|
| classified targets | 74 |
| commands executed | 64 |
| passed | 59 |
| failed | 0 |
| hardware skips | 3 |
| explicit manual/instrument non-runs | 12 |
| unavailable targets | 0 |

On 2026-08-29 the RTC validation repair changed the current build identity to
`350905033cf7650d194c6923608486c6301a19d2c181515c1dfd6b8ce1087e65`.
The complete host suite was rebuilt and rerun with the same 59 pass, 3 hardware
skip and 12 explicit non-run result, and the receipt now binds that identity.
The later generated-syscall repair produced build identity
`e2b05fdbdd2e230102ed2c9a1d6fdf88dcb99b4067f5f99a3f74c70f1e93e03c`.
Its new `user-syscalls-test.sh` gate raises the current inventory to 75 targets
and 65 automatic commands. The current receipt records 60 passes, 3 hardware
skips, 12 explicit non-runs and 0 failures.

The later KR-002 heap tranche produced build identity
`e3bc86589ad1c6172115f3735114c6a467fa4d28d3196d1edaa28351a28b8c48`.
The same 75-target inventory was rebuilt and rerun. It retains 60 passes, 3
hardware skips, 12 explicit non-runs and 0 failures; `HT-030 heaptest` now
records 2,162 assertions including tags, a 32-position failure sweep and a
bounded freed-block poison guard.

The KR-004 page-table tranche produced build identity
`cd1d52ee4a52f94947ae52ef7d35e37c891ff46770c93ef1aa27cc54d8230176`.
Its `pagetxntest` gate raises the current inventory to 76 targets and 66
automatic commands. The current receipt records 61 passes, 3 hardware skips,
12 explicit non-runs and 0 failures. The new target checks all 512 nth-write
rollback positions and the flush, readback, explicit rollback and commit paths.

Extending the same transaction to live framebuffer cache retyping produced the
then-current build identity
`b721f5e730442b99ca9a0c3d95ae73e5aef7cd3cffc69240ed557770fd1a1602`.
The complete host receipt remains 61 passes, 3 hardware skips, 12 explicit
non-runs and 0 failures, and the native UEFI receipt now proves both the heap
window publication and framebuffer write-combining transaction in QEMU.

The later full-register crash and evidence-closure tranche produced the
then-current build identity
`23fdccde122bd1f639b36ba4563c59184be84512b879d7e1c34e7d3433e02744`.
The 76-target host receipt still records 61 passes, 3 hardware skips, 12
explicit non-runs and 0 failures. Exact current identity is read from the
generated receipt rather than this historical sequence.

Exact current maturity counts remain in generated `program/FEATURE-STATUS.json`.

## CI runner binding

PR #8 exposed a missing consumer step rather than an EFI boot defect. The
native-UEFI64 job reached every required guest marker, then correctly rejected
`kernel/tests/host/test-run-receipt.json` as foreign. The checked-in receipt was
produced with Kali's toolchain, while `build-identity.json` includes the exact
compiler binary and tool versions and GitHub Actions rebuilt with Ubuntu's
toolchain.

The EFI workflow now regenerates the build identity, builds the full host-test
inventory, writes a current-runner inventory, and executes the receipt runner
before booting QEMU. The generated CI receipt is intentionally ephemeral: it
exists to make the host/QEMU join truthful for that runner, while the checked-in
receipt remains the durable evidence from its named host and toolchain.

## Commands

```sh
./build.sh
(cd kernel/tests/host && ./build.sh)
python3 kernel/tools/generators/gen-build-identity.py --write --selftest
python3 kernel/tools/generators/gen-test-inventory.py --write --selftest
python3 kernel/tools/run/run-host-tests.py --run --selftest
python3 kernel/tools/generators/gen-evidence-registry.py --write --selftest
python3 kernel/tools/generators/gen-evidence-registry.py --check --selftest
python3 tools/gen_feature_status.py --write --selftest
python3 tools/gen_feature_status.py --check --selftest
```

## Evidence ceiling

This is current build-input-bound host execution. The receipt identifies the
inventory, build script through that inventory, runner, executable bytes,
commands, outputs, exits, and elapsed times. It does not yet provide per-object
host-test provenance or a complete transitive include graph. The three real-GPU
targets remain hardware skips, twelve manual/instrument targets remain explicit
non-runs, and no QEMU or physical boot was performed for this repair.
