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

Read the bracket: `6ee234` is the **file offset of the faulting instruction
inside the binary**, and it is identical across crashes. The absolute `ip`
values differ only because of ASLR. `segfault at 10` is a read of address
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

and it is also a gate that **SIGTERMs QEMU** once it has seen its marker. A
teardown path dereferencing a device pointer that is already gone fits every
piece of the evidence: same instruction, small offset from NULL, intermittent,
and harmless to the guest - the observed crash happened *after* the kernel had
written every marker the gate wanted.

Not proven. Nobody has attached a debugger to QEMU, and this run did not try;
it is a hypothesis with the evidence that suggests it, written down so the next
person starts from here rather than from "the kernel is broken".

## What the gates did about it

Nothing, and that was the real problem. All five threw QEMU's `wait` status
away and judged on log contents alone, so a crashed emulator was
indistinguishable from a silent kernel:

```
FAIL  the kernel never started
```

True, and pointing at entirely the wrong thing. It cost a full push cycle to
work out, and it had already cost two undiagnosed failures earlier the same day.

`kernel/tools/checks/qemu-crash.sh` is now the single answer, sourced by all
five, with `qemu-crash-selftest.sh` proving it fires on 139/134/135 and stays
silent on 143 - **the SIGTERM the gates send themselves on every healthy boot**,
which is the case that matters, because a version treating "died by signal" as a
crash would fail every green run.

**It reports; it does not decide.** The first version of the fix returned a
verdict, and the first real crash exposed why that is wrong: the segfault
happened during teardown, after the kernel had written everything, and the gate
went red on a boot that had demonstrably succeeded. A mid-boot crash truncates
the log and the marker checks fail on their own - and then the crash line is the
explanation for that failure rather than a second one.
