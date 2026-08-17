# Desktop — the task list

**For whoever picks this up next, human or agent.** Ordered. Do them top to
bottom. Each task says what to change, and how you know it worked.

Nothing here is started. Read `desktop-build-guide.md` first if the words
"clipping" or "z-order" are not already familiar.

**Before starting anything:** `cut -d' ' -f1-3 /proc/loadavg`. If the 1-minute
figure is above ~4, wait. This box has 4 cores and has been OOM-killed twice.
Never run a QEMU boot alongside a multi-agent fan-out.

---

## Phase 0 — no laptop, no touchpad, all testable in QEMU

### [x] 0a. Stop the back buffer switching itself off at high resolution — **DONE 2026-08-18**

**Shipped.** Every ceiling is now the distance to the **next buffer in the
high-RAM map**, which is written out at the top of `fb.c` with each base
re-read from the file that owns it. `fb_setup` prints the verdict.

| mode | back | drag |
|---|---|---|
| 1920×1200 | ON (9,000 KiB of 16,384) | ON |
| **2560×1440** — the ThinkPad panel | **ON** (14,400 KiB) | **ON** |
| 3840×2160 | OFF, and it says why | ON, by 368 KiB |

That last row is why the boot line reports each buffer separately: `back` and
`drag` used to fail together only because `BACK_MAX` and `BG_MAX` happened to
be the same number. Sized from real neighbours they part company at 4K, and a
line reading "lost: window dragging" there would be a lie printed every boot.

**Verified.** `hosttest/fbbench` exercises the drag machinery functionally
(snapshot → scribble → restore, grab → stamp) at all three modes rather than
trusting the arithmetic. A real 2560×1440 boot logs `back ON, drag ON`, the
desktop draws (`shots/b1-2560.png`), and `probe-drag.py` moves the System
Monitor 12.22% of the screen (`shots/b1-drag-2560-after.png`).

**Two things that shot leaves behind, neither of them this task's:**
- a **smear trail** of shadow slivers along the drag path. Arithmetic:
  `fb_shadow(w, h, off = 8·u, soft = 6·u)` reaches `x + w + 28` at `u = 2`, and
  the drag erases `bg_rest(dox, doy, dgw + 16, dgh + 16)` — **12 px short on
  every step**. This is the "shadow halo artifact" 2d already promises to
  delete along with the sticker drag. Now with a number attached.
- dragging needs `-device usb-tablet` gone from `try.sh` — see T-5.

**The original problem, for the record.** `#define BACK_MAX (1920 * 1200)` and
`back_on = ((int)(width * height) <= BACK_MAX)`. The ThinkPad panel is
2560×1440 = 3,686,400 pixels, which is larger, so `back_on` became 0.

Four features are keyed off that one flag and all die together, silently:

| Lost | Where |
|---|---|
| the back buffer | `fb.c:101` — `fb_present()` returns immediately |
| subpixel text | `fb.c:290` — guard is `subpixel_on && back_on` |
| fast pixel readback | `fb.c:546` — falls to the 30–50× VRAM read |
| window dragging, entirely | `fb.c:797` — `bg_ok = 0`, so restore no-ops |

**Do.**
1. Size the back buffer from the mode actually received in `fb_setup()`, not
   from a compile-time constant.
2. **Print a boot line whenever it degrades.** The fallback is legitimate; the
   silence is the bug.
3. Check the resulting top address against the DMA arena at **224 MiB**. The
   buffer base is currently `0x0C000000` (192 MiB). At 4K it would end at
   223.6 MiB — 368 KiB of clearance. Compute the base; do not hardcode it.
   This is the repo's recurring bug class (see `HANDOFF.md`).

**Verify.** Force 2560×1440 in QEMU. Desktop draws, dragging works, boot log
names the mode and says whether the back buffer is on.

---

### [x] 0b. Add a clip rectangle to `fb.c` — **DONE 2026-08-18**

**Why this is the keystone.** Every primitive in `fb.c` clipped to the screen
and nothing else, so there was no way to repaint part of the screen. That, not
the window code, is what blocked a compositor.

**Shipped.** Four statics, `fb_clip(x,y,w,h)` and `fb_clip_none()`. One
rectangle, not a stack — the repaint loop computes the frame and client rects
rather than nesting them, so push/pop would buy nothing.

**This task said "exactly two functions" and that was wrong.** `fb_fill_px` and
`put_pixel` are two of *five*. The other three write the back buffer directly
and call neither: `draw_glyph`'s subpixel fast path, `draw_glyph`'s AA fast
path, `fb_gradient`'s `back_on` branch, and `fb_scroll`. With only the two
changed, **2,184,000 pixels escaped the scissor at 1920×1200** — the wallpaper
and every glyph on screen.

