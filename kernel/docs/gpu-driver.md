# The zlOS Intel GPU driver — what exists, what is proven, what is inert

2026-08-19. Three files in `kernel/`, one boundary, and an honest account of
which side of it each thing sits on.

Read `gpu-next.md` first for *why* the driver is aimed where it is. This is
*what is built*.

## The files

| file | what it does | proven how |
|---|---|---|
| `gpu.c` | builds `XY_COLOR_BLT` and `XY_SRC_COPY_BLT` | **on real 8086:9B41 silicon**, pixel-for-pixel, via i915's render node |
| `gpuring.c` | the BCS ring: buffer, GGTT, forcewake, `RING_START/CTL/HEAD/TAIL`, submit and wait | arithmetic tested and mutation-checked; **MMIO has never executed** |
| `gpucursor.c` | the 64x64 ARGB cursor image the display plane wants | compositing tested and mutation-checked; **MMIO has never executed** |

`hosttest/gputest.c` holds **109 checks** across all three. `hosttest/gpu_blt.c`
is the silicon witness. `hosttest/gpu_ring.c` + `gpu-ring-run.sh` are the
hardware bring-up, written and waiting.

## The boundary, stated plainly

**Everything that submits to hardware is gated off and has never run.**

```c
gpu_ring_arm(int on);      /* gpuring.c   - default 0 */
gpu_cursor_arm(int on);    /* gpucursor.c - default 0 */
```

Both check the gate inside the single function every register write goes
through, so no write can miss it. This is `intel.c`'s `lt_armed` pattern, chosen
for the same reason: these files can hang a GPU, and *"the code exists"* is not
*"the code works"*.

What **is** proven, and needs no hardware:

- the command encodings, on silicon
- ring space including the unsigned-underflow case, the wrap, and qword padding
- cursor alpha compositing, including the premultiplied/straight distinction
- that the memory map rejects an overlapping region at compile time

## Three things were wired and had no ignition

The same shape turned up three times in one day, and it is worth naming:

| subsystem | wiring | ignition |
|---|---|---|
| SMP band rendering | `fb.c` has `fb_par_hook`, `smp.c:265` calls it correctly | `smp_go()` reachable only from the old text shell's `*` key |
| hardware cursor | `intel.c` has all three registers, `runtime_kernel.c` publishes `cur_on`/`cur_move`/`cur_off` to zl | nothing calls them, and there was no image and no GGTT mapping |
| EFI truncation guard | four `-Werror=` flags, in the build script | `-w` in front of them silenced all four |

In every case the complete-looking half is the one people checked. **Verify the
ignition, not the wiring.**

## What is left, and what blocks it

### Blocked on a human, not on code

**Hardware bring-up of the ring and the cursor.** Both need i915 unbound, which
blanks the screen — and this machine runs a Wayland session with many agent
sessions inside it, all of which die with the display manager.

```bash
cd kernel/hosttest
sudo ./gpu-ring-run.sh --survey   # read-only, i915 stays bound. Zero risk.
sudo ./gpu-ring-run.sh --dry      # dark screen, writes NOTHING. Rehearsal.
sudo ./gpu-ring-run.sh --ring     # the real thing.
```

The runner re-execs under `setsid` first so its recovery outlives the session it
kills, and that recovery has been watched firing on `SIGTERM`.

### Blocked on file ownership

**Wiring the cursor into the compositor.** The pointer lives in `wm.c`
(`fb_pointer_show`/`fb_pointer_hide`, `ptr_x`/`ptr_y`) and `fb.c`, and both had
another session's uncommitted work in them at the time of writing. The seam is
already the right shape and needs no new code here:

```c
/* gpu_cursor_move returns 0 whenever the hardware path is not live, so the
 * software sprite stays the fallback and nothing regresses if it never is. */
if (!gpu_cursor_move(x, y)) fb_pointer_show(x, y);
```

### Blocked on an unanswered question

**The render engine**, which is where the 48x on blends lives. There is no Gen9
shader assembler in this tree *or on this box* — no `intel_clc`, no `aubinator`,
and `INTEL_DEBUG=fs` dumps nothing from the installed Mesa. Settle where a
pixel shader comes from before committing weeks to RCS.

## The order that follows from all of it

1. **SMP bands** — 1.78x on the desktop redraw, already written, switched off,
   no hardware risk. Independent of everything above.
2. **`--survey` → `--dry` → `--ring`** — answers whether a sole owner can drive
   the Gen9 legacy ring, which decides whether `gpuring.c` is a driver or a
   sketch.
3. **The cursor**, once the ring run has shown the display survives a takeover.
   It is the cheapest visible win here: the pointer stops costing pixels and
   stops being tied to frame rate.
4. **The shader question**, then RCS or not.
