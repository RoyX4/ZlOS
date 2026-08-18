# The feel & control track — what actually landed

The running record for `desktop/feel-and-control`, the way
[`desktop-look.md`](desktop-look.md) is for the look track. The brief is
[`FEEL-PROMPT.md`](FEEL-PROMPT.md); this is what came back.

Every entry states the gate **and its output**, or a number that was
**measured**. "It looks right" is not in here.

---

## The gate situation: the kernel does not link, and it is not this track's doing

Before anything else, because it shapes every item below.

`kernel/build.sh` at `8ed3d2e` — clean tree, toolchain freshly built — fails to
link with 17 undefined symbols: the `xhci_ptr_*` USB HID pointer driver,
`idt_set_pointer_bounds`, `idt_mouse_irqs`, and `console_vram`.

The cause is in the message of the commit that produced it, `b19207d`:

> NOT MY WORK - this is the display session's uncommitted changes to kernel.zl
> and runtime_kernel.c, committed on Roy's explicit instruction... Committed
> as-is, unreviewed.

It committed the **callers** and not the **definitions**. The definitions are
sitting uncommitted in the LOOK track's worktree (`../zl-linux/kernel/xhci.c`,
`idt.c`, `console.c`) — confirmed read-only, no worktree touched. FEEL-PROMPT
§1.1 forbids touching another worktree, and writing a second implementation of
the same 17 symbols would collide on merge, which is a worse outcome than a red
gate.

So **every QEMU gate is unavailable**: `verify.sh`, `verify-raw.sh`,
`verify-iso.sh`, `probe-shot.py`, `probe-mouse.py`. FEEL-PROMPT §1.3 names the
fallback and this track takes it: `hosttest/` links individual `.c` files with
its own stubs and is completely unaffected.

Where an item's stated gate needed QEMU, the substitute is recorded **on that
item**, along with what property it checks instead. Tracked as T-13 in
`.ultra/TENSIONS.md`. The QEMU gates get run the moment it closes.

---

## Item 1 — the anti-aliased cursor

### What was wrong

`fb_cursor_arrow` built a 12×18 triangle row by row with a hard step per row:

```c
for (int r = 0; r < 16; r++) {
    int w = r < 12 ? r + 1 : (16 - r) * 2;
    fb_fill_px(x, y + r, w, 1, fill);
    put_pixel(x + w, y + r, edge);
}
```

Every other asset in this renderer is a gamma-correct coverage atlas — both
mono fonts, the proportional font, the icons at two sizes. The cursor was
**zero of them**, and it is the one thing the eye follows constantly. It
survived three resampling fixes because it is *drawn*, not resampled, so no
resampling audit could see it.

**It also never scaled.** `sc` is 2 on every screen 1400px or wider — which is
every screen actually used — and the icons, the fonts and every metric in
`ui.h` follow `ui()`. The cursor stayed 9×16 physical pixels on a 2560-wide
panel. That was not in the brief; it fell out of reading the code.

### What it is now

`gen_cursor.py` → `cursor.inc`, four cursors (arrow, I-beam, resize, busy) at
two sizes, drawn as geometry at 4× and box-filtered down, exactly as
`gen_icons.py` does.

**Two coverage planes per cursor, not one.** A cursor is not an icon: it needs
a light fill *and* a dark outline or it vanishes against a background of its
own colour, and one coverage plane can carry one colour. So `cur_body` is the
shape dilated by the outline width and `cur_fill` is the shape; `fb.c` blends
the edge colour by `body`, then the fill colour by `fill` on top. Both edges
are anti-aliased, and the partial coverage where `fill` fades lets the edge
colour through as a soft rim.

It rides in `cursor.inc` inside `fb.c` rather than becoming `cursor.c`.
FEEL-PROMPT §2 offers `kernel/cursor.c`, but §1.4 is the stronger constraint:
four build scripts carry four separate source lists and adding a `.c` to only
some of them broke two builds twice in one day. §1.4 explicitly blesses the
alternative — "ride inside an existing translation unit like `font_prop.inc`
does" — and it needs zero build-script edits, so it cannot break the other two
tracks. The platform track's unified source list had not landed when this was
written; checked, as §1.4 asks.

### The save-under — the bug this item was going to produce

FEEL-PROMPT called it in advance and it is worth stating exactly why it bites.

The old cursor was **opaque**: every pixel it touched it overwrote, so an 11×17
save-under for a 9×16 arrow was sufficient. An anti-aliased cursor **blends**
with what is under it, and its soft edge reaches a pixel further out in every
direction than any hard-edged reading of the shape suggests. A saved patch one
pixel short leaves a partially-blended pixel behind on *every* move — and
nothing ever repaints that pixel again, so the halo trails the pointer across
the desktop permanently.

