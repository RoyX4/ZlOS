# MP-00 kernel toolchain and ABI manifest

## Outcome

[`../toolchain-manifest.json`](../toolchain-manifest.json) binds the exact
current compiler/linker contract rather than treating a version string as a
toolchain:

- 7 byte-identified tools: the zl compiler, GCC, Clang, GNU ld, `lld-link`,
  objcopy and NASM;
- 4 target lanes: BIOS/Multiboot ILP32, Multiboot LP64, native UEFI LLP64 and
  raw-BIOS ILP32;
- compiler flags extracted from the active build scripts;
- linker format, entry and reproducibility policy for each lane;
- macro-probed pointer, long, wchar, hosted and architecture widths;
- all 82 external compiler/sysroot headers reached by the SIMD framebuffer and
  reproducible-media helper preprocess closures, with bytes and SHA-256;
- a fail-closed environment contract for header/library/compiler injection.

This remains `PARTIAL_CURRENT`. It is exact for the current machine but is not
a hermetic portable toolchain, does not archive compiler/sysroot source
packages, and has no publisher-signed attestation.

## Bugs found while creating it

The older dependency lock claimed every current build/boot command but omitted
both Clang and `lld-link`, even though native UEFI requires them. They are now
two of 15 locked commands.

The first correction then resolved `/usr/bin/lld-link` to
`/usr/lib/llvm-21/bin/lld` before asking its version. LLD is a multi-call binary:
invoking the resolved name changed its personality and returned only “generic
driver.” The lock now records both invoked and resolved paths, hashes the
resolved bytes, and performs version discovery through the invoked alias. The
current result is Debian LLD 21.1.8.

## Four measured ABI lanes

| Lane | Object/image | ABI facts |
|---|---|---|
| BIOS/Multiboot32 | ELF32 i386 | `__i386__`, pointer 4, long 4, freestanding |
| Multiboot64 | ELF64 x86-64 | pointer 8, long 8, no red zone, large code model |
| Native UEFI64 | PE32+ EFI | `_WIN64`, pointer 8, long 4, wchar 2, no red zone |
| Raw BIOS32 | ELF32 then flat binary | pointer 4, long 4, 512-byte NASM boot sector |

Every lane requires freestanding mode, stack-protector removal, warning
visibility and `-Werror`. The IDT/APIC compilation rule requiring
`-mgeneral-regs-only` is also checked in all four scripts. Native UEFI separately
requires `/Brepro`, `/lldignoreenv`, no default libraries and the exact
`efi_main` entry.

## Environment admission

Generation fails when `CPATH`, `C_INCLUDE_PATH`, `CPLUS_INCLUDE_PATH`,
`LIBRARY_PATH`, `GCC_EXEC_PREFIX`, `COMPILER_PATH` or `LD_PRELOAD` is nonempty.
It also refuses a `CC` override other than the current literal `gcc`. Command
resolution must agree with `dependency-lock.json`.

This prevents a green manifest beside a compiler/header path silently supplied
by the caller. It does not make the machine hermetic.

## Rejection proof

```sh
python3 kernel/tools/generators/gen-dependency-lock.py --check --selftest
python3 kernel/tools/generators/gen-toolchain-manifest.py --check --selftest
```

The dependency lock rejects a missing command, binary drift, lost invocation
alias, missing firmware identity and missing build identity. The toolchain
manifest rejects a missing target lane, LP64 substituted for UEFI LLP64, a
removed warning policy, changed external-header hash and injected environment.

The landing gate regenerates and checks this manifest before any kernel build,
then the joined evidence registry preserves the missing hermetic build, signed
attestation and toolchain-source archive as open blockers.
