# The look of zlOS — what 25 agents found

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366`**

Wave 1 of the fleet run: 25 lenses over `wm.c`, `fb.c`, the font and icon atlases,
`ui.c`, and the desktop shell in `kernel.zl`.

**This board is agent output.** Items marked **✓ verified** were re-derived by hand.
Everything else is a lead — reproduce before acting.

---

## First: the brief was wrong four times, and the agents caught it

Every agent was briefed with claims taken from the repo's docs and told to verify rather
than accept them. Four framings in my own brief turned out to be false, and the
corrections are more useful than most of the findings:

| the brief assumed | what an agent established |
|---|---|
| Motion may be absent; specify a minimum viable system | **Motion is not absent.** `wm.c:315-484` is a complete 5-kind timeline (OPEN/CLOSE/PRESS/PULSE/FADE) with integer easing tables, an 8-slot array, refusal-on-full, a real composited fade, and 12 assertions in `hosttest/wmtest.c:605-712` |
| Gamma mismatch between glyph generator and compositor is the most likely text defect | **Not present.** The agent re-ran `gen_hd_font.py` into a scratch dir and reproduced the committed `kernel/src/graphics/fonts/font_aa.c` **byte-for-byte (279,043 bytes)**. Generator and compositor agree: linear area coverage, composited in linear light |
| Colours are probably scattered literals; propose a token layer | **A real token layer exists and is better than implied.** `struct ui_theme` (`ui.h:31-54`) holds 12 roles, `ui_theme_init` (`ui.c:43-99`) is the only place C names a colour, and `grep -oE '0x[0-9A-Fa-f]{6}' wm.c` returns **0** |
| Check the SSE paths for edge/tail divergence from scalar | *"I could not construct an input that makes either path differ from its scalar fallback."* `fill32` has a scalar prologue before any aligned store; `copy32` uses `loadu`/`storeu` and its only caller feeds disjoint buffers |

A negative result honestly reported is worth as much as a finding here, and three of the
four above are negative results.

---

## The real headline: there is no time base  ✓ verified

**Lens `motion`.** The animation system is complete except for the one thing that makes
it an animation:

> `anim_tick()` (`wm.c:431`) advances exactly **one table index per compositor pass**,
> never per unit of elapsed time.

So every duration in the design comments — `wm.c:299` says *"four frames at 100 Hz is
40 ms"* — is really "four frames", whatever a frame costs today. Animation speed is a
function of host load, scene complexity and resolution.

This is the single change with the largest visible effect on the look, and it is small:
sample `idt_ticks()` in `anim_tick` and advance the index by elapsed time rather than by
call count. Everything else in the timeline already exists.

**The clock is already in the file.** `idt_ticks()` is declared at `wm.c:133` and used
at `wm.c:1085`, and comments at `wm.c:1070` and `wm.c:1518` already reason correctly
about its 100 Hz / 10 ms resolution. Two subsystems in `wm.c` consult it; the animation
timeline is the one that does not. (Name collision to avoid: `kernel.zl:1146`'s
`fn anim_tick(aw, ah)` is the Animation *app*, not this.)

It also interacts with the SMP-bands work: making the redraw 1.78× faster currently
makes every animation 1.78× **faster**, not smoother.

---

## Confirmed defects, ranked by leverage

### ✓ 1. Alt+Tab never fires — one token
`wm.c:1450` tests `code == '\t'` (9); both keyboards emit `KEY_TAB` (0x103), and
`keycodes.h:10` states the boundary explicitly. Full trace in
[`VERIFICATION-LOG.md`](VERIFICATION-LOG.md). **Second instance of the class
`STATE-OF-THE-PROJECT.md` ranks third on the whole-project board.**

### ✓ 2. Drag-to-edge snap is a one-way door
Two independent snap state machines, `SNAP_NONE` `#define`d twice in one file for two
different enums whose values interleave. [`VERIFIED-WM-SNAP.md`](VERIFIED-WM-SNAP.md).

### 3. `layout.c` and `fb.c` disagree about how tall a run is
**Lens `text-layout`.** `layout.c` computes every run height as `size * 5/4`;
`fb_text_rich`'s `FBT_MONO` path ignores `size` and draws a fixed `cell_h`. On a
1920-wide screen that is a **32 px glyph inside a 30 px line box** — and it is on screen
on the browser's shipped `about:home`, which has twelve inline `<code>` runs and a
`<pre>` block.

