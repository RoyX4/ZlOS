# Every Codex thread — what it asked, what survived, what is still owed

Audit date 2026-08-19. Source: 26 rollouts under `~/.codex/sessions/2026/08/`,
of which 8 are named in `~/.codex/session_index.jsonl` and 10 contain real work.
The Codex account is capped until **2026-08-25 21:58**, so none of these can be
resumed in Codex; this file is what replaces them.

## The one-line summary

Three implementation threads built real work and **lost all of it to `/tmp`**.
Three review threads produced findings that were **already fixed in this repo
before the reviews ran**. One audit thread **never executed a single command**.

## Review threads — 8 of 8 findings already fixed, nothing owed

Verified against source on 2026-08-19, one at a time. Every finding was already
handled, and in every case the fix carries a comment explaining the same hazard
the reviewer described.

| Finding | Where it lives now | Status |
|---|---|---|
| P1 truncated 64-bit GPU BAR | `kernel/intel.c:401-425` — reads `pci_bar_hi()`, refuses on 32-bit, and combines on 64-bit via `((uptr)hi << 16) << 16` | fixed |
| P1 `HI_IMG` overlaps live buffers | `kernel/memmap.h:116` — `HI_IMG` is `0x03000000`, not `0x02000000`; `_Static_assert(HI_IMG >= ZL_LOW_END)` at :159 | fixed |
| P1 base64 accumulator overflow | `kernel/browser.c:445,457` — `unsigned acc`, masked every byte | fixed |
| P2 `PIPE_FRMCNT` signedness | `freestanding/runtime_kernel.c:1144` — `(double)(unsigned)intel_frame_count()` | fixed |
| P2 PSR blamed for a zero counter | `kernel/kernel.zl:3881` — guarded by `if intel_pipe() == 1` | fixed |
| P2 shell column clamp | `kernel/term.c:453` — uses `view_cols`, no 199 clamp exists | not applicable |
| P2 abandoned image fetch | `kernel/browser.c:797` — `if (fetching) { http_reset(); fetching = 0; }` | fixed |
| P2 port integer overflow | `kernel/browser.c:739` and `:895` — both saturate at 65536 before multiplying | fixed |

**Do not re-apply any of these.** The reviewer was describing a state this tree
had already left. The `zl-browser2` fixes predate their review by ~16 hours
(`memmap.h` last changed `fd92d1c`, 08-18 22:57; review ran 08-19 14:27).

Two caveats on the above, so this table is not itself a false claim:

- I verified each fix by reading the current source, not by running the kernel.
- `intel.c:678` computes `u32 frames = (u32)(f1 - f0)` from two `int`s. If the
  counter wraps the sign boundary between the two reads that subtraction is
  signed overflow — genuine UB, once per 2^31 frames (~414 days at 60 Hz).
  Real, and not worth a fix ahead of anything else here.

## Implementation threads — work lost, specs recovered

All three died the same way, and it is one root cause, not three accidents.
Codex runs `sandbox_mode = "workspace-write"`, which leaves `.git` read-only, so
`git worktree add` failed. It fell back to a clone under `/tmp` — writable
because `exclude_tmpdir_env_var = false` — and `/tmp` was then cleaned
mid-session. Nothing survived: no branch, no stash, and all 13
`refs/codex/turn-diffs/*` captures are clean 346-file bases.

**`.git` here is not actually read-only.** It is `drwxrwxr-x roy:roy` and
`git worktree add` succeeds when run as `roy`. The restriction was the agent's
own sandbox. Never let an agent route around that into `/tmp`.

| Thread | What it was building | Recovered spec |
|---|---|---|
| `01a0187a` | Files app + zlfs-backed editor save | [files-app-handoff.md](../handoffs/files-app-handoff.md) — full spec + cold-boot gate |
| `01a01877` | App suite: maze, Word Guess, Clock | below |
| `01a017cf` | zlOS v10 look-and-speed pass (icons) | below |

### `01a01877` — app suite, died mid-fix with four named bugs

It had compiled once and self-reviewed. The last message names exactly what it
was fixing when it died. Any rebuild should start with these already known:

1. one missing framebuffer declaration in the new modules
2. the maze needs **273 cells** but the shared board reserves **256**
3. Word Guess starts on row 2 (should not)
4. a single shared second-timer starves the Clock when another live system app
   ticks first

### `01a017cf` — look and speed

Driven from `/home/roy/zl OS v10.dc.html`, 17 user turns, the largest thread
(11 MB). The ask was explicitly about *looks and speed*, not features. It died
mid "icon pass" — same `/tmp` deletion. The reference HTML still exists; the
icon work does not.

