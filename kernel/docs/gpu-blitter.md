# The Intel blitter — the first GPU work zlOS has ever done

2026-08-19. Measured on the target machine itself: this laptop **is** the
ThinkPad X1 Carbon Gen 8 the driver was written for. `LENOVO 20U90041AU`,
`8086:9B41` CometLake-U GT2, BAR0 at `0xE9000000` / 16 MiB — the same address
`intel.c`'s header comment records.

## Where this starts from

zlOS has never used a GPU for anything. `intel.c` is 5000+ lines of **display**:
GGTT, pipes, DPLL, EDID, AUX. Not one line makes the hardware draw. Every pixel
the desktop has shown came out of a CPU loop in `fb.c`, `fb3d.c`'s header says
"no GPU" in as many words, and `virtio_gpu.c` disables virgl on purpose.

Grep confirms it rather than assuming:

```
$ grep -rniE "blitter|XY_COLOR_BLT|batch buffer|command streamer|MI_" kernel/*.c
(nothing outside intel.c's display registers)
```

## What is now true

**The Intel blitter has filled a rectangle for this project, on real Gen9.5
silicon, verified by reading the pixels back.**

```
$ kernel/hosttest/gpu_blt --blit
  device        0x9B41  (CometLake-U GT2, Gen9.5 - the ThinkPad panel)
  HAS_BLT       1
  engines       4
    class 1 instance 0   BCS  copy/blitter
  batch         32 bytes: DW0=0x54300005 BR13=0x03F03C00
  inside        270000/270000 pixels are 0x60D2EB
  outside       0 pixels clobbered (want 0)
  ok    THE BLITTER DREW IT. Command stream is correct on this silicon.
```

The command stream is seven dwords and is now known-good on this hardware:

```
DW0  0x54300005   BLT client(2) | XY_COLOR_BLT(0x50) | WRITE_ALPHA | WRITE_RGB | len 5
DW1  BR13         ROP PATCOPY(0xF0) | 32bpp(3) | dst pitch in BYTES
DW2  y1<<16 | x1  top-left
DW3  y2<<16 | x2  bottom-right, exclusive
DW4  dst addr low 32
DW5  dst addr high 32
DW6  colour
```

## The decision that shaped this: we did NOT detach i915

`modeset-run.sh` detaches i915 and blanks the screen, because it reprograms the
**display**, which is i915's exclusively. The blitter is not the display — it is
a DMA engine that moves pixels between buffers. Submitting to it through
`/dev/dri/renderD128` is the ordinary supported path for any userspace program,
runs alongside the desktop, and cannot blank anything.

That splits the problem in the useful place:

1. **Is the command right?** ← the host harness. i915 owns the ring and
   schedules our batch on the real BCS engine. Seconds per run, no reboot, and
   it does not interrupt whoever is using the laptop.
2. **Can we own the ring?** ← the zlOS side, still to do — and it starts from a
   command stream that is already proven rather than one that is merely
   plausible.

This matters because the command encoding is the part most likely to be wrong
and the part hardest to debug from inside a kernel with no debugger. Getting it
wrong inside zlOS looks like a hang.

