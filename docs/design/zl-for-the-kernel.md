# Writing the kernel in zl — what it actually costs

**Written 2026-08-18**, answering: *"in three months, if I make zl as good as C,
how hard would it be to change everything over?"*

**Reads on top of** [`PLAN_unboxing.md`](PLAN_unboxing.md) — which is the staged
implementation plan for the language half and was already written on 2026-08-02.
This document is the *kernel* half: what the port costs once that plan lands,
what it does **not** cover, and which numbers in it have since gone stale.

---

## 0. The answer in one line

**The changeover is the cheap part. The language work is the whole job — and
most of it is already designed.**

---

## 1. Where the kernel stands

| | lines |
|---|---|
| `kernel.zl` | 2,333 |
| hand-written C in `kernel/` | 16,906 |

Excludes generated `_gen*.c` / `out.c` and the font/icon data tables. So zlOS is
**~12% zl**. The largest C files are `intel.c` 5084, `fb.c` 2211, `xhci.c` 2009,
`wm.c` 985, `crypto.c` 543, `virtio_gpu.c` 495, `console.c` 437, `input.c` 434,
`ui.c` 417, `apic.c` 397, `idt.c` 394.

---

## 2. Why the changeover itself is cheap

**zl compiles to C.** `kernel/build.sh:16` runs `../compile kernel.zl` and copies
`out.c` to `_gen.c`, which is compiled and linked like any other translation
unit. A file that moves from hand-written C to zl still arrives at the linker as
C. The linker cannot tell the difference.

Three consequences:

1. **Port file by file, never big-bang.** Each file is an independent commit with
   its own gate. There is no flag day and no half-ported state that fails to link.
2. **The C↔zl seam is already proven in both directions.** `wmglue.c` binds zl's
   `app_draw` into the compositor through **weak symbols**, so the kernel links
   whether or not the zl side defines it. `runtime_kernel.c` exposes C to zl as
   builtins. Neither direction is speculative — both ship today.
3. **Calls stay ordinary calls.** `fn app_draw(...)` becomes
   `Value zl_fn_app_draw(Value, ...)`. No marshalling, no ABI boundary, no
   trampoline.

**The one real friction is the build scripts.** There are four — `build.sh`,
`build64.sh`, `buildefi.sh`, `mkdisk.sh` — each with its own source list, and
adding a `.c` has broken the UEFI and raw builds **three separate times**
(`font_prop.c`, then `wm/ui/wmglue`, then `term.c`) because `verify.sh` only
sees `build.sh`'s list. Every file that moves touches all four. That is an
annoyance with a known shape, not a design problem.

---

## 3. What actually blocks it, measured

Not "zl has no structs". No structs is a *symptom*. The barrier is that **zl has
no representation for a machine integer.**

### 3.1 Every value is a 64-byte struct

`src/runtime/runtime.c:35` enforces it:

```c
typedef char zl_value_is_64_bytes[(sizeof(Value) == 64) ? 1 : -1];
```

A C `long` is 8 bytes. A zl number is 64, and it is passed **by value**.

### 3.2 Every operator is a runtime string dispatch

`src/runtime/runtime.c:333` is `Value zl_binop(const char *op, Value l, Value r)`, and its
body is a `strcmp` ladder. The kernel's copy (`runtime_kernel.c:505`) is cheaper
— it compares `op[0]`/`op[1]` directly — but it is still a call per operator,
still on 64-byte structs, and still converts through `double` on every single
arithmetic operation:

```c
long long x = (long long)a.num, y = (long long)b.num;
```

### 3.3 The number

`fn blend(a, b, t) { return a + (b - a) * t / 255 }` — the innermost operation of
a renderer — compiles to:

```c
Value zl_fn_blend(Value v_a, Value v_b, Value v_t) {
    return zl_binop("+", v_a, zl_binop("/", zl_binop("*", zl_binop("-", v_b, v_a), v_t), zl_num(255)));
}
```

Instruction counts, `gcc -O2`, x86-64, counted from `objdump`:

