# STATE — cold-start context

> ## OVERNIGHT RUN SUMMARY — 2026-08-18, branch `desktop/overnight-compositor`
>
> **Read this first. Everything below the line is the older state file.**
>
> ### What landed, in order, each on a green gate
>
> | | | |
> |---|---|---|
> | **A1–A4** | the three resampling bugs + `div255` | icons rasterized at 48×48 (and **2× faster**, 20.1 → 10.0 cyc/px), Wu lines, logo from the coverage atlas |
> | **B1 / 0a** | the resolution cliff | every buffer sized from its real neighbour; 2560×1440 keeps back buffer **and** dragging, both were off |
> | **B2 / 0b** | **`fb_clip` — the keystone** | and it was **five** functions, not the two the plan named |
> | **B3 / 0c** | the damage list | two corners cost **66–107× less** than one union box |
> | **B4 / 0d** | `EV_MOUSE` into the queue | plus `hosttest/inputtest.c`, 12 assertions |
> | **C1–C3, D0, D1, F1, F2** | **the compositor** | `wm.c` + `ui.c` + `ui.h` |
> | **G1** | SIMD fill and blit | fill **2.2×**, blit **34%**, pixels byte-identical |
> | **H1** | windows animate open | four frames, a scale not a fade; hit testing ignores it |
> | **H2** | tabbed windows | several apps in one frame, the Essence idea |
> | **H3** | tiled barycentric rasterizer | correct, and **measured 2.8× slower** — so *not* wired in |
> | **F3** | proportional text | DejaVu **Sans** for labels; 12 and 20 distinct advances where mono gives 1 |
> | **0a, part 2** | the back buffer's **base** is computed too | 4K desktop redraw **44.4 ms → 9.1 ms** |
> | — | `wmglue.c` | the C half of the wiring, weak symbols, links today |
>
> **57 assertions** in `hosttest/wmtest.c`, 12 in `inputtest`, 9 in `tritest`,
> plus `fbbench`'s clip / damage / drag / hash checks. `wmshot` renders a frame
> to a picture — and immediately caught `ui_toggle` drawing a circle instead of
> a pill, which every assertion had passed.
>
> ### The single highest-leverage next move for Roy
>
> **Wire the compositor in — it is built, tested, and unreachable.** `wm.c`,
> `ui.c` and `ui.h` compile freestanding, link into the 32-bit, 64-bit and UEFI
> kernels, and pass 35 assertions. *Nothing calls them.* `kernel.zl` still ends
> in `while running == 1 { prompt, read a key, run_command }`.
>
> I did not do it because `kernel.zl` was mid-flight in another session all
> night — its `mouse_x` builtin was rewired underneath me *while I was
> measuring it* — and the brief forbids staging another session's work.
>
> The wiring is small and mechanical, because **zl compiles to C**: a zl
> function `app_draw` becomes `Value zl_fn_app_draw(Value, ...)`, so `wm.c`
> needs three shim functions of a dozen lines each, `wm_hooks()` takes them,
> and the boot sequence in OVERNIGHT-PROMPT's "BOOT AND INIT" goes at the
> bottom of `kernel.zl`. **Keep the `px_w() == 0` branch on the old shell
> loop** — `verify.sh` boots `-kernel -display none` where there is no
> framebuffer, and its transcript must stay byte-identical. See **T-8**.
>
> Do **C4** in the same change (**T-9**): deleting the sticker-drag machinery
> is blocked only because `kernel.zl` still calls those builtins. It frees
> 128–176 MiB, which lets the back buffer move down and cover 4K — the one
> mode `fb_setup` still reports as degraded.
>
> ### Honest about what is half-done
>
> - **Groups D2, E1–E7, C5 are not started.** All of them are policy in
>   `kernel.zl` and inherit T-8's block.
> - **B5 (`tsc()` builtin) not done.** It needs `freestanding/runtime_kernel.c`,
>   also mid-flight.
> - **F3 done, but on an assumption you may want to overturn.**
>   `desktop-look.md`'s plan for it does not work — the atlas font is DejaVu
>   Sans **Mono**, so per-glyph advances are all the same number. I added a
>   proportional *face* (DejaVu Sans, same superfamily) and read "no new fonts"
>   as "no second visual system". If you disagree, revert `ef5ba19`; nothing
>   else depends on it.
> - **F4 not started** (gated on D2, which is blocked). **G2 not started and
>   correctly so** — SMP band rendering is still the biggest single speed lever,
>   a real 4× with three cores parked in `cli; hlt`, and still the wrong risk
>   unattended.
> - **H3 is one step of several.** The tiled rasterizer is correct but slower
>   than the scanline fill it does not replace; SIMD across the partial tiles is
>   the step that would reverse that.
> - **T-10 — the display session's three commits landed on MY branch, not on
>   `main`.** `main` is still at `44346d6`. Nothing is lost or corrupted, but
>   `git switch -c` in a shared checkout moved HEAD for both sessions. Needs
>   Roy's eye before any merge; I did not touch it, since rewriting shared
>   history is on the forbidden list.
> - **A correction I had to make mid-run:** I claimed `-device usb-tablet`
>   steals the pointer and `try.sh` should drop it. **Wrong**, retracted in
>   T-5. zlOS drives *two* pointers and my probes were sending the wrong event
>   type. Both paths work.
> - **T-7:** `verify-efi.sh` went red once on unchanged code and green twice.
>   Suspected to be `mkusb.sh` catching another session's half-written `efi.c`.
>   Not diagnosed.
>
> ### How to check any of this in one command
>
> ```
> cd kernel && ./verify.sh && ./verify-efi.sh && ./verify-raw.sh &&
> cd hosttest && ./build.sh && ./fbbench && ./inputtest && ./wmtest && ./tritest
> ```
>
> All three boot paths matter and they cover different code: `verify.sh` is
> 32-bit BIOS, `verify-efi.sh` is the 64-bit UEFI application (the laptop's
> path, and the only gate where `fb.c`'s SIMD is live), `verify-raw.sh` is our
> own bootloader. F3 turned the latter two red — "did not build" — because they
> do not go through `build.sh`'s object list. `verify.sh` alone would not have
> noticed.
>
> `fbbench` also carries the FNV scene hashes that make "this change is
> supposed to be invisible" a number instead of a squint:
> `8473499efb49abb1` @1920×1200 and `81c4be85c58763e7` @2560×1440, neither of
> which has moved since Group A. The 4K one changed **once**, deliberately:
> `2275f08098c8291e` → `e735d8737eeff842`, when 4K gained a back buffer and
> therefore subpixel text, which is gated on `back_on`.

---


<!-- The operator is the single writer of this file. Keep it lean enough that a
     stranger (or a fresh session with no context) could pick up the work from
     this file alone. Summarize; don't accumulate. -->

## Objective

zlOS: a real OS written in `zl`, Roy's own language, on bare metal — no libc,
no GNU, no GRUB on two of the three boot paths.

**Two lines of work run in this repo. See T-4 — which one ULTRA CODE tracks is
unconfirmed.**

- **A — Intel display.** `kernel/HANDOFF.md` states it flatly: *"The point of the
  project is the Intel display driver — the DPLL and a cold-start modeset. The
  laptop is a test PC."* Stage 1 is done. Stage 2 (cold-start modeset) is blocked
  on hardware access only.
- **B — the desktop.** Compositor, input model, window management. Planned
  2026-08-17 in `kernel/docs/desktop-plan.md`. Not started.

## Success metric

**Unset pending T-4.** Candidates, one per line of work:

- A — modeset stages verified on the real panel. Baseline 1 of 2 (survey: 18
  passed, 0 failed, 2026-08-17). Target 2: a cold-start modeset lights the panel.
- B — apps running non-blocking in a window. Baseline 0 of 7 (`snake_game`,
  `paint`, `cube_demo`, `anim`, `editor`, `mousedemo`, `windows_demo` each own a
  `while` loop today). Target 7.

## Done

- **2026-08-17 — desktop architecture planned.** `kernel/docs/desktop-plan.md`.
  Layer split decided (mechanism in C, policy in zl), forced by the finding that
  the zl kernel subset has no lists. Four Phase-0 items identified that need
  neither the laptop nor a pointer. Planning only — no code written.
- **2026-08-17 — modeset Stage 1 complete.** `hosttest/modeset_test.c --survey`,
  read-only, i915 up: 18 passed, 0 failed. Every source conflict settled by
  measurement on the real panel. Buf-trans table identified 10/10 exact.
- `try.sh` GUI mode verified working — boots `-cdrom zlOS.iso`, not `-kernel`.
- 15 drivers, ~6,000 lines. 64-bit, 4 cores via INIT/SIPI, NVMe, USB HID
  keyboard, event-based input with modifiers and repeat.

## Next

Phase 0a of the desktop plan: **the resolution cliff.** At 2560×1440 —
the ThinkPad's native panel — `back_on` computes to 0 and four features turn off
silently (see T-1). It needs no laptop and no pointer; it is testable in QEMU by
forcing a 2560×1440 mode. Everything else in the desktop plan sits behind it.

Contingent on T-4. If ULTRA CODE tracks line A instead, the next move is instead
to ask Roy for a free console and run the cold-start modeset.

## Blocked

- **Cold-start modeset** — needs the display to itself. gnome-shell + Xwayland
  hold `/dev/dri/card0`. Unblocked only by Roy, at the machine:
  `Ctrl+Alt+F3` then `sudo systemctl stop lightdm`. Recovery:
  `sudo systemctl start lightdm`.
- **Desktop Phase 1 (pointer)** — the ThinkPad has never booted zlOS, and
  `i2c_hid.c` is a transport with no decoder (T-2).

## Hard constraints

Gate 4 checks against this list. None of these may be violated by any cycle.

**Hardware safety**
- Panel power: the 500 ms T12 power-cycle delay, and never drive AUX into an
  unpowered panel. These can *damage* hardware, not merely fail. `intel.c` is the
  one place "try it and see" is not acceptable.

**This box (4 cores, 15 GB — OOM-killed twice)**
- Check `cut -d' ' -f1-3 /proc/loadavg` before anything heavy. Wait above ~4.
- Never pair a QEMU boot with a multi-agent fan-out. Run gates in the background,
  sequentially.

**Gates**
- A gate must never be timing-sensitive. Poll for the expected output; never wait
  a fixed wall-clock time. `verify-raw.sh` already failed an unchanged kernel this
  way, tracking host load rather than code.

**Kernel**
- No heap, no filesystem, no scheduler in the default build.
- 256 KiB kernel stack. A 16 KiB stack once overflowed into console statics.
- Multi-MB buffers live at fixed high RAM, never in BSS.
- The DMA arena starts at 224 MiB; zlOS needs `-m 256` minimum. Check any new
  buffer against that ceiling — a DMA buffer outside guest RAM, or an address
  truncated to 32 bits, is the recurring bug class here (five times so far).
- Everything is zl or C with no libc.

**Repo**
- Build outputs and generated `.c` files do not go in git.
- Local timestamps are unreliable (RTC runs slow, NTP corrects mid-session). For
  "when did X happen", use `gh api repos/RoyX4/zl-linux --jq .pushed_at`.

---
Cycle: 0 · Last updated: 2026-08-17

---

## Overnight run log — branch `desktop/overnight-compositor`

One line per landed change, appended as it lands. Newest last.

- 2026-08-18 · baseline `./verify.sh` GREEN before touching anything
  ("ok kernel boots, shell responds, transcript matches golden.txt"), despite
  the other session's uncommitted `intel.c`/`kernel.zl`/`xhci.c` work.
- 2026-08-18 · **A1/0e icons** — `gen_icons.py` emits `icons24` + `icons48`,
  each rasterized at its own 4x supersample; `fb_icon24` picks the atlas for
  the scale and bilinearly resamples only for scales neither covers. icons24
  byte-identical to before. 10 dock icons 20.1 -> 10.0 cyc/px.
- 2026-08-18 · **A2/0f lines** — Wu's algorithm replaces Bresenham in
  `fb_line`. MEASURED 4.3x slower per line pixel (0.901 -> 3.845 ms for 200
  long diagonals); kept because it is a quality gate and real volume is ~8
  segments, not 200.
- 2026-08-18 · **A3/0g logo** — `fb_glyph_scaled` resamples the 16x32 coverage
  atlas instead of drawing 1-bit squares.
- 2026-08-18 · **A4 div255 deleted** from `fb.c`.
- 2026-08-18 · **new: `kernel/probe-shot.py`** — headless boot + QMP
  screendump + PNG crop/zoom. The visual gates in desktop-TODO.md all say
  "screendump it and look"; this is how that runs unattended.
- 2026-08-18 · **new: FNV-1a scene hash in `hosttest/fbbench.c`** — renders a
  fixed scene and hashes VRAM, so "this change is supposed to be invisible"
  (clip rect, damage list, SIMD blend) becomes a number instead of a squint.
  Also stubs `idt_set_pointer_bounds`, which another session's `fb.c` edit had
  silently broken the fbbench link with.
- 2026-08-18 · **B1/0a resolution cliff** — every fixed buffer in fb.c sized
  from its real neighbour in the high-RAM map instead of a compile-time pixel
  count, and `fb_setup` prints the verdict. 2560x1440: back ON, drag ON (both
  were off). 4K: back OFF, drag ON — they no longer fail together, so the log
  reports them separately.
- 2026-08-18 · **T-5 found: `-device usb-tablet` steals the pointer.** Measured
  both ways in one kernel: with the tablet, 21 known deltas move the pointer
  zero pixels (pinned at the screen centre); without it the decode is exact.
  zlOS's PS/2 code is fine. try.sh needs the line removed — Roy's, it is on the
  do-not-touch list. `exercise.py qemu_argv(tablet=False)` added for probes.
- 2026-08-18 · **B2/0b fb_clip - THE KEYSTONE** — and it was FIVE functions,
  not the two the plan named. Proof it mattered: two-function version leaked
  2,184,000 pixels at 1920x1200. All five now fold the scissor into their loop
  bounds, so clipped drawing costs no more per pixel than unclipped.
- 2026-08-18 · **new: `probe-drag.py`, `probe-mouse-sync.py`** — a drag gate and
  a "did the pointer go where it was told" gate. The existing probe-mouse.py
  only asks whether it moved at all, which passed while it was landing on 0,0.

---

## 2026-08-18 — the v10 pass. The compositor is the boot state.

**Objective:** everything in `kernel/docs/desktop-v10-plan.md`. All ten items of
its §6, in order, each behind a gate.

**Status: done.** Four boot paths green (`verify.sh` byte-identical,
`verify-raw.sh`, `verify-efi.sh`, `verify-iso.sh`), `wmtest` 69, `inputtest` 17,
`tritest` 9, `fbbench` clean with scene hashes byte-identical at all three
modes.

**The measurements that decided things**, `hosttest/fbbench`, 1920×1200:

| | |
|---|---|
| translucent fill | 22.2 cyc/px — which is why the wallpaper is a cached bitmap |
| cached wallpaper blit | 1.5 cyc/px |
| blur, cold | 7.37 ms (was 15.0 before the reciprocal) |
| blur, cached | 0.18 ms |
| whole desktop @4K | 44 ms → **9.71 ms**, because C4 gave the back buffer 48 MiB |

**What was NOT done, deliberately:** a real opacity fade (needs a copy of the
rectangle taken before the window is drawn on it — the arena can hold one, and
`ANIM_PULSE` composites today because a tint needs no copy); the seven
full-screen demos are still full-screen (desktop-TODO 2e); the start menu was
deleted rather than ported, because it rode on the grab/stamp C4 removed and
`WF_MODAL` is what replaces it.

**Five defects the work exposed**, all in `desktop-v10-plan.md` §8.2. The one
worth repeating here: **every gate and probe in this repo drives zlOS over
serial, and `wm_frame()` could not see a serial byte.** The plan assumed
`probe-shot.py -k` would work as a gate. It would have gone blind the moment the
compositor booted.


## 2026-08-18 (later) — scale, the pointer, and the dock

Asked: "why does everything look so small", and "did you do ALL of it — what
about the effects, animations, blur".

Both fair, and both found something. Full write-up:
`kernel/docs/desktop-scale-and-effects.md`.

| | |
|---|---|
| **the scale** | `ui()` was `cell_w()/8`, so 1 or 2 and never more, while the layout is written in 800 design units. At 3840 wide that is 1920 units of space for an 800-unit design. Derived from the screen now: **ui 2 / 3 / 4 at 1920 / 2560 / 3840**, verified in the shipping `fb.c` via fbbench. |
| **the effects** | all built and measured, **three with no caller**. Blur behind the menu and the dock; the timeline drives `wm_open` and pulses the shell on an unknown command; translucency in the dock, the menu and its hover wash. |
| **wm.c had TWO animation systems** | a legacy counter in the window struct that `wm_open` used, and the timeline that nothing triggered. One now. |
| **the compositor could not see the mouse** | `pump_mouse()` read the PS/2 pointer only; `mouse_x()` prefers the usb-tablet. Under the compositor the queue is the ONLY source, so with a tablet attached - which is what QEMU gives - **no EV_MOUSE at all**. No dragging, clicking, dock or menu. T-15. |
| **the dock was decoration** | not a window, so `wm_at()` found nothing and clicks were dropped. Hover/press states, running-app indicator, click-to-raise, and the start menu as the first `WF_MODAL` caller. |
| **every label was monospace** | only `wm.c`'s titles were proportional. They draw by role now, which is also what makes them follow the scale. |

New gate: **`probe-dock.py`** drives the POINTER instead of the keyboard. It is
the reason the mouse bug was found at all, and its first run answered 0 px to
all five questions.

Snake is a real app (`sn_draw`/`sn_event`/`sn_step`, no loop) and keeps playing
while another window is dragged across it - the interaction target
desktop-TODO states in Roy's words. Six demos still own the screen.
