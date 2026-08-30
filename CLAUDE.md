# zl-linux / zlOS — working notes for agents

## Read first, before changing anything

**`docs/PROJECT-STATUS.md` is the current front door.** It separates repository
integration, the complete implementation program, raw research, and actual
implementation evidence. Read it before picking up any task list.

`docs/STATE-OF-THE-PROJECT.md` is the detailed 2026-08-19 audit. It remains
valuable evidence but predates later integration. The archived
`docs/archive/superseded/ROAD-TO-TEN.md` is also historical, not the queue.

`docs/CODE-MAP.md` says where source, generated output, evidence, partial docs
and archived docs belong. Read it before searching for a subsystem or moving
files around.

`docs/evidence/MERGE-EVIDENCE.md` is the measured account of the eight parallel
tracks: the real shared base (`d61a481`, not `44346d6`), the landmines that
merge clean and then fail, and the landing order. Read it before merging
anything or starting a ninth track.

**`docs/evidence/MERGE-ROUND-2.md` is round 2** — the twelve worktrees standing on
2026-08-20, measured: 165 commits, 219 files, 129 uncommitted, **0 pushed**, and
`kernel/kernel.zl` contested by nine branches. It also records why opening a PR
per branch is the wrong shape here (local `main` is 45 ahead of `origin/main`,
so every PR would diff against a stale base), the landing order, and two places
`MERGE-EVIDENCE.md` is now stale in the pessimistic direction. Read it before
touching any `zl-linux-*` worktree.

`docs/archive/prompts/DOCS-RECONCILE-PROMPT.md` is the brief that produced
`STATE-OF-THE-PROJECT.md`. Done 2026-08-19; kept for method, not for work.

**`docs/GUARDS-THAT-DID-NOT-GUARD.md`** is five checks in this tree that
reported green while checking nothing, each with the command that establishes
it — including the `-w` claim this file used to make (below), a `check-memmap.sh`
discovery sweep that reads no C at all, and why a gate in this shared checkout
can fail for reasons that are not the code. Read it before trusting any green
result here, and before writing a new gate.

`kernel/docs/archive/prompts/BROWSER-STORAGE-PROMPT.md` was the brief for that work and it is
**DONE** - see `kernel/docs/evidence/browser-storage-run.md` for the run. The parser's
node array, its text arena, layout's runs and css's selectors were all static
and all full on a real page; they are the caller's now, in `memmap.h`'s new
`HI_DOM` region, and a real article parses whole (`8192/8192 with 7,807
dropped` -> `15,574/32,768 with 0 dropped`, `css_overflowed()` 1 -> 0).