**No panel power is touched anywhere in this work, deliberately.** `intel.c`'s
hazard list (T12's 500 ms power-cycle delay, AUX into an unpowered panel) is
about hardware the blitter never addresses. If a change here starts needing
panel registers it belongs in a different file with a different review.

## The numbers, and how much to trust them

`gpu_blt --sweep`, same 3840x2160 surface for every row so the comparison is not
about allocation luck. K is blits per submission — **zlOS owns its own ring, so
the K=1 column is the cost of the ioctl, not of the blitter.**

At `-O2` (the CPU fill auto-vectorises to 16-byte stores):

```
  rect          Mpix   K   blitter Mpix/s   CPU Mpix/s   winner
    64x64       0.00    1        125            798      CPU 6.38x
    64x64       0.00   64       1733           1148      blitter 1.51x
  1024x768      0.79   64       3009           1066      blitter 2.82x
  1920x1200     2.30   64       3333            995      blitter 3.35x
  3840x2160     8.29   64       2939           1010      blitter 2.91x
```

At `-O3 -march=native` (AVX2), the CPU closes considerably:

```
  1920x1200     2.30   64       2284           1633      blitter 1.40x
  3840x2160     8.29   64       2344           1833      blitter 1.28x
```

**Read that carefully, because the first version of this benchmark got the
answer backwards.** Measuring a 1024x768 surface with one blit per submission
said "CPU is 1.75x faster". Both halves of that were artefacts: a 1 MiB surface
re-filled 200 times is L2-resident, so it measured cache rather than memory, and
one-ioctl-per-rectangle is the pessimal submission pattern and not the one zlOS
will use. On a 31.6 MiB surface the CPU drops from ~3200 Mpix/s to ~1000, which
is the memory system, not the loop.

**Honest summary: the blitter is worth somewhere between 1.3x and 3x on
compositor-sized fills, depending on CPU codegen — not the 3x+ the first sweep
suggested, and not the loss the very first one suggested.**

Every number above was taken with the box at **load average 11–14** (roughly
thirty concurrent agent sessions). That moves both columns and not equally. The
harness now prints the load next to every result and flags it above 2.0, so a
figure copied out of the output carries its caveat. **Re-run on a quiet box
before quoting any of these.**

### The throughput ratio is probably not the real argument

zlOS's compositor is paced by a 100 Hz timer and runs on **one** core — `smp.c`
brings the other three up only from a dead text-shell command, so SMP band
rendering has never run on a desktop boot (`docs/GUARDS-THAT-DID-NOT-GUARD.md`
§3). A blit that the GPU performs costs the CPU nothing at all, which is a
different and larger win than the Mpix/s ratio: it frees the core rather than
speeding it up. That argument does not depend on the contested benchmark and is
the one worth designing against.

## The verification can fail, and has been watched doing it

`XY_COLOR_BLT` has a documented silent failure: clear `BLT_WRITE_RGB` and the
hardware accepts the batch, runs it, reports no error, and writes nothing. A
checker that only looks for "no error" would call that a pass.

`--negative` plants exactly that defect:

```
$ kernel/hosttest/gpu_blt --negative
  batch         32 bytes: DW0=0x54200005      <- WRITE_RGB cleared
  inside        0/270000 pixels are 0x60D2EB
  submit+wait   0.652 ms                      <- it RAN, and succeeded
  FAIL  the blit did not produce the expected rectangle
        nothing was written at all - the batch ran but drew nothing
  ok    the check REJECTED a blit that wrote nothing - it can fail
```

The destination is also poisoned with `0xDEADBEEF` before every run, so "filled
with the right colour" can never be confused with "never ran", and pixels
outside the rectangle are checked to still hold the poison.

## What zlOS needs next, in order

1. **A ring buffer and a way to submit to it.** This is the real work. `intel.c`
   already has `intel_ggtt_map()` and `intel_ggtt_size()`, so mapping our own
   pages where the engine can reach them is built; what is missing is the BCS
   ring (RING_TAIL/HEAD/START/CTL at the engine's MMIO base), and something to
   wait on completion.
2. **Decide GGTT vs ppGTT.** The harness softpins into a ppGTT because i915
   provides one. zlOS has no ppGTT and does not need one — the global GTT it
   already programs for scanout is enough for a blitter with one client.
3. **Point it at the back buffer.** `fb.c`'s back buffer lives at `HI_BACK`
   (128 MiB) and is what the compositor fills. That is the surface a blit should
   target, and its pitch already matches what BR13 wants (bytes, 16-bit field —
   a 3840-wide 32bpp surface is 15360, close to the format ceiling of 65535).
4. **Then, and only then, wire `fb_fill_px` to it** behind a flag, and re-run
   `fbbench` — because the number that decides this is the frame time, not the
   fill rate.

## Hazards found on the way

- **Softpinned addresses are yours to get right.** The first run failed with
  `ENOSPC`, "No space left on device", which reads like the GPU is out of
  memory. It was not: the 3 MiB destination at 2 MiB ran to 5 MiB and the batch
  was pinned at 3 MiB, inside it. There is now a `_Static_assert` on that,
  the same discipline `memmap.h` imposes on the kernel's high-RAM map and for
  exactly the same reason.
- **The pitch field is 16 bits.** Anything wider than 16383 pixels at 32bpp
  cannot be expressed. Not a limit today; it will be one for a tiled or very
  wide surface.
- **This part has LLC**, so a WB mapping is coherent and a plain read after
  `GEM_WAIT` sees the blit. A part without LLC would need WC plus explicit
  domain flushes. zlOS should not inherit the assumption casually.
- **`exit 77` meant "skip" to nobody but this file.** `gpu_blt.c:624` returns 77
  when there is no `/dev/dri/renderD128`, commented "77 = skip, not fail" — the
  autotools convention. `gates/land-gate.sh`'s harness loop treated *every*
  non-zero exit as a failure, so on any box with the drm headers and no Intel
  render node the whole land gate would have gone red, blaming a harness that
  had correctly declined to run. Fixed in the loop, not in the harness: 77 is
  now a counted, printed **SKIP**, distinct from a pass, because "27 passed"
  when three of them did nothing is the false green that gate exists to stop.
  Validated in all three directions with stub scripts exiting 0, 77 and 1.

  This is the same class as everything in `docs/GUARDS-THAT-DID-NOT-GUARD.md`:
  a guarantee stated in a comment and implemented by no consumer. Worth knowing
  for the next hardware-dependent harness — **the convention now exists, so use
  77 rather than inventing a second one.**