It passed at 3840×2160 while broken, because `back_on` is 0 there and those
paths fall back to `put_pixel`. Worth remembering: a gate that only runs in the
degraded configuration tests the wrong code.

All five fold the scissor into their **loop bounds** rather than testing per
pixel, so a clipped draw costs no more per pixel than an unclipped one. That is
the point — the compositor's hot path is many small clipped rectangles.

**Verified, both halves** (`hosttest/fbbench.c`, `clip_check()`):

| | |
|---|---|
| broke nothing | scene hash unchanged with the scissor at full screen: `8473499efb49abb1` @1920×1200, `81c4be85c58763e7` @2560×1440, `2275f08098c8291e` @3840×2160 |
| does something | **zero** pixels escape a set scissor — fill, gradient, rrect, shadow, AA text, icons, lines, checked at all four edges and one pixel outside each |

The first check alone is passed perfectly by an `fb_clip()` that does nothing.
Both are needed.

---

### [ ] 0c. Replace the single dirty box with a damage list

**Problem.** `fb.c:89-98` tracks one rectangle and `mark()` grows it. A clock in
one corner plus a monitor in the other unions to the whole screen every second.

**Do.** A small array — 8 rectangles is plenty. Merge on overlap. **When full,
merge everything into one**, which degrades to exactly today's behaviour, so the
worst case is "as slow as now", never "wrong". Have primitives call
`fb_damage()` once with their known rect rather than `mark()` per pixel.

**Verify.** Screendump identical; the presented area is measurably smaller.

---

### [ ] 0d. Put mouse events into the input queue

**Problem.** The keyboard produces proper events. The mouse does not — it is a
polled global at `idt.c:100`. `input.c:40` already declares `EV_MOUSE` and
**nothing ever pushes one**.

**Do.** Push `EV_MOUSE` from the same pump in `input_poll()`, carrying x, y and
buttons. Keep the old `mouse_x()`/`mouse_y()` builtins working during the
transition so nothing breaks mid-way.

**Verify.** The existing mouse demo (`x`) and paint (`d`) still work, reading
events instead of globals.

---

## Phase 0.5 — visual bugs, independent of everything else

These are not architecture. They are three resampling bugs that make a
near-modern renderer look 1990s. Any of them can be done at any time, by anyone,
without touching the compositor work. Full detail: `desktop-look.md`.

### [x] 0e. Stop destroying the icons — **DONE 2026-08-18**

`fb.c`, in `fb_icon24`: `int a = ic[y / sc][x / sc];` — nearest-neighbour.
`sc` is 2 on any screen ≥1400px wide, i.e. every screen actually used.

The icons are drawn as clean geometry at 96×96 and box-filtered to 24×24 — then
blown back up to 48×48 by copying each pixel into a 2×2 block. All the
anti-aliasing is thrown away.

**Shipped.** `gen_icons.py` now emits **two** atlases, `icons24` and `icons48`,
each rasterized from the same geometry at its *own* 4× supersample. Simply
box-filtering the 96×96 master by 2×2 would not have worked: 4×4 gives 17
coverage levels, 2×2 gives five, and five is a staircase again. The master has
to grow with the output, so a `ScaledDraw` proxy scales the (hand-tuned,
96-unit) geometry on its way into PIL rather than sixty literals being rewritten.

`fb_icon24` picks the atlas rasterized for the scale it is drawing at. A scale
neither atlas covers — 3× and up, which **0f4/F4**'s fractional UI scale will
want — bilinearly resamples the 48×48 set instead of copying it.

| | before | after |
|---|---|---|
| icons24 bytes | 5,760 | 5,760, **byte-identical** (proved: `ScaledDraw` at k=1 is an identity) |
| icons48 bytes | — | 23,040 (20 coverage levels, 18 intermediate) |
| `kernel.elf` | 1,134,848 | 1,160,996 (+26 KiB) |
| 10 dock icons @1920×1200 | 20.1 cyc/px | **10.0 cyc/px** — best of 3, and even the worst new run beat the best old one |
| whole desktop redraw | 5.65–5.89 ms | 5.74–5.85 ms — **unchanged within noise**, do not claim a win here |

It got *faster* because two integer divides per pixel cost more than the larger
atlas costs in cache.

**Verified.** `kernel/probe-shot.py` (new) → `shots/a-dock-zoom.png`, dock at
1920 wide, 3× magnified: edges are smooth, no 2×2 blocks.

### [x] 0f. Anti-alias `fb_line` — **DONE 2026-08-18**

`fb.c` was plain Bresenham. The System Monitor sparkline
(`kernel.zl:1650-1657`) is 8 diagonal segments and every one staircased, right
next to smooth text.

**Shipped.** Wu's algorithm, 16.16 fixed point, no floating point. The axis with
the greater extent is the one stepped along; the accumulator's fractional part
*is* the coverage split between the two bracketing pixels.

