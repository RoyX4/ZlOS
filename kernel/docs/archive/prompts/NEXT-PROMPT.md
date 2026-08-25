# NEXT-PROMPT — what to pick up, ranked, with the evidence for the ranking

Rewritten 2026-08-19 at the end of the session that worked the previous
ranking. Read [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md)
for everything open; this file is narrower — it is **which one to do next and
why**, with the measurement that ranks it.

**The previous ranking's items 2, 3 and 4 are done and gated.** They are kept at
the bottom under "What the last session closed", with what each one turned out
to be, because two of the four were not the thing the ranking said they were.

---

## Read these three first, in this order

1. [`visual-speed-northstar.md`](../../visual-speed-northstar.md) — Roy's own statement
   of what the v10 HTML is *for*. It outranks every execution brief in this
   folder. Its rule: say which layer you are judging — **look, feel, machinery,
   or applications** — and never answer with one completeness percentage.
2. [`DECISIONS.md`](../../DECISIONS.md) #29–#38 — the kernel is now settled against the
   northstar item by item. **#29 carries an unresolved tension that is Roy's to
   call**, not the next session's to quietly re-decide. #34–#38 are the last
   session's, and #37 contains a correction it made to its own work.
3. [`look-and-speed.md`](../../look-and-speed.md) — the frame budget, where the time
   goes, and the vsync survey. Note its two correction banners: one entry in it
   was wrong and says so.

---

## THE STANDING HAZARD, and it bit again on 2026-08-19

**Three to five Claude sessions run in this one checkout at once.** Not a
theory — measured twice now:

- Two `land-gate.sh` runs executed simultaneously, both building `kernel.elf`
  and `zlOS.iso` into the same paths. Neither result meant anything.
- One session's `git add` swept another's in-progress `fb.c` and `HANDOFF.md`
  edits into its commit.
- Load average hit **15**, and `CLAUDE.md` records what that costs: a 12 s boot
  taking 30 s, a gate reporting a regression that did not exist, and the OOM
  killer taking an agent at 7.9 GB.
- **New, and the reason the working tree is the way it is:** while the session
  that wrote this was editing `fb.c`, `term.c`, `ui.c` and `kernel.zl`, another
  session landed four commits on the GPU track (`f8124a3..e09dc3f`) **and left
  an uncommitted de-duplication in `kernel/hosttest/build.sh`** — a file the
  first session also had to edit, to register two new harnesses. So
  `git diff kernel/hosttest/build.sh` currently shows two sessions' work
  interleaved. That is exactly the incident above, caught before the `git add`
  rather than after. **Nothing was committed. Look at that diff before you
  stage anything.**

**So, before anything:** `git log --oneline -5` and `git status`, and
`pgrep -fa land-gate.sh` before starting a gate. If another gate is running,
wait — a contended gate is worse than no gate. And **verify a claim in a doc
against the tree before building on it**, because the doc may be twenty minutes
old and already overtaken.

---

## Ranked

### 1. The GPU ring — *IN PROGRESS IN ANOTHER SESSION as of `e09dc3f`. Do not start a second one.*

The previous ranking had this as "the biggest thing, and it needs its own
session". It got one. Four commits landed while the rest of this list was being
worked:

```
f8124a3  feat(gpu): the ring experiment, ready to run - and its recovery is proven
2b53ec3  docs(gpu): how to run the ring experiment, and what each outcome means
f2c8bd3  feat(gpu): the blitter COPIES - the present path, verified on silicon
e09dc3f  perf(gpu): the present path measured - and it refutes the last commit
```

touching `kernel/gpu.c`, `hosttest/gpu_ring.c`, `hosttest/gpu-ring-run.sh`,
`gpu_blt.c`, `gputest.c` and [`gpu-blitter.md`](../../gpu-blitter.md). **Read
`gpu-blitter.md` for the current state rather than this paragraph** — this is a
pointer, not a summary, and the last of those four commit messages says it
refutes the one before it, so a summary written here would be stale by
construction.

The one thing worth repeating because it constrains the design: at 64×64 with
one blit per submission **the CPU wins by 8.4×**. That is submission cost, not
the engine. A blitter call per damage rect is the wrong shape.

### 2. PSR, and what the frame pacer is actually allowed to read — *newly on the list, and it is a decision*

`look-and-speed.md` §4's plan is to replace the uneven 100 Hz PIT release with a
measured 60 Hz deadline. #37 got `intel_find()` running at boot so the display
registers are mapped at all — and then found that the register the plan wants is
**frozen**. `hosttest/intel_probe`, read-only, against this laptop's own GPU:

```
  pipe A enabled  1
  frame counter   0 -> 0 in 0.5 s  =  0.0 Hz   <- frozen: PSR is on
  pixel clock     241690 kHz   (from PIPE_LINK_M1/N1 - exact, PSR-immune)
```

