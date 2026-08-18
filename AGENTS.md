# zl-linux / zlOS — brief for AI agents

Read by **Codex** and **Cursor** natively. Claude Code reads `CLAUDE.md`, which has
the full hazard list — **read that too before changing kernel code.**

This file exists for one reason: **no model has been trained on zl.** There are
291,626 lines of it here and none of it was in anyone's training data. zl *looks*
like Python, which is the trap — a model that pattern-matches it to Python or C
will be confidently wrong. That has already happened.

---

## 1. zl in one breath

No declaration keyword. No type annotations. Blocks in `{ }` with no significant
indentation. No parens required around conditions. Word operators.

```
# comment to end of line
x = 5                       # no keyword, assignment creates. every number is a double
name = "Zac"
nums = [1, 2, 3]            # list, mutable, mixed types
print(f"{name} is {x * 2}") # f-string: {expr} is real code

fn add(a, b) {              # fn, not def / function
    return a + b
}

if x > 3 and not done { ... }     # and / or / not, never && || !
```

**The executable spec is `examples/syntax_tour.zl`.** `docs/REFERENCE.md` names it
the tiebreaker: if the docs and the language disagree, that file wins, because it
either runs or it does not.

```bash
./interp examples/syntax_tour.zl
```

## 2. The scoping rule — where models get it wrong

This is the single most common wrong finding about zl. Get it right:

- **Only parameters and `for`-loop variables are frame-scoped.**
- **Recursion works.** Each call gets fresh parameter slots.
- **A plain assignment inside a function writes the GLOBAL of that name, when one
  exists.** This is deliberate, documented, and the self-hosted compiler depends on
  it — it's how `compiler.zl` shares a cursor across functions.
- **Two functions' locals do NOT collide with each other.** A call frame's parent
  is the global env, not the caller, so the chain is exactly two links: locals,
  then globals. Only *top-level* names leak in.

Verified by running this, not by reading:

```
fn fact(n) { if n <= 1 { return 1 }  return n * fact(n - 1) }
counter = 100
fn bump() { counter = 7  return 0 }
fn setter() { tmp = 42  return 0 }
fn reader() { tmp = 1  setter()  return tmp }

fact(5)   → 120     recursion works
counter   → 7       the local assignment wrote the global
reader()  → 1       setter's tmp did not touch reader's tmp
```

`docs/design/design_scoping_decision.md` records a decision to change this
(assignment binds locally, explicit `global` opts in). **It is decided, not
landed.** Describe the behaviour above, which is what the tree does today.

## 3. "Just run it" is ambiguous — the engines disagree

There are several execution engines and **they do not all implement the same
language.** Measured 2026-08-18 on this 7-line program:

```
counter = 100
fn bump() { counter = 7  return 0 }
bump()
print(counter)
```

| engine | result |
|---|---|
| `./interp` (ground truth) | **7** |
| `./compile` → gcc (C backend) | **7** |
| `./compilel` → LLVM (unboxed backend) | **100** |

The LLVM backend gives every name a function assigns its own slot — it is already
on the other side of the scoping decision. `run_tests.sh` only smoke-tests
`compilel`; it never cross-checks it against the interpreter, which is why this
survives.

**Consequences for you:**

- **`./interp` is ground truth.** When engines disagree, the interpreter defines
  the language.
- `compilel` and `nativegen` accept an **integer-only subset** — no strings, no
  lists, no f-strings. Rejecting those is documented behaviour, not a bug.
- Before filing any finding about language semantics, **run it through `./interp`
  first.** If you did not run it, say so explicitly in the finding.

## 4. Hard rules for this repo

- **Never put a pointer through `unsigned long` in the EFI build.** `buildefi.sh`
  targets LLP64 where it is 4 bytes. This has bitten twice — as a struct field and
  as a cast. Use `unsigned long long`. Full write-up in `CLAUDE.md`.
- **"The code exists" is not "the code works."** Check for an actual caller before
  believing a function runs. Most of `kernel/intel.c`'s write paths are gated
  behind `lt_armed` and have never executed.
- **Gates must never wait on wall-clock time.** Poll for the expected marker. A
  fixed `timeout` turns host load into a false regression, and that has cost a
  bisect here before.
- **Build outputs do not belong in git** — no `kernel/_gen*.c`, no binaries.
- **The formatter re-indents; it does not reformat.** `zlfmt` copies every byte
  except leading/trailing whitespace. Do not turn it into an AST pretty-printer:
  the lexer discards comments and truncates tokens at 128 bytes, so a rebuild-from-
  tokens formatter deletes every comment in the corpus.

## 5. Never touch these from an agent

- **`kernel/intel.c` panel power.** Violating the 500 ms T12 delay or driving AUX
  into an unpowered panel **can damage real hardware** — this drives the physical
  panel on the ThinkPad test laptop, not an emulator. "Try it and see" is not
  acceptable here.
- **The ThinkPad boot path.** Physical machine, no serial port, the screen is the
  only diagnostic.

## 6. Gates

Fast, no emulator — run these freely:

```bash
./run_tests.sh          # interpreter vs C backend vs native, byte-identical
./verify_fmt.sh         # token stream identical before and after formatting
tools/hazard-scan.sh    # greps for the recorded bug classes above
```

Slow, boots QEMU — prefer letting CI run these:

```bash
kernel/verify.sh        # 32-bit BIOS vs a golden serial transcript
kernel/verify-raw.sh    # our own bootloader
kernel/verify-iso.sh    # GRUB, BIOS and UEFI
kernel/verify-efi.sh    # zlOS as its OWN UEFI app — the path real hardware takes
```

**`verify-efi.sh` is the one that matters most and the one most often skipped.**
Three gates were once green while the 64-bit build was dead, because nothing
exercised it. See `.github/workflows/boot.yml` — CI now runs all four every time.

## 7. Orientation docs, in reading order

1. `CLAUDE.md` — hazards, the EFI pointer bug, gate timing rules
2. `kernel/HANDOFF.md` — kept honest: records what is *verified*, not intended
3. `docs/REFERENCE.md` — the language reference
4. `docs/design/ci-and-agent-pipeline.md` — why CI exists and what is still design-only
5. `kernel/docs/input-stack.md` — the USB/HID traps
6. `kernel/docs/thinkpad-first-boot.md` — booting the real laptop

## 8. Reporting findings

This project's recurring failure is **green gates that don't exercise the real
path** — three green gates while the 64-bit boot was dead; 63 passing assertions
over 6 data-loss bugs; 86 defects in code no gate executes.

So: reading code is a legitimate and valuable form of verification here, because
large parts of this kernel cannot be booted in an emulator. But label it.

- Say what you **ran** vs what you **read**.
- Give the file and line.
- If a finding is about language semantics and you did not run `./interp`, say so.
