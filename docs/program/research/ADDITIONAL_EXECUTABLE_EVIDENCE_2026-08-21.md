# Additional executable evidence — NexiOS and Zinnia

Date: 2026-08-21
Scope: two deliberately selected build/test claims from the 33-repository shelf
Source shelf: `/home/roy/Documents/repos/zl-starred-sources`

## Result

This pass changes evidence strength for two repositories without changing their
source-level security findings:

- **NexiOS:** configuration, style, kernel, and ISO construction were executed
  locally. The debug kernel and bootable ISO built. The advertised self-test
  path did not produce a kernel test summary under either corrected UEFI or
  BIOS-like launch settings, so no target test earned verified status.
- **Zinnia:** the x86_64 kernel and all workspace driver crates built in release
  mode with the repository-pinned Rust nightly after removing an obsolete Cargo
  `-Z` argument from the README command. This proves current compilation of the
  checked-out workspace, not boot, driver initialization, POSIX behavior, or
  the correctness of its stubbed uACPI layer.

The two worktrees and all other downloaded repository worktrees were checked
after execution. No tracked source changes remained. Ignored build artifacts
were retained as local evidence.

## Evidence labels

- `[V-config]`: a configuration/static gate returned zero locally.
- `[V-build]`: a compilation or image-construction command returned zero
  locally and produced a fresh artifact.
- `[V-run-fail]`: the target runner executed but did not produce its required
  success verdict.
- `[V-environment]`: an execution problem belongs to the local runner/tooling
  environment, not the audited target source.
- `[S]`: source inspection only.

These labels are intentionally narrower than `[V-boot]`, `[V-test]`, or
`[V-hardware]`. None of those stronger labels was earned here.

## 1. NexiOS

Repository: `staycool1374-Ger__nexios`
Commit: `624b9e5f5209e51088754a5f3672c34b1c5df880`

### Configuration gate

Executed from the repository root:

```text
make check-config
```

Result: `[V-config]` exit 0. It emitted one material warning:

```text
CONFIG_TICK_HZ=1000 does not divide PIT base frequency (1193182) evenly
— TSC calibration may be inaccurate
```

This is a passed configuration gate with a timing-calibration warning, not
evidence of calibrated real-time behavior.

### Style gate

Executed:

```text
make check-style
```

Result: `[V-config]` exit 0 after scanning 251 files and reporting 854 warnings,
0 errors. The wrapper still printed `STYLE Passed.` This means the gate blocks
only its error class; it is not a zero-warning quality gate. Any synthesis that
calls it strict must state this distinction.

### Debug kernel and ISO build

Executed as part of the repository's normal self-test target:

```text
make execute-test x86 debug selftest
```

Before the runner failure, the normal build graph produced:

| Artifact | Measured size | Evidence |
|---|---:|---|
| `build/kernel-debug.elf` | 11,180,976 bytes | `[V-build]` |
| `debug/nexios-rtos.iso` | 36,323,328 bytes | `[V-build]` |
| generated FAT image | 1,196,034 bytes | `[V-build]` |

This proves compilation, linking, image population, and ISO construction on
this host. It does not prove that the kernel reaches its initialization or test
milestones.

### Runner portability failures

The first target attempt failed before QEMU:

```text
/bin/bash: line 38: gstdbuf: command not found
```

`[S][V-environment]` The active Makefile recipe unconditionally uses the macOS
Homebrew command name `gstdbuf`. Its host detection changes size tooling and a
package hint, but not the buffering command. For audit only, a temporary PATH
entry mapped `gstdbuf` to the installed GNU `stdbuf`; the repository was not
edited.

The next attempt reached QEMU but failed while opening the default firmware:

```text
/opt/homebrew/share/qemu/edk2-x86-code.fd
```

`[S][V-environment]` This is another macOS Homebrew default on a Linux host. A
second audit-only invocation supplied the installed firmware explicitly:

```text
QEMU_UEFI=/usr/share/OVMF/OVMF_CODE_4M.fd
```

### Corrected UEFI run

With both host-only corrections, QEMU reached firmware and GRUB and printed:

```text
Booting 'Jarvis RTOS'
```

The runner then exited with:

```text
RESULT: QEMU_EXIT (no TEST SUMMARY -- kernel crashed/hung)
```

Result: `[V-run-fail]`. No kernel test lines or `TEST SUMMARY` were observed.
The harness returned nonzero, which is good fail-closed behavior for this path.

