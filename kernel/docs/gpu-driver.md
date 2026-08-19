# The zlOS Intel GPU driver — what exists, what is proven, what is inert

2026-08-19. Three files in `kernel/`, one boundary, and an honest account of
which side of it each thing sits on.

Read `gpu-next.md` first for *why* the driver is aimed where it is. This is
*what is built*.

## The files

| file | what it does | proven how |
|---|---|---|
| `gpu.c` | builds `XY_COLOR_BLT` and `XY_SRC_COPY_BLT` | **on real 8086:9B41 silicon**, pixel-for-pixel, via i915's render node |
| `gpuring.c` | the BCS ring: buffer, GGTT, forcewake, `RING_START/CTL/HEAD/TAIL`, submit and wait | arithmetic tested and mutation-checked; **the same submission model is now PROVEN on silicon** — see below |
| `gpucursor.c` | the 64x64 ARGB cursor image the display plane wants | compositing tested and mutation-checked; **MMIO has never executed** |

`hosttest/gputest.c` holds **109 checks** across all three. `hosttest/gpu_blt.c`
is the silicon witness. `hosttest/gpu_ring.c` + `gpu-ring-run.sh` are the
hardware bring-up, written and waiting.

## PROVEN ON SILICON, 2026-08-19: zlOS can drive the ring itself

The question the whole driver rested on — can a sole owner run the Gen9.5
blitter's *legacy* ring, or does this silicon require i915's execlist machinery?
— is answered. With i915 unbound and nothing else touching the GPU:

```
ring   phys 0x32211b000 -> gfx 0x400000
dest   phys 0x150786000 -> gfx 0x500000
before  TAIL=0x00000000 HEAD=0x00000000 START=0x00000000 CTL=0x00000000
armed   TAIL=0x00000000 HEAD=0x00000000 START=0x00400000 CTL=0x00000001
after   TAIL=0x00000030 HEAD=0x00000030 START=0x00400000 CTL=0x00000001
HEAD chased TAIL   YES in 0.00 ms
destination        16384/16384 filled, 0 still poison
```

**Every pixel.** Our own GGTT entries, our own ring, our own command stream,
verified by reading the destination back rather than by the absence of a hang.

`gpuring.c` implements exactly this sequence, so its MMIO half is no longer a
guess — the model is confirmed. It still has not itself executed inside zlOS,
which is a different claim and stays gated behind `gpu_ring_arm()`.

### The bug that hid it for three runs

The first three attempts reported `HEAD chased TAIL` and drew nothing, and the
harness blamed addressing. It was addressing — but not one GGTT entry had been
written *to the GGTT*.

BAR0 is 16 MiB: 8 of registers, then 8 of page table (`MGGC0` reads `GGMS=3`).
A 16 MiB `mmap` of `resource0` is refused with `EINVAL`. Mapping 8 MiB and
indexing at `0x800000` anyway does **not** fault — it lands in whatever the
process mapped next:

```
GGTT[0x042C0] = 6F635F73 65725F5F   ->  "__re" "s_co"
GGTT[0x042C1] = 6F685F74 7865746E   ->  "ntex" "t_ho"
```

`__res_context_hostalias` — glibc's symbol table. Every PTE went into the
harness's heap; the engine saw no mapping, read zeros, parsed them as `MI_NOOP`
and advanced HEAD through them. A clean submission of nothing that looked
exactly like success.

**The PTE format was correct throughout.** Live entries under the running
desktop read `low = addr | present`, `high = addr bits 39:32` — precisely what
`ggtt_map` writes. Only the destination was wrong, and the fix is a second
`mmap` at file offset `0x800000`.

**This was a harness bug, not a driver bug.** `kernel/gpuring.c` reaches the
same table as `intel_mmio() + 0x800000` with no `mmap` in the way, which is
correct for a kernel addressing physical memory directly.

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

## Verified against live hardware, 2026-08-19

`intel.c`'s stated method is *"verified against what firmware programmed for the
same hardware."* The timing registers had `modeset_test` as their witness; the
**plane** registers had none — so the constants `gpucursor.c` depends on were the
one part of this work with nothing behind them. `hosttest/gpu_planes.c` is that
witness. Read-only, safe with i915 loaded, 77 without root.

Against a live 2560x1440 desktop:

```
plane 1  CTL=0xC2042400  ENABLED
         format 0x2  XRGB2101010 (XR30 - 30-bit)
         tiling 0x1  X-tiled
         alpha  0x0  ignored (opaque)
         size   2560x1440
cursor   CUR_CTL=0x04000027  mode 0x27
         mode 0x27 == intel.c's CUR_MODE_64_ARGB - CONFIRMED
```

