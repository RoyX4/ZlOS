> **AUDITED 2026-08-19 · MOSTLY DONE.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. 55 of 58 asks are built and booting — several better than specified. Its header still says "Nothing here is implemented"; that describes an inversion which is now the boot state. Keep §1.5 and §§3.3/4 as live reference.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**


# zlOS desktop — inverting the shell and the compositor

> **New to this? Read `desktop-build-guide.md` first.** It explains what we are
> building and why, in ordinary words. This file is the decisions and the
> evidence behind them.

Planning document. Nothing here is implemented. Written in the style of
`HANDOFF.md`: what is **verified** is marked as such, what is **inferred** says
so, and the traps come before the design.

The goal, in Zac's words: *"no more ig pressing Q or wtv, but more of a full
full one."* Concretely — apps keep running in windows instead of taking over the
screen and handing it back when you press a key.

---

## 1. What I read, and the four facts that constrain everything

All four were read out of the source, not remembered.

### 1.1 `fb.c` is not a compositor

It is an immediate-mode drawing library plus three separate tricks:

| Piece | What it actually is |
|---|---|
| back buffer (`back`, `back_on`) | one full-screen RAM buffer, blitted on `fb_present()` |
| damage | **exactly one** dirty box (`mark()` grows a single rect) |
| dragging | background snapshot + sprite grab/stamp — works for **one** window over a **static** background |
| cursor | save-under sprite, 11×17. This one is correct and stays. |

There is no window list, no z-order, and **no clipping** — every primitive clips
to the screen and nothing else. `fb.c:426` (`fb_fill_px`) and `fb.c:159`
(`put_pixel`) are the only two places that clamp.

### 1.2 zl has no lists — so the window table cannot live in zl

`kernel/README.md:154`, verbatim: *"The kernel subset has no lists."*
`runtime_kernel.c:471` makes calling a function value a named fault.

This is why `raise_windows()` (`kernel.zl:1739`) is a hand-unrolled if-ladder
over exactly two windows, why window state is the loose globals `mon_x`, `mon_y`,
`ab_x`, `ab_y`, `mon_open`, `ab_open`, `focus_win`, and why the click hit-test at
`kernel.zl:2049` is a hardcoded nest six `if`s deep.

**You cannot write a window loop in zl as the language stands.** Every design
below follows from this one fact.

### 1.3 Every builtin call is a linear string-compare chain

`runtime_kernel.c:483`, `zl_calln(const char *name, ...)` — **301 `streq`
comparisons**, in source order. Measured positions of the hot ones:

| builtin | position (of 301) | called |
|---|---|---|
| `fill_rgb` | 255 | every panel, every bar |
| `grad_rgb` | 256 | wallpaper, every title bar |
| `get_px` | 258 | every blended pixel |
| `shadow` | 260 | every window |
| `rrect` | 261 | every window, twice |
| `text_aa` | 262 | every label |
| `present` | 270 | every frame |
| `mouse_x` / `mouse_y` / `mouse_btn` | 285 / 286 / 287 | **every frame, three times** |

Each `Value` is ~56 bytes and travels through varargs **by value**. So the three
mouse reads at the top of the main loop are ~860 string compares per frame before
anything is decided.

Not fatal on its own. It is decisive for *where the per-window inner loop goes*:
a repaint that iterates N windows × ~30 primitives from zl pays ~250 compares per
primitive. From C it pays none.

### 1.4 On the ThinkPad's native panel the desktop silently loses four features

This is the finding that reorders the plan. Chain, each link read from source:

1. `efi.c:231-233` takes **the mode the firmware is already in** — it never calls
   `QueryMode`/`SetMode` to pick one. On the X1 Carbon that is the native
   **2560×1440** (HANDOFF confirms 2560×1440 on the eDP transcoder).
2. `kernel.zl:1830` only re-modesets `if px_w() < 1900`. 2560 is not < 1900, so
   `set_res` is **skipped**.
3. `fb.c:86` — `BACK_MAX (1920 * 1200)` = 2,304,000. `fb.c:155` —
   `back_on = ((int)(width * height) <= BACK_MAX)`.
   2560 × 1440 = **3,686,400 > 2,304,000 → `back_on = 0`.**

What turns off, all at once, with no message:

