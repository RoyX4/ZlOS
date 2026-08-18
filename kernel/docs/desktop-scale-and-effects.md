# Why the desktop looked small, and where the effects actually went

Two questions asked on 2026-08-18, both with the same shape: something was
*built* and something else was *observed*, and the gap between them was not
where anyone was looking.

---

## 1. "Why does everything look so small?"

Because the UI scale stopped growing while the screen kept growing.

The desktop's layout is written in **design units**, and the original layout
was drawn for an 800-unit-wide screen. `ui()` multiplies those units. It was
defined as `cell_w() / 8` — the console font cell divided by the base glyph
width — and the console has exactly **two** atlases, 8×16 and 16×32. So `ui()`
could only ever be 1 or 2.

What that produced, in units of layout space available for an 800-unit design:

| screen | `ui()` | design space | what you see |
|---|---|---|---|
| 800×600 | 1 | 800 units | correct, by construction |
| 1280×800 | 1 | 1280 units | 1.6× too much room |
| 1920×1200 | 2 | 960 units | about right |
| **2560×1440** | **2** | **1280 units** | 1.6× too much room |
| **3840×2160** | **2** | **1920 units** | **2.4× too much room** |

The bigger the panel, the smaller the desktop looked on it. On the ThinkPad's
own 2560×1440 that is the difference between a desktop and a postage stamp in
the corner of one.

### The fix: they were two questions welded into one number

The **console cell** must come from a real font atlas — a resampled console
would be both slow and soft, and it is drawn per character per frame. It stays
8×16 or 16×32.

The **layout scale** has no such constraint. It is now derived from the screen:

```c
ui_scale = (width + 400) / 800;      /* rounded, clamped 1..4 */
```

Rounded rather than truncated, because 1200 wide is much closer to 1.5 designs
than to 1 and truncation would leave it at 1 with half the screen empty.
Capped at 4 because past that a 24 px title becomes 96 px and the layout runs
out of design units before it runs out of screen.

Verified against the shipping `fb.c` through `hosttest/fbbench`, which prints
the mode line the kernel prints:

```
fb: 1920x1200x32 cell 16x32 ui 2x, back ON  ( 9000 KiB/mode)
fb: 2560x1440x32 cell 16x32 ui 3x, back ON  (14400 KiB/mode)
fb: 3840x2160x32 cell 16x32 ui 4x, back ON  (32400 KiB/mode)
```

Previously every one of those said `ui 2x`.

### What had to follow

Proportional text is picked by **role** — caption, body, title — and a role's
pixel height is `base × ui_scale`, so at ui 3 and 4 it lands between the three
generated atlases (16/24/32) and the glyph is **resampled** through
`blend_cov_scaled_s`. The advance is scaled with it; scaling the glyph and not
the advance measures one width and draws another, which shows up as text that
overflows the box it was centred in.

`blend_cov_scaled` had to grow a strided form. A font atlas row is `stride`
wide while the ink being resampled is narrower, and passing the ink width as
the stride steps a fraction into the next glyph's row per line — which reads as
text smeared to the right.

---

## 2. "What about the effects — animations, blur?"

They were all built, measured and asserted. **Three of them had no caller.**

That is the exact hazard `HANDOFF.md` records for `intel.c` — *"the code
exists" is not "the code works" — check for an actual caller* — and it had
reappeared one file over.

| effect | was | is |
|---|---|---|
| `fb_blur_cache` | built, 7.37 ms cold / 0.18 ms cached, asserted correct — **nothing called it** | behind the start menu and behind the dock |
| the animation timeline | five kinds, eight slots, 12 assertions — **nothing triggered it**, and `wm_open` used a *separate* legacy counter | `wm_open` starts `ANIM_OPEN` through the timeline; an unknown command pulses the shell window |
| `fb_fill_blend` | used only inside `fb_rrect_blend` and the wallpaper glows | the dock tint, the menu panel, the menu's hover wash |
| radial + conic gradients | in the wallpaper | unchanged — these did have a caller |