Read the run doc before the brief, for three things the brief did not know:
the brief says there are **two** fixed-address maps and there were **five**
(`virtio_net.c` owns the 64 MiB the storage was about to be placed on, and
`intel.c` was writing its EDID inside `fb.c`'s blur arena); a full CSS string
arena refused rules **without setting `css_overflowed()`**, invisible until
`MAX_SELS` moved; and `memmap-guard-test.sh` was scoring 10/12, having gone
stale when the AP stacks were inserted. `hosttest/parsestat.c` is the
measuring instrument, committed this time.

`kernel/docs/evidence/browser-render-run.md` is the record of the run that produced the
current state - images, flexbox, grid, `@media`, search, the network at boot -
with every number and the command that measured it, including the four gates
that turned out to be testing something other than what they claimed.

`kernel/docs/archive/prompts/BROWSER-RENDER-PROMPT.md` is the brief that run worked from, and
both its items are now marked done. Worth reading anyway for two reasons: it
opens by correcting two things a fresh session is likely to be told to do that
are already done (Google works, and AES-256 is not needed), and its §1 records
a diagnosis that was precise, plausible and WRONG - the URL-bar bug it blames
on the keyboard was in the mouse path, and the symptom it reasoned from was a
coincidence of one URL. The shape of that mistake is kept deliberately.

`kernel/docs/POINTER-PROMPT.md` is the CURRENT WORK: the pointer is
visibly broken after the eleven-track merge, the lead suspect is measured
(two drainers of one xHCI event ring), and a full-tree bug audit follows it.
Read it before touching input, xhci or wm.
Two docs written for specific traps, both worth reading before touching their
area:

- `kernel/docs/input-stack.md` — the keyboard, the pointer, and the three ways a
  USB stack written for ONE HID device breaks when a second one arrives. Also
  the `-mgeneral-regs-only` boundary rule, which is subtler than it looks.
- `kernel/docs/thinkpad-first-boot.md` — the step-by-step for booting the
  laptop, including the two things that waste an hour (Secure Boot, and the fact
  that there is no serial port so the screen is the only diagnostic).
- `kernel/docs/memory-model.md` — **the address space and, for each boundary in
  it, the thing that actually enforces it.** The four ceilings people conflate
  (what the loader carries, where the image may end, who owns which address, how
  much RAM exists) are four different numbers with four different guards. Read
  it before placing any buffer, and before believing a `_Static_assert` covers
  what you think it covers.
- `kernel/docs/evidence/overnight-2026-08-18.md` — one page covering the descriptor-pointer
  bug that made the 64-bit boot layout-sensitive, why three green gates missed it,
  and what is still open.
- `docs/design/ci-and-agent-pipeline.md` — **design only, not built.** Why the
  gates belong on GitHub Actions rather than this box, the per-PR desktop
  screenshot `wmshot` already makes possible, and why every agent needs a zl
  brief before its review findings are worth reading.

**Never put a pointer through `unsigned long` in the EFI build.** `buildefi.sh`
targets `x86_64-unknown-windows`, which is LLP64: `unsigned long` is 4 bytes
there and 8 everywhere else. This bit twice, in two different ways:

- **As a struct field.** `struct idt_ptr` and `struct gdt_ptr` came out 6 bytes
  instead of 10, so `lidt`/`lgdt` took the top half of each base from adjacent
  memory. Both now use `unsigned long long` with a `_Static_assert` on the size.
- **As a cast**, which the struct fix did *not* cover. `(unsigned long)&idt`
  truncates to 32 bits before the widening assignment, so a correctly-sized
  field was still being handed a truncated value. Worse, `set_gate` did
  `unsigned long a = (unsigned long)handler; ... (u32)(a >> 32)` — shifting a
  32-bit value by 32 is undefined, and **clang compiled that expression to a
  bare `ret`**, so every IDT gate's high 32 bits came from whatever was left in
  `eax`. Five such sites existed; all now cast straight to `unsigned long long`.

Below 4 GiB the truncating casts are harmless, which is exactly why QEMU never
showed them. Real firmware is free to load the image, or place the ACPI RSDP,
above 4 GiB.

**`-w` is why nobody saw any of it.** The EFI build silenced every warning,
including the four that name this bug class precisely:

```
-Werror=shift-count-overflow  -Werror=void-pointer-to-int-cast
-Werror=pointer-to-int-cast   -Werror=int-to-pointer-cast
```

**This paragraph used to say the four worked because "clang applies flags left
to right, so these must stay *after* the `-w`". That was never run, and it is
false.** Measured 2026-08-19 on clang 21.1.8: `-w` suppresses all four
regardless of position — before them, after them, either way. The guard
reported nothing for its entire life.

Behind it were **33 casts of exactly this class**, including `smp.c`'s
`ENTRY_PTR` store — the documented bug verbatim, a 64-bit destination handed an
address already truncated to 32 bits, which is the entry point every
application processor jumps to. Two other files, `virtio_net.c` and `fb.c`, had
already written defensive code *citing this guard as the reason*, for a guard
that did nothing.

`-w` is now gone from `buildefi.sh` and the four are genuinely fatal. The one
warning `-w` was legitimately buying, `-Wexcessive-regsave` (11 hits in
`idt.c`, inherent to `__attribute__((interrupt))`), is suppressed by name so a
*new* class surfaces instead of being swallowed.

**`kernel/wguard.sh` is the check, and it runs all three directions** — the
guard catches a planted defect, `-w` is shown to still silence it, and the real
source set is clean under it. Two seconds, no QEMU. Run it before touching that
flag line.

Note the guard is **EFI-only**. `idt.c:465`'s `(u64)&idt` warns on the 32-bit
gcc build and is *correct* — that is the widening direction, and it is the
fixed form of the bug above. Do not "fix" it back.


`kernel/HANDOFF.md` is the orientation doc and it is kept honest — it records what
is *verified* rather than what is intended. Read it before touching `kernel/`.

**All eleven tracks landed on `main` on 2026-08-19.** `docs/evidence/MERGE-EVIDENCE.md`
§Outcome is the record. Two branches are still out: `fix/pointer-drain` (3
commits, the written fix for the broken pointer) and `ci/gates-and-agent-brief`
(11 commits, the only CI and the only copy of `zlfmt.c` anywhere).
`docs/archive/superseded/INTEGRATION-PLAN.md` is **superseded** and its figures are wrong — do not
plan a merge from it.

`kernel/docs/gen9-modeset-plan.txt` is the researched Intel display plan (JSON;
the text is in `.result.plan`). It resolves 13 conflicts between sources and lists
hazards that can damage hardware.

## The formatter re-indents; it does not reformat

`zlfmt` (built by `build.sh`, source `zlfmt.c`) rewrites leading whitespace and
strips trailing whitespace. **Every other byte is copied from the original
buffer.** Do not "improve" it into an AST pretty-printer without first reading
`docs/design/design_tooling.md` §3: `lexer.c:272-273` throws comments away and
`lexer.c:88` truncates token text at 128 bytes, so a formatter that rebuilds
source from tokens or the tree deletes every comment in the corpus and silently
corrupts long string literals. Both hazards are impossible by construction
today; a rewrite gives them back.

`./verify_fmt.sh` is the gate — a second, no QEMU, no host-load sensitivity. It
proves the token stream is byte-identical before and after, line numbers
included, which is what makes the reformat provably semantics-preserving.

VS Code integration is `editors/vscode-zl/`, installed with its `install.sh`.
It also runs files: ▷ in the title bar / `Ctrl+F5` for the interpreter,
`Ctrl+Shift+F5` for the LLVM backend — see that folder's README for why it
uses a terminal and which cwd it runs from.
**Copying that folder into `~/.vscode/extensions/` does nothing** — current VS
Code only loads what is listed in `extensions.json`, and a folder-drop never
gets an entry. It fails silently: the file just stays "Plain Text".

## Long commands: start them in the background, then keep working

Almost everything here boots QEMU under TCG emulation, and TCG speed depends on
host load. Blocking on these wastes minutes per call. Start them with
`run_in_background: true`, go do something else, and collect the result.

| Command | Roughly | Notes |
|---|---|---|
| `kernel/verify.sh` | ~1 min | BIOS boot vs a golden transcript |
| `kernel/verify-raw.sh` | 1–3 min | our own bootloader; polls for its marker |
| `kernel/verify-iso.sh` | ~1.5 min | BIOS **and** UEFI through GRUB, two full boots |
| `kernel/verify-efi.sh` | ~1 min | zlOS as its OWN UEFI application - the ThinkPad's path |
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

## Three gates were green while the 64-bit build was dead

`verify.sh` and `verify-raw.sh` boot the **32-bit** kernel. `verify-iso.sh`'s
"UEFI" case boots **GRUB's** `bootx64.efi`, which multiboot-loads that same
32-bit kernel - the EFI binary inside `zlOS.iso` has 451 GRUB strings in it and
zero zlOS ones. So nothing exercised `kernel/efi.c`, the 64-bit build, or the
path a real laptop takes.

That is not hypothetical: a change to the interrupt handlers killed the 64-bit
boot dead inside `setup_idt()`, and all three gates stayed green through it.
`verify-efi.sh` closes that hole - it boots `zlOS-usb.img`, our own
`BOOTX64.EFI`, under OVMF and checks it comes up AS a UEFI application with a
real framebuffer. **Run it before believing a change is safe on hardware.**

It is validated in both directions: green on a good build, and red with the
right diagnosis when the GOP lookup is forced to fail.

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

1. **MOST write paths are gated behind `lt_armed`, and four are not.** Measured
   2026-08-19, because the blanket version of this sentence was wrong and it is
   the sentence people rely on:

   | function | `lt_armed`? | reachable from |
   |---|---|---|
   | `intel_plane_setup` | yes | — |
   | `intel_gamma_ramp` | yes | — |
   | `intel_cursor_enable` | **no** | zl builtin `cur_on` |
   | `intel_cursor_move` | **no** | zl builtin `cur_move` |
   | `intel_cursor_disable` | **no** | zl builtin `cur_off` |
   | `intel_set_surface` | **no** | the modeset sequence, `intel.c:4075` |

   `mmio_w` itself is **not** gated either — it writes whenever `mmio` is set.
   So the three cursor functions write display registers with no gate at all,
   and a zl program can reach every one of them. Nothing in the kernel calls
   them, so gating them would break nothing; `intel_set_surface` is different,
   it has a real internal caller inside the modeset, and adding a gate there
   needs someone who knows whether that path runs armed.

   **Write paths are gated behind `lt_armed` and most have never executed.**
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
`git rm --cached`'d (`kernel/_gen64.c`, `kernel/hosttest/dpll_test`,
`kernel/_genefi.c`, and `kernel/hosttest/{browsershot,wmshot}.ppm`). Before
committing, check that a new binary or generated `.c` is not being added.

