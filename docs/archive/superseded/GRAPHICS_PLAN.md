> **AUDITED 2026-08-19 · SUPERSEDED.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. A Windows-hosted graphics ladder (user32/gdi32/d2d1/opengl32 via FFI) added by the very commit that ported zl to Linux, never edited since. It has never described a platform this repo targets. The annotation `HANDOFF.md` and `DECISIONS.md` both claim was written into this file does not exist — this banner is it. On zlOS, 3D means a software rasterizer.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**

# Graphics plan — from BMP files to live windows to the GPU

> **2026-08-17 — this is the WINDOWS-HOSTED plan. It is still correct for `zl`
> the language. It does NOT describe zlOS, which went a different way.**
>
> This plan says layers 3–5 are gated on FFI into `user32.dll` / `opengl32.dll`.
> Eight days after it was written, **zlOS reached layer 3 with no FFI at all** —
> not by calling into an OS, but by *removing* the OS underneath. On bare metal
> the framebuffer is just memory and `poke32` reaches it. The `bytes` type was
> never needed either: the kernel's pixel buffers are C arrays at fixed physical
> addresses.
>
> **The one line that must not be carried into zlOS work:** layer 5 says the GPU
> is reached via `opengl32.dll`. **On bare metal that DLL does not exist and never
> will.** GPU acceleration on zlOS would mean writing a Gen9 3D driver, and
> `kernel/docs/desktop-prior-art.md` establishes that *no* hobby OS has done this
> — SerenityOS runs Half-Life on a **software** rasterizer, and Essence OS does
> animated vector UI on the CPU.
>
> So on zlOS, layer 5 is not "FFI to OpenGL". It is **"write a software
> rasterizer"**, and `kernel/fb3d.c` is already its first step.
>
> See `kernel/docs/desktop-prior-art.md` and `kernel/docs/desktop-plan.md`.

Drafted 2026-08-03. The layered path for zl graphics, what each layer needs, and what is buildable
now versus gated on FFI. Complements the existing `design_game_system.md` (973 lines, the games-
specific FFI+window design) by giving the whole graphics stack a single ordered map.

**The one-sentence version:** zl already does the *maths* of graphics (it computes pixels and
writes real image files today); it cannot yet open a *live window* (that is `CreateWindowEx`, an
FFI call). So graphics splits cleanly into "done now" and "one feature away."

---

## The five layers, bottom to top

Every graphics program sits somewhere on this ladder. Higher = more capable, more OS-dependent.

```
5. GPU / 3D        OpenGL, DirectX, Vulkan - hand triangles to the graphics card
4. accelerated 2D  Direct2D - hardware-drawn shapes, images, text
3. live window     CreateWindowEx + message loop + GDI - a real resizable window you draw in
2. raw framebuffer a block of pixels in memory you set one at a time
1. image file      compute pixels, write a .bmp/.png to disk        <- zl IS HERE
```

zl is solidly on **layer 1** and has everything it needs for **layer 2**. Layers 3-5 are all
FFI-gated (roadmap item 16), because a window and the GPU are reached through Windows DLLs.

---

## Layer 1 — image files. DONE.

zl draws graphics today; it just saves to a file instead of a screen.

- `stdlib/bmp.zl` writes real BMP files by computing RGB pixels and laying out the byte header.
- `examples/mandelbrot.zl` renders the Mandelbrot set to a BMP.
- `examples/raytracer.zl` does actual 3D ray tracing - camera, spheres, lighting, reflections -
  and writes the result to a BMP.
- `examples/game_of_life.zl` and `maze.zl` render to BMP and to the terminal.

**This proves the hard part is already solved: the MATHS of graphics.** Colour blending, projection,
lighting, rasterisation - zl does all of it. What is missing is not "how to make a picture," it is
"how to put a picture on the screen live."

**Small wins available now at this layer (pure zl, no FFI):**
- **PNG output** - everyone uses PNG, nobody uses BMP. A pure-zl PNG encoder needs a byte buffer
  (see the constraint below) and a `deflate` compressor - both writable in zl once bytes are sound.
- A `canvas` module - a 2D array of pixels with `set_pixel`, `line`, `rect`, `circle`, `fill`,
  `blit`, then `save_bmp`. This is the drawing API layer 3 will also use, so build it now against
  a file and reuse it against a window later. **Highest-value pure-zl graphics win today.**

**The blocker even here:** the byte-buffer gap (GAPS_REALWORLD 1.1). A pixel buffer is bytes, and
zl strings cannot hold a NUL, while a list-of-boxed-ints costs ~64x memory and never frees. `bmp.zl`
works only because images stay small. A real `canvas` at 1920x1080 is 8 MB of pixels - unworkable as
boxed ints. **So graphics, even file-only, wants the byte type (`bytes`) that the roadmap already
promoted to near the top.**

---

