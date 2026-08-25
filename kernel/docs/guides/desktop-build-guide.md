# Building the zlOS desktop — the plain-English version

Read this **before** `archive/superseded/desktop-plan.md`. That one is the decisions and the
evidence. This one explains what we are actually building and why, in ordinary
words, for someone who has forgotten everything (including future you).

---

## The mental model: paper on a desk

Think of the screen as a desk, and each window as a sheet of paper on it.

- The sheets **overlap**.
- They are in an **order** — some on top, some underneath. That order is called
  the **z-order**.
- When you slide one sheet, only a small part of the desk actually looks
  different. Everything else is unchanged.

A **compositor** is the thing that keeps track of the sheets and repaints the
desk when something moves.

### What zlOS does instead, today

It does not repaint. It does something clever and limited:

1. Takes a **photograph** of the whole desk.
2. Cuts the window out as a **sticker**.
3. Slides the sticker around, wiping the photo back down behind it.

That genuinely works — you can see it working right now. But it only works for
**one** sheet moving over a background that **never changes**. Add a second
moving window, or a clock that ticks underneath, and the photo is out of date and
everything smears.

That is the ceiling we are lifting.

---

## A compositor is only three things

1. **A list of windows.** For each: x, y, width, height, and which app draws it.
2. **An order.** Who is on top of who.
3. **A repaint rule.** Given a rectangle of screen that changed, draw the
   wallpaper, then every window that overlaps that rectangle, bottom to top.

That is the entire concept. Everything else — focus, dragging, menus, close
boxes — is built on those three.

---

## The one idea that makes it possible: clipping

Right now, if you tell zlOS *"draw the System Monitor"*, it draws the whole
window, over everything, everywhere.

There is no way to say *"draw it, but only inside this small rectangle."*

**Clipping** is that. A scissor, or a stencil. You set a rectangle, and every
drawing call after that gets cut off at its edges. Paint outside the stencil
simply does not land.

### Why it is the whole ballgame

When you drag a window, only a thin strip of the screen actually changes.

- **Without clipping:** you must redraw the entire screen to fix that strip.
- **With clipping:** you redraw the strip. That is maybe 2% of the work.

That is the difference between a desktop that feels smooth and one that feels
like a slideshow — and it works the same whether you have 2 windows or 12.

### The good news

`fb.c` has about twenty drawing functions, but **only two of them actually
touch the screen boundary**: `fb_fill_px` (`fb.c:426`) and `put_pixel`
(`fb.c:159`). Every other function — rectangles, shadows, text, icons, gradients,
lines — is built on top of those two.

So **clipping is a change to two functions**, and the whole library gets it.

---

## "Is this how Windows works?"

Yes — this is how Windows 95 through Windows 7 worked. One screen buffer, clip
rectangles, repaint the damaged regions in z-order. It shipped for over a decade
and ran everything.

Modern Windows and macOS do something different: every window draws into its own
private chunk of memory, and the GPU stacks them together. That is nicer, but it
needs two things zlOS does not have — a GPU compositing driver, and a heap to
allocate per-window buffers from.

So we build it the 90s way. **That is not a compromise, it is the correct answer
for a kernel with no heap and no GPU driver.**

---

## Why the window list has to be in C

Short version: **zl cannot hold a list.**

`kernel/README.md:154` says it outright — the kernel subset has no lists. And a
window table *is* a list.

This is not a style opinion. It is why `raise_windows()` in `kernel.zl` is a
hand-written if-ladder over exactly two windows, and why window positions are
loose global variables called `mon_x`, `mon_y`, `ab_x`, `ab_y`. The language
does not permit anything better.

So the split is:

| Lives in C (`wm.c`) | Lives in zl (`kernel.zl`) |
|---|---|
| the window list | the colours and the theme |
| who is on top | where things go on screen |
| which window you clicked | what each app draws |
| which window gets your keys | what the dock launches |
| the repaint loop | what each app *is* |

**C does the bookkeeping. zl does everything that makes it yours.** That is the
same line X11 and Wayland draw against their toolkits.

---

## The build order

Do these in order. Each one has a way to check it actually worked — do not skip
that part.

### Phase 0 — needs no laptop and no touchpad

#### Step 0a — fix the resolution cliff

**The problem, plainly:** one line in `fb.c` says *"if the screen has more pixels
than 1920×1200, do not use the fast RAM buffer."* The ThinkPad panel is
2560×1440. Bigger. So it does not use it.

And four separate features are wired to that same switch, so they all die at
once, silently:

- the fast RAM buffer (drawing goes straight to video memory instead)
- smooth subpixel text (falls back to greyscale)
- fast pixel readback — which every shadow and every rounded corner needs
- **window dragging, entirely** — it just stops working

**The fix is two parts:**

1. Size the buffer from the screen you actually got, instead of a number decided
   at compile time.
2. **If it ever does fall back, print a line in the boot log.** The falling back
   is fine. The *silence* is the bug — it means someone debugs "why is dragging
   broken" for an hour and never thinks to check the resolution.

**The trap:** that buffer sits at a fixed address, 192 MiB. The DMA arena starts
at 224 MiB. At 4K the buffer would end at 223.6 MiB — about 368 KB of clearance.
That is the exact bug class `HANDOFF.md` says has already cost five debugging
sessions. **Compute where the buffer goes. Do not hardcode it.**

**How you know it worked:** force a 2560×1440 mode in QEMU. The desktop draws,
dragging works, and the boot log states which mode it got.

#### Step 0b — add clipping

Add four numbers to `fb.c` (`clip_x0`, `clip_y0`, `clip_x1`, `clip_y1`) and two
functions to set and clear them. Then change `fb_fill_px` and `put_pixel` to
clamp against those instead of the screen edge.

