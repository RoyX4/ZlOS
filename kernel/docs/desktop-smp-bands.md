# SMP band rendering — what it does, and why it is not 4×

Queue item 7 (G2). Three of four cores sat parked in `cli; hlt` since `smp.c`
was written. They render now. This is what that bought, what it cost, and the
part of the original idea that turned out to be impossible.

**The headline is not 4×.** The brief asked for the real number either way, and
names `DECISIONS.md` #25 — an optimisation argued from an instruction count,
shipped, and measured 25% *slower* — as the thing not to repeat. So: measured
in both directions, on a box that was never quiet, and reported with its error
bars rather than its best run.

---

## 1. The idea that does not work, and why it matters

The obvious reading of "split the back buffer into disjoint horizontal bands"
is: run the **whole draw list** on N cores, each with its clip set to its own
band. Disjoint writes to plain RAM, no lock needed.

That is not possible here, and the reason is not concurrency.

**The draw list is `app_draw`, which is zl.** The zl runtime has global state —
the builtin dispatch, the value stack, the string table — none of it reentrant.
Four cores inside `zl_fn_app_draw` would corrupt the *interpreter*, long before
they got near the framebuffer. Making that work needs either

- a display list the C side can replay once per band, or
- a reentrant zl runtime,

and neither exists. This is a **language/runtime limit, not a missing
optimisation** — worth stating plainly, because "SMP band rendering" sounds
like a rendering problem and it is not one.

## 2. What is parallel instead

Each large **primitive**, internally. A call to `fb_gradient` or `fb_shadow`
already owns a rectangle and already writes it row by row with no dependence
between rows, so it can split its own work without anything above it knowing.

| primitive | banded | why |
|---|---|---|
| `fb_gradient` | yes | row colour derives only from the row index |
| `fb_shadow` | yes | per-pixel falloff, no row dependence |
| `fb_fill_px` | yes | rows are independent stores |
| `fb_present` | yes, ≥64 rows | disjoint reads, disjoint writes |
| `fb_text_aa`, `fb_rrect`, `fb_icon24` | no | small rectangles; barrier costs more than the work |

Everything else stays serial **by necessity, not oversight**: the `put_pixel`
path grows a shared damage accumulator, and the damage list is the one piece of
mutable state a band may never touch. Damage is reported **once, by the calling
core, after the barrier**.

## 3. Disjoint by construction

The bands are the gaps between a monotonic edge list whose ends are pinned to
the rectangle, so overlap and gaps are not expressible, never mind leavable by
accident. `fb_bands_check()` proves it anyway and `fbbench` asserts it at every
mode, for 1–8 bands, including a deliberately awkward range that does not
divide evenly and an empty one.

```
bands tile exactly (1..8, odd ranges) ok
```

The remainder is spread one row per band rather than dumped on the last one:
with a barrier at the end, a frame costs whatever the **slowest** band costs.

**Flags are one cache line apart.** Two cores bumping `done` in the same 64-byte
line ping-pong that line on every store, and the loop ends up slower than serial
with nothing in the code to show why. It is the most common way band rendering
fails to pay, and it is invisible.

## 4. The barrier

`fb_present` must not blit a row a core is still writing into. The dispatcher
does not return until every band has reported done — that is the whole contract,
and it is why the dispatch hook is a call and not a queue.

Band 0 runs on the **calling** core rather than the BSP sitting in the barrier
watching three cores work.

## 5. The park loop had to stop being `hlt`

A core halted with interrupts off can only be restarted by NMI/INIT/SIPI, so
there is no way to hand it work without an interrupt path this kernel does not
have. The APs spin on a sequence number instead.

**That burns a core.** It is the honest cost of having no scheduler — those
cores were doing nothing before either, just more quietly. Band rendering is
off until `smp` is typed, so nothing about a normal boot changes; `verify.sh`
boots `-smp 1` and never gets there.

## 6. Correctness: the same pixels, twice, two different ways

**Host** (`hosttest/fbbench.c`, pthread pool): the FNV scene hash is recomputed
at every band count and compared to the serial one. Identical at 1, 2, 3 and 4
bands, at all three resolutions:

