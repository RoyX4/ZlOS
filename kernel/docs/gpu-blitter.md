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

### Re-run at load 3.81 — the caveat was right, and the honest range is narrower

The instruction above was followed rather than left as advice. Same binary, same
`--sweep`, box at **3.81** instead of 11–14:

```
    rect        Mpix   K   blitter Mpix/s   CPU Mpix/s   winner
      64x64     0.00    1        201           1687      CPU 8.39x
      64x64     0.00   64       2530           2213      blitter 1.14x
     256x256    0.07   64       3310           1857      blitter 1.78x
    1024x768    0.79   64       3178           1499      blitter 2.12x
    1920x1200   2.30   64       3339           1704      blitter 1.96x
    3840x2160   8.29   64       3424           1925      blitter 1.78x
```

Two things change and one does not.

1. **The blitter column barely moves** — 3339 vs 3333 Mpix/s at 1920x1200. It is
   a DMA engine; host load is not its problem.
2. **The CPU column is what load was distorting**, 995 -> 1704 Mpix/s at the same
   size. So the contended runs were flattering the GPU, not the CPU.
3. **So the honest headline is ~1.8–2.1x at compositor sizes**, not 2.8–3.35x.
   That sits inside the "1.3x to 3x" summary above, which stands, but the top of
   that range was a load artefact and should not be quoted.

**And the small-rectangle result is the one to design around:** at 64x64 with one
blit per submission the CPU wins by **8.4x**. That is the ioctl, not the engine —
the same rect at K=64 wins by 1.14x. zlOS owns its ring and pays no ioctl, but it
does pay a submission and a wait, so **a blitter call per small damage rect is
the wrong shape**. Batch the frame's rects into one submission, or keep small
fills on the CPU.

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

## The kernel emits it now — `kernel/gpu.c`

The command builder lives in `kernel/gpu.c` and is compiled into all four
targets (`nm kernel.elf | grep gpu_fill_rect` confirms it links, rather than
assuming the build implies it). **`hosttest/gpu_blt.c` `#include`s it**, so the
thing proven on silicon and the thing that ships are one implementation, not two
that agree by inspection.

`gpu.c` builds commands and does not submit them. It touches no MMIO, no ring
register, nothing gated behind `lt_armed`. That is what makes it testable
everywhere and safe to land before the ring exists.

`hosttest/gputest.c` pins its output to the dwords that really drew — 35 checks,
including a `0xC0FFEE00` canary past the batch capacity. Both halves were
watched failing before it was committed:

| planted defect | caught by |
|---|---|
| drop `BLT_WRITE_RGB` (the silent hardware no-op) | golden DW0 + an explicit bit check — 2 failures |
| bounds-check per dword instead of per command | **the canary alone** — a partial command left in the tail |

The second is the one worth noticing: a test that only checked the return value
of `gpu_fill_rect` would have passed a version that writes a truncated command
into the batch. In the kernel that is a DMA engine parsing whatever followed it.

## What zlOS needs next, in order

0. ~~A command emitter~~ — done, `kernel/gpu.c`, verified on silicon and pinned
   by `gputest`.
1. **A ring buffer and a way to submit to it.** This is the real work, and it is
   the part that genuinely needs the hardware with i915 detached — which blanks
   the screen, so it is its own session. `intel.c`
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

## The BCS register map, read off this running GPU — not from a datasheet

The next step needs these addresses, and guessing them is how a bring-up loses a
day. `i915_engine_info` reports what the hardware actually has, on this exact
part, right now:

```
$ sudo cat /sys/kernel/debug/dri/0/i915_engine_info
rcs0    MMIO base: 0x00002000     render
bcs0    MMIO base: 0x00022000     THE BLITTER - this is the one
vcs0    MMIO base: 0x00012000     video decode
vecs0   MMIO base: 0x0001a000     video enhance
```

`bcs0`, idle, with i915 holding it:

```
RING_START: 0xfffe4000     RING_HEAD: 0x00000300    RING_TAIL: 0x00000300
RING_CTL:   0x00000000     RING_MODE: 0x00000200 [idle]
RING_IMR:   0xf6f7ffff     RING_ESR:  0x00000000
EL_STAT_LO: 0x00000301     EL_STAT_HI: 0x00000000
```

