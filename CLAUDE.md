# zl-linux / zlOS — working notes for agents

## Read first, before changing anything

Two docs written for specific traps, both worth reading before touching their
area:

- `kernel/docs/input-stack.md` — the keyboard, the pointer, and the three ways a
  USB stack written for ONE HID device breaks when a second one arrives. Also
  the `-mgeneral-regs-only` boundary rule, which is subtler than it looks.
- `kernel/docs/thinkpad-first-boot.md` — the step-by-step for booting the
  laptop, including the two things that waste an hour (Secure Boot, and the fact
  that there is no serial port so the screen is the only diagnostic).
- `kernel/docs/overnight-2026-08-18.md` — one page covering the descriptor-pointer
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
including the four that name this bug class precisely. They are now re-enabled
after `-w` and fatal:

```
-Werror=shift-count-overflow  -Werror=void-pointer-to-int-cast
-Werror=pointer-to-int-cast   -Werror=int-to-pointer-cast
```

Clang applies flags left to right, so these must stay *after* the `-w`. The
whole 28-file EFI build is clean under them; a reintroduction now fails the
build instead of failing on the laptop.


`kernel/HANDOFF.md` is the orientation doc and it is kept honest — it records what
is *verified* rather than what is intended. Read it before touching `kernel/`.
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
