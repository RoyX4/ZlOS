# The pointer, after the eleven-track merge

Companion to [MERGE-EVIDENCE.md](MERGE-EVIDENCE.md). That document records what
the merge cost and what it found. This one records the first thing a person
touched afterwards: moving the mouse in `./try.sh` produced jumpy, laggy,
unpredictable motion, and every headless gate in the repo was green while it
did.

Written against `06ced13`, on branch `fix/pointer-drain`.

**Status: the code change is done and measured. Phase 1 is NOT closed** — its
gate is a human moving a mouse in a live VM, and that has not happened yet.
Everything below is a bench measurement.

---

## 1. Why no gate could see it

Not because the probes were weak. Because they test a different device.

```
try.sh          -device usb-mouse       RELATIVE - reports a delta
probe-dock.py   -device usb-tablet      ABSOLUTE - reports a position
probe-drag.py   -device usb-tablet
probe-mouse-sync.py                     usb-tablet
probe-resize.py                         usb-tablet
probe-snake.py                          usb-tablet
probe-mouse.py  defaults to             usb-tablet
```

Every probe attaches a tablet. `try.sh` — the thing a person runs — attaches a
mouse. The relative path had **no automated coverage of any kind**.

That distinction is not cosmetic, and it is the reason the bug could hide:

- an absolute report carries the whole truth, so the **latest** one is
  sufficient and every earlier one is redundant. Dropping nine reports out of
  ten is invisible.
- a relative report carries a **delta**, so dropping nine out of ten drops
  nine tenths of the hand's movement.

The prompt's guess — that the probes move in single large jumps — is true, and
`probe-mouse.py` even leads with a stale claim that "xhci.c contains no mouse
code at all". But the device class is the sharper answer: on a tablet, one
report per frame is *correct*.

The harness says so itself, and says it backwards. `kernel/exercise.py:308`:

> No usb-mouse (relative): zlOS would rather have the TABLET, which is what
> try.sh attaches

`try.sh:30` attaches `-device usb-mouse,bus=xhci.0` and no tablet. The comment
asserting the two agree is the reason nobody checked whether they did.

## 2. What the hardware actually does

Verified against QEMU 11.0.1's `hw/input/hid.c`, not assumed.

`hid_pointer_sync()` merges a new event into the previous unread one when the
buttons have not changed (`prev->xdx += curr->xdx`), and when its queue is full
it returns **without clearing the pending slot**, so deltas keep accumulating
into it. `hid_pointer_poll()` then delivers

```c
dx = int_clamp(e->xdx, -127, 127);
e->xdx -= dx;
```

— at most 127 counts per report, **keeping the remainder**.

Two consequences, and both matter:

1. **The device never loses motion.** However slowly the guest drains, total
   displacement is preserved. So the fault could not be "QEMU dropped it".
2. **The guest's maximum pointer speed is exactly `127 x (reports taken per
   second)`.** That is a number, and it is the number the whole bug reduces to.

### Why a slow drain cannot be outrun

The one step that had to be *verified* rather than assumed: with a single
outstanding TRB, can a re-armed transfer complete within the microseconds
before the next poll in the same frame? If it could, the extra polls per frame
would recover the reports and no ceiling would exist.

It cannot. QEMU's `hw/usb/hcd-xhci.c`:

```c
static void xhci_calc_intr_kick(...)
{
    uint64_t asap = ((mfindex + epctx->interval - 1) & ~(epctx->interval-1));
    uint64_t kick = epctx->mfindex_last + epctx->interval;
    xfer->mfindex_kick = MAX(asap, kick);
}
```

and `xhci_check_intr_iso_kick()` arms a timer and sets `running_retry = 1` when
that moment has not arrived. An interrupt transfer submitted immediately after
the previous one completes is deferred to at least
`mfindex_last + interval` — the endpoint's service interval is enforced, and a
requeued TRB **cannot** complete early. `interval_encode()` gives 6 for a
full-speed `bInterval` of 10, so `1 << 6` = 64 microframes = **8 ms**.

So the depth-1 ceiling is real: draining harder within a frame finds nothing,
and only more outstanding buffers raise it.

## 3. The defects, measured

`kernel/hosttest/xhcitest.c` compiles the shipping `xhci.c` and `input.c`
unmodified and plays the controller against them. Measured on `06ced13`:

```
  reports the driver takes per frame: 1
  pointer travel available in one frame: 100 px
  -> at 100 Hz, a ceiling of 10,000 px/s
