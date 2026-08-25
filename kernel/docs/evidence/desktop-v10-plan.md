> **AUDITED 2026-08-19 · MOSTLY DONE.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. All ten §6 items landed and §8 is the most accurate account of a run in this repo — keep §8.1's measured numbers and §8.4's arithmetic. But §8.2.5's `mkdisk.sh` kernel-size guard was silently dropped by the final merge, and §§3, 4 and 7 are corrected by the tree.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**

# Making zlOS look like the v10 prototype — the plan

> **DONE 2026-08-18. All ten items in §6 landed, plus five defects the work
> exposed. What actually happened, with the numbers, is at the bottom of this
> file under "§8 — the run".** The plan below is left exactly as written so the
> two can be compared; where reality disagreed with it, §8 says so.

**Written 2026-08-18, as the handoff into a fresh session.** Read this, then
`archive/superseded/desktop-TODO.md` for the numbered task list.

The decision already taken: **option 2 — adopt the prototype's structure,
effects and typography, keep zlOS's navy/cyan palette.** Not a full repaint.
Reasons in §2.

---

## 0. Where the machine actually is right now

**The compositor is the desktop.** Not a demo, not behind a keypress any more
than `w` — and the shell is a window inside it, with a scrollback of its own.

| | state |
|---|---|
| `fb.c` — clip, damage, SIMD, proportional text, buffer sizing | **live, always** |
| `wm.c` / `ui.c` / `wmglue.c` — windows, z-order, tabs, animation, elevation | **live**, reachable with `w` |
| `term.c` — scrollback + typed commands | **live**, the shell is window 0 |
| four boot paths (BIOS / UEFI / raw / 64-bit) | **all green** |
| `wmtest` 59 · `inputtest` 12 · `tritest` 9 | **all green** |

Verified by boot, not by assertion: `kernel/shots/term.png`.

### What is NOT done, and is the first work of the new session

1. **The compositor is not yet the DEFAULT boot state.** `wm_session()` is still
   reached by typing `w`. The branch that makes it the boot path is written but
   not flipped — see §1.
2. **Typed commands are wired but unproven.** `term_key` → `term_cmd` →
   `run_command` is in place; nobody has typed `help` into it and watched.
3. **Two `zl>` prompts** appear in the shot: one captured into the scrollback
   from the old shell before the compositor started, one live. Cosmetic, and it
   goes away when §1 lands (the old shell never runs).

---

## 1. First: finish the inversion

**Task order matters here — do not reorder.**

### 1a. Prove typed commands work

Boot, press `w`, type `help`, then `uptime`, then `cube`, then `nonsense`.
Expect: help text in the scrollback, an uptime line, the cube demo, and
`unknown command: nonsense` for the last one. **The unknown-command path is the
one worth checking** — a shell that silently ignores what you typed is worse
than one with no commands.

`probe-shot.py -k w` sends keys over serial, so this can be a gate rather than
a hand test.

> **Corrected 2026-08-19.** Both paragraphs above are pre-compositor. `w` was a
> command when `run_command` dispatched on single characters; in the desktop it
> is one character in `term.c`'s line buffer and runs nothing. And `-k` as
> shipped typed characters without ever pressing Enter, so **the gate named here
> could not produce the result claimed for it in §8.1** — it photographed a
> frame no command had run in, silently. `-k` now takes a command and submits
> it, and fails loudly if the shell does not echo the line back. The
> reproducible form of this gate is
> `./probe-shot.py -k help -k unknownthing -o v10-typed`.
> Full measurement: `kernel/docs/typing-into-the-compositor.md`.

### 1b. Make the compositor the boot state

At the bottom of `kernel.zl`, the old `while running == 1 { ... }` becomes:

```
if wm_available() == 0 {
    while running == 1 { ...the existing shell loop, untouched... }
} else {
    wm_session()
}
```