## The audit thread that never ran — still fully owed

`01a01738` is the only thread that changed nothing at all. Its own last message:

> The sandbox is still broken; "continue" did not remount it. Every command
> fails before execution. […] No repo files have been changed yet.

Cause it diagnosed: the task mounted `zl-linux` **plus its linked worktrees**
`zl-browser-next` and `zl-main`, then tried nested mounts beneath the read-only
`zl-linux/.git`. Its own prescription was to open a task with **only**
`/home/roy/Documents/repos/zl-linux` as the workspace root.

What was asked, verbatim in intent: *every* file — errors, gaps, false claims —
and the docs compared against the code to find where the two disagree.

Scale, measured: **726 tracked files** — 161 `.zl` (25,770 lines), 129
hand-written `.c`/`.h` (81,319 lines, excluding generated `_gen*`/`out.c`),
6 `.S`, 119 `.md` (44,761 lines), 80 scripts.

First shippable increment: **the docs-vs-code false-claim pass.** It is the part
Zac named, and it is the failure mode this repo has already had — `0b28bb1`
is a commit literally titled *"the suite stopped BUILDING, and I claimed it
passed anyway"*.

### Increment 1 result — run 2026-08-19

**The repo already had the tool.** `tools/doc-check.sh` is exactly this audit,
already written, already wired into `.github/workflows/docs.yml:41` with no
`continue-on-error`. It did not need building — it needed reading.

**It is failing, and it has been failing on `main` in CI.** Run `32210170799`,
job `docs`, `main`, 2026-08-19T02:53:32Z → **failure**. Reproduced locally:
exit 1, 30 STALE lines. `gates` and `boot` on the same commit were green, which
is why nothing looked wrong.

Sorting the 30 by what they actually are:

| Class | Count | Verdict |
|---|---|---|
| Genuine broken cross-directory link | 3 | **fixed** — `e78de74` on `docs/fix-stale-links` |
| Doc quoting a wrong path while explaining that wrong path | 2 | checker false positive, left alone |
| Generated + gitignored (`_gen64.c`, `_genefi.c`, `out.c`) | 8 | environment artifact — they report differently on a built vs unbuilt tree |
| Plan docs naming files not yet written | ~17 | left failing on purpose; `doc-check-ignore.txt` says *"never add one to make a run green"* |

The three real ones were all the same mistake — a reference under `docs/` to a
file that lives under `kernel/docs/`: `what-is-a-bios.md`,
`typing-into-the-compositor.md`, `gen9-modeset-plan.txt`.

### A false claim the doc-checker could never have caught

`kernel/docs/system-track.md:359` states, as fact:

> the **editor** copies its whole buffer on Ctrl+C and appends the clipboard

It does not, and never did. `editor_key(code)` on `main` handles exactly four
cases — `27` (ESC/save), `8` (backspace), `13` (enter), and `code >= 32`
(printable). Codes **3** (Ctrl+C), **22** (Ctrl+V) and **19** (Ctrl+S) fall
through to `return 0`. Verified by extracting the whole function body from
`git show main:kernel/kernel.zl`, lines 1426-1450.

The same doc's §"The clipboard, and why no routing changed for it" is correct
about the *plumbing* — `input.c` really does produce chars 3 and 22, and the
compositor really did need no change. The claim that fails is the last mile: the
editor was never taught to consume them.

Eight lines below `editor_key`, the source says:

> a command that silently returns is this repo's most expensive recurring bug,
> and "the app list offers it but nothing happens" is exactly that shape

which is precisely what `editor_key` was doing for Ctrl+C, Ctrl+V and Ctrl+S.

**This is the finding that matters most for how to audit this repo.** It was not
found by reading docs, and `tools/doc-check.sh` cannot find it — the file it
references exists, so every path check passes. It surfaced only because someone
sat down to *implement* the feature and discovered it was already promised.
Now closed on `desktop/files-app`.

It also qualifies the audit's headline: this repo's docs are careful about file
references and about status, but "the feature exists" claims are a class nothing
currently checks.

### The biggest gap: 41 commits have never been through CI

`git rev-list --left-right --count origin/main...main` → **`0  41`**.