**The fix is to not do the arithmetic.** The saved patch is the whole atlas box,
deliberately larger than the ink. Saving the tight ink box would be the
calculation that gets this wrong; the box is 256 px at 1× and 1024 at 2×
against 3.7M for one 2560×1440 frame — 0.03% — so tight buys nothing and costs
a class of bug. Save the box, blit the ink, and containment is true by
construction.

### Gate

Stated gate was `probe-shot.py` with the cursor magnified 8×, then moved and
re-shot for a trail. That needs a booting kernel (T-13). Substituted with
**assertions in `hosttest/wmtest.c`**, which is strictly stronger than looking:
"no stair-steps" becomes "there exist intermediate blend values", and "no
trail" becomes "every pixel it left is exactly wallpaper again" — every pixel,
not a sample.

```
  the cursor sprite covers its own hotspot                 ok
  the cursor has opaque interior pixels                    ok
  ...and MORE partially-blended ones: soft edges, not stair-steps ok
  every inked pixel lies inside the ACTUALLY-saved patch   ok
  moving the pointer leaves NO halo where it was           ok
  ...and after a 3-pixel move, where the boxes overlap     ok
  at a 1x cell the pointer is 16px                         ok
  ...and follows ui() to 32px at a 2x cell                 ok

all good: 0 failure(s)
```

**The assertions were checked against a real bug, not just against green.**
Injecting a save-under two pixels short of the ink (`cur_saved = n - 2`):

```
  every inked pixel lies inside the ACTUALLY-saved patch   FAIL
  moving the pointer leaves NO halo where it was           FAIL
FAILED: 2 failure(s)
```

The first version of the containment assertion **passed with that bug injected**
— it read `fb_pointer_extent()`, the size the save was *supposed* to be, while
the bug was in `cur_saved`, the size actually walked by the restore loop. Check
and bug read different variables, so the check agreed with the intent rather
than the behaviour. That is bug class 6 from FEEL-PROMPT §5 — a predicate
weaker than the property — committed live, in a test written to catch class 6.
`fb_pointer_saved()` now reports what the restore loop reads.

The generator carries its own failing check: the body plane may not saturate the
atlas border, because a shape whose geometry touches the edge has its rim cut
off there. Verified to discriminate — with the arrow's inset removed it reports
`arrow@16: body saturates the atlas border (255) - outline clipped`.

Also compiled clean, zero warnings, under the real kernel flags both ways:
`-m32 ... -Wall -Wextra` and `-m64 -DZL_64 -mcmodel=large ... -Wall -Wextra`.

### Two things only looking found

Rendered over four backgrounds (dark wallpaper, light panel, accent, panel) at
8× and actually looked at:

1. **The arrow's outline was clipped along its top and left.** The tip sat at
   (0,0), so the dilated body plane landed at negative coordinates and was cut
   off by the atlas edge — leaving no dark rim on exactly the two edges that
   meet a light window. Fixed by insetting the shape and moving the hotspot
   with it, so the tip still means the tip.

2. **The arrow fell into two pieces on a light background.** The neck between
   head and tail was 2.0 units against a 0.62 rim; the rim grows *inward* at a
   concave vertex, so 2×0.62 of a 2.0 neck left 0.76 units of fill. On a dark
   background the white body carries the shape and it looks fine. On a light
   one the body vanishes and only the rim is left, and the arrow reads as an
   arrowhead with a detached blob beside it. Neck widened to 2.90 and the rim
   thinned to 0.55, leaving 1.80 — over a whole pixel of body at 16×16.

Neither is visible in any assertion, and neither is visible at 1×. This is what
"a screenshot you actually looked at — for the cursor, magnified" is for.

### The cost, measured

`fbbench` does not cover the pointer, so this number did not exist. `wm_frame`
calls `fb_pointer_hide` + `fb_pointer_show` exactly once per frame, so that
pair *is* the per-frame cost. A/B against `HEAD`'s `fb.c`, same `-O2`, best of
6 runs of 20,000 iterations, load average 3.2:

| mode | old (9×16, hard) | new (AA, scales) | factor |
|---|---|---|---|
| 1280×800 — 1× cell, 16px | 1.86 µs | 4.03 µs | 2.2× |
| 1600×900 — 2× cell, 32px | 1.78 µs | 14.21 µs | 8.0× |
| 2560×1440 — 2× cell, 32px | 2.19 µs | 14.86 µs | 6.8× |

