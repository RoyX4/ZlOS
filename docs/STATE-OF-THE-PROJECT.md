# State of the project — what is genuinely still open

**2026-08-19 · `06ced13` (audit baseline) · `ff27d57` (tree at writing) ·
`faac0f4` (tree at repair; both commits ahead are docs-only)**

> **Evening re-check, 2026-08-19 (HEAD `3f00366`, plus a dirty tree).** §1's
> ranking is stale against this checkout. Closed: land-gate now runs the host
> harnesses *and* `check-zl-calls.sh` / `check-memmap.sh` (§2.1–2.2);
> `fix/pointer-drain` is in the tree as a single `xhci_poll()` (§3.1);
> `mkdisk.sh`'s kernel-size guard is back (§2.4); CI is tracked (8 files under
> `.github/`); the self-hosting fixpoint is in `run_tests.sh` and
> `gates.yml`; `DISK_SCRATCH` is at `0x02040000` (the checker still does not
> name it). Still first: `key()` at `kernel.zl:1517` (§5.1), boot the ThinkPad,
> land `desktop/browser-next` (21 commits), Ring 3. Ranked plan for score, not
> bugs: [`docs/ROAD-TO-TEN.md`](ROAD-TO-TEN.md).

> **Repair pass, 2026-08-19.** Four adversarial reviewers attacked the first
> draft and raised 44 defects. Every one was re-verified against the tree by a
> fifth pass before being applied; 39 were confirmed, 5 were rejected or settled
> against the reviewer, and 2 reviewers who disagreed with each other were
> settled by re-running the count. Corrections are marked inline where the first
> draft was wrong, rather than silently overwritten — a document whose thesis is
> "reading one tells you what somebody wanted" has no business hiding its own
> revisions. The single worst error was §6.2, which declared the arena
> caller-less while `kernel.zl:3850` calls it at boot; three of the four
> reviewers found it independently. Six entries are **new** (§2.10, §4.14, §6.4,
> §7.3, §10.3, and the §5.8 link-training pair) — dropped by the first synthesis,
> not by the audit that fed it.

The sequence those open items run in is
[`docs/REMAINING-WORK.md`](REMAINING-WORK.md): H0 stop lying, H1 a desktop you
can use, H2 the ThinkPad lights its own panel, H3 write zl on zlOS.

This file replaces twenty-one planning documents totalling ~392 KB. Nineteen of
them were audited item by item against the merged tree (870 items), plus three
cross-check lenses; two more — `kernel/docs/POINTER-PROMPT.md`, which is live
work rather than history, and `docs/INTEGRATION-PLAN.md`, which is superseded —
are represented here without being re-audited. Almost every one of those
documents predates the eleven-track merge of 2026-08-19 and is therefore stale
by construction: reading one tells you what somebody wanted, never what is true.
**Everything below carries the command that established it, or the file and line
that does.** Where the audit could not settle something, it says so and names the
command that would. This file does not summarise the twenty-one documents. It is
the one page you read to know what to do next.

---

## How to read this

**The ordering principle is leverage: what buys the most, soonest.** Within each
subsystem, entries are ordered by (what the fix unblocks or the failure it
prevents) ÷ (what the fix costs), with a deliberate thumb on the scale for
changes that convert a *silent* failure into a *loud* one. A six-line change that
makes twenty-six harnesses start failing when they should outranks a
thousand-line feature, every time. Subsystems themselves are ordered the same
way, which is why "build and gates" comes before the display driver even though
the display driver is the point of the project.

Three words are used precisely throughout, because this repo's dominant failure
mode is confusing them:

| Word | Means |
|---|---|
| **exists** | the function is written and compiled |
| **reachable** | something in the shipping tree calls it |
| **covered** | a gate that actually runs executes the call |

*A primitive is not done when it passes its test; it is done when something calls
it and a gate covers the call.* Five complete, tested, gated primitives in this
repo had no caller at all (`kernel/HANDOFF.md:473-483`). The audit found more.

`unverifiable` is used only where a static check genuinely cannot settle the
question, and every such entry names the command that would.

---

## 1. The five that come first

Ranked across the whole board, not per subsystem. Each has its full entry below.

1. **Wire the gate to run what it builds.** `gates/land-gate.sh` compiles ~26
   host harnesses and executes none of them, and runs neither of the two
   static checkers that exist. Six lines. → §2.1, §2.2
2. **Land `fix/pointer-drain`.** Three commits, written, tested, unmerged,
   branching off the exact commit this audit was pointed at. The pointer is
   visibly broken on `main`. → §3.1
3. **Fix `key()` at `kernel.zl:1407`.** One token. It halts the kernel on the
   panel-handover path — the exact path that is the project's stated
   highest-impressive-per-remaining-work item. → §5.1
4. **`DISK_SCRATCH = 0x02040000`, and make `check-memmap.sh` sweep.** The
   collision `MERGE-EVIDENCE.md` records as fixed was never fixed, and the
   checker that should see it cannot. → §8.1
5. **Restore `mkdisk.sh`'s kernel-size guard.** Ten lines, cherry-pickable from
   `premerge/apps`. Without it an oversized kernel is silently truncated and
   jumped into. → §2.4

---

## 2. Build, gates and CI

This subsystem is first because everything else in this document is a claim, and
this is the machinery that decides whether claims stay true.

### 2.1 The land gate builds every host harness and runs none of them

`gates/land-gate.sh` executes exactly eight things: four builds, the SOURCES
coverage check, `hosttest/build.sh`, an inline reverse-SOURCES sweep, and seven
QEMU boot gates. `hosttest/build.sh` contains 30 `gcc` lines and zero execution
lines.

```
$ grep -n 'run "' gates/land-gate.sh
47:run "zl toolchain"     "$WT"               ./build.sh
48:run "kernel 32-bit"    "$WT/kernel"        ./build.sh
49:run "kernel 64-bit"    "$WT/kernel"        ./build64.sh
50:run "kernel EFI"       "$WT/kernel"        ./buildefi.sh
51:[ -x "$WT/kernel/verify-sources.sh" ] && run "SOURCES coverage" ...
52:run "hosttest build"   "$WT/kernel/hosttest" ./build.sh
90:  run "boot: $g" "$WT/kernel" "./$g"

$ grep -c gcc kernel/hosttest/build.sh
30
$ grep -n '^\s*\./' kernel/hosttest/build.sh
(no output — nothing is executed)
```

So `./wmtest`, `./inputtest`, `./tritest`, `./systest`, `./arenatest`,
`./exectest`, `./fbbench`, `./killtest.sh` and the rest are compiled and thrown
away. `MERGE-EVIDENCE.md:273` reports "26 harnesses pass, 0 fail" inside a list
of gate results; **that was a human typing the binaries by hand, and nothing
re-checks it.** Seven of the nineteen readers found this independently. It is one
structural fact with one six-line fix, and it converts ~26 harnesses and the feel
track's 213 assertions from decoration into coverage.

The pointer probes are in the same position and `HANDOFF.md:792` calls them "not
optional extras": `probe-dock.py`, `probe-resize.py`, `probe-drag.py`,
`probe-frame.py`, `probe-smp.py` are run by nothing. They need QEMU and belong
behind the same `guard()` the boot gates already use.

*Source: xcheck-status-conflict lens; EXEC-38, DTODO-43, OVN-GATE-01, FEEL-34,
OSLAND-07, GRAPHICS-18, DISPROAD-37.*

**Consequence for the rest of this document: no entry anywhere below may use the
word "gated" to mean a host harness. The honest word is "asserted, unrun."**

### 2.2 Three static checkers exist, none is in the gate, and one is a false green by construction

```
$ grep -n 'check-zl-calls\|check-memmap\|memmap-guard\|probe-' gates/land-gate.sh
(no output)
$ git ls-files | grep -E 'check-|memmap-guard'
kernel/check-memmap.sh
kernel/check-zl-calls.sh
kernel/hosttest/memmap-guard-test.sh
```

All three run in seconds with no QEMU and no hardware. Adding **three** lines to
the cheap block before `land-gate.sh:55` is the single cheapest change in this
document.

**`kernel/hosttest/memmap-guard-test.sh` is the one the first draft of this
document missed, and it is the strongest of the three.** It is `-fsyntax-only`
against the same CFLAGS `build.sh` uses, so it needs no toolchain, no QEMU and no
hardware. Twelve checks: one baseline compile of the six map owners
(`fb.c sched.c i2c_hid.c nvme.c xhci.c virtio_gpu.c`), five deliberate
perturbations of `memmap.h` that the build must *refuse* — including a replay of
the real HID-buffer-inside-the-blur-arena bug — and six `same` blocks proving
rebased address literals still equal the numbers they replaced.

```
$ grep -cE '^(expect_pass|expect_break|same) ' kernel/hosttest/memmap-guard-test.sh
12
$ grep -rn 'memmap-guard' gates/land-gate.sh kernel/hosttest/build.sh
(exit 1 — wired into neither)
$ grep -n 'memmap-guard' kernel/HANDOFF.md
756:cd kernel/hosttest && ./memmap-guard-test.sh    # seconds, no QEMU, no hardware
```

`HANDOFF.md` documents it by name with its run command and nothing runs it. It
also gives §5.3 a home: adding `intel.c` to that script's `OWNERS` list is where
the `edid_buf` static assert gets *proven*, not merely written.

`check-memmap.sh` is worse than absent — run by hand it still cannot see the live
collision, because it iterates a hardcoded list with no discovery step:

```
$ sed -n '30,31p' kernel/check-memmap.sh
for name in SNAKE_X SNAKE_Y FS_META FS_DATA FS_SLOT \
            LINE_BUF LINE_MAX HIST_BUF HIST_N; do

$ bash kernel/check-memmap.sh | tail -3
    0x02030000 .. 0x020300C8  LINE_BUF      200 bytes
    0x02031000 .. 0x02032000  HIST_BUF     4096 bytes
  OK: no overlaps, 56 bytes spare in each history slot
```

No `DISK_SCRATCH` row. No `PAINT_BUF` row (`kernel.zl:1161`). No `edid_buf` row.
The script's own header says "A check that can pass vacuously is worse than no
check" (`:28`), two lines above the loop at `:30` that does exactly that.

`MERGE-EVIDENCE.md:206-216` specified three checkers before the merge. Checker #3
(the CALL/REG table) is `check-zl-calls.sh`. **Checker #1 (the address-table
sweep) is half-written** — `memmap-guard-test.sh` covers `memmap.h` and the six
C owners, and covers neither `kernel.zl`'s `NAME = 0x…` constants nor `intel.c`'s
`edid_buf`, which is exactly the uncovered half that §8.1 and §5.3 are about.
**Checker #2 (the symbol table) was never written.**

*Source: xcheck-unowned lens; xcheck-contradiction XC-01, XC-03.*

### 2.3 The reverse-SOURCES sweep passes on the one file it was built to catch

`kernel/interp_kernel.c` is 721 lines — the kernel-side zl interpreter, the thing
that would let the kernel run a `.zl` from disk. It is not in `kernel/SOURCES`,
so no build compiles it. The sweep's three-way classification lets it through:

```
$ grep -n 'interp' kernel/SOURCES
(no output)
$ grep -n 'interp_kernel.c' kernel/hosttest/build.sh
199:gcc -O2 -w -o libctest libctest.c ../interp_kernel.c ../arena.c -lm
$ sed -n '66,67p' gates/land-gate.sh
    if grep -q "$b" "$WT/kernel/hosttest/build.sh" 2>/dev/null; then
      echo "host-only (not in the kernel): $b"; hostonly=$((hostonly+1))
```

`interp_kernel.c` lands in the "host-only, correct" bucket purely because
`libctest` happens to link it. The predicate is weaker than the property it
defends. Cheapest fix is an explicit `kernel/HOST_ONLY` allow-list so the
classification is a stated decision rather than a coincidence of who links what —
the same defect shape as `check-memmap.sh`'s name list.

*Source: EXEC-PROMPT reader; xcheck-contradiction XC-13.*

### 2.4 `mkdisk.sh`'s kernel-size guard was written, and did not survive the merge

Three documents say it exists. It does not.

```
$ grep -n 'CHUNKS\|LIMIT\|refus' kernel/mkdisk.sh
(no output)
$ git log --oneline -S'CHUNKS' -- kernel/mkdisk.sh
(no output — never in this history)
$ git show premerge/apps:kernel/mkdisk.sh | grep -n 'CHUNKS\|LIMIT\|FAIL'
120:CHUNKS=$(grep -oP 'CHUNKS\s+equ\s+\K[0-9]+' raw_boot.asm)
121:LIMIT=$((CHUNKS * 64 * 512))
123:if [ "$KSIZE" -gt "$LIMIT" ]; then
124:    echo "FAIL: kernel is $KSIZE bytes; raw_boot.asm loads only $LIMIT" >&2
```

`raw_boot.asm` loads a fixed number of chunks. A kernel over that limit is not a
build error — it is silently truncated and jumped into
(`HANDOFF.md` v10 item 3). This is the highest-severity item in the whole audit
that is a pure code regression rather than a documentation error, and it is
`MERGE-EVIDENCE.md`'s own under-weighted class — a deletion that kept nothing.
One cherry-pick of ~10 lines. Then `desktop-v10-plan.md §8.2.5`,
`desktop-TODO.md` and `HANDOFF.md` become true.

*Source: xcheck-contradiction XC-08; V10-29, DTODO-39.*

### 2.5 `ci/gates-and-agent-brief` is 11 commits unmerged and is the only place CI exists

`main` has zero CI. Not stale CI — no `.github` directory at all.

```
$ git ls-files '.github/*' | wc -l
0
$ git log --oneline main..ci/gates-and-agent-brief | wc -l
11
$ git ls-files | grep -ci zlfmt
0
```

The branch carries six GitHub Actions workflows, a PR template, a `tools/` suite
(`preflight.sh`, `hazard-scan.sh`, `doc-check.sh`, `engine-parity.sh`,
`journal.sh`, `todo.sh`, `install-hooks.sh`), `AGENTS.md`, `TODO.md`,
`docs/JOURNAL.md`, and **the only tracked copy of `zlfmt.c` and `verify_fmt.sh`
anywhere in the repository**. It forks at `c064742`, ~98 commits behind `main`,
so this is a real merge, not a fast-forward. This converts every gate in this
repo from "a human remembered" to "a push failed."

Two documents say 9 commits (`docs/DOCS-RECONCILE-PROMPT.md:131`,
`kernel/docs/POINTER-PROMPT.md:208`). The number is 11.

*Source: xcheck-unowned lens.*

### 2.6 Four compiled ELF binaries and two `.ppm` files are tracked

```
$ git ls-files | while read f; do [ -f "$f" ] && file -b "$f" | grep -q ELF \
    && echo "$f $(stat -c%s "$f")"; done
kernel/hosttest/inputtest_feel 56016
kernel/hosttest/inputtest_hid  31280
kernel/hosttest/wmbench       779736
kernel/hosttest/wmtest_feel   791432
```

They arrived with the feel and apps landings. `CLAUDE.md:128-131` gives a command
and asserts it "returns nothing"; re-run verbatim it returns four paths
(`.ultra/STATE.md`, `.ultra/TENSIONS.md`, and two `hosttest/*.ppm`). Note the
sharper point: the four ELF binaries match no ignore rule at all, so the stated
check would never have caught them either. `git rm --cached` the six, extend
`kernel/.gitignore`, and the `CLAUDE.md` sentence becomes true again.

