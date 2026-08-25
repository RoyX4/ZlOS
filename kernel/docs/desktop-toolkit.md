# The toolkit — the missing layer

Planning document, 2026-08-17. Nothing implemented.

`archive/superseded/desktop-northstar-feasibility.md` named the real gap and then did not solve it:

> **A layout engine.** 105 flex declarations mean the mockup never positions
> anything by hand. zlOS positions *everything* by hand — `sx + 14 * u`,
> `sy + 58 * u`. Adding one line of text to the System Monitor means editing
> every coordinate below it.

This document solves it. It is the layer between `wm.c` (which owns windows) and
`kernel.zl` (which says what an app contains).

---

## The problem, concretely

`draw_sysmon()` today:

```
text_aa(sx + 14 * u, sy + 40 * u,  "CPU",   TXT_DIM)
text_aa(sx + 14 * u, sy + 98 * u,  "MEM",   TXT_DIM)
fill_rgb(sx + 56 * u, sy + 100 * u, 214 * u, 12 * u, ...)
text_aa(sx + 14 * u, sy + 122 * u, "TICKS", TXT_DIM)
```

Every number is hand-placed. Insert one row and you edit every line below it.
Resize the window and nothing moves. There is no concept of "inside" or "next".

---

## The decision: immediate mode

**Not** a retained widget tree (Qt, GTK, SerenityOS's LibGUI). Those allocate
objects, hold parent/child pointers, and run a layout pass over a tree.

**Immediate mode** instead — the Dear ImGui model. You call `ui_button("OK")`
during drawing and it returns whether it was clicked. There is no tree, no
objects, and **no allocation at all.**

### Why it is the right call here, specifically

| Constraint | Retained tree | Immediate mode |
|---|---|---|
| **No heap** | needs one per widget | **none — nothing is allocated** |
| **zl has no lists** | a tree is a list of children | **no tree exists** |
| **No filesystem** | usually loads layout files | layout is code |
| 256 KiB stack | recursive layout passes | a flat loop |
| State lives where? | in the toolkit | **in the app**, where it already is |

The no-heap and no-lists constraints do not merely permit immediate mode — they
**select** it. A retained tree cannot be built here without first building an
allocator, which the project has deliberately refused.

---

## The design

### The context — one struct, in C

```c
#define UI_DRAW    0
#define UI_HITTEST 1

struct ui_ctx {
    int x, y, w, h;      /* the content box we are filling */
    int cx, cy;          /* layout cursor inside it */
    int row_h;           /* tallest widget in the current row */
    int mode;            /* UI_DRAW or UI_HITTEST */
    int mx, my, mdown;   /* pointer state, for hit testing */
    int hot, active;     /* widget ids: under pointer / being pressed */
    int id;              /* auto-increments per widget this pass */
};
static struct ui_ctx ui;
```

**Widget identity is the call order.** `id` increments on every widget call, and
the call order is stable between frames, so widget 3 is the same widget next
frame. That is how ImGui does it and it needs no names and no storage.

### The theme — data, not code

Stolen directly from SerenityOS, where window frame colours and metrics live in
a theme object rather than in the painting code (`os-landscape.md`).

```c
struct ui_theme {
    unsigned bg, panel, text, text_dim, accent, border, danger;
    int pad;        /* inside a panel edge   */
    int gap;        /* between widgets       */
    int row_h;      /* one row of controls   */
    int radius;     /* corner radius         */
    int title_h;
};
static struct ui_theme th;
```

**This one struct kills the "hand-picked spacing numbers" problem** from
`desktop-polish-and-speed.md`. Every widget reads `th.pad` and `th.gap`; nothing
writes a literal.

**And the spacing scale goes in it:** 4 / 8 / 12 / 16 / 24. Nothing may use a
value that is not one of those, times `ui()`.

### Layout — a flowing cursor, no tree

```
ui_begin(x, y, w, h)   set the content box, cursor to top-left + pad
ui_row()               subsequent widgets go left-to-right
ui_col()               subsequent widgets go top-to-bottom
ui_space(n)            advance the cursor by n
ui_end()
```

A widget asks for a size, is placed at the cursor, and the cursor advances by
that size plus `th.gap`. A row wraps at the content width. `row_h` tracks the
tallest thing so the next row clears it.

That is the whole layout algorithm. No constraint solver, no tree walk.

### The zl-facing API — every call returns an int

zl has no lists and no structs, so **every builtin takes and returns plain
integers or a string literal.** That is the whole reason this API looks the way
it does.

```
ui_begin(win)                    start laying out inside a window
ui_row()   ui_col()   ui_space(n)
ui_label(text)
ui_button(text)        -> 1 if clicked this frame
ui_toggle(text, on)    -> the new on/off value
ui_slider(v, lo, hi)   -> the new value
ui_bar(v, max)                   a progress/meter bar
ui_sep()                         a separator line
ui_end()
```

So `draw_sysmon` becomes:

```
ui_begin(w)
  ui_label("CPU")     ui_bar(cpu_pct(), 100)
  ui_label("MEM")     ui_bar(mem_used(), mem_total())
  ui_label("TICKS")   ui_num(ticks())
ui_end()
```

No coordinates. Insert a row anywhere and everything below moves. Resize the
window and it reflows.

---

## Hit testing without drawing — the trick

A click has to know which widget it landed on. In immediate mode, widget
rectangles only exist *during* the layout pass.

**Solution: run the same pass with drawing switched off.**

```
wm_route_click():
    ui.mode = UI_HITTEST
    ui.mx, ui.my = pointer
    app_draw(app, ...)        /* same code, draws nothing */
    -> ui.hot is the widget that was hit
```

Every widget begins with:

```c
if (ui.mode == UI_HITTEST) { if (point_in(rect)) ui.hot = id; return 0; }
```

**This is the same technique `intel.c` already uses.** `intel_modeset_dry()`
walks all 35 steps writing nothing, so the sequence can be reviewed without
touching hardware. `MS_STEP` is a macro rather than a flag precisely because C
evaluates arguments eagerly.

**The UI layer needs the same care:** `ui_button("Run", do_thing())` would
evaluate `do_thing()` even in hit-test mode. So widgets must **return** whether
they fired and the caller acts on the result — never take an action as an
argument. That rule is not stylistic; it is forced by the same language
behaviour that forced `MS_STEP`.

---

## How it fits the rest of the plan

```
kernel.zl     app_draw calls ui_* — no coordinates anywhere
    |
ui.c          NEW: layout cursor, widgets, theme          <- this document
    |
wm.c          windows, z-order, focus, damage, routing    <- archive/superseded/desktop-plan.md
    |
fb.c          pixels + clip rect + damage list            <- archive/superseded/desktop-TODO.md 0b/0c
```

**`ui.c` depends on `fb_clip` existing.** A widget must not draw outside its
window, and clipping is the only thing that guarantees that. So
`archive/superseded/desktop-TODO.md` step 0b is a hard prerequisite — the same step the compositor
and the 3D rasterizer both need. Third customer for one small change.

---

## The widget set — what to build, in order

| # | Widget | Notes |
|---|---|---|
| 1 | `ui_label` | proves layout works. No interaction. |
| 2 | `ui_bar` | the System Monitor already has one, hand-placed |
| 3 | `ui_button` | first interactive widget — proves hot/active and hit-test mode |
| 4 | `ui_sep`, `ui_space` | trivial, make everything else look right |
| 5 | `ui_toggle` | button plus a state return |
| 6 | `ui_slider` | first widget needing a **pointer grab** — proves that path in `wm.c` |
| 7 | `ui_num` | formatted integer; zl has no string building |
| 8 | `ui_list_row` | fixed-height rows for a file list; **not** a scrolling view |
| 9 | `ui_scroll` | needs clipping plus an offset. Do last. |

**Stop at 9.** No text field, no dropdown, no tree view. Those need text editing
and popup layering, and the shell already owns the only text input that matters.

---

## What this does NOT solve

Named honestly, since the last feasibility estimate failed by being optimistic.

- **Not a constraint layout.** No "align these two", no grid spanning. A flowing
  cursor handles rows and columns and nothing cleverer.
- **No text wrapping or measurement** until per-glyph advance widths are emitted
  from `gen_hd_font.py` (`desktop-look.md`). Until then, labels are monospace-
  advanced and `ui_label` cannot know its own width for proportional text.
- **No animation.** That is gated on damage tracking
  (`desktop-polish-and-speed.md`).
- **No accessibility, no input method, no RTL.**
- **Immediate mode fights damage tracking.** IMGUI usually redraws every frame;
  this design must only run `app_draw` when a window is *damaged*. That is
  workable — the app marks itself damaged when its state changes — but it is a
  real tension and the first thing likely to be got wrong.

---

## Estimated shape

`ui.c` at roughly **400–600 lines** for widgets 1–8, plus ~10 new zl builtins.
That is small next to `wm.c`, and it is the layer that changes how every app is
written.

**Do it after `wm.c`, not before** — a widget needs a window to live in. But
design them together, because `ui_slider` is what proves `wm.c`'s pointer-grab
path actually works.

---

Architecture: `archive/superseded/desktop-plan.md` · Task list: `archive/superseded/desktop-TODO.md` · Why the gap
existed: `archive/superseded/desktop-northstar-feasibility.md` · Theme idea source:
`os-landscape.md`