**It costs 8× more at 2× scale, and that is the honest headline.** The old
cursor was 144 px regardless of mode; the new one at 2× saves and restores
1024 px and blends two planes over the ink, so ~7× the pixels and a
read-modify-write per blended pixel instead of a row fill.

In absolute terms 14.9 µs at 2560×1440 is **1.5% of one full-screen fill**
(1020 µs) and **0.09% of a 16.7 ms frame at 60 Hz**. Not worth optimising, but
it is 8×, and quoting only the 1.5% would be choosing the flattering framing.

### Also fixed, found while working — Item 1

`fb_setup` is fully re-entrant — it recomputes the cell, the scissor, the back
buffer and the damage list from its arguments. It did **not** reset the cursor
sprite. `cur_up` stayed 1 across a mode change with `cur_buf` holding pixels
from the old geometry and `cur_x`/`cur_y` addressing it, so the next
`fb_pointer_hide` would stamp stale pixels at a stale position into the new
mode. The old cursor had the same latent bug and escaped it only because
nothing had re-run `fb_setup` yet — the new `wmtest` scale assertion does.
`fb_pointer_forget()` now runs from `fb_setup`.

---

## Item 2 — pointer acceleration and speed

### What was wrong

`input.c` applied raw 1:1 deltas. No acceleration curve, no sensitivity, no
smoothing. Crossing a 2560-wide screen took a physical hand sweep, and slow
precise movement was exactly as coarse as fast movement.

### Where it lives, and why that is not a style choice

In `input.c`'s `pump_mouse`, **not** in the IRQ12 handler. `idt.c` is compiled
`-mgeneral-regs-only` so an ISR never touches SSE; a gain calculation in the
IRQ12 path — or a call out from it to code that uses SSE — corrupts whatever
the interrupted code held in XMM, and since every zl number is a double the
interrupted code is usually the interpreter itself. FEEL-PROMPT records that
this exact mistake killed the 64-bit boot once.

**Verified, not asserted.** `idt.c` still compiles under `-mgeneral-regs-only`,
and its object's complete undefined-symbol list is:

```
$ nm -u _idt64.o
    U apic_active     U apic_eoi     U zl_inb     U zl_outb
```

Nothing from `input.c`. The ISR cannot reach the acceleration code. `input.o`
does contain 11 XMM instructions, and they are `movd`/`movdqa`/`movaps`/
`punpckldq` — GCC using the wide registers as *integer* movers for the
`struct event` copies. `pump_mouse` itself disassembles to **zero** SSE ops;
the arithmetic is integer throughout. `input.c` is not built
`-mgeneral-regs-only` and does not need to be.

### Working from a position, not a delta

The ISR has already integrated the PS/2 deltas by the time `pump_mouse` runs,
so the raw delta is recovered as the difference between consecutive raw
positions. The alternative — publishing a delta accumulator from `idt.c` —
would mean editing a file with uncommitted changes in another session's
worktree. This needs no `idt.c` change at all.

A consequence worth stating: a "delta" here is one poll's worth of movement,
roughly one frame, so it is a *velocity*, which is the right input for a curve.
It also makes the curve frame-rate dependent — under load a frame is longer,
the per-frame delta larger, and the pointer accelerates more. Fixing that needs
a clock finer than `idt_ticks()`' 100 Hz, so it is **noted rather than faked**.

### The shape

Integer fixed-point, in percent, 100 = 1:1. Two segments, as asked, not a
spline: at or below a 4-unit delta the curve contributes nothing at all, and
above it the gain climbs linearly to a ceiling. One gain is derived from the
2D magnitude and applied to **both** axes — deriving it per-axis would give a
fast-horizontal, slow-vertical move two different gains and bend the direction
the hand actually moved. `max + min/2` approximates the hypotenuse to ~12% with
no `sqrt`.

Sub-unit movement is **carried, not discarded**. At 50% a 1-unit delta
truncates to zero, so without a remainder the pointer would simply never
respond to slow movement — the precise case the two-segment curve exists to
protect. C truncates toward zero, which is what makes the remainder symmetric
so the pointer does not drift one way over time.

### Gate

`hosttest/inputtest.c`, 24 assertions, all green — the 12 that were there
before plus 12 new:

