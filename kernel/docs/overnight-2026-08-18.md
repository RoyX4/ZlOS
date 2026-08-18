# Overnight session, 17→18 Aug 2026 — what changed and what to do next

Read this first. It is the whole night in one page. Nothing here is committed;
`git status` will show it all as working-tree changes.

## The one thing you should care about

**Your ThinkPad's boot path was loading a GDT and an IDT whose base addresses
were half uninitialised.** `lgdt` and `lidt` each take a 10-byte operand — 2
bytes of limit, 8 of base. Both structs declared the base as `unsigned long`,
which is 8 bytes under gcc but **4** under the EFI build's clang target
(`x86_64-unknown-windows`, LLP64). So in that build the structs were **6 bytes**
and the CPU took the top 32 bits of each base from whatever memory happened to
follow.

It only ever worked because those bytes happened to be zero. That made it
sensitive to code layout: any unrelated edit could shift things and the 64-bit
boot would die inside `setup_idt()` with no diagnostic at all — and on real
firmware, which loads the image at a different address, the adjacent bytes are a
different gamble entirely.

Fixed in `idt.c` and `gdt64.c` with `unsigned long long`, plus
`_Static_assert(sizeof(...) == 10)` so a reintroduction fails at compile time
rather than silently at the `lidt`.

### That fix was incomplete — the casts truncate too

Widening the struct field does nothing if the value assigned to it was already
truncated. `idtp.base = (unsigned long)&idt;` cuts the address to 32 bits and
*then* zero-extends into the now-correct 8-byte field. Four more sites had the
same shape, including two in `efi.c` that pass the **ACPI RSDP** through
`unsigned long` — and firmware is entitled to put the RSDP above 4 GiB, where a
truncated pointer sends APIC and SMP discovery into wild memory.

The worst one was in `set_gate`:

```c
unsigned long a = (unsigned long)handler;   /* 4 bytes here */
idt[n].hi = (u32)(a >> 32);                 /* shift by 32 of a 32-bit value */
```

Shifting a value by its own width is undefined. Compiled standalone under the
exact EFI flags, clang emits **a bare `retq`** for that expression — it does not
compute a shift, and it does not zero the register either. So the high 32 bits
of every interrupt gate were whatever happened to be sitting in `eax`.

All five now cast directly to a 64-bit type. Confirmed at the instruction level:
`idt_init` in the rebuilt object contains `shrq $0x20, %r8`, where the previous
build contained no shift at all.

**The general rule:** never put a pointer through `unsigned long` in the EFI
build — not as a field, not as a cast, not as an intermediate. The EFI build is
LLP64 and the others are LP64, so the same declaration is a different size in
different builds of the same file.

### And why five of them hid in the boot path

`buildefi.sh` compiled with `-w`, silencing every warning — including the four
that name this exact class. They are now re-enabled after the `-w` and fatal:
`-Werror=shift-count-overflow`, `-Werror=void-pointer-to-int-cast`,
`-Werror=pointer-to-int-cast`, `-Werror=int-to-pointer-cast`. The full 28-file
EFI build is clean under them, so these five were the only instances.

QEMU could never have caught any of this: it loads the image below 4 GiB, where
truncating the top half of an address costs nothing. Only real firmware does.

## Why nothing caught it

None of the three existing gates ever executed `kernel/efi.c`:

| Gate | What it actually boots |
|---|---|
| `verify.sh` | the **32-bit** kernel via `-kernel` |
| `verify-raw.sh` | the **32-bit** kernel via our own bootloader |
| `verify-iso.sh` | **GRUB's** `bootx64.efi`, which multiboot-loads that same 32-bit kernel |

The EFI binary inside `zlOS.iso` has 451 GRUB strings in it and zero zlOS ones.
So the path the laptop takes had no coverage at all, and a boot-killer sat in the
tree while all three gates stayed green.

**New: `kernel/verify-efi.sh`** boots `zlOS-usb.img` — our own `BOOTX64.EFI`,
no GRUB — under OVMF and checks it comes up *as a UEFI application* with a real
framebuffer. Validated both ways: green on a good build, and red with the correct
diagnosis when the GOP lookup is forced to fail. It is wired into
`run_tests.sh`, guarded on OVMF being installed.

