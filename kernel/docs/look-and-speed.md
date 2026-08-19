# look-and-speed — what a frame costs, what paces it, and what to do next

> **CORRECTION 2026-08-19:** `term_draw()` now rejects scrollback rows outside
> `fb_clip_top()`/`fb_clip_bot()` before drawing glyphs. References below that
> call terminal damage clipping the next implementation task are stale. The
> remaining task is to remeasure the real drag path and prove the source change
> removed the measured cost.

Written 2026-08-19, answering `LOOK-AND-SPEED-PROMPT.md`. The look half is in
[`DECISIONS.md`](DECISIONS.md) #29–#33, where the brief asked for it. This is the
speed half plus the frame target.

**Read [`visual-speed-northstar.md`](visual-speed-northstar.md) first** — it is
Roy's own clarification of what the reference is *for*, and it outranks the
execution brief this document answers. Two things it says bear directly here:
"fast is perceived latency and smoothness, not a high average frame rate printed
after the fact", which is exactly why §1 counts misses rather than reporting an
average; and its list of things that must not ship — "no coarse scaling, digit
debris, clipped text, duplicated palette roles or status indicators that claim
something the machine has not proved". Three of those five are what this session
found and are tracked in `DECISIONS.md` open items E and G. It also creates a
genuine tension with #29, flagged in that entry rather than resolved here.

**Read the numbers as ratios.** Every absolute millisecond here was taken on a
box whose 1-minute load ranged from 2.0 to 14.1 while another session built in
the same checkout. Same-run comparisons are sound; cross-run absolutes are not.
`fbbench` reports cycles per pixel for exactly this reason and those are the
figures to trust.

---

## 1. What actually paces a frame

```
idt.c:344   divisor = 1193182 / 100        the PIT ticks at 100 Hz
wm.c:1520   unsigned int now = idt_ticks();
wm.c:1521   if (now == last_tick) return;
```

One pass per tick, and **nothing else** — no vsync, no deadline, no yield. Three
consequences, all now measurable rather than argued:

| | before today | now |
|---|---|---|
| a frame over budget | invisible | `wm_late()`, on screen, red |
| a tick with no frame in it | invisible | `wm_lost()` |
| how many frames were painted at all | invisible | `wm_painted()` |

### The target, stated so it can be gated

> **Every frame under 16.67 ms, and the peak under 16.67 ms too.**

16.67 and not 10.00, deliberately. The PIT gives a 10 ms slot, but the thing
being missed is a **panel refresh**, and the ThinkPad's panel is measured at
**59.998 Hz** (`HANDOFF.md`, from `PIPE_LINK_M1/N1`). A frame between 10 and
16.6 ms loses a tick without ever costing a visible refresh; charging that as
stutter would report a smooth desktop as broken. So:

- `wm_lost` counts **lost ticks** — what the 100 Hz pacing costs.
- `wm_late` counts **frames over 16.67 ms** — what a person can actually see.

The tray shows `late`, in `--crit` red the moment it is non-zero, because a `0`
that can quietly become a `3` in the same colour is a number nobody re-reads.
`peak` prints all of it over serial on one machine-readable line:

```
  FRAMEUS <us> <peak> END
  FRAMEMISS late <n> lost <n> of <n> budget 16667 FIN
```

Neither counter is a rate. A rate needs a denominator, and the honest one —
painted frames — is not elapsed ticks on a desktop that idles, so both totals
plus the denominator are printed and the divide is left to whoever wants it.

*(The `FIN` terminator is not decoration. `exercise.py`'s `Serial.wait()` matches
a marker as a **substring** and leaves the remainder in the buffer, so a second
line ending `END2` gets matched by the next probe waiting for `END` and hands it
half the previous answer. Measured: `probe-tray.py` read `None` for its first
sample until this said `FIN`.)*

---

## 2. Vsync: there is exactly one source, and it is not reachable today

Surveyed per backend. The short version is that **nothing the compositor
currently draws through can tell it where the beam is.**

