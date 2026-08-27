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

## T-8 — Generated evidence manifests disagree on build identity. OPEN.

The retained visual evidence moved from ignored `kernel/exercise-out/` into
`kernel/docs/evidence/exercises/2026-08-24/`. The visual and accessibility
registries now regenerate independently, but the downstream release-note,
provenance-viewer and joined-evidence generators cannot refresh honestly.

The checked-in inputs contain three incompatible build identities:

| Identity | Current owners |
|---|---|
| `2c873b665279da5a53c3a58bfa9cdd1c53a8a36f73702259a1bb991c1524d90e` | build identity, license, toolchain, visual and accessibility registries |
| `1f9e16ad4e48590f1f19c9fbdb64fee01171b48d7ebc294540652307a682fb04` | source/build graph, wrapper and artifact registries |
| `85027b159c9a594045c2f900e5971bb3408dd418dd61a373625425fba9030d13` | decision, benchmark, security, observability, release, provenance and joined-evidence registries |

`python3 kernel/tools/generators/gen-release-notes.py --check --selftest`
currently stops with `release-note manifests disagree on build identity`.
The provenance check independently stops with `provenance inputs disagree on
build identity`, and the joined-evidence check stops with `registry build
identities disagree`. After the Handoff authority correction, the decision
ledger check also stops with `decision ledger has stale build context`; it must
not be refreshed alone against the incompatible registry families.
Consequently, `kernel/metadata/provenance-viewer.json` and
`kernel/docs/provenance-viewer.html` remain dated artifacts: they still report
46 visuals while the current visual registry discovers 41.

**Close by:** choose or regenerate one authoritative build identity, bring every
input registry onto it from the same source/artifact snapshot, then regenerate
release notes, provenance viewer, evidence registry and feature status in
dependency order. All generator `--check --selftest` gates must pass before the
viewer is described as current.

---

## T-9 — Boot recovery policy remains at the kernel root. OPEN.

`kernel/boot_state.c` and `kernel/boot_state.h` are the only live implementation
files left at the zlOS product root. The pure policy is compiled by
`kernel/tests/host/boot_state_test.c`, but neither file appears in
`kernel/SOURCES`; the typed boot handover document correctly treats target
wiring as future work.

The ownership destination is `kernel/src/core/boot/`. This structure-only pass
does not move the pair because the host compile gate is deliberately deferred.

**Close by:** move both files together, update the host harness include/source
paths and every metadata/document reference, rerun `boot_state_test`, and prove
the shared build/source registries agree before removing this exception.

---

## T-20 — `NAMESET_MAX` silently dropped names; the kernel built clean and halted. CLOSED.

**Gate:** boot under QEMU after the PRESSWORK depth pass. **RED**, with a green build.

The depth pass took the kernel from 927 zl functions to 1083. `NAMESET_MAX` in
`src/backends/c/compile.c` was 1024 and `set_add()` had no failure path - past the
cap it simply stopped storing. The last 59 names were dropped in silence.

A dropped name is not a compile error. `set_has()` reports it unknown, so calls to it
are emitted as dynamic `zl_calln()` rather than direct `zl_fn_x()`, and the runtime
answers an unknown name with `kfatal("builtin not available in the kernel subset")`.
`kernel.elf` linked with **0 undefined symbols** at 5.59 MB and halted on boot.

`check-zlcalls.py` reported **"every call site resolves"** throughout, because it
verifies a name exists in the zl sources - not that the backend bound it.

**Cause, verified.** Compiling `origin/main`'s `kernel.zl` with the SAME compiler emits
2 direct `zl_fn_layout()` calls; this tree emitted 2 dynamic `zl_calln("layout")` for
the identical construct. Same source shape, different binding, therefore the tool.

Three earlier hypotheses died to measurement first: builtins conditionally compiled out
(preprocessed with the kernel's own flags - 759 both sides, zero difference); C calling
zl by name (zero `zl_calln(` outside generated files); a zero-argument binding quirk
(`label_windows` and `rail_reg_rows` are zero-arg and bind directly).

**The comment above the constant recorded this same bug at 256 and then said:**
*"1024 is headroom, not a guess - the kernel is nowhere near it either way."* It was at
1083. That sentence is why nobody looked at the compiler for three hypotheses.

**Closed by** raising the cap to 4096 AND removing the silence - `set_add()` now fails
hard, names the function it could not fit, and states the consequence. Validated in both
directions: cap planted at 64 gives `zl: too many names for the C backend: 'au_row_x10'
is number 65 and NAMESET_MAX is 64` with build exit 1; restored gives toolchain exit 0,
kernel exit 0, and a booting desktop.

**Still open from it:** `check-zlcalls.py` is structurally blind to this class. The
direct test is to scan generated `out.c` for a `zl_calln()` whose name is a known zl
function. Not written. Recorded in TODO.md.

---

## T-21 — the harness cannot render the shell, and screenshots from it were read as the OS. OPEN.

`kernel/tests/host/wmshot.c` links `wm.c`, `ui.c`, `uikit.c` and `fb.c` and draws its
own wallpaper, top bar and dock. The rail, raster strip and foot are `kernel.zl` and do
not compile into it, so **no wmshot render can show the shell** - and its furniture is
not the shell's.

This was read as the desktop being wrong, twice, before anyone booted the real thing.
It also renders geometry at `ui_theme_init(2)` while `fb.c` derives its type scale as
`width*256/1920` = 1.0 at 1920 wide, so harness shots understate the type by half.

Not closed: the honest fix is either to make the harness say so on the image itself, or
to make the desktop-shot CI job boot QEMU rather than run wmshot. Neither is done.

---

## T-22 — eyes were used as a colour instrument, and were wrong three times. CLOSED as a rule.

The focused window's knockout was called broken on two separate occasions, hours apart,
from downscaled renders. Sampling the framebuffer both times gave `#B6B0AB` exactly -
`ZD_KNOCK` - with its title ink at `#181411`, exactly `ZD_KNOCK_INK`.

At normal viewing scale that token against a dark surround reads as mid-grey. It is not
mid-grey.

**The rule:** eyes answer *"is the focus obvious at a glance"*; they do not answer *"is
this the right colour"*. Both questions get asked of the same picture, which is what
makes them easy to conflate. Sample the framebuffer for the second one - the instrument
is three lines of Python and it has been right every time against a confident wrong
reading.
