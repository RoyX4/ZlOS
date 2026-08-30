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
`main` (see `docs/evidence/MERGE-EVIDENCE.md`). The pointer is the first thing a person
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

## Phase 1c — the desktop looks worse than v10, cause NOT found

> **Restored 2026-08-19.** This section and both of its PNGs were deleted by
> `6bb8086` ("the three defects the documentation audit found in my own merge"),
> which does not mention the deletion in its message — 235 lines of removals rode
> along with a three-item fix. `LOOK-AND-SPEED-PROMPT.md` then cited "§1c" and
> "`docs/shots/`" as if both were present. They were not. Restored byte-identical
> from `ff27d57` (`sha256 8c73b422…` on `before-merge-help.png`, matched).

> ## ANSWERED 2026-08-19 — read this before the rest of the section
>
> **The cause is `term_draw()`. The shell scrollback changed font.** On the
> branch that drew the good shot it uses the PROPORTIONAL `fb_text_prop`
> (per-glyph advance from `prop16_adv[]`); on `main` it uses the MONOSPACE
> `fb_text_aa`, which advances by a fixed `cell_w` = `GLYPH_W * 2` = **16 px** at
> 1920 wide. Changed by `663a110`.
>
> | | before | after |
> |---|---|---|
> | `term.c` line renderer | `fb_text_prop` | `fb_text_aa` |
> | advance | per glyph, avg **14.97 px** | fixed **16 px** |
> | "mouse ..." row, ink span | 707 px | **814 px** (+15.1%) |
> | "poke ..." row, ink span | 892 px | **1021 px** (+14.5%) |
> | longest row | 1220 px | **1315 px** |
> | shell client right edge | 1318 px | 1318 px — *unchanged* |
>
> So the window did not get narrower; **the text got 15% wider inside it**, and
> two rows now run into the frame and are cut mid-glyph. Measured off the two
> committed PNGs, not eyeballed.
>
> **A SECOND parameter, same effect, different commit.** `fb_text_prop_h()` went
> from `prop_big() ? 32 : 16` to `prop_cell(TEXT_BODY)` = `role_base[TEXT_BODY]
> * ui_scale` = 12 × 2 = **24** at ui 2 (`a8bb1b4`). That is a **25% cut** to
> every piece of chrome drawn with it — window titles, the top bar, About, the
> System Monitor labels. Measured in the PNGs: "System Monitor" title ink
> 28 px tall → 20, "Activities" 20 → 16. If "worse" also meant "smaller", this
> is that half.
>
> **WHY THE ORIGINAL SEARCH FOUND NOTHING — the premise below is wrong.**
> `before-merge-help.png` was **not** taken on `desktop/overnight-compositor`.
> Overnight already contains `663a110`, so a shot from it would look like the
> *after*. Three independent fingerprints agree, each a grep count: the shot's
> terminal is proportional and `fb_text_prop(x, ty, line` appears **0** times in
> overnight's `term.c`; the shot's tray reads `state: compositor  up:`, which
> `663a110` is the commit that *replaced*; and the shot's About window says
> "press h for the app list", which is 0 occurrences on overnight and on main.
> The shot comes from the `apps-in-windows` lineage. **Every diff run below was
> main-vs-overnight and was therefore guaranteed to come back empty.** That is
> why "the drawing code is intact" was both true and useless.
>
> **The decision, and it is not "revert it".** A terminal is monospace — that is
> what a terminal is, the northstar's own spec says the terminal body is mono,
> and `663a110`'s reason is sound: `help`, the PCI dump and the CPUID report are
> space-aligned tables that only line up in a fixed advance. Reverting would
> restore the v10 look and re-break all three. Measured cost of keeping it, from
> `fbbench`: 40 lines of mono AA text is **5.147 ms** against **3.584 ms**
> proportional, so mono is also ~44% dearer — noted, and still not enough to
> make a proportional terminal right.
>
> **What is left open is the CLIPPING, which is a real defect either way** and is
> now `DECISIONS.md` open item G: at 1920x1200 the shell client is 1236 px = 77
> columns of 16 px, and the longest `help` line needs ~82. Fix is width or
> wrapping, not font.
>
> > **CLOSED 2026-08-19 by `DECISIONS.md` #35 — wrapping, and one correction to
> > the count above.** It is **75** columns, not 77. 1236/16 = 77 is the CLIENT
> > rect; `kernel.zl:2934` insets it by the toolkit's padding before `term_draw`
> > sees it — `term_draw(ax + 8*u, ay + 6*u, aw - 16*u, ah - 12*u, ...)` at
> > `u = 2` — so the terminal gets 1204 px. The longest `help` line is exactly
> > **82** characters (`kernel.zl:627`, the i2c row), so seven characters went
> > past the edge, not five.
> >
> > Width was the other option this box allowed and it is the wrong one: the
> > window has a resize grip and `mode` changes the screen under it, so a wider
> > boot window fixes one size and no other. `term_draw` wraps, walking the
> > scrollback backwards by **display** rows rather than stored ones so the
> > newest line still lands against the prompt. The typed line had the same
> > defect and scrolls sideways instead, because the prompt owns one row.
> > Gated by `hosttest/termwrap`, watched going red.