```

A brisk mouse move is 3,000–10,000 px/s. The pointer was running **at its
ceiling during ordinary use**, and past it the surplus queued inside QEMU and
dribbled out at 127 counts per frame — which is why it kept gliding after the
hand stopped.

### 3a. Two drainers of one event ring — merge damage

`xhci_kbd_poll()` and `xhci_ptr_poll()` were byte-for-byte the same function,
one from each side of the merge. Neither was wrong alone. `input.c` called them
at different rates off the **same** ring — the pointer's once from
`pump_mouse()`, the keyboard's in a loop — so a keystroke sitting in front of a
pointer report meant `pump_mouse()` spent its single poll on the keystroke and
the pointer did not move at all that frame.

Reproduced directly:

```
  keystroke first on the ring: the pointer still moves 30 px  FAIL  (got 0, wanted 30)
```

`kbd_event()` already dispatched by slot+endpoint, so the prompt's hope that
"the fix is mostly deletion" was right for this part.

### 3b. One outstanding buffer — NOT merge damage

This one predates every branch, and it is the larger half.

An interrupt endpoint only transfers when a buffer is posted for it.
`ptr_requeue()` posted exactly one, and only from inside the completion
handler — so the device got **one service opportunity per frame** regardless of
how often the bus offered one. Its own comment said so plainly:

> One outstanding at a time is enough for a keyboard and keeps the ring easy to
> reason about.

Which is true. A keyboard is an event source and 100 Hz is ample. A pointer is
a *rate* source, and the same choice caps it at the compositor's frame rate.
Eight reports offered in one frame, one delivered, seven service intervals
starved.

### 3c. `xhci_ptr_ready()` asked where `xhci_ptr_abs()` was meant

`pump_mouse()` decided "this pointer is absolute" from *"a USB pointer
exists"*. A usb-tablet is absolute, so it looked right — and every probe
attaches a tablet, so nothing disagreed. A usb-mouse took the same branch:
**every USB mouse ran at exactly 1:1 with no acceleration, and Settings'
pointer-speed slider moved a number that reached nothing.**

This is `desktop/feel-and-control`'s entire feature, silently off for the one
device `try.sh` attaches.

### 3d. A transfer wait that took anyone's completion — the worst of the four

Found by an adversarial pass, not by the original suspect list, and it is more
severe than the other three.

`event_wait(want, ...)` returned the first event of the requested *type*. Every
real caller passed `want == TRB_TRANSFER_EVENT` — `xhci_control_in()` (about
forty times during enumeration) and `bulk_xfer()` (every USB-storage read, and
`try.sh` attaches two `usb-storage` devices). So a HID completion landing
inside either window was consumed **as if it were that transfer's own**.

The function even carried the right lesson in a comment —

> A keypress that lands while we are waiting on a command must NOT be thrown
> away: dropping it also drops our obligation to post another buffer, and the
> keyboard goes silent forever.

— and then made exactly that mistake, because the `t == want` test ran *first*
and matched the keypress. The dispatch below it was unreachable for every real
caller. This is the same trap `cmd_wait()` a few lines above already documents
("matching on 'the next command completion' is a trap"), one layer down.

Reproduced against `HEAD`:

```
  it does not mistake the pointer's completion for its own  FAIL  (got 32, wanted 0)
  ...the pointer's report was decoded anyway                FAIL  (got 0, wanted 1)
  ...and its endpoint was re-armed, so it still works       FAIL
