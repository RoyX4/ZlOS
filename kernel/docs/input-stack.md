# The input stack: keyboard, pointer, and what breaks when there are two

Written after a session that started with "the mouse doesn't work" and ended
four bugs later. Everything here is measured, not remembered; where something
is unproven it says so.

## The short version

zlOS now has three pointer paths, where it had one:

| Device | Kind | Driver | Where it matters |
|---|---|---|---|
| USB tablet | **absolute** | `xhci.c`, `xhci_ptr_*` | the VM - the cursor cannot drift |
| USB mouse | relative | same code, boot protocol | an external mouse on the laptop |
| PS/2 mouse | relative | `idt.c`, IRQ12 | the ThinkPad's TrackPoint |

`runtime_kernel.c` prefers the USB pointer when one is present and falls back
to PS/2. zl code calls `mouse_x()` / `mouse_y()` / `mouse_btn()` and never
learns which.

## Why a tablet, and not just "fix the mouse"

A PS/2 mouse is **relative**: it reports "moved 3 right, 2 up" and the driver
accumulates a position. That means the guest cursor and the host cursor are two
separate positions kept in step only by luck, and in a window they drift apart -
the host pointer leaves the window while the guest one lags behind and can never
be pushed to the far edges. QEMU states it plainly:

```
query-mice -> {"name": "QEMU PS/2 Mouse", "current": true, "absolute": false}
```

No clamp, resolution change or kernel fix addresses that; it is what relative
input *is*. The cure is an **absolute** device, where every report carries the
position itself so there is nothing to accumulate and nothing to drift:

```
query-mice -> {"name": "QEMU HID Tablet", "current": true, "absolute": true}

sent 75%,90%      -> guest at 1439,1079     (0.75*1919, 0.90*1199)
sent 10%,10%      -> guest at  191,119
sent bottom-right -> guest at 1919,1199
```

Exact, every time. No pointer grab needed.

**A tablet must not be put in boot protocol.** Boot protocol gives you the
4-byte relative mouse report and throws the absolute position away - which is
the entire reason for using one. `xhci_ptr_init()` sends `SET_PROTOCOL` only
for a boot mouse.

Report layouts, for reference:

```
boot mouse (subclass 1, protocol 2):  [buttons, dx, dy, wheel]         relative
tablet     (subclass 0, protocol 0):  [buttons, xlo, xhi, ylo, yhi, w] absolute
```

A tablet is not a boot device, so it cannot be matched on the class triple the
keyboard uses. The test is "HID, an interrupt IN endpoint, and not a keyboard".

## The trap: this stack was written for ONE HID device

Adding a pointer broke three separate assumptions, all of them the same
assumption written three different ways: *the next thing I see is mine*.

**1. The keyboard's event handler swallowed the pointer's completions.**
One event ring carries both devices, so whichever poll runs first sees the
other's events too. Dropping a transfer event also drops its requeue, and that
endpoint then goes silent permanently. `kbd_event()` now dispatches on
slot+endpoint and requeues whichever device the event belonged to.

**2. Pointer discovery re-enumerated the keyboard.** `xhci_ptr_init()` walks
the ports looking for a pointer, and `xhci_enumerate()` *resets the port and
re-addresses the device*. Running that over the port the keyboard already owns
tears a working keyboard out from under itself. A `slot == kbd_slot` check
afterwards is far too late - the damage happens inside `xhci_enumerate()`. It
records `kbd_port` and skips that port **before** touching it.

**3. `xhci_key()` gave up on the first foreign event.** It polled the ring once
and returned whatever was in the key queue. When the popped event belonged to
the pointer the queue was empty, so it returned 0 - and `input.c:271` reads 0 as
"no more keys" and stops draining. With a pointer producing events steadily,
real keystrokes queue up behind them and never surface: the keyboard appears
completely dead while the ring is in fact busy. It now drains until a key comes
out or the ring is empty.

Measured, with `exercise.py --only input,editor`:

```
tablet attached, before the fix:  editor never regains the keyboard, cascades
tablet detached (control):        3/3 ok
```

**The rule for anything sharing that event ring:** dispatch by slot+endpoint,
never assume the next event is yours, and never stop draining because one event
was not.

## Interrupt handlers must not call out of their file

`build.sh` and `build64.sh` compile `idt.c` and `apic.c` with
`-mgeneral-regs-only` so a handler can never touch SSE. That guarantee **only
covers code inside those files**. The moment a handler calls a function
compiled normally, the callee may freely use XMM registers the handler never
saved - and because every zl number is a double, the corruption lands directly
on the interpreter.

`buildefi.sh` did **not** apply that flag. The EFI build - the only one the
laptop runs - has never had the protection the other two do. It stayed
invisible only because the handlers were small enough that the compiler had no
reason to reach for XMM. That is fixed.

Consequence for the pointer clamp: the mouse ISR clamps to the screen, and it
gets the screen size **pushed in** by `fb_setup()` calling
`idt_set_pointer_bounds()`, rather than pulling it with `console_pxw()`.
Handlers get data pushed to them; they do not call out to fetch it.

## The clamp was wrong for the hardware

`idt.c` clamped the pointer to a hardcoded 2000x1500. On a 1920x1200 panel that
is 80 px right of and 300 px below the visible area, so the pointer could sit
somewhere with no pixels and need 300 px of travel before the cursor moved
again - which reads as "the mouse will not go where I put it".

It is worse on the ThinkPad, whose panel is **2560x1440**: a clamp of 2000 would
have made the right 560 px of the screen permanently unreachable. It now clamps
to the live framebuffer.

## Diagnostics left behind

The mouse demo (`x`) prints, on exit, how many IRQ12 packets arrived and where
the pointer ended up. That single line is what made the difference between
"dead" and "arriving but decoded wrong" - two failures that look identical from
the outside and have nothing in common.

Probe scripts, each written to settle one question:

| Script | Answers |
|---|---|
| `probe-mouse.py` | does the pointer move at all, and does a click register |
| `probe-mouse-range.py` | how far can it reach - a clamp bug, or the window running out |
| `probe-mouse-stress.py` | do packets survive keyboard traffic on the shared 8042 |
| `probe-keys.py` | does an injected key reach zlOS, and by which road |
| `probe-anim.py` | is an animated demo actually reaching the screen |
| `probe-uefi.py` | which display devices does OVMF publish a usable GOP for |

## The fourth one: configure_endpoint reset another device's ring state

**FOUND AND FIXED.** This is the bug that presented as "the editor never gets
its ESC", and it was never an editor bug at all.

`configure_endpoint()` began:

```c
static int configure_endpoint(int slot, int dci, ...)
{
    ring_init(INT_RING(slot));
    kbd_enq = 0;                 /* the KEYBOARD's producer index */
    kbd_cyc = 1;                 /* reset for ANY slot */
```

It reset the **keyboard's** transfer-ring producer state on every call, whatever
slot it was configuring. Safe while the keyboard was the only caller. The
pointer calls it too, for its own slot, and the sequence then is:

1. `xhci_kbd_init` configures slot 2, arms the ring at index 0, `kbd_enq` = 1
2. `xhci_ptr_init` configures slot 3 — and rewinds `kbd_enq` to 0
3. a key arrives, the controller completes the TRB at index 0, dequeue moves to 1
4. `kbd_requeue()` writes index **0** again, because `kbd_enq` was rewound
5. the controller is waiting at index 1, where the cycle bit will never match

One report, a requeue issued into a slot the controller had already passed, and
then silence forever. Exactly what was measured: `kbd events 1, requeues 1,
last cc 1` — the re-arm *was* issued, it just went to the wrong place.

Fixed by moving the producer-state reset out of `configure_endpoint` and into
each device's own init, next to where it arms its own ring. The state is
per-endpoint; it never belonged in a shared helper.