Firmware leaves Panel Self Refresh on (`EDP_PSR_CTL = 0x81F00406`), the pipe is
not fetching, and `PIPE_FRMCNT` does not advance. `intel.c`'s comment above
`intel_pixel_clock_khz()` says this, and `HANDOFF.md`'s pixel-clock section says
it harder — **both were written down and nobody had connected either to the
pacing plan:**

> **Do not trust the frame counter for this.** It is not reliably zero, it is
> *intermittently* zero — 0.0 Hz idle, a correct 60.0 Hz with a terminal
> scrolling. It passes in testing and returns 0 in the field.

**Intermittent is the part that decides this.** A frame-counter pacer works on
the desk with something moving on screen and stalls exactly when the desktop
goes quiet — the failure mode that never shows up while you are testing it.

So there are three ways forward and they are not equivalent:

1. **Pace off the pixel clock.** 241690 kHz against `2720×1481` total gives the
   59.998 Hz already recorded, and it is exact and PSR-immune. Needs no panel
   write. This is the one to try first.
2. **Turn PSR off** to get a live frame counter. That is a display-engine write
   on a panel `intel.c`'s hazard list is about. Not a casual experiment.
3. **A TSC deadline** with no display involvement at all — the option
   `visual-speed-northstar.md` lists first, and the only one that needs no
   Intel-specific anything.

**This is a decision, not a task.** Whoever takes it should say which, and why,
in `DECISIONS.md` before writing the pacer.

### 3. `gpu_ring` turns the land gate red on any box that is not root — *five minutes, and it belongs to the GPU-track session*

```
$ ./gpu_ring >/dev/null 2>&1; echo $?
2                       # "run me with sudo"
```

`gates/land-gate.sh` counts every non-zero exit that is not 77 as a FAIL, so the
gate is currently **RED with two failures and neither is a code defect** — this,
and the known `crypto.c`/`css.c` SOURCES gap from
`STATE-OF-THE-PROJECT.md` §2.3. Commit `e1eb44a` established `exit 77 = SKIP`
and [`gpu-blitter.md`](../../gpu-blitter.md) says in as many words: "the convention now
exists — use it for the next hardware harness." `gpu_ring` is the next hardware
harness. **Left alone deliberately** — it was another session's file and in
flight while this was written. Whoever owns that track: it is a one-line change
in `need_root`.

### 4. The two remaining northstar contradictions — *`DECISIONS.md` open items F and A, both Roy's*

- **F: mono versus proportional chrome.** The northstar says "everything inside
  the screen is mono"; the kernel deliberately moved every dock/menu/tray/title
  label to proportional DejaVu Sans, citing `desktop-look.md` item 4. A straight
  contradiction, not drift — one of the two documents has to lose. Note that
  item G's fix (#35) already settled the *terminal* half: it stays mono, and
  that half is not reopened by whatever F decides.
- **A: `.ultra/METRICS.json` has a blank primary metric** — modeset or desktop.
  They give different numbers. Logged as T-4.

### 5. The wallpaper arena, now that `fb_cache_reset()` works

#36 made the arena rewind on a mode change, which is what made the 16 MiB
compartment behave. It did **not** make it bigger, and #29's tension is still
Roy's: blur-then-wallpaper does not fit 16 MiB at 2560×1440, and the recommended
fix if the blur is wanted back is **(3), grow the arena** — move `HI_NVME` up,
16 → 20 MiB — not (2). `memmap.h` has the `_Static_assert`s to make that safe and
`hosttest/walltest` is where the new numbers get asserted.

### 6. Do NOT do these yet, and here is why

- **Raising the PIT.** 30+ dependent sites, and one is hardware-hazardous:
  `cpu.c:259` feeds `cpu_delay_us`, which enforces the panel's 500 ms T12
  delay. Introduce `PIT_HZ`/`idt_hz()` *first*. Full blast radius in
  `look-and-speed.md` §2, including the class `grep` will not find — window
  animations are counted in **frames**, so their wall-clock duration scales
  inversely with the tick rate.
- **Turning on SMP band rendering.** It is one call and it should not be made.
  Measured: **1.76× at four bands, not 4× — and two bands is *slower* than
  serial.** It only helps full-screen paths, which damage tracking made rare,
  and the AP park loop is a `pause` spin by design, so it burns three cores
  permanently. On this box that means every probe QEMU pins four host cores.

---

## What the last session closed, and what each one turned out to be

Two of these four were not what the ranking said they were, which is the reason
this section exists rather than a line saying "done".

| was ranked | closed as | what it actually was |
|---|---|---|
| 2. the two palettes | **#34**, item **E** closed | not a taste call at all — the reference's own header says it was transcribed *from* `kernel.zl`, so `ui.c` was simply the file that had drifted |
| 3. the clipped shell | **#35**, item **G** closed | wrapping, not width. And it is **75** columns, not the 77 §1c computed — `kernel.zl:2934` insets the client before `term_draw` sees it |
| 4a. `fb_cache_reset()` has no caller | **#36** | it had no caller because it was **incomplete**: rewinding the bump pointer alone would have been a use-after-free that *paints*. And it is a refusal, not a leak — one `mode` change turned the wallpaper cache off for the session |
| 4b. `intel_find()` never runs at boot | **#37** | done, and it immediately produced item 2 above — the counter it unblocks is frozen by PSR on the target panel |

