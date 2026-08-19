# What to do about the GPU — the decision, with the numbers behind it

2026-08-19. Written after taking the blitter all the way to working silicon and
finding out it was the wrong engine. `kernel/docs/gpu-blitter.md` is the full
account; this is the part someone has to decide.

Every number here was measured on this machine, which **is** the target: a
ThinkPad X1 Carbon Gen 8, `8086:9B41` CometLake-U GT2.

## The landscape, measured

`fbbench` on the shipping `fb.c`, and `gpu_fillrate` on the same GPU:

| operation | CPU (fb.c) | GPU | ratio |
|---|---|---|---|
| plain fill | 3486 Mpix/s (0.66 cyc/px, SSE) | ~3600 (blitter) | **~1x** |
| copy / present | 1506 Mpix/s into WC | 1274 (blitter) | **0.85x — CPU wins** |
| gradient | 1449 Mpix/s (1.53 cyc/px) | ~5900 | ~4x |
| `fill_blend` a=160 | **122 Mpix/s** (18.89 cyc/px) | 5930 | **48x** |
| radial glow | **62 Mpix/s** (36.97 cyc/px) | 5930 | **96x** |

The shape of it: **the CPU is at memory bandwidth for anything write-only, and
falls off a cliff the moment a pixel has to be read, multiplied and written
back.** That cliff is where the northstar's entire visual language lives — alpha
blends, gradients, soft shadows.

At the panel's own 2560x1440, gradient (2.94 ms) and present (3.25 ms) alone are
**6.2 ms of a 16.67 ms frame**, before a single glyph or blend.

## The three options

### 1. SMP band rendering — do this first, regardless

**1.78x on the whole desktop redraw**, from code already written, already in the
tree, and switched off. `fb.c` has `fb_par_hook`/`fb_par_run`, `smp.c:265` calls
it properly, and the only missing piece is that `smp_go()` is reachable solely
from the old text shell's `*` key, so `smp_start()` never runs on a desktop boot.

- **Cost:** approximately one call, in `kernel.zl`'s boot path.
- **Risk:** real but bounded. Only one of the seven boot gates runs more than one
  core (`verify-efi.sh`, `-smp 2`); the other six get QEMU's default of one. And
  `kernel.zl:1874` says out loud that the APs "park immediately: nothing is
  lock-protected yet" — their only job would be `smp_band_dispatch`, and that
  needs to be true rather than assumed.
- **Caveat:** 2 bands ≈ 4 bands in every run. The redraw is bandwidth-bound, so
  do not expect 4x from four cores.

### 2. The render engine — the only path to the 48x

RCS, MMIO base `0x2000`, through the 3D pipeline. This is what Mesa uses to hit
5.93 Gpix/s blended above.

- **Cost: weeks, not an afternoon.** The blitter needed a ring and seven dwords.
  RCS needs a ring, `3DSTATE_*` pipeline state, surface state and binding tables,
  a vertex buffer, and a pixel shader.
- **The unresolved risk is the shader.** There is no Gen9 ISA assembler in this
  tree, and **none on this box** — checked: no `intel_clc`, no `aubinator`, and
  `INTEL_DEBUG=fs` dumps nothing from the installed Mesa. So the shader has to
  come from somewhere: installing Intel's tools, hand-encoding Gen9 ISA, or
  embedding a blob built elsewhere. **Settle that before committing to this.**
- **What is already de-risked:** GGTT mapping, forcewake, ring registers and the
  submission model are the same for RCS as for BCS. `gpu_ring.c` covers all of it
  and its survey passes.

### 3. The blitter ring — not justified by fill rate

`gpu.c`, `gputest` and `gpu-ring-run.sh` are written, correct, and proven
pixel-for-pixel on silicon. But the measurement says a blitter fill ties the CPU
and a blitter copy loses to it. **Build this only for a reason other than speed** —
a 4K external panel, where its ~1.15x becomes real, or as the stepping stone to
option 2, which it genuinely is.

## The recommendation

1. **Turn SMP bands on and re-measure.** Best ratio of win to risk on the board.
2. **Answer the shader question** — one afternoon of finding out whether a Gen9
   pixel shader can be obtained on this box. It is the gate on the whole 48x.
3. **Then decide on RCS** with that answer in hand.

Do not build the blitter ring for performance. It was the right first experiment
— it made the GPU draw for this project for the first time, and it proved the
whole submission model cheaply — but its own numbers say the compositor will not
notice it.

## What this cost, and what it bought

An afternoon, and the answer to "should zlOS use the GPU" changed three times
under measurement:

1. *The blitter beats the CPU 2.8–3.35x* — wrong; the box was loaded and the
   CPU column was a naive loop.
2. *The blitter loses to `fb.c`* — right, and it kills the fill argument.
3. *The GPU wins 48x, but on a different engine* — the actual answer.

Every reversal came from measuring the thing rather than the thing next to it.
The first two would each have justified a decision, and both would have been
wrong.