### wm.c had two animation systems and one of them never ran

`wm_open` set `wins[i].anim = ANIM_FRAMES` and `anim_pct` read that counter;
the timeline was a second mechanism beside it with no caller in zl at all. Now
`wm_open` starts an `ANIM_OPEN` and `anim_pct` reads the timeline, so the open
scale and every other kind share one code path — a bug in one is a bug you can
actually see.

The change immediately failed an assertion, which is the point of having them:
`wm_open` now consumes an animation slot, so a test that opened eight windows
and started eight fades in the same loop was measuring "slots left after the
opens" rather than the ceiling. The test now lets the opens settle first.

### The blur rules, which are design decisions rather than limits

- **Behind the start menu** — blurred *before the window exists*. `wm.c` draws
  the chrome and then calls `app_draw`, so by the time the menu could blur its
  own client rect the panel fill is already sitting in it. The moment the
  screen still holds only what is underneath is inside `open_menu()`, before
  `wm_open`.
- **Behind the dock** — taken once, at boot, right after the wallpaper is
  baked. The dock is furniture: drawn before every window, never on top of one,
  so what is behind it is always the wallpaper and the wallpaper never changes.
  That makes it the cheapest possible backdrop blur.
- **A blurred panel must not be draggable.** 7.37 ms to blur against a 16.67 ms
  frame. Neither of the two blurred surfaces can be dragged, and that is why.

---

## 3. The dock was a picture of a dock

`desk_draw` had painted a dock, a start button and a tray since the compositor
first booted, and every one of them was decoration. The dock is not a window,
so `wm_at()` found nothing under the pointer and `route_mouse` returned. **No
gate could have caught it** — every gate in this repo types, and a dock that
does nothing photographs identically to one that does something.

`wm.c` now hands pointer events that hit no window to a desktop hook — *every*
event, not just presses, because a dock with no hover state reads as a picture
of a dock and knowing where the pointer is, is the whole of making it feel like
a control.

What that unlocked:

- **hover / press / rest** states on every tile. Press draws the tile 2 units
  smaller and 1 down, which is `zpress` in the prototype.
- **an accent bar under a tile whose app is open** — the difference between a
  launcher and a taskbar.
- **click-to-raise**: a tile whose window already exists raises it rather than
  opening a second copy. "One window per app" is policy, and `wm.c` gives no
  such rule for free.
- **the start menu as a `WF_MODAL` window.** That flag had a setter in `wm.c`
  and no caller anywhere — built, and therefore looking done. The old menu drew
  straight onto the screen and saved the patch underneath with grab/stamp, the
  sticker technique C4 deleted. A modal takes every event and a click outside
  dismisses it, both already in `route_mouse`, and restoring what was
  underneath is a repaint rather than a saved bitmap.

### A separate row of window chips was the wrong design, and arithmetic said so

The first attempt put a taskbar chip per open window beside the launchers. Nine
launchers end at 1244 px on a 1920 screen and the tray starts at 1320: **76 px
of gap** for a row of 150-unit chips. The guard silently drew nothing, which is
the worst of both worlds.

So the **dock is the taskbar** — the shape every modern dock uses. The chip row
survives only for windows whose app has no tile, and fits as many as the space
between the last launcher and the tray allows, from a count derived from that
space rather than a ceiling that happens to be wrong on one screen size.

---

## 3.5 And the pointer never reached the compositor at all

The dock probe's first run answered **0 px changed** to all five questions —
not "a small difference", *zero*. That is a harness answer, not a feature
answer, and chasing it found something much larger than a dock.

zlOS drives **two** pointers: an absolute usb-tablet through `xhci.c` and a
relative PS/2 mouse through `idt.c`. The `mouse_x` builtin has preferred the
tablet since it was written — a tablet cannot drift, and on a UEFI laptop the
PS/2 emulation dies with ExitBootServices.

