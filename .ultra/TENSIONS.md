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

**The plan was wrong.** `docs/archive/superseded/INTEGRATION-PLAN.md` and the
synthesised runbook both asserted L1 would gate green on its own. It cannot.

**Fix:** cherry-picked `da34635` onto `main` (411 lines, now under
`kernel/src/drivers/input/xhci.c` and `kernel/src/arch/x86/idt.c`, clean). All
11 symbols verified defined.

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

## T-3 — `kernel/tools/checks/check-memmap.sh` cannot catch the collision it exists for. CLOSED.

It iterates a hardcoded list: `SNAKE_X SNAKE_Y FS_META FS_DATA FS_SLOT LINE_BUF
LINE_MAX HIST_BUF HIST_N`. No `DISK_SCRATCH`, no discovery of new constants.

The live collision it misses (both verified by reading the files):

```
claude/quirky-pare-05454c  kernel/src/kernel.zl:581   LINE_BUF     = 0x02030000
desktop/system-track       kernel/src/kernel.zl:1045  DISK_SCRATCH = 0x02030000
desktop/system-track       kernel/src/kernel.zl:643   LINE_BUF     = 0x02020000  <- never saw the move
```

~460 lines apart, different bases: a 3-way merge takes both with no conflict. zl
has no static asserts, so nothing fails.

**Close by:** replacing the list with a sweep of every `^[A-Z_]+ *= *0x` in
`kernel/src/kernel.zl`, paired to a `*_MAX`/`*_N`/`*_SIZE` size lookup, printing unsized
regions as UNSIZED rather than skipping them. Then prove it fails on a synthetic
tree with both constants at `0x02030000`.

**Closure:** the checker discovers every fixed-address zl constant, derives
cross-owner sizes from the clipboard and filesystem sources, and its
`--selftest` plants `CODEX_DUPLICATE = 0x02030000`. The current map passes and
the mutation is rejected before any build or boot.

---

## T-4 — Files that exist in no commit on any branch. CLOSED.

Found while gating: `kernel/tests/host/build.sh` is `set -e` and compiles
`gpu_fillrate.c`, which is tracked on **no branch** — it lives only in
`zl-linux`'s working tree. Every clean checkout dies at that step.

Copied into `zl-main` to unblock the gate; **not yet committed anywhere**.

Others in the same class, from the session sweep, all preserved in `refs/wip/*`
and the `~/zlos-freeze-*` tars but in no commit: `kernel/src/net/crypto.c` +
`kernel/tests/host/cryptotest.c`, `src/tools/zlfmt.c` + `verify_fmt.sh`,
`kernel/docs/archive/prompts/OVERNIGHT-PROMPT.md` (the brief that produced 19 of the
compositor's commits), `editors/vscode-zl/*`, `docs/LEARNING.md` + `learn/`,
~12 `kernel/docs/*.md`, 7 `probe-*.py`, and the `.ultra/` files in `zl-system`,
`zl-exec` and `zl-apps` (matched by `~/.gitignore_global`).

**Close by:** deciding each explicitly — land or defer in writing. Not by
neglect.

**Closure:** the structure publication tracked the listed host tests, crypto,
formatter, editor support, learning material, probes, prompts and `.ultra`
records. `git ls-files` now resolves the retained set in the clean publication
tree; the original dirty checkout remains preserved separately.

---

## T-5 — Two implementations of one thing, under different names. CLOSED.

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

**Close by:** the symbol table in `docs/evidence/MERGE-EVIDENCE.md` §3, run before each
landing, with a decision recorded per row.

**Closure:** the current source has one snake implementation (`sn_*`), one
resize-grip implementation, one `wm_frame_us`, one console-mute state, one snap
classifier and one pointer-bound owner. The retired spellings survive only in
comments that record the merge decision.

---

## T-6 — `fn ui()` silently reverts a fixed regression. CLOSED.

`kernel/src/kernel.zl`: `fn ui()` is `ui_scale()` on overnight and `cell_w() / 8` on apps.
Both builtins exist after a `runtime_kernel.c` union, so taking apps's side of
that hunk compiles, links, boots — and restores the "everything is tiny at 4K"
regression overnight diagnosed and fixed.

**Close by:** an explicit grep in the L3 gate — `grep -n 'fn ui()' kernel/src/kernel.zl`
must show `ui_scale()`.

**Closure:** `kernel/src/kernel.zl` contains exactly
`fn ui() { return ui_scale() }`, and `gates/land-gate.sh` now rejects any other
definition through its static UI-scale contract step.

---

## T-7 — The generated TODO hid every open tension. CLOSED.

`tools/todo.sh` expected headings shaped like `### T-N | date | open`, while
this file uses `## T-N — title. OPEN.`. The parser therefore emitted `_none
open._` while T-3 through T-6 visibly remained open in the source document.

**Fix:** accept both level-two/three OPEN heading forms, derive the TODO text
from the actual heading, and keep `--selftest-tensions` as a two-format
regression check.

---

## T-8 — Generated evidence manifests disagree on build identity. CLOSED.

The authoritative current build-input identity is
`9ff27c31667052639c426eb6675779e381c9798cabc1064efa49a93592c41d35`
over 148 declared inputs. Its digest now covers source/toolchain/route fields,
while Git head, branch and build-input dirty state remain conservative
generation context. This removes the impossible cycle in which writing and
committing the checked-in identity changed the identity itself.

Source snapshot, dependency, toolchain, license, wrapper, build-recipe, visual,
accessibility, decision, event-schema, security and observability registries
regenerate on that identity. Release notes, the JSON/HTML provenance viewer,
the joined evidence registry and the 906-row feature status then regenerate in
dependency order from the same current identity.

The dated artifact, application, init, reproducibility, benchmark, crash,
event-trace and adversarial receipts were not relabelled. They remain exact
evidence for their original `1f9e16ad...`, `85027b15...` or later historical
subject builds. Current registries carry those subject identities with
`current_build_bound: false`; current artifact, QEMU, host-test and benchmark
gaps remain explicit. Mutation tests reject changing those flags to true.

**Closure evidence:** every affected generator passes `--check --selftest`.
The current joined registry reports 0 current-build-bound artifacts, 9
historical artifacts, 0 current-build-bound QEMU routes and 6 historical QEMU
routes. Feature status demotes stale runtime claims to `HISTORICAL_ONLY` or
`PARTIAL_HISTORICAL`; it does not promote historical execution as current.

**Irreducible historical boundary:** old exact artifact/runtime receipts cannot
be made current without producing and testing new artifacts. Rewriting their
identity would destroy provenance, so they remain historical by design.

---

## T-9 — Boot recovery policy remains at the kernel root. CLOSED.

`boot_state.c` and `boot_state.h` now live together under their owning subsystem
at `kernel/src/core/boot/`. The policy source follows `boot_handover.c` in
`kernel/SOURCES`, so all four build routes compile it without claiming that a
loader calls it yet. `kernel/tests/host/boot_state_test.c` includes and compiles
the owned paths directly.

**Evidence:** the focused host harness passes all 91 checks with warnings fatal;
the 32-bit, 64-bit, EFI/LLP64, and raw-lane-equivalent compile checks all accept
the moved source through the shared manifest. The shared-source recovery
selftest and directory-capsule check also pass. The dated source snapshot and
T-8 build graph were regenerated through the identity-safe evidence chain.
Stage-zero persistent selection and ready-mark wiring remain future boot work,
not part of this structure closure.
