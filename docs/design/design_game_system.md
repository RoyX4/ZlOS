# Design: A Game System for zl (W8)

**Status:** proposal
**Author:** language/systems design pass, 2026-07-31
**Scope:** `docs/archive/prompts/OVERNIGHT_CAMPAIGN.md` W8. Consumes W5 (raw memory + FFI), W3/W4 (speed),
and the existing stdlib (`bmp.zl`, `noise.zl`, `easing.zl`, `vec.zl`, `geometry.zl`).
No code is changed by this document.

---

## 0. TL;DR

A game is the best end-to-end proof zl works, because nothing else in the repo needs
speed, structs, raw memory and FFI **at the same time, inside a 16-millisecond budget,
for ten minutes without stopping**. Every other program in `stdlib/` runs once and exits.

The design, in one paragraph:

- **One framebuffer, two presenters.** The game draws 32-bit BGRX pixels into a raw
  `alloc()`'d buffer. `present_window` hands it to **GDI `StretchDIBits`** once per frame
  (recommended over a DIB section and over writing frames to disk — §2.1). `present_file`
  dumps the same buffer through `bmp.zl` as a numbered `.bmp`. The file presenter is not a
  consolation prize: it is the only **deterministic, testable** path, so it stays forever
  and it is where M0 ships before any FFI exists.
- **No window callback.** The window class registers `DefWindowProcA` *itself* as the
  `WNDPROC` and the loop drives everything from `PeekMessageA`. This removes FFI Stage 4
  (callback thunks) from the critical path entirely (§2.2).
- **Fixed timestep, integer time, integer state.** 16.16 fixed point everywhere, time in
  whole microseconds, interpolation alpha as an integer 0..65536. Variable timestep is
  rejected not mainly for physics reasons but because it destroys determinism, and
  determinism is what turns the game into a test that `verify.ps1`-style tooling can grade
  (§2.4). It also sidesteps the fact that the *only engine that will have FFI* —
  `nativeval` — has no floats at all.
- **The pixel loop is not written in zl.** Measured from `bench/README.md`, an elementary
  zl operation costs ~19 ns on the boxed C backend and ~140 ns interpreted. A per-pixel
  zl loop over a 320×180 buffer is **~17 ms — the entire frame budget, to clear the
  screen**. So the framebuffer API is span-shaped, and exactly three primitives
  (`fill32`, `copy32`, `blit32`) do the per-byte work inside the runtime (§1.3, §2.5).
  This is `bench/README.md`'s own `b5_string` lesson applied on purpose: *put the work in
  the runtime*.
- **Sprites are compiled to spans at load**, so the blit inner loop has no per-pixel
  transparency branch (§2.6).
- **Audio is deferred** to M5, with the route mapped (§2.8): `wav.zl` first (pure zl,
  testable, the exact complement of `bmp.zl`), then `PlaySoundA` one-shots, then polled
  `waveOut` double-buffering — which, like the window, needs **no callbacks**.
- **First milestone that is genuinely a playable game: Breakout** (§6). It needs no
  assets, no audio, no callbacks and no floats. It is blocked on precisely five things,
  listed with owners in §6.3.

And one finding that falls out of the budget work and matters beyond games:
**the native runtime's bump arena never frees** (`design_native_runtime.md` §7: "a compile
run is short-lived; fine"). A game runs forever. At ~12 FFI calls a frame, each building
an argument list in the arena, the 64 MB arena dies in about ten minutes. The fix is ten
lines — an arena mark/release stack — and the game is the first program in the repo that
would ever have found it (§2.10).

---

## 1. Motivation

### 1.1 Why a game, and why now

W8 is last in the campaign for a reason: it is the integration test. Look at what a single
frame of Breakout touches:

| Frame does | Needs | From |
|---|---|---|
| clears a 230 KB buffer | raw memory, a runtime span primitive | W5 + §2.5 |
| steps physics at a fixed rate | integer arithmetic that is *fast* | W3/W4 |
| reads keyboard and mouse | FFI into `user32.dll` | W5 |
| pushes the buffer to the screen | FFI into `gdi32.dll`, 13-argument call | W5 |
| holds sprite and entity tables | structs / fixed-layout records | W5 |
| does all of it 60× a second, for minutes | allocation discipline, no leaks | §2.10 |

Nothing else in the repo does more than two of those. `raytracer.zl` is pure compute.
`sysmon.zl` is pure I/O. The self-host is a batch job. A game is the first zl program that
is a *system*, and that is the whole point of running it as a wave.

### 1.2 What already exists and is directly reusable

This is not green field. Five stdlib modules were written with this wave in mind, and
`noise.zl`'s own header says so ("Groundwork for the game system").

| Module | Used for | How |
|---|---|---|
| `bmp.zl` | the file presenter, the golden-image oracle, **and the asset authoring tool** | `bmp_save` already writes a correct 24-bit BMP; the game's sprite loader reads the same format back (§2.9) |
| `noise.zl` | backgrounds, starfields, terrain, ore veins | `fbm_2d(x, y, oct, seed)` is a pure function of coordinates — a whole background is four numbers, not a stored bitmap |
| `easing.zl` | UI motion, level-intro tweens, screen shake decay, paddle-hit flash | `tween(a, b, t, name)`; note `ease()` dispatches **by string** because zl has no first-class functions — the game must follow the same pattern |
| `vec.zl` | ball velocity, positions | `vadd`/`vsub` are pure add/subtract and are **already correct on 16.16 fixed point**; `vscale`/`vdot` need one shift (§2.4) |
| `geometry.zl` | collision maths | `cross`, `segments_intersect`, `on_segment` are multiply-only and survive fixed point; `dist`/`perimeter` use `hypot`/`sqrt` and do not |

The design below deliberately mirrors `bmp.zl`'s conventions rather than inventing new
ones: an image is `[w, h, px]` with row 0 at the **top**, flat pixel storage, and one
writer that clamps. A framebuffer is `[w, h, addr, pitch]` — the same shape, one field
swapped from a boxed list to an address. Drawing code reads the same either way.

### 1.3 The budget — the measured number that shapes every other decision

From `bench/README.md` (2026-07-30, best of 5): `b2_arith` runs 2.4 M loop iterations in
3,344 ms interpreted and 455 ms on the boxed C backend. Each iteration is roughly ten AST
node evaluations, so:

| Engine | ns per elementary op | Source |
|---|---|---|
| `interp` | ~140 ns | 3,344 ms / 2.4 M / 10 |
| `boxed-C` (`compile.exe` + `cl /O2`) | ~19 ns | 455 ms / 2.4 M / 10 |
| `nativeval` (the engine FFI lands on) | **assume ~50 ns**, unmeasured | boxed class, no optimizer, a runtime `call` per op |
| `unboxed-C` / `llvm` / `nativegen` | ~4 ns | 30–80× interp — **but they cannot run lists, strings, or FFI** |

Now price a frame at 60 Hz (16.6 ms) on a 320×180 buffer (57,600 pixels):