```

The endpoint is never re-armed, so **the pointer dies outright** — not slows,
dies — and stays dead. Move the mouse while reading the USB stick and that is
the end of the pointer until reboot. `xfer_wait(slot, dci, ...)` now matches on
the slot and endpoint the Transfer Event names, and hands everything else to
the dispatcher that owns it.

## 4. The fix

Three changes, each with a check that fails without it.

**One owner for the ring.** `xhci_poll(max)` is now the only function that
takes events off it. It dispatches each to the endpoint the event names and
drains generously rather than exactly once. `input_poll()` calls it once, at
the top, before anything reads decoded state. `xhci_ptr_poll()` and
`xhci_kbd_poll()` survive as thin wrappers because the zl builtins `mouse_x`
and `usb_poll` call them from outside `input.c`; they are no longer owners.
`xhci_key_event()` no longer polls at all.

**A pipeline for the pointer, and only the pointer.** `PTR_NBUF = 8` buffers,
each named by its own TRB, so the bus sets the report rate instead of the frame
rate. The keyboard is deliberately left at one outstanding buffer: 100 Hz is
ample for keys, and its requeue is the fragile path that keeps the endpoint
alive.

Each completion is decoded from **the buffer its own TRB named**, derived from
the TRB address the Transfer Event carries. Posting several TRBs at one shared
buffer would be worse than posting one — the controller would overwrite the
same bytes N times and N-1 deltas would be lost before anything read them.

**A mouse is treated as a mouse.** `pump_mouse()` now branches on
`xhci_ptr_abs()`. The relative path takes a read-and-clear raw delta
(`xhci_ptr_take_dx/dy`) rather than differencing two positions — it has to,
because `ptr_decode()` clamps its own `ptr_x` to the screen, so consecutive
positions stop differing at an edge and below 1x the accelerated pointer could
never reach that edge at all. `ptr_x`/`ptr_y` keep their old 1:1 clamped
meaning for `kernel.zl`'s `mouse_x()` builtin.

After:

```
  reports the driver takes per frame: 8
  pointer travel available in one frame: 799 px
  -> at 100 Hz, a ceiling of 79,900 px/s