**`verify.sh` boots `-kernel -display none`, where `px_w() == 0`,** so
`wm_available()` is 0 and the plain text shell runs exactly as it does today.
That transcript must stay byte-identical — it is the single most likely thing
to go red.

### 1c. Delete what the inversion obsoletes

- `windows_demo` — already superseded, `w` now points at `wm_session`
- **C4**: `fb_bg_snapshot` / `fb_bg_restore` / `fb_grab` / `fb_stamp` /
  `bg_buf` / `sp_buf`, and their builtins. The compositor repaints from damage
  and needs none of it. This frees **128–176 MiB**, takes the 640×480 drag
  ceiling and the 12 px shadow smear with it, and lets the back buffer cover 4K
  from its own span rather than borrowing the drag arena.
- **Keep** `fb_pointer_show`/`fb_pointer_hide` — the 11×17 cursor save-under is
  a different and correct technique.

---

## 2. Why option 2, and what it costs

The prototype is `~/zl OS v10.dc.html`, 216 KB. **It cannot render on this box**
— it needs `./support.js`, a design-tool runtime that does not exist here, plus
Google Fonts that are not installed. Every `{{ bootMark }}` shows as literal
text. Everything below is read from its source, not from seeing it run.

Its palette is a **different visual system**, not a refinement:

| | zlOS | v10 |
|---|---|---|
| background | navy `#141A2E` | near-black `#0b0d0f` |
| surface | `#1E2A44` | grey `#1c2024` / `#22262b` |
| accent | cyan `#55D6FF` | **lime `#b8e838`** |

`desktop-look.md` says *"never introduce a second visual system; extend the one
that exists"*. Adopting the palette wholesale contradicts that rule. Adopting
the **structure and effects** does not. Hence option 2.

---

## 3. What the prototype actually asks for

Counted from the source, not estimated:

| feature | count | verdict |
|---|---|---|
| `rgba()` transparency | **107** | **writeable** — `fb.c` already blends against read-back for text, icons and shadows. A translucent fill is a `fb_fill_blend()`, ~20 lines. |
| `border-radius` | 34 | **done** — `fb_rrect` |
| `position:absolute` | 32 | **done** — everything is absolute already |
| `transform` | 29 | **mostly done** — scale exists (the open animation); translate is free |
| `font-family` | 24 (Public Sans + Roboto Mono) | **writeable**, costs kernel size |
| `animation` / `@keyframes` | 22 / 7 | **writeable** — §5 |
| `display:flex` + `grid` | 63 + 13 | **done** — `ui.c`'s flowing cursor |
| `box-shadow` | 11 | **done** — `fb_shadow`, three elevations |
| gradients (linear/radial/conic) | 7 / 4 / 2 | linear **done**; radial and conic ~30 lines each |
| `sc-for` list loops | **106** | **writeable** — `ui_list_row` in a `while`, data in raw memory the way snake already does it |
| `sc-if` | 84 | **done** — that is `if` |
| **blur** | 9 (10–34 px) | **the one real limit** — §4 |

**The old "20%" verdict in `desktop-northstar-feasibility.md` is stale.** It
counted two things as permanent: "no lists" and "no layout engine". `ui.c` is a
layout engine, and `ui_list_row` expresses a list without a list *type*. Those
were "nobody has written it yet", not "the language cannot". That document
needs rewriting with these numbers — it is the first doc task of the new
session.

---

## 4. Blur — the only genuine wall, and it is a budget not a barrier

**Measured: 8.7 ms for one menu-sized backdrop blur.** Frame budget 16.67 ms.

The prototype wants `backdrop-filter:blur(10..22px)` in **7** places and
`filter:blur(30..34px)` in 2 — nine in total, which is what §3's own table
already said.

