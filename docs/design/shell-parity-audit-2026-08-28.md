# The desktop shell against the prototype — a four-agent audit

2026-08-28. Four read-only agents were pointed at
`docs/design/presswork-prototype.html` and told to state the repo's claims as
fact and then verify them. Three took an app renderer each; the fourth was told
to audit everything that is **not** an app renderer, on the theory that the
parity effort had been walking the register rail app by app and would therefore
have missed the shell the apps sit in.

That theory was right, and the shell is where the serious defects were.

Every finding below is marked with how it was established. Nothing here was
accepted because an agent said it — the ones marked REPRODUCED were re-derived
from source in the main session before being acted on.

---

## The trap: Escape and F1 were unreachable, and their constants were wrong

**REPRODUCED. Fixed.** This is the worst defect found in the parity effort and
it is worth stating carefully, because two independent bugs were protecting each
other.

`desk_key` in `kernel.zl` handles three keys: Escape, F1 and Super. It opens with
this comment:

> ESC DISMISSES WHATEVER IS UP, and it is tested first so a modal cannot trap the
> desktop. The prototype prints "esc to dismiss" on two of its five surfaces; a
> modal that lists a key it does not answer is worse than one that lists none.

**Layer one — the routing.** `hook_desk_key` has exactly one call site, and it is
gated:

```c
if (type == EV_KEY_DOWN && code == KEY_SUPER) {
    if (hook_desk_key) hook_desk_key(code, mods);
    return;
}
```

Escape and F1 never reach `desk_key` at all. So every overlay — palette,
activities, field menu, lock sheet — could be opened and **not dismissed**, while
`ov_draw_activities` painted the words `ESC TO DISMISS` on screen. The function
states the exact contract it is failing, one file away from the reason.

**Layer two — the constants.** Even routed, it would not have worked:

| `desk_key` tests | this kernel's actual code |
|---|---|
| `27` (Escape) | `KEY_ESC` = `0x101` |
| `0x13B` (F1) | `KEY_F1` = `0x120` |
| `0x11A` (Super) | `KEY_SUPER` = `0x11A` ✓ |

`27` and `0x13B` are Linux-style codes (`0x100 | 0x1B`, `0x100 | 0x3B`) from a
scheme this kernel does not use. `keycodes.h` has its own.

**Why both survived.** Look at which one is correct. Super is the only binding
that was ever *reachable*, and Super is the only binding whose constant is
*right*. The other two were written, never routed, therefore never pressed, and a
key nobody can press cannot be observed to test the wrong number. The routing bug
made the constant bug invisible; the constant bug meant fixing the routing alone
would have produced no visible change and probably been reverted as ineffective.

This is the sharpest instance yet of a rule this repo keeps relearning: complete-
looking wiring is the half people check. Something has to actually CALL it, and
the call has to arrive with the value the callee is looking for.

---

## Confirmed and fixed this session

| # | Finding | Status |
|---|---|---|
| 1 | **The System Monitor's CPU chart was fabricated** — seven `line()` calls at fixed offsets `-3,-1,-6,-3,-7,-4,-6`, in an instrument, filled with the overprint against `design.h:659`. The real series (`wm_sn()`/`wm_sf(i)`, a 256-deep per-frame microsecond ring) was bound to zl the whole time. | REPRODUCED, replaced with the measurement, scaled against `wm_budget()` |
| 2 | **`wm_frame()` used as a value** — it is bound `{ wm_frame(); return zl_nil(); }`, the compositor's per-frame *driver*. The refresh card printed nil as `0` and re-entered the compositor from inside `app_draw`. | REPRODUCED, now `wm_painted()` |
| 3 | **`rail_win()` was a fifth stale table keyed by position** — `rail_app(3)` said `APP_EDIT`, `rail_win(3)` returned `browser_win`. The browser wore the editor's register and subtitle; slots 11–13 got no label. | REPRODUCED, table deleted, now derived from `rail_app` + `wm_app()` |
| 4 | **`wins[win].app_us` was measured and unreadable** — no accessor in any `.c`, `.h` or `.zl`. The settings pane printed `wm_us()` (the whole frame) labelled `THIS PANE, LAST DRAW`, with a comment above asserting it was per-window. | REPRODUCED, `wm_win_us(win)` exported |
| 5 | **Escape and F1** — above. | REPRODUCED |
| 6 | **`HANDOFF-APP-PARITY.md` was stale about `R.set`**, saying it was open and must be reconciled against `settings.c` — when `R.set` shipped in `3b8692b` and `settings.c`'s draw path has no caller outside the host test. | REPRODUCED, doc corrected |

---

## Reported, not yet acted on

Ordered by how quickly a person would hit them. These are the agents' readings;
each still needs reproducing before it is treated as fact.

**Would be hit immediately**
- Right-click on the desk cannot open the field menu. `desk_click` does
  `down = band(cbtn, 1)` — bit 0 only — so a right press yields `down == 0` and
  returns. `desk_menu_at` has one caller: the `ctxmenu` *shell command*, at a
  hardcoded position.