| Feature | Line | Result at 2560×1440 |
|---|---|---|
| back buffer | `fb.c:101` | `fb_present()` returns immediately; all drawing goes straight to VRAM |
| subpixel text | `fb.c:290` | guard is `subpixel_on && back_on` → **grayscale AA only** |
| `fb_get_px` | `fb.c:546` | falls to the VRAM read path — the 30–50× read `fb.c:76` warns about |
| background snapshot | `fb.c:797` | `bg_ok = 0` → `fb_bg_restore` no-ops → **dragging does nothing** |

And every shadow, rounded corner and AA glyph is a `fb_get_px` per pixel. One
`draw_window` shadow at 600×460 with `soft=12` is ~30k read-modify-writes against
uncached write-combining VRAM.

**The fallback is not wrong. It is silent.** That is the bug. Four features
vanish and the boot log says nothing.

> This is verified by reading, and it is deterministic arithmetic — but it has
> **not** been observed on the hardware, because the ThinkPad has not booted
> zlOS yet. It is testable in QEMU today by forcing a 2560×1440 BGA mode.

### 1.5 Two more, smaller

**The sprite buffer caps window size.** `SP_MAX = 640*480 = 307,200`
(`fb.c:783`). At `ui()==2` a System Monitor drag grabs 600×460 = 276,000 — fits,
with 10% headroom. The terminal is 1256×944 = **1,185,664, nearly 4× over**.
That, not a design choice, is why the terminal cannot be dragged.

**`i2c_hid.c` is a transport with no decoder.** `i2c_hid_read_report()` fills a
buffer; `i2c_hid_byte(i)` hands back **raw undecoded bytes**. There is no report
descriptor parser and no `(x, y, buttons)` anywhere. A precision touchpad reports
*absolute* coordinates plus contact count plus tip-switch — it is not a mouse.
So "the touchpad works" is two jobs, not one: bring up the I2C transport on real
hardware, **and** write the report decoder plus a pointer policy (absolute→screen
scaling, tap-to-click, two-finger scroll).

---

## 2. Blockers, honestly ordered

Zac's rule — *don't build a pointer UI you can't test with a pointer* — is right.
I would put one thing before it.

1. **§1.4, the resolution cliff.** Independent of the touchpad, testable in QEMU
   today, and until it is fixed the desktop does not usefully run on the target
   panel at all. A working touchpad driving a slideshow is still a slideshow.
2. **The touchpad**, both halves of it (§1.5).
3. Then the compositor.

Phase 0 below is everything that needs neither the laptop nor a pointer.

---

## 3. The architecture

### 3.1 Layer split

```
kernel.zl     POLICY   theme, layout, which apps exist, what each one draws
    |                  ~20 new builtins
wm.c          MECHANISM window table, z-order, hit-test, focus, damage, routing
    |
fb.c          PIXELS   primitives  + NEW: clip rect, damage list
input.c       EVENTS   keyboard (works) + NEW: mouse events in the same queue
i2c_hid.c     the touchpad (transport only, today)
```

The split is **mechanism in C, policy in zl** — the same line X11 and Wayland
draw against their toolkits. It is forced by §1.2 (mechanism needs arrays;
zl has none) and confirmed by §1.3 (mechanism runs per-primitive; zl pays 250
string compares per call).

It is not a retreat from "an OS written in zl." zl keeps everything that is
actually the OS's character: the theme, the layout maths, what a window contains,
what the dock launches, what each app does. It gives up the bookkeeping loop it
has no data structure for.

### 3.2 The window record — `wm.c`

Fixed array, no heap:

```c
#define WM_MAX 12
struct win {
    int   x, y, w, h;        /* frame rect, pixels */
    int   app;               /* app ID; zl dispatches on it */
    int   flags;             /* OPEN|DECORATED|RESIZABLE|MODAL|FOCUSABLE */
    int   min_w, min_h;
    char  title[32];         /* fixed, no heap */
};
static struct win wins[WM_MAX];
static int  zorder[WM_MAX];  /* indices, BACK to FRONT */
static int  nwin, focus, pgrab, modal;
```

**z-order is the `zorder` array and nothing else.** Iteration order is paint
order. Raise = remove + append, O(n) with n ≤ 12. Hit-test walks it backwards.
That single representation replaces `raise_windows()`'s if-ladder entirely.