*Source: FEEL-33; xcheck-contradiction XC-17.*

### 2.7 `run_tests.sh` — the zl language suite — is not in the land gate

`run_tests.sh` is the only thing that checks the language the kernel is written
in, and `gates/land-gate.sh` does not call it. It also only cross-checks
`tests/*.zl` against the C backend, never against `compilel`, which matters for
§9.2.

*Source: UNBOX-10B, STDLIB-45.*

### 2.8 Half the boot gates omit `-m`, so under those every address above 128 MiB is unbacked

**Corrected 2026-08-19 by the repair pass. The first draft of this entry said
"no gate passes `-m`", filed it as unverifiable-without-QEMU, and was wrong on
both counts.** Which scripts carry the flag is textual and settles with one grep:

```
$ for f in kernel/verify*.sh; do printf '%-24s %s\n' "$f" "$(grep -o '\-m [0-9]*' $f | head -1)"; done
kernel/verify-clock.sh   -m 512
kernel/verify-disk.sh    -m 512
kernel/verify-efi.sh     -m 512
kernel/verify-iso.sh
kernel/verify-raw.sh
kernel/verify-sources.sh
kernel/verify.sh
$ grep -n 'for g in' gates/land-gate.sh
87:for g in mkiso.sh verify.sh verify-iso.sh verify-efi.sh verify-raw.sh verify-disk.sh verify-clock.sh; do
```

`land-gate.sh:87` names seven scripts and **six of them boot** — `mkiso.sh`
builds an ISO and only prints a `qemu-system-i386` command for a human
(`mkiso.sh:71`). Of the six that boot, three pass `-m 512` and three pass
nothing.

`kernel/HANDOFF.md:764-769` names `verify.sh`, `verify-raw.sh` and `verify-iso.sh`
and measures QEMU's default at 128 MiB (`query-memory-size-summary` →
`base-memory: 134217728`). **That measurement stands for those three scripts. Its
next sentence — "NOT ONE GATE passes `-m`" — over-generalises and is false on
`main`, and this document inherited the over-generalisation into a heading.**
`verify-disk.sh` could not be otherwise: `nvme.c:103` puts `NMEM_ASQ` at
`HI_NVME` = `0x0D000000` (208 MiB), and that gate passes and power-cycles three
times (§8.3).

The real hazard is narrower and still worth fixing: a fixed buffer above 128 MiB
is exercised by three of the six booting gates and invisible to the other three,
so a high-RAM regression fails a *subset* of the gate rather than all of it. Add
`-m 512` to `verify.sh`, `verify-raw.sh` and `verify-iso.sh` so the whole loop
agrees on one guest size. Three one-line edits, plus a correction to
`HANDOFF.md:764`. The 128 MiB default value itself is HANDOFF's measurement and
was **not** re-taken here.

### 2.9 The browser's network path is covered by no gate, and cannot be

```
$ git grep -ln 'netdev\|-nic\|virtio-net' -- '*.sh' '*.py'
(empty)
```

`kernel/try.sh` attaches nvme, xhci, usb-storage, usb-kbd and usb-mouse, and no
network device. QEMU's default NIC for the i386 `pc` machine is e1000, which
`virtio_net_find()` will not match. The `N` command's ARP gate is written and
reachable (`kernel.zl:2634 fn net_gate()`, bound at `:2124`), and nothing runs
it. Fix is two QEMU flags in `try.sh`'s `COMMON[]` plus a `probe-net.py` shaped
like `probe-term.py`.

*Source: BROWSER-13.*

### 2.10 `exectest-nofs` is a harness that is not even compiled

**Added by the repair pass.** The first draft filed this in §11 as one of five
HANDOFF staleness points — i.e. as a documentation error. Its source graded it as
a coverage gap, and that is what it is:

```
$ grep -n 'EXECTEST_NO_FS' kernel/hosttest/exectest.c
109  150  173      (three guarded blocks)
$ grep -n 'exectest' kernel/hosttest/build.sh
213:gcc -O2 -w -o exectest exectest.c ../exec.c        (no -D)
$ grep -rn 'nofs' kernel/hosttest/build.sh
(no output)
$ grep -n 'exectest-nofs' kernel/HANDOFF.md
845:./exectest-nofs    # `run`, as it actually ships      32 checks, no QEMU
```

This is strictly worse than §2.1's ~26 harnesses, which at least compile:
`exec.c`'s NULL-weak "no fs driver" branch — the one every fs-less build lands on
— has no test binary at all. One line in `kernel/hosttest/build.sh`, the same
shape as §5.7's `dpll_test` line:
`gcc -O2 -w -DEXECTEST_NO_FS -o exectest-nofs exectest.c ../exec.c`.

*Source: EXEC-12; mis-filed by the first synthesis.*

---

## 3. Input and pointer

### 3.1 The pointer is visibly broken, and the fix is written and unlanded

`kernel/docs/POINTER-PROMPT.md` is the live work, written the morning after the
merge. Its Phase 1 diagnosis is measured and is **not re-diagnosed here**: two
independent drainers of one xHCI event ring, with `input.c` calling the pointer
once per frame and the keyboard up to sixteen times, so the keyboard loop eats
pointer reports. Moving the mouse in `./try.sh` produces jumpy, laggy motion.

**One correction to the names, made by the repair pass, because §3.3 depends on
getting the ownership question right.** The two functions are printed against
each other's line numbers in `POINTER-PROMPT.md:20-21`, and the first draft of
this document carried the swap:

```
$ grep -n '^int xhci_kbd_poll\|^int xhci_ptr_poll\|event_poll(0' kernel/xhci.c
1770:int xhci_kbd_poll(void)
1775:    int type = event_poll(0, &status, &ctrl, 1);
1784:int xhci_ptr_poll(void)
1789:    int type = event_poll(0, &status, &ctrl, 1);
```

`xhci_kbd_poll` is at `:1770` and drains at `:1775`; `xhci_ptr_poll` is at
`:1784` and drains at `:1789`. Correct it in `POINTER-PROMPT.md` too — the tree
is the referee.

The fix exists, on a branch nobody is landing:

```
$ git log --oneline main..fix/pointer-drain
340adee docs(pointer): correct the assertion counts (14 fail before, 26 pass after)
eae5248 docs(pointer): the findings phase 1 confirmed but deliberately did not fix
f334a3a fix(xhci): one owner for the event ring, and a pipeline for the pointer
$ git log --oneline -1 $(git merge-base main fix/pointer-drain)
06ced13
$ git ls-files | grep xhcitest
(exit 1 — the 651-line harness does not exist on main)
```

It branches off `06ced13`, the exact commit the nineteen readers were pointed at,
so **no reader could see it and none did.** `git diff --stat main...fix/pointer-drain`
is 17 files, 1,439 insertions, including a new 651-line `hosttest/xhcitest.c`.

Landing it needs a gate run **and** the human check `POINTER-PROMPT.md:3-4`
explicitly requires: *"Do not start phase 2 until phase 1 is confirmed by a human
looking at a live VM, not by a gate going green."* Whether the 26 assertions pass
against merged `main` is **unverifiable statically**; `git merge --no-commit
--no-ff fix/pointer-drain` followed by building and running `xhcitest` settles it.

*Source: `kernel/docs/POINTER-PROMPT.md` Phase 1; xcheck-unowned lens.*

### 3.2 The dock readout leaves digit debris — confirmed visual regression

Carried from `kernel/docs/POINTER-PROMPT.md` Phase 1b, not re-diagnosed. The tray
reads `frame 0  us peak 0  )08  up 1`; the `)08` is the tail of a previous, wider
number. `kernel.zl` draws the status numbers at fixed x-offsets with no
background clear, and only `draw_clock()` clears, and only the `up` region. The
provenance is exact: the tray used to read the constant string
`state: compositor`, and `desktop/apps-in-windows` replaced it with live numbers
without adding the clear that live numbers need.

`probe-frame.py` cannot see this — it greps the serial log, not the screen. The
gate has to photograph the strip.

### 3.3 `input_ptr_x/y` — the seam that was built for this — has no caller

```
$ git grep -n 'input_ptr_x' -- kernel/ freestanding/
kernel/input.c:476:int input_ptr_x(void) { return px_x; }
kernel/hosttest/inputtest_feel.c: (harness only)
```

`freestanding/runtime_kernel.c:1497-1499` still resolves `mouse_x` to
`xhci_ptr_poll(); xhci_ptr_x()` or `idt_mouse_x()` — the raw ISR position,
unaccelerated — and `kernel.zl` still calls the raw builtins at `:619-621` and
`:665-666`. This is the fifth instance of the repo's own named pattern, and it is
entangled with §3.1: the duplicate `xhci_ptr_poll()` inside the `mouse_x` builtin
is one of the ring's unowned drainers. **Do not fix this before landing
`fix/pointer-drain`** — that branch changes the ownership question this entry
depends on.

*Source: FEEL-10.*

### 3.4 Pointer acceleration is implemented, correct, and dead under every gate

Two readers of the same document graded this `done` and `contradicted`, and both
were right about different things. The curve at `input.c:480-485` and `:555-560`
is implemented and correct.

**Corrected by the repair pass — the first draft said "every QEMU path in this
repo attaches a `usb-tablet`", which is false and contradicted this document's own
§2.9 eleven pages earlier.** The truth is worse for the curve, not better:

```
$ grep -n 'usb-mouse\|usb-tablet' kernel/try.sh
30:  -device usb-mouse,bus=xhci.0
$ for f in kernel/verify.sh kernel/verify-iso.sh kernel/verify-efi.sh \
           kernel/verify-raw.sh kernel/verify-disk.sh kernel/verify-clock.sh; do
    grep -o 'usb-[a-z]*' $f; done
(no output — no boot gate attaches any pointer device at all)
$ grep -rln 'usb-tablet' --include='*.py' kernel/
probe-dock.py probe-mouse.py probe-drag.py probe-snake.py probe-resize.py
probe-mouse-sync.py exercise.py
$ sed -n '525p;550,552p' kernel/input.c
    int x, y, b, tablet = xhci_ptr_ready();
    if (tablet) {
        px_x = x; px_y = y;
    } else if (dx | dy) {
```

`pump_mouse()` decides "tablet" from `xhci_ptr_ready()` (`:525`), which is true
for **any** xHCI pointer — tablet or relative mouse. `try.sh` attaches a
`usb-mouse`, so the absolute branch is taken and the curve is skipped there too.
Only the seven `probe-*.py` scripts attach a real `usb-tablet`, and no
`verify-*.sh` boot gate attaches a pointer at all. So the curve executes only on
a machine whose pointer arrives through PS/2 rather than xHCI — the ThinkPad's
TrackPoint, and the ThinkPad has never booted zlOS.

**Do not record "pointer acceleration: done."** No gate in this repo can support
that claim. `POINTER-PROMPT.md` item 1 raises the adjacent decision that also has
no owner: `pump_mouse()` decides "tablet" from `xhci_ptr_ready()` rather than
`xhci_ptr_abs()`, so Settings' pointer-speed slider does nothing for any USB
mouse. Decide deliberately — accelerate the relative case, or say in a comment
that USB pointers are 1:1 by design and make Settings say so.

*Source: FEEL-06 / FEEL-09; xcheck-status-conflict lens.*

### 3.5 `i2c_hid.c` is a transport with no decoder

343 lines, and its whole exported surface is probe/ready/address/vid/pid/
version/max_input/rdesc_len/read_report/byte. `i2c_hid_byte(i)` returns raw
undecoded bytes. Nothing turns a touchpad report into an x, a y and a button —
that code does not exist. QEMU has no Intel LPSS I2C, so the laptop is the only
machine that can produce a real descriptor; write the report-descriptor walker as
a host test first, fed a captured byte array. `kernel/hosttest/inputtest_hid.c`
already exists as the harness shape.

*Source: DESKPLAN-15; corroborated `HANDOFF.md:450-453`.*

### 3.6 The desktop looks coarser than v10 — ~~cause not found~~ **cause found, and both halves are now closed**

> **UPDATED 2026-08-19, second session.** The entry below is the state before
> §1c was answered. Both halves are settled and the answer is not in this
> section any more:
>
> - **Why it looked coarser:** §1c's own ANSWERED box. `term_draw` went
>   monospace at `663a110`, so the shell's text got 15% wider inside an
>   unchanged window. That change is **right** — a terminal is monospace and
>   three space-aligned tables depend on it — so the look difference stays.
> - **The clipping it caused** was `DECISIONS.md` item G and is closed by **#35**:
>   `term_draw` wraps. One correction to §1c's arithmetic while doing it — the
>   shell is **75** columns, not 77, because `kernel.zl:2934` insets the client
>   by the toolkit's padding before `term_draw` is handed it. Gated by
>   `hosttest/termwrap`.
> - **A second, larger contributor found separately:** two palettes were on
>   screen at once — two cyans and two panel colours. `DECISIONS.md` item E,
>   closed by **#34**, gated by `hosttest/palette`. Before/after renders are in
>   [`shots/palette-before-two-palettes.png`](shots/palette-before-two-palettes.png)
>   and [`shots/palette-after-one-palette.png`](shots/palette-after-one-palette.png).
>
> The concurrency note below turned out to be right and is worth keeping as the
> example: that session's deletion of §1c and its two PNGs was in-progress work,
> not a resolution, and §1c was later restored byte-identical from `ff27d57`.

`kernel/docs/POINTER-PROMPT.md` Phase 1c records this as an open question with
four things already ruled out by command — no visual function was lost,
`fb_text_aa` and `fb_glyph_aa` are byte-identical to overnight's, `term.c` lost
nothing relevant, and `prop_big()` being unused is pre-existing. It points at
state or parameters rather than deleted code: `fn ui()`, `cell_w` in `fb_setup`,
and the window client rect.

**Concurrency note, stated because it changes what you will see:** at writing,
another session has a staged (uncommitted) deletion of Phase 1c and of
`docs/shots/{before,after}-merge-help.png` in this worktree's index. That is
their work in progress, not a resolution. Check `git status` before assuming this
entry is closed.

---

## 4. Compositor, toolkit and apps

### 4.1 `ui.c` is a complete toolkit that zl cannot call — the highest-leverage single unfixed thing in the repo

```
$ grep -nE 'streq\(name, *"ui' freestanding/runtime_kernel.c
987:  ui_scale
1169: ui_theme
```

Two `ui_*` builtins out of 540 registered, and neither is a widget. `ui.h:203-215`
declares eleven widget entry points — `ui_label`, `ui_label_dim`, `ui_bar`,
`ui_button`, `ui_toggle`, `ui_slider`, `ui_num`, `ui_list_row`,
`ui_scroll_begin/end/content` — and no zl app can reach one.

**This is "nobody has written it yet", not "the language cannot".**
`wmglue.c:8-13` documents that zl compiles to ordinary C functions, so the bridge
is mechanical; `wmglue.c:57-60` states the cost as ~15 new builtins. The real
design question, which no document answers, is the out-parameter crossing:
`ui_toggle(s, int *on)` in a language with no pointers. The obvious answer is a
zl-side state slot id.