- No overlay is modal to the pointer. `ov_active()` is defined and has **zero
  readers**; `wm_overlay` is a draw hook only. Clicks pass through the palette to
  the windows and rail underneath.
- Windows have no drag bounds. `wm_move` assigns x/y verbatim; the only rail
  reservation is inside `snap_to_rect`, so snapped windows respect the rail and
  dragged ones do not. A window can be dragged over the rail, the strip, the
  foot, or to negative coordinates with no route back. The prototype states this
  as its rule 1.
- Five of the six shortcuts the context menu advertises have no handler at all
  (`CTRL K`, `CTRL G`, `CTRL L`, `ALT 1`, `ALT 2`); the knockout toggle is
  window-scoped inside Settings, not global.

**Visible on inspection**
- The rail's two mono readouts use `TH_SURF_7` = `ZD_TEXT_INERT`, measured
  **2.02:1** on `ZD_RAISE`. `ui.c:187` labels that token `STRUCTURE ONLY. never a
  glyph.` and the prototype's own CSS names 2.02 as the reason there is no fifth
  text rung. The prototype uses `ink3` (5.36:1) for both.
- The scrollbar has the ladder inverted: track is `panel_hi` (a *raised* rung)
  where the prototype has a sunken pit, thumb is `ZD_TEXT_3` where the prototype
  uses `ZD_TEXT_INERT`. Taken with the rail readouts, the two `ZD_TEXT_INERT`
  decisions are exactly swapped — used where forbidden, unused where sanctioned.
- The focus transition, which `design.h` calls the load-bearing half of the focus
  signal (`ZD_MS_RISE 90`, with a CUT explicitly *refused*), is not implemented.
  The prototype puts six RISE transitions on the focus change; `wm_focus` only
  invalidates and damages. The knockout flip is the refused instantaneous cut.
- The scrim constant is derived from a version of the prototype that no longer
  exists. `kernel.zl` cites `.scrim { background: rgba(0,0,0,.38) }`; `rgba(` does
  not appear anywhere in the current prototype, which uses
  `color-mix(in srgb, var(--zd-cut) 78%, transparent)`. The shipped scrim is
  roughly half the specified strength and neutral where it should be warm.
- The toast is a different object: one at a time vs three stacked, single line vs
  title+body, and **three different dwell numbers** across the tree (`NOTE_TICKS`
  3 s, `ZD_TOAST_MS` 4200, prototype 8000). `notify_hit()` has no caller, so a
  toast cannot be clicked away.
- The activities overview uses a scrim over the whole screen where the prototype
  uses an opaque desk-coloured panel over the field only; its focused tile takes
  a full-height vermilion bar that crosses its own header — vermilion on the
  knockout measures **1.40:1** — where the prototype knocks the header out and
  starts the bar below it.

**Subtler**
- `.scroll::after`, the 2px sticky trim rule, is not drawn anywhere. Its purpose
  is stated in the prototype: a cut-off last row reads as a rendering fault
  rather than as more-below. Every scrolling pane chops its last row against bare
  ground.
- The register truncates rather than scrolls, so on a short panel the overflow
  rows are unreachable — and the rail is stated to be the only route to every
  system function.
- The command palette shows 10 of the prototype's 16 rows, prints `10 of 10`
  where the prototype prints matches-of-total, and has no query, no arrow
  navigation and no Enter. `ov_pal_app(i)` — written specifically so the palette
  would return an app id rather than a label — is defined and referenced nowhere.
- Menu separators are 1px `TH_CUT` inset 8dp; the prototype's are 2dp full-width
  `ZD_LIT`. Menu hover uses `ZD_FLOAT` + a bar; the prototype uses the knockout.
- The three off-plane objects (menu, sheet, toast) carry none of their off-plane
  treatment — no `ZD_LIFT` shadow, no `ZD_EDGE_OVER` ring, no radius, no
  knocked-out header band. Only the toast gets a shadow.

## What the OS has that the prototype does not

Recorded so nobody "fixes" these toward the prototype: edge-drag snapping with a
live preview, Super+arrow snap, Super+1..9 workspaces, Alt+Tab z-cycling, Ctrl+W
close, wheel routed to the window under the pointer, and window tabs. The
prototype has no snapping and no wheel handler — every scroll there is native
CSS overflow. `THE PROTOTYPE WINS` arbitrates disagreements; it does not require
deleting capability the prototype never had.

## Method note

The fourth agent computed its own contrast figures from `design.h`'s hex rather
than quoting them, which is why the 2.02:1 and 1.40:1 numbers above are load-
bearing rather than decorative. It also flagged what it could not determine:
it did not build or boot, so every claim is from source, grep and arithmetic.
The six findings in the fixed table were re-derived in the main session before
any edit — three of them (the fabricated chart, the `wm_frame()` value, the
keycode mismatch) turned out to be exactly as reported, and the `rail_win`
finding was verifiable by reading two adjacent functions.
