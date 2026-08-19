> **AUDITED 2026-08-19 · LIVE WORK, NOT HISTORY.** This file was deliberately NOT audited
> against the merged tree — it is the current task, not a record of an old one.
> Its two open items (the xHCI event-ring double drain, and the dock readout's
> digit debris) are carried into
> [`docs/STATE-OF-THE-PROJECT.md`](../../docs/STATE-OF-THE-PROJECT.md) §§3.1–3.2 and 3.6 as OPEN, with the
> diagnosis here treated as settled and not re-derived. Work from THIS file;
> use that one for everything around it.

# POINTER-PROMPT — fix the pointer, then audit the whole tree

Two phases, strictly in order. **Do not start phase 2 until phase 1 is
confirmed by a human looking at a live VM**, not by a gate going green.

Written 2026-08-19, immediately after eleven parallel tracks were merged into
`main` (see `docs/MERGE-EVIDENCE.md`). The pointer is the first thing a person
touched afterwards and it is visibly wrong: moving the mouse in `./try.sh`
produces jumpy, laggy, unpredictable motion.

---

## Phase 1 — the pointer

### The lead suspect, already measured

**Two independent drainers of one xHCI event ring.**

```
xhci.c:1775   xhci_ptr_poll()   -> event_poll(0, &status, &ctrl, 1)
xhci.c:1789   xhci_kbd_poll()   -> event_poll(0, &status, &ctrl, 1)
```

Both take events off the same ring and dispatch whatever they get. Per frame,
`input.c` calls them at very different rates:

```
input.c:527   xhci_ptr_poll()    once, inside pump_mouse()
input.c:669   xhci_key_event()   in a loop, up to 16 times
```

So a pointer report can be consumed by the keyboard loop, and `pump_mouse()` -
which runs FIRST - can only ever advance the pointer by one report per frame no
matter how many the device sent. Move the mouse quickly and reports arrive far
faster than 100 Hz; the surplus is drained by the keyboard loop *after* the
position was sampled, so the pointer lags, then jumps when the next frame reads
an accumulated position.

**This is merge damage, and neither branch could have had it.**
`desktop/overnight-compositor` had `xhci_ptr_poll()` and no `xhci_key_event()`
(it used `xhci_key()`, which returned decoded characters).
`claude/ecstatic-lewin-f617bb` added `xhci_key_event()` with its own poll,
because arrow keys have no character. The merge kept both and nobody owns the
ring.

### What the fix probably looks like

ONE drainer. A single `xhci_poll()` that takes events off the ring, dispatches
each to the endpoint it belongs to (pointer reports update `ptr_*`, keyboard
reports push onto `kevq`), and is called once per frame from `input_poll()`.
`xhci_ptr_poll()` and `xhci_key_event()` then become *readers of already-decoded
state*, not pollers. Drain generously - bounded, but far more than one - because
a fast hand produces many reports per frame and the surplus must not sit in the
ring until the next one.

Do not guess at this. Read `event_poll()` (xhci.c:551), `xhci_kbd_poll()`,
`ptr_decode()` (xhci.c:1324) and `pump_mouse()` (input.c:~520) first, and check
whether the dispatch already routes by endpoint - if it does, the fix is mostly
deletion.

### Three more things to check, all real, none confirmed as the cause

1. **`try.sh` attaches a `usb-mouse`, not a `usb-tablet`** - so this is the
   RELATIVE path. `xhci.c:1101` has `ptr_abs` and `ptr_decode()` accumulates
   relative deltas into `ptr_x/ptr_y` and clamps them to the live screen, so by
   the time `input.c` sees it, it *is* an absolute position. That is correct.
   But `input.c`'s `pump_mouse()` decides "tablet" from `xhci_ptr_ready()`, not
   from `xhci_ptr_abs()`, and takes the absolute branch either way - which
   silently disables `desktop/feel-and-control`'s acceleration and speed
   settings for every USB mouse. Settings' pointer-speed slider does nothing.
   Decide deliberately: either accelerate the relative case, or state in a
   comment that USB pointers are 1:1 by design and make Settings say so.

2. **Three clamps in series**, set by two different callers, and they disagree
   by one:
   ```
   idt.c:150     mouse_max_x = 2000, mouse_max_y = 1500   (PS/2)
   input.c:450   bnd_w = 2000, bnd_h = 1500               (accelerated)
   xhci.c        clamps to console_pxw()-1 / console_pxh()-1
   fb.c:726-727  idt_set_pointer_bounds(w, h)  and  input_set_bounds(w, h)
   input.c:454   bnd_w = w - 1     <- minus one
   idt.c         mouse_max_x = w - 1  <- check this is still true after the merge
   ```
   Confirm all three agree on the last valid pixel, and that `fb_setup()` is the
   only writer.

3. **`EV_MOUSE` coalescing drops the button when the position repeats.**
   `input.c` returns early when `px_x`, `px_y` and `b` all match the last
   published values. That is right. But check the ordering against
   `wm.c`'s `route_mouse` - a press and release inside one frame is already
   gone at `idt.c` (it keeps only the current mask), which `input.c`'s own
   comment admits. With reports now backing up in the ring, that window is
   wider than it was.

### The gate for phase 1

`probe-mouse.py`, `probe-mouse-sync.py`, `probe-drag.py` and `probe-dock.py`
exist and drive the pointer headlessly. They are necessary and **not
sufficient** - every one of them passed while the pointer was visibly broken,
because they move the pointer in single large jumps and assert on the result,
which is exactly the motion pattern that hides a drain-rate bug.