`input.c`'s `pump_mouse()` read `idt_mouse_x()` **and nothing else.**

While the shell owned the screen that was invisible: the shell called
`mouse_x()` directly and got the right answer, and the queue's mouse events
were nobody's input. The moment `wm_frame()` became the top of the system the
queue became the compositor's *only* source of pointer events — so on any
machine with a tablet attached, which is what QEMU gives and what `try.sh`
attaches, **no `EV_MOUSE` was ever pushed**. No dragging, no click-to-focus, no
close box, no dock, no menu. The entire pointer half of the desktop.

And the compositor's own assertions could not see it either: `wmtest` injects
pointer state through stubs that write `idt_mouse_*`, so the harness had the
same bug as the kernel and the two agreed with each other.

This is the same shape as the serial bug earlier the same day, with a different
subject. **When a rewrite moves the top of the system, everything the old top
read DIRECTLY becomes something the new top must read through its own plumbing
— and the plumbing may only carry half of it.** Two instances makes it a rule:
enumerate what the old top read, and check each one reaches the new top.

### And every label was monospace

`wm.c`'s window titles were the only proportional text on screen. Everything
`kernel.zl` drew — the dock, the start menu, the tray, the About card, the
System Monitor — went through `text_aa`, which is DejaVu Sans **Mono**. Uniform
advance is the strongest "this is a terminal" signal there is, applied to four
things that are not terminals. They draw by ROLE now (caption / body / title,
regular or bold), which is also what makes them follow the UI scale.

---

## 4. What is gated now that was not

`probe-dock.py` — the dock as a *control*, five pixel-difference assertions:
hover changes a tile, press changes it again, a click opens a window, the start
button opens the menu, a click outside dismisses it. None of those is visible
to a gate that types, and none of them is distinguishable in a screenshot from
the version that does nothing.

---

## 4.5 "Why is it so slow" — and the instrument that should have existed first

`desk_draw(x, y, w, h)` is called **once per damage rectangle**, up to eight
times a frame. The wallpaper honoured the rectangle it was handed. The header
and the dock did not — both redrew in full, every call.

That was survivable while the dock was a flat gradient. Then §2 turned it into
a cached-blur blit plus a full-width translucent tint, and from `fbbench`'s own
numbers that is 245,760 px at 1.54 cyc/px plus 22.16 cyc/px — about 5.8M
cycles, **2.5 ms**. Eight damage rectangles is **20 ms of dock alone** against
a 16.67 ms budget, and seven of those eight usually do not touch the dock at
all.

**The fix is not an optimisation of the drawing.** It is noticing that
`desk_draw` was handed a rectangle and ignored it.

### The instrument came last, and desktop-TODO said it should come first

> *0h. Add a `tsc()` builtin and put frame time on screen. **Do this before any
> performance work.** Optimising without measurement is guessing.*

That has been in the task list since it was written. The v10 run did the
performance work first and the measurement never — so every claim about the
compositor's speed rested on per-primitive numbers from `fbbench`, which is not
the same thing as a frame.

`wm_frame()` times itself with the TSC now:

- **microseconds, not milliseconds.** A cheap frame is well under 1 ms and an
  integer millisecond reports every one of them as "0".
- **only frames that repaint are counted.** A frame that finds no damage
  returns almost immediately, and averaging those in reports a desktop at rest
  as infinitely fast.
- **the peak is reset once boot settles.** The first frame repaints the whole
  screen and bakes the wallpaper — three radial glows, two conic sweeps and a
  vignette at full resolution — a one-off costing over a tenth of a second that
  would otherwise sit in the peak for ever, hiding every real regression under
  it. It measured **139,864 µs** on the run that found this.
- **it is on screen**, in the tray, where "state: compositor" used to be — a
  label that had said the same thing on every boot since it was written.

`probe-frame.py` drives real interaction — hovering across the dock, dragging a
window twelve steps — and reads the numbers back out of the shell.

