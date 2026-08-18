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

### Also fixed, found while working

`fb_setup` is fully re-entrant — it recomputes the cell, the scissor, the back
buffer and the damage list from its arguments. It did **not** reset the cursor
sprite. `cur_up` stayed 1 across a mode change with `cur_buf` holding pixels
from the old geometry and `cur_x`/`cur_y` addressing it, so the next
`fb_pointer_hide` would stamp stale pixels at a stale position into the new
mode. The old cursor had the same latent bug and escaped it only because
nothing had re-run `fb_setup` yet — the new `wmtest` scale assertion does.
`fb_pointer_forget()` now runs from `fb_setup`.