| Approach | Cost per frame | Verdict |
|---|---|---|
| per-pixel zl loop: `poke32(base + i*4, v)` ≈ 6 ops ≈ 300 ns/px | **17.3 ms** | dead on arrival — that is a *clear*, before drawing anything |
| same, at 160×90 (14,400 px) | 4.3 ms | 26% of budget to clear the screen. Still unacceptable |
| span-shaped: ~400 sprite rows/frame × ~20 ops of zl setup | **0.4 ms** | comfortable |
| the spans themselves, in a runtime `fill32`/`copy32` (230 KB at ~4 GB/s) | ~0.06 ms | free |

**The conclusion is not negotiable: zl issues spans, the runtime moves bytes.** This is
exactly the `b5_string` result from `bench/README.md` — "compiling the control flow is
worthless when the work is in the runtime" — read as a design instruction instead of a
complaint. The C-vs-zl boundary is drawn at three primitives (§2.5), which is smaller than
the string-routine surface `src/runtime/runtime.c` already owns for `at`/`find`/`replace`.

A second consequence: **shrink the framebuffer and let GDI scale it.** 320×180 stretched
to 1280×720 is four times fewer pixels for the game to touch, at zero cost to us, and it
is a genuine reason to prefer `StretchDIBits` over `BitBlt` (§2.1).

---

## 2. Design decisions (proposed, to be locked)

### 2.1 Getting pixels on screen — three options, one recommendation

