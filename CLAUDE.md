# zl-linux / zlOS — working notes for agents

## Read first, before changing anything

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
`git rm --cached`'d (`kernel/_gen64.c`, `kernel/hosttest/dpll_test`). Before
committing, check that a new binary or generated `.c` is not being added.
