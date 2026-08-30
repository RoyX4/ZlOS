# Crash-receipt feature classification - 2026-08-29

This receipt classifies EV-017 and KR-032 against the canonical 906-feature
ledger. It records current 32-bit and 64-bit pre-prologue exception frames,
including an error-code fault and a double fault on a dedicated emergency
stack, without promoting the missing broader matrix, persistence,
symbolization or recovery contracts.

## Current proof

The 32-bit and 64-bit exception gates use assembly entry stubs that normalize
CPU error codes and save the general registers before any C compiler prologue.
The bounded crash record is version 3 and has one asserted 240-byte layout in
ILP32, LP64 and LLP64 compilation lanes. It contains:

- vector, error-code presence/value, IP, CS, flags, SP, SS and CR2;
- an explicit register-validity mask;
- AX, BX, CX, DX, SI, DI, BP and the fault-time SP on the proved BIOS32 route;
- reserved zero lanes for R8 through R15 on the proved 32-bit route, and all
  sixteen general registers on the proved 64-bit route; and
- the handler stack pointer and emergency-stack low/high bounds; and
- a checksum committed before the record magic, so a nested capture cannot
  overwrite or expose a half-written first cause.

The exact current `zlOS.iso` and `zlOS-usb.img` produced four passing QEMU
receipts. BIOS32 and native UEFI64 each execute a real `UD2` and record vector
6. A second native UEFI64 route loads invalid selector `0x38` into DS and
records vector 13 with error code `0x38`. Those three routes bind the recorded
IP to the exact runtime fault label. All eight 32-bit or sixteen 64-bit general
registers match their expected values, the recorded register SP matches the
control-frame SP, each checksum independently verifies, and each guest remains
halted with no returned prompt.

The fourth route clears RSP and performs a memory access. The resulting page
fault cannot construct its ordinary frame and escalates to vector 8. The
64-bit TSS selects a dedicated 16 KiB IST1 stack for that vector. The receipt
requires the serialized handler SP and an independently queried halted QEMU CPU
RSP to both fall strictly inside the recorded IST1 bounds. The interrupted SP
is preserved as zero and the architecturally fixed double-fault error code is
zero. Intel documents that the saved instruction pointer and program state for
`#DF` are undefined, so this receipt deliberately makes no symbol/IP claim for
that route. See the [Intel 64 and IA-32 Architectures Software Developer's
Manual, Volume 3A](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf),
sections 6.12.4.1 and 7.14.2.

The host harness links the real `crash.c` under ASan/UBSan and passes 39 checks
covering admission, schema identity, all-register copies, checksum changes,
first-cause preservation, bounded formatting, register masks, invalid
double-fault error codes and emergency-stack escape. The QEMU verifier rejects
wrong vectors, error codes, checksums or symbols; missing or wrong registers;
invented upper registers; stale artifacts; a non-halted guest; and a double
fault whose recorded or independently observed stack pointer leaves IST1.

## Evidence ceiling

EV-017 and KR-032 remain `PARTIAL_CURRENT`. The current fatal-crash QEMU
receipts cover vectors 6, 8 and 13, but do not constitute a broad exception or
spurious-interrupt matrix. Only UD2 has both 32-bit and 64-bit route coverage.
A separate bounded native-UEFI64 two-slot process receipt proves one Ring-3 GP
offender can be removed while its sibling continues, but it is not a general
process recovery service. The emergency stack has no guard page, and stopping
a double fault is not recovery. There is no checked unwind/symbol table,
durable crash partition commit, recovery-readable store, bounded reboot/rescue
policy, crash service, or physical fault/power-loss receipt.

The next contained step is broader exception/spurious-interrupt coverage plus a
guarded emergency stack and checked symbol/unwind binding. Durable promotion
additionally requires a
recovery-readable exact-build crash bundle that survives the required
power-loss cases without damaging earlier evidence.
