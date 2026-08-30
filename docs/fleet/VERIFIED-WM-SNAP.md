# The two snap state machines in `wm.c` — verified by hand

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366`**

A fleet agent (lens `wm-drag`) reported drag-to-edge snapping as a **one-way door**.
The conclusion is **correct**. Its stated evidence was **wrong**, and the difference
matters, because acting on the wrong evidence would have produced the wrong patch.

---

## What the agent claimed

> `snap_release()` has no reachable caller in the shipping tree at all, because its
> only call chain runs through `wm_snap_key()`, whose sole caller in the whole repo is
> `hosttest/toasttest.c:328`.

## What is actually true

`snap_release()` **does** have a shipping caller:

```
$ grep -rnw snap_release kernel/ | grep -v hosttest
kernel/src/graphics/windowing/wm.c:102:  int  snap_release(int win, int *x, int *y, int *w, int *h);   /* decl */
kernel/src/graphics/windowing/wm.c:1281:     if (!snap_release(win, &nx, &ny, &nw, &nh)) return;       /* call */
kernel/src/graphics/ui/snap.c:151: int snap_release(int win, int *x, int *y, int *w, int *h)   /* defn */
```

So the flat "no caller" claim is false. The *reachability* claim survives, for a
different and more interesting reason.

---

## The real chain

```
snap_release()                       snap.c:151
  ← snap_to_rect(win, SNAP_NONE, …)  wm.c:1281   ── the un-snap branch
      ← snap_to(win, z)              wm.c:1301
          ← wm_snap_key(win, dir)    wm.c:1308
              ← hosttest/toasttest.c:328          ── the ONLY caller in the repo
      ← drag-drop                    wm.c:1358   ── guarded: if (z != SNAP_NONE)
```

Two facts close it:

1. **The drag path is structurally incapable of un-snapping.** `wm.c:1358` reads
   `if (z != SNAP_NONE) snap_to_rect(pgrab, z, …)`. The one branch that would release a
   snap is the one branch the guard excludes.
2. **The only path that *can* pass `SNAP_NONE` is `wm_snap_key`**, and
   `grep -rnw wm_snap_key` returns its definition (`wm.c:1308`), a prototype in
   `hosttest/toasttest.c:47`, and one call at `hosttest/toasttest.c:328`. Nothing in
   `kernel.zl` or any `kernel/*.c` calls it.

So: **a window snapped by dragging it to an edge can never be released by any input
the shipping tree accepts.** One-way door, confirmed — via the guard at 1358, not via
a missing caller.

---

## The second state machine, and why they never meet

`route_key` *does* handle Super+arrow, and it does reach a `SNAP_NONE`:

```
wm.c:1467:  if (code == KEY_DOWN)  { wm_snap(focus_win, SNAP_NONE);  return; }
```

But `wm_snap` is a **different mechanism** — the `wins[].maxed` / `sav_*` machine
declared around `wm.c:1115`. It does not touch `snap.c` at all. So the keyboard drives
one state machine and the drag drives the other, and neither knows the other exists.

### The trap that makes this hard to see

`SNAP_NONE` is `#define`d **twice in the same file**, for two different vocabularies
that happen to share the value 0:

```
wm.c:94    #define SNAP_NONE 0     /* snap.c's zones */
wm.c:95    #define SK_LEFT   1
wm.c:96    #define SK_RIGHT  2

wm.c:1115  #define SNAP_NONE   0   /* wm.c's own maxed/sav_ machine */
wm.c:1116  #define SNAP_MAX    1
wm.c:1117  #define SNAP_LEFT   2
```

`SK_LEFT` and `SNAP_MAX` are both `1`. `SK_RIGHT` and `SNAP_LEFT` are both `2`. The two
enums are numerically interleaved, so a value crossing from one machine to the other
would not produce a compile error, a warning, or an obviously wrong number — it would
produce a *plausible* wrong snap. Nothing currently crosses, but nothing prevents it
either.

---

## What the fix actually is

Not "add a caller for `snap_release`". The two candidate fixes are:

1. **Make the drag path able to un-snap** — remove or invert the `z != SNAP_NONE` guard
   at `wm.c:1358` so dragging a snapped window away from the edge releases it. This is
   the behaviour users expect and is the smaller change.
2. **Collapse the two state machines into one.** `wm_snap`/`wins[].maxed` and
   `snap.c`'s `zone[]`/`saved[]` are solving the same problem twice, and the duplicate
   `SNAP_NONE` is the symptom. This is the correct fix and the larger one.

Either way, **rename one of the two `SNAP_NONE`s first.** That is a mechanical change
that costs nothing and makes the rest of the work legible.

---

## The gate note, which is the part worth keeping

`hosttest/toasttest.c:328` is a green test over a function no shipping code calls, and
the test's own comment asserts it drives *"the same entry point `route_key` uses."*
`route_key` (`wm.c:1448`) does not call `wm_snap_key`. It calls `wm_snap`.

That is this repo's documented failure class — a gate that passes while checking
nothing — caught fresh, in a subsystem the existing `GUARDS-THAT-DID-NOT-GUARD.md`
does not cover. The comment should be corrected whether or not the snapping is fixed,
because right now it is evidence *for* a claim that is false.

---

## Method note

The agent's conclusion was right and its evidence was wrong. Both halves get recorded
here on purpose. A finding repeated without reproducing it would have shipped the
sentence "`snap_release` has no caller", which a five-second grep refutes — and the
credibility of the other 99 findings would have gone with it.
