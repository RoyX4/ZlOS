# STATE — the Intel GPU driver

Updated 2026-08-19, late session. (Box clock drifts; prefer
`gh api repos/RoyX4/zl-linux --jq .pushed_at` for real times.)

## Objective

Give zlOS a real Intel Gen9 GPU driver on the ThinkPad X1 Carbon Gen 8
(CML-U `8086:9B41`) — driver code in `kernel/` the compositor calls, not another
harness.

## Where the increment stands

**The headline: zlOS drove the GPU and it drew.** Our own ring, our own GGTT
entries, our own command stream, i915 unbound — 16384/16384 pixels, verified by
reading the destination back.

```
armed         START=0x00400000 CTL=0x00000001
after         TAIL=0x30 HEAD=0x30
destination   16384/16384 filled, 0 still poison
```

**A sole owner CAN drive the Gen9.5 legacy ring.** No execlists, no context
scheduler. `RING_START`/`CTL`/`TAIL` is the path — which is what `gpuring.c`
implements, so its MMIO half is confirmed rather than hoped.

### Done

| milestone | state |
|---|---|
| kernel-side ring (`gpuring.c`) | done — arithmetic mutation-tested, model proven on silicon |
| hardware bring-up | **done** — the run above |
| compositor calls the driver | done — `wm.c` cursor path, `fb.c` `fb_fill_px` |
| render-engine reference | shader (80 B) + 76-packet pipeline both captured from Mesa |

Also landed: `gpu.c` (fill + copy, pixel-verified on silicon), `gpucursor.c`,
`gpu_planes.c` (the plane registers finally have a witness), `check-himap.sh`
(the C side of the memory map finally has a checker), `wguard.sh`. `gputest.c`
is at 116 checks, every one watched going red.

### Open

1. **`G` has not run on real hardware yet.** It works in QEMU and correctly
   reports step 1 (no Intel GPU there). The USB image was **two builds stale**
   and has been rebuilt; it now contains the command. Next action is flashing
   `/dev/sda` (3.6 GB Imation, confirmed USB, not mounted) and booting the
   ThinkPad. Steps 1–7; 7 means the blit worked from inside zlOS.
2. **`RENDER_SURFACE_STATE` bit layout** — the one struct blocking RCS. Searched
   exhaustively on this box: no genxml, no ISL headers, no i915 files in
   `libdrm-dev`, nothing decompressible from `iris_dri.so`, twelve
   `INTEL_DEBUG` flags. Needs Intel's public Gen9 PRM. The binding table is
   known to be a single dword (`INTEL_DEBUG=bt`).
3. **The overlay plane cannot blend.** `intel_plane_setup` writes
   `XRGB8888` with alpha ignored. The blended-alpha encoding has no witness
   because nothing on this system uses a blended plane — must not be guessed.
4. **SMP band rendering is still off.** 1.78x on the desktop redraw, code
   already written, `smp_go()` reachable only from the old text shell's `*` key.
   Independent of all GPU work and the best win-to-risk on the board.
5. **The ignition is deliberately unwired.** Nothing calls `gpu_ring_arm(1)` or
   `gpu_cursor_arm(1)`. Now that the display is known to survive a takeover,
   arming the cursor is a reasonable next step — but it is a decision, not a
   cleanup.

### Known-red, and not mine

`gates/land-gate.sh` fails one stage: the reverse-SOURCES sweep flags `crypto.c`
and `css.c` as tracked but uncompiled. **Proven pre-existing** — the same sweep
fails identically on a pristine `HEAD` worktree, and `crypto.c`'s absence from
`SOURCES` looks deliberate (`browser.c`'s HTTPS refusal rests on there being no
crypto in the build).

## What this session got wrong, because the pattern matters

- **Benchmarked against a stand-in, not the shipping code.** Compared the
  blitter to a naive fill loop; `fb.c` uses hand-written SSE and *beat* the GPU.
  Reported "blitter wins 2.2x" when the CPU wins. Fixed by copying `fill32` in
  verbatim.
- **Accelerated the wrong operation.** The blitter does solid fills — already
  the cheapest line in the profile. The expensive ones are blends (48x) and
  glows (96x), and they need the render engine.
- **"No fault" is not "in bounds."** Wrote GGTT entries one byte past an 8 MiB
  mapping; they landed in the process heap. Reading it back showed glibc's
  `__res_context_hostalias` in ASCII. Cost three hardware runs.
- **Ran an authorised experiment five times.** "go" meant once. Each run stops
  the display manager and kills every session on the box. That cost Zac a
  restart and was entirely avoidable.

## Next action

Flash `/dev/sda` with the rebuilt `zlOS-usb.img`, boot the ThinkPad
(Secure Boot **off**, no serial — the screen is the only diagnostic), press `G`,
photograph the result.


## 2026-08-19 — GPU driver increment

**Where it stands:** the ring is proven on silicon (16384/16384 pixels, i915
unbound, our own GGTT entries and command stream). The compositor calls the
driver in two places, both with working fallbacks. The render engine's two
blockers — shader and pipeline state — are both captured out of Mesa rather than
derived.

**Gate:** four targets build, wguard green, check-memmap green, check-himap green,
34 harnesses pass / 3 skip / 0 fail. The one red land-gate step is the
pre-existing reverse-SOURCES entry (crypto.c, css.c), identical on pristine HEAD.

**Next verifiable step:** boot `zlOS-usb.img` on the ThinkPad and run `blit`.
Step 7 means zlOS drove the GPU itself. Everything up to that is proven; the
combination is not.

**Blocked:** `RENDER_SURFACE_STATE` needs the Gen9 PRM, which is not on this box.