Before and after, injecting three keys into two consecutive runs of `=`:

```
before:  run 1: 1 event ('a')          run 2: 0 events
after:   run 1: 3 events (a, b, ESC)   run 2: 3 events (c, d, ESC)
```

### The pattern, stated once

Four bugs, one mistake in four costumes — **state that was accidentally correct
while exactly one HID device existed**:

| | What was shared that should not have been |
|---|---|
| 1 | `kbd_event()` treated any transfer event as the keyboard's |
| 2 | `xhci_ptr_init()` re-enumerated a port another device owned |
| 3 | `xhci_key()` stopped draining on the first foreign event |
| 4 | `configure_endpoint()` reset the keyboard's producer state for any slot |

Number 4 was in **pre-existing** code, not in the new driver — adding a second
HID device merely supplied the second caller it had always been waiting for.

**The rule: state named after a device belongs to that device.** `kbd_enq` and
`kbd_cyc` sitting in a function that takes a `slot` parameter was the tell, and
it was visible by reading long before it was found by measuring.

## How it looked before it was understood

Measured, all of it:

| Sequence | Result |
|---|---|
| `editor` alone | passes |
| `mouse` then `editor` | passes |
| `mouse, input, mouse` | passes 3/3 |
| `input` then `editor` | **fails** - editor never returns |
| `input` then `editor`, tablet detached | passes |

State at the time: `usb kbd slot 2 ep 3`, `ptr slot 3 ep 3` — different slots,
so no `INT_RING()` collision. During `=` only the FIRST injected key is
reported. Reproduces identically on both BIOS and UEFI, and costs three steps
(`editor`, `clear`, `halt`) out of thirty.

Theories tested: shared-ring dispatch, keyboard port re-enumeration,
`xhci_key()` giving up on foreign events, slot collision, drain depth, and the
descriptor-pointer bug below. The first three were real bugs and are fixed —
they are just not *this* bug.

### The measurement that was wrong, and the lead it opens

"`ptr reports 1`, therefore no event storm" was **not a valid conclusion**, and
it is worth saying so plainly because it closed off the most likely explanation
for hours.

`ptr_reports` is incremented inside `ptr_decode()`, which only runs when the
completion code is success or short-packet (`cc == 1 || cc == 13`). But
`kbd_event()` calls `ptr_requeue()` **unconditionally, for any completion
code**. So an endpoint erroring repeatedly — a STALL (`cc == 6`), say — would be
requeued forever, would monopolise the shared event ring, would starve the
keyboard behind it, and would leave `ptr_reports` sitting at 1. Which is exactly
what was observed.

The counter measured the wrong thing, so the experiment was redone properly:
`ptr_events` counts **every** dispatch regardless of completion code, and
`ptr_lastcc` records the last one. Both are printed by the `x` demo.

Result, after running the demo that triggers the failure:

```
usb kbd slot 2 ep 3   ptr slot 3 ep 3   ptr reports 1   events 1   last cc 13
```

**One dispatch, completion code 13 (short packet — success). There is no storm.**
The pointer endpoint is quiet and healthy and is not interfering with the ring.
The hypothesis is now genuinely disproven rather than dismissed on bad evidence.

That leaves the second, independent lead: the editor is the only path that calls
`xhci_key()` **directly**, bypassing `input_poll()` and its `evq`. Everything
else reads through `in_char()`/`in_next()`. So a key consumed by `input_poll`
and pushed into `evq` is invisible to the editor.

The `editor` step is ordered LAST in `exercise.py` so this failure costs only
itself instead of cascading over every step after it. When it does wedge, the
harness now reports the remaining steps as **skipped** rather than failed — they
were never run, and calling them failures overstates the damage as much as
hiding them would understate it.

`system_reset` over QMP is **not** a usable recovery here: the guest runs with
`-no-reboot` on purpose, so a reset makes QEMU exit and the serial socket break
mid-run. That was tried and it took the harness down with it.