### BIOS-like run

A separate invocation removed pflash firmware from the architecture flags:

```text
QEMU_ARCH_FLAGS='-boot order=d -cpu max'
```

It also ended with:

```text
RESULT: QEMU_EXIT (no TEST SUMMARY -- kernel crashed/hung)
```

No boot or test output was observed. This second result prevents attributing
the missing summary only to the supplied OVMF file, but it does not isolate the
kernel/runtime cause.

### NexiOS conclusion

Verified now:

- configuration gate completes;
- warning-only style gate completes;
- debug kernel links;
- FAT payload and ISO are constructed;
- active runner rejects EOF without a summary.

Still unverified:

- kernel initialization after the GRUB handoff;
- any self-test execution or pass;
- scheduler deadline behavior under load;
- AArch64/RISC-V runtime behavior;
- hardware behavior.

The weakest link is the missing serial/kernel trace after GRUB. The correct next
diagnostic is a symbolized early-boot trace or QEMU debugger capture, not a
claim that the tests passed because CI names the target.

## 2. Zinnia

Repository: `zinnia-os__zinnia`
Commit: `1c10cf45381092aa3bb0388b6b63af10859a0e22`
Pinned submodule: uACPI at
`022bc38ee675da1c63e6a6567e510cf58b3d2f7e`

### Toolchain state

The repository-pinned toolchain resolved after installation/update activity:

```text
rustc +nightly-2025-12-30 --version
rustc 1.94.0-nightly (56f24e00c 2025-12-29)
```

During the first inspection, overlapping Rustup activity caused rename/rollback
errors (`Directory not empty`) while toolchains were being synchronized. That
was a local orchestration mistake, not a Zinnia defect. After the updater had
fully exited, `rustup toolchain list` and the pinned `rustc` command succeeded.
No target evidence is assigned to the failed concurrent updater attempt.

### Documented command drift

The README documents:

```text
cargo +nightly build --release -Zjson-target-spec \
  --target toolchain/x86_64-kernel.json
```

With the pinned Cargo, that failed immediately:

```text
error: unknown `-Z` flag specified: json-target-spec
```

Cargo itself advised that this `-Z` flag is unknown. The custom JSON target is
accepted without it, so the smallest audit-only correction was to remove the
obsolete argument; no repository file was changed.

### Release build

Executed:

```text
cargo +nightly-2025-12-30 build --release \
  --target toolchain/x86_64-kernel.json
```

Result: `[V-build]` exit 0 in 1 minute 29 seconds. Cargo compiled the kernel,
uACPI binding, and the connected workspace driver crates, including:

- NVMe;
- Intel `igc` networking;
- virtio common, block, network, and GPU drivers;
- xHCI;
- ext2.

The build emitted seven warnings: one unused import and six unused syscall
number constants. It produced a measured 40,897,976-byte
`target/x86_64-kernel/release/zinnia.kso`, plus driver `.kso` outputs. The whole
local `target` directory measured 649 MiB after the build.

### Zinnia conclusion

Verified now:

- the initialized uACPI submodule is usable by the host build;
- the pinned x86_64 workspace compiles in release mode;
- kernel and driver crates are genuinely connected to the workspace build;
- the README's extra Cargo `-Z` argument is stale for its pinned toolchain.

Still unverified:

- a bootable system image, because this repository explicitly delegates image
  creation to a separate bootstrap repository;
- kernel boot or test execution;
- driver probe/interrupt/DMA behavior;
- POSIX/BSD/Linux compatibility claims;
- non-x86 targets;
- uACPI runtime correctness.

The source refutation remains decisive: successful compilation cannot upgrade
uACPI hooks that return fake success, do nothing, or call `todo!()`. Likewise,
it does not validate malformed ELF admission or failure-atomic virtual-memory
range changes.

## Cross-project evidence correction

The implementation plan should keep five receipts separate:

1. **configured:** the intended subsystem is selected and its gate ran;
2. **built:** fresh object/kernel/image artifacts exist and are hashed;
3. **booted:** the target emitted a nonce-bound boot milestone;
4. **tested:** planned/executed/failed counts came from the current target run;
5. **hardware:** a named device/firmware path produced the required receipt.

NexiOS currently reaches step 2 locally; Zinnia reaches the kernel/driver part
of step 2 but not image construction. Neither reaches steps 3–5 in this pass.
No CI label or checked-in screenshot should silently fill those gaps.