Two things follow, and both shape the next session:

1. **The legacy ring registers are real and live on Gen9.** `RING_START`,
   `RING_HEAD`, `RING_TAIL`, `RING_CTL` all exist at `base + 0x30..0x3C` and hold
   sensible values. zlOS can address them.
2. **i915 drives this part through EXECLISTS, not the legacy ring** — all four
   engines report `EL_STAT`, and `RING_CTL` reads 0 while idle because the
   context, not the driver, owns the ring. So the open question for zlOS is
   whether a sole owner can still program the legacy ringbuffer path directly on
   Gen9.5, as i915 itself did on Gen8 before it switched. **Not answered here.**
   It is answerable in one experiment, and that experiment needs the GPU to
   itself.

### Why the next step is blocked, specifically

Everything so far ran alongside i915 on purpose, and that is now exhausted:
building a command stream needs no ownership, but **programming `RING_CTL` does**.
Two ways to get it, both needing a human:

- **Detach i915** (`modeset-run.sh`'s pattern, EXIT-trap recovery). This blanks
  the screen of whoever is using the laptop, so it is not something to start
  while someone is working.
- **Boot zlOS on the ThinkPad from USB.** `kernel/docs/thinkpad-first-boot.md`
  has the procedure, including the two things that waste an hour.

Until one of those happens, `gpu.c` is as far as this can honestly go: the
command stream is correct on silicon, pinned by `gputest`, compiled into all
four targets, and calling nothing.

## The experiment is written and waiting — three commands, in this order

`kernel/hosttest/gpu_ring.c` + `gpu-ring-run.sh`. Everything that can be proven
without taking the GPU already has been (see below), so only the ring write is
left.

```bash
cd kernel/hosttest
sudo ./gpu-ring-run.sh --survey    # read-only, i915 stays bound. Zero risk.
sudo ./gpu-ring-run.sh --dry       # unbinds and rebinds, writes NOTHING.
sudo ./gpu-ring-run.sh --ring      # the real thing.
```

**Do not skip `--dry`.** It is the recovery rehearsal: the screen goes dark, no
register is written, and the only thing under test is whether the display comes
back. If it does not, `--ring` must not be run.

### What is already proven, with i915 still loaded

```
BAR0 mapped            8 MiB of 0000:00:02.0
forcewake blitter      acked
bcs0 ring              TAIL=0x420 HEAD=0x420 START=0xFFFE4000 CTL=0
pagemap                va -> phys 0x362388000
```

Forcewake is the one that would have failed silently: on Gen9 a read of a
sleeping GT power well returns 0 and **a write is discarded**. Without holding
it, every ring write would vanish and the run would report "the hardware ignored
me". It acks.

Note the physical address is **above 4 GiB**. The GGTT PTE writes the high half
into `pte[1]`; a 32-bit type there would have truncated it silently — the same
class as the `-Werror` guard work earlier the same day, in a different place.

### The runner survives the session it kills

`modeset-run.sh` says to run it from a TTY, because stopping the display manager
kills the terminal — and with it the recovery trap. This machine runs a Wayland
session (`loginctl`: `Type=wayland`) with many agent sessions inside it, all of
which die with the display manager. So `gpu-ring-run.sh` re-execs itself under
`setsid` onto a log file **before** stopping anything. The trap then belongs to a
process with no session to lose.

**Recovery is proven, not asserted.** The survey run was killed with `SIGTERM`
mid-flight and the trap fired:

```
== restoring ==
  i915 already bound
  brightness restored to 21842
  lightdm restarted
```

### What the result will mean

`gpu_ring --ring` reports one of three things, and all three are useful:

| outcome | meaning |
|---|---|
| pixels land | a sole owner **can** drive the legacy ring; zlOS's path is `RING_START/CTL/TAIL`, no execlists |
| `RING_CTL` will not enable | Gen9.5 refuses legacy submission; zlOS needs the execlist path |
| HEAD reaches TAIL, no pixels | submission works, addressing is wrong — a GGTT problem, not a ring problem |