**It is 4.3× slower per line pixel and that is the honest number** — 200 long
diagonals went 0.901 ms → 3.845 ms (best of 3 each, `hosttest/fbbench`). Two
blended pixels per step instead of one raw store, each costing a read-back plus
a gamma-correct blend. Kept anyway, because this is a *quality* gate, not a
speed one, and at real volume it is nothing: the sparkline is 8 segments, not
200, so it costs ~0.15 ms against a 16.67 ms frame. Axis-aligned lines are
unaffected — they hit the `a == 255` fast path and never read back.

**Verified.** `shots/a-sparkline-zoom.png`, 4× magnified: blended edge pixels
along every diagonal, no steps.

### [x] 0g. Fix the `logo` path — **DONE 2026-08-18**

`fb_glyph_scaled` read the **1-bit** `font8x16` and drew each set bit as a
filled square. It was the blockiest path in the codebase and it drew the largest
text on screen. `text_big` already did this correctly — `logo` was the odd one
out.

**Shipped.** It now resamples the 16×32 **coverage** atlas — the same
FreeType-hinted glyphs the console draws — into the 8·scale × 16·scale box it
has always occupied, through the shared `blend_cov_scaled`. At scale 2 that is
1:1 with the atlas and costs nothing; above it, an interpolated real glyph.

**Verified.** `shots/a-logo-zoom.png` — the `a` demo's bouncing `zlOS` at
scale 3, 8× magnified: round bowls on the O and S, blended edges throughout.

---

### [ ] 0h. Add a `tsc()` builtin and put frame time on screen

**Nothing measures a frame.** `cpu_tsc()` and `cpu_tsc_khz()` exist and work
(`cpu.c:212`, `cpu.c:220`) but are **not exposed to zl** — no `tsc` builtin.
`idt_ticks()` is 100 Hz, which is 10 ms resolution, too coarse for a 16 ms frame.

**Do this before any performance work.** Optimising without measurement is
guessing. Full reasoning: `desktop-polish-and-speed.md`.

### [x] 0i. ~~Table the shadow falloff~~ — **DONE, but not the way this said**

**This task originally recommended tabling the per-pixel divide in `fb_shadow`.
That was measured and it is WRONG — tabling it made the shadow ~25% SLOWER,
consistently across three runs.** The loop is not arithmetic-bound.

The real problem was algorithmic: `fb_shadow` darkened the window's **entire
footprint**, and the caller then drew the window on top of ~90% of it.

**Shipped** in `fb.c` — skip the covered rectangle, inset by
`SHADOW_SKIP_INSET` (16) so the rounded corners keep their shadow. Verified
pixel-identical by FNV hash of the whole back buffer.

| | before | after |
|---|---|---|
| shadow 600×460 | 4.34 ms | **0.61 ms** |
| one window, full chrome | 5.12 ms | **0.90 ms** |
| whole desktop, 3 windows @1920×1200 | 19.98 ms | **4.88 ms** |

*(Also learned: the blend path has zero divides — GCC already strength-reduces
`/255`. `div255()` at `fb.c:198` is dead code, never called. Delete it.)*
**`div255()` deleted 2026-08-18.** A comment stands where it was, so the next
person optimising this file does not rediscover it and wire it up.

**The lesson, which is the point of `hosttest/fbbench.c` existing:** an
instruction count is not a measurement. Measure before optimising, and measure
again after.

---

## Phase 1 — needs the ThinkPad

### [ ] 1a. I2C-HID transport on real hardware

`i2c_hid.c` has never executed. QEMU has no Intel LPSS I2C, so it cannot be
tested here at all.

### [ ] 1b. Write the HID report decoder

**This does not exist.** `i2c_hid_byte(i)` returns raw undecoded bytes. Nothing
turns a touchpad report into an x, a y and a button.

**This one can be written and tested before 1a works** — capture a report dump
and decode it offline.

### [ ] 1c. Pointer policy

A precision touchpad is not a mouse. It reports absolute positions, contact
count, and tip-switch. Decide and implement: absolute→screen mapping,
tap-to-click, two-finger scroll.

---

## Phase 2 — the compositor

Do not start before Phase 1. Building a pointer UI you cannot test with a
pointer is guesswork.

### [ ] 2a. `wm.c` — the window table

```c
#define WM_MAX 12
struct win { int x, y, w, h; int app; int flags; int min_w, min_h; char title[32]; };
static struct win wins[WM_MAX];
static int zorder[WM_MAX];       /* indices, BACK to FRONT */
static int nwin, focus, pgrab, modal;
```

In C, not zl — the zl kernel subset has no lists (`kernel/README.md:154`), and
`zl_list_n` is a hard fault (`freestanding/runtime_kernel.c:413`).