```
1920x1200   8473499efb49abb1
2560x1440   81c4be85c58763e7
3840x2160   e735d8737eeff842
```

**Kernel** (`probe-smp.py`): boots the real thing, wakes the cores with
INIT/SIPI, and renders the same shell content before and after.

```
ok    the other cores woke up   cores online now: 4 of 4
ok    banded rendering is pixel-identical   0 of 961608 pixels differ across 4 cores
```

That second one is the one that matters. The host pool never touches `smp.c`,
the trampolines, the AP stacks, or a core woken by an IPI pair — everything that
can actually go wrong with three real cores writing one buffer lives on the
other side of that line.

## 7. The numbers, and the error bars

`fbbench`, 1920×1200, best of 16 (noise can only make a run slower, so the
minimum converges from above). **The box was never quiet** — several other
agents were running throughout, load average 5–12 on 8 logical cores — so these
are a **floor with wide error bars**, not a clean bench.

| run (load) | serial | 2 bands | 3 bands | 4 bands | best speedup |
|---|---|---|---|---|---|
| A (10.9) | 5.248 ms | 4.811 | 3.850 | **2.949** | 1.78× |
| B (11.7) | 4.833 ms | 3.790 | 3.668 | 3.132 | 1.54× |
| C (12.4) | 8.042 ms | 4.583 | 3.651 | 3.059 | 2.63× |
| D (5.3) | 7.489 ms | 4.706 | 5.536 | 6.811 | 1.10× |

Taking the **best observed** time in each column, which is the least-interfered
estimate of each:

```
whole desktop   serial 4.833 ms  ->  4 bands 2.949 ms     1.64x
gradient        serial 1.974 ms  ->  4 bands 0.981 ms     2.01x
present         serial 1.883 ms  ->  4 bands 1.326 ms     1.42x
shadow          serial 0.634 ms  ->  4 bands 0.505 ms     1.26x
```

**Run D is in the table on purpose.** At the *lowest* load it produced the
*worst* result, with 4 bands slower than 2 — spinning workers competing with
other processes for the same cores. A table showing only runs A–C would be a
nicer number and a false one.

### Why it is not 4×

1. **Amdahl.** At 1920×1200 the parallelised parts of the desktop scene are the
   full-screen gradient (~2.0 ms), three shadows (~0.6 ms each) and the dock
   gradient — roughly 80% of a ~4.8 ms frame. Even at a perfect 4× on that
   share the ceiling is ~2.6×, before any overhead at all.
2. **Nine dispatches per frame.** The scene makes nine banded calls, so it pays
   nine barriers. A barrier is cheap; nine of them against 4.8 ms is not free.
3. **Memory bandwidth.** `present` gains least (1.42×) because it is a copy,
   not a computation — four cores do not make DRAM faster. On real hardware it
   crosses PCIe into write-combining VRAM, where this will be worse still.
4. **The text is not parallel**, and AA text is the single most expensive thing
   in the scene after the wallpaper (4.9 ms for 40 lines, standalone).

### The first attempt was slower, and that number is kept

The pthread backend originally spawned a thread per band per call. The whole
desktop went **4.98 ms serial → 5.87 ms at 4 bands, 0.85×** — nine calls × four
bands = 27 `pthread_create`/`join` pairs per frame at ~10–25 µs each.

That was real, and it was measuring the *wrong thing*: the kernel spawns
nothing, so a per-call-spawn host backend is a benchmark of pthreads. Replacing
it with a persistent spinning pool — the same shape as the kernel's parked
cores — is what made the host figure mean anything.

## 8. What would actually get the 4×

In rough order of value:

1. **A display list.** Record the C-level draw calls once, replay per band. This
   removes the zl reentrancy blocker outright and makes the *whole* scene
   parallel, text included — the 4× the brief is after.
2. **Band the AA text path.** A glyph run is a row range like anything else;
   it needs `put_pixel`'s accumulator to become per-band.
3. **One dispatch per frame** instead of nine, by batching the repaint's
   primitives behind a single barrier — which is (1) again.

---

Numbers: `hosttest/fbbench.c` · Kernel proof: `probe-smp.py` · Dispatch:
`smp.c` · Bands: `fb.c` · Queue: `PLATFORM-PROMPT.md` item 7
