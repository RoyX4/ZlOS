# Wiring the compositor in — the last step, and the only one left

**Status 2026-08-18: `wm.c`, `ui.c`, `wmglue.c` and `ui.h` are built, tested
and linked into every kernel. Nothing calls them.** This document is the
remaining work, written out so it is a copy-paste rather than a design job.

It was not done in the overnight run for one reason: every line below goes in
`kernel.zl`, and `kernel.zl` was mid-flight in another session all night — its
`mouse_x` builtin was rewired underneath me *while I was measuring it*. The
brief forbids staging another session's unfinished work, so the mechanism got
built and the policy was left. See `.ultra/TENSIONS.md` T-8 and T-9.

---

## Why this is small

**zl compiles to C.** A zl function

```
fn app_draw(id, x, y, w, h, focus) { ... }
```

becomes, in the generated `_gen.c`:

```c
Value zl_fn_app_draw(Value, Value, Value, Value, Value, Value);
```

So C calling zl is an ordinary function call — no interpreter, no dispatch
table, no marshalling past boxing a number. That single fact is what made the
layer split in `ui.h` possible at all; without it the app callbacks would have
had to live in C and the "policy in zl" half of DECISIONS #1 would have been a
fiction.

`wmglue.c` already contains every shim. Its references to the four zl functions
are **weak**, so it links today in a kernel where they do not exist, and starts
working the day they do — with no change to any C file.

---

## Step 1 — the three app functions

The contract is in `ui.h` and it is three functions with **no loop, ever**.

```
# app_draw: repaint my CLIENT AREA. Called only when damaged, not every frame.
# MUST be position-pure - every coordinate derived from the x,y,w,h passed in.
# draw_sysmon(sx, sy, sfoc) already has exactly this shape; copy it.
fn app_draw(id, x, y, w, h, focus) {
    if id == APP_SHELL   { return draw_shell(x, y, w, h, focus) }
    if id == APP_MONITOR { return draw_sysmon(x, y, focus) }
    if id == APP_ABOUT   { return draw_about(x, y, focus) }
    if id == APP_SNAKE   { return snake_draw(x, y, w, h) }
    return 0
}

# app_event: return 1 if handled. type 4 is EV_MOUSE, 3 is EV_CHAR.
fn app_event(id, win, type, code, x, y) {
    if id == APP_SHELL { return shell_event(type, code) }
    if id == APP_SNAKE { return snake_event(type, code) }
    return 0
}

# app_tick: runs EVERY frame, must be cheap, MUST NOT DRAW. Return 1 to say
# "my state changed, repaint me". This is how a clock ticks and a snake moves
# without owning the frame - and it is the whole reason those demos stop
# needing a while-loop.
fn app_tick(id, win) {
    if id == APP_MONITOR { return mon_changed() }
    if id == APP_SNAKE   { return snake_step() }
    return 0
}

# the wallpaper pass: furniture, drawn before any window, never in the z-order
fn desk_draw(x, y, w, h) {
    grad_rgb(0, 0, px_w(), px_h(), WALL_TOP, WALL_BOT)
    draw_header()
    draw_dock()
    return 0
}
```

`desk_draw` is handed the damage rectangle. It may ignore it and draw the whole
desktop — **the scissor is already set to that rectangle, so nothing outside it
reaches the screen**. Ignoring it is correct but wasteful; using it to skip work
is the optimisation, not the correctness.

## Step 2 — the boot sequence

At the bottom of `kernel.zl`, replacing `while running == 1 { ... }`:

```
kbd_init()
fs_init()

if wm_available() == 0 {
    # TEXT MODE. No compositor at all. Keep the OLD shell loop, unchanged.
    # verify.sh boots -kernel -display none, where QEMU's multiboot loader
    # supplies no framebuffer tag, so px_w() == 0 and this is the path taken.
    # Its serial transcript must stay BYTE-IDENTICAL to golden.txt.
    while running == 1 { ...the existing loop, untouched... }
} else {
    layout()
    ui_theme_init(ui())
    wm_init()
    wm_bind_zl()

    # ORDERING THAT MATTERS: the shell window must EXIST before the boot log
    # prints, because the log goes inside it. draw_desk() already does this
    # shape today - the terminal frame is drawn and then the log fills it.
    w_shell = wm_open(APP_SHELL, "zl shell   ~", term_x, term_y, term_w, term_h)
    ...boot log prints into window 0's client area, as it does today...
    wm_open(APP_MONITOR, "System Monitor", mon_x, mon_y, MON_W * ui(), MON_H * ui())
    wm_open(APP_ABOUT, "About", ab_x, ab_y, MON_W * ui(), AB_H * ui())

    while wm_running() == 1 { wm_frame() }
    print("halting")          # verify.sh greps for this
}
```

`wm_available()` is in `wmglue.c` and checks **both** conditions, which are
different and both necessary: that there is a framebuffer at all, and that
`kernel.zl` actually defines `app_draw`. Either one missing means the text
shell.

## Step 3 — delete the sticker drag (C4 / task 2d)

Once `wm_frame()` owns the screen, nothing calls `bg_snap`, `bg_rest`, `grab`
or `stamp`. Delete the builtins in `runtime_kernel.c`, then delete
`fb_bg_snapshot`, `fb_bg_restore`, `fb_grab`, `fb_stamp`, `bg_buf` and `sp_buf`
from `fb.c`. **Keep `fb_pointer_show`/`fb_pointer_hide`** — the 11×17 cursor
save-under is a different and correct technique.

That takes with it:

- the 640×480 drag ceiling (the terminal is 1256×944, nearly 4× over — the real
  reason it could never be dragged)
- the shadow-halo smear trail, measured in task 0a: the shadow reaches
  `x + w + 28` at `ui() == 2` and the drag erased only `w + 16`, so it was
  **12 px short on every step**
- ~10 MiB of fixed high RAM at 128 and 160 MiB

And it unblocks the last degraded mode: with 128–176 MiB free, `back` can move
down and cover 3840×2160, which `fb_setup` currently reports as
`back OFF, drag ON`.

## Step 4 — the interaction model (Group E2)

Most of it is already in `wm.c` and needs only policy:

| | where it is |
|---|---|
| close box and Ctrl+W | **done**, `wm.c` `route_key` / `in_closebox` |
| Alt+Tab cycles focus | **done**, walks the z-order backwards |
| the dock launches into a window | `dock_cmd()` calls `wm_open(id)` instead of `run_command` |
| Super opens the start menu | `MOD_SUPER` is tracked and used for nothing; open a `WF_MODAL` window (C5) |
| `read_line` stops looping | a state machine fed one char per `app_event`. Smaller than it sounds: `LINE_BUF` and the history are **already** in raw memory, so the state is already external |

---

## How to know it worked

```bash
cd kernel && ./verify.sh          # the text path - MUST stay byte-identical
./verify-efi.sh                   # the 64-bit UEFI path, the laptop's
./probe-shot.py -o wired          # look at it
./probe-drag.py                   # drag a window in the real device config
cd hosttest && ./build.sh && ./wmtest && ./inputtest && ./fbbench
```

`verify.sh` is the one that matters and the one most likely to go red first.
The compositor must be **unreachable** when `px_w() == 0`; if the text path
prints one extra line, the gate fails.

---

Contract: `ui.h` · Mechanism: `wm.c` · Widgets: `ui.c` · Seam: `wmglue.c` ·
Task list: `desktop-TODO.md` · Blocks: `../../.ultra/TENSIONS.md` T-8, T-9