| backend | vblank available? | evidence |
|---|---|---|
| VGA (`vga.c`) | no — and it is the text path, irrelevant at desktop modes | the classic `0x3DA` bit 3 would report the legacy CRTC, not the pipe scanning out the linear framebuffer |
| BGA/VBE (`bga.c`) | no — the Bochs DISPI register set implemented here has no status register at all | nothing to poll |
| virtio-gpu | no — `RESOURCE_FLUSH` is synchronous to *command completion*, not to scanout; `VIRTIO_GPU_F_EDID` is not negotiated so there is no refresh rate either | and it is **not in the desktop present path** — `fb_present` blits straight to `fb_base` |
| `intel.c` | **yes, one**: `PIPE_FRMCNT_A`, a free-running frame counter, a pure read path not gated behind `lt_armed` | `intel_wait_vblank()` and `intel_flip()` exist and have **zero callers** anywhere |

So the only real signal is written, compiled, exported to zl — and never called.
Two things block it, in order:

1. **`intel.c`'s MMIO base is never mapped at boot.** `intel_find()` is called
   only from two shell commands, so `intel_frame_count()` returns 0 for the
   entire desktop session. This is the cheapest unblock in the survey and it is
   read-only (`pci_enable()` plus BAR reads).
2. **PSR freezes the frame counter on this panel.** So pacing on `FRMCNT` would
   work while the desktop animates and silently stall when it idles — precisely
   inverted from what is wanted. Un-freezing it is a write path, i.e. `lt_armed`
   territory, i.e. the hazard list.

### The recommendation: pace on a TSC deadline, not on a counter

Do not build an APIC timer or an HPET for this (neither exists today; ACPI's
`acpi_find_table()` would make an HPET ~40 lines if a hardware interrupt is ever
genuinely wanted). **The higher-resolution clock already exists, is already
calibrated, and is already used inside `wm_frame` — it is the TSC.**

The frame *period* is computable without touching the frame counter at all, and
PSR cannot affect it: `pixel_clock / (htotal * vtotal)` = 241,690 kHz /
(2720 × 1481) ≈ **59.998 Hz**, which is where that number came from in the first
place. That is the exact input a TSC-deadline pacer needs. The zl binding for it
is missing and is one line.

### Do not simply raise the PIT — the blast radius is 30+ sites

Ranked by how badly each fails, because they do not fail equally:

- **Class A, hardware-hazardous.** Raising the divisor miscalibrates the TSC,
  which miscalibrates `cpu_delay_us` (`cpu.c:259`), which is what enforces the
  panel's **500 ms T12 power-cycle delay**. That is the one delay this project
  treats as "try it and see is not acceptable". Fix first: one `PIT_HZ` constant
  in `idt.c`, an `idt_hz()` export, every ms↔tick conversion derived from it.
- **Class B.** Six identical `(ms / 10) + 1` conversions become 10× short. They
  fail safe-ish (early return, not hang) — but a 10× short xHCI port-reset
  timeout presents as "the USB keyboard sometimes does not enumerate", which is
  a week of the wrong debugging. One shared `ms_to_ticks()` fixes all six.
- **Class C.** Fifteen hardcoded tick constants encoding wall-clock time. Key
  repeat 10× faster, toasts gone in 300 ms, and the double-click window down to
  40 ms — which breaks double-click outright. `tcp.c`'s RTO set is the one with
  protocol consequences.
- **Class D.** Eleven on the zl side, including `golden.txt:10` — so `verify.sh`
  goes red byte-for-byte and the golden transcript needs regenerating. Expected,
  but budget for it.
- **Class E, and this is the one `grep` will not find.** Window animations are
  counted in **frames**, not ticks, so their wall-clock duration scales
  *inversely* with the PIT rate. At 200 Hz every open/close/press/pulse/fade
  halves and reads as a flicker rather than a motion.

**And a side finding that changes the shape of any pacing work:** the
compositor's top loop is a bare busy-spin with no `hlt` and no yield, which
contradicts `wm.c`'s own stated intent ("It must NOT spin at 100% CPU"). The
100 Hz gate is currently the only thing bounding the repaint rate; the CPU is
pinned regardless. A deadline pacer must add the `hlt` that was always intended,
or raising the cap just burns the same core faster.