Three results:

1. **`CUR_MODE_64_ARGB` is right.** `gpucursor.c` hands that exact value to the
   display engine and nothing had ever checked it. Now something has.
2. **The panel is running XR30, not 8888.** `intel.c`'s
   `PLANE_CTL_FORMAT_XRGB8888` is `0x4` and the live plane reads `0x2` — not a
   contradiction, those are different formats, but it means a zlOS takeover
   inherits nothing about pixel format and must set it.
3. **The blended-alpha encoding is still unverified.** Alpha bits read `0`
   (ignored) because nothing on this system uses a blended plane. So the bits
   for a *blending* overlay cannot be confirmed this way and **must not be
   guessed** — which is exactly why `intel_plane_setup` still writes
   `XRGB8888` and the overlay cannot blend yet.

That third point is the honest limit on the shader-free path: Gen9's display
engine can blend an overlay plane at scanout with no ring and no shader, and
this driver cannot yet do it because the register encoding has no witness.

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

### DONE — the ring runs on the real GPU

**2026-08-19. A sole owner CAN drive the Gen9.5 blitter's legacy ring.** This
was the question the whole driver was gated on, and it is answered on the
target machine with i915 unbound:

```
forcewake     acked
ring   phys 0x32211b000 -> gfx 0x400000
dest   phys 0x150786000 -> gfx 0x500000
before        TAIL=0 HEAD=0 START=0        CTL=0
armed         TAIL=0 HEAD=0 START=0x400000 CTL=1
after         TAIL=0x30 HEAD=0x30
destination   16384/16384 filled, 0 still poison
```

**zlOS's path is `RING_START`/`CTL`/`TAIL`. No execlists needed.** i915 drives
this hardware through execlists and a context scheduler; none of that has to be
built. Program four registers, put commands in a page, move the tail.

Every piece `gpuring.c` assumes is now confirmed on silicon: forcewake acks,
the GGTT entry format (address | present, high bits in the second dword), the
ring enables from a cold `CTL=0`, and the engine executes what we wrote.

It took four runs. Three of them drew nothing and blamed addressing — correctly,
but the message pointed at the wrong layer. See `gpu-blitter.md`; the short
version is that the harness was writing its page-table entries into its own
heap and "HEAD chased TAIL" was a clean submission of nothing.

### Still blocked on a human

**The hardware cursor.** `gpu_cursor_arm(1)` has still never run — the ring run
above did not exercise a single display register. It needs i915 unbound, which
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

### DONE — the compositor calls the driver

`wm.c` now asks the display engine to move the pointer before compositing a
sprite. This is the first time anything in zlOS's compositor calls the GPU
driver at all.

```c
/* The plane first; the sprite only if it did not take. */
if (!gpu_cursor_move(ptr_x, ptr_y))
    fb_pointer_show(ptr_x, ptr_y);

/* ...and only the SPRITE has a save-under to go stale before a repaint. */
if (!gpu_cursor_is_live())
    fb_pointer_hide();
```

Safe to land because `gpu_cursor_move` returns 0 until `gpu_cursor_arm(1)` is
called on real hardware, and nothing calls that — so every build today takes
exactly the path it took before.

**The branch was proven to switch**, not assumed. `hoststubs.c` provides both
symbols as *weak* stubs (the trick `idt_mouse_wheel` already uses), so a harness
can override them and win the link. `wmshot` was built twice, once each way, and
the rendered PPMs compared:

```
bytes differing between the software-cursor and hardware-cursor renders: 927
```

Non-zero is the result that matters: with the plane live, the sprite is not
drawn. Zero would have meant the wiring was inert.

**What is deliberately NOT done: the ignition.** Nothing calls
`gpu_cursor_arm(1)` or `gpu_cursor_install()`. Arming it executes display MMIO
that has never run, and the whole point of the gate is that a human turns it on
*after* a hardware run shows the display survives a takeover. Wiring the
ignition now would be the exact mistake this file's own table is about.

### ~~Blocked on an unanswered question~~ — ANSWERED, both halves

The render engine needed two things and neither is missing now, and neither was
derived from a manual this tree does not have:

| piece | where | how |
|---|---|---|
| the pixel shader | [`gen9-shader-source.md`](gen9-shader-source.md), `kernel/gpu_shader.inc` | lifted out of Mesa — 80 bytes, colour patchable in place |
| the pipeline state | [`gen9-blend-pipeline.md`](gen9-blend-pipeline.md) | captured from the vendor driver — 76 packets, decoded |

