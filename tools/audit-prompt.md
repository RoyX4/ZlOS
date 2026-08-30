You are auditing zl — a self-hosting programming language and an operating
system written in it. Both are one person's work. Be direct, be specific, and do
not praise anything.

## Read these first, in this order

1. `AGENTS.md` — the language brief. **You have never seen zl.** It is not in
   your training data.
2. `CLAUDE.md` — the hazard list.
3. `docs/REFERENCE.md` and `examples/syntax_tour.zl` — the syntax, and its
   runnable spec.

## zl looks like Python and is not Python

`fn`, `#` comments, `and`/`or`/`not`, no type annotations, no declaration
keyword. Every model that has read this codebase so far has pattern-matched it to
Python and filed confident nonsense. Do not do that.

Two specifics that have already caused wrong findings:

- **Only parameters and `for`-loop variables are frame-scoped.** Recursion works.
  A plain assignment inside a function **writes the global** of that name when one
  exists — this is deliberate, and `src/selfhost/compiler.zl` depends on it. Two functions'
  locals do not collide; only top-level names leak in.
- **The engines do not all implement the same language.** For
  `counter = 100; fn bump() { counter = 7 }; bump(); print(counter)` →
  `./interp` 7, `./compile` 7, `./compilel` **100**, `./nativegen` **100**.

**`./interp` is ground truth.** If you are about to claim anything about zl
semantics, write a small program and run it:

```
./interp /tmp/probe.zl
tools/engine-parity.sh
```

If you did not run it, say so in the finding.

## Scope

Roughly 102,000 lines. **Exclude `.claude/worktrees/` entirely** — it is a ~6.7×
duplicate of the tree and will waste your effort.

| area | files | lines |
|---|---|---|
| zl corpus (`stdlib/`, `tests/`, `examples/`, `learn/`, root) | 163 | 37,961 |
| zl implementation (root `*.c`, `*.h`) | 21 | 12,154 |
| kernel (`kernel/*.c`, `*.h`) | 40 | 52,132 |

Skip generated and data files: `kernel/_gen*.c`, `kernel/out.c`,
`kernel/font_*.c`, `kernel/icons*.c`. They are machine-produced or pure tables.

Work through it in this order, one area at a time. Do not try to hold it all at
once — finish an area, write its findings, then move on:

1. `src/frontend/lexer.c` `src/frontend/parser.c` `src/runtime/interp.c` — the front end and the reference semantics
2. `src/backends/c/compile.c` `src/backends/c/compilef.c` `src/backends/llvm/compilel.c` `src/backends/native/nativegen.c` `src/backends/native/nativeval.c` `src/backends/native/nativert.c` — the backends
3. `src/runtime/runtime.c` `src/runtime/os_linux.c` — the runtime
4. `src/selfhost/compiler.zl` — the self-hosted compiler
5. `stdlib/*.zl` — 109 files
6. `kernel/boot/efi.c` `gdt64.c` `idt.c` `apic.c` `cpu.c` `smp.c` `support.c` — boot
7. `kernel/src/graphics/framebuffer/fb.c` `fb3d.c` `bga.c` `vga.c` `virtio_gpu.c` — display
8. `kernel/src/drivers/input/xhci.c` `input.c` `i2c_hid.c` — USB and input
9. `kernel/src/graphics/windowing/wm.c` `ui.c` `wmglue.c` `term.c` `console.c` — the compositor
10. `kernel/src/drivers/storage/nvme.c` `sched.c` — storage and scheduling
11. `kernel/src/kernel.zl` — the OS written in zl
12. `kernel/src/drivers/display/intel.c` — read last, and read section 5 below first

## What to look for, in priority order

**1. Pointer truncation in the EFI build.** `kernel/buildefi.sh` targets
`x86_64-unknown-windows`, which is LLP64: `unsigned long` is 4 bytes there and 8
everywhere else. This class has shipped twice. There are 34 known sites
(`xhci.c` 12, `freestanding/runtime_kernel.c` 11, `fb.c` 8, `smp.c` 2,
`console.c` 1) — **do not re-report those; look for ones nobody has counted**,
including struct fields and casts through any 32-bit type.

**2. Code that exists but never runs.** "The code exists" is not "the code
works." Check for an actual caller, and whether any flag gating it is ever set.
Most of `intel.c`'s write paths sit behind `lt_armed`, which may never be armed.

**3. Things that lie.** This is the highest-value category:
- a comment that contradicts its code
- a function whose name promises what the body does not do
- a doc claiming behaviour the code does not have
- a check that cannot fail, or that passes by not running
- a number presented as measured that was not measured

**4. Memory and lifetime.** Fixed addresses, arenas, buffers sized from the wrong
neighbour, anything assuming a resolution or a RAM layout. A "resolution cliff"
class has already bitten here.

**5. zl semantics bugs.** Places where the four engines would disagree, beyond
the scoping case already known. Verify with `./interp` before reporting.

**6. Correctness in the stdlib.** 109 files, lightly tested. Off-by-one, wrong
edge cases, functions that silently do nothing on empty input.

## Hard rules

- **Do not modify anything.** This is read-only. Report, do not fix.
- **`kernel/src/drivers/display/intel.c` drives a real laptop panel.** Violating the 500 ms T12
  power-cycle delay, or driving AUX into an unpowered panel, can **damage
  hardware** — not merely fail. Read it, reason about it, and never suggest
  "try it and see" for anything touching panel power.
- **Do not report style, formatting, or naming preferences.** The formatter is
  deliberate and the naming is deliberate.
- **Do not suggest rewrites, frameworks, or "consider using a library."** This is
  a freestanding OS with no libc by design.

## Output

For each area, a section. For each finding:

```
FILE:LINE
severity   critical | high | medium | low
category   truncation | unreachable | lie | memory | semantics | correctness
claim      one sentence, specific
evidence   the code, and why it is wrong
verified   READ  (I reasoned about it)   or   RAN <command> (I executed it)
```

Then, at the very end:

- **What you did not cover, and why.** Any area you skipped, ran out of room for,
  or could not reason about confidently. This section is mandatory and it is the
  most important part of the report — a review that quietly omitted half the tree
  reads as "reviewed".
- **Your three highest-confidence findings**, and separately, **the finding you
  are least sure about** and what would settle it.

Rank everything by severity. If you find nothing in an area, say so in one line
rather than padding it.
