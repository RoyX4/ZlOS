# Hosted user-process gate

Date: 2026-09-03

## Subject

- Repository: `RoyX4/ZlOS`
- Pull request: [#12](https://github.com/RoyX4/ZlOS/pull/12)
- Branch head: `1d8e8dd20e578801ab9f387d35c41b067f4811ec`
- GitHub Actions run:
  [33712768713](https://github.com/RoyX4/ZlOS/actions/runs/33712768713)
- Runner boundary: GitHub-hosted Ubuntu 24.04, QEMU TCG. This run used no local
  laptop build and makes no physical ThinkPad execution claim.

## Result

The run completed green. Its boot matrix independently passed BIOS32, the raw
bootloader, GRUB BIOS/UEFI ISO, and native UEFI64. The EFI job generated current
host evidence first: 84 registered targets, 74 commands executed, 68 passed,
zero failed, four hardware skips, and 12 explicit non-runs. The new
`processlifecycletest`, `schedulerpolicytest`, and `userprocessservicetest`
targets all passed.

The native UEFI gate then passed its scheduler and user-process receipt
validators and ended with `EFI gate green`. Its boot transcript included the
bounded persistent-service marker `ST12`.

The additional command probe used a snapshot layer over the exact boot image,
created `/system/user.bin` through Files and the disk-backed editor, confirmed
its four bytes through the graphical terminal, admitted PID 1000 with
`userexec`, contained its intentional invalid-opcode fault, observed it with
`userps`, reaped slot 1, and observed an empty process table. All six receipt
assertions passed.

The exact downloaded JSON receipt is
[`kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json`](../../kernel/docs/receipts/user-process-command-native-uefi64-qemu-2026-09-03.json).
Its SHA-256 is
`19bbffd91a65f8958096ff34e54ba2a1e5de508eaf2c02ac3f734f0bb0d36f6d`.
The receipt binds build identity
`7778a28751d33b29d1a0f70e090b51cc0e702087ab1789ca6ad253aed5405bf8`,
boot-image SHA-256
`652de8b150dfa69911fb6c35d7bdad926d7a68333881eff0eb8bc609686d45e5`,
the exact probe, seven implementation files, and serial transcript SHA-256
`fca6a745e3d355bb1835ba05fe0e1b46d4b73b72ffd5085cc23d958ed34e1444`.

## Evidence ceiling

This is remote host/build/QEMU evidence. It proves the bounded two-slot service
and the external-file desktop command route on one native UEFI64 virtual
machine. It does not prove a general userspace process-management API, a
successful application workload, per-CPU scheduling or migration, concurrent
teardown/PID reuse, physical input, or physical ThinkPad behavior.
