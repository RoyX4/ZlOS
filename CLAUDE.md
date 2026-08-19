# zl-linux / zlOS — working notes for agents

## Read first, before changing anything

`docs/CODE-MAP.md` says where the code actually lives. The layout is not what
the directory names imply: the desktop and all eight apps are one zl file
(`kernel/kernel.zl`), the browser exists only on the `desktop/browser` branch,
and `kernel/out.c` is generated output that must never be edited or counted.
Read it before searching for a subsystem.

`docs/MERGE-EVIDENCE.md` is the measured account of the eight parallel
tracks: the real shared base (`d61a481`, not `44346d6`), the landmines that
merge clean and then fail, and the landing order. Read it before merging
anything or starting a ninth track.

`docs/DOCS-RECONCILE-PROMPT.md` is the brief for turning the 21 planning
documents (~392 KB, most written before the eleven-track merge) into one
honest `docs/STATE-OF-THE-PROJECT.md`. They are stale by construction; every
claim has to be checked against the merged tree.

`kernel/docs/BROWSER-NEXT-PROMPT.md` continues the browser, which landed
working - it fetches http://example.com/ by name and renders it. It records
the two things the merge cost it (italic, and continuous text sizes) and the
catalogue correction that was explicitly deferred until this branch merged.

`kernel/docs/BROWSER-RENDER-PROMPT.md` is the CURRENT BROWSER WORK: make pages
LOOK right. It opens by correcting two things a fresh session is likely to be
told to do that are already done - Google works, and AES-256 is not needed -
then orders the rendering work by visual win per line, and records the one open
URL-bar bug with what is known and what is not.

`kernel/docs/POINTER-PROMPT.md` is the CURRENT WORK: the pointer is
visibly broken after the eleven-track merge, the lead suspect is measured
(two drainers of one xHCI event ring), and a full-tree bug audit follows it.
Read it before touching input, xhci or wm.

`kernel/HANDOFF.md` is the orientation doc and it is kept honest — it records what
is *verified* rather than what is intended. Read it before touching `kernel/`.

**Eight tracks are in flight in parallel worktrees and none of them are merged.**
`docs/INTEGRATION-PLAN.md` measures the divergence and sets the landing order,
the per-landing gate, and which paths `main` owns. Read it before merging
anything, and before starting a ninth track.
`kernel/docs/gen9-modeset-plan.txt` is the researched Intel display plan (JSON;
the text is in `.result.plan`). It resolves 13 conflicts between sources and lists
hazards that can damage hardware.

## Long commands: start them in the background, then keep working

Almost everything here boots QEMU under TCG emulation, and TCG speed depends on
host load. Blocking on these wastes minutes per call. Start them with
`run_in_background: true`, go do something else, and collect the result.

| Command | Roughly | Notes |
|---|---|---|
| `kernel/verify.sh` | ~1 min | BIOS boot vs a golden transcript |
| `kernel/verify-raw.sh` | 1–3 min | our own bootloader; polls for its marker |
| `kernel/verify-iso.sh` | ~1.5 min | BIOS **and** UEFI, two full boots |
| `kernel/mkiso.sh` | under a minute | rebuilds the kernel first |
| `./build.sh` (repo root) | ~1 min | the zl toolchain; needed before `kernel/mkdisk.sh` |
| `kernel/build{,64,efi}.sh` | seconds | just compiles |
| any `qemu-system-*` boot | 15 s – 3 min | **entirely dependent on host load** |

Good pattern: kick off the three gates in the background together, then read code
or write the next change while they run, and collect all three at the end.

**Do not run several QEMU instances plus a fan-out of agents at once.** This box
has 4 cores and 15 GB. Measured consequences, both real:

- load average hit 6, a boot that normally takes 12 s took over 30 s, and the
  gate below reported a regression that did not exist
- the OOM killer took out the agent process itself at 7.9 GB resident
  (`Out of memory: Killed process 29833 (claude)`), taking the session with it

Run gates in the background and sequentially, and do not pair them with a
multi-agent fan-out. Check `cut -d' ' -f1-3 /proc/loadavg` before starting
anything heavy; if the 1-minute figure is already above ~4, wait.

## Gates must never be timing-sensitive