```
  at 1x with accel off every delta is reproduced exactly ok
  a 1-unit delta is never accelerated, at any speed    ok
  a 100-unit delta moves 100 at 1x                     ok
  ...and exactly 200 at 2x                             ok
  at 0.5x ten 1-unit steps move 5, not 0 - the remainder is carried ok
  ...and back to exactly where it started going the other way ok
  a 3-unit move is unaccelerated                       ok
  ...a 60-unit move travels further than 60            ok
  ...and the curve SATURATES rather than running away  ok
  accel off returns the same 60 units                  ok
  at 4x with accel the pointer never leaves the screen ok
  ...and comes straight back off the edge, not after an overshoot ok

all good: 0 failure(s)
```

The saturation check deliberately does **not** name the ceiling's value.
Mirroring `input.c`'s `ACC_MAX` in the test would be one constant in two places
that can drift, and a gate agreeing with a stale copy of the thing it checks is
bug class 6. Instead: past the ceiling the gain stops depending on the delta,
so a 10× bigger move must travel exactly 10× further. An uncapped linear curve
would make it 10× further *times* a 10× gain.

### What this broke, and why the breakage was right

Adding the clamp turned **`wmtest` red — 11 failures**, then 5, then 3. Every
one was a real assumption, not a flake:

1. **Acceleration ships ON by default**, so `pointer(x,y)` no longer put the
   pointer at `x,y`, and every assertion that clicks a close box, a tab or a
   client area failed. Both `inputtest`'s plumbing tests and `wmtest` now pin
   the gain to the identity, because "does the queue behave" and "does the
   curve behave" are two questions and aiming a click under acceleration would
   mean inverting the curve inside the test.

   Accel is on by default deliberately: a defect that persists until someone
   finds a toggle is not fixed.

2. **`wmtest`'s fake hardware returned unclamped positions.** The real IRQ12
   handler can never publish a negative or off-screen pointer, but the harness
   handed back whatever a test set — one drag was checked by pulling the
   pointer to `-250,-86`. Those tests failed against a *more* faithful model,
   not a broken one. The fake `idt_mouse_x/y` now clamp like the hardware.

3. **The drag test inherited its setup from the tests above it.** It read
   `wm_geometry(a)` and dragged by a hardcoded offset, and by the time control
   reaches it window `a` is at 700,400 rather than the 100,100 it was opened
   at — so the drag ran off the right edge and came up 71 px short. Invisible
   while the pointer could go anywhere. It now moves the window to a known
   position first.

None of the three was caused by acceleration; acceleration only made them
visible. That is what the harness is for.

### Known divergence — `kernel.zl` is still unaccelerated

There are two pointer consumers. `input.c`'s pump feeds `EV_MOUSE` to the
compositor, and that path is accelerated. `kernel.zl` also calls the `mouse_x()`
/ `mouse_y()` builtins directly (lines 403, 426, 976, 2201), which read the raw
ISR position and are **not**.

FEEL-PROMPT §2 scopes this track to "`input.c` — accel only", and the builtins
resolve through `runtime_kernel.c`, which is the display session's file and the
source of T-13. So the seam is provided rather than crossed:
`input_ptr_x()` / `input_ptr_y()` return the accelerated position and are what
anything asking "where is the pointer" should call. Unifying the builtins onto
them is a one-line change per call site once T-13 closes.

---

## Item 3 — a Settings app that actually changes things

### What was wrong

FEEL-PROMPT finding 0.3: runtime customisation was **one function call away and
unused**. `ui_theme_set()` already existed. Every colour and metric was already
data in a struct on a 4/8/12/16/24 scale. `fb_set_subpixel()` existed and
nothing exposed it. So this was never a feature to build — it was a feature to
*expose*.

Same for the toolkit. `ui_toggle`, `ui_slider`, `ui_label`, `ui_sep` were built
and asserted and **nothing used them**: grepping `ui_begin` outside `ui.c` finds
only `hosttest`. Nothing in the kernel or `wm.c` ever ran a `UI_HITTEST` pass
either, so the widget layer had no event path at all.

### Why it is C and not `kernel.zl`

FEEL-PROMPT §2 offers the Settings app's `app_draw` branch in `kernel.zl`, and
that is the natural home — policy belongs in zl. It is not reachable: **zl
exposes no natives for `ui_*` at all**, so a zl Settings app needs ~15 new
builtins in `runtime_kernel.c` first, and `runtime_kernel.c` is the display
session's file and the source of T-13. In C it works and is gateable today, and
moving it later is a translation rather than a redesign because the widget calls
map one to one.

It is dispatched from `wmglue.c`, which is already *"deliberately the ONLY
place"* that crosses between the compositor and an app. `wm_bind_zl`'s control
flow is untouched, because the boot transcript is ungateable under T-13 and
`verify.sh` demands it be byte-identical.

`settings.c` is added to all four kernel build scripts, which is what §1.4 asks
for when a new `.c` is genuinely needed.