### 3.3 Event routing — three modes, checked in order

1. **Pointer grab.** A drag, a resize, a scrollbar. One window owns *all* pointer
   events until button-up, regardless of where the pointer is. Today's drag only
   survives the pointer outrunning the window because it is a bitmap stamp; with
   real repaint, an explicit grab is required, not optional.
2. **Modal.** Start menu open. Events go to the modal window; a click outside
   dismisses it.
3. **Normal.** Pointer → topmost window containing (x,y). Keys → `focus`.

Focus is **separate from pointer-over**. Click-to-focus, matching today.
Raise implies focus; focus does not imply raise (so a menu can take keys without
reordering).

`input.c` already declares `EV_MOUSE` (`input.c:40`) and **never pushes one** —
the mouse is a separate polled global (`idt.c:100`). Unifying that is Phase 0d
and it is small: `evq_push(EV_MOUSE, buttons, mods, x, y)` from the same pump.

### 3.4 The frame

```
wm_frame():
  1. input_poll()                    drain PS/2 + USB HID + touchpad -> one queue
  2. drain queue -> wm_route()       routing may mark damage
  3. for each open window: app_tick()   apps mark their own damage
  4. wm_repaint()                    for each damage rect:
                                       clip to it
                                       wallpaper, then windows back-to-front,
                                       each clipped to rect ∩ window
  5. pointer sprite
  6. fb_present()
```

Step 4 does not exist today and cannot be written without §3.5.

### 3.5 The clip rect — the single enabling change

One global scissor in `fb.c`:

```c
void fb_clip(int x, int y, int w, int h);
void fb_clip_none(void);
```

`fb_fill_px` (`fb.c:429-434`) already clamps to the screen — change those four
clamps to clamp against the scissor. `put_pixel` (`fb.c:161`) already
bounds-tests — make it a scissor test. **Every other primitive is built on those
two, so the whole library becomes clippable by editing two functions.**

Gate: with the scissor at full screen, output must be **pixel-identical** to
today. QMP screendump diff, before and after. That is a real gate, not a vibe.

### 3.6 Damage

Replace the single dirty box with a small ring:

```c
#define DMG_MAX 8
void fb_damage(int x, int y, int w, int h);   /* merge on overlap */
```

When full, merge everything into one rect — which degrades to exactly today's
behaviour, so the failure mode is "slower", never "wrong". Primitives should call
`fb_damage` once with their known rect rather than `mark()` per pixel.

Without this, a live clock in one corner and a System Monitor in the other union
to a full-screen present every second.

### 3.6b Boot, init, and the dual-mode requirement

**The compositor must be optional.** `verify.sh` — the gate — boots with
`-kernel kernel.elf ... -display none`, so QEMU's multiboot loader never
supplies a framebuffer tag and **`px_w()` is 0 on the gate path.**

So the bottom of `kernel.zl` forks:

```
kbd_init(); fs_init()
if px_w() == 0:
    TEXT MODE — no compositor, keep the existing shell loop, transcript
    must stay byte-identical to golden.txt
else:
    layout(); wm_init()
    w_shell = wm_open(APP_SHELL)      <- window 0, FIRST
    ...boot log prints into its client area...
    wm_open(APP_MONITOR); wm_open(APP_ABOUT)
    forever: wm_frame()
```

**Ordering that matters:** the shell window must exist *before* the boot log
prints, because the log goes inside it. `draw_desk()` already has this shape —
it draws the terminal frame, then the log fills it.

**Desktop furniture is not windows.** Wallpaper, dock and header are drawn by
`wm_repaint` in the wallpaper pass, before any window. Always at the bottom,
never overlapped, never in `zorder`. Only the **start menu** becomes a real
window, because it must appear *on top*.

**Frame pacing:** `wm_frame()` must not spin at 100% CPU. Gate it on `ticks()`
or the new `tsc()`.

**Halt:** `q` currently returns 1 and falls out of the shell loop. It must now
ask the compositor to stop — and must keep printing `halting`, which
`verify.sh` greps for.

### 3.7 Window lifecycle