**And #38 — what a different model family found in the other four.** Codex
reviewed the uncommitted diff for memory safety; four findings, all four
reproduced against the tree, all four real, one of them a P1 (`intel_find()`
truncating a 64-bit BAR, which #37 had just promoted from "only if you type `k`"
to every boot). **Three of the four were cases where this repo already held the
answer** — `pci_bar_hi()` existed in `pci.c`, the `x << 32` hazard was in
`CLAUDE.md`, the PSR intermittency was in `HANDOFF.md`. None needed new
information, only somebody who had not just written the code. Detail in #38.

Three new host gates, all watched going red before being believed:

| gate | what it pins | its negative control |
|---|---|---|
| `hosttest/palette` | one palette across the reference HTML, `kernel.zl` and `ui.c` — **parsed, not restated** | five checks on its own two parsers, because every assertion in it is an equality between two numbers it read, and a parser returning `-1` for everything would pass them all |
| `hosttest/termwrap` | no segment wider than the window; segments reassemble to the line | the pre-fix emission. **It found that the reassembly check alone is green against the bug** — one segment equal to the whole line does reassemble to it. The width check is the one that catches it |
| `hosttest/walltest` case C | the wallpaper cache survives a mode switch | **twice round the toggle, not once** — one hop fits in what is left and would have been green against the bug |

---

## The rules that actually caught things

- **A documented diagnosis is a claim.** `POINTER-PROMPT.md` §1b said the tray
  debris was "no background clear"; the clear existed and the real cause was a
  damage rectangle computed from a second origin. §1c said a screenshot came
  from a branch that already contained the regression, so every diff it
  recommended was guaranteed to return empty. **Honour a doc's ruled-out list —
  those cost real work — and re-derive its stated cause.** §1c's own column
  count was off by two for the same reason: it measured the client rect and not
  what `term_draw` is handed.
- **A stated consequence is also a claim.** #37's boot line first printed the
  frame counter as a liveness fact. One read-only run of `intel_probe` against
  the real GPU refuted it. **The refutation cost thirty seconds and the fact was
  already written in `intel.c`** — the expensive version of that mistake is
  finding it after building a pacer on top.
- **I broke my own rule and it is recorded**: `look-and-speed.md` §3 asserted a
  `term_draw` mechanism taken from a sub-agent summary that never went through
  the adversarial pass. It was false and had been for months. The correction is
  in place; the lesson is that the unverified claim is the one that gets
  promoted to "the next target".
- **A gate that cannot go red is decoration.** `gpu_blt` returns exit **77** for
  "no GPU here"; `land-gate.sh` called every non-zero a FAIL, so the convention
  was honoured by nobody. Fixed, and validated with stubs exiting 0, 77 and 1.
  **The 77 convention now exists — use it for the next hardware harness.**
- **A gate that cannot go red is not always the obvious one.** `termwrap`'s
  reassembly check is a perfectly sensible-looking assertion that is green both
  before and after the fix. It is kept, next to the one that works, precisely so
  nobody later deletes the effective check and keeps the decorative one.
- **QEMU cannot reach 2560×1440.** GRUB falls back to 800×600 and `set_res()`
  stops at 1920×1200, so no boot gate can see the ThinkPad's real mode.
  `hosttest/walltest.c` is the only thing that does. Anything panel-sized needs
  a host harness. **QEMU also has no Intel display controller at all**, so #37's
  `OK` branch cannot execute under any boot gate here — only the `no Intel GPU
  on the bus` branch does.
- **`fb:` is printed twice at boot** — once at console init, once after
  `set_res()`. Parse the **last** one or you will think the desktop runs at
  800×600.

---

## The two things to check on the first ThinkPad boot

1. **The wallpaper cache.** #29 removed the dock blur so it fits at 2560×1440.
   That is a **host** measurement of the shipping `fb.c`, not a boot — nobody has
   seen this desktop on the panel. The boot line must read:

   ```
   fb: wallpaper cached, 14400 KiB
   ```

   If it says `refused`, the decision was wrong and #29 has the three ways to
   fix it, with the recommended one named.

2. **The Intel line, which has never executed.** #37's `OK` branch is unreachable
   under QEMU. On the panel it should read:

   ```
   [  OK  ] Intel 0x9B41 mapped at boot, pipe 1, pixel clock 241690 kHz
   [ INFO ] PIPE_FRMCNT reads 0 - PSR freezes it, pace off the pixel clock
   ```

   The pixel clock is the number to check: `intel_probe` reads 241690 kHz from
   Linux userspace on this same GPU, so a boot that disagrees means the driver
   sees a different pipe than i915 does.
