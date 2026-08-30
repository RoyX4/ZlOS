# QEMU segfaults on this box, and the gates blamed the kernel

Four times on 2026-08-27, `qemu-system-x86_64` crashed while running a boot gate.
Two of the four were never diagnosed at the time, because every gate reported
them as a kernel failure.

## It is the same crash every time

```
18:51:24   19:19:10   21:11:36   21:29:14
```

The kernel log for the last two:

```
qemu-system-x86[1249433]: segfault at 10 ip 000055df0e6f1234 sp 00007f17ead12f48
    error 4 in qemu-system-x86_64[6ee234,55df0e4ca000+67b000]
qemu-system-x86[1290219]: segfault at 10 ip 0000564f170a6234 sp 00007f3440314f48
    error 4 in qemu-system-x86_64[6ee234,564f16e7f000+67b000]
```

Subtracting the mapped executable base from the instruction pointer gives
`0x227234` in every observed crash. The absolute `ip` values differ only
because of ASLR. The bracketed `6ee234` field is also identical, but this
receipt does not assign it a meaning. `segfault at 10` is a read of address
`0x10` - a NULL struct pointer plus a small field offset - and `error 4` is a
user-mode read of an unmapped page.

So this is **one deterministic bug at one instruction**, not memory pressure and
not random corruption. Load was a red herring: it crashed at load 4.56 and it
also completed cleanly three times in a row at load 1.77.

## Only the EFI gate triggers it

`verify-efi.sh` is the only gate that drives this combination:

```
-device qemu-xhci,id=xhci
-device usb-storage,bus=xhci.0,drive=boot
-device usb-kbd,bus=xhci.0
-device usb-mouse,bus=xhci.0
    ... under OVMF pflash
```

and it is also a gate that **SIGTERMs QEMU** once it has seen its marker. The
2026-08-27 evidence suggested a teardown path dereferencing a device pointer
that was already gone because those observed crashes happened after the kernel
had written every required marker. The 2026-08-30 recurrence included crashes
before the final prompt, so teardown is no longer an adequate explanation.

The QEMU source cause is not proven. Nobody has attached a debugger to QEMU;
the stable relative instruction and NULL-like address are the current hard
boundary, written down so the next person starts from evidence rather than
from "the kernel is broken".

## What the gates did about it

Nothing, and that was the real problem. The first audit found five gates that
threw QEMU's `wait` status
away and judged on log contents alone, so a crashed emulator was
indistinguishable from a silent kernel:

```
FAIL  the kernel never started
```

True, and pointing at entirely the wrong thing. It cost a full push cycle to
work out, and it had already cost two undiagnosed failures earlier the same day.

The closure audit found the same discarded status in the clock, disk and
network gates too. `kernel/tools/checks/qemu-crash.sh` is now the single answer,
sourced by all eight landing boot gates, with `qemu-crash-selftest.sh` proving
it fires for unexpected signal exits 128..142 and stays silent on 143 - **the
SIGTERM the gates send themselves on every healthy boot**,
which is the case that matters, because a version treating "died by signal" as a
crash would fail every green run.

**It reports; it does not decide.** The first version of the fix returned a
verdict, and the first real crash exposed why that is wrong: the segfault
happened during teardown, after the kernel had written everything, and the gate
went red on a boot that had demonstrably succeeded. A mid-boot crash truncates
the log and the marker checks fail on their own - and then the crash line is the
explanation for that failure rather than a second one.

## 2026-08-30 recurrence and bounded recovery

QEMU 11.0.3 (`Debian 1:11.0.3+ds-2`, build ID
`6728e9df9faf18c487ff53140f10ee58ea3931dd`) reproduced the same fault four
more times at 10:10:03, 10:30:28, 10:31:08, and 10:33:18. Each event had
`segfault at 10`, relative instruction `0x227234`, and bracket field `6ee234`.
The first two occurred in complete `verify-efi.sh` runs; a subsequent run of
the byte-identical `zlOS-usb.img` passed, refuting the image as the cause. One
crash happened after zlOS had already reached its persistent-journal marker,
so "the kernel started" is not a valid boundary for classifying this emulator
fault.

`verify-efi.sh` now keeps KVM as its first route. If and only if QEMU exits from
an unexpected signal before the final `ready.` marker, the verifier reports the
emulator crash, recreates the OVMF variables file and USB-image working copy,
and retries once with TCG. Guest timeouts, ordinary non-signal exits, missing
guest observations, and any failed TCG retry remain red. `ZLOS_FORCE_TCG=1`
exists only to exercise the fallback execution lane directly; that lane passed
the complete UEFI marker set and regenerated the scheduler, process, allocator,
and page-table receipts.

This is QEMU execution proof, not physical-hardware proof. The exact QEMU
source function at relative instruction `0x227234` is still unknown because the
installed binary has no usable line symbols and no debugger was attached.