**How you know it worked:** set the clip to the whole screen and take a
screenshot. It must be **pixel-identical** to before. If anything moved, the
clamp is wrong.

#### Step 0c — add a damage list

Today `fb.c` tracks **one** rectangle of "stuff changed", and it grows to swallow
everything. A clock ticking in one corner plus a monitor updating in the other
means the whole screen gets pushed every second.

Replace it with a small array — say 8 rectangles. When it fills up, merge them
all into one, which is exactly today's behaviour. So the worst case is "as slow
as now", never "wrong".

**How you know it worked:** screenshot identical, but the pushed area is smaller.

#### Step 0d — make the mouse produce events

The keyboard already produces proper events — press, release, character, with
modifiers attached. The mouse does not. It is a separate variable you poll.

`input.c:40` already declares `EV_MOUSE` and **nothing ever creates one**. Wire
it up so both come out of the same queue.

**How you know it worked:** the existing mouse demo still works, reading events
instead of globals.

### Phase 1 — needs the ThinkPad

The touchpad. Three jobs, not one:

1. Get the I2C transport working on real hardware. `i2c_hid.c` has never run —
   QEMU has no Intel LPSS I2C, so it cannot be tested here at all.
2. **Write the decoder.** This does not exist. Today `i2c_hid_byte(i)` hands back
   raw undecoded bytes — there is no code anywhere that turns a touchpad report
   into an x, a y and a button.
3. Write the pointer policy. A laptop touchpad is not a mouse: it reports
   *absolute* positions, how many fingers are down, and whether they are pressing.
   Somebody has to decide what a tap means, what two fingers scrolling means, and
   how pad coordinates map to screen coordinates.

You can write and test #2 against a captured dump before #1 works.

### Phase 2 — the compositor itself

Now build the thing. In order:

1. `wm.c` — the window array and the z-order array
2. the repaint loop (the four lines below)
3. routing — clicks and keys going to the right window
4. delete the photo-and-sticker code
5. convert the apps, snake first
6. the shell becomes app 0

**The repaint loop, in full:**

```
for each damaged rectangle R:
    fb_clip(R)
    draw the wallpaper
    for each window, bottom to top:
        if it overlaps R: draw it
    fb_clip_none()
```

That is not pseudocode standing in for something complicated. That is the
compositor.

---

## What an app becomes

Today every demo owns the machine. `snake_game()` is a `while` loop that runs
until you press a key, then hands the screen back. That is why everything ends
with "press any key to exit".

After the change, an app is three functions and **no loop**:

```
app_draw (id, x, y, w, h, focus)    repaint my inside
app_event(id, win, type, code, x, y) here is a keypress or a click
app_tick (id, win)                   one frame happened, update yourself
```

The compositor owns the loop and calls these. Snake keeps playing while you drag
the System Monitor, because snake is no longer *the* loop — it is one thing
called from it.

**Seven apps to convert:** `snake_game`, `paint`, `cube_demo`, `anim`, `editor`,
`mousedemo`, `windows_demo`.

**Start with snake.** Its state already lives in raw memory (`SNAKE_X`,
`SNAKE_Y`), so there is less to hoist than the others.

**Two are already the right shape:** `draw_sysmon(sx, sy, sfoc)` and
`draw_about(ax, ay, afoc)` already take their position as arguments and hard-code
nothing. Copy that pattern.

---

## What gets deleted

This matters as much as what gets built. Once real repainting exists, the
photo-and-sticker machinery is dead code: `fb_bg_snapshot`, `fb_bg_restore`,
`fb_grab`, `fb_stamp`, and the two buffers `bg_buf` and `sp_buf`
(`fb.c:775-834`).

Deleting it also deletes four problems:

- **The 640×480 window size limit.** The sticker buffer is that big. Your
  terminal is 1256×944 — nearly four times over. *That* is why the terminal
  cannot be dragged. Not a design decision. A buffer size.
- **The halo artifact.** The sticker is cut off one background and pressed onto
  another, so its soft shadow edges carry the *old* background with them. On a
  gradient wallpaper you can see it.
- 10 MB of fixed high memory.
- The start menu's separate save-and-restore path — a menu just becomes a window.

**Keep** the mouse cursor's save-under (`fb_pointer_show`/`fb_pointer_hide`).
For an 11×17 patch that technique is genuinely correct and has none of these
problems.

---

## Vocabulary

Terms used in the code and the other docs.

| Word | Means |
|---|---|
| **compositor** | the thing that keeps the window list and repaints the screen |
| **z-order** | the stacking order — who is in front of who |
| **clipping / scissor** | a rectangle that cuts off all drawing at its edges |
| **damage** | a rectangle of screen that changed and needs repainting |
| **focus** | which window gets your keystrokes |
| **pointer grab** | one window taking *all* mouse events until you let go, used during a drag so it does not break when the pointer outruns the window |
| **modal** | a window that takes everything until dismissed — a menu |
| **back buffer** | a copy of the screen in normal RAM; you draw there and copy it out, because reading video memory is 30–50× slower than reading RAM |
| **immediate mode** | draw it and forget it — nothing remembers what is on screen. What `fb.c` does today |
| **retained mode** | the system keeps a model of what should be on screen and repaints from it. What we are moving toward |

---

## The short version

- A compositor is a **list**, an **order**, and a **repaint rule**.
- **Clipping** is what makes the repaint rule cheap, and it is two functions.
- The list has to be in **C** because zl has no lists.
- Apps stop being **loops** and become **three callbacks**.
- Fix the **resolution** thing first — it needs no hardware and everything else
  sits behind it.

Full detail, evidence and line numbers: `archive/superseded/desktop-plan.md`.
Current status and blockers: `../../.ultra/STATE.md`.
