# The fleet run — 2026-08-19

A ~100-agent audit of zlOS across four fronts, run from one session into an isolated
worktree. This directory is its output.

**Branch:** `fleet/audit-2026-08-19` · **worktree:** `/home/roy/Documents/repos/zl-linux-fleet`
· **base:** `main` at `3f00366`

Nothing here was committed to `main`, and no agent in the run had write access to any
file. Five other worktrees were live in sibling directories during the run
(`zl-browser2` had a QEMU running); none were touched.

---

## Read these first — hand-verified, highest consequence

Every claim in the four files below was re-derived directly from the tree by hand,
not taken from an agent's report. Where an agent's evidence was wrong but its
conclusion right, both are recorded.

| file | what it settles |
|---|---|
| [`CRITICAL-gpuring-bar-truncation.md`](CRITICAL-gpuring-bar-truncation.md) | `intel_mmio()` returns `u32`, so **every GPU ring register access truncates a 64-bit BAR**. `intel.c` builds the >4 GiB address correctly and the accessor throws the top half away. The harness stubs it to `0`, which is why 116 checks stayed green. **Blocks arming the ring.** |
| [`PANEL-POWER-IS-REACHABLE.md`](PANEL-POWER-IS-REACHABLE.md) | `lt_armed` **is** armed by the shipping tree — `kernel.zl:1489` `P` → `panel_up` → `intel.c:4323`. Five doc sites in three files say nothing outside `hosttest/` arms it. The T12 and AUX discipline on that path is sound; the documented safety boundary is not. |
| [`CRITICAL-ci-truncation-gate-is-blind.md`](CRITICAL-ci-truncation-gate-is-blind.md) | `tools/hazard-scan.sh` scrapes its file list out of `buildefi.sh` **as text**, so `$CORE` never expands. It scans **6 of ~50** translation units — and 2 of the 6 by accident — while CI prints *"no new truncation sites."* The gate for this repo's #1 bug class never opens `fb.c`, `intel.c`, `xhci.c`, `http.c` or 37 others. |
| [`GROUND-TRUTH-CORRECTIONS.md`](GROUND-TRUTH-CORRECTIONS.md) | `kernel/docs/driver-build-order.md` **Wave 0 is already landed** — `acpi_find_table` linkage, `write_msr`, and the `pci.c` capability walk all exist. The blur/HID collision is fixed by `kernel/memmap.h`. Also: `intel.c` orphans measured at **101 of 303**, not 63, and **not one function in the file is `static`**. |
| [`VERIFIED-ggtt-span-collision.md`](VERIFIED-ggtt-span-collision.md) | The hardware cursor and the GPU self-test surface **overlap by 3 of 4 pages in GGTT space**. Both `_Static_assert`s pass because each checks only its own object against `HI_BLUR`. Found independently by two agents. |
| [`CRITICAL-browser-urlbar-keys.md`](CRITICAL-browser-urlbar-keys.md) | The URL bar **inserts every character twice**, and Enter/Esc/Backspace are **dead on the ThinkPad's own keyboard**. `kernel.zl:2993` handles `APP_BROWSER` above the `nav_to_char` translation and returns before reaching it — `nav_to_char` has exactly one call site and the browser is not it. One line. Invisible to every gate because every probe boots `-device usb-kbd`. |
| [`VERIFIED-WM-SNAP.md`](VERIFIED-WM-SNAP.md) | Drag-to-edge snapping is a **one-way door** — but not for the reason reported. The agent's evidence (`snap_release` has no caller) is false; the real cause is the `z != SNAP_NONE` guard at `wm.c:1358`. Also: `SNAP_NONE` is `#define`d twice in one file for two different enums whose values interleave. |

---

## The four waves

| wave | agents | front | board |
|---|---|---|---|
| 1 | 25 | the OS look — compositor, framebuffer, fonts, icons, theme, motion | `LOOK-BOARD.md` |
| 2 | 25 | Intel Gen9 GPU accel, every other driver, new driver design | `DRIVER-BOARD.md` |
| 3 | 10 + verifiers | bugs, swept by **class** rather than by file, each finding adversarially refuted | `BUG-BOARD.md` |
| 4 | 25 | the browser — parser, layout, CSS, net stack, robustness, bounded scope | `BROWSER-BOARD.md` |

Each wave ran as one workflow with a hard concurrency cap of **6** — `min(16, cores−2)`
on this 8-logical-core box. A hundred agents is a queue, not a hundred lanes.

---

## How the run was constrained, and why

`CLAUDE.md` records this box OOM-killing an agent process at 7.9 GB when a fan-out was
paired with QEMU. So:

- **No agent ran QEMU, any `verify*.sh`, any `build*.sh`, or `land-gate.sh`.** The fleet
  read code; it did not execute it. Memory was monitored throughout and peaked with
  ~5.5 GB still free.
- **No agent had write access.** All findings are reports; every file in this directory
  was written by the orchestrating session.
- **Writes were isolated to a worktree** because five sibling worktrees were live and
  other sessions edit the same checkout.

## The instruction that earned its tokens

Every agent was briefed with the repo's own documented claims **stated as fact**, and
told explicitly to verify rather than accept them. That is why the run produced
`PANEL-POWER-IS-REACHABLE.md` and `GROUND-TRUTH-CORRECTIONS.md` at all — in both cases
an agent read the code and contradicted its own brief.

The repo's vocabulary was given to every agent because confusing these three is its
dominant failure mode:

> **exists** = the function is written and compiles · **reachable** = something in the
> shipping tree calls it · **covered** = a gate that actually runs executes the call

## Standing caveat

Findings in the wave boards are **agent output**. The five files at the top of this page
are hand-verified; the boards are not, and at least one agent finding has already been
shown to have a correct conclusion resting on false evidence. Reproduce before acting —
which is this repo's existing rule for cross-model review, applied to its own fleet.
