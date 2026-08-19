# NEXT-PROMPT — what to pick up, ranked, with the evidence for the ranking

Written 2026-08-19 at the end of the look-and-speed session. Read
[`docs/STATE-OF-THE-PROJECT.md`](../../docs/STATE-OF-THE-PROJECT.md) for
everything open; this file is narrower — it is **which one to do next and why**,
with the measurement that ranks it.

---

## Read these three first, in this order

1. [`visual-speed-northstar.md`](visual-speed-northstar.md) — Roy's own statement
   of what the v10 HTML is *for*. It outranks every execution brief in this
   folder. Its rule: say which layer you are judging — **look, feel, machinery,
   or applications** — and never answer with one completeness percentage.
2. [`DECISIONS.md`](DECISIONS.md) #29–#33 — the kernel is now settled against the
   northstar item by item. **#29 carries an unresolved tension that is Roy's to
   call**, not the next session's to quietly re-decide.
3. [`look-and-speed.md`](look-and-speed.md) — the frame budget, where the time
   goes, and the vsync survey. Note its two correction banners: one entry in it
   was wrong and says so.

---

## THE STANDING HAZARD, and it bit this session repeatedly

**Three to five Claude sessions run in this one checkout at once.** Not a
theory — measured on 2026-08-19:

- Two `land-gate.sh` runs executed simultaneously, both building `kernel.elf`
  and `zlOS.iso` into the same paths. Neither result meant anything.
- One session's `git add` swept another's in-progress `fb.c` and `HANDOFF.md`
  edits into its commit.
- Load average hit **15**, and `CLAUDE.md` records what that costs: a 12 s boot
  taking 30 s, a gate reporting a regression that did not exist, and the OOM
  killer taking an agent at 7.9 GB.
- Two sessions independently found the same `smp_go()` fact and wrote it up
  twice.

**So, before anything:** `git log --oneline -5` and `git status`, and
`pgrep -fa land-gate.sh` before starting a gate. If another gate is running,
wait — a contended gate is worse than no gate. And **verify a claim in a doc
against the tree before building on it**, because the doc may be twenty minutes
old and already overtaken.

---

## Ranked

### 1. Own the ring — the GPU track. *The biggest thing, and it needs its own session.*

`kernel/gpu.c` emits a correct blitter command stream and `gputest` pins it.
`hosttest/gpu_blt` proved the encoding on real Gen9.5 silicon: 270000/270000
pixels, and its `--negative` control has been watched going red. What is missing
is everything between "we can write the command" and "the hardware ran it
because *we* submitted it":

- the BCS ring — `RING_TAIL`/`HEAD`/`START`/`CTL` at the engine's MMIO base;
- something to wait on completion;
- GGTT rather than ppGTT (`intel_ggtt_map()` already exists; zlOS has one
  client and does not need a ppGTT);
- then point it at `HI_BACK` and wire `fb_fill_px` behind a flag.

**Why it is its own session:** this is the part that genuinely needs i915
detached, and detaching it blanks the screen. Do not start it inside a session
that is also doing desktop work.

**And read the numbers before designing:** at 64×64 with one blit per submission
the **CPU wins by 8.4×**. That is submission cost, not the engine. A blitter call
per damage rect is the wrong shape — batch the frame's rects, or keep small
fills on the CPU. Full encoding, hazards and figures:
[`gpu-blitter.md`](gpu-blitter.md).

### 2. The two palettes — *the largest visual divergence, and it is one decision*

`DECISIONS.md` open item **E**. Two palettes ship simultaneously:

| | paints | agrees with the northstar |
|---|---|---|
| `kernel.zl`'s `rgb()` constants | header bar, dock, two legacy app bodies | 11 of 21 tokens |
| `ui.c`'s `ui_theme` struct | **every window frame on screen** | 2 of 10 roles |

`ui_theme`'s panel is `rgb(30,42,68)` against the northstar's `rgb(5,6,10)`, and
two different cyans are on screen at once — `ACCENT` `rgb(96,210,235)` for the
header and dock, `ui.c`'s `rgb(85,214,255)` for window focus.
`visual-speed-northstar.md` names "duplicated palette roles" as a thing that
must not ship. **This is a decision, not a bug fix** — pick the source of truth,
then make the other follow.

### 3. The clipped shell — *open item G, and §1c's other half*

At 1920×1200 the shell client is 1236 px = 77 columns of 16 px, and the longest
`help` line needs ~82, so two rows are cut mid-glyph. Cause is measured and
settled (`POINTER-PROMPT.md` §1c): the terminal went monospace at `663a110` and
that is **right** — a terminal is monospace, the northstar agrees, and three
space-aligned tables depend on it. So the fix is **width or wrapping, not font.**

### 4. Two small ones with real consequences

- **`fb_cache_reset()` has no caller.** The arena never rewinds, so one mode
  switch leaks the previous wallpaper — 8.8 MiB of 16 at 1920×1200. It is the
  same arena that `DECISIONS.md` #29 is about.
- **`intel_find()` never runs at boot**, so `intel_frame_count()` returns 0 for
  the whole desktop session. Read-only, safe, and it is what unblocks any vblank
  work at all.

### 5. Do NOT do these yet, and here is why

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

## The rules that actually caught things this session

- **A documented diagnosis is a claim.** `POINTER-PROMPT.md` §1b said the tray
  debris was "no background clear"; the clear existed and the real cause was a
  damage rectangle computed from a second origin. §1c said a screenshot came
  from a branch that already contained the regression, so every diff it
  recommended was guaranteed to return empty. **Honour a doc's ruled-out list —
  those cost real work — and re-derive its stated cause.**
- **I broke my own rule and it is recorded**: `look-and-speed.md` §3 asserted a
  `term_draw` mechanism taken from a sub-agent summary that never went through
  the adversarial pass. It was false and had been for months. The correction is
  in place; the lesson is that the unverified claim is the one that gets
  promoted to "the next target".
- **A gate that cannot go red is decoration.** `gpu_blt` returns exit **77** for
  "no GPU here"; `land-gate.sh` called every non-zero a FAIL, so the convention
  was honoured by nobody. Fixed, and validated with stubs exiting 0, 77 and 1.
  **The 77 convention now exists — use it for the next hardware harness.**
- **QEMU cannot reach 2560×1440.** GRUB falls back to 800×600 and `set_res()`
  stops at 1920×1200, so no boot gate can see the ThinkPad's real mode.
  `hosttest/walltest.c` is the only thing that does. Anything panel-sized needs
  a host harness.
- **`fb:` is printed twice at boot** — once at console init, once after
  `set_res()`. Parse the **last** one or you will think the desktop runs at
  800×600.

---

## The one thing to check on the first ThinkPad boot

`DECISIONS.md` #29 removed the dock blur so the wallpaper cache fits at
2560×1440. That is a **host** measurement of the shipping `fb.c`, not a boot —
nobody has seen this desktop on the panel. The boot line must read:

```
fb: wallpaper cached, 14400 KiB
```

If it says `refused`, the decision was wrong and #29 has the three ways to fix
it, with the recommended one named.
