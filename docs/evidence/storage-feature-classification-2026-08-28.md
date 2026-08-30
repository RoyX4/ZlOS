# Current storage-feature classification

Date: 2026-08-28

This receipt maps current-build host execution into seven storage rows in the
canonical 906-feature ledger. It does not change storage code or convert host
behavior into QEMU or physical-device proof.

## Subject identity

The exact current subject is the `build_identity` field in generated
`docs/program/FEATURE-STATUS.json` and the host execution receipt; this page
does not duplicate that volatile value.

The receipt is bound to the canonical 76-target inventory and the exact host
runner. A fresh direct rerun produced output hashes identical to the receipt
for both relevant executables:

- `blocktest`: 10 assertions, 0 failures;
- `fstest`: 133 assertions, 0 failures.

## Promoted rows

Every row below moves only from `PLANNED_UNPROVED` to `PARTIAL_CURRENT`:

| Feature | Current bounded evidence | Still open |
|---|---|---|
| FS-001 block-device contract | bounded read/write/cache/flush seam and 10 host assertions | typed asynchronous requests, discard, cancellation, detach, provider breadth, physical proof |
| FS-005 block cache | fixed 128-page cache, read-your-write, ordered one-block service, saturation refusal and telemetry | concurrency ownership, memory pressure, retry/reset and device loss, target fault injection |
| FS-011 zlfs core | format/mount and named create/read/write/delete/rename across 512/4096-byte geometry and a separate-process cold remount | flat 32-file/23-byte-name limits, directories, permissions, allocation depth, current target persistence |
| FS-015 file growth/truncate | checked growth relocation, neighbor preservation, overflow refusal and failed-copy rollback | explicit truncate, sparse policy, concurrent maps, efficient allocation, target proof |
| FS-020 durability API | explicit sync, bounded dirty drain and data-before-metadata ordering | controller-cache/stable-media semantics, per-file barriers, current target power-cut and physical proof |
| FS-021 journal/transaction log | dual checksummed generations, seven replacement cut points and corrupt-newest fallback | append/replay journal, multi-object transactions, broader generation policy, target power-loss proof |
| FS-024 corrupt-media admission | bad magic/version/checksum/geometry/range refusals and safe generation fallback | fuzz corpus, broader filesystem graphs, current QEMU and physical corrupt-media proof |

The ledger does not promote FS-004 asynchronous block I/O, FS-006 VFS, FS-007
path resolution, FS-012 directories or any permissions/package feature. Those
contracts do not exist merely because the current flat filesystem is useful.

## Verification

```sh
cd kernel/tests/host
./blocktest
./fstest

cd ../../..
python3 kernel/tools/generators/gen-test-inventory.py --check --selftest
python3 kernel/tools/run/run-host-tests.py --selftest
python3 tools/gen_feature_status.py --write --selftest
python3 tools/gen_feature_status.py --check --selftest
python3 tools/validate_master_program.py --self-test
tools/doc-check.sh
```

Exact current maturity counts remain in generated `program/FEATURE-STATUS.json`.
The host storage gates were rerun under
the subject identity above. No current QEMU power-cut or physical storage gate
is claimed.