`verify-raw.sh` used to `timeout 30` and grep the output afterwards. Under load
the boot did not finish inside 30 s, so the gate failed on an unchanged kernel —
an A/B against a clean worktree showed baseline and modified passing and failing
*together*, tracking host load. A gate that fails for reasons unrelated to the
code is worse than no gate: it costs a bisect every time.

It now polls for its marker with a generous ceiling. **If you add or change a
gate, wait for the expected output, never for a fixed wall-clock time.**

## The Intel display driver is different from every other driver here

`kernel/intel.c` drives the real panel on the test laptop (ThinkPad X1 Carbon
Gen 8, CML-U 8086:9B41). Two rules:

1. **Write paths are gated behind `lt_armed` and most have never executed.**
   "The code exists" is not "the code works" — check for an actual caller, and
   check whether anything arms `lt_armed`, before believing a function runs.
2. **Some of it can damage hardware**, not merely fail: violating the panel's
   500 ms T12 power-cycle delay, or driving AUX into an unpowered panel. The
   hazard list is section 4.1 of the plan. Treat panel power as the one area
   where "try it and see" is not acceptable.

Develop it from Linux userspace against the live GPU via `kernel/hosttest/` —
seconds per iteration, no reboots:

```
cd kernel/hosttest
./gpu-dev.sh probe            # read everything, safe, i915 keeps running
sudo ./modeset_test --survey   # firmware-state survey, read-only
sudo ./modeset-run.sh --survey # same, with i915 detached; auto-recovers
```

Map **8 MiB** of BAR0, not 16 — the kernel refuses the full BAR while i915 holds
it, and every display register is under 1 MiB.

`modeset-run.sh` blanks the screen. Its recovery runs from an EXIT trap so it
fires on success, failure, crash and Ctrl-C alike — keep it that way.

## Local timestamps on this machine are unreliable

The hardware clock runs behind and NTP corrects it mid-session, so `git log`
dates and file mtimes can be days off. For "when did X actually happen", use a
server-side source (`gh api repos/RoyX4/zl-linux --jq .pushed_at`).

## Build outputs do not belong in git

`.gitignore` covers them, but several were already tracked and had to be
`git rm --cached`'d (`kernel/_gen64.c`, `kernel/hosttest/dpll_test`, and
`kernel/_genefi.c`). Before committing, check that a new binary or generated
`.c` is not being added.

**A tracked file is never ignored.** `.gitignore` only applies to files git is
not already following, so adding a pattern does nothing for a path that is
already in the index — it has to be `git rm --cached`'d as well. That is why
`kernel/_gen*.c` sat in `.gitignore` while `_genefi.c` still produced a
~1400-line diff on every `buildefi.sh` run. To find the rest:

```
git ls-files -z | git check-ignore --no-index --stdin -z -v
```

As of the `_genefi.c` removal that command returns nothing — no tracked file in
the repo matches an ignore rule.

All four build scripts that emit generated C write under `kernel/_gen*.c`
(`build.sh`/`mkdisk.sh` → `_gen.c`, `build64.sh` → `_gen64.c`, `buildefi.sh` →
`_genefi.c`) and each `cp`s over its output unconditionally, so no committed
copy is ever an input to a build.

`font_aa.c`, `font_sub.c`, `font8x16.c`, `icons.c` and `icons_rgb.c` are also
generated (by `gen_*.py` / `mkfont.py`) but are **deliberately tracked** — no
build script regenerates them, and keeping them in git is what lets the kernel
build without Python. The test is whether a *build script* rewrites the file,
not whether a generator once produced it.

## The zl compiler is deterministic — a generated-file diff is a real signal

Verified, not assumed: `../compile kernel.zl` run three times on unchanged
source gave byte-identical output, and regenerating from `kernel.zl` as of
`f6e0ec3` with today's compiler reproduced the `_genefi.c` committed at
`f6e0ec3` **byte for byte**. No timestamp, path or hash-ordering leaks into the
output.

So the ~1400 lines that made `_genefi.c` painful were pure staleness: it was
last regenerated at `f6e0ec3`, and `kernel.zl` then gained 35 lines at `b55f3f9`
which expanded to 737 added / 693 removed lines of boxed C.

Consequence worth keeping: if a regenerated file differs from its committed
copy, the source really changed. Don't wave such a diff off as compiler noise.
