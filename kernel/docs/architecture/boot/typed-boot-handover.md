# Typed boot handover v1

**Status:** Implemented locally on all three existing entry families. The host
mutation gate and all four compile/link routes pass. Boot execution gates remain
required before this can be promoted to runtime-verified.

## What changed

Before this slice, each route carried different facts into the kernel:

- Multiboot passed a foreign structure pointer directly to the console.
- `raw_boot.asm` left eleven VBE bytes at physical address `0x8300`.
- native UEFI held GOP, ACPI and memory-map facts in private `efi.c` globals.

All three now construct the same 256-byte `zlos_boot_handover` before
`kernel.zl` starts. The record is versioned, length-delimited, zero-reserved,
checksummed and immutable after sealing. The 32-bit Multiboot, 64-bit
Multiboot and raw-BIOS assembly entries stop before `main` if validation fails;
native UEFI prints a refusal and stops after firmware exit.

The record currently carries:

- route and architecture;
- selected generation, attempt ordinal/limit and previous-failure slot;
- boot-source address;
- ACPI root address;
- memory-map address, byte length, entry shape and version;
- framebuffer address, exact byte span, geometry, depth and pixel format;
- explicit firmware-retired state;
- reserved 32-byte exact-origin and exact-kernel identities.

The identity fields are not populated by the shipping loaders yet. Their flags
remain clear, so v1 does **not** claim artifact authentication or exact disk,
partition, filesystem and path identity. A future loader may set either exact
flag only with a non-zero 32-byte identity; the validator rejects an empty
claim.

## Ownership and lifetime

The record copies numbers only. It never retains a UEFI protocol or firmware
structure pointer. Native UEFI takes the final memory map, successfully exits
boot services, marks firmware retired, seals the record and only then starts
the kernel. The map buffer itself is zlOS-owned static storage and remains
alive after firmware retirement.

Multiboot memory-map entries are explicitly marked variable-length. UEFI
entries carry their descriptor size and version. Consumers must check the
record before walking either format; v1 does not pretend those two foreign map
formats are already normalized.

## Recovery boundary

The ABI can represent current, previous, recovery, assertion and debug images,
plus a bounded attempt count and prior failure reason. The present entry paths
select `current`, attempt `1` of `3`. This is the contract needed by the next
slice; it is not yet a boot-menu or automatic rollback implementation.

The next loader slice must select and authenticate multiple images, persist an
attempt/failure record independently of the selected kernel, and populate both
identity fields before claiming exact selection.

The pure selection policy for that next slice now exists in
`src/core/boot/boot_state.c` and `src/core/boot/boot_state.h`. Its 64-byte
checksummed record observes an unready pending boot,
allows two attempts per generation, chooses current then previous then recovery,
records a ready boundary, and explicitly degrades rather than bricking a device
that contains only one image. `tests/host/boot_state_test.c` passes 91 checks.

That policy is deliberately **not wired into UEFI stage zero yet**. Stage zero
still loads only `EFI/ZLOS/ZLOS.EFI`; no firmware variable is read or written,
no previous/recovery image is selected, and the kernel does not yet publish its
ready mark back to firmware. The tested state machine is a finished dependency,
not a claim that automatic rollback is live.

## Proof

`kernel/tests/host/boot_handover_test.c` compiles the shipping implementation and
checks:

- all 256 bytes are covered by structural validation or the checksum;
- mutation after sealing is refused;
- UEFI/32-bit and attempt-over-budget claims are refused;
- unknown flags and non-zero reserved bytes are refused even after recomputing
  the checksum;
- framebuffer underflow/overflow and hostile Multiboot geometry fail closed;
- fixed-size UEFI and variable-size Multiboot memory maps stay distinct;
- raw, Multiboot and UEFI route values map to the existing ZL `loader()` codes;
- a recovery-generation record with exact origin/kernel identities can seal.

Build and run the focused host gate:

```sh
cd kernel/hosttest
gcc -O2 -g -Wall -Wextra -o boot_handover_test \
    boot_handover_test.c ../boot_handover.c
./boot_handover_test
```

Required promotion gates are the four normal builds plus raw BIOS, Multiboot32,
Multiboot64 and native UEFI boots. Hardware promotion additionally needs a new
ThinkPad journal; QEMU cannot prove the physical firmware's map, GOP placement
or boot-device identity.

Current build receipt from this isolated tree:

- Multiboot32: `kernel.elf`, 4,758,296 bytes, zero undefined symbols;
- Multiboot64: `kernel64.elf`, 2,877,832 bytes, zero undefined symbols;
- raw BIOS: 512-byte boot sector, 4,612,044-byte kernel, 1,640 KiB loader
  headroom;
- native UEFI: 20,480-byte stage zero and 2,895,872-byte kernel PE image;
- EFI witness structural gate: passed;
- typed-handover hostile host gate: 288 checks, zero failures.

These are build/host facts, not boot or hardware claims.