**Confirmed independent of the descriptor-pointer bug below.** With `idt_ptr`
and `gdt_ptr` fixed and all four gates green, `input` then `editor` still fails
exactly as before. Two separate problems that happened to be in flight at the
same time.

## The descriptor pointers were half-uninitialised in the EFI build

This is the one that mattered, and it hid behind everything else all night.

`lgdt` and `lidt` each take a 10-byte operand: 2 bytes of limit, **8 of base**.
Both structs were declared with `unsigned long base`:

```c
struct idt_ptr { u16 limit; unsigned long base; } __attribute__((packed));
struct gdt_ptr { unsigned short limit; unsigned long base; } __attribute__((packed));
```

`unsigned long` is 8 bytes under gcc (LP64), so `build.sh` and `build64.sh` were
always correct. It is **4 bytes** under the EFI build's clang target,
`x86_64-unknown-windows` (LLP64) — making both structs **6 bytes**. The CPU
still reads 10, so the **top 32 bits of both descriptor table bases came from
whatever happened to sit after the struct in memory.**

It worked only while those neighbouring bytes were zero. That makes it
exquisitely sensitive to code layout: an unrelated edit anywhere could shift
things and the 64-bit boot would die at the `lidt` with no diagnostic at all.

How it presented, and why it wasted hours:

- A change to the interrupt handlers "broke the 64-bit boot". It did not — it
  moved the layout. The change was reverted for the wrong reason, then
  re-applied afterwards and passed 3/3.
- Two functionally *identical* versions of the same code behaved differently.
- Adding a few bytes of unrelated debug instrumentation made the failure
  disappear, which is the tell.

Proof it is real, not inferred:

```
gcc                                sizeof(idt_ptr) == 10   correct
clang -target x86_64-unknown-windows   static assertion FAILED - it is 6
```

Fixed with `unsigned long long` in both files, plus
`_Static_assert(sizeof(...) == 10)` so a reintroduction fails at compile time
instead of silently at `lidt`.

**On real hardware this is worse than in QEMU.** Firmware loads the image at a
different address, so the bytes after the struct are a different lottery. A
laptop that boots today could stop booting after an unrelated commit, with a
black screen and nothing on the (non-existent) serial port to explain it.

**The general rule:** never use `long` in a structure the CPU reads. The EFI
build is LLP64 and the others are LP64, so any `long` in a hardware-defined
layout is a different size in different builds of the same file. `set_gate()`
still casts a handler pointer through `unsigned long`, which truncates to 32
bits on that target - harmless only while the image loads below 4 GiB
(measured: `&idt` at 0x3DD331B0, about 1 GiB), and worth fixing for the same
reason.

## Things that are still not known

- **Why the restructured interrupt handlers killed the 64-bit boot.** A bisect
  proves they did - original handlers boot, the restructured ones die inside
  `setup_idt()` - but the mechanism was never found. Ruled out: the ISR calling
  `console_pxw()`, the tablet driver, and the missing `-mgeneral-regs-only`.
  The restructure was reverted because it was an unproven fix for a collision
  that a controlled A/B could not reproduce.
- **The touchpad.** `i2c_hid.c` is complete and has zero test coverage. QEMU
  does not emulate Intel's LPSS I2C controller at PCI 00:15.1, so there is
  nothing for it to talk to and no flag that conjures one. It can only ever be
  tested on the laptop.
- **The USB HID path emits only `EV_CHAR`.** `input.c:270` pushes characters and
  nothing else, so over USB there are no key-down/key-up events and no
  navigation keys - an ESC arrives as character 27, not `0x101`. Anything keying
  off arrows or Home/End works on PS/2 and not on USB.

## Testing the pointer

`exercise.py`'s `mouse` step sends an **absolute position** and asserts the
guest lands on exactly `1439,1079`. Pixels are the wrong test and cost real
time: the cursor is 12x18 on a 1920x1200 screen, so moving it repaints 0.02% -
indistinguishable from noise. Ask the guest what it received instead.
