# TENSIONS — zl-main integration

Failed gates, wrong claims, and known hazards. Nothing is removed from here
because it became inconvenient; entries close with evidence or not at all.

---

## T-1 — The spine does not link on its own. CLOSED.

**Gate:** build gate on `main` after landing `d61a481`. **RED.**

11 undefined references at link: `xhci_ptr_abs`, `xhci_ptr_reports`,
`xhci_ptr_events`, `xhci_ptr_lastcc`, `xhci_kbd_events`, `xhci_kbd_requeues`,
`xhci_kbd_lastcc`, `xhci_ptr_slot`, `xhci_ptr_ep`, `idt_mouse_irqs`,
`idt_set_pointer_bounds`.

**Cause, verified.** `main@3facf35` had **0** references to any of them; after
the spine merge it had 2 each. The spine brings a `freestanding/runtime_kernel.c`
that *calls* these, while the definitions live in commits above it on each track.
`d61a481` is `git merge-base --octopus` over all eight tracks — a shared
ancestor, **never a released or linkable state**. Each track completed it
independently.

Corroboration that this is not new: `da34635` on `desktop/overnight-compositor`
is titled *"ship the xhci pointer and idt mouse code the tree already calls"*
and its message says the tree "did not build from a clean checkout".

**The plan was wrong.** `docs/INTEGRATION-PLAN.md` and the synthesised runbook
both asserted L1 would gate green on its own. It cannot.

**Fix:** cherry-picked `da34635` onto `main` (411 lines, `kernel/xhci.c` +
`kernel/idt.c`, clean). All 11 symbols verified defined.

**Rule this produces:** a landing whose payload is a *merge-base* is not a
landing. Gate the spine only together with the first track that completes it, or
carry the completing commit with it.

---

## T-2 — The gate script reported green on a tree that did not link. CLOSED.

The first `land-gate.sh` piped every build through `| tail`, so each `exit: $?`
was **tail's** status, which is always 0. A tree with 11 undefined references
printed `exit: 0` three times.

**Fix:** rewritten at `gates/land-gate.sh`. Each step's output is captured to a
variable and its real exit code tested. Also now guards on available memory
(≥3 GB) as well as loadavg (<4.0), and uses `pgrep '^qemu-system'` rather than
`pgrep -f qemu-system`, which matches the grep itself.

**Rule:** a gate that can only pass is worse than no gate. Any new gate must be
shown failing on a case it is supposed to fail on before it is trusted.

---

## T-3 — `kernel/check-memmap.sh` cannot catch the collision it exists for. OPEN.

It iterates a hardcoded list: `SNAKE_X SNAKE_Y FS_META FS_DATA FS_SLOT LINE_BUF
LINE_MAX HIST_BUF HIST_N`. No `DISK_SCRATCH`, no discovery of new constants.

The live collision it misses (both verified by reading the files):

```
claude/quirky-pare-05454c  kernel.zl:581   LINE_BUF     = 0x02030000
desktop/system-track       kernel.zl:1045  DISK_SCRATCH = 0x02030000
desktop/system-track       kernel.zl:643   LINE_BUF     = 0x02020000  <- never saw the move
```

~460 lines apart, different bases: a 3-way merge takes both with no conflict. zl
has no static asserts, so nothing fails.

**Close by:** replacing the list with a sweep of every `^[A-Z_]+ *= *0x` in
`kernel.zl`, paired to a `*_MAX`/`*_N`/`*_SIZE` size lookup, printing unsized
regions as UNSIZED rather than skipping them. Then prove it fails on a synthetic
tree with both constants at `0x02030000`.

---

## T-4 — Files that exist in no commit on any branch. OPEN.

Found while gating: `kernel/hosttest/build.sh` is `set -e` and compiles
`gpu_fillrate.c`, which is tracked on **no branch** — it lives only in
`zl-linux`'s working tree. Every clean checkout dies at that step.

Copied into `zl-main` to unblock the gate; **not yet committed anywhere**.

Others in the same class, from the session sweep, all preserved in `refs/wip/*`
and the `~/zlos-freeze-*` tars but in no commit: `kernel/crypto.c` +
`hosttest/cryptotest.c`, `zlfmt.c` + `verify_fmt.sh`,
`kernel/docs/OVERNIGHT-PROMPT.md` (the brief that produced 19 of the
compositor's commits), `editors/vscode-zl/*`, `docs/LEARNING.md` + `learn/`,
~12 `kernel/docs/*.md`, 7 `probe-*.py`, and the `.ultra/` files in `zl-system`,
`zl-exec` and `zl-apps` (matched by `~/.gitignore_global`).

**Close by:** deciding each explicitly — land or defer in writing. Not by
neglect.

---

## T-5 — Two implementations of one thing, under different names. OPEN.

Git cannot see this class at all; it unions both and nothing warns.

| Thing | One side | Other side |
|---|---|---|
| windowed snake | overnight `sn_draw/sn_event/sn_rand/sn_reset/sn_step` | apps `snake_draw/snake_key/snake_rand/snake_start/snake_step` |
| resize grip | overnight `in_resize_grip` | feel `in_grip` |
| frame timer | overnight `int wm_frame_us()` (16-frame avg, `wm.c:1096`) | apps `unsigned int wm_frame_us()` (instant, `wm.c:821`) |
| console mute | overnight `console_mute`/`con_mute` | apps/system/exec `console_quiet`/`con_quiet` |
| window snapping | feel `wm_snap`, rect held in `wm.c` | system `snap_zone_for_point` in `snap.c` |
| pointer bounds | overnight `ptr_lim_x/y = w,h` | apps/system/exec `mouse_max_x/y = w-1,h-1` |

`wm_frame_us` is the lucky one — same name, so it fails at link. The rest fail
silently or ship twice.

**Close by:** the symbol table in `docs/MERGE-EVIDENCE.md` §3, run before each
landing, with a decision recorded per row.

---

## T-6 — `fn ui()` silently reverts a fixed regression. OPEN.

`kernel.zl`: `fn ui()` is `ui_scale()` on overnight and `cell_w() / 8` on apps.
Both builtins exist after a `runtime_kernel.c` union, so taking apps's side of
that hunk compiles, links, boots — and restores the "everything is tiny at 4K"
regression overnight diagnosed and fixed.

**Close by:** an explicit grep in the L3 gate — `grep -n 'fn ui()' kernel/kernel.zl`
must show `ui_scale()`.