**One thing the ~15-builtin figure does not cover, and it is the larger half.**
Fifteen builtins make the toolkit *callable*; they do not make `kernel.zl` use
it. `ui.h:1-14` states the layering contract — *"kernel.zl POLICY … calls `ui_*`
only - no coordinates, no `fb_*` calls"* — and the tree does the opposite:

```
$ grep -oE '\bui_[a-z_]+\(' kernel/kernel.zl | sort | uniq -c
      1 ui_scale(
      1 ui_theme(
$ for p in fill_rgb label line grad_rgb rrect gradient char_aa rrblend shadow \
           blend text_box; do printf '%-9s %s\n' "$p" "$(grep -c "\b$p(" kernel/kernel.zl)"; done
fill_rgb 26   label 25   line 8   grad_rgb 7   rrect 5   gradient 4
char_aa 2   rrblend 2   shadow 1   blend 1   text_box 1      # 82 raw calls
```

Plus a second copy of the theme at `kernel.zl:33-41` (`ACCENT`, `PANEL`,
`TXT_HI`, `TXT_DIM`, …) duplicating `ui.c`'s `struct ui_theme`. So the honest cost
is ~15 builtins **plus** migrating 82 call sites **plus** deleting one of the two
themes, and `ui.h`'s contract is today a statement of intent rather than a
description of the tree. That does not change the ranking — nothing else unblocks
applications — but a reader who budgets fifteen builtins will be wrong by an
order of magnitude.

Two documents call this their own highest-leverage item (`desktop-TODO.md` T-18,
`desktop-northstar-feasibility.md`). They are right. Everything the mockup still
lacks is applications, and applications in this repo are zl.

*Source: NSTAR-01, DTODO-25; xcheck-contradiction XC-14.*

### 4.2 The Settings app is compiled into every build and cannot be opened

`kernel/settings.c` is 620 lines, in `kernel/SOURCES`, with all six controls laid
out (`build_ui()`, `:198-219`) and all six sinks driven (`settings_apply()`,
`:111-141`). `wmglue.c:75` defines `APP_SETTINGS 6` and dispatches draw and event
to it.

```
$ grep -n 'APP_SETTINGS' kernel/kernel.zl kernel/wmglue.c
kernel/wmglue.c:75:#define APP_SETTINGS 6
kernel/wmglue.c:80: ... :88   (dispatch arms)
kernel/kernel.zl:2736,2738    (comments only — no declaration)
```

`kernel.zl` numbers 5 then 7 and skips 6. **Nothing can open it.** One
declaration plus a menu row. `wmglue.c`'s own comment says zl and C apps share
one id namespace — that is the class the merge hit eight times.

*Source: FEEL-12; xcheck-contradiction XC-14.*

### 4.3 Settings persistence is write-only

```
$ git grep -n 'settings_load' -- ':!kernel/out.c'
kernel/settings.c:476        (the definition)
kernel/hosttest/settingstest.c (14 call sites, plus the forward
                               declaration at :34, which is not one)
```

No kernel caller — nothing in `kernel.zl`, `wm.c`, `wmglue.c`, `console.c` or any
boot path. Call it once next to `console_init`/`fb_setup` before the first
`wm_frame` and let its own diagnostics report the fallback. That is one line, and
it depends on §4.2 to be worth anything.

*Source: FEEL-19.*

### 4.4 RESOLVED: the editor owns the clipboard write path

**Files-app increment, 2026-08-19.** `editor_key()` now handles Ctrl+C by
calling `clip_new()`, `clip_add()` for every document byte, then `clip_done()`;
Ctrl+V reads `clip_n()`/`clip_ch()` back into the editor. `probe-files.py`
drives both shortcuts before its cold-boot persistence check. The evidence
below describes the audit baseline and is kept because it identifies the merge
deletion that caused the gap.

`clip.c` is 138 lines, compiled (`SOURCES:65`), and covered by
`hosttest/systest.c` (104 assertions, including a two-app copy/paste round-trip).
At the audit baseline, nothing called it.

```
$ grep -oE '\bclip_[a-z_]+' kernel/kernel.zl | sort -u
clip_ch
clip_n
$ grep -rn 'clip_push\|clip_begin\|clip_commit' kernel/*.c freestanding/*.c \
    | grep -v kernel/clip.c
freestanding/runtime_kernel.c:692-695   (externs)
freestanding/runtime_kernel.c:1620-1623 (registrations)
```

At the audit baseline, `kernel.zl` called only the read side.

**The producer is not missing from `wm.c`, `term.c` or `input.c` — it belongs in
`kernel.zl`, it was written, and the merge deleted it.** The first draft of this
entry sent the reader to the wrong three files:

```
$ git merge-base --is-ancestor c2123f0 HEAD && echo ancestor
ancestor
$ git log --oneline -1 c2123f0
c2123f0 feat(clip): copy in the editor, paste onto the disk - and no routing changed
$ git show c2123f0 -- kernel/kernel.zl | grep -E '^\+.*clip_(new|add|done)'
+                clip_new()
+                while ci < len { clip_add(peek8(EDIT_BUF + ci))  ci = ci + 1 }
+                clip_done()
$ sed -n '1316,1326p' kernel/kernel.zl
fn editor_key(code) {      # branches on 27, 8, 13 and code >= 32 only —
                           # 3 (Ctrl+C) and 22 (Ctrl+V) fall through to return 0
```

`c2123f0` added `clip_new`/`clip_add`/`clip_done` branches to `fn editor_key`;
`apps-in-windows`' rewrite of `editor_key` won the `kernel.zl` collision and took
them with it. Half the plumbing survived — `edit_key()` at `kernel.zl:100-103`
still tracks the ctrl modifier. So this is `git show c2123f0` re-applied against
today's `ed_len`/`ed_slot`, not new code, and it is another instance of
`MERGE-EVIDENCE`'s under-weighted deletion class — here, a deletion that kept its
*readers*.

The consequence with a face on it:
`run_command`'s `clip.txt` writer at `kernel.zl:1951` opens with
`if clip_n() == 0 { ... }`, so on real hardware that command prints its
nothing-to-save message forever. Until a key binding exists, **no document should
describe the clipboard as a shipped feature.**

*Source: xcheck-unowned lens; SYSTEM-PROMPT reader.*

### 4.5 `fb.c` declares `icons24[10]`; `icons.c` defines `icons24[20]` — CLOSED 2026-08-19

> **CLOSED**, `desktop/v10-look` `76d15aa`. `ICON_N` raised 10 → 20, both
> externs corrected. **Only the mismatch is fixed** — no caller in `kernel.zl`
> passes `n >= 10` yet (`dock_icon()` still returns 0–9 only), so the ten
> icons are drawable now but nothing draws them. Wiring one to a dock slot,
> menu row or title-bar icon is a separate, undecided design question.
> `kernel/docs/DECISIONS.md` #40.

```
$ grep -n 'icons24\|ICON_N' kernel/fb.c
2949:extern const unsigned char icons24[10][24][24];
2954:#define ICON_N  10
2975:    if ((unsigned)n >= ICON_N) return;
$ grep -n 'const unsigned char icons24' kernel/icons.c
38:const unsigned char icons24[20][24][24] = {
```

It links only because the element type matches. The ten icons the v10 pass added
cannot be drawn by any path. Two-line edit, and it is a real mismatched-extern
bug, not just an unreachable feature.

*Source: xcheck-contradiction XC-14.*

### 4.6 Two resize-grip renderers ship inside one function — CLOSED 2026-08-19

> **CLOSED**, `desktop/v10-look` `580aabe`. Deleted the earlier `:805-815`
> block exactly as this entry recommended. What was "unverifiable statically"
> here is now verified: `hosttest/wmshot` before/after at the same corner is
> `docs/shots/grip-before-two-renderers.png` /
> `docs/shots/grip-after-one-renderer.png` — the extra, longer diagonal
> strokes are gone. `kernel/docs/DECISIONS.md` #41.

```
$ grep -n 'THE GRIP HAS TO BE VISIBLE\|THE RESIZE GRIP' kernel/wm.c
805:    /* THE GRIP HAS TO BE VISIBLE or it is a secret. ...
865:    /* THE RESIZE GRIP, drawn. A corner you cannot see is a corner nobody finds,
1162:/* THE RESIZE GRIP. wm_resize() has existed since wm.c was written ...
1225:/* THE RESIZE GRIP. ...
```

`chrome()` draws the grip twice, from two merge parents, in one pass: three
`fb_line` diagonals from a `UI_S1`-scaled corner in `t->border` at `:805-815`, and
three more from a `UI_S3`-scaled corner in `t->text_dim` at `:867-886`. Six
diagonals, two colours, two scales, on every window, every repaint. This is
`MERGE-EVIDENCE.md` §2.2's class surviving in the shipped compositor. The
hit-test half of the same collision *was* resolved — only `in_resize_grip`
survives at `wm.c:1175`.

Delete the `:805-815` block; it is the earlier, dimmer, wrongly-scaled one and it
draws before the title bar is composited. **Whether the result looks right is
unverifiable statically** — settling it needs `kernel/hosttest/wmshot` built, run,
and `wmshot.ppm` looked at.

*Source: DESKPLAN reader; xcheck-contradiction XC-18.*

### 4.7 Four stale "wm_resize has no caller" comments, and one orphaned prose block

`wm_resize` is now reachable from four independent paths and every one works
(`wm.c:1114`, `:1129-1131` snap; `:1278` grip; `:1327` drag-resize;
`runtime_kernel.c:1321` the `wm_size` builtin). What did not get cleaned is the
prose — **but only part of it, and the first draft's "keep one claim, delete the
rest" would have been a net loss of information.**

Four sites state the old fact and then, in the same or the next sentence, name
the caller they are introducing. That is correct provenance and the only record
of why each caller exists. **Leave them alone:** `wm.c:76-77` ("These two
triggers are its first callers"), `runtime_kernel.c:1318-1320` ("The browser is
the first thing that needs it"), `kernel.zl:2838-2839` ("this is the first"),
`ui.h:123` ("had no caller at all until the resize grip").

Three carry the bare claim with no correction attached and are now simply false:
`snap.c:3-4`, `wm.c:1162-1163`, `wm.c:1227-1229`. Fix those three. Separately,
`wm.c:1225-1239`'s "THE RESIZE GRIP" block now heads `in_closebox()`
(`sed -n '1241p' kernel/wm.c`) and needs re-siting. This is the canonical example
of the repo's naming hazard and costs nothing but attention.

*Source: xcheck-status-conflict lens.*

### 4.8 `ui_list_row` and `ui_scroll_*` have no caller outside the harness

The rest of the toolkit is genuinely used from C — `settings.c:203` `ui_button`,
`:208/:212` `ui_slider`, `:214/:217/:218` `ui_toggle`. But `ui_list_row`
(`ui.c:414`) and `ui_scroll_begin/end/content` (`ui.c:443`, `:476`, `:518`) are
called only from host harnesses — **three of them, not one as the first draft
said**: `hosttest/wmtest.c:465-499`, `hosttest/wmtest_feel.c:144-146` and
`:573-607`, and `hosttest/wmshot.c:138-140`. `wmshot` is the one that matters,
because §4.6 and §14 both send the reader to build and look at it. `settings.c:187`
mentions `ui_list_row` only in a comment about a draft that was removed.
`desktop-TODO.md` marks the toolkit done "through `ui_scroll`"; it is not.

*Source: OVN-PROC-07 vs DTODO-24; xcheck-status-conflict lens.*

### 4.9 Frame time is on screen, in the tray, not in the System Monitor

Two readers disagreed and both were describing the truth of a different
destination. `kernel.zl:425` draws `wm_us()` in the taskbar tray and `:2152`/`:2166`
print it from the shell. `sysmon_body` (`kernel.zl:2250`) has no `wm_us` call.
State the destination rather than picking a status. Worth recording as the
counter-example: `MERGE-EVIDENCE.md` §2.3's duplicate-`wm_frame_us` landmine did
*not* survive — there is exactly one definition, at `wm.c:1514`.

Whether the on-screen number agrees with `fbbench` is **unverifiable statically**;
it needs `fbbench` built and run against a booted `probe-frame.py`.

*Source: DTODO-12 vs PLATFORM-32/33; xcheck-status-conflict lens.*

### 4.10 Two snake window openers share one global with different geometry

The duplicate *implementation* is genuinely resolved — only `sn_*` exists, no
`snake_rand/start/key/step/draw`. What survived is two openers writing one
`snake_win` global: `kernel.zl:2984` (280×160, from the typed command) and
`kernel.zl:3189` (400×330, from the dock), and only one calls `sn_reset`. Benign
today because `sn_draw` self-heals on a grid-size change. Debris, not a bug — but
`MERGE-EVIDENCE.md`'s "Two snakes, resolved" should not be read as closing the
item.

*Source: PLATFORM-48; xcheck-status-conflict lens.*

### 4.11 The line editor and command history have no caller, and nothing replaced them

The inversion that made the compositor the top of the system replaced blocking
`read_line()` with per-character `term_key()`. `kernel.zl:2861` records the intent
plainly. What it does not say is what that stranded:

```
$ grep -n 'read_line' kernel/kernel.zl
903   (the definition)
2861  (a comment)
       — no call site
$ grep -n 'hist_load\|hist_save' kernel/kernel.zl
858, 870  (definitions); 914, 919, 954, 966 — all inside read_line (903-1000)
$ grep -n 'hist' kernel/term.c ; wc -l kernel/term.c
(no match)  357 kernel/term.c
```

So backspace-within-line, cursor movement and up/down history recall all went
dark, and `term.c` contains no history ring at all. ~170 lines of dead zl
reserving two address ranges. **This is also why §8.1's address collision is
currently inert — fixing the shell re-arms the address bug.** Someone must
decide, and no document does: port history into `term.c`, or delete
`kernel.zl:827-1000` with `LINE_BUF`/`HIST_BUF` and record that zlOS has no
command history.

*Source: xcheck-unowned lens — no planning document owns this.*

### 4.12 `font_big.c` and `icons_rgb.c` are 6,733 lines referenced by nothing, and the gate is designed never to fail on them

```
$ grep -rn 'font24x48' kernel/*.c kernel/*.h freestanding/*.c
kernel/font_big.c:6   (only)
$ grep -rn 'icons_rgb' kernel/*.c kernel/*.h freestanding/*.c | grep -v icons_rgb.c
(no output)
$ grep -n 'font_big\|icons_rgb' kernel/SOURCES
(exit 1)
$ wc -l kernel/font_big.c kernel/icons_rgb.c
4757 + 1976 = 6733
```

Neither is in SOURCES, so this is dead weight rather than a link hazard. It will
sit there forever for a structural reason: `gates/land-gate.sh:74-77` classifies a
`.c` absent from SOURCES and referenced by nothing as "dead (referenced by
nothing)" and deliberately does not count it toward `$miss`. The gate prints the
observation on every run and never forces a decision.

Worth checking before deleting: the nearest live builtin, `text_big`
(`runtime_kernel.c:1471`), routes to `console_text_aa2x` — a 2× scaler over the
existing AA font, not `font24x48_aa`. Both generators (`gen_bigfont.py`,
`gen_icons_rgb.py`) are tracked, so deletion is reversible by regeneration.

