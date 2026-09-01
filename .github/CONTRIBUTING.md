# Contributing to zlOS

zlOS accepts focused fixes, tests, documentation corrections, and bounded
feature work. The project is experimental systems software: a change is not
complete merely because the source exists or one build succeeds.

## Before changing anything

1. Read [`docs/PROJECT-STATUS.md`](../docs/PROJECT-STATUS.md) for current truth.
2. Read [`docs/CODE-MAP.md`](../docs/CODE-MAP.md) for ownership.
3. Read the nearest directory `README.md`, `STATUS.md`, `RULES.md`, and
   `SOP.md` before editing that area.
4. Check the working tree and preserve unrelated work.
5. Keep changes small enough that their evidence boundary is obvious.

## Evidence is part of the change

Report exactly which lanes were exercised:

- source inspection;
- host tests;
- 32-bit, 64-bit, or EFI build;
- QEMU BIOS, GRUB, or native UEFI boot;
- physical hardware;
- release/publication.

A stronger lane does not happen automatically. QEMU is not ThinkPad proof, a
host harness is not a booted driver, and a public file is not a verified
release.

For kernel changes, start with the smallest focused test and then use the boot
gate that actually reaches the changed route. Native UEFI work requires
`kernel/tools/checks/verify-efi.sh`.

## Pull requests

- Explain what changed and why.
- Name commands actually run and documents actually read.
- State failed, skipped, unbuilt, QEMU-only, and physical-only lanes.
- Link the owning feature/contract when the work belongs to the 906-feature
  program.
- Do not bundle generated churn, unrelated cleanup, or another contributor's
  changes into the same commit.

The pull-request template contains the hardware and EFI hazards that must be
checked before landing.

## Hardware safety

`kernel/src/drivers/display/intel.c` includes panel-power sequences that can
damage hardware when timing or power-state rules are violated. Do not "try it
and see" on a physical panel. Use the documented host harness and preserve the
UEFI GOP fallback until the exact native route is proved.

## Style

- Prefer the smallest root-cause fix.
- Keep explicit bounds and error handling at trust boundaries.
- Preserve generated-file ownership; change the source or generator instead.
- Use conventional commits: `type: concise description`.
- Do not claim a feature, driver, application, or release is complete without
  its required evidence.