What remains for RCS is real work — emitting those packets, building surface
state and a binding table, landing the first `3DPRIMITIVE` — but it is
transcription against a working reference rather than archaeology.

Original section follows.

### ~~Blocked on an unanswered question~~ — ANSWERED, [`gen9-shader-source.md`](gen9-shader-source.md)

> **The `INTEL_DEBUG=fs` row below is wrong, and it is the row the rest of this
> section rests on.** It is not "that build has no debug dump" — **the shader
> cache was eating it.** `~/.cache/mesa_shader_cache` is 7.8 MB; on a hit iris
> loads the compiled binary and the compiler never runs, so there is nothing to
> print. Reproduced three times with one program: cold **82 lines**, warm
> **0 lines**, `MESA_SHADER_CACHE_DISABLE=true` **82 lines** again. Mesa 26.1.5
> has been a working Gen9 compiler *and* disassembler on this exact part
> (`Mesa Intel(R) UHD Graphics (CML GT2)`) the entire time.
>
> The kernel is **five instructions, 48 bytes** — four moves of a push constant
> into the RT write payload and one `send` with EOT, because blending is the
> fixed-function output merger and not the shader. Verified *running* by pixel
> readback: predicted `30 69 76`, read back `30 69 76`.
>
> The paragraph below is right that hand-encoding "is not absurd" and right about
> why — it just did not need to be hand-encoded. **What this changes is the
> estimate, not the difficulty:** the 48 bytes were never the cost. `3DSTATE_PS`,
> `3DSTATE_CONSTANT_PS`, binding tables, `RENDER_SURFACE_STATE`, blend state and
> `STATE_BASE_ADDRESS` are. Weighing "write a Gen9 shader" as the expensive part
> was weighing the wrong thing.
>
> The `glGetProgramBinary` finding below stands and is still worth keeping — that
> blob really is a Mesa cache entry and really would have been the wrong thing to
> embed.

**The render engine**, which is where the 48x on blends lives. It needs a Gen9
pixel shader, and getting one is genuinely open.

Ruled out so far, each with the command that established it:

| idea | result |
|---|---|
| a Gen9 assembler in this tree | none |
| `intel_clc` / `aubinator` on this box | neither installed |
| `INTEL_DEBUG=fs` shader dump from Mesa | silent — that build has no debug dump |
| **`glGetProgramBinary`** | **works, and gives the wrong thing** |

The last one was worth trying and is worth writing down. Mesa 26.1.5 on this
part *does* advertise `GL_ARB_get_program_binary` with one supported format, and
a trivial constant-colour fragment shader links and hands back **4510 bytes**.
But that blob is Mesa's own cache format, not Gen9 ISA: one string
(`gl_FragColor`), a hash-like header, no `send` opcodes at any 8-byte stride,
and it is neither zlib nor zstd at the top level. Embedding it in zlOS would
embed a Mesa cache entry, not a program the EUs can run.

So the shader still has to come from somewhere — installing Intel's tools, or
hand-encoding Gen9 EU ISA. **Settle that before committing weeks to RCS**, and
note that hand-encoding is not absurd: a constant-colour pixel shader is a
handful of instructions, and the encoding is documented.

## The order that follows from all of it

1. ~~**SMP bands** — 1.78x on the desktop redraw, already written, switched off,
   no hardware risk. Independent of everything above.~~

   > **RETRACTED, `DECISIONS.md` #39.** "No hardware risk" is true and it is not
   > the cost. The park loop is a **spin**, not `cli; hlt` — `smp.c` says so and
   > explains why there is no alternative without an interrupt path this kernel
   > lacks — so this burns three cores from `smp_go()` until reboot. The 1.78x is
   > **1.88 ms** on a full-screen redraw that damage tracking already made rare,
   > and the expensive full-screen work is not even on the band path:
   > `fb_grad_radial` and the conic wedge, the wallpaper's three glows and two
   > wedges at ~12.2 ms each, do not route through `fb_par_run`. Only fill,
   > gradient, shadow and present do.
   >
   > The one variant worth measuring is the **boot wallpaper bake** — full-screen,
   > expensive, pure C, and once, so the cores can be parked again afterwards.
   > Unmeasured; #39 says what to measure first.
2. **`--survey` → `--dry` → `--ring`** — answers whether a sole owner can drive
   the Gen9 legacy ring, which decides whether `gpuring.c` is a driver or a
   sketch.
3. **The cursor**, once the ring run has shown the display survives a takeover.
   It is the cheapest visible win here: the pointer stops costing pixels and
   stops being tied to frame rate.
4. **The shader question**, then RCS or not.
