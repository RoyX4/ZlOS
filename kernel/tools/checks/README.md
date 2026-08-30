<!-- Managed by tools/directory-docs.py. Edit only the LOCAL block. -->
<!-- BEGIN GENERATED: directory-docs -->
# kernel/tools/checks

## Purpose

Deterministic static, host, image, and boot verification entry points.

## Classification

- **Class:** `tooling`
- **State:** active tooling
- **Evidence boundary:** A tool is useful only when its invocation, failure behavior, and consumer are verified.

## Start here

- Read the [project status](../../../docs/PROJECT-STATUS.md) for current truth.
- Read the local [status](STATUS.md), [tasks](TODO.md), [rules](RULES.md),
  [values](VALUES.md), and [operating procedure](SOP.md).
- Editing agents must also read [AGENTS.md](AGENTS.md).

## Child directories

- None.

## Direct tracked contents

- `check-appids.py`
- `check-boot-prereqs.py`
- `check-build-contract.py`
- `check-dead-state-selftest.sh`
- `check-dead-state.py`
- `check-dma.sh`
- `check-elf-permissions.py`
- `check-header-mirror-selftest.sh`
- `check-header-mirror.py`
- `check-himap.sh`
- `check-land-gate.py`
- `check-memmap-mirror-selftest.sh`
- `check-memmap-mirror.py`
- `check-memmap.sh`
- `check-nativeargs-selftest.sh`
- `check-nativeargs.py`
- `check-ram.sh`
- `check-reproducible-build.py`
- `check-shell-layout.py`
- `check-version-selftest.py`
- `check-version.py`
- `check-visual-goldens.py`
- `check-zl-calls.sh`
- `check-zl-dispatch.py`
- `check-zlcalls.py`
- `dead-state-baseline.txt`
- `qemu-crash-selftest.sh`
- `qemu-crash.sh`
- `test-real-xhci.sh`
- `verify-64.sh`
- `verify-clock.sh`
- `verify-crash.py`
- `verify-dependency-archives.py`
- `verify-disk.sh`
- `verify-efi.sh`
- `verify-event-trace.py`
- `verify-iso.sh`
- `verify-net.sh`
- `verify-raw.sh`
- `verify-sources.sh`
- `wguard.sh`
- `write-clock-receipt.py`
- `write-page-table-receipt.py`
- `write-pmm-receipt.py`
- `write-scheduler-receipt.py`
- `write-user-process-receipt.py`

This inventory is local orientation, not a completion claim. See the [code map](../../../docs/CODE-MAP.md)
for repository-wide ownership.
<!-- END GENERATED: directory-docs -->

<!-- BEGIN LOCAL: directory-docs -->
`verify-crash.py` owns the current BIOS32/native-UEFI64 UD2, native-UEFI64
general-protection, and native-UEFI64 double-fault QEMU receipts. The
double-fault check independently queries the halted CPU RSP and requires both
it and the serialized handler SP to fall inside the dedicated IST1 bounds.

`verify-efi.sh` and its forced-ZLLOG-refusal subprobe expose the image through
QEMU xHCI as `usb-storage` with `removable=on`. Keep that flag: QEMU 11.0.3
repeatedly dereferenced a null request in `scsi_req_get_buf` when the removable
USB image was incorrectly modeled as a fixed disk; the corrected launch shape
survived five consecutive full verifier repetitions.
<!-- END LOCAL: directory-docs -->