z-order **is** the `zorder` array. Iteration order is paint order. Raise =
remove + append. Hit-test walks it backwards.

### [ ] 2b. The repaint loop

```
for each damaged rectangle R:
    fb_clip(R)
    draw wallpaper
    for each window, bottom to top:
        if it overlaps R: draw it
    fb_clip_none()
```

### [ ] 2c. Event routing

Three modes, checked in this order: **pointer grab** (a drag owns all pointer
events until button-up) → **modal** (menu open) → **normal** (pointer to topmost
window containing the point; keys to focus).

### [ ] 2d. Delete the photo-and-sticker code

`fb_bg_snapshot`, `fb_bg_restore`, `fb_grab`, `fb_stamp`, `bg_buf`, `sp_buf`
(`fb.c:775-834`). Removing them also removes the 640×480 drag ceiling (the
terminal is 1256×944 — nearly 4× over, which is the real reason it cannot be
dragged), the shadow halo artifact, and 10 MiB of fixed high RAM.

**Keep** `fb_pointer_show`/`fb_pointer_hide`. Save-under for an 11×17 patch is
correct and has none of these problems.

### [ ] 2e. Convert the apps — 7 of them

Each becomes three functions and **no loop of its own**:

```
app_draw (id, x, y, w, h, focus)
app_event(id, win, type, code, x, y)
app_tick (id, win)
```

| App | Note |
|---|---|
| `snake_game` | **start here** — state is already in raw memory (`SNAKE_X`, `SNAKE_Y`) |
| `paint` | |
| `cube_demo` | |
| `anim` | |
| `editor` | |
| `mousedemo` | |
| `windows_demo` | probably just delete it — the real thing replaces it |

Already the right shape, copy the pattern: `draw_sysmon(sx, sy, sfoc)` and
`draw_about(ax, ay, afoc)` take position as arguments and hardcode nothing.

### [ ] 2f. `ui.c` — the toolkit. **Immediate mode, no allocation.**

Full design: `desktop-toolkit.md`. This is the layer that was missing from every
earlier plan.

**Immediate mode**, not a retained widget tree — forced by the constraints, not
chosen for taste: a tree needs a heap and is a list of children, and zl has
neither. `ui_button("OK")` returns whether it was clicked; nothing is allocated.

- **A theme struct** holds every colour and metric. Kills the hand-picked
  spacing numbers. Spacing scale: 4 / 8 / 12 / 16 / 24, times `ui()`.
- **A flowing cursor** is the whole layout algorithm. No tree walk.
- **Hit testing re-runs `app_draw` with drawing off** — the same trick
  `intel_modeset_dry()` uses. Which means **widgets must return whether they
  fired, never take an action as an argument** — C evaluates arguments eagerly,
  exactly the reason `MS_STEP` is a macro.

Build order: `ui_label` → `ui_bar` → `ui_button` → `ui_sep`/`ui_space` →
`ui_toggle` → `ui_slider` (proves `wm.c`'s pointer grab) → `ui_num` →
`ui_list_row` → `ui_scroll`. **Stop there.**

Needs `fb_clip` (step 0b) — a widget must not draw outside its window.

### [ ] 2g. The shell becomes app 0

- `read_line()` stops looping — becomes a state machine fed one char per event.
  Smaller than it sounds: `LINE_BUF` and the history are already in raw memory,
  so the state is already external.
- `run_command` stops blocking. Commands that open an app call `wm_open(id)`.
- The terminal stops being special and joins the z-order like anything else.

---

## Design rules — do not violate these while building

The look is already coherent. Extend it; do not introduce a second system.

- Focus is signalled by **title-bar hue + the accent underline**. Both already
  exist. Do not also change the border colour — one signal, not two.
- Unfocused windows get a **shallower shadow**. `off` and `soft` are already
  parameters of `fb_shadow`, so this is free.
- No new fonts, no new corner radii. The nested 5px/4px `rrect` and `TITLE_H 28`
  are the system.
- Menus reuse window chrome — which becomes automatic once a menu is a window.
- No generic flat/material look. This is a hand-built OS with a real
  workstation aesthetic. Keep it.

---

## The interaction target

What "done" feels like, in Roy's words: *"no more pressing Q or whatever, but a
full one."*

- Apps run in windows and **keep running**. Snake keeps playing while you drag
  the System Monitor.
- Closing is the close box or Ctrl+W. Never "press any key".
- The dock launches **into a window**, not over the whole screen.
- Alt+Tab cycles focus. Super opens the menu.
- The shell is one window among several.

---

Background and decisions: `desktop-plan.md` · Plain-English intro:
`desktop-build-guide.md` · Why it looks blocky: `desktop-look.md` · Polish and
performance: `desktop-polish-and-speed.md` · Status and blockers:
`../../.ultra/STATE.md`
