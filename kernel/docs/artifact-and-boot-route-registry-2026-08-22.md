# zlOS artifact and boot-route truth

This is the durable orientation for the files produced by the zlOS build. The
machine-readable authority is
[`../artifact-registry.json`](../artifact-registry.json), generated and checked
by [`../gen-artifact-registry.py`](../gen-artifact-registry.py). Do not infer a
runtime or hardware claim merely because a similarly named artifact passed.

## Current evidence boundary

The current build identity is generated from every shipped source, compiler,
linker and packaging input. Nine outputs have a receipt from two independent
complete recipe executions that produced byte-identical files. Six exact disk
or ISO boot routes run in QEMU. No current receipt binds any of these exact
hashes to physical hardware. That limitation is an intentional field in the
registry, not a prose exception.

| Artifact | What it is | Producer | Runtime proof |
|---|---|---|---|
| `kernel.elf` | 32-bit multiboot ELF | `build.sh` | Byte-exact member of the ISO that passed GRUB BIOS and GRUB UEFI |
| `kernel64.elf` | 64-bit freestanding ELF | `build64.sh` | Byte-exact member of `zlOS64.iso`; passed BIOS/UEFI GRUB entry, owned long-mode switch and interactive zl execution |
| `kernel_raw.elf` | 32-bit ELF linked for the raw loader | `mkdisk.sh` | Its checked `objcopy` payload is embedded in the passing raw disk |
| `BOOTX64.EFI` | 64-bit PE32+ native UEFI application | `buildefi.sh` | Byte-exact member of the passing UEFI USB disk |
| `zlOS.iso` | Hybrid GRUB ISO | `mkiso.sh` | Exact hash passed GRUB BIOS and GRUB UEFI in QEMU |
| `zlOS64.iso` | Hybrid GRUB ISO with `kernel64.elf` | `mkiso64.sh` | Exact hash passed GRUB BIOS and GRUB UEFI, then zlOS's owned long-mode switch |
| `zlOS.img` | Raw BIOS disk with zlOS's 512-byte loader | `mkdisk.sh` | Exact hash passed raw BIOS in QEMU and its shell responded |
| `zlOS-usb.img` | GPT disk, FAT ESP and native EFI app | `mkusb.sh` | Exact hash passed native 64-bit UEFI in QEMU |
| `boot-media-ids.json` | Content-derived GPT/FAT identity metadata | `gen-boot-media-ids.py` | Metadata only; reproducible, not executable |

## The six boot routes

1. `raw-bios` boots `zlOS.img` through zlOS's own 512-byte BIOS loader. It does
   not involve GRUB.
2. `native-uefi64` boots the full `zlOS-usb.img` through OVMF's removable-media
   path. Firmware loads `/EFI/BOOT/BOOTX64.EFI`; zlOS is 64-bit from entry and
   does not involve GRUB or a separate zlOS bootloader.
3. `grub-bios32` boots `zlOS.iso` through legacy BIOS and GRUB, which hands the
   32-bit multiboot kernel its framebuffer and memory information.
4. `grub-uefi32` boots the same ISO through OVMF and GRUB. Firmware and GRUB are
   64-bit, but the handed-off zlOS multiboot kernel is 32-bit.
5. `grub-bios64` boots `zlOS64.iso` through BIOS and GRUB. GRUB enters the
   Multiboot payload in 32-bit protected mode; `boot64.S` owns page-table setup,
   PAE/EFER/paging and the far jump into 64-bit zlOS.
6. `grub-uefi64` boots the same `zlOS64.iso` through OVMF and GRUB, followed by
   the same owned long-mode transition. Both 64-bit routes wait for `ready.`,
   then send `fib 20` over a serial socket and require `6765`.

The ISO harness previously stopped QEMU as soon as it saw `compositor:`. The
BIOS route can emit that marker before `ready.`, so host load turned a healthy
boot into a false failure. The gate now waits for both markers in either order,
checks the manifest/build/source receipts, gives constrained software emulation
a larger ceiling, and creates independent GRUB BIOS and GRUB UEFI receipts.

