# CRITICAL — the browser URL bar double-types, and cannot be submitted from the ThinkPad's own keyboard

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366`**

**Status: static read, every link verified against source. Not observed at runtime** —
no gate was run for this audit (see [`README.md`](README.md)). The chain below is
complete and each step carries its file:line; the honest label is *"this is what the
code says"*, not *"this was seen on screen."* It is cheap to confirm: type two
characters into the URL bar.

Found by following a lead from fleet look-agent `wm-focus` (the `'\t'` vs `KEY_TAB`
bug) into the rest of the key-routing path.

---

## Two defects, one cause

`app_event` in `kernel/kernel.zl` handles `APP_BROWSER` **before** the key-translation
layer that exists specifically to prevent both of these, and returns without ever
reaching it.

```
fn app_event(id, win, ety, ecode, ex, ey) {
    if id == APP_SNAKE   { return sn_event(...) }
    if id == APP_MENU    { return menu_event(...) }
    if id == APP_BROWSER {
        if ety == 3 { … br_key(ecode) …  return 1 }      # kernel.zl:2987  EV_CHAR
        if ety == 1 { br_key(ecode);     return 1 }      # kernel.zl:2993  EV_KEY_DOWN  ← raw
        if ety == 4 { … br_click(…) …    return 1 }
        return 1
    }
    # ─────── kernel.zl:2997-3004 — the translation the browser never reaches ───────
    ac = ecode
    if ety == 1 {
        ac = nav_to_char(ecode)
        if ac == 0 { return 0 }        # ← non-nav EV_KEY_DOWN is DROPPED here
        ety = 3
    }
    ecode = ac
```

`nav_to_char` is defined at `kernel.zl:2972` and has **exactly one call site**, line
3001. `grep -n nav_to_char kernel/kernel.zl` returns two lines: the definition and that
call.

---

## Defect 1 — every printable character is inserted twice

`kernel/input.c:343-370` pushes **both** event types for one physical keypress, and for
a printable key both carry the *same value*:

```c
u32 key = ext ? sc_extended(code) : sc_special(code);
u32 ch  = ext ? 0 : to_char(code, mods);
if (!key && ch) key = ch;                 /* printable: code IS the char */
...
evq_push(EV_KEY_DOWN, key, mods, 0, 0);
if (ch) evq_push(EV_CHAR, ch, mods, 0, 0);
```

The USB HID path does the same at `input.c:679-680`. So **both keyboards** deliver
`EV_KEY_DOWN('a')` followed by `EV_CHAR('a')`.

The generic path handles this correctly: `nav_to_char('a')` returns 0, and
`if ac == 0 { return 0 }` **drops the EV_KEY_DOWN**, leaving EV_CHAR as the single
delivery. That `return 0` is the deduplication.

The browser branch skips it and calls `br_key` on both. `browser_key`
(`kernel/browser.c:709-712`) appends on either:

```c
if (code >= 32 && code < 127 && url_len < URL_MAX - 2) {
    url[url_len++] = (char)code;
    return 1;
}
```

There is no dedupe guard anywhere in `browser_key`.

**Typing `ab` produces `aabb`.** The select-all-on-focus logic
(`browser.c:691-695`) clears the buffer on the first keystroke, which masks the very
first duplicate and makes the failure start from the *second* character — the shape
that looks like a rendering glitch rather than an input bug.

## Defect 2 — Enter, Esc and Backspace are dead on PS/2

`kernel.zl:2952-2966` documents this exact hazard, in the file, immediately above the
function that fails to apply it:

> **THE PS/2 KEYBOARD DOES NOT PRODUCE A CHARACTER FOR ENTER.**
> `input.c`'s `sc_plain[]` has 0 at 0x01 (ESC), 0x0E (backspace) and 0x1C (Enter), so
> `handle_scancode` pushes `EV_KEY_DOWN` with a navigation code and skips the `if (ch)`
> that would push `EV_CHAR`. […] An app that listens for EV_CHAR alone can therefore be
> **TYPED INTO but never submitted to** — letters arrive, Enter does nothing, and no
> command ever runs.
>
> That is the ThinkPad's own keyboard, **and no gate could see it: every probe boots
> with `-device usb-kbd` attached.**

`nav_to_char` was written to fix precisely this. The browser bypasses it, so it receives
the raw navigation codes:

| key | PS/2 delivers | `browser_key` tests | match |
|---|---|---|---|
| Enter | `EV_KEY_DOWN 0x104` | `code == 13 \|\| code == 10` (`browser.c:697`) | **no** |
| Esc | `EV_KEY_DOWN 0x101` | `code == 27` (`browser.c:704`) | **no** |
| Backspace | `EV_KEY_DOWN 0x102` | `code == 8 \|\| code == 127` (`browser.c:705`) | **no** |

`0x104` is 260, so it also fails `code >= 32 && code < 127` and falls through to
`return 0`.

**On the ThinkPad's own keyboard the URL bar can be typed into and never submitted,
cancelled, or corrected.** The browser is the project's headline capability and this is
its only text-entry surface.

## Why every gate is green

The comment above already names it: *every probe boots with `-device usb-kbd`.* The
USB HID path maps Enter/Esc/Backspace to 13/27/8 via `xhci.c` `hid_to_ascii`, so
Defect 2 cannot reproduce under QEMU. Defect 1 *does* affect both keyboards — but
`hosttest/browsertest.c` calls `browser_key` directly, once per key, so no harness ever
delivers the EV_KEY_DOWN/EV_CHAR pair that causes it.

This is the repo's `-device usb-kbd` blind spot and its harness-shape blind spot firing
together on one code path.

---

## The fix

**One line.** Delete `kernel.zl:2993` so the browser falls through to the shared
translation:

```zl
if ety == 1 { if br_key(ecode) == 1 { wm_dmg(win) }  return 1 }     # ← remove
```

With it gone, `APP_BROWSER`'s `ety == 3` branch is reached with `ety` rewritten to 3 by
line 3003, `nav_to_char` supplies 13/27/8, and non-nav `EV_KEY_DOWN` is dropped at line
3002 — fixing both defects at once.

That requires moving the `APP_BROWSER` block below the translation, since it currently
sits above it. The minimal alternative, if the block must stay where it is:

```zl
if ety == 1 {
    ac = nav_to_char(ecode)
    if ac == 0 { return 1 }
    if br_key(ac) == 1 { wm_dmg(win) }
    return 1
}
```

### `APP_SNAKE` and `APP_MENU` are fine — checked, so the move is safe

Both also sit above the translation, so both were checked before recommending the move.
**Neither needs to move, and neither has either defect:**

```zl
fn sn_event(ety, ecode) {
    if ety != 3 { return 1 }        # kernel.zl — EV_CHAR only
    if ecode == 119 { … }           # w/s/a/d = 119/115/97/100, plus 'r' = 114
```

```zl
fn menu_event(ety, ecode, ex, ey, win) {
    if ety != 4 { return 1 }        # EV_MOUSE only
```

Snake filters to `EV_CHAR` and drives on WASD, so it never sees the `EV_KEY_DOWN`
duplicate and never needs a navigation code. Menu is mouse-only. **`APP_BROWSER` is the
only app in `app_event` that handles both `ety == 3` and `ety == 1`**, which is exactly
why it is the only one with these defects.

So moving the `APP_BROWSER` block below line 3004 is safe: `APP_SNAKE` and `APP_MENU`
stay where they are and keep working, because they do their own filtering.

Note this also rules out the tempting one-word fix of copying snake's
`if ety != 3 { return 1 }` into the browser — that would stop the double-typing but
would leave Enter, Esc and Backspace dead on PS/2, since those never produce an
`EV_CHAR` at all. The browser needs the translation, not the filter.

## The check to leave behind

`hosttest/browsertest.c` should drive the **event pair**, not `browser_key` directly —
push `EV_KEY_DOWN('a')` + `EV_CHAR('a')` and assert `url_len == 1`. Watch it go red
against the current tree first. A harness that calls the handler once per key can never
see a double-delivery bug, which is the general lesson here and is worth adding to
`docs/GUARDS-THAT-DID-NOT-GUARD.md`.