### The one widget sequence

`build_ui()` emits the controls once and **both passes call it**. An
immediate-mode toolkit identifies a widget by its order, so a draw pass and a
hit-test pass that emit different sequences hit-test the wrong control — and
they drift the moment someone edits one and not the other.

Draw passes always pass `click = 0`. `ui.c`'s `fire()` is level-triggered on
`L.click` and `ui_toggle` flips its variable inside `fire()`, so a control
visited twice with the button down toggles twice and nothing appears to happen.

### Gate — the stated one

*"a `wmshot`-style render at two accent colours and two scales from the same
binary, side by side."*

Four renders from one process, with nothing between them but **clicks on the
Settings window's own controls** — no direct calls into `settings.c`. If the
routing were wrong the images would be identical, which is the failure the gate
is for.

```
Ice / scale 2      accent=0 scale=2  ok
Amber / scale 2    accent=2 scale=2  ok
Ice / scale 1      accent=0 scale=1  ok
Amber / scale 1    accent=2 scale=1  ok
all four variants set by clicking the app's own controls
```

Looked at as a 2×2 sheet: the accent drives the sliders, the toggles and the
title-bar underline; the scale drives every metric — title bar height, row
height, font size, padding — and at scale 1 the accent buttons fit on one row
where at scale 2 they wrap to two, which is `ui_row()`'s wrap doing its job.

`inputtest`, `wmtest` and `tritest` all green. `settings.c` compiles clean with
`-Wall -Wextra` under both the 32-bit and 64-bit kernel flag sets.

### Three bugs the gate found

**1. `ui_slider` could never reach its maximum.** `t = (px-x)*(hi-lo)/w + lo`
divides the track into `hi-lo` buckets for `hi-lo+1` values, so the top value
was selectable only at exactly one pixel *past* the track's last pixel — and
the `t > hi` clamp was dead code. A 1..3 scale slider could only ever produce 1
or 2. Found by watching the harness refuse to reach scale 3. Every slider in the
toolkit had it, so pointer speed could not reach 400 either. Fixed to
`hi - lo + 1`, with four new `wmtest` assertions; the HIGH-end one fails on the
old expression.

**2. The panel's content silently overflowed its window.** `ui.c`'s `place()`
advances a cursor and never reports running out of room, so a widget past the
bottom of the client area is still laid out, still counted for widget identity,
and simply drawn outside the scissor — invisible *and* unclickable, with nothing
saying so. The first draft was ~995 px of content in a 642 px client at scale 2:
everything from "Interface scale" down was unreachable, and it looked like a
Settings window with two controls. The layout is compacted (accents as a button
row, 112 px instead of 368); the silent overflow itself is logged against `ui.c`.

**3. `ui_slider` does not track outside its own rectangle**, contrary to its own
comment — which says *"once pressed it must keep tracking after the pointer
leaves its rectangle, which only works because the window that owns the grab
keeps receiving the events"*. `wm.c`'s grab does keep delivering the events and
that half is asserted; `ui_slider` drops them, because `fire()` requires a hit.
So straying a few pixels above a slider mid-drag stops the drag dead. Asserted
as the behaviour it actually has and logged, rather than changed here: giving a
widget grab semantics needs identity that survives between frames, which is a
real change to the toolkit.

### A note the harness earned

Driving this by absolute pointer position does not work while acceleration is
on — `settings_apply()` turns it on, and any sweep inevitably clicks the
acceleration toggle and the speed slider too, after which the pointer overshoots
by up to 3× and slams into the clamp. The harness re-pins the gain to the
identity between clicks. That is the app working correctly, not a defect, and it
is the same lesson `wmtest` learned in Item 2.

---

## Item 4 — settings that survive a reboot

### The block

```
0   4   magic     'z' 'l' 'S' '1'
4   2   version
6   2   count     how many u32 fields follow
8   4   checksum  FNV-1a over the record with THIS FIELD ZEROED
12  4n  the settings, one u32 each
```

A fixed LBA, a magic number, a version field and a checksum, exactly as the
item asks. A bad magic, a bad version, a bad count or a bad checksum falls back
to defaults **and prints a line saying which**. `settings_load` never writes.

One rule the brief does not name, because it is the one that bites later:
**a valid checksum is not a valid value.** A block from a future version, or a
bit flip that happens to collide, can carry a UI scale of two billion. Every
field is clamped on the way in.

### The bug that would have destroyed Roy's laptop

FEEL-PROMPT §6 asks for an adversarial reviewer on this path because *"it is the
first code in the project that can destroy data, and the author is the worst
person to review it"*. That is exactly what happened.