Nothing has been pushed since `55e0226` (2026-08-19T02:53:39Z, confirmed by the
repo's own `pushedAt`). Everything after it is local only:

- the entire GPU ring track, including `ccb1415` *"PROVEN ON SILICON - zlOS can
  drive the Gen9 ring itself"* and `90e182b` *"THE RING RUNS ON REAL SILICON"*
- `7491c11`, which **adds `check-himap.sh`** — a gate that has therefore never
  run in CI, because the commit introducing it was never pushed
- `0b28bb1` *"the suite stopped BUILDING, and I claimed it passed anyway"*

This changes what the red `docs` job means. It is a true failure, but it is a
failure **about `55e0226`**, not about current `main`. Locally reproduced on the
current tree, doc-check still fails — so the problem is live either way — but no
CI result exists for any of the 41 commits.

A green CI badge on this repo currently describes a tree from 02:53 this morning.

**Not pushed by this session.** Pushing 41 commits is a shared-state change and
is Zac's call, not an agent's.

### 140 of 552 builtins are unreachable from any `.zl` program

`kernel/check-zl-calls.sh` ends with `note: 152 registered builtin(s) with no
caller in kernel.zl` — and then never says which. A count nobody can act on.
Extracted them, and widened the search from `kernel.zl` to **every tracked
`.zl` file**:

- registered builtins: **552**
- no caller in `kernel.zl`: 148
- **no caller in any `.zl` file: 140** — 25% of the runtime's builtin surface

They cluster by subsystem, and the clusters are the interesting part — whole
capabilities with no consumer:

| Cluster | Examples |
|---|---|
| USB | `usb_kbd usb_poll usb_run usb_reset usb_rings usb_scratch usb_barhi usb_pen` |
| scheduler | `sched_on sched_cur sched_n sched_st sched_tk` |
| SMP | `smp_n smp_last` |
| NVMe | `nv_ok nv_ready nv_rd_to nv_wr_from nv_ram nv_sb` |
| TCP | `tcp_open tcp_shut tcp_cwnd tcp_dupack tcp_badsum tcp_kill` |
| APIC | `apic_on apic_id apic_gsi apic_madt apic_rsdp apic_red apic_pins` |
| GPU | `gpu_flip gpu_flips gpu_mode gpu_reg gpu_vbl ggtt_map` |
| clipboard | `clip_add clip_new clip_done clip_seq clip_wipe` |
| render | `blur blurdraw blurfree blend subpix subpix_on` |

**Stated precisely, because the distinction matters:** these are dead *bindings*,
not proven-dead subsystems. Spot-checked `usb_kbd`, `sched_on`, `smp_n`,
`nv_ready`, `tcp_open`, `clip_add` — each appears in C only at its own
`streq(name, …)` registration line, so nothing calls the builtin. But the C
behind them is real and reachable from C: `xhci_kbd_init` lives in
`kernel/xhci.c`, `sched_start` in `kernel/sched.c`. What is dead is the path
from a zl program to that capability.

This is the same class as `docs/GUARDS-THAT-DID-NOT-GUARD.md` §3 — *"The SMP
wiring is complete. The ignition is missing."* — but 140 wide instead of one.

Two things worth doing, neither done here (three builders are mid-flight in
this repo and this is not their branch):
1. Make `check-zl-calls.sh` **print the names**, not just the count.
2. Triage the 140 into *deliberate API surface* vs *forgotten ignition*. The
   USB, scheduler and NVMe clusters are the ones to look at first — those
   subsystems are documented as working.

### Two more gaps found alongside it

- **CI runs 3 of the 9 gate scripts.** `verify-efi.sh`, `verify-iso.sh` and
  `verify-raw.sh` appear in `.github/workflows/`. `check-himap.sh`,
  `check-memmap.sh`, `check-zl-calls.sh`, `verify-sources.sh`,
  `verify-clock.sh` and `verify-disk.sh` are reachable **only** through
  `gates/land-gate.sh`, which nothing in CI invokes. Six checks run only if a
  human remembers to run them locally.
- **The `nightly` workflow has been `in_progress` for 4h11m** (run
  `32217177535`, started 04:49:57Z). Long enough to be worth confirming it is
  running rather than wedged.

### Local gate run, same day

`gates/land-gate.sh` against `00fc205` in an isolated worktree: zl toolchain,
kernel 32-bit (0 undefined symbols, multiboot header found), kernel 64-bit
(0 undefined, ELF64), kernel EFI — **all ok**. The QEMU boot gates then
correctly refused to start: the gate's own guard requires loadavg < 4.0 and the
box was at 9.52 with six concurrent agent sessions. That is the guard working,
not a failure.

### One caution about the checker itself

`tools/doc-check.sh` returns a correct exit code. If you measure it as
`bash tools/doc-check.sh | tail` and read `$?`, you get `tail`'s status and it
looks green. That is failure mode §4 in `docs/GUARDS-THAT-DID-NOT-GUARD.md`,
and it caught me during this very audit. Redirect to a file, then read `$?`.

## The rebuild — dispatched 2026-08-19

Three isolated worktrees, one builder each, all off `main`. Every one of them is
a real `git worktree`, which is the thing Codex could not create and the reason
its work died.

| Branch | Worktree | Base | Scope |
|---|---|---|---|
| `desktop/files-app` | `../zl-linux-files-app` | `00fc205` | Files window + zlfs-backed editor save |
| `desktop/app-suite` | `../zl-linux-apps` | `a7259db` | the 53-app reference suite |
| `desktop/v10-look` | `../zl-linux-v10look` | `a7259db` | v10 look-and-speed pass |
| `docs/fix-stale-links` | `../zl-linux-docfix` | `1f42adf` | 3 broken doc links (done, `e78de74`) |

### Integration risk, checked up front

`main` moved twice during this session — `00fc205` → `1f42adf` → `a7259db` —
because other sessions are committing to the same repo. The `desktop/files-app`
branch is therefore based on an older tip than the other two.

Measured: `00fc205..a7259db` touches only `kernel/gpuring.c`,
`kernel/hosttest/{build.sh,gpu_aperture.c,gputest.c}`, `kernel/.gitignore` and
two GPU docs — 7 files, all GPU-ring work. None of it overlaps the filesystem,
editor, compositor or memory-map surfaces the three rebuilds touch, except
`kernel/hosttest/build.sh`. So rebasing `desktop/files-app` forward should be
clean or near-clean; expect the only contention at `hosttest/build.sh`.

### Results — independently verified

Each branch was re-verified by a script that never pipes a build (the
land-gate's own header records that piping through `tail` made a non-linking
tree gate green). All three kernel builds, all three static gates, the hosttest
harness, and `nm -u` on both ELFs.

**`desktop/v10-look` → GREEN** (`afc04d0`, 5 commits)
Three real defects, each confirmed by reading before fixing:
- `fb.c`'s `icons24`/`icons48` externs declared `[10]`; `icons.c` defines `[20]`.
  Linked fine because the element type matched — silently unreachable icons.
- `wm.c`'s `chrome()` drew the resize grip **twice**, the first at the wrong
  size and colour. Before/after `wmshot` crops committed as evidence.
- `wm.c`'s `wm_repaint()` composited a fading window's saved backdrop at the
  **wrong origin** — `cx/cy/cw/ch` were captured for the stash then clobbered by
  a later `isect()` before the blend reused them. Measured: 19 of 20 sampled
  rows changed, largest single-channel delta 40/255.

Most of the v10 look backlog turned out to be **already closed** by an earlier
session — including the dock digit-debris bug, which re-derivation showed was
already fixed at `kernel.zl:3247-3265`. The same pattern as everything else here.

**`desktop/files-app` → GREEN** (`2c4fe1e`, 2 commits)
The full spec shipped: a Files window (`APP_FILES`, reachable as `filemgr` /
`explorer`), auto-mount on open, create/open/delete by name, and the editor's
disk-backed mode with real Ctrl+S, Ctrl+C and Ctrl+V. The ten numbered RAM slots
are untouched, as the spec required.

**The cold-boot gate actually ran**, which is the part that matters: boot 1
created `probe.txt` and saved it, the QEMU process was **killed**, and an
independent `qemu-system-i386` against the same NVMe image reopened the file
with matching content (ink 1016 in both boots). Committed as
`kernel/probe-files.py`, so it is now a permanent gate rather than a one-off.
`hosttest/fstest`: 117 assertions, 0 failures.

One deviation, correctly flagged rather than hidden: the brief said to declare
the new buffer in `memmap.h`, but that file's own header scopes it to the
≥128 MiB high-RAM map. `FILES_NAME_BUF` is a 24-byte low-memory buffer, so it
was registered in `check-memmap.sh`'s sized overlap check instead — the file
that actually governs that range.

**Both branches boot.** `kernel/verify-raw.sh` run independently on each once
box load fell below the gate's own 4.0 threshold:

```
zl-linux-v10look    exit=0  ok  kernel boots via our own bootloader (no GRUB), shell responds
zl-linux-files-app  exit=0  ok  kernel boots via our own bootloader (no GRUB), shell responds
```

That closes the one gap the v10-look builder had flagged as unverified — it
could not boot-test while six concurrent sessions held the box above load 6.

### Verification stance

An agent reporting "build green" is a claim, not evidence. Every branch gets its
builds and static checks re-run independently before it is called done.

## Threads that are genuinely done

`01a014bd` GitHub connector (answered — setup instructions given, connector
itself may still not be authorized), `01a017b9` Codex-vs-Claude comparison,
`01a017c5` tool inventory. `01a0192d` is a capped one-word probe of mine.
The `/tmp/claude-1000/**` rollouts are MCP bridge tests, not project work.