---

## 3. Where the time actually goes

`fbbench`, 1920×1200, shipping `fb.c`, this CPU at 2.304 GHz:

```
fill whole screen                1.029 ms    1.03 cyc/px
gradient whole screen            2.073 ms    2.07 cyc/px
fill + present (blit)            2.062 ms    2.06 cyc/px
shadow 600x460 soft=12           0.626 ms    4.78 cyc/px
rrect 600x460 r=10               0.061 ms    0.51 cyc/px
ONE WINDOW (full chrome)         0.787 ms
WHOLE DESKTOP redraw             4.728 ms    4.73 cyc/px
40 lines of AA text (mono)       5.147 ms   <-- the most expensive thing on screen
40 lines PROPORTIONAL            3.584 ms
radial glow 900x700             12.244 ms   44.78 cyc/px
conic wedge 900x700              7.366 ms   26.94 cyc/px
BLUR 600x460 r=20 (cold)         7.466 ms   62.32 cyc/px
...cached, painted               0.185 ms    1.55 cyc/px
```

Read in order of what that tells you:

1. **`fb_shadow` is not the problem any more, and the brief's suspicion #1 is
   closed.** It was "4.3 ms of a 5.1 ms window redraw" before the fix in
   `DECISIONS.md` #12. It is now 0.626 ms — ~4% of the budget. The scissor fix
   (#6) changed it neither way; that is re-measured and recorded there.
2. **The terminal is.** 5.1 ms of a 16.67 ms budget to draw 40 lines, and a real
   drag spends 41–48% of its frame in the shell's scrollback redraw, because
   `term_draw` redraws the whole scrollback into the window rather than the
   intersection with the damage rect. **This is the next target** and it is the
   same function as §1c's font finding.
3. **The wallpaper cannot be drawn live and never could.** One radial glow is
   12.2 ms on its own and the wallpaper has three, plus two wedges. This is why
   `fb.c` calls the cache "not an optimisation, the only way the look exists" —
   and why `DECISIONS.md` #29 is a speed decision wearing a look decision's
   clothes.
4. **The present blit is 2 ms and does not parallelise.** At 3840×2160 it is
   ~5 ms on its own. Full-screen repaint at 4K is ~17.8 ms — over budget before
   anything is drawn. Damage-rect repaint is what makes 4K work at all, and it
   already exists.

### The wallpaper cache holds again — but only since today

`hosttest/walltest.c`, new, asserts this against the shipping `fb.c`:

| mode | with the dock blur | without |
|---|---|---|
| 1920×1200 | cached, 5464 KiB spare | cached, 7384 KiB spare |
| **2560×1440** — the ThinkPad's panel | **REFUSED** | cached, 1984 KiB spare |
| 3840×2160 | refused | refused — 31.6 MiB will not fit 16, by design |

See `DECISIONS.md` #29. The load-bearing point for this document is that **the
mode cannot be booted under QEMU** — GRUB falls back to 800×600 on the emulated
card and `kernel.zl`'s `set_res()` ladder stops at 1920×1200 — so no boot gate
could ever have caught it, and `walltest` is the only thing in the tree that
exercises the panel's real mode.

**A second, still-open bug from the same root:** `fb_cache_reset()` has **no
callers**. The arena never rewinds, so a mode change leaks the previous
wallpaper and every blur slot — 8.8 MiB of 16, permanently, for one resolution
switch at 1920×1200.

---

## 4. SMP band rendering: it is one call, and it should not be made

The brief called this "the single largest untapped win and it may be one call".
It is one call, and the win is smaller than it looks.

**Confirmed: nothing at boot turns it on.** `fb.c`'s `par_dispatch` is NULL by
default and `smp.c:249` is the only thing that ever sets it — inside
`smp_start()`, reached only from `smp_go()`, called from exactly one place:

```
kernel.zl:1792    smn = smp_go()        the `smp` SHELL COMMAND
```

So every band runs serially on the boot core for the entire desktop session
unless a human types `smp`.

**Measured, same `fbbench` run so the ratios are honest:**

```
SMP bands            desktop  gradient   shadow  present   speedup
  1 (serial, today)  4.930ms   2.268ms  0.633ms  2.192ms    1.00x
  2 bands            5.345ms   2.143ms  0.706ms  1.970ms    0.92x   <-- SLOWER
  3 bands            4.283ms   1.427ms  0.671ms  1.511ms    1.15x
  4 bands            2.796ms   1.070ms  0.351ms  1.503ms    1.76x
```

Three things fall out of that table:

- **1.76×, not 4×.** And two bands is *slower than serial* — the barrier and the
  cache traffic cost more than the second core returns.
- It buys that on **full-screen** paths. A window-sized shadow gains nothing
  useful, and the desktop stopped doing full-screen repaints when damage
  tracking landed. The remaining full-screen work is the boot wallpaper bake.
- The `present` column barely moves, and present is the 4K bottleneck.

**And the cost is not zero.** The AP park loop is a `pause` spin, not `cli; hlt`,
and `smp.c` is explicit that it has to be: a core halted with interrupts off can
only be restarted by NMI/INIT/SIPI, and this kernel has no interrupt path to
hand it work. So waking the cores burns three of them permanently. On this
development box that means every probe QEMU pins four host cores — and
`CLAUDE.md` records what load does here: a boot that takes 12 s taking 30, a
gate reporting a regression that did not exist, and the OOM killer taking the
agent process at 7.9 GB.

**Decision: leave it off at boot.** The version that would pay is not "call it
earlier" — it is *wake the cores around the expensive one-off, then park them
again*, and `smp_start()` has no stop. Recording the shape here so the next
person does not re-derive "it is one call" and then discover the spin:

- `smp_go()` before `draw_wallpaper()` in `wm_boot_start`, `wall_save()`, then a
  `smp_park()` that does not exist yet — 1.76× on the one path that is still
  full-screen, for a 20 ms × cores bring-up.
- `probe-smp.py` already proves banded rendering draws **identical pixels** in
  the real kernel, so correctness is gated. Only the lifecycle is missing.

---

## 5. What is done, and what is next

Done today, each with the command that says so:

| | gate |
|---|---|
| the tray's digit debris — root cause, not the documented symptom | `probe-tray.py`, red at 357/5760 px before, 0 after |
| `late` / `lost` / `painted` counters, on screen and over serial | `peak` prints `FRAMEMISS`, `probe-tray.py` reads it |
| the tray row no longer draws off-screen at 800×600 | the fields it cannot fit are dropped, not clipped |
| blur removed; wallpaper cache holds at 2560×1440 | `hosttest/walltest.c`, 6 assertions |
| `dock_baked` no longer lies when the cache is refused | it is `wall_save()`'s return value now |
| §1c — the coarseness, named and measured | `POINTER-PROMPT.md` §1c, answered |
| `fb_shadow` re-timed after the scissor fix | `fbbench`, `DECISIONS.md` #12 |

Next, in the order the measurements rank them:

1. **Remeasure `term_draw` on the real drag path.** It now skips rows outside
   the active framebuffer clip before drawing glyphs. The old measurement was
   41–48% of a drag frame; do not carry that number forward without rerunning
   the probe.
2. **`fb_cache_reset()` has no caller.** One mode switch leaks 8.8 MiB.
3. **`intel_find()` at boot**, read-only, which is what unblocks any vblank work
   at all.
4. **The `hlt` the frame loop never got**, before any pacing change.
5. `PIT_HZ` and `idt_hz()`, so a pacing change stops being hazardous — the
   Class A site is `cpu.c:259`.

### The weakest link in this document

The 2560×1440 result is a **host** measurement of the shipping `fb.c`, not a
boot. It is the strongest evidence available — the mode is unreachable under
QEMU — but nobody has seen that desktop on the panel. The first ThinkPad boot
after this change should be photographed and the boot line read: it must say
`fb: wallpaper cached, 14400 KiB` and not `refused`.
