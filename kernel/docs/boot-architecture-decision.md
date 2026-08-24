# ADR: zlOS owns the complete boot path without turning the loader into a second kernel

**Status:** Accepted
**Date:** 2026-08-22
**Decider:** Zac (RoyX4)

## Context

zlOS currently has three independent boot evidence lanes:

1. GRUB/Multiboot ISO for broad BIOS/UEFI compatibility and regression.
2. The zlOS-owned 512-byte legacy BIOS loader in `raw_boot.asm`.
3. The primary physical-laptop path: zlOS-owned UEFI stage zero
   `EFI/BOOT/BOOTX64.EFI`, which records a witness and chainloads
   `EFI/ZLOS/ZLOS.EFI` before the kernel exits firmware boot services.

The 33-repository audit found useful additional mechanisms, especially Hyper's
filesystem/media/partition matrix, exact-origin semantics, structured handover,
network acquisition and assertion-kernel tests. The product direction is to own
and deepen zlOS's boot path, not replace it with a donor loader.

The phrase “add every bootloader feature” is too broad. A loader that retains
permanent hardware drivers, filesystems, networking, update policy, UI policy
and recovery policy after boot is a second kernel with weaker protections.

## Decision

Keep all three lanes. Treat native UEFI as the primary current physical path,
the raw loader as the minimal ownership/control path, and GRUB/Multiboot as a
compatibility and independent regression oracle.

Build a zlOS-owned boot manager/loader architecture that eventually provides
the useful superset of the audited boot systems, but only for four bounded jobs:

1. **Discover/acquire** an explicitly selected kernel, modules and configuration
   from supported local media or an optional network source.
2. **Authenticate/validate** every input, version, range, format, dependency,
   signature/digest and rollback policy before mutation or execution.
3. **Prepare/preserve** facts that disappear at handoff: exact boot origin,
   memory map, framebuffer, firmware tables, entropy, measurements and failure
   records.
4. **Transfer** through one versioned, length-delimited, architecture-neutral
   handover, then terminate firmware boot services and give permanent ownership
   to the kernel.

The boot manager may select current, previous-known-good, recovery, debug,
assertion or architecture-specific kernels. Every selected artifact must carry
an exact digest, build identity, configuration and origin. A filename or version
label alone is not identity.

Planned acquisition and proof breadth includes:

- FAT/ISO and GPT/MBR/whole-device/hybrid media where a supported scenario
  requires them;
- fragmented-file and oversized/hostile metadata tests;
- exact disk, partition, filesystem and path origin;
- modules/initramfs and bounded command/configuration data;
- optional authenticated PXE/TFTP-or-successor network acquisition;
- current, previous-known-good and recovery generations;
- secure/verified and later measured-boot receipts;
- x86-64 first, with AArch64/RISC-V handover only when their real ports exist;
- a tiny assertion kernel across firmware, architecture, medium, filesystem,
  partition, fragmentation and failure combinations.

These are destination capabilities, not claims about the current implementation.

## Component boundary

```text
platform firmware
  wakes DRAM/chipset and launches our first image
        |
zlOS stage zero
  proves entry, records bounded pre-kernel failure evidence, locates boot manager
        |
zlOS boot manager/loader
  selects -> acquires -> validates -> authenticates -> prepares typed handover
        |
zlOS kernel
  revalidates handover, retires firmware boot services, owns hardware and policy
        |
supervised services and applications
```

Permanent device management, general VFS, sockets, user accounts, application
policy, package policy, desktop policy and agent/model logic do not belong in
the loader. A temporary acquisition driver must be bounded, torn down at
handoff and replaced by the normal kernel/provider lifecycle.

## Options considered

### Keep only native UEFI

Simple modern physical path, but loses independent BIOS/raw evidence and makes
firmware behavior the only entrance oracle.

### Use GRUB/Limine permanently

Strong compatibility and mature parsing, but gives away the ownership goal and
cannot provide the exact zlOS witness/handover/evidence architecture. They
remain useful independent oracles.

### Add every possible feature to one monolithic loader

Maximum apparent breadth, but duplicates a kernel before memory protection,
increases parser/driver attack surface and makes recovery depend on the same
large component. Rejected.

### Bounded zlOS boot manager with multiple acquisition providers

Selected. It retains ownership and breadth while making the permanent boundary
explicit and independently testable.

## Writing the UEFI boundary in zl

Deferred, not rejected. Rewriting `efi_stage0.c`/`efi.c` in zl would not expose
additional UEFI powers; C and zl can call the same firmware protocols. Its value
is architectural:

- one self-hosted language/toolchain through more of the trusted path;
- generated and checked firmware ABI layouts;
- shared validation/error/evidence types;
- less C/zl semantic duplication;
- stronger proof of zl as a systems language.

It becomes useful after zl has the Microsoft x64 ABI, PE/COFF sections and
relocations, packed/aligned layouts, exact pointer/function types, UTF-16
boundaries and freestanding firmware tests. Until then, the small C boundary is
lower risk.

## Consequences

- Native UEFI stays the ThinkPad's primary path.
- GRUB and raw boot are not deleted merely because native UEFI is preferred.
- Hyper contributes requirements and tests, not source code or whole-system
  architecture.
- “More control” means exact selection, validation, identity, evidence and
  handover—not keeping firmware or loader code alive indefinitely.
- Every new loader parser/provider needs strict input lengths, resource/deadline
  limits, deterministic failure, rollback/teardown and a mutation-tested gate.
- Recovery must not share every failure mode with the primary image.
- Current/proposed/evidence levels remain explicit in all boot documents.

## Initial action order

1. Preserve and identify all three existing lanes.
2. Define the versioned typed handover and exact-origin identity.
3. Add an assertion-kernel matrix before broadening loader features.
4. Add file/media selectors and multiple identified kernel generations.
5. Add verified recovery and rollback.
6. Add optional network acquisition only behind authentication and bounds.
7. Revisit a zl-written UEFI boundary after compiler/ABI prerequisites land.

## Implementation status

The first typed-handover slice is now implemented. See
[`typed-boot-handover.md`](typed-boot-handover.md) for the exact carried facts,
fail-closed entry gates, current evidence ceiling and the recovery/identity
work that remains. This does not change the accepted acquisition architecture
or promote exact-origin/authentication claims prematurely.
