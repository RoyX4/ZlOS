# The 64-bit BIOS boot route was broken and unwatched

Found while regenerating the application-manifest boot receipts on 2026-08-27.
Not caused by that work — measured, not argued. Recorded because the *reason*
nobody knew is more useful than the symptom.

## Resolution recorded later that day

This is now historical failure evidence, not current status. After the C
backend's silent `NAMESET_MAX` overflow was made a hard error and the PRESSWORK
tree was reconciled, the BIOS route reached `ready.` and returned
`fib(20)=6765`. The tracked `grub-bios64` receipt records a passing QEMU boot,
and `verify-64.sh` is now mandatory in `gates/land-gate.sh`; the landing-gate
selftest rejects removing it.

The original location claim below was also wrong. The boot lines through
"keyboard on IRQ1" are printed from `kernel.zl`, so the route had already
crossed the C-to-zl boundary. The next work includes USB and diagnostic retry
before the persistent-observer line. No exact stopped symbol was captured in
the failing build, so this record does not promote the later compiler repair
from a strong causal candidate to a proved per-symbol root cause.

## The symptom

`kernel/tools/checks/verify-64.sh` boots the 64-bit kernel by two routes. One
passes and one does not:

```
== 64-bit multiboot: legacy BIOS + GRUB ==
  FAIL  BIOS - missing marker: ready.
        last successful markers:
            [  OK  ] stack established, 256 KiB
            [  OK  ] COM1 initialised, 115200 8N1
            [  OK  ] framebuffer console, 240x75
            [  OK  ] GDT loaded - 64-bit flat segments, 4-level paging, SSE on
            [  OK  ] IDT installed, PIC remapped, interrupts ON
            [  OK  ] APIC: IRQs via I/O APIC at 0xFEC00000, 2 CPU(s)
            [  OK  ] PIT timer running at 100 Hz on IRQ0
            [  OK  ] keyboard on IRQ1 - no more polling the port
== 64-bit multiboot: UEFI + GRUB ==
  ok    UEFI - exact kernel64.elf entered long mode, opened the compositor, fib(20)=6765
```

At the time this was first written, the stop was incorrectly placed after the
C-side init and before the zl program spoke. Source inspection later disproved
that: these markers are already emitted by `kernel.zl`. The next visible marker
in a healthy boot is the persistent observer, after USB and diagnostic retry.

**The same `kernel64.elf` passes by UEFI.** Long mode, paging, the compositor and
`fib(20)=6765` all work. So this is not the 64-bit kernel being broken; it is
something about what the BIOS+GRUB multiboot path hands it.

## It is NOT new — the A/B

Run in a detached worktree at `7d1a11b`, the last commit pushed to origin, which
predates the whole `design/presswork` branch:

```
git worktree add --detach /tmp/zlbase 7d1a11b
cd /tmp/zlbase        && ./build.sh      # the zl toolchain
cd /tmp/zlbase/kernel && ./build.sh      # the kernel
bash /tmp/zlbase/kernel/tools/checks/verify-64.sh
  -> FAIL  BIOS - missing marker: ready.
  -> ok    UEFI - ...
  -> BASELINE verify-64 EXIT: 1
```

The failure signature is byte-identical to the current tree's: the same eight
markers, the same stall point.

## Why nobody knew

**`tools/preflight.sh` does not run `verify-64.sh`.** Its boot block is:

```
run_noskip "boot: bios32" kernel/verify.sh
run_noskip "boot: raw"    kernel/tools/checks/verify-raw.sh
run_noskip "boot: iso"    kernel/tools/checks/verify-iso.sh
run_noskip "boot: efi"    kernel/tools/checks/verify-efi.sh
```

Four routes gated, and the 64-bit multiboot pair is not among them. The pre-push
hook runs preflight, so this route has never blocked a push.

The receipts show exactly how long that has been true. Every other route's
boot receipt was regenerated at `7d1a11b` against manifest `d6c31620…`:

| receipt | source_head | manifest sha |
|---|---|---|
| raw-bios, grub-bios32, grub-uefi32, grub-uefi64, native-uefi64 | `7d1a11b` | `d6c31620…` |
| **grub-bios64** | **`b8a00ec`** | **`a1c4ec15…`** |

`grub-bios64` fell out of regeneration a commit era earlier than the rest, and
its stale `PASS` has been sitting in the tree ever since — a receipt that says
the route works, dated from the last time it did.

## What had NOT been done at discovery time

The cause was not diagnosed and no fix was attempted in that run; it was about the
manifest receipts and the desktop, and a 64-bit multiboot hand-off hang is a
different subsystem. What is established is: it is real, it is reproducible, it
is not new, and it is invisible to every gate that runs.

The required sequence was to repair the route, confirm both paths green, then
make the verifier mandatory and regenerate its receipt. That sequence has now
been completed in the contained landing path. Physical BIOS firmware remains
unverified; this evidence is QEMU only.