## Layer 2 — raw framebuffer. Needs the byte type.

A framebuffer is just layer 1's pixel block, held in memory as a flat `bytes` and updated in place.
Everything a `canvas` module does. The only thing standing between zl and a fast software renderer
is the `bytes` type - once a pixel buffer is real bytes (not boxed ints), `set_pixel` is O(1),
memory is one allocation, and blitting is a `memcpy`. **This layer is the payoff of shipping `bytes`.**

---

## Layer 3 — a live window. The first FFI layer.

This is "how does C make a window," and the answer is four DLL calls, all in `user32.dll` /
`gdi32.dll`:

1. `RegisterClassEx` - describe the window class (its behaviour, icon, cursor).
2. `CreateWindowEx` - ask Windows for the window; get back an `HWND` handle.
3. **The message loop** - `GetMessage` / `DispatchMessage` in a `while`. Windows sends you events
   (mouse, keys, resize, and `WM_PAINT` = "redraw yourself now"). You do not draw whenever you
   like; Windows tells you when. This is the mental-model shift from layer 1.
4. `BeginPaint` + GDI calls (`BitBlt` to slam a framebuffer onto the window, or `Rectangle`/
   `TextOut` for shapes) + `EndPaint`.

**What zl needs for this, in order:**
- **FFI (item 16)** to call the four functions. This is the gate.
- **Sized integers (item 15)** to declare their arguments (`HWND`, `UINT`, `LPARAM`, `WPARAM`).
- **A callback from C into zl** - the window procedure (`WndProc`) is a function C calls back on
  every event. FFI must support "give C a pointer to a zl function." This is the hard part of the
  FFI design and `design_game_system.md` already grapples with it (it reads input via
  `GetAsyncKeyState` specifically to SIDESTEP needing a callback initially - a smart staging choice).
- **The `bytes` framebuffer** from layer 2 - what you `BitBlt` onto the window.

Once these exist, a live window is `stdlib/window.zl` - library code wrapping `user32.dll`, exactly
as C's `#include <windows.h>` is just declarations of the same functions. **Not engine work.**

**The terminal shortcut:** before any of this, a game/TUI can run in the TERMINAL (layer 1.5) with
just three tiny engine builtins - `sleep`, a `kbhit` non-blocking key poll, and the interpreter
enabling VT console mode (GAPS_REALWORLD). That gives a playable snake/pong with NO FFI, NO window,
NO GPU. It is the fastest route to an interactive graphical program and should come first.

---

## Layer 4 — accelerated 2D (Direct2D). FFI, later.

Hardware-drawn shapes, images and text via `d2d1.dll`. Same shape as layer 3 (FFI + handles), more
functions. A nice-to-have once layer 3 works; the software `canvas` covers most needs first.

## Layer 5 — the GPU / 3D. FFI, furthest out.

`opengl32.dll` / DirectX / Vulkan. You stop setting pixels and start handing the graphics card
**triangles + shaders**, and it fills millions of pixels in parallel. This is real games and 3D.
It needs everything below it plus a lot of FFI surface (context creation, buffer uploads, shader
compilation) and ideally SIMD (a Zig delta noted in GAPS_REALWORLD_2). The raytracer proves zl can
do 3D maths; the GPU is about doing it fast enough for 60fps, which is a large, later bet.

---

## The ordered path

1. **`bytes` type** - unblocks a real framebuffer and PNG. Already near the top of the roadmap for
   other reasons (networking, binary files); graphics is another vote for it.
2. **`canvas` module (pure zl)** - `set_pixel/line/rect/circle/fill/blit/save_bmp`. Buildable the
   moment `bytes` lands, reused by every layer above. Highest-value graphics win.
3. **PNG encoder (pure zl)** - `bytes` + `deflate`. Real image output.
4. **Terminal interactivity** - `sleep` + `kbhit` + VT mode (3 tiny builtins). A playable terminal
   game with no FFI. The fastest "graphics feels alive" milestone.
5. **FFI (item 16) + sized ints (item 15)** - the gate for everything windowed.
6. **`stdlib/window.zl`** - `CreateWindowEx` + message loop + `BitBlt` the canvas. A real window.
   Follows `design_game_system.md`.
7. **Direct2D, then OpenGL/GPU** - accelerated 2D and 3D, furthest out, largest FFI surface.

**Is it down the line?** The *live window* is behind FFI, so yes - but not far, and the path is
fully mapped. And the *maths* is done now: zl already ray-traces. The nearest satisfying milestone
is not a window at all - it is a **terminal game** (step 4), three small builtins away, needing no
FFI. That is where "zl does graphics you can play" actually starts.

**What this shares with everything else:** layers 3-5 are FFI + a zl library, not engine work -
the same conclusion as PC control, networking and the kernel. Graphics is one more thing that FFI
turns from "reimplement it" into "wrap the DLL Windows already ships."