```

An 8x rise in the ceiling, and no starved service intervals.

### A side effect worth naming

The acceleration curve's input is one frame's displacement — a velocity. While
only one report per frame got through, the curve was fed a *fraction* of the
hand's real velocity and under-accelerated accordingly. Feeding it every report
is not merely more travel; it is the first time the input to that curve has
been the number it is documented to take.

## 5. The bug the fix introduced, and what caught it

The first version put the Link TRB at `PTR_NBUF`, making the pointer's ring
exactly as long as its buffer count. It worked for exactly eight reports and
then the endpoint went silent **for good**.

The cycle bit is why. A Link TRB is handed to the controller carrying the
producer's current cycle, and the controller toggles its own when it follows
it. With the pipeline as long as the ring, the producer laps the consumer: we
rewrote the Link with the *next* cycle while the controller was still one TRB
short of reading it, so it arrived at a Link that no longer matched, concluded
the ring was empty, and stopped.

`PTR_RING_USE = 32` gives 24 slots of slack, and must be a multiple of
`PTR_NBUF` so the index-to-buffer mapping stays collision-free across the wrap.
Both conditions are now `_Static_assert`s.

This is the exact hazard the driver's own comments warn about — "dropping a
transfer event also drops the requeue, and that endpoint then goes silent for
good" — and it was caught by an assertion, in a second, having never booted.
A screenshot could not have found it; nor could a probe that moves the pointer
once.

## 6. The secondary suspects

| # | Suspect | Verdict |
|---|---|---|
| 1 | tablet decided from `ptr_ready`, not `ptr_abs` | **CONFIRMED** — fixed, see 3c |
| 2 | three clamps disagreeing by one | **NOT A DEFECT** — but see below |
| 3 | `EV_MOUSE` coalescing drops a button | **PRE-EXISTING**, not made worse |

### The clamps agree

All three name the same last valid pixel, and `fb_setup()` is the only writer
of both setters:

| where | stores | last valid pixel |
|---|---|---|
| `idt.c` `ptr_lim_x` | `w` | `lim_x - 1` |
| `input.c` `bnd_w` | `w - 1` | `bnd_w` |
| `xhci.c` `ptr_decode` | — | `console_pxw() - 1` |

`idt.c` stores `w` and subtracts at use; `input.c` stores `w-1` and uses it
directly. Different conventions, same answer.

### ...but the prompt quoted a dead variable

`kernel/idt.c:150` declares

```c
static volatile int mouse_max_x = 2000, mouse_max_y = 1500;
```

with a comment describing it as the live clamp. **Nothing reads or writes it.**
The live pair is `ptr_lim_x`/`ptr_lim_y` at `idt.c:37`, used at `idt.c:225`.
This is exactly phase 2's "two implementations of one thing under different
names" class, sitting inside the pointer path, and it is why the prompt's own
description of the clamps was wrong. Left in place deliberately — removing dead
code belongs to the phase 2 sweep, not to a fix whose gate is a human's hand.

## 6a. Found on the way, verified, deliberately NOT fixed here

Each of these was read in the source and confirmed. None is fixed in this
change, because phase 1's gate is a human's hand on a mouse and widening the
diff makes that verdict harder to attribute. They are phase 2's, and they are
written down so phase 2 does not have to find them again.

**`idt.c`'s `mouse_max_x`/`mouse_max_y` are dead.** Declared at `idt.c:150`
with a comment describing them as the live clamp; read and written by nothing.
The live pair is `ptr_lim_x`/`ptr_lim_y` at `idt.c:37`. Two names for one
thing — phase 2's class 2, inside the pointer path.

**The telescoping clamp is live on the PS/2 path.** `idt.c:225-229` clamps the
raw ISR position, and `input.c` derives its delta by differencing two
*already-clamped* positions. At any pointer speed below 100% with acceleration
off, the accelerated pointer moves less than the raw one, so the raw position
pins against the edge, the difference goes to zero, and **the accelerated
pointer can never reach that edge**. It does not bite by default only because
`settings.c:95` ships `speed = 100`. This is the same hazard the USB path now
avoids by publishing a raw delta; the PS/2 path needs the same treatment.

**A failed mode switch leaves all three clamps stale.** `console.c:230-232`
returns 0 if `bga_framebuffer()` comes back NULL — *after* `bga_set_mode()` has
already reprogrammed the geometry. `fb_setup()` is never reached, so the clamps
still describe the previous mode.

**A USB mouse's scroll wheel is dropped.** The boot-mouse report is
`[buttons, dx, dy, wheel]` and `ptr_decode()`'s relative branch never reads
`r[3]`. Not fixed, and not merely for scope: `EV_WHEEL` currently reaches
`route_wheel()` → `hook_event()` and **no app handles it**, for either device.
Wiring USB into it would feed an event nothing consumes. Missing feature, not
regression.

**xHCI runs with interrupts disabled.** `XRT_IMAN` and `XRT_IMOD` are defined
at `xhci.c:393-394` and never written, and the file contains no ISR. That is a
deliberate, working design — but it is *why* the frame clock gates the whole
input stack, and it is the thing to revisit if the pipeline ever proves too
shallow.

**`probe-mouse.py` tests neither pointer.** It defaults to no tablet and no
usb-mouse, so it drives a PS/2-only machine, and its header still claims
"xhci.c contains no mouse code at all".

## 7. What is checked, and where

`kernel/hosttest/xhcitest.c`, wired into `hosttest/build.sh` and therefore into
`gates/land-gate.sh`. It is the first coverage the relative pointer path has
ever had.

The same file, built against `HEAD`'s `xhci.c` and `input.c` with a two-line
shim for the names the pre-fix driver does not have, **fails 8 assertions**.
Against the fix it passes 22 of 22. A test that did not fail first would not be
evidence of anything.

## 8. What this does NOT establish

Said plainly, because the temptation is to call this done:

- **Nobody has moved a real mouse yet.** The ceiling went from 10,000 px/s to
  79,900. Whether that is what "jumpy" was made of is a claim about a hand, and
  a bench cannot settle it.
- **The fake controller is a model.** It implements the cycle bit, the Link
  TRB, per-TRB buffers and the 127-count device clamp, and it is faithful to
  the QEMU source for the parts that were checked. Real silicon on the ThinkPad
  is not in this picture at all, and `PTR_NBUF = 8` outstanding TRBs is a
  bigger change there than in QEMU.
- **The frame rate is inferred.** `wm_frame()` is gated by `idt_ticks()` at
  100 Hz, and `hosttest/wmbench` measures a composite at ~2.6 ms on this host —
  so frames should be tick-bound rather than paint-bound. That has not been
  measured *inside* the guest.
- **The 3,000–10,000 px/s figure for a hand is a reference point, not a
  measurement taken on this machine.**

### So measure it in the guest

`ptr_reports`, `ptr_events`, `ptr_lastcc` and `kbd_events` were **already**
registered as zl builtins (`freestanding/runtime_kernel.c:1508-1511`) — they
simply had no caller anywhere in `kernel.zl`. An earlier draft of this document
said they reached no builtin; that was wrong.

So the `=` input-events demo now prints the pointer rate once a second:

```
    pointer 118 reports/s  ->  ceiling 14986 px/s   events 118
```

That is the number this whole document is about, read from the running kernel
rather than from a bench. `check-zl-calls.sh`'s count of registered builtins
with no caller dropped from 150 to 148, which is the cheap confirmation that
the two are now actually wired.

**When you run `./try.sh`, press `=` and move the mouse.** Before the fix that
line should read about one report per frame; after it, the endpoint's own rate.