**A tracked file is never ignored.** `.gitignore` only applies to files git is
not already following, so adding a pattern does nothing for a path that is
already in the index — it has to be `git rm --cached`'d as well. That is why
`kernel/_gen*.c` sat in `.gitignore` while `_genefi.c` still produced a
~1400-line diff on every `buildefi.sh` run. To find the rest:

```
git ls-files -z | git check-ignore --no-index --stdin -z -v
```

As of the `browsershot.ppm` / `wmshot.ppm` removal that command returns only two
entries, both `.ultra/STATE.md` and `.ultra/TENSIONS.md` matching `.ultra/` in
`~/.gitignore_global`. Those come from the machine's global ignore file, not
from this repo, and are expected — **leave them alone.** No tracked file matches
a rule in any of the repo's own `.gitignore`s.

That claim was stale for a while before anyone re-ran the command, so re-run it
rather than trusting this paragraph. Two `.ppm` renders sat tracked against the
repo's own `kernel/.gitignore` line `hosttest/*.ppm`, rewritten on every
`./wmshot` and `./browsershot` run.

**The command only finds tracked files that some ignore rule already matches.**
A build output nobody ever wrote a pattern for is invisible to it. `kernel/.gitignore`'s
hosttest section is a hand-maintained allowlist, one line per binary, and it is
still incomplete: `hosttest/inputtest_feel`, `hosttest/inputtest_hid`,
`hosttest/wmbench` and `hosttest/wmtest_feel` are tracked, unlisted, and
rewritten by `./build.sh` on every run. `git ls-files kernel/hosttest/ | grep -v '\.c$'`
is the check that actually catches those.

All four build scripts that emit generated C write under `kernel/_gen*.c`
(`build.sh`/`mkdisk.sh` → `_gen.c`, `build64.sh` → `_gen64.c`, `buildefi.sh` →
`_genefi.c`) and each `cp`s over its output unconditionally, so no committed
copy is ever an input to a build.

Same for the two `.ppm`s: `hosttest/wmshot.c:204` and `hosttest/browsershot.c:76`
default their output path to `wmshot.ppm` / `browsershot.ppm` and truncate it.
Nothing reads them back — `probe-shot.py` and `probe-drag.py` are the only
`.ppm` readers in the tree and both open a QEMU `screendump` they just wrote
into a temp dir.

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