`SET_LBA` was **2048**, defended by a comment reading *"far enough from block 0
that anything that later wants a superblock does not land on us"*. That is
precisely backwards. LBA 2048 is the 1 MiB alignment boundary every partitioner
since ~2010 uses for the start of partition 1. On the machine this was written
on:

```
$ cat /sys/block/nvme0n1/nvme0n1p1/start
2048
$ lsblk -o NAME,START,SIZE,PARTTYPENAME
nvme0n1p1   2048   200M   EFI System
```

And the safety argument — "the NVMe namespace is a dedicated scratch image,
`try.sh` makes a 64 MiB `/tmp/zlos-nvme.img`" — is a property of **one shell
script**, not of the code. `nvme_find()` takes the *first* PCI device with class
01 / subclass 08 / prog-if 02 and stops, with no filter on model, serial or
size; on the ThinkPad that is the internal 477 GB system SSD. `install-esp.sh`
is a documented, supported way to boot zlOS on that laptop from its own ESP.

So the full chain was: boot zlOS on the ThinkPad → open Settings → click any
control → overwrite the EFI System Partition's boot sector. The machine would
not have booted again, and nothing would have said why.

I verified all three links myself rather than taking the agent's word: the
partition start, `nvme_find`'s lack of any filter, and `install-esp.sh`.

**The fix is not a better LBA.** The LBA is not the safety mechanism; the
refusal is. `set_disk_is_ours()` now runs before **every** write and refuses:

- a disk with an MBR or GPT protective MBR (`0x55 0xAA` at the end of LBA 0)
- a GPT disk (`"EFI PART"` at LBA 1)
- a namespace over 1 GiB — `try.sh` makes 64 MiB; anything larger is somebody's
  real disk

Any one of those stops the ThinkPad case dead. `SET_LBA` moved to 64 as well,
but that is hygiene, not protection.

### Gate

The stated gate — change a setting, reboot in QEMU, confirm it survived, then
corrupt the block and confirm the fallback — needs a booting kernel and is
blocked by T-13. `hosttest/settingstest.c` runs the same `settings.c` against a
fake NVMe instead, and is strictly more thorough than a reboot, which can only
show one corruption at a time:

```
settingstest - the settings block, against a fake disk
  ...  68 assertions  ...
  every single-bit flip in the record is refused             ok
  ...and none of them is refused SILENTLY                    ok
       (288 flips, 288 caught, 0 silent)
  a disk with a PARTITION TABLE is refused                   ok
  a GPT disk is refused                                      ok
  a namespace far too large to be the scratch disk is refused ok
  ...and partitioning it later is caught on the NEXT write   ok
  ...and 40 motion events later, STILL nothing has been written ok
  ...the write happens once, on release                      ok

all good: 0 failure(s)
```

The fixture is built by the **test**, independently of `settings.c`, and the
round trip asserts the written block is byte-identical to it — so the writer is
checked against a second implementation of the format rather than against its
own reader. A writer and reader sharing one buggy encoder agree perfectly and
are both wrong.

### Three more the review caught

- **A slider drag wrote the block once per mouse-motion event** — 376
  synchronous writes for one gesture, measured. `wm.c` holds a pointer grab for
  the whole press. Applying still happens on every change (that is the item);
  persisting is now deferred to button-up, and asserted.
- **Block size was bounded below but not above.** `nvme.c` transfers one block
  into a single 4 KiB page and never programs PRP2, so an LBA format larger
  than a page would write past it. Refused.
- **The load clamp said 1..4 while the slider offered 1..3**, so a block
  carrying 4 loaded and then drew a slider pinned past its own maximum. Both
  now read one named constant.

What the review could **not** break was the record format itself — all three
lenses independently cleared the endianness, the checksum exclusion, the
`set_lba_ok` off-by-one, the all-zero/all-0xFF rejection, and the nine return
paths. Five further findings are real and either belong to `nvme.c` or are
blocked by T-13; they are written up as T-16 in `.ultra/TENSIONS.md`, including
that **persistence is currently write-only** because nothing in `kernel.zl`
opens an `APP_SETTINGS` window or calls `settings_load()` yet.

---

## Item 6 — the interaction gaps

Taken in the order FEEL-PROMPT gives them.

### 6.1 — window resize, by a corner grip

`wm_resize` has existed since the window table did — `min_w`/`min_h` clamping,
damage on both the old and the new rect, all correct — and **nothing had ever
called it**. That is this project's own named hazard sitting in the compositor:
"the code exists" is not "the code works".

