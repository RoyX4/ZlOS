# Workspaces — what filters, and what deliberately does not

Three workspaces, numbered from 1. The pip row in the top island is the
indicator and, since this change, also the control.

## The one thing that was wrong before

`kernel.zl` held a `ws_cur` variable. `draw_header` read it to decide which pip
was a 20px lime lozenge and which two were 7px dots. **Nothing else read it.**
No window carried a workspace, no paint or hit test consulted one, and `set_ws`
had no caller at all — the pips were not clickable and there was no key
binding. So the feature was an indicator that told the truth about a variable
and lied about the machine.

The fix is not "add filtering next to the variable". It is to move the variable
to where the windows are. **`wm.c` owns the current workspace now**, because it
owns the window table, and `cur_ws()` / `set_ws()` in `kernel.zl` are one call
each.

## What a workspace is here

A field on a window and an integer on the compositor. Not a second z-order.

```
struct win { ...; int ws; };      /* wm.c */
static int ws_cur = 1;            /* the one being looked at */
static int ws_n   = 1;            /* the ceiling; told, not assumed */
```

`zorder[]` stays **global and untouched** by a switch. That is the property
worth having: coming back to a workspace shows the stack exactly as it was
left, because nothing reordered it while you were away. The filter is applied
at read time, by one predicate:

```
static int on_ws(int win)      { return wins[win].ws == ws_cur; }
static int win_visible(int win){ return !(wins[win].flags & WF_MINIMIZED) && on_ws(win); }
```

It is one predicate and not five inline comparisons for a specific reason: five
sites are five places to forget, and a window that paints but cannot be clicked
(or the reverse) is a far worse bug than one that is simply hidden.

## What filters

| site | function | effect |
|---|---|---|
| paint | `wm_repaint`'s z-order walk | a window on another workspace draws nothing |
| hit test | `wm_at` | clicks fall through to the desktop |
| modal | `modal_win` | a modal on another workspace does not swallow input |
| focus | `top_visible` | close / minimise / switch land focus on something visible |
| Alt+Tab | `cycle_focus` | bounded walk, skips other workspaces |
| taskbar | `draw_tasklist`, `task_slot_at` in kernel.zl | both loops, one edit |

The taskbar needed **both** loops changed in the same edit. `task_slot_at`
counts chips to turn an x into a window; if it counts a different set than
`draw_tasklist` draws, every click after the first hidden window lands on the
wrong one.

## What does not filter, on purpose

- **`wm_damage_win`** is unchanged. Damaging a hidden window's rectangle simply
  repaints the wallpaper there, which is correct.
- **The dock's per-tile "app is running" accent** is workspace-independent, as
  it is in the reference (`st.open[a.id]`, with no `winWs` test).
- **`wm_raise` and `wm_focus` PULL a window to the current workspace** rather
  than refusing. Every caller is someone asking for the window — a dock tile, a
  taskbar chip, `reg_open()` finding the app already running — and the
  alternative is a click that produces nothing at all, because the window
  really did come to the front of a stack nobody is looking at. It is also what
  the reference does: `ds.html`'s `openApp()` writes `winWs[id] = s.ws` whether
  the window was already open or not.

  Both need it, not just `wm_raise`: `reg_open()` calls focus *then* raise, and
  between those two lines focus would otherwise sit on an invisible window.
  Pulling is not raising — the z-order is untouched, so wmtest's "focus does
  not imply raise" still holds. It is a different axis.

## Where a new window lands

On the workspace you are looking at. `wm_open` writes `ws = ws_cur`.

That is the reference's rule too, and it is worth being precise about, because
the `ws:` field in `ds-reference.html`'s APPS table looks like it says
otherwise. It does not. That table seeds `winWs` **once**, at mount; `openApp()`
overwrites it with the current workspace every time. So the per-app `ws:` value
is what a window carries *before anyone has opened it*, and the only place it
can be observed is the boot set.

`app_ws(id)` in `kernel.zl` is that table — the twenty named apps one by one,
plus two lines standing in for the thirty-four utilities and games via
`reg_cat()`, which splits exactly where the reference's two `.map()`s do. It is
read in exactly one place: `wm_boot_start`, where the three windows the desktop
opens for itself (Terminal, Files, System Monitor) get their workspace from the
same table their size and position already come from. All three are workspace 1
in the reference, so today those three calls change nothing and are refused as
no-ops. They are there so the day the boot set gains a fourth window, its
workspace does not come from the accident of what was current at boot.

## The controls

- **Click a pip.** `pip_at()` in `kernel.zl`, checked in `desk_click` between
  the Activities corner and the dock.
- **Super+1/2/3** switches; **Super+Shift+1/2/3** sends the focused window.

The key bindings exist instead of the reference's title-bar "move to workspace"
menu item. zlOS's title bar has three controls and no menu at all, so that
menu item would have needed a whole popup surface built before it could be
reached once; the bindings are the same capability for four lines, in
`route_key` beside the two window bindings that already live there.

`pip_at` repeats `draw_header`'s arithmetic rather than reading a stored x, and
that is the second-best shape, chosen knowingly. The island is laid out as it
is painted, so the pip row's x exists only as a local inside that loop. The
rule this file's neighbours learned the hard way — see `app_tick`'s note on
`draw_status` and the tray strip — is that a second *independently derived*
origin for the same row is how a control ends up drawn in one place and
clickable in another. So `pip_at` sits directly below the function it mirrors,
one term at a time in the same order, and spells out `island_sep`'s advance
inline because `island_sep` itself draws and a hit test must not.

The hit band is the **whole island height**, not the 7px pip. A seven-pixel-tall
target is not clickable by a person and nothing else occupies that column.

## How many, and why the compositor is told rather than deciding

`WS_N = 3` stays in `kernel.zl` — three pips is a design decision.
`wm_wsn(WS_N)` hands it to the compositor right after `wm_init`, and `ws_n`
**defaults to 1**. Two consequences, both wanted:

1. Every host test (`wmtest`, `wmtest_feel`, `toasttest`, `wmshot`) calls none
   of the workspace setters, so `ws_n` stays 1, every window's `ws` is 1, and
   `on_ws()` is identically true — the filter is provably inert there and those
   harnesses behave exactly as they did before this existed.
2. `Super+9` is refused rather than switching to an empty ninth workspace with
   no pip to come back from.

`wm_set_ws_n` also refuses to *shrink* below where anything currently is: a
window stranded on workspace 4 after the count drops to 3 would be open,
drawn nowhere, and focusable by nothing.

## The consequence nobody should have to rediscover

Serial bytes are input. `input.c` drains COM1 into the event queue and `wm.c`
routes them to the **focus window** — so on a workspace with no windows,
`focus_win` is -1 and typing into the serial console does nothing.

That is correct (no window, no keys — every WM behaves this way) but it is a
trap for the oracle: **you cannot switch to an empty workspace and then type a
shell command to open something there.** `kernel/tests/oracle/shot-apps.py` documents
the same wall from the other side, and it is why the per-app screenshots keep
the boot desktop behind the app instead of hiding it on another workspace.