Zac's report, and he is right that the two screens differ. Compare
`docs/shots/before-merge-help.png` (~~desktop/overnight-compositor~~ **the
apps-in-windows lineage — see the box above**, "v10-wall") with
`docs/shots/after-merge-help.png` (merged main, same `help` screen, same
1920x1200). The merged one reads as coarser, and its longest lines are clipped
at the window edge where the v10 shot fits them.

**Everything from here down is the ORIGINAL, UNANSWERED text, kept for its
ruled-out list. Its central assumption - that the good shot came from
overnight - is false, which is what made the search below unwinnable.**

Ruled out, each by a command whose output was read:

- **No visual function was lost.** Every function `desktop/overnight-compositor`
  defines in `fb.c` exists in the merged `fb.c`. Diffed both directions on the
  full symbol list; the difference is empty.
- **`fb_text_aa` is byte-identical** to overnight's, as is `fb_glyph_aa`. It
  advances by a fixed `cell_w`, so the terminal is monospace BY DESIGN on both
  branches - the "v10 was proportional" reading is wrong.
- **`term.c` lost nothing relevant.** The only removal against overnight is the
  unknown-command block, deliberately replaced with exec-track's `term_say`
  version so the message reaches the serial log for `probe-term.py`.
- **`prop_big()` being unused** is pre-existing: it is unused on overnight too.
  The compiler warning is not a symptom of the merge.

So the drawing code is intact. That points at **state or parameters**, not
deleted code - scale, cell size, padding, theme, or the console geometry - which
narrows it to `kernel.zl`, where fifteen hunks were resolved by hand, and to
`fb_setup`'s sizing.

Concrete places to start:

1. `fn ui()` - the known landmine. It is `ui_scale()` on overnight and
   `cell_w() / 8` on apps-in-windows, and taking apps's version compiles, links,
   boots and silently restores the "everything is tiny at 4K" regression. The
   merge kept overnight's. **Verify that is still true** and that nothing else
   redefines `ui`.
2. `cell_w = (width >= 1400) ? GLYPH_W * 2 : GLYPH_W` in `fb_setup` - the boot
   log says `cell 16x32 ui 2x`. Check what v10 reported at the same resolution;
   if it differed, the console geometry moved and everything downstream with it.
3. The window's client rect: the merged boot prints
   `compositor: 4 windows, shell client 82,160 1236x834`. Compare against a v10
   boot. A narrower client with the same cell means fewer columns and clipping.
4. `docs/shots/` has both PNGs. Measure rather than eyeball - my own pixel
   comparison was inconclusive because the two shots show different content at
   the same row, which is exactly the mistake to avoid repeating.

A good outcome here is either a named parameter that changed and a fix, or a
measured statement that the two are equivalent and the difference is the content
on screen. Both are useful; "looks fine to me" is not.

---

## Phase 2 — the full audit

Only after phase 1 is confirmed.

Eleven branches were merged in one night. The merge found and fixed a great deal
(see `docs/evidence/MERGE-EVIDENCE.md` §Outcome), but the failure modes it kept hitting
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

`kernel/src/arch/x86/memmap.h` is the single source of truth now and has `_Static_assert`s,
but `check-memmap.sh` iterates a **hardcoded nine-name list** and does not
discover new constants - it could not see `DISK_SCRATCH`. Rewrite it to sweep
every `^[A-Z_]+ *= *0x` in `kernel.zl` and every `#define HI_*` in `memmap.h`,
and to fail on any overlap.

Known live: `intel.c`'s `edid_buf` is still a hardcoded `0x0C980000`, which is
inside `HI_BLUR`. `intel_set_edid_buffer()` exists and is the one-line fix.

### 4. Gates that cannot fail

Two were found by using them: a `land-gate.sh` that reported the exit status of
`tail`, and a `verify-clock.sh` that had been committed without ever passing.
Check every gate actually fails on a deliberately broken tree. A gate that has
never gone red has not been tested.

### 5. The known-broken list

- `key()` is called in `kernel.zl` on the panel-handover path and is defined
  **nowhere**, on every branch, since `b55f3f9`. Pick the right builtin
  (`in_key`, `term_key`, `usb_key`) and remove it from `check-zl-calls.sh`'s
  known list.
- `font_big.c` and `icons_rgb.c` are referenced by nothing at all.
- `ci/gates-and-agent-brief` is 9 commits unmerged, and carries the fix that
  makes `build.sh` build `zlfmt`.
- 55 files exist only in `refs/wip/*` snapshots and on no branch - the whole
  `learn/` course, `crypto.c`, `src/tools/zlfmt.c`, 7 probes, 12 docs.

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
- Write findings into `docs/evidence/MERGE-EVIDENCE.md` or a sibling, not into a chat
  reply. The chat is gone next session; the repo is not.