> **Corrected 2026-08-19, twice over.**
>
> **The count was 6, and it is 7.** `grep -o 'blur([^)]*)' "~/zl OS v10.dc.html"`
> gives 10, 16, 18, 18, 20, 22, 22, 30, 34. The seventh backdrop blur is a JS
> inline style — `backdropFilter:'blur(16px)'` at line 2685 — which a
> `backdrop-filter:` grep cannot see. §3's "9, 10–34 px" was right all along.
>
> **And "the prototype" is not the northstar.** `LOOK-AND-SPEED-PROMPT.md`
> suspected this paragraph was written from memory, because
> `docs/design/zlOS-design-northstar.html` contains **zero** blur of either
> kind. It was not: this paragraph describes `~/zl OS v10.dc.html`, 216 KB,
> named at §5 and §8 below and never committed to this repo — verified, no
> blur-using HTML has ever existed here on any branch, tag, remote or
> `refs/wip/*` ref. The two documents are a day apart and describe different
> artifacts. Which one wins, per item, is `DECISIONS.md` #29–#32; the answer for
> blur specifically is the northstar, and the reason turned out to be memory
> rather than taste — see #29.

| case | verdict |
|---|---|
| blur behind a panel that opens and **sits still** | **yes** — blur once into a cached buffer on open, reuse every frame |
| blur behind a **moving** window, per frame | **no** — that is half the frame, every frame |
| the two big `filter:blur(30px)` **background** decorations | **yes** — 304 ms once at boot, already measured |

**The design consequence:** panels that blur must not be draggable, or must
re-blur only on drop. That is a UI decision to take deliberately, not a bug to
discover later.

There is no allocator, so a cached blur buffer is another fixed arena — and
after C4 frees 128–176 MiB there is room for several.

---

## 5. Animation — what exists and what is missing

`wm.c` has a **4-frame open animation**: a scale, not a fade, because a fade
needs an offscreen buffer per window and a scale needs nothing new. Hit testing
deliberately ignores it.

The prototype's seven keyframes are `zov`, `zpop`, `zpress`, `zpulse`,
`zsweep`, `ztoast`, `zwin` — window open, button press, pulse, sweep, toast.

**What is missing is a timeline**, not the ability to animate:

```c
struct anim { int win; int kind; int frame; int len; };
```

A fixed array of these, ticked once per frame in `wm_frame`, each marking its
target damaged. ~60 lines in `wm.c`. Deliberately still no easing curves — a
table of 4–8 integer steps per kind is enough and cannot produce a wrong
in-between value.

**Transparency is the prerequisite for half of them.** `zpop`, `ztoast` and
`zov` are opacity fades, and those need `fb_fill_blend()` first. Do §6 before
§5.

---

## 6. Ordered work list

Each item is small enough to gate, and each gate is a screenshot or an
assertion, never "looks right to me".

| # | task | where | gate |
|---|---|---|---|
| 1 | prove typed commands | — | ~~`probe-shot.py -k w` types `help`, `nonsense`~~ — `-k w` typed one character and never pressed Enter; see §1a. Now `probe-shot.py -k help -k unknownthing` |
| 2 | compositor as boot state | `kernel.zl` | `verify.sh` byte-identical |
| 3 | C4 — delete the sticker drag | `fb.c`, `kernel.zl`, runtime | all four boots; 4K back buffer moves to its own span |
| 4 | `fb_fill_blend()` — translucency | `fb.c` | `fbbench` hash + a blend-over-known-background assertion |
| 5 | radial + conic gradients | `fb.c` | `wmshot` render |
| 6 | the animation timeline | `wm.c` | `wmtest` — starts, ends, hit-testing unaffected |
| 7 | cached blur for panels | `fb.c` + arena | `fbbench` — one blur ≤ 9 ms, cache hit ≈ 0 |
| 8 | type scale: 3 sizes, 2 weights | `gen_prop_font.py` | kernel size delta stated; `verify-raw.sh` |
| 9 | more icons, from the prototype's set | `gen_icons.py` | dock screenshot |
| 10 | the prototype's spacing + component shapes | `ui.c`, `wm.c` | side-by-side with the mockup source |

**Do not start 5–10 before 1–3.** Everything after C4 wants the memory it frees,
and everything wants the compositor to be the thing that boots.