*Source: `MERGE-EVIDENCE.md` "Still open"; xcheck-unowned lens.*

### 4.13 The tiled rasterizer has no production caller, on purpose and with the measurement attached

Recorded so nobody deletes or "fixes" it. `fb3d.c` as a file is reachable —
`kernel.zl:1037` → `cube3d` builtin (`runtime_kernel.c:1483`) → `console.c:457` →
`fb_cube_filled`. But `fb_cube_filled` takes the *scanline* `fill_poly` at
`fb3d.c:343`; `fb3d_tri` (`:146`) and `fb3d_poly` (`:253`) are called only from
`hosttest/tritest.c`. Commit `003daa2` states why, with numbers: the tiled path is
2.8× slower for flat-shaded triangles, 400 large triangles scanline 45.2 µs vs
tiled 126.5 µs. The tile structure is there for the depth test, texture lookup and
vector unit that have not arrived.

**`GRAPHICS-18`'s headline — "fb3d.c has NO CALLER" — is wrong as written** and
would send a reader to delete or wire code that is already reachable. The correct
statement is the scoped one.

*Source: OSLAND-03 vs GRAPHICS-18; xcheck-status-conflict lens.*

### 4.14 The window fade blends its saved backdrop at the wrong origin — CLOSED 2026-08-19

> **CLOSED**, `desktop/v10-look` `6e63bf8`. `sx/sy/sw/sh` now capture the
> stash's own rectangle before the client `isect` below can clobber
> `cx/cy/cw/ch`, and the blend uses those. Also widened the blend's scissor
> from the client rect to the full frame+shadow box, on the reasoning that the
> surrounding comment's own algebra (`window * a + behind * (1-a)`) is for the
> whole window, not the client alone — see `kernel/docs/DECISIONS.md` #42 for
> that call and for a "found while checking this" note that `WF_NOCHROME` is
> declared and checked in six places and set in none. **"Whether the
> misalignment is visible is unverifiable statically" is no longer true**: a
> standalone probe measured up to a 40/255 per-channel shift at a fixed
> mid-fade frame, before vs after, on the same scene — numbers in #42. What is
> still true: `hosttest/wmtest`'s existing `ANIM_FADE` assertions passed
> before AND after this fix, so they are a regression floor, not proof of it —
> a precision-of-origin assertion does not exist yet.

**Added by the repair pass. The northstar audit found this and the first
synthesis dropped it** — the only entry in §4 that is a live bug in the shipped
compositor rather than an unreachable primitive, and it is the §4.6/§4.7 class:
one name meaning two things inside one function.

```
$ grep -c 'int cx, cy, cw, ch' kernel/wm.c
1
$ sed -n '946p;973,974p;987p;996,997p;1018p' kernel/wm.c
            int cx, cy, cw, ch;                       # declared once, per window
                       W->x + W->w + reach, W->y + W->h + reach,
                       rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch)) continue;   # FRAME+SHADOW
                if (fade < 255) stash = fb_stash(cx, cy, cw, ch);
            if (hook_draw && isect(ax, ay, ax + aw, ay + ah,
                                   rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch)) {  # CLIENT
                fb_stash_blend(stash, cx, cy, 255 - fade);      # <- clobbered origin
```

`fb_stash` (`fb.c:2549`) captures the frame-plus-shadow rectangle. The
client-rect `isect()` then overwrites the same four variables — `isect`
(`wm.c:766-775`) writes its out-params only when it returns 1 — and
`fb_stash_blend` uses the clobbered pair as the destination origin. A fading
window therefore composites its saved backdrop offset by the border width and the
title-bar height. It is intermittent, which is why nobody has seen it: when
`hook_draw` is null or the client rect misses the damage rect, the second `isect`
does not fire and the origin survives. Fix is four lines — keep the stash rect in
its own `sx/sy/sw/sh` and blend against those.

While you are in there: `wm.c:1004-1015` still tells the next reader that
ANIM_FADE "is NOT drawn here" and "waits for the scratch arena in `fb.c`", three
lines above the code that draws it. §4.7's class again.

**Whether the misalignment is visible is unverifiable statically** — it needs
`hosttest/wmshot` built and run mid-fade.

*Source: NSTAR-03 — dropped by the first synthesis, recovered by refute-completeness.*

---

## 5. Display — the Intel driver

The project's stated point. `HANDOFF.md` is the authority on what the driver has
proven on real hardware and this document does not re-litigate any of it. What
follows is only what is open, and the ordering is by what stands between the
current state and *zlOS lighting the ThinkPad's panel itself*.

### 5.1 `key()` halts the kernel on the panel-handover path — one token

```
$ bash kernel/check-zl-calls.sh
ok: every kernel.zl call resolves to a builtin or a zl fn
KNOWN UNRESOLVED (pre-existing, see the header): key
note: 150 registered builtin(s) with no caller in kernel.zl

$ sed -n '1404,1409p' kernel/kernel.zl
        put("    the panel is ours. framebuffer at 0x")  hex(fb, 8)  print("")
        color(C_GREY)
        print("    press a key to move the console onto it")
        key()
        panel_console()
```

An unresolved call falls through the builtin dispatcher to
`kfatal("builtin not available in the kernel subset")`
(`freestanding/runtime_kernel.c:1678`), and `kfatal` (`:749-769`) prints and then
`for(;;) hlt`. **So typing `P` lights the panel and then halts the machine,
between a successful `panel_up()` at `:1395` and `panel_console()` at `:1408`.**

`in_key`, `term_key` and `usb_key` are the plausible intents;
`check-zl-calls.sh`'s header names all three and deliberately declines to guess.
That was the right call inside a merge and is the wrong call now. Ship the fix and
the gate wiring (§2.2) together — the fix alone closes this instance and none of
the next ones.

*Source: `MERGE-EVIDENCE.md` "Still open"; xcheck-contradiction XC-03.*

### 5.2 Correction to the most-repeated stale claim in the repo

**Five tracked files** say "nothing in the kernel arms `lt_armed`" (the first
draft said "seven files plus `CLAUDE.md`" and its own list six lines below named
five; the list was right). That is false on `main`, and it is the claim that
decides whether display Phase 0.1 is "one caller away" or already wired.

```
$ git grep -n 'intel_link_train_arm' -- kernel/ freestanding/ | grep -v hosttest
kernel/intel.c:2095:void intel_link_train_arm(int on) { lt_armed = on ? 1 : 0; }
kernel/intel.c:4232:    intel_link_train_arm(1);
kernel/intel.c:4234:    intel_link_train_arm(0);
kernel/intel.c:4245:    intel_link_train_arm(1);
kernel/intel.c:4247:    intel_link_train_arm(0);
$ grep -n 'panel_up' freestanding/runtime_kernel.c kernel/kernel.zl
freestanding/runtime_kernel.c:1443: streq(name, "panel_up") -> intel_bringup_panel()
kernel/kernel.zl:1395:        fb = panel_up()
$ grep -n 'intel.c' kernel/SOURCES
39:intel.c
```

**The corrected sentence, exactly:** `intel.c:4232` arms `lt_armed` inside
`intel_bringup_panel()`, reachable from `kernel.zl:1395` via the `panel_up`
builtin under the `P` shell command. *Nothing arms it automatically at boot.*

One of the nineteen readers (`wireless-plan.json`) asserted the unqualified
version was still true and mistook the setter's definition for its call sites.
**That bullet must not be carried forward.** Stale asserters to correct:
`HANDOFF.md:198,302,354`; `display-roadmap.md:14,30`;
`what-is-actually-impossible.md:156`; `wireless-plan.md:171`;
`driver-build-order.md:67`. **`CLAUDE.md:84-86` is not on this list** — it tells
the reader to *check* whether anything arms `lt_armed` rather than asserting that
nothing does, and that instruction is the one piece of guidance in the repo that
would have caught this class. Leave it alone.

*Source: xcheck-contradiction XC-02, XC-03; xcheck-status-conflict lens.*

### 5.3 `edid_buf` was at `0x0C980000`, inside `HI_BLUR` — CLOSED 2026-08-19

> **Fixed, and NOT by the three edits proposed below.** `edid_buf` is now an
> ordinary `static u8 edid_store[128]` in `intel.c`, reached through an
> `edid_addr()` accessor that still honours `intel_set_edid_buffer()` for
> `hosttest/intel_probe.c`.
>
> **It never needed a physical address.** Every byte arrives by CPU store —
> `gmbus_read_edid` reads the `GMBUS3` register and writes bytes out one at a
> time (`intel.c:751`), and the AUX path at `intel.c:1909` does the same. No
> engine DMAs into it, and DMA is the only thing that would require a known
> physical address. So the fix is not "give it a map entry and assert it" but
> "take it out of the map", which needs no `HI_EDID`, no ordering-chain entry,
> no `#include "memmap.h"` and no assert — there is nothing left to collide.
>
> The analysis below is kept because it is correct about the *defect*; only its
> prescription was one rung too high on the ladder.
>
> Verified: all four targets build; `verify.sh` and `verify-efi.sh` green.

*What follows is the original diagnosis.*

### 5.3-orig `edid_buf` is still at `0x0C980000`, inside `HI_BLUR`, and `intel.c` asserts nothing

```
$ grep -n 'edid_buf' kernel/intel.c
762:static uptr edid_buf = 0x0C980000u;
763:void intel_set_edid_buffer(uptr p);
    (reads/writes at 783, 784, 795, 1893, 1909, 1922)
$ grep -rn 'intel_set_edid_buffer' kernel/
kernel/intel.c:763  kernel/hosttest/intel_probe.c:122,458
    (no kernel-side caller)
$ grep -n '_Static_assert\|memmap.h' kernel/intel.c
(no output)
$ grep -rln 'memmap.h' kernel/*.c
fb.c nvme.c i2c_hid.c virtio_gpu.c xhci.c sched.c      (intel.c absent)
$ grep -n 'BLUR_LIMIT' kernel/fb.c
178:#define BLUR_LIMIT ((unsigned int)(HI_NVME - HI_BLUR))
```

`0x0C980000` is `HI_BLUR + 9.5 MiB`, and `BLUR_LIMIT` is the full 16 MiB, so
`fb.c` hands out cached-blur tiles across the address `intel.c:1909` writes EDID
bytes into. `memmap.h`'s own header says: put the base here, add it to the
ordering chain, and have the owning file assert its highest byte lands under the
next base — "All three, or the check has a hole in it." **`intel.c` does none of
the three.** `MERGE-EVIDENCE.md` §2.4 predicted this and lists it under "Still
open".

Three mechanical edits: declare `HI_EDID` in `memmap.h` (better still, outside
the blur arena entirely), `#include "memmap.h"` in `intel.c` and static-assert its
128 bytes, and call `intel_set_edid_buffer(HI_EDID)` once on the kernel bringup
path.

*Source: xcheck-unowned lens; DISPROAD-13, GEN9-04.*

### 5.4 The VBT parser's entry point is dead — Phase 1 unblocks Phases 2 and 3 and is not wired

`kernel/intel.c:4365 intel_vbt_find()` reads `gpu_cfg(ASLS_REG)` and attaches, and
has **no caller** — a grep over `kernel/*.c`, `kernel/*.h`, `kernel/hosttest/*.c`
and `freestanding/*.c` returns only the definition. No VBT builtin is registered
for zl (`grep -n 'vbt' freestanding/runtime_kernel.c` → nothing). No VBT value is
consumed by the modeset. So on `main` the VBT is reachable only from a manual
`sudo` host-harness run, and the hardcoding Phase 1 exists to remove is still
what the kernel runs on. Call `intel_vbt_find()` next to `intel_find()` and make
`intel_bringup_panel()` take T9/T12 and the low-vswing flag from it when
`vbt_ok`.

*Source: DISPROAD-16.*

### 5.5 EDID over AUX — three documents, three different wrong statuses

Two functions with almost the same name, different caller status, which is why
the readings diverged.

- `intel_edid_over_aux()` (`intel.c:1891`) — written; **two** callers, and
  neither ships: `intel.c:1921`, inside the dead wrapper below, and
  `hosttest/intel_probe.c:809` behind an `if (unsafe)` guard. (The first draft
  said one, which hid the fact that the two form a chain whose head is dead —
  and that shape is the fix.)
- `intel_read_edid_aux()` (`intel.c:1919`), the checksum-validated wrapper —
  **no caller at all.**
- The `edid*` builtins at `runtime_kernel.c:1077-1084` all go to the GMBUS path,
  not the AUX path.

So zlOS itself still cannot read its panel's EDID. `HANDOFF.md:252`'s "still
missing" is wrong as written — it is written but unreachable, which is a different
fix. Closing the remaining half needs hardware: an `intel_read_edid_aux()` call
plus `sudo ./modeset-run.sh --survey`.

*Source: DISPROAD-11 vs GEN9-S25/I4 vs `HANDOFF.md:252`; xcheck-status-conflict lens.*

### 5.6 Hotplug: the decode exists, nothing can call it, and there is no interrupt path

`intel.c:4886-4960` defines **seven** `intel_hpd_*` functions against measured
firmware values (`grep -nE '^[a-z].*intel_hpd_[a-z_]*\(' kernel/intel.c` → 4886,
4894, 4907, 4917, 4933, 4939, 4956; `:4962` onward is the phase-3 external-DP
comment block, not hotplug decode — the first draft said nine, to 4967). `grep -rn 'intel_hpd_pending'` outside `intel.c` returns nothing; no
builtin is registered. So **`display-roadmap.md`'s "done" is done-as-definition,
`feature-catalogue.md`'s "not-started" is wrong (the code exists), and
`HANDOFF.md:324`'s "zero lines today" is wrong too.** The missing part is
nameable: a builtin, a caller, and a real interrupt path.

*Source: DISPROAD-25 vs FEATCAT-29; xcheck-status-conflict lens.*

### 5.7 `dpll_test.c` — the only harness that programs real display hardware — is built by no script

```
$ grep -n 'dpll' kernel/hosttest/build.sh
(exit 1)
$ grep -rn 'dpll_test' --include='*.sh' --include=Makefile .
(exit 1)
$ ls kernel/hosttest/dpll_test
No such file or directory
$ git ls-files kernel/hosttest/dpll_test.c
kernel/hosttest/dpll_test.c   (tracked)
```

It is the single `.c` in `kernel/hosttest/` that `build.sh` does not mention, and
the previously-committed binary was `git rm --cached`'d. **`HANDOFF.md:29` — and
only it — instructs you to run `sudo ./dpll_test 2 148500`**; following that
instruction as written fails at the shell, because nothing builds the binary.
(`grep -rn '\./dpll_test' --include='*.md' .` returns one site outside this file.
The first draft also cited `HANDOFF.md:198` and `driver-build-order.md:67`; both
say something different — that `dpll_test.c` is the only thing that arms
`intel_link_train_arm`, a claim about the source that §5.2 separately shows is
false, and neither tells you to run anything.) One `gcc` line, modelled on the
`intel_probe` line.

Correcting the record: `gen9-modeset-plan.json` states that `hosttest/build.sh`
builds `modeset_test`, `intel_probe` and `dpll_test`. It builds the first two.
Whether `dpll_test.c` still compiles against merged `intel.c` is **unverifiable
statically**; `gcc -O2 -w -o dpll_test dpll_test.c ../intel.c hoststubs.c`
settles it.