Add one that reproduces the fault: **many small moves in quick succession**,
asserting that the reported position tracks the sum of the deltas rather than
lagging behind it. If that test does not fail before the fix, it is not testing
the bug.

Then, and only then: `./try.sh`, move the mouse by hand, and have Zac confirm.
**A human says it is fixed, or it is not fixed.**

---

## Phase 1b — the dock readout leaves debris (confirmed, visual)

The tray reads `frame 0  us peak 0  )08  up 1`. The `)08` is not a value, it is
the tail of a previous, wider number.

`kernel.zl` draws the status numbers at FIXED x-offsets with **no background
clear**:

```
label    (tray +   0, ..., "frame")
label_num(tray +  44, ..., wm_us())        <- nothing clears behind this
label    (tray +  92, ..., "us   peak")
label_num(tray + 152, ..., wm_peak())      <- nor this
label    (tray + 210, ..., "up")
label_num(tray + 232, ..., ticks() / 100)
```

Only `draw_clock()` clears, and only the `up` region
(`grad_rgb(ctray + 226 * ui(), ...)`, 70 px wide). So when `wm_peak()` shrinks -
say from a six-digit boot peak to `0` - the digits that are no longer drawn are
never erased.

This is a REGRESSION and its provenance is exact: the tray used to read
`state: compositor`, a constant string that cannot shrink.
`desktop/apps-in-windows` replaced it with live numbers, correctly citing
desktop-TODO 0h ("a number nobody can see is a number nobody checks"), and did
not add the clear that live numbers need.

Fix: clear each number's cell before drawing it, the way `draw_clock` already
does for `up`. Better, since the font is proportional and the offsets are fixed
literals: clear the whole tray strip once per redraw and draw the row into it.

Gate: `probe-frame.py` already reads the frame timer over serial and would not
have seen this - it greps the log, not the screen. Photograph the tray with
`probe-shot.py --crop` at two different peak values and assert the strip is
identical apart from the digits that changed.

---

## Phase 2 — the full audit

Only after phase 1 is confirmed.

Eleven branches were merged in one night. The merge found and fixed a great deal
(see `docs/MERGE-EVIDENCE.md` §Outcome), but the failure modes it kept hitting
are the ones a clean merge cannot show, and there is no reason to believe they
are all found. Audit for these specifically, in this order:

### 1. Deletions that kept their callers

The class that cost the most during the merge. A merge takes one side's removal
and the other side's use; the result compiles or doesn't depending on luck.
Already found and fixed: `fb.c`'s 91-line band-parallel block, `editor_key`'s
body, `verify-raw.sh`'s `OUT`, the whole static-desktop mouse loop.

Sweep: every function called in `kernel/*.c` and `freestanding/*.c` that nothing
defines; every zl call site that resolves to nothing (`./check-zl-calls.sh`
already does the zl half and is in the gate).

### 2. Two implementations of one thing, under different names

Eight app-id collisions were found. Two snakes. Two resize grips. Two frame
timers. Two names for the console mute. `wm_focus` meaning opposite things on
two branches. Assume more remain.

Sweep: for each pair of files that both define a symbol with the same *purpose*
but a different *name*, decide which survives. `nm -g --defined-only` on the
objects will not find these - the names differ, which is the entire problem.
Compare per-file symbol lists across `prelanding/*` tags.

### 3. Fixed addresses

`kernel/memmap.h` is the single source of truth now and has `_Static_assert`s,
but `check-memmap.sh` used to iterate a **hardcoded nine-name list** and could
not see `DISK_SCRATCH`. It now parses `DISK_SCRATCH`, `DISK_SCRATCH_SIZE`,
`PAINT_BUF` and `PAINT_MAX`. A full sweep of every `^[A-Z_]+ *= *0x` is still
the better detector.

`intel.c`'s `edid_buf` default is `HI_EDID` (`0x0BFF0000`, top of the HID
window). Host harnesses still override via `intel_set_edid_buffer()`.

### 4. Gates that cannot fail

Two were found by using them: a `land-gate.sh` that reported the exit status of
`tail`, and a `verify-clock.sh` that had been committed without ever passing.
Check every gate actually fails on a deliberately broken tree. A gate that has
never gone red has not been tested.

### 5. The known-broken list

- `key()` on the panel-handover path is gone: it waits on `in_char()` like
  every other blocking read. `check-zl-calls.sh`'s known-unresolved list is
  empty; the next hole is a FAIL.
- `font_big.c` and `icons_rgb.c` are referenced by nothing at all.
- `ci/gates-and-agent-brief` landed on `main` (`2550091`).
- 55 files exist only in `refs/wip/*` snapshots and on no branch - the whole
  `learn/` course, `crypto.c`, `zlfmt.c`, 7 probes, 12 docs.

---

## Rules for both phases

- **Root cause, not symptom.** The pointer bug is not "add a clamp"; it is
  "decide who owns the event ring".
- **One runnable check per fix.** A fix with no test that failed before it is a
  guess.
- Gates run backgrounded and sequentially. Check `/proc/loadavg` first; this box
  has 8 cores and QEMU is under TCG.
- Every landing is revertable at its `premerge/*` tag. Nothing has been
  force-pushed and nothing should be.
- Write findings into `docs/MERGE-EVIDENCE.md` or a sibling, not into a chat
  reply. The chat is gone next session; the repo is not.