The agent's own framing is the useful part: *width* is handled carefully — `prop_adv`
exists precisely so measure and draw cannot disagree about advance — and *height* has no
equivalent. There is a `prop_adv` for horizontal and nothing for vertical.

### 4. The hardware cursor path can never activate
**Lens `cursor`.** `gpu_cursor_install` has no caller outside `gputest.c` and no zl
builtin, so `cursor_live` can never become 1. `wm.c:1620,1627` call `gpu_cursor_move` /
`gpu_cursor_is_live`, and both permanently take the false branch. Inside that
unreachable path are three defects that fire on first arming — including the
✓-verified [GGTT page collision](VERIFIED-ggtt-span-collision.md).

### 5. A runtime resolution change destroys the wallpaper permanently
**Lens `fb-glow`.** `kernel.zl:1610-1621`, the `n` command: `set_res` →
`console_init_fb` → `fb.c:707` `fb_cache_reset()`, which drops the baked glow wallpaper
and the baked header/dock. **Nothing re-bakes them.** Reachable from a typed command.

### 6. Glow alpha ramps are integer-truncated with no dither — and the bands are baked
**Lens `fb-gradient`, severity critical.** `fb.c:1882` and `:1912-1913` hand `glow_px`
a whole-integer alpha. 8-bit-per-channel ramps band visibly without dither, and because
the wallpaper is *cached*, the banding is baked into the cache rather than recomputed.

### 7. Two different resize grips are drawn in the same corner
**Lens `wm-chrome`.** Grip A at `wm.c:819-826`, grip B at `wm.c:886-901`. Both drawn,
same corner, every window.

### 8. Focus changes don't damage the window that lost focus
**Lens `wm-focus`.** `wm_open` (`wm.c:633`) assigns `focus_win` directly and damages only
the new window; `wm_close` (`wm.c:664`) moves focus to the new top and never damages it,
so the focused window renders unfocused. `wm.c:688-697` gets this right and says why —
*"both title bars change"* — so the correct pattern is already in the file.

---

## What is genuinely good, and should not be "improved"

Recording this because the fastest way to damage this codebase is to refactor a part
that is already right:

- **The subpixel atlas carries FreeType's real LCD filter, not a naive one.** The agent
  proved it from the bytes: measured step response into a saturated run is
  `10,87,172,247,255`; FreeType's cumulative taps predict `7,84,170,247,255`; a naive
  `(1,2,3,2,1)/9` kernel would give `28,85,170,226,255`. Max channel separation across
  all 3,786 inked body pixels is 163 — exactly `255*(171-8)/256`. Channel order is
  correct too (`gen_subfont.py:81-82` → `blend_sub` at `fb.c:925-927`).
- **The theme layer is real and covered.** And it corrects
  `STATE-OF-THE-PROJECT.md` §2.1: `hosttest/palette.c` **is** covered, because
  `gates/land-gate.sh:76-90` now runs every executable in `hosttest/`. It parses the
  reference HTML, `kernel.zl` and `settings.c` rather than restating them, and has
  negative controls.
- **The SSE paths are correct**, including their tails and prologues.
- **The northstar docs are unusually honest** — the LOOK half is largely shipped
  (palette matches, blur callers gone, radial/conic kept deliberately, dock digit debris
  genuinely fixed at `kernel.zl:378-412`). It is the SPEED half where intent outruns the
  tree.

---

## The one measurement gap worth closing

`hosttest/fbbench.c`'s hashed scene — the only correctness net over `fb.c` — never calls
`fb_fill_blend` or `fb_rrect_blend`. Those are the two most performance-critical
primitives in the file (measured 48× and 96× slower than the GPU) and they have **no
correctness coverage**, while the scene's own comment claims *"every primitive that has
a correctness risk appears at least once."* Details and the fix in
[`VERIFICATION-LOG.md`](VERIFICATION-LOG.md).

---

## Suggested order

1. **Give `anim_tick` a time base.** Largest visible effect, small change, and it has to
   land before SMP bands or the animations get faster instead of smoother.
2. **`code == KEY_TAB`.** One token, restores a dead feature.
3. **Reconcile run height between `layout.c` and `fb_text_rich`** — a `prop_adv` for the
   vertical axis. Visible on the shipped home page.
4. **Damage the losing window on focus change.** The correct pattern is already at
   `wm.c:688-697`; copy it into `wm_open` and `wm_close`.
5. **Re-bake the wallpaper after `fb_cache_reset`**, or refuse the resolution change.
6. **Dither the glow ramps** before anything else in the gradient area — banding baked
   into a cache is the worst version of this bug.