*Source: xcheck-unowned lens.*

### 5.8 The ordered modeset has never executed from zlOS, and one step deviates from the plan deliberately

`HANDOFF.md` is the authority here and it is honest about it: the 34/35-step
sequence lit the ThinkPad's panel on 2026-08-17 from
`hosttest/modeset-run.sh --modeset`, with a clean teardown and no underruns. It
has never run from the kernel. The remaining Phase-0 items — second modeset
(off→on→off→on), link-training retry, X-tiled scanout — are open. The retry
deviation is deliberate and documented: step 40 makes one attempt and fails
loudly, and a correct retry must cycle the port first; worth adding once the
sequence has run once from zlOS, not before.

**Whether any `intel.c` write path works from zlOS on the real ThinkPad is
unverifiable by any gate this project can build** — QEMU has no Gen9 GPU. That is
a stated project limit, not an audit gap. It settles only on the laptop.

Also still open from the plan, all at the level of one to three lines. **The
first draft carried only `S40b` and dropped the two the gen9 audit's own
`next_action` ranks ahead of it; both of those are wrong on *every* run rather
than only on a hard link.** The tree facts below were re-checked here; the
prescribed fixes are the plan's, not this audit's, and were not verified against
Intel documentation.

- **`GEN9-S40g` — enhanced framing forced on mid-training.** `intel_port_enable`
  sets `DP_TP_CTL_ENHANCED_FRAME` from the real `enhanced` argument
  (`intel.c:2203-2205`), and then both training loops pass a hardcoded `1`
  (`intel.c:2353`, `:2407`) while the port is enabled; the stop sequence puts it
  back (`:2465`, `:2467` pass `enhanced`). On a panel that does not support
  enhanced framing this is a live mismatch against DPCD 0x101 b7, which the plan
  lists twice as a hazard (4.3 #8 and #20). Thread `enhanced` through
  `train_clock_recovery()` and `train_channel_eq()` — two signatures, two call
  sites.
- **`GEN9-S40c` — per-lane DPCD, lane-0 transmitter.** `set_drive`
  (`intel.c:2321-2336`) builds a per-lane `v[4]`, writes each lane its own byte
  to `DPCD_TRAINING_LANE0`, then selects the buffer-translation entry from
  `swing[0]`/`pre[0]` alone — and its own comment admits `DDI_BUF_CTL` has one
  entry select for the whole port. If lane 2 asks for more swing than lane 0, the
  sink is told lane 2 got it and the hardware drives lane 0's level everywhere.
  The plan's rule is max-over-lanes, same byte to all lanes.
- **`GEN9-S40b`** — step 40d, clamp `v` against `p`
  (`v = min(v, dp_voltage_max(p))`) so vswing+preemph ≤ 3.
- **C9** — the narrow-vs-wide watermark field encoding, which firmware's own
  values cannot distinguish (`HANDOFF.md:172-174`).

The retry deviation above is deliberate and documented. These four are not.

*Source: GEN9-S40b/S40c/S40g and GEN9-I6; S40g and S40c were dropped by the first
synthesis.*

---

## 6. Exec, the heap, and running programs

### 6.1 The kernel-side interpreter is finished and not linked

```
$ grep -n 'interp' kernel/SOURCES
(no output)
$ wc -l kernel/interp_kernel.c
721 kernel/interp_kernel.c
```

`interp_kernel.c` is complete (`k_memcpy` … `k_atan`, `k_malloc` → `arena_alloc`
at `:191`) and `interp.c` compiles freestanding (commit `b514d97`). Nothing in
the four kernel builds compiles either. `exec.c` stops at `EX_LOADED` and says so
in its own message (`exec.c:229-238`), and never calls `fs_read`, so the bytes are
not even loaded into the arena. **Item 2 of the exec brief — the kernel actually
executing a `.zl` — is unreachable, and the gate cannot see the drop** (§2.3).

Fix is a decision plus mechanics: add `interp_kernel.c` to `kernel/SOURCES`, add
`interp.c` (+ `lexer.c`, `parser.c`) to the four builds under `-DZL_FREESTANDING`,
then have `exec.c` `fs_read` into `arena_alloc` and call `zl_run_program` with
`zi_limit`/`zi_confine` armed.

*Source: EXEC-14.*

### 6.2 The arena is up at boot, and nothing allocates from it

**Corrected 2026-08-19 by the repair pass, and this is the worst error the first
draft made.** It was headed "the heap … has no kernel caller" and prescribed
adding an `arena_init()` on the boot path. There has been one since the arena
landed. Three of the four adversarial reviewers found this independently; it was
inherited from WIAI-19, which contains the same two errors.

```
$ grep -n 'arena_up' freestanding/runtime_kernel.c kernel/kernel.zl
freestanding/runtime_kernel.c:1521:  streq(name, "arena_up") -> arena_init()
kernel/kernel.zl:3850:arena_up()
$ grep -nx 'arena.c' kernel/SOURCES
71:arena.c
$ bash kernel/check-zl-calls.sh | head -1
ok: every kernel.zl call resolves to a builtin or a zl fn
```

`what-is-actually-impossible.md` calls a heap "the single highest-leverage missing
piece" at ~300 lines. `kernel/arena.c` is **311 lines, `kernel/SOURCES:71`**, with
`arena_init` at `:224` and `arena_alloc` at `:259`, compiled into all four builds,
and `kernel.zl:3843-3849` states why `arena_up()` is called at boot rather than
lazily on the first `run`. Eleven `arena_*` builtins are registered at
`runtime_kernel.c:1521-1536`.

**The true claim is one level down: `arena_alloc()` has no kernel caller.** Its
only caller anywhere is `interp_kernel.c:191`, and that file is not in SOURCES
(§6.1). The kernel brings a 16 MiB arena up, prints its address, and never hands
out a byte of it.

Two further corrections to what the first draft said the gate does. `arena.c`
**is** in SOURCES, so `land-gate.sh:65` (`grep -qx "$b" SOURCES && continue`)
skips it before any classification runs — the "host-only (not in the kernel)"
bucket catches `interp_kernel.c`, not `arena.c`. And the decision is not unmade:
`kernel/docs/DECISIONS.md` #E3 (`:224-233`) records deliberately that a bump
allocator with a reset is not a heap and that the boot log's `no heap` clause
stands until the interpreter boxes lists and strings — an argument `kernel.zl`
repeats inline at `:3852-3857`. (The *other two* clauses of that same boot line
are a different matter: see §7.3.)

What is missing is §6.1's change plus a first `arena_alloc()` consumer.

`hosttest/arenatest.c` has **63 static assertion call sites** (`grep -oE '\bok\('
→ 44 and `\bokv\(` → 21, less the two definitions at `:72` and `:78`), three of
them inside `for` loops, so the run-time tally is higher than 63.
`HANDOFF.md:842` reports 62 and **was not re-counted here** — `cd kernel/hosttest
&& ./build.sh && ./arenatest` prints its own `%d checks` line (`arenatest.c:268`)
and settles it. The two reviewers who attacked this number disagreed with each
other (43 vs 63); 63 is the count that survives, because the lower one matched
`ok(` and missed `okv(`.

*Source: WIAI-19, corrected against the tree.*

### 6.3 Shipping a `.zl` that is not built in — blocked

No `.zl` program is placed on any image: `grep -n '\.zl' kernel/mkdisk.sh
kernel/mkiso.sh` returns only `kernel.zl` as the compiler input. This is the exec
brief's own stated proof and it cannot happen until §6.1 closes.

*Source: EXEC-21.*

### 6.4 The scheduler is compiled into every build and reachable only from a demo key

**Added by the repair pass.** `sched.c` appeared exactly once in the first draft
— incidentally, inside a grep output listing which files include `memmap.h`. It
is the largest instance of this document's own headline pattern that the document
did not name.

```
$ grep -nx 'sched.c' kernel/SOURCES
48:sched.c
$ wc -l kernel/sched.c
305 kernel/sched.c
$ grep -n 'sched_' kernel/kernel.zl
1777:        put("    tasks now runnable: ")  print(sched_go())
1785:            put("  switches=")  print(sched_sw())
$ git grep -n 'sched_init' -- kernel/ freestanding/ | grep -v hosttest
freestanding/runtime_kernel.c:321  (extern)
kernel/sched.c:189  (definition)   kernel/sched.c:298  (its only caller)
```

Seven `sched_*` builtins are registered at `runtime_kernel.c:1369-1375`;
`kernel.zl` calls two of them, both inside `if cmd == 43` — the `+` multitasking
demo (`kernel.zl:1771`). `sched_start_demo()` is the only thing that ever calls
`sched_init()`. Nothing on the boot path and nothing in `wm_frame` creates a
task, so the desktop is one thread and always has been.

Unlike `ui.c` (§4.1) this is not a missing bridge — the builtins exist and
`kernel.zl` reaches them. It is a missing *decision*: does zlOS get preemption,
or is `sched.c` a demo that should say so? The decision sits on §6's critical
path, because the exec brief's Item 2 gate is "three scripts, and the desktop
still responsive after each", and that gate cannot exist while the shell and the
compositor are the same thread.

`sched.c` is a model citizen on the other axis — it includes `memmap.h` and
static-asserts its own region, which is exactly what §5.3 asks of `intel.c`.

*Source: dropped by the first synthesis; recovered by refute-completeness.*

---

## 7. Network and browser

The browser is built, compiled and opened at boot (see §12). What is open is
everything around it telling the user otherwise, plus the parts that were never
written.

### 7.1 The shipped browser's home page asserts the absence of a shipped feature

```
$ sed -n '141,144p' kernel/browser.c
"<li><strong>The network.</strong> There is no driver yet, so nothing can "
"be fetched. The header's <code>net up</code> is decorative and always was.</li>\n"
"<li><strong>HTTPS.</strong> Refused, deliberately. There is no cipher in "
"this kernel - only hashes ..."
$ grep -n 'virtio_net.c' kernel/SOURCES
78:virtio_net.c
```

That string is self-loaded at `browser.c:478`, so it is what a user sees. It is
wrong twice. `virtio_net.c` is 763 lines and in SOURCES. And **no hashes are
linked into the kernel either** — a search for sha1/sha256/hmac/aes/md5/chacha
across `kernel/*.c` and `freestanding/*.c` returns three hits, none of them an
implementation: `cpu.c:142 cpu_has_aes()` reads the CPUID AES-NI feature bit and
`runtime_kernel.c:301`/`:1426` export it as the `cpu_aes` builtin. (The first
draft said the search "returns nothing", which is not what the command prints;
the conclusion survives, the sentence did not.) The only hash in the kernel is a
non-cryptographic FNV-1a used for a settings checksum (`settings.c:304`);
`stdlib/sha256.zl` is hosted zl and is not linked into the kernel. The no-HTTPS
*policy* survives on stronger ground; its stated reason does not.

**Two claims live here and only one of them is true. Keep them apart.**

*"No crypto is linked into the kernel"* — **true**, and it is the whole reason
the HTTPS refusal is honest. `crypto.c` is in no `SOURCES` and in no build
script on any ref: checked across all six `refs/wip/*`, every local branch and
every remote branch.

```
$ for r in $(git for-each-ref --format='%(refname)' refs/wip refs/heads refs/remotes); do
    git show "$r:kernel/SOURCES" 2>/dev/null | grep -i crypto; done
(no output — 41 refs)
```

*"No crypto has ever been written here"* — **false, and this document said it.**
An earlier draft of this section asserted that `BROWSER-PROMPT.md` §0 and §5
cited a file that "has never existed in this repo" and instructed the reader to
strike the citation. That was a measurement error, not a finding. The checks
behind it were `git ls-files | grep -i crypto` (working tree and index only) and
`git log --all` — and **neither one sees `refs/wip/*`**. `--all` covers
`refs/heads`, `refs/remotes` and `refs/tags`; nothing else. `kernel/crypto.c` is
on three WIP refs:

```
$ git for-each-ref --format='%(refname)' refs/wip |
    while read r; do git cat-file -e "$r:kernel/crypto.c" 2>/dev/null &&
      echo "$r HAS crypto.c"; done
refs/wip/zl-linux HAS crypto.c
refs/wip/tmp-wtclean HAS crypto.c
refs/wip/tmp-wtw0 HAS crypto.c
$ git cat-file -s refs/wip/zl-linux:kernel/crypto.c
21270
$ git show refs/wip/zl-linux:kernel/crypto.c | wc -l
543
```

543 lines: SHA-1, SHA-256, HMAC-SHA1, HMAC-SHA256, PBKDF2-HMAC-SHA1, AES-128
(encrypt, decrypt, CTR, CMAC) and the IEEE 802.11i PRF. Alongside it,
`kernel/hosttest/cryptotest.c` (246 lines) asserts against published vectors —
FIPS 180-1, FIPS 180-4, RFC 2202, RFC 4231, RFC 6070, FIPS-197 C.1, RFC 4493,
IEEE 802.11i-2004.

**`BROWSER-PROMPT.md`'s "543 lines" is exact. Do not strike it.** What it needs
is the qualifier this section failed to supply: the file is real, vector-tested,
and linked into nothing. §10.1 already had it right — it lists `crypto.c` +
`cryptotest.c` under "lost test surface" — and the two sections contradicted
each other until now.

The same evidence is recorded on `desktop/browser-next` in
`kernel/docs/browser-status.md:405`, "HTTPS: the decision, and the file the audit
said did not exist". The home-page string quoted above is already fixed there
(`acec0f5`).

*Source: xcheck-contradiction XC-04, XC-10; xcheck-status-conflict lens.
Correction 2026-08-19: the negative-existence claim was refuted against
`refs/wip`; the linkage conclusion was re-verified and stands.*

### 7.2 The tray draws "net up" unconditionally

```
$ sed -n '250,252p' kernel/kernel.zl
        label(w - 108 * u, ht, "net", TOP_DIM, T_CAPTION, W_REG)
        fill_rgb(w - 66 * u, (hb - 8 * u) / 2, 8 * u, 8 * u, OK_GRN)
        label(w - 48 * u, ht, "up", TOP_DIM, T_CAPTION, W_REG)
```

No `net_up()` or `net_link()` guard — those appear only at `kernel.zl:2647`/`:2651`
inside the `N` diagnostic. **`HANDOFF.md:715`'s claim that "the header has stopped
drawing net up" is false.** The screen is still dishonest, for a new reason: the
dot is a decoration, not a reading, and on the ThinkPad (e1000e, no virtio-net) it
will be green with no link. `SYSTEM-PROMPT.md` item 7 asked for the lie to be
removed or the driver written; the driver was written and the lie stayed.

**And the honest thing that used to be in that corner was deleted by the merge —
the more actionable half, which the first draft dropped.** `577a01a` ("a clock
that knows what time it is, and a header that stops lying") removed exactly these
lines and put an RTC clock there with a `--:--` fallback. It is an ancestor of
`main`, and the eleven-track merge reverted it:

```
$ git merge-base --is-ancestor 577a01a HEAD && echo ancestor
ancestor
$ git show 577a01a -- kernel/kernel.zl | grep -E '^-.*("net"|"up")'
-        text_aa(w - 108 * u, ht, "net", TOP_DIM)
-        text_aa(w - 48 * u, ht, "up", TOP_DIM)
$ grep -c 'rtc_ch' kernel/kernel.zl
0
$ grep -n '"rtc_ch"' freestanding/runtime_kernel.c
1614:    if (streq(name, "rtc_ch"))     return zl_num((double)rtc_hhmm_byte(...));
```

So `rtc.c` (290 lines, `SOURCES:68`) and its `rtc_ch` glyph builtin are a
complete, registered primitive with **no caller** — this document's headline class,
in this section's own corner. Every other `rtc_*` call in `kernel.zl` is inside
the `;` shell command (`:1920-1938`). The fix is one cherry-picked hunk, not a new
`if`: restore `577a01a`'s `draw_header` block over `kernel.zl:250-252`.

`verify-clock.sh` is in `land-gate.sh`'s boot loop and cannot catch this — it
greps the serial output of the `;` command, never the header. A gate that reads
as clock coverage without being it.

### 7.3 RESOLVED: the boot log says zlfs mounts on demand

**Files-app increment, 2026-08-19.** The line is now
`no heap, zlfs mounts on demand, no scheduler`, and opening Files calls
`nv_setup()` plus `fs_mount()`. The audit evidence below records the stale
state that prompted the correction.

**Added by the repair pass** — third instance of the same class as §7.1 and §7.2,
and the only one that contradicts this document's own §12 row for zlfs.

```
$ sed -n '3858p' kernel/kernel.zl
info_line("no heap, no filesystem, no scheduler")
$ grep -nx 'fs.c' kernel/SOURCES
64:fs.c
$ grep -c 'streq(name, "fs_' freestanding/runtime_kernel.c
22
```

Two of the three clauses are defended and true. `kernel.zl:3852-3857` argues the
heap clause inline and correctly — a bump allocator with no `free()` is not a
heap — and `DECISIONS.md` #E3 records it (§6.2). "No scheduler" is true in the
sense that matters: nothing on the boot path creates a task (§6.4).

The filesystem clause is simply stale. `fs.c` is compiled, 22 `fs_*` builtins are
registered, `kernel.zl:1872`/`:1878` mount zlfs, and `verify-disk.sh` power-cycles
it three times inside the gate loop. **The honest narrower statement, which
neither clause makes: zlfs is not mounted automatically at boot — the mount lives
in the `o` shell command (`kernel.zl:1796`).** So the line should say that, or say
`no heap, no processes`. Neither the inline comment nor `DECISIONS.md` #E3 says
anything about the filesystem clause. One line.

*Source: EXEC-22 — dropped by the first synthesis.*

### 7.4 There is no WiFi and no Bluetooth code, and the plan's ladder is correct

Zero lines of either. `wireless-plan.md` remains the reference and its measured
corrections stand: Bluetooth is not a wall (`8087:0026` on USB, class 224/1/1,
HCI is a published SIG standard, and `xhci.c` already implements all three
endpoint types — ~2,000 lines to "zlOS lists nearby devices"); the iwlwifi blob is
1,406,572 bytes and redistributable; the AR9271 dongle's firmware is 50 KB and
open source (~3,800 lines to "zlOS printed my SSID list"). Shared crypto
primitives (~1,500 lines, "write once, use twice" across WPA2, BT SSP and TLS)
are **unlinked, not unwritten** — an earlier draft of this paragraph said the
estimate "starts from zero, not from a `crypto.c` that does not exist", which
followed §7.1's since-corrected error. It does exist:
`refs/wip/zl-linux:kernel/crypto.c` is 543 vector-tested lines already covering
SHA-1, SHA-256, HMAC, PBKDF2, AES-128/CTR/CMAC and the IEEE 802.11i PRF — the
WPA2 half of "write once, use twice" is done. What is missing is a `SOURCES`
entry, a landing decision, and an entropy source (`crypto.c` has no RNG; this CPU
has `rdrand` and `rdseed`). Re-estimate from 543, not from 0.

The ESP32-over-serial shortcut has both ends of the plug already: `ser_rx()` at
`support.c:173` with a live caller at `input.c:696`, `zl_serial_putc()` used at
`term.c:114`, and `net_link()` at `net.c:75` already injected for virtio at
`runtime_kernel.c:1182`. **Decide AT-mode vs raw-frame first** — an ESP32 in AT
mode exposes sockets, not frames, and would bypass `net.c`/`tcp.c` entirely. Only
the raw-frame path feeds `net_link()`.

*Source: WIRELESS-08/09/13/14/18.*

---

## 8. Storage, memory map and system

### 8.1 `LINE_BUF` and `DISK_SCRATCH` are still the same address, and the ground-truth document says otherwise

> **CORRECTION 2026-08-19 — the collision is fixed; two of the three actions
> below are done.** Found by re-running this section's own `git grep` across
> every ref namespace (the method §15 now requires, after §7.1's
> negative-existence error). `6bb8086` — "fix: the three defects the
> documentation audit found in my own merge", an ancestor of `HEAD` — set
> `DISK_SCRATCH = 0x02040000` and corrected `MERGE-EVIDENCE.md`'s Outcome entry
> in place, marking it "**THIS ENTRY WAS FALSE WHEN WRITTEN**".
>
> ```
> $ grep -n 'LINE_BUF  =\|DISK_SCRATCH =' kernel/kernel.zl
> 848:LINE_BUF  = 0x02030000
> 1269:DISK_SCRATCH = 0x02040000
> $ git merge-base --is-ancestor 6bb8086 HEAD && echo on-HEAD
> on-HEAD
> ```
>
> **What is still open is only the third action.** `check-memmap.sh` still does
> not know the constant — its name list at `:31` is `LINE_BUF LINE_MAX HIST_BUF
> HIST_N` with no `DISK_SCRATCH` — so the sweep that would have caught this class
> automatically remains unwritten (§2.2). The landmine is gone; the detector is
> not. **This entry no longer belongs at #4 in §1; the sweep does, at whatever
> rank a missing detector earns.**
>
> Everything below is preserved as written, because the diagnosis is the reason
> §2.2's sweep is worth building and the merge mechanism it describes is still
> the live hazard.

```
$ grep -n 'LINE_BUF  =\|DISK_SCRATCH =' kernel/kernel.zl
848:LINE_BUF  = 0x02030000
1253:DISK_SCRATCH = 0x02030000
$ git grep -n '0x02040000'
docs/MERGE-EVIDENCE.md:93   (the §2.1 prescription)
docs/MERGE-EVIDENCE.md:318  (the Outcome claim)
    — no source file, on any branch
$ git log --oneline -S'DISK_SCRATCH = 0x0204' -- kernel/kernel.zl
(no output)
```

`MERGE-EVIDENCE.md:315-318`, inside the "Outcome (2026-08-19)" section this audit
was told is load-bearing, says the collision "did not fire, because `quirky-pare`
landed before `exec-track` and the address was set to `0x02040000` at the exec
landing." **Both halves are false.** The string `0x02040000` has never existed in
any source file on any branch; it exists twice, both times in `MERGE-EVIDENCE.md`'s
own prose. Five of the nineteen readers found this independently. `check-memmap.sh`
reports green because `DISK_SCRATCH` is not in its name list (§2.2).

`kernel.zl` contradicts itself seventeen lines apart: `:1236` lists
`0x02030000 LINE_BUF LINE_MAX 200` in a map block, and `:1253` assigns
`DISK_SCRATCH` that same address under a comment asserting it "collides with none
of them" — a comment that enumerates only `FS_DATA` and the old Map-A high-RAM
names (`bg 128 MiB`, `sp 160`) that do not exist on `main`.

**Honest severity, and this is the part no single reader established: the
collision is currently INERT**, because nothing reads `LINE_BUF` — `read_line()`
is its sole consumer and has no caller (§4.11). It is a live *landmine*, not a
live bug, and it arms the instant anyone re-wires the line editor into `term.c`'s
shell. Both `DISK_SCRATCH` writes are inside `fn run_command`
(`kernel.zl:1347`), the ordinary command path. Blast radius if armed: 4 bytes (the
boot counter, `:1897-1901`) or up to `CLIP_MAX` 4096 bytes (the clipboard save at
`:1972`) — and `0x02030000 + 4096` is exactly `HIST_BUF`'s base, so a full
clipboard obliterates `LINE_BUF` and stops one byte short of the history ring.

Three actions, and the first is the important one: **correct
`MERGE-EVIDENCE.md`'s Outcome section in place**, because every downstream reader
was told to trust it. Then set `DISK_SCRATCH = 0x02040000` and delete the stale
Map-A names from its comment. Then write the address sweep (§2.2).

*Source: xcheck-contradiction XC-01; xcheck-unowned lens; PLATFORM-45,
SYSPROMPT-33, DTODO, OVN-B1a, GEN9.*

### 8.2 `arena.c` and `virtio_net.c` hand-copy the high-RAM map, and one copy names a symbol that no longer exists

`memmap.h` exists precisely to kill hand-copied address lists. `arena.c:110-111`
defines `HI_BG` and `RAM_CEILING` as `0x08000000` and static-asserts against them
(`:123-125`); `virtio_net.c:97` defines `NET_CEIL 0x08000000` citing "fb.c: HI_BG".
Neither includes `memmap.h` (`grep -n '#include' kernel/arena.c
kernel/virtio_net.c` → no output from either).

Both are numerically correct today, so this is drift, not a bug. But the drift has
already started: `HI_BG` and `bg_buf` do not exist on `main` (`fb.c:151` records
the region "used to hold `bg_buf` and `sp_buf`"), and `arena.c`'s map comment
cites `fb.c:120` for a symbol that line no longer contains. The predicted failure
is that `HI_BACK` moves in `memmap.h` and `arena.c`'s `_Static_assert` keeps
passing against its stale private copy. Fold this into the same change as §5.3 —
same contract, same reviewer should see all three region owners at once.

*Source: xcheck-unowned lens — no planning document owns this.*

### 8.3 DECIDED: RAM slots are the compatibility path; zlfs is primary

**Files-app increment, 2026-08-19.** The ambiguity is gone. Files exposes zlfs
by name and zlEDIT opened from Files saves through to NVMe. `edit <n>` retains
the ten RAM slots so existing shell workflows do not break; they are explicitly
not the primary store. The boundary and cold-boot gate are in
`kernel/docs/storage-and-files.md`.

zlfs is real, backed by NVMe, with 22 builtins and covered by a gate that
power-cycles the machine (`verify-disk.sh`, which *is* in `land-gate.sh`'s boot
loop). But the ten-slot RAM filesystem it replaced is still there:
`kernel.zl:1255-1256` `fn fs_init() { fill_mem(FS_META, 0, 64) }` and `fn fs_len(slot)`
over `FS_META`/`FS_DATA`, and those addresses are still in `check-memmap.sh`'s
table. Two filesystems, one namespace of shell verbs. Decide which one wins and
delete the other, or the address checker keeps reasoning about a subsystem nobody
uses.

*Source: FEATCAT-18.*

---

## 9. zl, the language

### 9.1 `json_parse` and `json_pretty` abort, and the root cause is pinned

The stdlib plan's own #1 priority, still broken, with a one-line fix identified
and verified by running the interpreter:

```
$ timeout 20 ./interp stdlib/json_parse.zl; echo exit=$?
runtime error: num needs a string or number
exit=1
```

Cause: `stdlib/json_parse.zl:167-169` `fn isList(ilV) { return not (ilV == ilV) }`,
with the comment "A list compared to itself is never equal (interpreter quirk)".
That quirk is gone — `xs=[1,2]  print(xs == xs)` now prints `true` — so `isList()`
returns false for every list and `show()` falls through to `num()` on a list.
`print(type([1,2]))` prints `list`, so the replacement is available today:
`type(ilV) == "list"`, at `json_parse.zl:168` and the identical line at
`json_pretty.zl:15`. Those are the only two uses of the idiom in the stdlib.

*Source: STDLIB-03.*

### 9.2 Native `i64` bitwise ops in `compilel.c` — needs no hardware, and buys zlOS nothing

**Read the second half of that heading before starting. The first draft called
this "the only major item that needs no hardware" and left out the numerator.**
All four kernel builds compile `kernel.zl` through `../compile`, which the
toolchain's own build script labels `[ARCHIVED]`; `compilel`, the
`[SPEED BACKEND]`, is invoked by no script under `kernel/` or `gates/`:

```
$ grep -n '^\.\./compile' kernel/build.sh kernel/build64.sh kernel/buildefi.sh kernel/mkdisk.sh
kernel/buildefi.sh:10   kernel/build64.sh:12   kernel/mkdisk.sh:21   kernel/build.sh:16
$ grep -n 'ARCHIVED\|SPEED BACKEND' build.sh
13:==> compile   (zl -> boxed C -> gcc -> native ELF)   [ARCHIVED]
22:==> compilel  (zl -> LLVM IR -> clang -> native ELF)  [SPEED BACKEND]
$ grep -rn 'compilel' kernel/*.sh gates/*.sh
(no output)
```

So this is the best-scoped item in the *language* and the worst-leveraged item in
the *OS*. Take it because the language matters on its own, or take the
backend-consolidation decision first. **Do not take it because it is the cheapest
thing on the board.**

`HANDOFF.md:410-414` names it as the one item that needs no laptop, no panel, no
hardware access, cannot break a running driver, and is testable on the host —
all true. The rest of the premise is confirmed against the tree: `compilel.c:526`
lists `band/bor/bxor/bnot/shl/shr` in the `NUMS[]` bridge table, so
`builtin_bridge_ty` returns `T_NUM` (double) at `:554` (the first draft cited
`:519`, which is a comment line 35 above) and `:956-1075` emits coerce-to-double, `alloca 2*VALSZ`,
`@zlx_num` box each, `@zlx_call` **by name pointer**, `@zlx_as_num` unbox. There
is no `and i64` / `shl i64` emission anywhere in `compilel.c`.

The dispatch it lands in got *worse* since HANDOFF measured it:
`grep -c 'streq(name,' freestanding/runtime_kernel.c` → **644** (HANDOFF
originally said 309), with `band` at dispatch position 635 of 644
(`runtime_kernel.c:2005`).
HANDOFF's measured 10M-iteration figures — C 7 ms, zl arithmetic 4 ms, zl bitwise
999 ms — were taken against the old chain and before Value16; they have not
been re-taken.

`tests/test_bitwise.zl` (356 lines) exists as the parity oracle, but
`run_tests.sh` cross-checks `tests/*.zl` against the C backend only, never against
`compilel`, so the new arm needs its own check.

*Source: UNBOX-18.*

### 9.3 The `Value` union's new read rule has no test, and the audit it needs was never done

`lang/value-16` shrank `Value` 64 → 16 bytes. The guard rails hold (`compilel.c`'s
`VALSZ` updated, `runtime.c` static-asserts it). The hazard is the *new rule* the
anonymous union creates: reading a non-active member now yields garbage where it
used to yield zero. Every builtin added by another track — browser +299, exec
+321, apps +145, overnight ~27 — must check `a[i].type` before reading `a[i].num`
or `a[i].str`. `MERGE-EVIDENCE.md` §2.7 states plainly: **there is no test for
this; it is a read-every-new-builtin job.** `PLAN_unboxing.md`'s Stage 2 asks for
exactly that audit over `runtime.c` and `freestanding/runtime_kernel.c`, and it
was not done.

**And the audit is not prospective — the part of it that *was* done found the
hazard already realised, in `runtime.c`'s oldest builtins rather than in any new
track's. The first draft dropped that.**

```
$ sed -n '1023,1034p' runtime.c
    if (strcmp(name, "sin") == 0)   { return zl_num(sin(args[0].num)); }
    ...
    if (strcmp(name, "sign") == 0)  { double x=args[0].num; ... }
    (seed randint sin cos tan log exp atan sign gcd: no type guard at all;
     sqrt pow floor ceil round: guard `nargs`, never `.type`)

$ printf 'print(sqrt("hello"))\nprint(sign("hello"))\nprint(floor("abc"))\n' >/tmp/t.zl
$ timeout 15 ./interp /tmp/t.zl
0
0
0
```

`UNBOX-02C` reports the same program through `./compile` + `gcc -O2` +
`runtime.c` giving `2.15586e-155 / 1 / 0` — the string's pointer bits read as a
double, which is exactly the new union rule. **That compiled arm is the unboxing
audit's measurement and was not re-taken here**; re-taking it is one `./compile`
and one `gcc`. The interpreter arm above *was* re-run here and prints what it
shows. So the read-every-builtin job has a known first yield, and the two engines
disagree today on a program a stdlib module could plausibly write. That is the
difference between a chore and a scheduled item.

*Source: `MERGE-EVIDENCE.md` §2.7; UNBOX-02C, UNBOX-02D.*

### 9.4 The stdlib has essentially no callers, and importing parts of it breaks the C backend

Two findings that belong together: name collisions across stdlib modules break the
C backend on import (`STDLIB-15`), four modules silently shadow builtins once
imported (`STDLIB-16`), and the library as a whole has essentially no consumers
(`STDLIB-49`). A library nothing imports has no pressure on it to be correct,
which is how §9.1 survived. `STDLIB_PLAN.md`'s own honest framing is the right
one: imports plus fixing the JSON that is broken today, before anything new.

---

## 10. Work the code needs that no planning document owns

This is the cross-check Lens C output, and it is the part nobody reading the
twenty-one documents would have found — by construction, since the test applied
was *does a planning document ask for it*. Twelve findings from that lens, plus
four the repair pass added (marked below by their sections). All but two are
entered above under their subsystem, because that is where a person will look for
them; they are listed here by pointer so the class is visible as a class. §10.1,
§10.2 and §10.3 are new here and have no home anywhere else.

| Finding | Owned by | Entry |
|---|---|---|
| `fix/pointer-drain` unlanded | the *work* by POINTER-PROMPT.md; the *landing* by nothing | §3.1 |
| `ci/gates-and-agent-brief` unlanded; no CI on main | nothing | §2.5 |
| `LINE_BUF`/`DISK_SCRATCH` live; ground truth records a fix never made | nothing (MERGE-EVIDENCE closes it) | §8.1 |
| Clipboard write path has no caller | nothing | §4.4 |
| Two checkers unwired; `check-memmap.sh` false green by construction | MERGE-EVIDENCE §3 specified it, nobody wrote it | §2.2 |
| `edid_buf` inside `HI_BLUR`; `intel.c` asserts nothing | roadmap names the address, not the contract | §5.3 |
| `read_line`/`hist_*` stranded by the shell inversion | **nothing** | §4.11 |
| `dpll_test.c` built by nothing | nothing | §5.7 |
| `font_big.c`/`icons_rgb.c` dead, gate can't fail on them | nothing | §4.12 |
| `key()` unresolved | MERGE-EVIDENCE lists it, no plan asks for the fix | §5.1 |
| `arena.c`/`virtio_net.c` hand-copied maps | **nothing** | §8.2 |
| 55 files exist only in `refs/wip` | partially; count is wrong | §10.1 |
| Four `T-EXEC-*` ids cited as tracked; `TENSIONS.md` has none | **nothing** | §10.2 |
| `rtc_ch`: a registered, complete clock builtin the merge orphaned | **nothing** | §7.2 |
| The scheduler is compiled and reachable only from a demo key | **nothing** | §6.4 |
| The window fade blends at a clobbered origin | **nothing** | §4.14 |

### 10.1 Fifty-five files exist only in `refs/wip/*` and on no branch

`docs/DOCS-RECONCILE-PROMPT.md:133` says 49. The number is 55, measured by
diffing the union of all six local `refs/wip/*` trees against `main`'s tracked
set. All six refs are already local; no fetch is needed. The composition matters
more than the count, because three of the groups are different kinds of problem:

- **Dangling docs.** `desktop-prior-art.md`, `desktop-toolkit.md`,
  `desktop-build-guide.md`, `intel-graphics-stack.md` and `beyond-the-kernel.md`
  are linked by 3, 5, 7, 1 and 3 tracked `.md` files respectively and exist on no
  branch. Every one of those links is broken today. `beyond-the-kernel.md` is the
  worst case because `HANDOFF.md:391` makes it the authority that overturns a
  design verdict (§13.5), and `HANDOFF.md` is the document everyone is told to
  trust. A dangling link there is worse than no link.
- **Lost test surface.** Seven `probe-*.py` scripts (`main` has 13 of the 20 that
  were written), `crypto.c` + `cryptotest.c`, `zlfmt.c` + `verify_fmt.sh`. The
  last pair arrives free with §2.5's merge — land that first and re-count. Note
  the seven lost probes are referenced by zero tracked `.md` files, so unlike the
  docs, nothing on `main` is currently broken by their absence.
- **Lost teaching material.** The complete 23-file `learn/` course, and a 4-file
  VS Code extension under `editors/vscode-zl` that no document in the repo
  mentions at all. Product decisions nobody has made.

### 10.2 Five filenames are cited by twenty-plus tracked files and none is in the repo

```
$ for d in beyond-the-kernel.md desktop-toolkit.md desktop-build-guide.md \
           desktop-prior-art.md intel-graphics-stack.md MASTER_PLAN.md; do
    printf '%-26s tracked=%s citers=%s\n' "$d" \
      "$(git ls-files | grep -c $d)" "$(git grep -l $d | wc -l)"; done
beyond-the-kernel.md       tracked=0 citers=3
desktop-toolkit.md         tracked=0 citers=5
desktop-build-guide.md     tracked=0 citers=7
desktop-prior-art.md       tracked=0 citers=3
intel-graphics-stack.md    tracked=0 citers=1
MASTER_PLAN.md             tracked=0 citers=14
```

`README.md` and eleven design docs route the reader to `MASTER_PLAN.md`.
`desktop-build-guide.md` and `desktop-toolkit.md` are the two `OVERNIGHT-PROMPT.md`
lists under "READ FIRST". `06ced13` rescued six orphaned docs onto `main`; these
five were not among them. Two of them (`beyond-the-kernel.md`, `desktop-toolkit.md`)
exist in the dangling WIP commit `5557f4a`, which is on no branch.

**Recommendation that removes the whole class:** "a doc cites a file that
`git ls-files` does not have" is one grep, costs nothing, and belongs in
`land-gate.sh`. It would have caught all five — and it would have caught this
document's own first draft, which cited `docs/desktop-platform-run.md` for a file
that lives at `kernel/docs/desktop-platform-run.md` (§13.3, now fixed).

**The grep catches the file-level class and misses a second one that is live
now.** Four kernel docs defer a hazard by saying it is "tracked as `T-EXEC-n` in
`.ultra/TENSIONS.md`", and it is not:

```
$ grep -c 'T-EXEC' .ultra/TENSIONS.md
0
$ grep -rn 'T-EXEC' kernel/docs/*.md
memory-map.md:103 (T-EXEC-1)   :136 (T-EXEC-5)   :192 (T-EXEC-3)
exec-kill-path.md:300 (T-EXEC-8)
```

The cited *file* exists, so a file-existence check passes and the citation still
leads nowhere. `T-EXEC-5` and `T-EXEC-8` are the load-bearing pair —
`memory-map.md:136` declines to fix its hazard *because* it says the tension is
recorded elsewhere. Either copy the four into `.ultra/TENSIONS.md` or repoint the
citations. A second one-line check ("every `T-*` id cited in a doc appears in
`TENSIONS.md`") closes this class too.

*Source: xcheck-unowned lens; xcheck-contradiction XC-09; EXEC-35, dropped by the
first synthesis.*

### 10.3 The merge's own rollback estate, unpruned and miscounted

`MERGE-EVIDENCE.md`'s third and last "Still open" bullet is the only one the
first draft of this document did not carry, and its count is wrong at the source:

```
$ git tag | grep -c '^prelanding/'
15
$ git tag | grep -c '^premerge/'
16
$ sed -n '359p' docs/MERGE-EVIDENCE.md
- The eight `prelanding/*` tags are the rollback points and are pushed. Three
```

Fifteen `prelanding/*` tags plus sixteen `premerge/*` tags are pushed rollback
points for a merge that has now been landed and worked on. Nothing breaks either
way — this is housekeeping, and pruning is a judgment call this audit does not
make — but it is the one still-open item whose stated precondition ("once the
tree has been used for a while") is now met, and correcting `eight` → `15` in
`MERGE-EVIDENCE.md` belongs in the same edit as §8.1's correction to its Outcome
section, since a reader is being sent to that file for both.

*Source: `MERGE-EVIDENCE.md` "Still open" bullet 3 — dropped by the first synthesis.*

---

## 11. Where the documents contradicted each other, and which side the tree supports

Only the conflicts where a reader would otherwise be sent to the wrong fix. In
every row the tree is the referee.

| Question | Side A | Side B | The tree |
|---|---|---|---|
| Does anything arm `lt_armed`? | `wireless-plan.json`: nothing does | four other readers: something does | **B.** `intel.c:4232` inside `intel_bringup_panel()`, reachable from `kernel.zl:1395`. Nothing arms it *automatically at boot* — that is the true, narrower claim (§5.2) |
| Is Phase 0.1 done? | `display-roadmap.md`: row marked done | `what-is-actually-impossible.md:156`: blocked on one missing caller | **Neither.** The caller exists; `key()` three lines later halts the kernel (§5.1) |
| Did `LINE_BUF`/`DISK_SCRATCH` fire? | `MERGE-EVIDENCE.md` Outcome: no, it was fixed | five readers: it is live | **The readers.** No commit ever set `0x02040000` (§8.1) |
| Is the browser worth building, and what gates it? | `feature-catalogue.md`: not worth it, needs a heap | `HANDOFF.md:390`: that call was wrong, the gate is a heap | **Neither reason survives.** Eight browser/network `.c` files are in SOURCES and a browser window opens at boot, and they were built with **no heap** — `fs.c:17`, `html.c:1`, `browser.c:84` say so in their own comments. And `beyond-the-kernel.md`, the doc HANDOFF nominates as the correction, is not in the repo (§10.2) |
| Does `fb3d.c` have a caller? | `GRAPHICS-18`: no caller at all | `OSLAND-01`: full chain, done | **Split by symbol.** The file is reachable; the *tiled* rasterizer is not, deliberately and with the measurement attached (§4.13) |
| Is the toolkit done "through `ui_scroll`"? | `desktop-TODO.md`: yes | `OVERNIGHT-PROMPT.md`: caller-less | **B**, for those two widgets; the rest of the toolkit is used from C (§4.8) |
| Hotplug | `display-roadmap.md`: done | `feature-catalogue.md`: not started | **Neither.** Decode exists, nothing can call it, no interrupt path (§5.6) |
| EDID over AUX | roadmap: done / plan: partial / `HANDOFF.md:252`: still missing | | **All three wrong differently** (§5.5) |
| Is the frame time on screen? | `desktop-TODO.md`: done | `PLATFORM-PROMPT.md`: partial | **Both, of different destinations** — tray yes, System Monitor no (§4.9) |
| How big is zlOS? | `11,374` lines, cited in 8 places incl. on-screen text at `wmshot.c:128` | three readers gave 41,970 / ~27,982+3,995 / 38,176+74,569 | **All four are unusable, and so was the cross-check's own replacement.** The spread is entirely explained by undeclared denominators — a fifth attempt while writing this file produced 84,933 from a differently-quoted pathspec. See the boxed measurement below; use that, with its command attached |
| Does this box have 4 cores? | `CLAUDE.md:57` and six planning docs: 4 | `nproc`: 8 | **8.** Flag but do not change `land-gate.sh:37`'s `loadavg > 4.0` guard (`:36` is the tail of the comment above it) — whether 4.0 is still right on 8 cores is a judgment call, and a conservative guard is not a defect |
| Was `GRAPHICS_PLAN.md` annotated? | `HANDOFF.md:590` and `DECISIONS.md:193`: yes, in place | `git log`: one commit, the add | **No annotation exists.** `grep -ci 'supersed\|stale\|annotat\|zlOS'` over it returns 0. A reader following HANDOFF's pointer opens 136 lines of user32/gdi32/opengl32 FFI with no warning attached |

**The size of zlOS, measured here, with the command that measured it.** Six
different numbers have been asserted for this; the corrective for an unmeasured
number is a measured number *with its denominator*, not a different unmeasured
one.

```
$ git ls-files 'kernel/*.c' 'kernel/*.h' 'kernel/*.S' \
               'freestanding/*.c' 'freestanding/*.h' \
    | grep -vE 'font_big|font_aa|font_sub|font8x16|icons\.c|icons_rgb' \
    | xargs wc -l | tail -1
  43833 total
$ wc -l kernel/kernel.zl
3995 kernel/kernel.zl
$ git ls-files 'kernel/hosttest/*.c' 'kernel/hosttest/*.h' | xargs wc -l | tail -1
 13341 total
```

Denominator, stated: top-level `kernel/` and `freestanding/` C, headers and
assembly; the five generated font and icon tables excluded; host harnesses
excluded and counted separately. **43,833 lines of hand-written C/H/S, plus
3,995 lines of `kernel.zl`, plus 13,341 lines of host harnesses.** Fix the eight
citation sites — including `hosttest/wmshot.c:128`, which draws the stale figure
on screen.

**`kernel/HANDOFF.md` is the most reliable document in this repo and it is stale
on five checkable points.** Stated plainly because a reader who stops there gets
each of these wrong: (a) it carries `~65%` for the northstar figure, a number its
own source retracted to `~35%`; (b) it lists `./exectest-nofs` as a gate with 32
checks and nothing builds that binary — which is a coverage gap, not a doc error
(§2.10); (c) it says `killtest.sh` is 14 cases, and **only HANDOFF's number is
wrong here — the other two figures are the same script counted at two depths and
the tally is settled statically, with no run needed.** `killtest.sh` increments
its `checks` counter at exactly 25 sites (13 `run_case` calls, 6 `run_confined`
calls, 6 unconditional inline increments at `:188, 241, 258, 272, 314, 340`) and
`:358` prints `"$checks cases"`, so it prints **25**, matching
`exec-kill-path.md:284`. The first draft filed "19 or 25?" in §14 as needing a
run; it did not; (d) it describes the boot fork as
`if wm_avail()==0 {shell} else {wm_session()}` while `kernel.zl:3782` does
`wm_boot = wm_boot_start()` with `wm_session()` demoted to the `w` command —
`MERGE-EVIDENCE.md:350-353` records `verify-iso.sh` being broken by exactly this
mismatch; (e) it says "fb.c has no clipping" and `fb.c:763-798` is the scissor —
`fb_clip` at `:769`, `fb_clip_none` at `:781`, `fb_clip_get` at `:793` and four
accessors at `:763-766` — with callers in `wm.c`, `ui.c`, `browser.c` and
`term.c`.

**`docs/CODE-MAP.md` contradicts itself six lines apart** and was offered to this
audit as orientation. `:4-6` says the desktop, the apps and the browser "are all
real and all checked in"; `:86-88` says the browser is branch-only and grepping
`main` "finds nothing"; `:116-120` says `main` has no windowed desktop, zero
`wm_*` calls, zero `APP_` constants, and never compiles `wm.c`/`ui.c`/`term.c`.
The first is true; the second and third are false —
`grep -cE '^APP_[A-Z]+ *=' kernel/kernel.zl` → 12, `grep -c 'wm_' kernel/kernel.zl`
→ 174, and `SOURCES:52-54,85` list `term.c`, `wm.c`, `ui.c` and `browser.c`. Its
*layout* advice is confirmed and should be kept. Everything from its "The browser"
heading onward must go. **Because CODE-MAP is orientation text, this outranks any
per-plan staleness.**

---

## 12. Done — and the documents still call it open

This section exists to stop the next session rebuilding finished work. Every row
was verified against the merged tree.

| The documents say | The tree says |
|---|---|
| "Window server / compositor — DESIGNED, unbuilt" (`feature-catalogue.md`) | `wm.c` is 1,582 lines, in SOURCES, and **is the boot state**. `kernel.zl:3782 wm_boot = wm_boot_start()` |
| "Nothing here is started" (`desktop-TODO.md:27`) | In a document whose own banner at `:3` declares the platform queue done, above **27** `### [x]` headings (`grep -c '^### \[x\]'`; 31 `### [` headings in all). The first draft said 29 and did not measure it |
| "Nothing here is implemented" (`desktop-plan.md:7`) | About an inversion that is now the boot state |
| "Clipping / scissor — NO, the keystone gap" (`feature-catalogue.md`) | `fb.c:763-798` is the scissor; `wm.c`, `ui.c`, `browser.c`, `term.c` all use it; `fbbench.c:482-527` asserts it suppresses pixels |
| "Buttons, sliders, toggles, scrollbars — NO" | **Split.** Buttons, sliders and toggles are real and used from C: `ui.c:250`, `:317`, `:286`, called at `settings.c:203, 208, 212, 214, 217, 218` — seven of the widgets defined in `ui.c:239-476` are called from that file, out of 31 `ui_*` calls in it overall (`grep -oE '\bui_[a-z_]+\(' kernel/settings.c \| wc -l`). **Scrollbars are NOT done** — `ui_scroll_begin/end/content` and `ui_list_row` have no caller outside three host harnesses (§4.8). The first draft's "16 call sites" matched no reproducible denominator and closed a widget group this document elsewhere proves open |
| "A real on-disk filesystem — NO, RAM disk, 10 fixed slots" | zlfs (`fs.c`) with superblock, flat directory, contiguous runs on NVMe; 22 builtins; `verify-disk.sh` power-cycles three times and is in the gate loop |
| "Clipboard — NO" | `clip.c` in SOURCES, seven builtins, 104 assertions — but see §4.4, the write path has no caller |
| "TCP/IP stack — NO" | `virtio_net.c` 763 + `net.c` 540 + `dns.c` 433 + `tcp.c` 812 = 2,548 lines, all in SOURCES, with real congestion control |
| "A web browser — NO", "browsers are Unbounded" | `browser.c` 719 + `html.c` 567 + `layout.c` 533, in SOURCES, opened at boot at `kernel.zl:3547`, and built **with no heap** |
| "Mouse events in the same queue as keys — NO, `EV_MOUSE` declared, never pushed" | `input.c:582` pushes it, `wm.c:1492` consumes it, `inputtest.c:174` asserts it |
| "Proportional text layout — NO, monospace cell grid" | Contradicted; see `feature-catalogue`'s own FEATCAT-09 evidence |
| "Maintain four build source lists" (six documents) | **One list.** `kernel/SOURCES`, proven forward by `verify-sources.sh` and backward by `land-gate.sh:55-84`, both in the gate. This one is genuinely *superseded* — the docs were right when written and commit `c7d5bf0` changed the tree |
| `wm_frame_us` defined twice with different types (`MERGE-EVIDENCE` §2.3) | One definition, `wm.c:1514`. The landmine did not survive |
| Two `in_grip` hit-testers | One, `wm.c:1175`. Only the *drawing* half survived (§4.6) |
| `HI_APSTK` missing from `memmap.h` | **Half true, and the first draft overstated it.** Declared at `memmap.h:70`, and `fb.c:177` sizes `BACK_LIMIT` from it — but it carries exactly **one** `_Static_assert` (`fb.c:192`, `AP_STACK_BASE == HI_APSTK`), not three, and `memmap.h`'s own ordering chain at `:90-96` skips it: `HI_BACK < HI_SCHED` jumps straight over the AP stacks. Add `HI_BACK < HI_APSTK` and `HI_APSTK < HI_SCHED` to the chain. Overstating coverage on this row is the exact failure `memmap.h:66-69` warns about |

---

## 13. Not open, and here is why

### 13.1 The static desktop

`desktop/apps-in-windows`' static-desktop mouse loop (182 lines) survived into
`main`'s text shell during the merge, calling functions over globals that do not
exist here, and was removed. The trunk is `wm_boot_start()` plus
`while wm_run() == 1 { wm_frame() }`, and `kernel.zl:3986-3991` says so
explicitly. `wm_session()` survives only as the `w` command. Nothing to build; if
a document sends you looking for an `if/else` at the bottom of `kernel.zl`,
correct the document.

### 13.2 `bg_snapshot` / `bg_restore` / `grab` / `stamp` drag

Deleted deliberately at C4, and the deletion is the reason the compositor is
fast. `bg_buf` and `sp_buf` at 128 and 160 MiB went away, the back buffer moved
into their 48 MiB and now covers 3840×2160, and a whole-desktop redraw went
44 ms → 9.71 ms with scene hashes byte-identical at every mode
(`HANDOFF.md` v10 item 2). `MERGE-EVIDENCE.md` §2.8 records that the builtins were
removed and any surviving `kernel.zl` call site would fail at runtime;
`check-zl-calls.sh` now reports zero such calls. **This is closed. Do not restore
it, and do not treat `desktop-polish-and-speed.md:144,157`'s
`bg_snapshot returns in 0.00 ms because bg_ok = 0` rows as a defect** — that is
the measurement of a thing that was then removed. (The first draft attributed
those rows to `fbbench`; `grep -c 'bg_snapshot' kernel/hosttest/fbbench.c` → 0,
which is the entry's own point.)

### 13.3 The full-screen demos

Every demo is an app in a window now: no `while` loop, no "press any key". Typing
`snake`, `paint`, `cube`, `anim`, `mouse` or `edit` opens one, and
`probe-apps.py` boots five at once. The full account is
`kernel/docs/desktop-platform-run.md` (the first draft of this file cited
`docs/desktop-platform-run.md`, which does not exist — §10.2's own recommended
gate would have caught it, and now says so). Any document describing a demo that
takes over the screen is describing the previous design.

### 13.4 `docs/INTEGRATION-PLAN.md`

**Superseded by `docs/MERGE-EVIDENCE.md`.** It was written against the wrong base
and its headline figure is wrong — it says 332 commits were at risk; the whole
repo is 186 (219 at `ff27d57`), and the document's own later section says 69. It
carries no superseded banner of any kind, so a reader who opens it gets a
live-looking plan. Add the banner or delete the file; `MERGE-EVIDENCE.md:4-6`
claims the supersession one-directionally and the claim is invisible from inside
the file being superseded. One line.

### 13.5 "A browser needs a heap" — retired as a reason, in both directions

`feature-catalogue.md` said a browser, a network stack and an attribute-indexed
filesystem each need a heap and were therefore not worth taking.
`HANDOFF.md:390-393` said that call was wrong and "the gate is a heap (~300
lines)". **Both reasons are refuted by the same evidence:** all eight files
shipped, and they shipped with no heap, and their own comments say so. The heap
question (§6.2) is still live for *other* reasons — the interpreter, other
languages, ELF loading — but it never gated the browser and should stop being
cited as though it did.

---

## 14. Unverifiable — and the command that settles each

Listed so nobody promotes one of these to a fact.

| Question | What settles it |
|---|---|
| Do `fix/pointer-drain`'s 26 assertions pass against merged `main`? | `git merge --no-commit --no-ff fix/pointer-drain`, then build and run `hosttest/xhcitest` |
| Do the six CI workflows pass against merged `main`? | A push, or `act` |
| Does the browser actually fetch `http://example.com/`? (`browser-status.md:26`) | A QEMU boot with `-netdev user,id=n0 -device virtio-net-pci,netdev=n0` |
| The "1.07 MB whole kernel" figure (`HANDOFF.md:597`, `os-landscape.md:271`) | `./build.sh && stat -c%s kernel/kernel.elf`. The untracked `kernel.elf` on disk is 1,571,676 bytes but is a build artefact of unknown provenance |
| Is the v10 wallpaper five passes or six, 100 ms or 130 ms? | Build and run `hosttest/fbbench`, read its wallpaper row. Both existing numbers are estimates taken **before** the wallpaper cache landed; neither was re-taken |
| Does deleting `wm.c:805-815` (§4.6) look right? | Build and run `hosttest/wmshot`, look at `wmshot.ppm` |
| Does the on-screen frame time agree with `fbbench`? | Build and run `fbbench`, compare against a booted `probe-frame.py` |
| Does `dpll_test.c` still compile against merged `intel.c`? | `cd kernel/hosttest && gcc -O2 -w -o dpll_test dpll_test.c ../intel.c hoststubs.c` |
| Does any `intel.c` write path work from zlOS on the real ThinkPad? | **No gate can cover this** — QEMU has no Gen9 GPU, and it is hardware-damage-adjacent. A stated project limit, not an audit gap |
| Is QEMU's default guest really 128 MiB on this host? | A QMP `query-memory-size-summary` on a `verify.sh`, `verify-raw.sh` or `verify-iso.sh` boot — the three booting gates that pass no `-m`. Carried from HANDOFF as measured; not re-measured here. **Which scripts carry `-m` is *not* on this list** — `grep -l '\-m 512' kernel/verify*.sh` settles it and did (§2.8) |

---

## 15. The weakest link of this document

**§8.1's severity call.** Everything about the `LINE_BUF`/`DISK_SCRATCH` entry is
mechanically verified except the sentence that matters most to a reader deciding
whether to drop everything: *the collision is currently inert because nothing
reads `LINE_BUF`.* That rests on one grep showing `read_line()` has no call site
in `kernel.zl` — and `kernel.zl` is 3,995 lines compiled by a language with no
linker, no type checker and no static asserts, which is the exact property that
let every other silent collision in this repo survive. If any path reaches
`LINE_BUF` that a `grep -n 'read_line'` does not show — an indirect write, a
`fill_mem` over a computed address, a builtin holding the constant — then this is
not a landmine, it is live memory corruption on the ordinary command path, and it
belongs at the very top of §1 rather than fourth.

The honest thing to do about it is also the cheapest: **§8.1's one-line fix costs
nothing and removes the need to be right about the severity.** Do it before
investigating whether the severity call was correct.

**What the repair pass did not fix, said before the runners-up because it is
newer information.** Four reviewers found 44 defects in a document that carried a
command for nearly every claim. That rate — roughly one defect per thirty lines,
in evidenced prose — is the honest estimate of what a *fifth* reviewer would
find, and the classes it found are not exhausted:

- **Inherited numbers are the recurring failure.** Every high-severity defect
  except §4.14 was a figure or a claim copied from `HANDOFF.md`,
  `POINTER-PROMPT.md`, `MERGE-EVIDENCE.md` or an audit item, restated in this
  document's voice, and never re-run: `-m` (§2.8), the arena (§6.2), the swapped
  xHCI names (§3.1), `62` assertions (§6.2), `usb-tablet` (§3.4). **A quoted
  command is not a run command.** Where an entry says "carried from", assume it
  was not re-checked unless it also says it was.
- **Dropped items, not wrong items, were the second class.** Six of the entries
  now in this file were graded by the nineteen readers and lost in the synthesis
  — including a statically provable compositor bug (§4.14) and a whole compiled
  subsystem (§6.4). Nothing in the process that produced this document would
  have caught that; only a reviewer with the audit JSONs in hand did. **The
  22 audit files under `scratchpad/audit/` are the check on this file, and they
  are not in the repo.**
- **Counts in this file are now measured but thinly.** Where a number appears
  without a command beside it, it is still inherited. §11's boxed measurement,
  §2.2's twelve, §6.2's sixty-three, §12's twenty-seven and §10.3's fifteen each
  carry theirs. Others do not.
- **A third class, found after the repair pass: negative existence proved with a
  command that does not cover the whole repository.** §7.1 asserted that
  `kernel/crypto.c` had "never existed in this repo" and told the reader to
  strike a correct citation. The evidence was `git ls-files` (working tree and
  index) and `git log --all` — and **`--all` is not all**: it expands to
  `refs/heads`, `refs/remotes` and `refs/tags`, and this repo keeps six refs
  under `refs/wip/*`. The file was on three of them, 543 lines, vector-tested.
  §10.1 had it right the whole time, from a different method, and the two
  sections sat contradicting each other. This is the same structural blind spot
  as `fix/pointer-drain` below, and it is more dangerous than a wrong number: a
  wrong "does not exist" gets a citation deleted and the work re-done. **Before
  writing that something does not exist, enumerate with
  `git for-each-ref` over every namespace, not `--all`.** §7.1 and §7.4 are
  corrected.

  **Re-running that enumeration against this file's one other never-existed
  claim immediately found a second stale entry — a different failure, same
  command.** §8.1 and §11 assert that `0x02040000` "has never existed in any
  source file on any branch". It is now at `main:kernel/kernel.zl:1269`, landed
  by `6bb8086`, which is an ancestor of `HEAD`. That claim was *true when
  written* and the tree moved under it, so this is staleness rather than a
  measurement error — but the same one-line sweep catches both, which is the
  argument for putting it in the gate. §8.1 now carries a correction block;
  **§1's item 4 and §11's `LINE_BUF`/`DISK_SCRATCH` table row still describe the
  collision as live and have not been re-ranked.** Do that before anyone plans
  work off §1.

Two runners-up, said out loud rather than buried:

- **The top-five ordering is a judgment, not a measurement.** Leverage was
  estimated, not timed. Nobody measured how long any of these take. If §2.1's
  "six lines" turns out to be sixty because the harnesses have not been run since
  the merge and several now fail, its position at #1 is still right — a gate that
  goes red immediately is the point — but the *sequencing* argument behind #2
  through #5 would change.
- **This document inherits the readers' reach.** Nineteen agents read nineteen
  documents against one commit. Three lenses crossed them. Where all twenty-two
  looked in the same place, a shared blind spot stays a blind spot, and the
  `fix/pointer-drain` branch — invisible to all nineteen readers for a structural
  reason — is the proof that such blind spots exist and are not hypothetical.