## What the registry proves mechanically

The generator refuses to write or pass when any of these conditions fails:

- the reproducibility receipt does not name exactly the nine current outputs;
- an output's byte count or SHA-256 differs from the paired-build receipt;
- the build identity or source hashes in a receipt are stale;
- an ELF class, PE signature, ISO descriptor, BIOS signature or GPT header is
  wrong;
- the ISO's `/boot/kernel.elf` differs from the standalone `kernel.elf`;
- the USB image's `/EFI/BOOT/BOOTX64.EFI` differs from `BOOTX64.EFI`;
- the raw disk does not contain the current boot sector, deterministic binary
  transform of `kernel_raw.elf`, or zero padding after that payload;
- one of the six named boot receipts is missing, failed, points at another
  artifact hash, or reports another origin/build identity;
- the app evidence is not bound to the same ISO and build identity;
- the byte-exact `kernel64.elf` parent/runtime relationship is lost; or
- any artifact is labeled physically verified without an exact-hash physical
  receipt.

The self-test plants missing-artifact, failed-route, physical-overclaim and
lost-64-bit-runtime-proof mutations and requires every one to be rejected.

The outer landing gate also runs `check-boot-prereqs.py --selftest` before its
first build. Standalone boot scripts may still report an honest skip on a
reduced developer machine, but a full landing now requires all 13 packaging and
QEMU commands plus both OVMF code and variable-store images. Its mutation test
proves that a missing command or firmware image is detected; the checker itself
is mandatory, so deleting it also makes the landing gate red.

The same fail-closed rule now applies to the gate's own authorities. Build
identity, source coverage, test inventory/execution, memory and call checks, app
manifest/lifecycle checks, all eight named boot scripts, the three graphical
probes, both aggregate evidence generators and `kernel/SOURCES` are mandatory.
There is no `if it exists` path around them: deleting a verifier is a failure,
not removal of the requirement. `check-land-gate.py --selftest` machine-checks
64 mandatory seams and plants deleted-verifier, optional-verifier,
missing-`SOURCES` and deleted-boot-route mutations.

`check-elf-permissions.py --selftest` parses the 32-bit, 64-bit and raw ELF
program-header tables directly. Each now has separate read/execute, read-only
and read/write LOAD segments. A planted writable+executable flag on every image
must be rejected; a linker warning can no longer scroll past while the outer
gate stays green.

All four source-consuming route compilers now use `-Wall -Wextra -Werror` with
only named, justified suppressions. The repair removed four dead generated-zl
locals, two dead C helpers, a duplicate EFI-only mouse clamp, an architecture-
incorrect IDT-base cast diagnostic, hosted bounds reasoning on the freestanding
BIOS Data Area read, and a 32-bit-impossible heap branch. The focused 32-bit,
64-bit, raw and native-UEFI builds are warning-clean. `check-build-contract.py`
plants missing-`-Werror`, blanket-`-w` and missing-route mutations so warning
strictness cannot quietly disappear later.

## How to refresh it

From `kernel/`, run the paired build first, then the six boot routes across four
boot gates and the app evidence gates. Finally regenerate and check the
registry:

```sh
python3 check-reproducible-build.py --check --selftest
./verify-raw.sh
./verify-efi.sh
./verify-iso.sh
./verify-64.sh
python3 probe-app-routes.py --no-build \
  --receipt docs/receipts/app-routes-qemu-2026-08-22.json
python3 probe-app-lifecycle.py --no-build \
  --receipt docs/receipts/app-lifecycle-qemu-2026-08-22.json
python3 probe-run.py --no-build \
  --receipt docs/receipts/run-qemu-2026-08-22.json
python3 gen-app-evidence.py --write --verify-artifact
python3 gen-app-evidence.py --check --selftest --verify-artifact
python3 gen-artifact-registry.py --write --selftest
python3 gen-artifact-registry.py --check --selftest
```

`--no-build` is safe only in this ordered sequence: the paired gate has just
produced the exact ISO and the boot receipts bind it to the current identity.
Outside this sequence, omit `--no-build`.