### And the terminal was drawing with the proportional font

Every column-aligned thing the shell prints — the help table, the PCI dump, the
CPUID report — is aligned with **spaces**, which only lines up when every
character is the same width. `term_draw` used `fb_text_prop`. The columns came
out ragged and it read as "the formatting is broken" rather than as "the font
is wrong", which is exactly why it survived.

A terminal is a grid. It uses the mono atlas, and `fb_text_prop` is for the
things that are not terminals — which is the split desktop-look.md item 4 draws
and this had inverted.

---

## 5. The pattern underneath all of it

Five separate things in `wm.c` and `fb.c` were complete, correct, gated — and
had **no caller anywhere**. Each looked finished from the inside and did
nothing from the outside:

| | had | was missing |
|---|---|---|
| `WF_MODAL` | a setter and a branch in `route_mouse` | anything that set it |
| `wm_resize()` | clamping, damage on both sides | anything that called it |
| `fb_blur_cache` | 7.37 ms measured, three correctness assertions | anything that blurred |
| the animation timeline | five kinds, eight slots, 12 assertions | anything that started one — and `wm_open` used a *separate* legacy counter |
| `MOD_SUPER` | tracked since `input.c` was written | any event to fire on — a modifier emits none |

This repo is written mechanism-first and gated hard, which is exactly what
makes the shape so easy to produce: a primitive arrives with tests, a
measurement and a comment explaining its design, and passes every check while
being unreachable. `HANDOFF.md` already names it for `intel.c` — *"the code
exists" is not "the code works" — check for an actual caller*. It had
reappeared in three more files.

**A primitive is not done when it passes its test. It is done when something
calls it and a gate covers the call.** And for visual work the assertion has to
check the *pixels*, not the state: `wm_anim_alpha()` reported a fade correctly
for hours while nothing drew one.

---

Scale and cost numbers: `desktop-polish-and-speed.md` · What the mockup asks
for: `desktop-northstar-feasibility.md` · The run that built the primitives:
`desktop-v10-plan.md` §8 · Task list: `desktop-TODO.md`

---

## 6. The measurement, and what it cost to get

`probe-frame.py`, 1920×1200, drives real interaction and reads the numbers back
out of the shell.

| | before the bake | after |
|---|---|---|
| hovering nine dock tiles, typical frame | **7,426 µs** | **1,258 µs** |

**5.9× on the case a person actually feels.** 7.4 ms to light up one chip is
not a subtle regression; it is most of a frame to change one rounded rectangle.

### The peaks in that run are not usable, and saying so matters

The host was at **load average 17.9** on a four-core box while these ran — a
second session on the same machine was compiling and running QEMU alongside
this one. TSC timing inside the guest measures wall-clock, so host contention
inflates it directly: the peak went *up* between the two runs (18,455 →
34,476 µs) while the typical frame fell 5.9×, which is contention, not code.

`CLAUDE.md` already records this failure mode for the boot gates — *"an A/B
against a clean worktree showed baseline and modified passing and failing
together, tracking host load"* — and it applies to the frame timer with more
force, because the frame timer has no golden value to compare against.

**So: the 5.9× is real and repeatable; the peak numbers wait for a quiet box.**
Quoting them would be the same mistake as the 65%.

### The instrument itself

- **microseconds, not milliseconds.** A cheap frame is well under 1 ms and an
  integer millisecond reports every one of them as "0".
- **only frames that repaint are counted**, or a desktop at rest averages out
  as infinitely fast.
- **the peak resets once boot settles.** The first frame repaints the screen
  and bakes the wallpaper — over a tenth of a second, once — and it would
  otherwise sit in the peak for ever, hiding every real regression under it.
- **32-bit throughout.** A 64-bit divide would pull in libgcc's `__udivdi3` and
  this kernel links no libgcc; a 32-bit TSC wraps every ~1.8 s at 2.3 GHz, so a
  wrapped delta is discarded rather than reported as a colossal frame.
