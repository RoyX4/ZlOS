<!--
Delete any section that does not apply. This template exists because every
line in the checklist below corresponds to a bug that actually shipped here.
-->

## What changed


## How it was verified

<!-- Say what you RAN and what you READ. Both count - large parts of this
     kernel cannot be booted in an emulator, so reading is legitimate
     verification. Conflating the two is what is not. -->

- ran:
- read:

## Hazard checklist

Tick what applies. CI checks the mechanical ones; these are the ones it cannot.

- [ ] **No pointer through `unsigned long` in the EFI build.** LLP64 makes it
      4 bytes there. Harmless below 4 GiB, which is why QEMU never shows it,
      and why this class has shipped twice.
- [ ] **New code has a caller.** "The code exists" is not "the code works" -
      most of `intel.c`'s write paths are gated behind `lt_armed` and have
      never executed.
- [ ] **Any new gate polls for a marker**, never a fixed wall clock. A gate
      that fails for reasons unrelated to the code costs a bisect every time.
- [ ] **No generated `.c` or binary added.** `.gitignore` does not apply to
      files already in the index.
- [ ] **`verify-efi.sh` was run** if this touches the boot path. It is the only
      check on the path real hardware takes; three gates were once green while
      the 64-bit build was dead.

## Hardware

- [ ] This does **not** touch `intel.c` panel power.

<!-- If it does: the 500 ms T12 delay and AUX-into-an-unpowered-panel can
     DAMAGE the ThinkPad, not merely fail. "Try it and see" is not available.
     Say which hazard in section 4.1 of kernel/docs/gen9-modeset-plan.txt
     applies and how you avoided it. -->

## Language semantics

- [ ] If this claims anything about how zl behaves, it was run through
      `./interp` — ground truth when the engines disagree, and they do
      (`tools/engine-parity.sh`). If it was not run, say so.