| | instructions in the function body | calls |
|---|---|---|
| C `long blend(long, long, long)` | **10** | 0 |
| zl, identical expression | **87** | 5 |

The 87 **excludes** the four `zl_binop` bodies it calls. This is instruction
count, not wall-clock, deliberately: the box was at load average 9.6 with three
QEMUs running when this was measured, and a timing number taken under that load
would be fiction. Instruction count is load-independent.

**This is why `fb.c` is not zl, and it is a much harder blocker than the missing
`struct` keyword.** A pixel loop at 9x instruction count is not slow, it is
unusable.

---

## 4. The plan already exists

[`PLAN_unboxing.md`](PLAN_unboxing.md), 642 lines, 2026-08-02, nine stages. It
independently arrives at the same diagnosis and settles the syntax:

```
fn area(w: int, h: int) -> int { return w * h }
```

Its stages, and what each is for:

| | stage | what it buys |
|---|---|---|
| 1 | kill the string-keyed operator dispatch | cheaper boxed path, **no language change** |
| 2 | shrink `Value` 48 → 16 bytes | cheaper boxed path, no language change |
| 3 | annotations lex and parse, nothing reads them | syntax lands, zero behaviour change |
| 4 | the type pass, check-only | |
| 5 | `src/backends/llvm/compilel.c` consumes types — unboxed `int`/`bool` | **the actual speed** |
| 6 | the boundary: box/unbox shims and `any` | typed and untyped code interoperate |
| 7 | records | the `struct` unlock — ~23 corpus files |
| 8 | nullables and generics | |
| 9 | annotate `src/selfhost/compiler.zl` | optional, fixpoint risk, last |

Two design decisions in it are load-bearing and worth knowing before touching
anything:

- **The typed side always pays.** Untyped code generates byte-identical output
  before and after every stage. That is a gate, not an aspiration — so the 96
  stdlib modules cannot regress.
- **Per-function type inference is unsound as the language stands**, because
  assigning a name inside a function writes the *global* of that name when one
  exists (`src/runtime/interp.c:173-185`). Any inference pass must start with a
  whole-program global collection. The plan calls this the single most likely
  way to get the feature subtly wrong, and it is right.

### 4.1 zl already has an unboxed backend

`src/backends/llvm/compilel.c` (2,465 lines) emits LLVM IR with a static `T_INT`(i64) /
`T_NUM`(double) lattice and **no boxing**. `PLAN_unboxing.md` §0 records it at
parity with C on four integer benchmarks — 67 ms vs 71, 58 vs 61, 33 vs 33,
62 vs 62 — and `nativegen` with no optimiser at all matching `clang -O2` on the
benchmark loops.

**So "zl cannot go as fast as C" is already false.** What is true is that the
fast backends buy their speed by refusing most of the language.

---

## 5. What the plan does NOT cover — the kernel gap

**Stage 5 makes `src/backends/llvm/compilel.c` consume types. The kernel uses `src/backends/c/compile.c`.**

`kernel/build.sh` runs `../compile` (the boxed C backend). Nothing in
`PLAN_unboxing.md` makes *that* backend unboxed. So landing all nine stages
produces a fast zl and leaves the kernel exactly where it is.

Closing that gap is one of two choices, and it has not been made:

- **(a) Give `src/backends/c/compile.c` the typed path too.** It is only 573 lines with 92 emit
  sites, so this is smaller than it sounds — but it is duplicated work against
  `src/backends/llvm/compilel.c`.