A **corner**, not an edge. An edge grip has to decide *which* edge from a few
pixels of hit area, and every one of those decisions is another place for an
off-by-one against the frame rect. The bottom-right corner is one rectangle, the
same size as the close box, and it grows the window in the direction the pointer
is already moving. It sits inside the frame so it cannot overlap the shadow,
which is drawn outside the frame and is not part of the window for hit-testing.

Three things fell out of it:

- **The grab mode became a named set.** It was a bare 0/1 meaning "the app has
  it" or "we are moving it"; resize is a third answer, and three states in two
  values is how a bug gets in. `GRAB_APP` / `GRAB_MOVE` / `GRAB_RESIZE`.
- **`grab_dx/dy` hold the offset from the pointer to the corner**, so the corner
  stays under the cursor instead of snapping to it on the first motion event.
- **The pointer shape is the affordance.** Over the grip the cursor becomes
  `CURSOR_RESIZE` — which finally gives the resize cursor generated in Item 1 a
  caller, instead of it being an atlas entry nothing selects. It is held for the
  whole drag so it does not flicker back to an arrow when the pointer outruns
  the corner.

`ui.h` publishes the cursor kinds and `fb.c` static-asserts the generated
`cursor.inc` order against them. Two lists of the same constants drift the
moment a cursor is *inserted* rather than appended, and the symptom is the wrong
picture rather than any failure — which nothing tests. `fb.c` is the pixels
layer and does not include `ui.h` (that would invert the layering `ui.h` itself
sets out), so the assertions are pinned to literals with a comment naming the
other list.

**Gate** — seven new `wmtest` assertions:

```
  the pointer over the grip asks for the RESIZE cursor     ok
  dragging the grip RESIZES the window                     ok
  ...and does not MOVE it                                  ok
  ...and the cursor stays RESIZE mid-drag                  ok
  ...and a drag past the minimum clamps instead of inverting ok
  the title bar still MOVES rather than resizing           ok
  ...and off the grip the cursor is an ARROW again         ok
```

The last two matter as much as the first: a grip that swallowed the title bar's
drag would stop windows moving, and a cursor that never went back to an arrow
would look broken everywhere else.

**And one thing only looking found.** The first version drew the three rules at
the same offset with different lengths, which merges into a single L-bracket —
it renders, and it reads as a border artefact rather than as a grip. They are
three rules *parallel to the corner's diagonal* now, stepping inward, which is
the universal mark for it. No assertion would ever have caught that.

### 6.2 — double-click, and maximise

There was no notion of one anywhere in the kernel.

It is decided in `wm.c`, not `input.c`, because it is a question about **place**
as well as time — two presses 300 ms apart at opposite corners of the screen are
not a double-click, and `input.c` deliberately knows nothing about where windows
are. `idt_ticks()` is 100 Hz, which is ample for a 400 ms window. The slop
follows `ui()`, because "a few pixels" on a 2560-wide panel at 2× is not the
same distance as on an 800-wide one, and a fixed number makes the gesture
hardest on exactly the screens where the pointer travels furthest.

The elapsed-time test is an **unsigned** subtraction, so a tick counter that
wraps cannot report a gigantic elapsed time and silently disable the gesture for
the rest of the uptime.

A double-click on the title bar **maximises**, and again restores the exact rect
— which gives the compositor something to *do* with the gesture rather than only
forwarding it, and puts `wm_move` and `wm_resize` both on a second caller. Apps
get it as `MOUSE_DOUBLE`, a bit in the button mask, rather than as a new event
type: PS/2 uses bits 0..2, bit 8 is free, and an app that masks for button 1
keeps working unchanged. A new event type would have made every existing
`app_event` handler wrong by omission instead.

**Gate** — seven new `wmtest` assertions:

```
  double-clicking the title bar MAXIMISES                  ok
  ...and doing it again RESTORES the exact rect            ok
  two SLOW clicks are not a double                         ok
  two clicks far apart are not a double                    ok
  a TRIPLE click maximises once, it does not toggle twice  ok
  a double-click in the CLIENT area reaches the app with MOUSE_DOUBLE ok
  ...and a lone click does NOT carry it                    ok
```

The negative cases carry the weight. "Two slow clicks" differs from the passing
case *only* in the clock, and "far apart" only in position, so between them they
pin down that the detector is not simply counting presses. The triple-click case
is the one a naive implementation fails: without consuming the gesture, three
clicks are two overlapping doubles, so the window maximises and instantly
restores and appears not to respond at all.

### 6.3 — the scroll wheel: DEFERRED, and why

