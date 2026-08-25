> **AUDITED 2026-08-19 · PARTLY OPEN.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. The code almost all landed and is unusually good. What did not land is REACHABILITY: the Settings app, its NVMe persistence, the focus ring and the pointer-speed controls are compiled into every build and cannot be reached from the running desktop. Do not read its rules (§§1.2, 1.4, 2) as current.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**

# The feel & control track — the pointer, settings, customisation, bugs

The third track. `desktop-v10-plan.md` is the **look**, `PLATFORM-PROMPT.md` is
**apps and speed**, and this is **how it feels to use and how much of it you
can change**.

Carved so all three can run at once. §2 is the ownership split; obey it and
nobody stomps on anybody.

---

## How to start it

Open a session in `~/Documents/repos/zl-feel` and paste only this:

```
Read kernel/docs/archive/prompts/FEEL-PROMPT.md in full and execute it exactly as written.
You are in a git WORKTREE on branch desktop/feel-and-control - confirm with
`git branch --show-current` before your first commit. Two other sessions are
running (the LOOK and the PLATFORM tracks); §2's ownership split is what keeps
all three apart. Work the queue in order, gate every item, commit on green.
Agents and skills are authorised - §6 says how without OOM-killing the box.
Do not stop until I tell you to.
```

---

## 0. The three findings this track exists for

Established by reading the source, not guessed:

### 0.1 The mouse cursor is the LAST 1-bit asset in the renderer

`fb.c`'s `fb_cursor_arrow` builds a 12×18 triangle **row by row with hard
edges**:

```c
for (int r = 0; r < 16; r++) {
    int w = r < 12 ? r + 1 : (16 - r) * 2;
    fb_fill_px(x, y + r, w, 1, fill);           /* a hard step per row */
    put_pixel(x + w, y + r, edge);
}
```

Every other asset in this renderer is a **gamma-correct coverage atlas** —
fonts (mono and proportional), icons at two sizes, 16 references in `fb.c`
alone. The cursor is **zero of them**.

**And it is the single thing your eye follows constantly.** `desktop-look.md`
found three resampling bugs and fixed them all; this is the fourth and it was
missed because it is drawn, not resampled. It is the honest answer to "why does
everything still look blocky".

### 0.2 There is no pointer acceleration at all

`input.c` applies raw 1:1 deltas. No acceleration curve, no sensitivity, no
smoothing. Crossing a 2560-wide screen takes a physical hand sweep, and slow
precise movement is as coarse as fast movement. That is *all* of "mouse feel"
and none of it exists.

### 0.3 Runtime customisation is one function call away and unused

`ui_theme_set()` already exists in `ui.c`. Every colour and metric is already
**data in a struct**, on a 4/8/12/16/24 spacing scale. Nothing calls it.

So "change the accent colour live" is not a feature to build — it is a feature
to *expose*. Same for UI scale, subpixel on/off, animation on/off.

---

## 1. HARD RULES

### 1.1 Stay in your worktree

One `.git` shared between sessions cost a corrupted object store and three
commits on the wrong branch on 2026-08-18 (T-10, T-12). **Never `git checkout`
another branch here. Never touch the other worktrees.** `git status` before
every commit; stage by name; never `git add -A`.

### 1.2 Resources

4 cores, 15 GB, OOM-killed twice. `cut -d' ' -f1-3 /proc/loadavg` before
anything heavy; above ~4, wait. **ONE QEMU at a time, ever** — and three
sessions exist now, so check whether another is already booting before you do.
Gates in the background, collected.

### 1.3 Gates