**Option A — GDI `StretchDIBits` from a plain memory buffer.** The buffer is ours
(`alloc(w*h*4)`, i.e. the native runtime's `VirtualAlloc` arena). A `BITMAPINFOHEADER` is
built once in raw memory. Every frame:

```
call!(StretchDIBits, [hdc, 0, 0, cw, ch, 0, 0, fbw, fbh, bits, bmi, 0, 13369376])
                                                              #  DIB_RGB_COLORS, SRCCOPY
```

- **Pro:** one call per frame, zero GDI objects, zero handle lifetimes.
- **Pro:** the pixel memory is ours — same arena as everything else, same
  `alloc`/`peek`/`poke` story, lives independently of the window.
- **Pro:** **scaling is free.** A 320×180 buffer fills a 1280×720 window, which is a 4×
  reduction in pixels zl must touch (§1.3). Set `SetStretchBltMode(hdc, 3)`
  (`COLORONCOLOR` = nearest neighbour) once so pixel art stays crisp.
- **Pro:** window resize needs no reaction at all — the destination rect changes, the
  source buffer does not.
- **Pro:** 13 arguments, so it exercises the FFI trampoline's stack-argument path
  (`design_ffi_syscalls.md` §2.4) on the very first frame. Good: that path is the one
  most likely to be subtly wrong, and here it fails loudly and visibly.
- **Con:** GDI does the scale+copy on the CPU each frame. At 230 KB source this is
  sub-millisecond and irrelevant at our sizes.

**Option B — `CreateDIBSection` + `BitBlt` from a memory DC.** `CreateCompatibleDC`,
`CreateDIBSection` (which returns an `HBITMAP` *and writes the pixel pointer through an
out-parameter*), `SelectObject`, then `BitBlt` per frame.

- **Pro:** a format-matched `BitBlt` is the fastest 1:1 blit GDI offers.
- **Con:** the pixel memory is **owned by GDI**, handed back through an `LPVOID*`
  out-parameter — so the first thing this path needs is `peek64` on a slot, and a pointer
  that is *not* from our arena, breaking the "every address came from `alloc` or the OS"
  simplicity that §2.2 of the FFI doc leans on.
- **Con:** four extra API calls and three handle lifetimes (DC, bitmap, old bitmap) to get
  wrong, all of which must be recreated on resize.
- **Con:** no free scaling. Wanting a small buffer on a big window means `StretchBlt`
  anyway — at which point Option B's only advantage is gone and its costs remain.

**Option C — write frames to disk.** Render into the framebuffer, dump each frame with
`bmp.zl`, assemble externally.

- **Pro:** **zero FFI.** Runs today, on `interp.exe`, on the language as it exists.
- **Pro:** **deterministic and diffable.** Frame 42 has a SHA-256. This is the only
  presenter that can be graded by a test, and this repo grades everything.
- **Con:** it is not a game. No input, no realtime.
- **Con:** `bmp_save` builds a boxed list of `w*h*3` numbers per frame — a 320×180 frame
  is 172,800 `push` calls. Fine for 60 golden frames; hopeless for 60 frames a second.

**Recommendation: A for the game, C for the tests, both permanently.**

Define one framebuffer and two presenters chosen at startup:

```
present = "window"          # or "file"
```

Everything above the presenter is identical. This buys three things at once: M0 ships
before FFI exists (§4), the game keeps a regression test that runs on the interpreter, and
a recorded input stream replayed through the file presenter produces byte-identical BMPs —
i.e. **the game becomes a `verify.ps1`-shaped artifact**, not a demo you have to eyeball.

*Rejected without much agony:* Direct2D / Direct3D / OpenGL. All need COM vtable dispatch
(call-through-a-pointer-in-a-struct) or by-value struct arguments, both explicit non-goals
of `design_ffi_syscalls.md` §8. DirectDraw is deprecated. GDI is the only 2D API on
Windows reachable with integer-only marshalling, and that is the layer W5 will actually
have.

### 2.2 The window and the message loop — and how to not need a callback

The obstacle: `RegisterClassA` wants a `WNDPROC` — a function pointer *Windows* calls.
That is FFI Stage 4 (callback thunks), the last and hardest stage.

**Decision: register `DefWindowProcA` as the window procedure and never write one.**

```
user32   = dll!("user32.dll")
defproc  = sym!(user32, "DefWindowProcA")
# ... poke defproc into WNDCLASSA.lpfnWndProc
```

Consequences, all of them acceptable and one of them a bonus:

| Message | What `DefWindowProcA` does | What we do |
|---|---|---|
| `WM_ERASEBKGND` | with `hbrBackground = 0`, returns FALSE and paints nothing | nothing — **and this is why there is no flicker** |
| `WM_PAINT` | `BeginPaint`/`EndPaint`, validating the region without drawing | nothing — we blit every frame regardless, so the window is never stale for more than 16 ms |
| `WM_CLOSE` (the X button) | calls `DestroyWindow` | detect it: see below |
| `WM_DESTROY` | nothing — **it does not post `WM_QUIT`** | so do not wait for `WM_QUIT` |

Quit detection therefore is **`IsWindow(hwnd) == 0`**, checked once per frame after the
pump. That is one extra FFI call and it is exactly correct: the window is gone when the
window is gone.

The full startup, with the struct layouts spelled out because getting them wrong is the
single likeliest source of a silent crash. `WNDCLASSA` on x64 is **72 bytes**:

| Offset | Field | Value |
|---|---|---|
| 0 | `style` (UINT) | `0x0023` = `CS_OWNDC | CS_HREDRAW | CS_VREDRAW` |
| 4 | *padding* | — |
| 8 | `lpfnWndProc` | address of `DefWindowProcA` |
| 16 | `cbClsExtra` (int) | 0 |
| 20 | `cbWndExtra` (int) | 0 |
| 24 | `hInstance` | `GetModuleHandleA(0)` |
| 32 | `hIcon` | 0 |
| 40 | `hCursor` | `LoadCursorA(0, 32512)` (`IDC_ARROW`) |
| 48 | `hbrBackground` | **0** — no background brush, no erase, no flicker |
| 56 | `lpszMenuName` | 0 |
| 64 | `lpszClassName` | `cstr("zlwin")` |

`CS_OWNDC` matters: it gives the window a private device context, so `GetDC` returns the
same DC every time and `ReleaseDC` is a no-op. **Call `GetDC` once at startup and cache
it** — an FFI call in zl costs more in argument-list construction than the Win32 call
costs to execute (§2.10), so hoisting per-frame calls out of the loop is real work, not
micro-optimisation.

`MSG` on x64 is **48 bytes**: `hwnd` at 0, `message` (UINT) at 8, `wParam` at 16, `lParam`
at 24, `time` at 32, `pt.x` at 36, `pt.y` at 40.

```
fn pump(pm_msg, pm_hwnd) {
    # PM_REMOVE = 1. Drain the queue; return false when the window is gone.
    while call!(PeekMessageA, [pm_msg, 0, 0, 0, 1]) != 0 {
        pm_kind = peek32(pm_msg + 8)
        if pm_kind == 256 {                      # WM_KEYDOWN  - edge event
            key_edge_push(peek32(pm_msg + 16))
        } elif pm_kind == 513 {                  # WM_LBUTTONDOWN
            mouse_click = 1
        }
        call!(TranslateMessage, [pm_msg])
        call!(DispatchMessageA, [pm_msg])
    }
    return call!(IsWindow, [pm_hwnd]) != 0
}
```

Note what that loop just bought: because the `MSG` struct is *our* memory, zl reads
`message` and `wParam` straight out of it with `peek32`. **Edge-triggered key events with
no callback.** The thing a `WNDPROC` is usually for, obtained by reading 8 bytes.

Window size: `CreateWindowExA`'s width/height are the *outer* frame, not the client area,
so a 1280×720 request yields a 1264×681 client and a scaled, slightly-wrong image. Fix:
build a 16-byte `RECT` (`{0, 0, 1280, 720}`), call `AdjustWindowRect(rect, style, 0)`, and
pass `right-left` / `bottom-top`. Twelve arguments to `CreateWindowExA`; more stack-arg
exercise for the trampoline.

**`subsystem!("gui")`** (FFI doc Stage 1) is *not* blocking. Without it the program is a
console app that also opens a window — ugly, entirely functional. Take it when it lands.

### 2.3 Keyboard and mouse

Two mechanisms, both already paid for, each doing what it is good at:

| Need | Mechanism | Why |
|---|---|---|
| "is left held **right now**" | `GetAsyncKeyState(vk)`, test bit 0x8000 | level state; a game asks this every tick, and it must not depend on message timing |
| "was fire pressed **this frame**" | `WM_KEYDOWN` read out of the `MSG` struct in the pump (§2.2) | edge events; polling misses a press+release inside one frame |
| paddle position | `GetCursorPos(pt)` + `ScreenToClient(hwnd, pt)`, then scale by `fbw / clientw` | absolute and smooth; the message path gives you `WM_MOUSEMOVE` coalesced and lagged |
| clicks | `WM_LBUTTONDOWN` in the pump | edge, same as keys |

`GetAsyncKeyState` is **global** — it reports keys pressed while another app has focus.
Gate it: `if call!(GetForegroundWindow, []) == hwnd`. Without that, alt-tabbing away leaves
the paddle stuck against the wall.

zl has no bitwise operators, so the down-test is `band` (`REFERENCE.md`, Bitwise):

```
fn key_down(kd_vk) {
    if focused == 0 { return false }
    return band(call!(GetAsyncKeyState, [kd_vk]), 32768) != 0
}
```

`band` on the possibly sign-extended `SHORT` return is safe: 0xFFFF8000 and 0x00008000
both have bit 15 set.

Keys the first game needs: `VK_LEFT` 0x25, `VK_RIGHT` 0x27, `VK_SPACE` 0x20,
`VK_ESCAPE` 0x1B, `VK_LBUTTON` 0x01.

### 2.4 The fixed timestep — and why variable is the wrong answer

**The loop** (the standard accumulator, stated in integers):

```
DT_US    = 8333          # 1/120 s, in whole microseconds
MAX_US   = 250000        # never simulate more than 0.25 s of catch-up

fn frame() {
    now_us  = qpc_us()
    delta   = now_us - last_us
    last_us = now_us
    if delta > MAX_US { delta = MAX_US }        # the spiral-of-death clamp
    acc = acc + delta
    while acc >= DT_US {
        state_prev_copy()                        # prev <- cur, before stepping
        step(DT_US)
        acc = acc - DT_US
    }
    alpha = int(acc * 65536 / DT_US)             # 0..65535, a 16.16 fraction
    render(alpha)
}
```

**Interpolation** at render time, per drawn entity:

```
rx = prev_x + int((cur_x - prev_x) * al_alpha / 65536)
```

Without it the ball visibly stutters whenever the render rate and the tick rate do not
divide evenly — which is always, because a 60 Hz display and a 120 Hz tick drift against
each other continuously.

**Why not a variable timestep** (`step(delta)` with whatever the frame took):

1. **It destroys determinism, and determinism is this repo's currency.** With a fixed step
   plus `seed(n)`, a recorded input stream replays to a byte-identical frame — which is
   what makes §2.1's file presenter a *test*. With a variable step the same inputs on a
   slower machine produce a different game. There is no golden image, no regression gate,
   no bisect. Everything in `verify.ps1`'s philosophy argues against it.
2. **Collision correctness becomes frame-rate dependent.** A ball at 900 px/s moves 7.5 px
   per 1/120 s tick and 60 px during one 66 ms hitch. Bricks are 8 px tall. A variable step
   tunnels through the level on any stutter; a fixed step tunnels never (and §2.7's swept
   test removes even the 7.5 px case).
3. **Integrator stability.** Anything with a spring, drag, or gravity term has a stable
   `dt` range. `dt` supplied by the OS scheduler is not in any range.
4. **The debugger case.** Break for five seconds, continue, and a variable step advances
   the world by five seconds — the ball is in another postcode. The clamp in the fixed
   loop makes a breakpoint cost at most 0.25 s of simulation.
5. **Replays, ghosts, and future networking** are free with a fixed step and impossible
   without one.

**Integer time, integer state.** Two independent reasons, and they agree:

- `nativeval` — the boxed native backend, and **the only engine FFI is landing on**
  (`design_ffi_syscalls.md` §3) — tags integers `(n<<1)|1` and has **no float
  representation at all**. `design_floats.md` Stage 2 would fix the *other* native
  backend; nothing fixes `nativeval` today. A float game literally cannot be compiled by
  the engine that can open a window.
- Even once floats exist, float state would make the interpreter's golden frames and the
  native build's frames disagree in the last bit and diverge over minutes. Integers make
  the three-engine agreement property hold *for the game*.

So: **positions and velocities are 16.16 fixed point, time is whole microseconds, alpha is
0..65536.** One helper, and the `vec.zl` interaction stated honestly:

```
FIX = 65536
fn fixmul(fm_a, fm_b) { return int(fm_a * fm_b / FIX) }
fn fixdiv(fd_a, fd_b) { return int(fd_a * FIX / fd_b) }
fn topx(tp_v)         { return int(tp_v / FIX) }        # to whole pixels
```

`vadd`/`vsub` from `vec.zl` work on 16.16 **unchanged** (add and subtract are
scale-preserving). `vscale` and `vdot` multiply, so they need the `/FIX` — call `fixmul`
rather than editing `vec.zl` (surgical-changes rule; the game gets `fixvec.zl`).
`vlen` needs `sqrt` — avoid it: compare squared distances, which is what collision wants
anyway. From `geometry.zl`, `cross` and `segments_intersect` are multiply-only, so they
work in fixed point with a known scale factor (`cross` comes back at 32.32 — only its
*sign* is used, so the scale never matters). `dist`/`perimeter` use `hypot`; leave them for
the float engines.

**Timing source:** `QueryPerformanceCounter` / `QueryPerformanceFrequency` (kernel32) into
an 8-byte slot, read with `peek64`. `now()` is not an option on the shipping path: it is
`clock()`-based (`src/runtime/interp.c:805`), millisecond-resolution, and **not in the native runtime's
builtin set** at all. `Sleep(1)` at the end of a frame that finished early keeps the
process off a spin loop; do not `Sleep(0)`, which yields but burns the core.

### 2.5 The framebuffer

**Representation** — deliberately `bmp.zl`'s `[w, h, px]` with one field swapped:

```
fb = [w, h, addr, pitch]        # pitch = w * 4, kept explicit for sub-surfaces
```

| Decision | Value | Reason |
|---|---|---|
| pixel format | 32-bit `BI_RGB`, i.e. dword `0x00RRGGBB` (bytes B,G,R,X in memory) | GDI's native no-conversion format; one `poke32` per pixel; the free X byte becomes alpha in v2 |
| row order | **row 0 is the top**, matching `bmp.zl` | set `biHeight` **negative** in the `BITMAPINFOHEADER` and GDI reads top-down; no flip anywhere, ever |
| storage | raw `alloc()`, not a zl list | `StretchDIBits` needs contiguous bytes; a boxed list is also 57,600 `Value` allocations |
| resolution | 320×180, presented at 1280×720 | §1.3 — 4× fewer pixels, and GDI scales for free |

Colour packing needs no bitwise ops:

```
fn rgb(rg_r, rg_g, rg_b) { return rg_r * 65536 + rg_g * 256 + rg_b }
```

**The API is span-shaped.** `fb_px` exists for debugging and text and is documented as slow:

```
fn fb_new(nw_w, nw_h)                                      # -> [w,h,addr,pitch]
fn fb_clear(cl_fb, cl_rgb)                                 # fill32 over the whole buffer
fn fb_px(px_fb, px_x, px_y, px_rgb)                        # one pixel - clipped, SLOW
fn fb_hline(hl_fb, hl_x, hl_y, hl_n, hl_rgb)               # clipped span -> fill32
fn fb_rect(rc_fb, rc_x, rc_y, rc_w, rc_h, rc_rgb)          # h clipped spans
fn fb_blit(bl_fb, bl_spr, bl_x, bl_y)                      # compiled sprite, §2.6
fn fb_save_bmp(sv_fb, sv_path)                             # via bmp.zl, for tests
```

**The three primitives that must live in the runtime** (`src/runtime/interp.c` **and** `src/runtime/runtime.c`
identically — the campaign's parity rule):

| Builtin | Signature | Does |
|---|---|---|
| `fill32(addr, count, dword)` | — | store `dword` `count` times |
| `copy32(dst, src, count)` | — | forward copy of `count` dwords |
| `blit32(dst, dpitch, src, spitch, w, h)` | — | `h` rows of `copy32`; the one convenience primitive, because a 32-row sprite would otherwise be 32 zl-level calls |

That is the entire C surface the game adds. Everything else — clipping, coordinates,
sprites, collision, the loop — is zl. Compare against the alternative of writing the
blitter in C: this boundary is three functions with no policy in them, and it is smaller
than the string surface `src/runtime/runtime.c` already owns.

**Clipping happens once per span, never per pixel.** And a real zl hazard applies here,
noted at the top of `geometry.zl`: **`and`/`or` evaluate both sides**, so a bounds guard
cannot be folded into a condition that also indexes. Clip arithmetically:

```
fn fb_hline(hl_fb, hl_x, hl_y, hl_n, hl_rgb) {
    if hl_y < 0 { return false }
    if hl_y >= hl_fb[1] { return false }
    hl_x0 = hl_x
    hl_x1 = hl_x + hl_n
    if hl_x0 < 0 { hl_x0 = 0 }
    if hl_x1 > hl_fb[0] { hl_x1 = hl_fb[0] }
    if hl_x1 <= hl_x0 { return false }
    fill32(hl_fb[2] + hl_y * hl_fb[3] + hl_x0 * 4, hl_x1 - hl_x0, hl_rgb)
    return true
}
```

**Dirty rectangles are the other half of the budget.** Breakout's background is static; the
only pixels that change are the paddle, the ball, and the bricks that just died. Redrawing
the old rect and the new rect of each moving entity is a few thousand pixels a frame
instead of 57,600. Design for it from the start — the entity table carries `prev_x/prev_y`
already, for interpolation — but do not implement it in M0; measure first, per W4's rule.

### 2.6 Sprites, and compiling them to spans

A per-pixel transparency test (`if pixel != magenta`) puts a branch in the innermost loop,
which is exactly where §1.3 says zl must not be. **Solution: transparency is resolved at
load time, not draw time.** A sprite is stored as a list of opaque horizontal runs.

```
sprite = [w, h, addr]                   # bmp.zl's [w,h,px] again, addr instead of a list

addr layout:
  +0            i32  nspans
  +4            i32  pixel-block byte offset
  +8 + 16*k     span k:  i32 x, i32 y, i32 n, i32 pixoff
  pixels:       n dwords per span, packed in span order
```

Blitting is then one `copy32` per run, with the same arithmetic clip as `fb_hline`
(adjust `x`, `n`, and `pixoff` together). No branches, no key comparisons, no alpha test.

| Property | Consequence |
|---|---|
| a fully opaque 16×16 sprite | 16 spans, 16 `copy32` calls |
| a sparse sprite (a bullet, a spark) | fewer spans than rows — sparse sprites get *cheaper*, which is the opposite of the naive blitter |
| worst case (dithered/checkerboard alpha) | up to `w/2` spans per row — documented, and the answer is "don't author sprites like that" |

Colour key at **load** time: magenta `0xFF00FF`, the convention every pixel-art tool
already understands, and the one `bmp.zl` can already write. Per-pixel alpha blending is
v2 and needs a different primitive (`blend32`), not a different sprite format.

**Text needs no sprite at all.** A 5×7 bitmap font packed as a hex *string* literal (zl
string literals are unbounded; **list literals cannot span lines** — `easing.zl` says so
explicitly) decoded once into spans. A six-character score is ~210 lit pixels; at ~300 ns
per zl-drawn pixel that is 63 µs, comfortably inside budget with `fb_px`. This is why M4
needs **zero asset files**.

### 2.7 Collision

Three layers, in increasing cost, each used only where needed:

**1. AABB overlap** — the workhorse, all integers, no division:

```
fn aabb(ab_ax, ab_ay, ab_aw, ab_ah, ab_bx, ab_by, ab_bw, ab_bh) {
    if ab_ax + ab_aw <= ab_bx { return false }
    if ab_bx + ab_bw <= ab_ax { return false }
    if ab_ay + ab_ah <= ab_by { return false }
    if ab_by + ab_bh <= ab_ay { return false }
    return true
}
```

Written as four early returns rather than one `and` chain, because `and` evaluates both
sides anyway (`geometry.zl`) so there is nothing to gain and readability to lose.

**2. Swept AABB (slab method)** — for the ball, so speed can rise without tunnelling.
Expand the target box by the mover's half-extents, then intersect the mover's centre ray
with it. Entry time per axis, `t = max(tx0, ty0)`; if that is ≤ the exit time and inside
`[0, FIX]`, it is a hit, and the axis that produced the max is the axis to reflect:

```
# returns [hit, t_fix, axis]   axis 0 = x, 1 = y ; t in 16.16 of this tick
fn sweep(sw_px, sw_py, sw_dx, sw_dy, sw_bx, sw_by, sw_bw, sw_bh)
```

Resolution: move to the contact point, negate the velocity component on the hit axis,
and re-sweep with the remaining `(FIX - t)` of the tick. Two sub-steps handles a corner;
cap at three and stop.

**3. Broadphase by grid indexing, not iteration.** Breakout's bricks are a grid, so the
candidate set is `int(ball_x / brick_w)` ± 1 — arithmetic, not a loop over 40 bricks. For
a tile-based platformer (M5) the same trick gives tile collision for free. A general
spatial hash is not needed and should not be written until something needs it.

Circle tests, when wanted, come from `geometry.zl`'s vocabulary but compare **squared**
distances so no `sqrt` and no float appears.

### 2.8 Audio — defer it, and here is the route when it lands

**Recommendation: defer to M5. It is not on the critical path and it teaches the language
nothing new.** Three reasons, in order of weight:

1. **It proves nothing FFI has not already proven.** `waveOut` is more integer-marshalled
   calls against more structs in raw memory. The window path already demonstrates that
   capability; audio would demonstrate it again, louder.
2. **The per-sample budget is a different design conversation.** 44,100 samples/s with two
   sounds mixed is 88,200 multiply-adds a second. At §1.3's ~50 ns that is 4.4 ms/s — 
   survivable — but a real mixer with eight voices and volume ramps is 35 ms/s and climbing,
   and the answer would be *another* runtime primitive (`mix16`). That decision deserves its
   own measurement, not a guess made while chasing a window.
3. **Silence does not stop Breakout being a game.** The milestone survives without it.

When it lands, the route, cheapest first:

| Step | Mechanism | Cost | Limit |
|---|---|---|---|
| a | **`wav.zl`** — a PCM WAV writer in pure zl | zero FFI, fully testable, the exact complement of `bmp.zl` (44-byte header, then samples) | writes files; makes no sound |
| b | `PlaySoundA(path, 0, SND_ASYNC|SND_FILENAME)` from `winmm.dll` | **3 arguments, one call** | one sound at a time — a new one cuts off the old |
| c | polled `waveOut` double-buffering | ~5 calls: `waveOutOpen` (a 18-byte `WAVEFORMATEX` + an `HWAVEOUT` out-slot), `waveOutPrepareHeader`, `waveOutWrite` × 2 buffers | real mixing, real streaming |

The important structural finding, which is why (c) is not scary: **`waveOutOpen` with
`CALLBACK_NULL` needs no callback.** Poll `WAVEHDR.dwFlags` (offset 20 in the 48-byte x64
`WAVEHDR`) for `WHDR_DONE` (0x0001) once per frame and refill the finished buffer. Audio,
like the window, stays clear of FFI Stage 4.

Generating the sounds is where `noise.zl` reappears: a white-noise burst shaped by
`easing.zl`'s `ease_out_quad` envelope is a passable brick-break, and it is four lines.

### 2.9 Assets

**The blocker first, because it is real and it is already documented in this repo.**
`bmp.zl`'s own demo discovered it:

> `read()` hands back a C string, so it stops at the first zero byte — here that is byte 4

`read()` cannot load binary data. zl can *write* bytes (`write_bytes`) and cannot read them
back. That asymmetry has to close before any asset exists.

**Proposal — two builtins, in `src/runtime/interp.c` and `src/runtime/runtime.c` identically:**

| Builtin | Signature | For |
|---|---|---|
| `read_bytes(path)` | `-> list of 0..255` | the exact complement of `write_bytes`; tools, tests, small files |
| `load_file(path, addr, cap)` | `-> bytes read` | the shipping path: straight into raw memory, no boxed list |

`read_bytes` on a 320×180 BMP is 172,854 boxed numbers, i.e. 172,854 allocations. That is
fine for a unit test and unacceptable in a loading screen, hence both.

**Format: BMP. Do not invent one.** Reasons that compound:

- zl already **writes** it (`bmp.zl`), so the loader is graded against a writer that is
  already tested.
- Every art tool on earth edits it, and 24-bit uncompressed is the one variant everything
  agrees on.
- The loader is ~40 lines: parse the 54-byte header, walk rows bottom-up, convert BGR to
  `0x00RRGGBB`, treat `0xFF00FF` as transparent, and hand the result to the span compiler
  of §2.6.
- It closes a loop that is genuinely pleasing: **`noise.zl` generates a texture → `bmp.zl`
  saves it → the game loads it.** The asset pipeline is written in the language it feeds.

**Embedding, for assets too small to deserve a file:** a hex or base64 *string* literal
decoded at startup (`stdlib/encoding.zl`, `stdlib/base64.zl` already exist). This is the
only way to bake data into source today, because list literals cannot span lines. The 5×7
font (§2.6) uses it.

**An asset pack format is YAGNI.** A folder of `.bmp` files is correct until something
hurts.

### 2.10 The arena problem — the bug the game finds

`design_native_runtime.md` §7 states the runtime's allocation policy plainly:

> **GC:** none. Bump-allocate, never free. A compile run is short-lived; the 64 MB arena is
> plenty.

**A game is not short-lived.** Count the allocations in one frame:

| Source | Per frame | Bytes |
|---|---|---|
| `call!(f, [args])` builds an argument list per call; ~12 FFI calls/frame | 12 lists | ~1.2 KB |
| `cstr(s)` for any string handed to Win32 | 0–2 | ~100 B |
| `str(score)` + `"Score: " + …` for the HUD | ~4 strings | ~200 B |
| entity/vec lists created in the step function | ~20 | ~1.5 KB |

Call it 3 KB a frame. At 60 fps that is **180 KB/s**, and the 64 MB arena is exhausted in
**about six minutes.** Then the bump pointer walks off the end of the `VirtualAlloc`
reservation and the process dies with no diagnostic.

This is a genuine finding, not a hypothetical: no existing zl program loops long enough to
hit it, so nothing in the repo has ever tested it.

**Recommended fix — an arena mark/release stack, roughly ten lines of runtime:**

| Builtin | Does |
|---|---|
| `mark()` | returns the current bump pointer as a tagged int |
| `release(m)` | sets the bump pointer back to `m` |

The frame loop becomes:

```
m = mark()
frame()
release(m)          # everything the frame allocated is gone, at zero cost
```

This is a stack discipline, not garbage collection: no tracing, no headers, no pauses, and
it is exactly right for a program whose lifetime is naturally divided into frames. The
rule the program must obey — *nothing allocated inside a frame may outlive it* — is
enforceable by review, and long-lived data (sprites, the framebuffer, the level) is simply
allocated before the first `mark()`.

Two supporting measures:

- **Hoist argument lists.** `call!` takes a list; pre-build one list per call site at
  startup and mutate it with index assignment (`args[0] = hwnd`), which the interpreter
  already does in place (`src/runtime/interp.c` `N_ASSIGN`/`N_INDEX`: "mutate the list element in
  place"). `nativeval` must match that semantic, or the hoisting silently does nothing.
- **Grow the arena** from 64 MB and make the size a startup option. Necessary but not
  sufficient — it converts "dies in six minutes" into "dies in an hour."

---

## 3. Current-state map (for the implementer)

| Concern | Today | Gap |
|---|---|---|
| pixel buffer | `bmp.zl` — a boxed list, `[w,h,px]`, flat, top-down | needs a raw-memory twin with the same shape |
| BMP output | `bmp_save` works, verified, 24-bit, bottom-up flip contained in one function | reuse as-is for the file presenter |
| BMP input | **none** — `read()` truncates at the first NUL byte | `read_bytes` / `load_file` (§2.9) |
| raw memory | `peek`/`poke` are names in `src/runtime/interp.c:190`'s `SIMULATED[]` table with no widths and no semantics | W5 companion half: `alloc`/`free`/`peek8/32/64`/`poke8/32/64` |
| span primitives | none | `fill32`/`copy32`/`blit32` (§2.5) |
| FFI | designed (`design_ffi_syscalls.md`), not built | Stage 2 (`dll`/`sym`/`call!`) blocks everything visual |
| callbacks | FFI Stage 4, not built | **not required** — §2.2 |
| GUI subsystem | `write_pe` hardcodes console (`pu16(opt+68,3)`) | FFI Stage 1; cosmetic, not blocking |
| timing | `now()` = `clock()`, ms resolution, interpreter-only | `QueryPerformanceCounter` via FFI |
| floats on the FFI engine | `nativeval` is tagged-int only; **no floats** | avoided by design — 16.16 fixed point (§2.4) |
| multiple source files | none; `include` is a proposal (`design_modules.md` Option A) | concatenate as a build step until it lands (§4) |
| long-running allocation | bump arena, never frees | `mark`/`release` (§2.10) |
| noise / easing / vec / geometry | present and tested | `vec`/`geometry` need fixed-point care (§2.4) |

Two things to read from that table. First, **the visual half is blocked entirely on W5**
and nothing else — no new syntax, no type system, no optimizer. Second, **the logic half
is blocked on nothing**, which is why M0 can start immediately.

---

## 4. Staged implementation

Every stage ends with a check that can fail. New files only (`game/` and proposed
`fb`, `win32`, and `sprite` modules under `stdlib/`), so the wave is parallel-safe except for the runtime
bricks, which touch `src/runtime/interp.c`/`src/runtime/runtime.c` and are therefore **serial, alone**, per the
campaign rules.

**Multi-file note:** until `include` lands (`design_modules.md`), a five-line `pack.ps1`
concatenates `win32.zl + fb.zl + sprite.zl + game.zl` into one `build.zl`. Two conventions
make that safe and should be adopted now: game modules carry **no bottom-of-file demo
block** (unlike `stdlib/`), and every function's locals carry the two-letter prefix
`bmp.zl`/`noise.zl` already use, because top-level globals are shared across the whole
concatenation.

### M0 — the whole game, headless, zero FFI

Buildable **today**, on `interp.exe`, against the language as it exists.

1. `fb.zl` backed by a boxed list (the `bmp.zl` representation) behind the §2.5 API.
2. The fixed-timestep loop (§2.4) driven by a **synthetic clock** — `t += 16667` per frame,
   not `now()` — so a run is reproducible.
3. A bouncing box, AABB collision, interpolated rendering.
4. `present_file`: `fb_save_bmp` per frame into `examples_out/`.

**Verify:** 120 frames written; frame 42's SHA-256 is recorded and frozen; a second run
reproduces every byte. Interpreter and boxed C backend produce identical files (all
integer state, so this must hold).

### M1 — raw memory underneath, same pictures on top

Serial (runtime files).

1. W5 companion half: `alloc`/`free`/`peek*`/`poke*`.
2. `fill32`/`copy32`/`blit32` in `src/runtime/interp.c` **and** `src/runtime/runtime.c` (parity rule).
3. Re-point `fb.zl` at raw memory. **No call site above `fb_*` changes.**

**Verify:** M0's 120 BMPs are byte-identical. A new `bench/b6_fill.zl` records the real
cost of a full clear, replacing §1.3's estimate with a measurement.

### M2 — a window, with pixels in it

Serial (FFI bricks). Depends on `design_ffi_syscalls.md` Stage 2.

1. `win32.zl`: the struct offsets and constants of §2.2 as named globals.
2. Register the class with `DefWindowProcA`, create the window, `AdjustWindowRect`,
   cache the DC (`CS_OWNDC`), build the `BITMAPINFOHEADER`.
3. `present_window` via `StretchDIBits`; pump; quit on `IsWindow == 0`.

**Verify:** the M0 bouncing box, on screen, at 1280×720 from a 320×180 buffer, closable by
the X button. `dumpbin /imports` shows kernel32 + user32 + gdi32 and **no CRT**.
Switching `present` back to `"file"` still reproduces M0's frozen SHA.

### M3 — input, real time, and the determinism harness

1. `GetAsyncKeyState` + `GetForegroundWindow` gate; edge events out of the `MSG` struct.
2. `QueryPerformanceCounter`; `Sleep(1)` when the frame finishes early.
3. **The input recorder:** every tick appends the input bitmask to a list; a run can be
   replayed from that list through the file presenter.
4. `mark()`/`release()` per frame (§2.10).

**Verify:** play for 30 seconds recording input; replay through `present_file`; the frames
match a previous replay byte for byte. Run for 30 minutes and watch the arena high-water
mark stay flat — the check that would have caught §2.10.

### M4 — Breakout (§6)

**Verify:** it is playable, and a recorded 60-second winning run replays byte-identically.

### M5 — assets, audio, and a second game

`read_bytes`/`load_file`, the BMP sprite loader, `wav.zl`, `PlaySoundA` one-shots, and a
tile-based platformer to prove the system generalises past one grid of rectangles.

---

## 5. Real syntax, end to end

What `game/breakout.zl` actually looks like once M2–M4 land. Every construct here exists
today or is specified in W5's two design docs.

```
subsystem!("gui")                       # FFI Stage 1 - optional, cosmetic

# ---- bindings ---------------------------------------------------------
user32 = dll!("user32.dll")
gdi32  = dll!("gdi32.dll")
k32    = dll!("kernel32.dll")

RegisterClassA  = sym!(user32, "RegisterClassA")
CreateWindowExA = sym!(user32, "CreateWindowExA")
PeekMessageA    = sym!(user32, "PeekMessageA")
DispatchMessageA= sym!(user32, "DispatchMessageA")
IsWindow        = sym!(user32, "IsWindow")
GetDC           = sym!(user32, "GetDC")
GetAsyncKeyState= sym!(user32, "GetAsyncKeyState")
StretchDIBits   = sym!(gdi32,  "StretchDIBits")
QPC             = sym!(k32,    "QueryPerformanceCounter")

# ---- one-time setup ---------------------------------------------------
FBW = 320
FBH = 180
fb  = fb_new(FBW, FBH)                  # [w, h, addr, pitch]
bmi = bmi_new(FBW, FBH)                 # 40-byte header, biHeight negative
hwnd = win_open("zl breakout", 1280, 720)
hdc  = call!(GetDC, [hwnd])             # CS_OWNDC: hoisted, valid forever
msg  = alloc(48)                        # the MSG struct, reused every frame
qbuf = alloc(8)                         # QueryPerformanceCounter destination

# 16.16 fixed point. The ball is at (160.0, 120.0) moving right and up.
ball_x = 160 * FIX
ball_y = 120 * FIX
ball_vx = fixdiv(90 * FIX, 1000)        # per tick
ball_vy = 0 - fixdiv(140 * FIX, 1000)
prev_bx = ball_x
prev_by = ball_y

acc = 0
last_us = qpc_us()
running = true

# ---- the loop ---------------------------------------------------------
while running {
    m = mark()                                  # §2.10: frame-scoped arena

    if not pump(msg, hwnd) { running = false }

    now_us = qpc_us()
    dt = now_us - last_us
    last_us = now_us
    if dt > MAX_US { dt = MAX_US }
    acc = acc + dt

    while acc >= DT_US {
        prev_bx = ball_x
        prev_by = ball_y
        step()                                  # fixed-step physics
        acc = acc - DT_US
    }

    alpha = int(acc * 65536 / DT_US)
    render(alpha)                               # draws into fb
    call!(StretchDIBits, args13)                # args13 pre-built, §2.10
    call!(Sleep, [1])

    release(m)
}

# ---- render, with interpolation --------------------------------------
fn render(rn_alpha) {
    fb_clear(fb, rgb(8, 8, 16))
    draw_bricks()
    fb_rect(fb, paddle_px(rn_alpha), 168, 40, 4, rgb(220, 220, 220))
    rn_x = topx(prev_bx + int((ball_x - prev_bx) * rn_alpha / 65536))
    rn_y = topx(prev_by + int((ball_y - prev_by) * rn_alpha / 65536))
    fb_rect(fb, rn_x - 2, rn_y - 2, 4, 4, rgb(255, 240, 120))
    text(fb, 4, 4, "SCORE " + str(score), rgb(160, 160, 200))
}
```

Points worth noticing in that listing, because they are decisions and not accidents:
every side-effecting foreign call carries `!` (the FFI doc's §7 gate); there is not one
floating-point number anywhere; `msg`, `qbuf` and `args13` are allocated **before** the
loop and reused; and the entire per-frame FFI surface is six calls.

---

## 6. The first milestone: Breakout

### 6.1 Why Breakout and not the alternatives

The brief is "the smallest thing that is genuinely a playable game." Three candidates:

| Candidate | Why not / why |
|---|---|
| **Pong** | Smaller, but half of it is either a second human or an AI opponent, and a solo wall-bounce is a screensaver, not a game. |
| **Snake** | Genuinely small and genuinely a game — but grid-stepped, so it exercises neither continuous collision nor interpolation. It would let two of the design's load-bearing decisions (§2.4, §2.7) ship untested. |
| **Breakout** | **Recommended.** Continuous motion (interpolation earns its keep), reflection off three surface types (swept AABB earns its keep), a grid of destructibles (grid broadphase earns its keep), win *and* lose states, a score. Roughly 150 lines of zl on top of the framework. |

### 6.2 The exact scope of M4

| Element | Spec |
|---|---|
| resolution | 320×180 framebuffer, 1280×720 window, nearest-neighbour |
| paddle | 40×4, mouse (absolute, scaled) with arrow keys as a fallback |
| ball | 4×4, 16.16 position and velocity, swept collision, speed +5% per brick row cleared |
| bricks | 8 columns × 5 rows, 36×8, grid-indexed broadphase, colour ramp from `easing.zl`'s `tween` across the rows |
| background | a static `noise.zl` `fbm_2d` starfield, rendered **once** into a backing buffer and `copy32`'d — not recomputed per frame |
| HUD | score and lives in the 5×7 hex-string font (§2.6) |
| states | serve → play → life lost → game over → win, with an `easing.zl` `ease_out_back` intro tween on the brick wall |
| assets | **none** |
| audio | **none** |
| sprites | **none** — every element is a filled rect, so even `fb_blit` is optional at M4 |

That last row matters: Breakout can ship before the sprite system does. Sprites arrive with
M5's platformer, where they are actually needed.

### 6.3 What M4 is blocked on

Precisely five things, three of them already designed:

| # | Blocker | Owner | Blocking? |
|---|---|---|---|
| 1 | **Raw memory** — `alloc`, `free`, `peek8/32/64`, `poke8/32/64`. Today `peek`/`poke` are only names in `src/runtime/interp.c:190`'s `SIMULATED[]` list, with no widths and no semantics. | W5 companion half | **Hard.** No framebuffer without it. |
| 2 | **FFI Stage 2** — `dll`/`sym`/`call!` with the ≥5-argument stack path working. `CreateWindowExA` takes 12 arguments, `StretchDIBits` 13; both land in the stack-argument path on their first call. | `design_ffi_syscalls.md` Stage 2 | **Hard.** No window without it. |
| 3 | **`fill32` / `copy32` / `blit32`** in `src/runtime/interp.c` and `src/runtime/runtime.c` (parity). Without them §1.3's arithmetic says the game misses 60 Hz by 3–4×. | this doc, §2.5 | **Hard** for 60 Hz; a 160×90 buffer at 30 Hz limps without them. |
| 4 | **Arena `mark`/`release`** (§2.10), plus in-place list index assignment on `nativeval` so hoisted argument lists actually help. | native runtime | **Soft** — the game runs, and dies after ~6 minutes. Blocking for "playable," not for "runs." |
| 5 | **The native runtime's builtin coverage.** `src/backends/native/nativert.c` implements the 7 builtins the self-host needs. Breakout additionally needs `int`, `abs`, `min`, `max`, `band`, `str`-of-number, `slice`, `index_at`, plus the new raw-memory and FFI set. `zl_int_str` and `zl_at` already exist, so this is a short list of byte loops — but it must be enumerated before M4, not discovered during it. | native runtime | **Hard**, and the one most likely to be forgotten. |

Explicitly **not** blockers, each with the reason:

- **Callbacks (FFI Stage 4)** — removed from the path by §2.2's `DefWindowProcA` decision.
- **Floats (`design_floats.md`)** — removed by §2.4's fixed-point decision. If floats
  arrive first, nothing here needs to change.
- **`subsystem!("gui")`** — a stray console window is ugly, not fatal.
- **The type system (W3) and the optimizer (W4)** — they make it faster; §1.3's budget
  already closes without them, provided #3 lands.
- **`include` (`design_modules.md`)** — a `pack.ps1` concatenator covers M0–M4 (§4).
- **Asset loading (`read_bytes`)** — M4 has no assets, by design.

---

## 7. Risks

| # | Risk | Mitigation |
|---|---|---|
| 1 | **The speed estimate for `nativeval` is a guess.** §1.3 assumes ~50 ns/op from the boxed class; a hand-emitted backend with a runtime call per operation could be 3× worse. | M1 ships `bench/b6_fill.zl` and a frame-time counter before M2 depends on the number. If it is 3× worse: drop to 160×90 (still 4× scaled), add dirty rects, and push `fb_rect` itself into the runtime. All three are pre-planned, none is a redesign. |
| 2 | **`nativeval` FFI is the single point of failure.** Everything visual sits on a trampoline that does not exist yet, on the one backend that can run it. | M0/M1 deliver a complete, testable game with no FFI at all. If Stage 2 slips, W8 still ships something real. |
| 3 | **Struct offsets are silent killers.** One wrong offset in `WNDCLASSA` gives a window that never appears, with no error. | Put every offset in `win32.zl` as a named constant with the field name; add a startup assertion that `RegisterClassA` returned nonzero and `CreateWindowExA` returned nonzero, and `exit(1)` with the field dump if not. `GetLastError` is one more `sym`. |
| 4 | **The arena wall (§2.10) is discovered late**, because it takes six minutes to appear and every test run is shorter than that. | M3's verification includes an explicit 30-minute soak. Make it a checked step, not a hope. |
| 5 | **Fixed-point overflow.** Positions in 16.16 hold ±32,767 — fine. But `fixmul` computes `a * b` *before* dividing, and zl numbers are doubles: exact only to 2^53. Two 16.16 values near 32,767 multiply to ~2^62 and lose bits. | Document the safe range (operands under 2^18 in 16.16, i.e. ±4 in one factor when the other is large); `noise.zl`'s `mul32` already establishes the precedent of splitting a product to stay exact, if it is ever needed. |
| 6 | **`and`/`or` evaluate both sides**, so a guard-and-index idiom imported from another language faults. | Called out in `geometry.zl` and again in §2.5/§2.7; every clip and bounds test in the framework is written as early returns. |
| 7 | **Global-assignment-in-a-function** clobbers state across the concatenated build. | The two-letter-prefix convention (`bmp.zl`, `noise.zl`) is mandatory in game modules, not optional. |
| 8 | **The game is exempt from `verify.ps1`'s cross-engine diff** the moment FFI enters, per `design_ffi_syscalls.md` §1.3. | The file presenter keeps a diffable path forever, and it runs on the interpreter and the boxed C backend — so two of three engines stay under a real gate. That is why §2.1 recommends keeping it permanently. |

---

## 8. Non-goals (explicit)

- **Hardware acceleration.** No D3D, no OpenGL, no shaders. COM vtable dispatch and
  by-value structs are FFI non-goals; software rendering at 320×180 is the point.
- **Alpha blending, rotation, scaling of sprites.** Colour-key spans only. Blending needs
  a `blend32` primitive and a per-pixel read-modify-write — a v2 conversation.
- **A general entity/component system.** Breakout has a paddle, a ball, and a brick grid.
  Fixed arrays in raw memory, not an architecture.
- **Gamepads, `XInput`, raw input.** Keyboard and mouse only.
- **Networking, replays as a shipped feature, save games.** Determinism makes them
  possible; nothing here builds them.
- **Cross-platform.** Win64, GDI, PE. An X11/SDL port is a different document.
- **A level editor.** `noise.zl` plus a `bmp.zl`-authored level image is enough.
- **Audio at M4.** §2.8.

---

## 9. Definition of done

1. §2 locked: `StretchDIBits` presenter, `DefWindowProcA` no-callback window, fixed
   timestep with 16.16 integer state, span-shaped framebuffer with three runtime
   primitives, compiled-span sprites, deferred audio, BMP assets, arena mark/release.
2. **M0**: a complete, deterministic, headless game loop on `interp.exe` with a frozen
   golden-frame hash — reproducible on the boxed C backend too.
3. **M1**: the same frames, byte-identical, out of a raw-memory framebuffer; `b6_fill`
   replaces §1.3's estimate with a measurement.
4. **M2**: a real window showing a real animation, `dumpbin /imports` clean of the CRT,
   and the file presenter still reproducing M0's hash.
5. **M3**: input, real time, recorded-input replay determinism, and a 30-minute soak with
   a flat arena high-water mark.
6. **M4**: **Breakout is playable** — serve, bounce, break, score, lose, win — and a
   recorded winning run replays byte-identically through the file presenter.
7. The proposed `fb`, `win32`, and `sprite` stdlib modules documented in `REFERENCE.md`
   alongside the new builtins (`alloc`/`free`/`peek*`/`poke*`/`fill32`/`copy32`/`blit32`/
   `mark`/`release`/`read_bytes`/`load_file`), and the module count corrected — it drifts
   every wave.
8. `docs/archive/prompts/OVERNIGHT_CAMPAIGN.md`'s W8 row records what shipped, and the arena finding (§2.10) is
   written back into `design_native_runtime.md` §7, which currently says the opposite.