| Event | What happens |
|---|---|
| open | claim a `wins[]` slot, append to `zorder`, focus it, damage its rect |
| raise | move to end of `zorder`, damage its rect |
| move | damage old rect ∪ new rect. No sprite, no snapshot. |
| resize | same, plus `app_draw` at the new size. Apps must be size-agnostic. |
| close | clear OPEN, remove from `zorder`, damage its rect, focus the new top |

No allocation anywhere. `WM_MAX 12` is a hard ceiling and "no free slot" is a
refusal with a message, not a silent drop.

---

## 4. What defines an app

Not a process — no heap, no memory protection. Not a scheduler task either:
`sched.c` works, but preemptive tasks sharing one framebuffer with no protection
is a data race with extra steps. **Do not wire `sched.c` into the desktop.**

An app is:

- an **integer ID**, and
- **three zl entry points**, dispatched by an if-ladder — the idiom `run_command`,
  `dock_cmd` and `menu_cmd` already use:

```
fn app_draw (id, x, y, w, h, focus)   # repaint client area. Must be position-pure.
fn app_event(id, win, type, code, x, y) -> handled
fn app_tick (id, win) -> damaged
```

- and **no loop of its own.** That is the whole inversion.

Already the right shape: `draw_sysmon(sx, sy, sfoc)` and `draw_about(ax, ay, afoc)`
take position as arguments and bake in no globals. Good sign — that pattern is
the target.

Need converting, each a `while` loop today: `snake_game`, `paint`, `cube_demo`,
`anim`, `editor`, `mousedemo`, `windows_demo`. The conversion is mechanical —
hoist locals to globals, turn the loop body into `app_tick`, delete the exit
condition. **Snake is easiest** (its state is already in raw memory at
`SNAKE_X`/`SNAKE_Y`) and is the right one to prove the model on.

Seven boring conversions is the bulk of the work. Boring is correct here.

---

## 5. What the shell becomes

App 0, window 0. Nothing special about it.

The seam already exists: `fb_set_text_box(c0, c1)` (`fb.c:62`) confines the
console to a column band, and — the part that matters — `fb_scroll` only moves
those columns (`fb.c:854`), so a scrolling terminal never smears its neighbours.
That was built for this.

Three changes:

- **`read_line()` stops looping.** It becomes a state machine fed one char per
  `app_event`. Smaller than it sounds: the line buffer (`LINE_BUF`) and history
  are *already* in raw memory, so the state is already externalized.
- **`run_command` stops blocking.** Commands that open an app call `wm_open(id)`
  and return. Commands that print are unchanged.
- **Focus loss** just means no more keys. No other change.

And once the terminal is an ordinary window it becomes draggable — which needs
the `SP_MAX` ceiling gone, which §6 deletes.

---

## 6. What gets deleted

As important as what gets added. The snapshot/sprite drag machinery exists
*only* because there is no clipped redraw. With §3.5 + §3.6 it is dead:

`fb_bg_snapshot`, `fb_bg_restore`, `fb_grab`, `fb_stamp`, `bg_buf`, `sp_buf`
(`fb.c:775-834`) — and with them:

- the 640×480 draggable-window ceiling
- "the terminal cannot be dragged"
- the rectangular halo when a shadow is grabbed off one background and stamped
  onto another (visible on a gradient wallpaper — the sprite carries the *old*
  background through its soft edges and rounded corners)
- **10 MiB of fixed high-RAM buffers** at 128 MiB and 160 MiB

The start menu's grab/stamp goes too: a menu becomes a window with `MODAL` set,
and `open_menu`'s chrome (`kernel.zl:1769-1773`) is already window chrome —
`shadow` + nested `rrect` + gradient header. Make that literal.

**Keep** `fb_pointer_show/hide`. Save-under for an 11×17 patch is the right
technique and has none of the scaling problem.

### Memory after the change

| Buffer | Base | 1920×1200 | 2560×1440 | 3840×2160 |
|---|---|---|---|---|
| back | 0x0C000000 (192 MiB) | 8.8 MiB | 14.1 MiB | **31.6 MiB** |
| bg_buf | 0x08000000 | 8.8 MiB | — deleted — | |
| sp_buf | 0x0A000000 | 1.2 MiB | — deleted — | |