## The mouse, which is where the night started

`try.sh` attached `-device usb-mouse`; QEMU made it the active pointer; zlOS has
no USB mouse driver, so every movement went to a device it could not read while
the PS/2 mouse it *does* drive sat idle. Beyond that, a PS/2 mouse is
**relative**, so the guest and host cursors drift apart and the far edges of the
screen cannot be reached.

Now there is a **USB tablet driver** (`xhci.c`, `xhci_ptr_*`) — an *absolute*
pointer. Every report carries the position, so nothing accumulates and nothing
drifts. Measured: sending 75%/90% lands the guest cursor on exactly
`1439,1079`, and the bottom-right corner on exactly `1919,1199`.

Three pointer paths exist now where there was one: USB tablet (absolute), USB
mouse (relative, boot protocol — new capability), PS/2 (the TrackPoint).

Also fixed: the pointer clamped to a hardcoded 2000x1500. On your 2560-wide
panel that would have made the right **560 px unreachable**.

Full detail, including three bugs from two HID devices sharing one controller:
`kernel/docs/input-stack.md`.

## State of the tree

```
verify.sh   verify-raw.sh   verify-iso.sh   verify-efi.sh      all PASS
sweep BIOS   30/30 steps ok
sweep UEFI   30/30 steps ok
soak: 3 rounds x (4 gates + 2 sweeps) - zero variance, no flakiness
```

Nothing is failing. Every driver and every command the shell advertises is
exercised and confirmed on both boot paths.

## The bug that looked like an editor bug

It was never the editor. `configure_endpoint()` opened with:

```c
ring_init(INT_RING(slot));
kbd_enq = 0;              /* the KEYBOARD's producer index */
kbd_cyc = 1;              /* reset for ANY slot */
```

It reset the keyboard's transfer-ring producer state on every call, whatever
slot it was configuring — fine while the keyboard was the only caller. The
pointer became the second: configuring slot 3 rewound `kbd_enq` after the
keyboard had already armed index 0, so the keyboard's next requeue wrote index 0
again while the controller waited at index 1 for a cycle bit that would never
match. One key, then dead forever.

Measured signature, which named it exactly: `kbd events 1, requeues 1,
last cc 1` — the re-arm *was* issued, into a slot the controller had passed.

Fixed by moving the producer-state reset into each device's own init. Verified:
injecting three keys into two consecutive runs of `=` gave 1 then 0 events
before, and 3 then 3 after.

**This was the fourth bug of the same shape** — state that was accidentally
correct while exactly one HID device existed — and the only one in pre-existing
code rather than the new driver. The pattern and the rule are in
`input-stack.md`: *state named after a device belongs to that device.*

## Things I got wrong, so you can discount accordingly

- Reverted a **correct** ISR fix because it appeared to break the 64-bit boot. It
  did not; it moved the layout and exposed the descriptor bug. Restored, passes.
- Used `ptr_reports` to rule out an event storm. That counter only increments on
  a *successful* decode, so it could never have seen one. Redone properly.
- Rebuilt in the same directory as my own soak, clobbering object files — the
  exact hazard I had warned about for two concurrent sessions. Cost one false
  "build error" and a wrong turn.
- Gave three wrong explanations for the mouse before the right one.

## Next steps

1. **Boot the laptop.** `kernel/docs/thinkpad-first-boot.md` is the checklist.
   Secure Boot off; **do not press `o`** (NVMe demo writes to LBA 1000 of your
   real disk); `k` is the one worth booting for; `?` is the touchpad, which has
   never executed anywhere.
2. The USB HID path emits only `EV_CHAR` (`input.c:270`) — no key-down/up, so no
   arrows or Home/End over USB. Works on PS/2, not on USB. Not blocking.

## Not touched

`kernel/HANDOFF.md`, `kernel/intel.c`, `kernel/hosttest/` — the other session
owns those. Nothing committed, nothing pushed, no block device written.