`verify.sh` is 32-bit BIOS only. Also `verify-efi.sh` (the laptop's path) and
`verify-raw.sh`. `hosttest/{wmtest,inputtest,tritest,fbbench}` need no QEMU and
run in seconds — lean on those.

**Never a fixed wall-clock wait in a gate.** Poll for the expected output.
**Never build on a red gate** — log it to `.ultra/TENSIONS.md` and move on.

### 1.4 Four build scripts have four source lists

`build.sh`, `build64.sh`, `buildefi.sh`, `mkdisk.sh`. `verify.sh` sees only the
first, and adding a `.c` broke two builds twice in one day. Add to all four, or
ride inside an existing translation unit like `font_prop.inc` does.

*(The platform track's item 4 may fix this. Check whether it has landed before
adding a file.)*

### 1.5 Do not touch `intel.c`

Its write paths can **damage hardware**. Another session owns it.

---

## 2. OWNERSHIP — three tracks, one tree

| yours | look track | platform track |
|---|---|---|
| `cursor.c` *(new)* | `kernel/ui.c` | `kernel/term.c` |
| `kernel/settings.c` *(new)* | `gen_prop_font.py` | `kernel/smp.c`, `fb3d.c` |
| `kernel/gen_cursor.py` *(new)* | `gen_icons.py` | build scripts |
| `kernel/input.c` — accel only | `font_prop.*` | `kernel/input.c` — sources |
| `kernel/probe-mouse*.py` | `hosttest/wmshot.c` | `hosttest/*` |

**Shared, so be surgical:**

- **`fb.c`** — you own `fb_cursor_arrow` / `fb_pointer_*` **only**. The look
  track owns blending and effects; the platform track owns the memory map and
  blit.
- **`ui.h`** — the look track owns the theme *struct*. You may **add** fields
  (pointer speed, toggles); do not reorder or rename existing ones.
- **`kernel.zl`** — the platform track owns the bottom, the look track the
  middle. You own **only** the Settings app's `app_draw` branch. Keep it to one
  contiguous block.
- **`input.c`** — the platform track added serial input as a third source. You
  are adding **acceleration to the pointer pump**. Different functions; commit
  often and say which.

---

## 3. THE WORK QUEUE

### Item 1 — an anti-aliased cursor (finding 0.1)

The renderer already has everything needed: `blend_cov` blits a coverage
bitmap, `blend_cov_scaled` resamples one, and `gen_icons.py` shows how to draw
geometry at 4× and box-filter it down.

- `gen_cursor.py` — draw the arrow as **geometry** at 4× (like `gen_icons.py`),
  box-filter to a coverage atlas. An arrow, an I-beam, a resize arrow and a
  busy indicator to start.
- emit at **two sizes**, like the icons, so it scales with `ui()` rather than
  being blown up. This project already made that mistake once — nearest-neighbour
  icon upscaling was the single most visible source of blockiness.
- `fb_cursor_arrow` blits from the atlas instead of computing rows.

**Watch the save-under.** `fb_pointer_show` saves an 11×17 patch. An
anti-aliased cursor blends against what is under it, so the saved patch must
cover the **whole** new cursor including its soft edge, or a one-pixel halo
trails the pointer forever. **That is the bug this item will produce if
rushed** — check it explicitly.

**Gate:** `probe-shot.py`, cursor cropped and magnified 8×. Soft edges, no
stair-steps. Then move the pointer and screenshot again: **no trail**.

### Item 2 — pointer acceleration and speed (finding 0.2)

In `input.c`'s `pump_mouse`, between the raw delta and the reported position.

- a **speed multiplier** (integer fixed-point; no floats in the kernel)
- an **acceleration curve**: small deltas pass through 1:1 for precision, large
  deltas scale up. Two segments is enough — do not build a spline.
- both settable at runtime, because Item 3 exposes them

**The trap:** `idt.c` is built `-mgeneral-regs-only` so an ISR never touches
SSE. Acceleration must live in `input.c` (which may use SSE), **not** in the
IRQ12 handler. Putting it in the ISR would corrupt whatever the interrupted
code had in XMM — and "every zl number is a double" means that is the
interpreter itself. This exact mistake killed the 64-bit boot once already.

**Gate:** extend `hosttest/inputtest.c`. Assert: at 1× nothing changes
(byte-identical to today); small deltas stay 1:1 at every setting; a large
delta at 2× moves twice as far; the pointer still cannot leave the screen.

### Item 3 — a Settings app that actually changes things (finding 0.3)

The toolkit already has `ui_toggle`, `ui_slider`, `ui_label`, `ui_sep` —
built and asserted. Nothing uses them yet. This is what they were for.

`APP_SETTINGS`, with live controls:

| control | wired to |
|---|---|
| accent colour | `ui_theme_set()` |
| UI scale | `ui_theme_init(n)` |
| pointer speed | Item 2 |
| acceleration on/off | Item 2 |
| subpixel text | `fb_set_subpixel()` — exists, unexposed |
| animations on/off | `wm.c`'s `ANIM_FRAMES` |

**Every change must apply immediately and damage the whole screen**, so it
repaints. A settings panel where the change appears on next boot is a
configuration file with extra steps.

**Gate:** `wmshot`-style render at two accent colours and two scales from the
same binary, side by side.

### Item 4 — make the settings survive a reboot

There is **NVMe** (`nvme.c`) and a working disk. A settings block is a few
hundred bytes.

Do it carefully — this is the first thing in the project that **writes** to a
disk:
- a fixed LBA, a magic number, a version field and a checksum
- **refuse to load a block whose magic or checksum is wrong**, and say so
- never write on boot; only when a setting changes

**Gate:** change a setting, reboot in QEMU, confirm it survived. Then corrupt
the block deliberately and confirm it falls back to defaults **and prints a
line about it**.

### Item 5 — the bug hunt

A standing audit, not a one-off. The classes this project has actually
produced, each of which has bitten at least once:

1. **a fixed address that could collide with a neighbour** — the recurring one,
   five times so far
2. **a `u32` holding a 64-bit address** — T-11 is live right now
3. **an off-by-one in a clip or damage rect** — invisible until two things
   share an edge
4. **a silent fallback that should print a line** — found twice on 2026-08-18,
   both in code written that same night
5. **code with no caller** — "the code exists" is not "the code works"
6. **a gate whose predicate is weaker than the property** — `probe-mouse.py`
   passed for weeks while the pointer was pinned at 0,0

**Run it against `wm.c`, `ui.c`, `term.c`, `fb.c`, `input.c`.** Use agents for
this — it is reading, it parallelises perfectly, and a fresh reader beats the
author every time. Anything found gets a **failing test first**, then a fix.

### Item 6 — fill the interaction gaps

Things a desktop has that this does not, all of them small now the compositor
exists:

- **window resize** — `wm_resize` exists and nothing calls it. A corner grip.
- **double-click** — no notion of it anywhere
- **scroll wheel** — the PS/2 protocol has one; `idt.c` reads 3-byte packets
  and never asks for the 4-byte wheel format
- **keyboard focus ring** — focus is title-bar hue plus an accent underline;
  a focused *control* has no indicator at all
- **Super key** — tracked as a modifier, `MOD_SUPER`, used for nothing

Pick them off in that order. Each is a `wmtest` assertion.

---

## 4. What "done" looks like

Never "it looks right". One of:

- a gate command **and its output**
- a screenshot you **actually looked at** — for the cursor, magnified
- a number you **measured** — an instruction count is not a measurement
- an assertion added to a harness, for anything invisible

---

## 5. When the queue is done — the loop

1. Re-run every gate from scratch, all three boots plus every harness.
2. Re-run the Item 5 bug hunt against everything **you** wrote since last time.
3. `fbbench` — has the cursor work cost anything? Report the real number.
4. Improve any doc that was wrong when you tried to follow it.
5. Back to 1.

---

## 6. Agents

Authorised. Best used here for:

- **the Item 5 bug hunt** — fan out readers, one bug class each, across five
  files. This is the ideal shape for it.
- **an adversarial reviewer on the NVMe write path** — it is the first code in
  the project that can destroy data, and the author is the worst person to
  review it.

**Not** for parallel QEMU boots. Three sessions already share one box.

**If you delegate, you own collection.** Never end a turn waiting on an agent.

---

## 7. Hazards, each already paid for

- **The zl parser takes no multi-line call arguments.** One line per call.
- **No runtime strings in the zl kernel subset.** Literals only; comparisons in
  C — that is why `term.c` holds the command table.
- **No lists.** Data in raw memory with an index; snake does this already.
- **`-mgeneral-regs-only` on `idt.c`** — no SSE in an ISR, and no calling out to
  code that uses it. See Item 2.
- **Damage is frame PLUS shadow**, and the reach varies with elevation. Use
  `shadow_reach()` in `wm.c`.
- **Local timestamps are unreliable.** Use
  `gh api repos/RoyX4/zl-linux --jq .pushed_at`.

---

**What came back: [`desktop-feel.md`](../../evidence/desktop-feel.md)** — this track's running
record, gate output and measured numbers per item.

Look: [`desktop-v10-plan.md`](../../desktop-v10-plan.md) · Platform:
[`PLATFORM-PROMPT.md`](PLATFORM-PROMPT.md) · Contract: [`../ui.h`](../../../src/graphics/ui/ui.h) ·
History: [`desktop-overnight-run.md`](../../evidence/desktop-overnight-run.md)