DMA arena starts at **224 MiB** (HANDOFF §"recurring bug class"). At 4K the back
buffer ends at 223.6 MiB — **368 KiB of margin**. That is exactly the collision
class HANDOFF warns has already cost five debugging sessions. Either compute the
buffer base from the actual mode at `fb_setup()` time, or cap the supported mode
explicitly and **say so in the boot log** rather than degrading in silence.

---

## 7. Visual and interaction language

Read off the existing code, not invented. The current look is a coherent
workstation aesthetic — closest relative BeOS/NeXT, not Windows or macOS:

- deep navy wallpaper gradient, dithered (`fb.c:466`) so it does not band
- 5px rounded panel with a 1px darker outer ring nested inside it
- gradient title bar: blue focused (`rgb(48,92,168)→rgb(22,40,92)`), grey not
- a 2px accent underline on the focused title bar **only**
- soft chebyshev drop shadow, 62% max darkening, smooth falloff
- gamma-correct subpixel-AA text
- gradient dock, 48px tiles

Rules for the new chrome — extend the system, do not add a second one:

- **Focus stays signalled by title-bar hue + accent underline.** Both are already
  there and both already work. Do not also change the border colour; one signal
  is enough and two read as noise.
- **Depth ordering costs nothing.** `off` and `soft` are already parameters of
  `fb_shadow`. Unfocused windows get a shallower shadow. That is a one-line
  change that makes the stack legible.
- **Resize grip**: a three-dot diagonal in the bottom-right, accent colour. Not a
  full border. Matches the coverage-atlas icon idiom (`fb.c:919`).
- **Menus reuse window chrome exactly** — which becomes automatic once a menu is
  a window.
- No new fonts, no new radii. The nested 5px/4px `rrect` and `TITLE_H 28` are the
  system.

One thing I would change: `draw_window` hardcodes a red close box
(`kernel.zl:429`). At this scale three buttons would be mush, so keep one — but
make it **accent on hover, red only on press**. One line, and it is the
difference between drawn and designed.

### The interaction model — "no more pressing Q"

- Every app runs in a window and **keeps running**. Snake keeps playing while you
  drag the System Monitor.
- Closing is the close box or Ctrl+W. Never "press any key".
- The dock launches **into a window** instead of taking over the screen.
- Alt+Tab cycles focus — walk `zorder` backwards.
- Super opens the menu.
- The shell is one window among several.

---

## 8. Sequence, with gates

### Phase 0 — no laptop, no pointer, verifiable in QEMU today

| # | Change | Gate |
|---|---|---|
| 0a | Resolution cliff: back buffer sized from the actual mode; **loud** boot line when it degrades | force 2560×1440 BGA in QEMU; desktop draws, log states the mode |
| 0b | `fb_clip` scissor (two functions) | screendump **pixel-identical** at full-screen scissor |
| 0c | damage list, `DMG_MAX 8`, merge-when-full | screendump identical; present is smaller |
| 0d | mouse into `input.c`'s event queue (`EV_MOUSE` already declared, unused) | existing mouse demo works reading events, not globals |

None of this is guesswork. All of it is testable with the PS/2 mouse in QEMU.

### Phase 1 — needs the ThinkPad

| # | Change |
|---|---|
| 1a | I2C-HID transport on real hardware |
| 1b | HID report descriptor decoder → `(x, y, contacts, tip)` |
| 1c | pointer policy: absolute→screen, tap-to-click, two-finger scroll |

### Phase 2 — the compositor

`wm.c` window table → routing state machine → damage-based repaint → delete the
sprite path → convert apps, snake first → the shell becomes app 0.

**Do not start Phase 2 before Phase 1.** Zac's rule and it is correct.

---

## 9. Open questions — for Zac

1. **Does the shell keep a whole window, or become a dock-summoned overlay?**
   A real desktop makes the terminal one app; but zlOS's shell is also its *boot
   log* and its primary UI. Leaning: it stays a normal window, opened at boot,
   and simply stops being privileged.
2. **`WM_MAX 12` — right ceiling?** 12 × ~80 bytes is nothing. The cost is the
   repaint, not the table.
3. **Alt+Tab or Super+Tab?** Alt is free; Super is currently unused as anything
   but a modifier (`input.c:226`).
4. **4K.** The 368 KiB margin in §6 says the buffer layout should be computed,
   not fixed. Worth doing at 0a rather than later.