- **(b) Point the kernel build at `compilel` → LLVM IR → clang, freestanding.**
  Cheaper if it works, and the fit is better than it first appears: `compilel`
  supports no lists, and the kernel subset *also* has no lists
  (`runtime_kernel.c`'s `zl_list_n` is a hard fault), so they refuse the same
  things. The open question is whether `compilel` covers everything
  `kernel.zl` actually uses.

**This is the decision to take first**, before any porting, and it is not
recorded anywhere yet.

### 5.1 One stale number, one that resolves in our favour

`PLAN_unboxing.md` was written against the Windows repo (`verify.ps1`,
`os_win.c`, Win64 ABI). Two things changed in the move to Linux:

- **`Value` is 64 bytes now, not 48.** `fnptr` and `fnargs` were added for `V_FN`.
  Stage 2's "48 → 16" is really 64 → 16, so it is worth *more* than written.
- **Stage 2's unverifiable caveat resolves favourably here.** The plan notes that
  16 bytes still would not ride in a register on Win64, which passes aggregates
  in registers only at 1/2/4/8 bytes. **Linux is SysV, which passes a 16-byte
  aggregate in two registers.** Verified by compiling a 16-byte tagged union and
  disassembling the call: no hidden pointer, no stack slot, and the return comes
  back in `rax:rdx`. So on this box Stage 2 removes the memory round-trip
  entirely, which is the thing the plan hedged on.

---

## 6. What stays C or assembly forever

Roughly **1,500–2,000 lines**, and no type system changes this:

- `boot.S`, `boot64.S` — assembly, always
- ISR entry stubs — `__attribute__((interrupt))` is a calling convention, not a
  language feature
- units compiled `-mgeneral-regs-only` (`idt.c`) — a *compiler flag*. An ISR must
  not touch SSE, and every zl number is a double, so the interpreter itself is
  the thing that must not be called from there
- port I/O (`outb`/`inb`) and `lidt`/`lgdt` — instructions, reachable from zl
  only through builtins or a future intrinsic

This is a normal floor. Every OS has one.

---

## 7. Sizing

| | difficulty | why |
|---|---|---|
| the changeover mechanics | **easy** | file-by-file, seam proven, four build scripts to update |
| `PLAN_unboxing.md` stages 1–2 | **small** | `src/runtime/runtime.c`/`src/runtime/runtime.h` only, no language change, kernel benefits immediately |
| stages 3–6 (the typed subset) | **the real work** | a type pass and a typed backend path |
| closing the §5 kernel gap | **unscoped** | decision (a) or (b) not yet taken |
| stage 7 (records) | **the `wm.c` unlock** | window table, event queue, driver register layouts |
| porting ~12,000 lines | **long but boring** | mechanical; gate every file |

**Stages 1 and 2 are worth doing regardless of whether the port ever happens.**
They touch two files, change no language semantics, cannot regress untyped code,
and make the kernel's existing zl faster today. Nothing about them is a bet.

---

## 8. Order, and the trap in it

**The typed subset must land before the port, not alongside it.**

If `fb.c` gets ported to untyped zl "to get started", the result is a renderer at
9x instruction count, and the conclusion drawn will be "zl cannot do this". That
conclusion would be *wrong* but it would be drawn from a *real* measurement,
which is the most expensive kind of mistake available here.

---

## 9. Weakest link in this document

The 12,000-lines-portable figure is arithmetic (16,906 minus the ~2,000-line
machine-level floor minus `intel.c`'s 5,084, which nobody should touch), not an
audit. **No one has gone through the C file by file and asked "could this be
zl".** Until that inventory exists, treat the port sizing as an order of
magnitude, not a number.

The §5 gap is the other soft spot: whether `compilel` covers what `kernel.zl`
uses is stated as an open question because it *is* one — it was not checked.

---

## 10. The run — what actually happened

### Stage 1 — landed, `c69ef4e`

`zl_binop` and `zl_unop` now switch on `op[0]` instead of walking a ladder of
15 `strcmp`s in source order. Signature unchanged, so no caller, no already
generated `.c` and no line of `src/selfhost/compiler.zl` moved.

**Measured, interleaved A/B, best of 7:**

| | before | after | |
|---|---|---|---|
| `b2_arith` | 561 ms | **459 ms** | **1.22x** |
| `b4_list` | 300 ms | 286 ms | 1.05x |
| `b5_string` | 152 ms | 151 ms | 1.01x |

**The plan estimated 1.3–1.6x and it was optimistic.** The disassembly says
why: `gcc -O2` had **already inlined every `strcmp` in both arms** — zero
`strcmp` calls in either object file — so the win is dynamic path length, not
call removal. Static size only moved 480 → 446 instructions.

Gates: `run_tests.sh` ALL GREEN (40 ok, was 39), `verify_fmt.sh` PASS (557
files). New: `tests/test_operators.zl`, 44 checks, one per dispatcher arm.

### 10.1 The measurement harness, and why the first numbers were wrong

`PLAN_unboxing.md` cites `bench\run_bench.ps1` throughout. **That harness does
not exist in this repo** — it was never ported from Windows. `bench/` is new:
`run_bench.sh` replaces it, and `ab.sh` was written after the first attempt
produced a bad number.

**Measuring one arm at a time reported `b4_list` at 12.8% faster while its own
interpreter control moved 12%** — indistinguishable from host load, on a box
that had three other sessions' QEMU instances running. Interleaving the two
builds within the same seconds gives the real figure: **4.7%**. This is the
same failure `CLAUDE.md` records under "Gates must never be timing-
sensitive", arriving from a different direction.

**One correction to the plan's method.** It says the interpreter column is a
free control because `interp` does not link `src/runtime/runtime.c`. That is a Windows
fact and it is false here — `build.sh:11` links it. It is still a valid
control, for a different reason: `src/runtime/interp.c` makes **zero** calls to `zl_binop`
(it has its own `Value` and its own `eval`), so a `zl_binop` change cannot move
that column.

### 10.2 Stage 2 is smaller than it looks, and lands on the kernel

Counted, not estimated — list-field accesses (`items`/`nitems`/`cap`/`tip`) per
file:

| file | sites | consequence |
|---|---|---|
| `src/runtime/runtime.c` | ~160 | the work |
| `src/runtime/interp.c` | ~150 | **none** — it does not include `src/runtime/runtime.h`, it has its own `Value` |
| `src/backends/c/compile.c` | **0** | **generated code is insulated** — it goes through `zl_len_list`/`zl_item`/`zl_index`/`zl_set`, never a field |
| `src/backends/llvm/compilel.c` | 3 | trivial |
| `runtime_kernel.c` | 5 | all inside refusal stubs |

**`src/backends/c/compile.c` at zero is the important one.** It means the layout change cannot
break a single line of already-generated output, which is what makes Stage 2 an
afternoon rather than a week.

**But `src/runtime/runtime.h` is shared with the kernel.** `kernel/build.sh` compiles with
`-I..` and there is no second copy — verified, `src/runtime/runtime.h` does not
exist. So Stage 2 changes the kernel's ABI directly and needs all four boot
gates, not just `run_tests.sh`. That is the point rather than a cost: zlOS runs
on fixed stacks, and a 4x smaller `Value` is a 4x smaller zl stack frame.

### 10.3 The constraint Stage 2 must not break

`push()` is the one place in the runtime where two zl values **deliberately
share mutable storage**, and `src/runtime/runtime.c` records that it has been wrong twice —
appending unconditionally was a use-after-free, copying every time was O(n²).
Tip tracking is the version that is neither, and it splits a list's state:

| | |
|---|---|
| `nitems` | **per value** — how much of the array this value can see |
| `items`, `cap`, `tip` | **shared** — the array, and its high-water mark |

A 16-byte `Value` has to keep that split, which rules out the obvious layout of
allocating the header contiguously with the array: a `realloc` would move the
header, and every aliasing value would hold a stale pointer to it.

`tests/test_list_aliasing.zl` (`a8676e3`) pins it — 26 checks, green on both
engines before any layout change, so it describes current behaviour rather than
an assumption about it.

### 10.4 Stage 2 belongs in a worktree

Other sessions are editing this same checkout — a commit from another one
landed between the two above. A half-applied `src/runtime/runtime.h` change breaks
everyone's build, so this one does not get done in place.

### 10.5 Stage 2 — done, on `lang/value-16`, **deliberately not merged**

`d004ca9` in the worktree `~/Documents/repos/zl-value16`. **`sizeof(Value)` is
16, down from 64.**

**Measured, interleaved A/B of the two whole checkouts (`bench/abtree.sh`),
best of 7, on top of Stage 1:**

| | 64-byte | 16-byte | |
|---|---|---|---|
| `b2_arith` | 554 ms | **354 ms** | **1.56x** |
| `b4_list` | 287 ms | **162 ms** | **1.77x** |
| `b5_string` | 158 ms | 139 ms | 1.14x |

The plan estimated **1.5–2x** here and this one lands inside the band — unlike
Stage 1, which came in under. **Both stages together, against the original
baseline: 1.58x on arithmetic, 1.85x on lists.**

**Two changes, and the second is what makes the first safe.**

1. **The payload becomes two anonymous unions** — `nitems`/`fnargs`, and
   `num`/`str`/`items`/`fnptr`. *Anonymous* so every existing `v.num`,
   `v.str`, `v.items`, `v.nitems` and `v.fnargs` keeps compiling: shrinking
   the struct did not become a rename of several thousand field accesses. The
   pairs are disjoint by type, so no value needs two members of one union at
   once. **The cost:** reading an inactive member is now garbage rather than a
   zero, so every read must be guarded by `type`. The runtime already switched
   on type first everywhere; that stopped being a style preference.

2. **`cap` and `tip` move out of `Value` and in front of the items array.**
   They were never properties of a *value* — they are properties of the
   *array*, and §10.3's tip tracking depends on two aliasing values agreeing
   about them. An 8-byte header at a fixed offset before `items[0]` makes that
   sharing **structural** instead of a convention every copy has to preserve
   by hand, and it drops the separate `malloc` that `int *tip` needed.

   Safe only because **the array never moves under an alias**: `push` never
   reallocs, and the runtime's one `realloc` runs on a list still under
   construction. Every non-push allocation passes `tip = -1`, which is exactly
   what `tip == NULL` used to mean.

**Gates:** `run_tests.sh` ALL GREEN (40 ok) · `kernel/verify.sh` boots and
matches `golden.txt` · **ASan + UBSan** with leaks off: `test_list_aliasing`
26/26, `test_lists` 280/280, no heap-buffer-overflow, no use-after-free, no UB.

The sanitizer run is the one that counts. The header is pointer arithmetic on
either side of an allocation, in the same code that carried a real
use-after-free once.

### 10.6 Why it is not merged — and a problem worth fixing first

**The kernel gates could not run against the committed tree, because HEAD does
not build a kernel at all.**

| | |
|---|---|
| `kernel/src/drivers/input/keycodes.h` | **untracked** |
| `kernel/src/net/crypto.c` | **untracked** — and it is in `build.sh`'s source list |
| 12 kernel/freestanding sources | interdependent **uncommitted** changes |

A fresh clone of this repo cannot build zlOS. The two untracked `.c`/`.h` files
are real source, not build output, and they exist only in one working
directory on one machine — a `git clean` or a disk failure loses them.

The Stage 2 boot gate above ran with those files copied in and the change
re-applied on top, so it proves the layout is compatible with **the tree as it
stands today**, not with the committed tree. That is why the branch stays
unmerged: the gate has to be repeatable before the result means anything.

**First action for whoever picks this up: commit `kernel/src/drivers/input/keycodes.h` and
`kernel/src/net/crypto.c`.** Then re-run `kernel/verify.sh`, `verify-raw.sh`,
`verify-efi.sh` and `verify-iso.sh` on `lang/value-16` and merge.

---

Language plan: [`PLAN_unboxing.md`](PLAN_unboxing.md) · Type syntax:
`design_type_system.md` · Records: `design_memory_structs.md` · Kernel builtins:
`design_kernel.md` · FFI: `design_ffi_syscalls.md` · Kernel orientation:
[`../../kernel/HANDOFF.md`](../../kernel/HANDOFF.md)