Not blocked by difficulty. A read-only diff of `kernel/idt.c` against
`../zl-linux/kernel/idt.c` shows the LOOK track has **wholesale rewritten** the
mouse packet path in its uncommitted work — packet assembly extracted into a new
`mouse_byte()`, made callable from *both* IRQ1 and IRQ12 (the 8042 shares one
output buffer, and a keystroke overlapping a mouse packet was swallowing a byte
out of the middle of it), `keyboard_isr` and `mouse_isr` both gutted, and the
clamp moved onto `idt_set_pointer_bounds`.

The wheel changes exactly that code — packet length 3 → 4, plus the enable
sequence at init. Writing it against **this** worktree's `mouse_isr` would
conflict with a wholesale rewrite of the same function *and* be built on a
structure about to be deleted. §1.1 exists for this.

Their `mouse_byte()` is a better home for it than the old ISR anyway. The full
recipe — the IntelliMouse sample-rate knock, the 4-bit **signed** wheel field
that must be sign-extended, and why the wheel has to be published as a
delta accumulator rather than a position — is written up as **T-17** in
`.ultra/TENSIONS.md`.

### 6.4 — a focus ring on controls

Window focus was a title-bar hue plus an accent underline; a focused **control**
had no indicator at all, and no way to move between controls without the mouse.

**The focus index lives in `ui.c`, not in `L`.** `L` is reset by every
`ui_begin`, and focus has to survive between the hit-test pass and the draw pass
— and between frames, which is the entire point of it. *Choosing* which widget
is focused is still the app's, like every other piece of widget state; `ui.c`
only remembers the number. `-1` is the default, because a desktop that boots
with a ring on some arbitrary control looks broken.

**Keyboard activation goes through the same `fire()` a click does.** The moment
a widget can tell Enter from a click, the two paths drift and one grows a bug
the other does not have.

The ring is drawn by each firing widget at the end of its own draw block, and
takes no widget id: `fire()` has already advanced `L.index`, so `L.index - 1` is
the caller's own id — which means it cannot be passed the wrong one. It draws
*outside* the control's rect so it never covers the label.

Tab walks the controls and Enter presses the focused one, wired in `settings.c`.
Tab's modulus is `ui_widget_count()`, whatever the last pass counted, so it
follows the layout at the current scale rather than a number written down
somewhere. Clicking a control also moves the ring onto it — otherwise the next
Enter presses something other than what the user is looking at.

**Gate** — six new `wmtest` assertions:

```
  three firing widgets are counted                         ok
  ...and nothing is focused by default                     ok
  the FOCUSED widget fires on activation                   ok
  ...and it really toggled                                 ok
  ...and ONLY that one                                     ok
  ...and activation is ONE-SHOT, not sticky                ok
```

The last two are the ones a naive implementation fails: an activation that fired
every widget would look correct on whichever control happened to be checked
first, and a flag that is not one-shot means holding Enter toggles the control on
every repaint. Confirmed by eye too — the ring renders as a clean accent hairline
around the focused slider and around nothing else.

### 6.5 — the Super key snaps windows

`MOD_SUPER` had been tracked by `input.c` since it was written and used for
**nothing**.

Snapping is the binding worth spending it on: it is the one window operation
that is genuinely painful with a pointer and trivial with a key, and it needs no
launcher, no menu and no new policy in `kernel.zl` — only `wm_move` and
`wm_resize`, which the grip and the double-click had already given callers.
Super+Left/Right for the halves, Super+Up to maximise, Super+Down to restore.

**One saved rect serves all of them, and `maxed` means "snapped somewhere",
not "maximised".** Snapping left and then right must not overwrite the original
geometry with the left half — capturing the saved rect on every snap instead of
only the first is the bug every naive version of this has, and it makes restore
return you to the *previous snap* rather than to where you started. The test
snaps three times before restoring, for exactly that reason.

**Gate** — seven new `wmtest` assertions:

```
  Super+Left snaps to the left half                        ok
  Super+Right snaps to the right half                      ok
  Super+Up maximises                                       ok
  Super+Down restores the rect it had BEFORE the first snap ok
  ...and a second restore does nothing                     ok
  a bare arrow key does not snap                           ok
  ...it reaches the app instead                            ok
```

The last two matter: arrows that stopped reaching apps would break every text
field in the system, which is a much worse bug than no snapping.

The harness grew a real **scancode queue** for this, so the chord goes through
`input.c`'s actual decoder — the `0xE0` extended prefix, the modifier tracking,
the release codes. A Super+arrow chord that only works against invented events
proves nothing about the one a keyboard sends.