---

## 7. Standing hazards, all of them already paid for once

- **Four build scripts have their own source lists** — `build.sh`,
  `build64.sh`, `buildefi.sh`, `mkdisk.sh`. `verify.sh` sees only the first.
  Adding a `.c` broke the UEFI and raw builds **twice** today. Either add to all
  four or ride along inside an existing file the way `font_prop.inc` does.
- **`verify.sh` alone is not enough.** It is 32-bit BIOS. Run `verify-efi.sh`
  (the laptop's path) and `verify-raw.sh` too.
- **The zl parser takes no multi-line call arguments.** A call must be on one
  line.
- **The zl kernel subset has no runtime strings.** String literals exist;
  comparing two runtime strings does not. That is why `term.c` holds the command
  table.
- **`unsigned long` is 4 bytes in the EFI build** — T-11, still open, and the
  `-Werror` guard does not catch the cast that causes it.
- **One `.git`, two sessions** cost a corrupted object store this morning (T-12)
  and three commits on the wrong branch (T-10). Use a worktree.

---

Prototype: `~/zl OS v10.dc.html` · Contract: [`../../src/graphics/ui/ui.h`](../../src/graphics/ui/ui.h) · Wiring:
[`desktop-wiring.md`](../desktop-wiring.md) · Task list:
[`desktop-TODO.md`](../archive/superseded/desktop-TODO.md) · Numbers:
[`desktop-polish-and-speed.md`](../desktop-polish-and-speed.md) · Last night:
[`desktop-overnight-run.md`](desktop-overnight-run.md)


---

## 8. The run — what landed, 2026-08-18

All ten items in §6, in order, each gated. Five things the plan did not
anticipate are in §8.2, and they are the more interesting half.

### 8.1 The ten

| # | task | gate | result |
|---|---|---|---|
| 1 | prove typed commands | `probe-shot.py` types `help` then a bad word | **green** — `shots/v10-typed.png` shows the help text and `unknown command: unknownthing`. **The capability is real; the gate as written was not runnable** — `-k` typed characters without an Enter, so no invocation of it could have produced that picture, and the row went unchallenged for a day. Reproducible since 2026-08-19: `./probe-shot.py -k help -k unknownthing -o v10-typed` |
| 2 | compositor as boot state | `verify.sh` byte-identical | **green**, and `verify-raw.sh` + `verify-efi.sh` too |
| 3 | C4 — delete the sticker drag | all four boots; 4K back buffer | **green**. Scene hashes byte-identical at all three modes. 4K back buffer now **ON** |
| 4 | `fb_fill_blend()` | fbbench hash + blend assertion | **green** — 22.2 cyc/px, hashes unchanged |
| 5 | radial + conic gradients | a render | **green** — both in the wallpaper, proved by sampling a row |
| 6 | the animation timeline | `wmtest` starts/ends/hit-test | **green** — 12 new assertions, 69 total |
| 7 | cached blur | one blur ≤ 9 ms, cache hit ≈ 0 | **green** — **7.37 ms** cold, **0.18 ms** cached |
| 8 | type scale 3×2 | kernel size stated; `verify-raw.sh` | **green** — +250 KiB, and see §8.2 |
| 9 | more icons | dock screenshot | **green** — 10 → 20 icons, +28 KiB |
| 10 | prototype spacing + shapes | side-by-side | **green** — radius 5 → 12, row_h 24 → 28 |

Measured numbers, `hosttest/fbbench` at 1920×1200 on the i7-10510U:

```
fill_blend 600x460 (a=160)      2.655 ms    22.16 cyc/px
rrect_blend 600x460 r=10        2.662 ms    22.22 cyc/px
radial glow 900x700            12.348 ms    45.16 cyc/px
conic wedge 900x700             7.344 ms    26.86 cyc/px
BLUR 600x460 r=20 (cold)        7.368 ms    61.50 cyc/px
...cached, painted              0.184 ms     1.54 cyc/px
WHOLE DESKTOP redraw            5.377 ms     5.38 cyc/px
```

**C4's headline, and it is the one worth keeping:** at 3840×2160 the back
buffer was OFF and a whole-desktop redraw cost 44 ms. It is ON now and costs
**9.71 ms**. A full-screen fill went from 7.97 cyc/px to **0.71**.

### 8.2 The five things the plan did not know

**1. `probe-shot.py -k` could not type into the compositor at all.** §1a assumed
it could. `wm_frame()` reads `input.c`'s queue; zl's `key_get()` read COM1
directly; the two never met. So the moment the compositor became the boot
state, *every gate and probe in the repo would have gone blind*. Serial is now
a third source feeding the one queue, alongside PS/2 and USB — and it had to
carry a UART-absence check, because an undecoded port floats high and would
have injected an endless stream of 0xFF keystrokes on the ThinkPad, which has
no serial port and no other way to be debugged.

**2. Focus went to the wrong window, and the ordering that caused it was
load-bearing.** `wm_open` focuses what it opens; the shell must be opened FIRST
so the boot log lands inside it. Those two facts together gave the keyboard to
the About window, which has no `app_event`. Typing did nothing and said
nothing. There was no `wm_focus` **setter** builtin at all.

**3. The console kept painting underneath the compositor.** Every `print()`
went to the window (via term.c's scrollback) *and* to the framebuffer console's
own scrolling region, leaving a black band across the desktop. `con_mute()`
stops the pixels while the scrollback and COM1 both keep flowing — which is the
split that keeps every gate reading what it read before.

**4. The serial prompt vanished with the text shell**, and every probe waits for
`zl> `. Restored as a serial-only write (`ser_out`), because routing it through
`print` would put a permanently-empty second prompt in the window under the
live one.

**5. `verify-raw.sh` failed, correctly.** That path boots through
`raw_boot.asm`, which sets up its own framebuffer — so the compositor is the
boot state there and `.h20fq` typed five characters into a line buffer and
pressed nothing. Also found on the way: **`raw_boot.asm` loads a fixed 40
chunks**, 1.25 MiB, against a 1.23 MiB kernel — 84 KiB of headroom, which §6.8
would have walked straight through. A kernel over that limit is not a build
error, it is silently truncated and jumped into. `mkdisk.sh` now refuses to
build such an image, and CHUNKS is 60.

Two smaller ones: `wm_running()` was initialised to **1**, so it answered "the
compositor is up" on a machine that had never had a framebuffer; and ten of
`run_command`'s commands had no typed name, so they were reachable from the text
shell and from nowhere else.

### 8.3 What was deliberately not done

- **A real opacity fade.** `ANIM_FADE`'s alpha is computed, exposed and
  asserted, but nothing composites it: a fade needs the window drawn against
  what is *behind* it, which needs a copy of the rectangle taken before the
  window was drawn on it. `ANIM_PULSE` composites today because a tint needs no
  such copy. Naming the difference beats shipping a tint that pretends.
- **The seven full-screen demos are still full-screen.** Converting them is
  desktop-TODO 2e and it was not on this plan's list.
- **The start menu.** Deleted rather than ported — it rode on the same
  grab/stamp C4 removed, and `WF_MODAL` is the mechanism that replaces it.

### 8.4 One thing §4 got wrong, and it is worth the correction

§4 said a translucent fill is "a `fb_fill_blend()`, ~20 lines". True. What it
did not say is that **a full-screen translucent pass costs 22 ms**, and the
compositor redraws the wallpaper inside *every* damage rectangle. The v10
background is six such passes. There is no arrangement of that which fits in a
frame — so the wallpaper is drawn once into a cache and blitted at 1.5 cyc/px.
That is not an optimisation; it is the only way the look exists. The cache is
one screen-sized buffer out of the arena C4 freed, and at 4K it refuses and
says so.
