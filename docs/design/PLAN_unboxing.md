> **AUDITED 2026-08-19 · PARTLY OPEN.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. Stages 1 and 2 landed exactly as specified, with measured numbers and named commits. Stages 3–9 are untouched, and every tool they name (`verify.ps1`, `run_bench.ps1`, `build.bat`) does not exist on this Linux tree. Keep it as analysis — its lexer safety proof reproduces verbatim today — and re-base the build plan: `compilel` is not the backend zlOS builds through.
>
> **What is still open from this document is in
> [`docs/STATE-OF-THE-PROJECT.md`](../STATE-OF-THE-PROJECT.md) — read that first, and do not
> work from the task list below.**

# PLAN: Removing Boxing, in Stages

**Status:** implementation plan (the ordering, not the analysis)
**Author:** planning pass, 2026-08-02
**Scope:** `runtime.h`, `runtime.c`, `lexer.c`, `parser.c/.h`, a new type pass,
`compilel.c`, and — last, and only last — `compiler.zl`. No code is changed by
this document.

**Reads on top of:** `design_type_system.md` (the *what*: syntax, type set,
boundary tables, Neutrality) and `design_types.md` (the earlier
checker-shaped proposal it supersedes). This document is the *when and in what
order*, and it corrects two things those docs assert that are no longer true
(§2.1, §8 Stage 3 note).

**Read alongside:** [`zl-for-the-kernel.md`](zl-for-the-kernel.md) — what this
plan costs and buys *for zlOS specifically*. It records two things this document
cannot know: that `Value` is now **64 bytes, not 48** (so Stage 2 is worth more
than estimated here), that Stage 2's unverified Win64 register caveat **resolves
favourably on Linux/SysV**, and that Stage 5 targets `compilel.c` while the
kernel builds through `compile.c` — a gap this plan does not close.

---

## 0. The goal, in one paragraph

zl's compiled output is 4–6x slower than it needs to be, and the reason is not
codegen — `nativegen` with no optimiser at all matches `clang -O2` on the
benchmark loops, and `compilel` is at parity with C on four integer benchmarks
(67ms vs 71, 58 vs 61, 33 vs 33, 62 vs 62). The reason is that every value in a
`compile.c`-generated program is a 48-byte tagged `Value` handed to a
string-keyed dispatcher, and the fast backends only escape that by refusing to
compile most of the language. The fix is *not* one change; it is a sequence in
which the first two steps make the boxed path cheaper without touching the
language at all, the middle steps let a programmer write down a type so the
compiler may stop boxing that specific value, and the last steps widen what can
be written down until the type system covers the corpus instead of covering
four benchmarks. Every stage below ends with `.\verify.ps1` green, and every
stage is sized so one agent finishes it in one sitting — two agents have
already died mid-task in this repo by taking a whole floor at once.

---

## 1. Annotation syntax (settled — restated for reference)

From `design_type_system.md` §2, unchanged:

```
count: int       = 0
ratio: float     = 0.5
name:  str       = "Zac"
ok:    bool      = true
xs:    list[int] = [1, 2, 3]
box:   any       = read_config()      # explicit opt-out

fn area(w: int, h: int) -> int { return w * h }
fn f(n)            { }                # unchanged from today, fully dynamic
fn f(n: int)       { }                # typed param, unstated (= any) return
fn f(n) -> int     { }                # untyped param, typed return
```

Types: `int`, `float`, `bool`, `str`, `list[T]`, `any`. Unannotated is `any`.
`any` is today's boxed `Value`, bit for bit. Assignability is
`A == B  or  A is any  or  B is any` — no widening, no coercion.

### 1.1 The lexing situation, corrected

`design_type_system.md` §2.2 proves `:` is free by quoting a `lexer.c` line that
rejects it. **That proof is stale.** The ternary work has since added `?` and
`:` to the accepted set:

```c
/* lexer.c:252-258 */
/* '?' and ':' are only ever the two halves of a ternary. Neither
   starts a two-character symbol, so there is nothing above to
   shadow: adding them here can only turn text that used to be a
   lexer ERROR into text that lexes. */
if (strchr("(){}[],.+-*/%=!<>?:", c) == NULL) {
    die(line, "I don't know this character:", c);
}
```

So `:` already produces a `T_SYMBOL`. The safety proof has *moved from the lexer
to the parser*, and it still holds. Measured today against the live
`interp.exe`:

```
$ ./interp.exe _tychk.zl            # x: int = 5
line 1: expected a value (got ':')

$ ./interp.exe _tychk2.zl           # fn f(n) -> n { return n }
line 1: expected '{' (got '-')
```

Both are hard parse errors, so no currently-valid program occupies either
position. Nothing can break; but the claim to make in the commit message is
"`:` is a parse error in annotation position", not "`:` does not lex".

`->` is genuinely free and must be added **inside `lex_symbol`, after the
compound-assignment ladder at `lexer.c:246-250`**, not in `next_token`'s
dispatch. The reason is that `->` appears in comments and inside string
literals (`stdlib/brainfuck.zl`), and `next_token` strips comments at
`lexer.c:271-273` and consumes strings at `lexer.c:290` *before* reaching
`return lex_symbol(lx);` at `lexer.c:312`. Put the check anywhere earlier and a
comment starts mangling tokens.

---

## 2. The boxed/unboxed boundary rule

> **The typed side always pays.**

When a value of static type `T` meets a position of type `any`, the compiler
emits `zl_box_T` on the typed side. When an `any` meets a position of type `T`,
it emits a **checked** `zl_unbox_T` — tag load, compare, branch, and on
mismatch `zl_type_error` prints `type error: expected int, got str` and exits.
It never coerces and never substitutes a default.

Two consequences, and they are the whole reason to state the rule this way:

1. **Untyped code cannot regress.** All 96 stdlib modules contain zero
   annotations, are therefore 100% `any`, and generate byte-identical output
   before and after every stage in this plan. That is a gate (§4 Stage 3), not
   an aspiration.
2. **The cost is visible where it was chosen.** The programmer who wrote
   `fn hot(n: int)` is the one who pays for the box at each call from untyped
   code. Someone who annotated nothing pays nothing.

The full site-by-site tables (B1–B7 boxing sites, U1–U6 unboxing sites, the
O(n) `list[T] ↔ any` trap, the fast-builtin table) are `design_type_system.md`
§4.2–§4.6 and are adopted here without change.

### 2.1 Two language-level facts that constrain every stage

**(a) `+` is three-way overloaded and it is a tested guarantee.** `runtime.c`'s
`binop_plus` is `num+num → num`, `list+list → concat`, and *otherwise stringify
both sides and join*. `tests/test_syntax.zl:369-378` locks all of it:

```
s = "n="
s += 5
check("string += number",  s, "n=5")
n = 5
n += "x"
check("number += string",  n, "5x")    # + is a join as soon as one side is text
```

Therefore: **an unboxed `add i64` may only be emitted when *both* operand
static types are `int`** (or both `float`). One `any` on either side and the
whole expression takes the boxed path. This is not a limitation to engineer
around; it is the correct reading of the semantics.

**(b) An assignment inside a function writes the global slot.** `interp.c:173-185`:

```c
/* NOTE, deliberate and shared with the C backend: assigning a name
 * inside a function writes the GLOBAL of that name when one exists.
 * That is what lets the self-hosted compiler share a cursor across
 * functions. Only parameter binding is scoped; see env_define. */
static void env_assign(Env *e, const char *name, Value val)
```

Therefore **per-function type inference is unsound as the language stands.**
Any inference pass must begin with a whole-program collection of global names,
and treat an assignment to a name that has a global as a store into that
global's declared type (`any` unless annotated). This is one extra pre-pass, it
is exactly the pass `compile.c` and `compiler.zl` already run to emit globals,
and skipping it produces silently wrong code rather than an error. It is the
single most likely way to get this feature subtly wrong.

---

## 3. Ordering argument: why records are Stage 7, not Stage 0

The heterogeneity finding is the strongest input to this plan and it appears to
argue for reordering. 51 of 111 `.zl` files use mixed-type lists, but zl has no
struct, no map and no tuple, so *every record in the corpus is a heterogeneous
list* — `stdlib/astar.zl` says so out loud ("zl has no maps"). Tiered:

| Tier | Need | Files |
|---|---|---|
| A | irreducibly dynamic — keep them dynamic | 8 (`jsonw`, `json_pretty`, `json_parse`, `sortx`, `lisp_interp`, `uuid`, `compiler.zl`, `examples/calc_repl`) |
| B | tuples / records | ~23 |
| C | nullable / option (33 functions using `nil()` as a sentinel) | 13 |
| D | generics `list[T]` | ~11 (`dict`, `set`, `hashset`, `deque`, `heapq`, `stack_queue`, `binarytree`, `listx`, `searching`, `sorting`, `quicksort`) |

So records + nullables + generics collapse 40 of the 51, and the type system's
irreducible job is 8 files, not 51. The natural conclusion is "build records
first, or the type system will be designed wrong."

**I think that conclusion is half right, and the half that is right is a spec
obligation, not a build-order one.** Three reasons to keep records at Stage 7:

1. **A record buys zero speed until there is a type system to unbox its
   fields.** A record implemented as a boxed `Value` is exactly as slow as the
   heterogeneous list it replaces — same malloc, same tag, same `zl_binop`. The
   heterogeneity finding is a *coverage* result, and coverage is worth a lot
   (`MASTER_PLAN.md` §10: "the bottleneck is coverage, not codegen"), but it is
   not a measurable win and this plan opens with a measurable win on purpose.
2. **The benchmarks contain no records.** `b1`–`b3` and `c1`–`c4` are pure
   scalar integer work. The entire measurable path from "boxed" to "at parity
   with C" runs through `int` and `bool` and touches no aggregate at all. Doing
   records first means Stages 1–6 of speed work sit behind a language feature
   they do not use.
3. **Records have the largest blast radius in the repo.** A record type touches
   `runtime.h`, `runtime.c`, `interp.c`, `compile.c`, `nativegen.c` *and*
   `compiler.zl` (which must at minimum parse them), plus it collides with
   `design_memory_structs.md`, which specifies a `struct` that is *an address
   and an offset table, never a value* — a different feature that shares a word.
   Annotations, by contrast, are provably inert until something reads them.

What the finding *does* change, and this is the half that is right:

> **Stage 0 must reserve grammar and assignability room for records,
> nullables and `list[T]` before a single type is checked.** If the type
> grammar ships as `int | float | bool | str | list[T] | any` with no plan for
> a named product type, Stage 7 re-litigates the assignability rule and the
> boundary tables for 23 files' worth of new shapes. Design the grammar for six
> types; implement three.

Concretely: `type := NAME | NAME "[" type ("," type)* "]"` from day one, so
`Point`, `list[int]` and a future `map[str,int]` all parse under one rule, and
write §2's assignability with "a named type is assignable only to itself or
`any`" stated even though no named type exists yet.

---

## 4. The stages

Every stage: **one commit, `.\verify.ps1` green, one agent.** Measurement is
`pwsh -File bench\run_bench.ps1` (`-Only bN` to isolate, `-Runs 5` to de-noise).
One property of the harness makes Stages 1–2 unusually easy to measure:
`interp.exe` does **not** link `runtime.c` (`build.bat` builds it from
`interp.c parser.c lexer.c os_win.c`), so a `runtime.c` change moves the
`boxed-C` column and leaves the `interp` column fixed. The harness's own
speedup-vs-interpreter ratio is therefore the metric, with the control built in.

---

### Stage 1 — Kill the string-keyed operator dispatch

**Changes:** `runtime.c` only. `zl_binop` keeps its exact signature
(`Value zl_binop(const char *op, Value l, Value r)` — `runtime.c:312`), so no
caller, no generated `.c`, and no line of `compiler.zl` changes. Replace the
`strcmp` ladder with a `switch (op[0])` plus one `op[1]` test:

```c
if (strcmp(op, "+") == 0)  return binop_plus(l, r);
if (strcmp(op, "==") == 0) return zl_bool(values_equal(l, r));
...                                        /* 15 more, in order */
```

`op[0]` discriminates all of them: `+ - * / % = ! < > a o` (`"and"` → `'a'`,
`"or"` → `'o'`), with `op[1] == '='` separating `<`/`<=` and `>`/`>=`. Keep the
`rt_error("unknown operator")` default arm.

**Files:** `runtime.c`.

**Expected:** the measured cost is 46 `strcmp`s per iteration of `b2_arith`,
110 million for the run. Removing them should be worth roughly 1.3–1.6x on the
`boxed-C` column of `b2_arith`. **This is an estimate, not a measurement** — I
did not profile the replacement, and part of the 46 calls are short strings the
compiler may already have inlined.

**Measured by:** `bench\run_bench.ps1 -Only b2 -Runs 5`, `boxed-C` column,
before and after, with the `interp` column as the unchanged control. Also run
`-Only b4` (list) and `-Only b5` (string) to confirm no regression.

**What could break:** an operator string reaching `zl_binop` that the switch
does not enumerate. `in` does *not* reach it — `compile.c:236` routes it to
`contains()`/`has()` separately — but grep every `zl_binop(` call site in
`compile.c` (lines 245, 316, 331) and in `compiler.zl:487` and confirm the
operator set matches before deleting the ladder.

---

### Stage 2 — Shrink `Value` from 48 bytes to 16

**Changes:** `runtime.h` + `runtime.c`. Today:

```c
/* runtime.h:14-22 */
typedef struct Value {
    ValueType      type;
    double         num;      /* V_NUM, and V_BOOL (0/1) */
    char          *str;      /* V_STR                   */
    struct Value **items;    /* V_LIST                  */
    int            nitems;
    int            cap;      /* V_LIST spare capacity (amortized push)  */
    int           *tip;      /* V_LIST slots handed out - see zl push() */
} Value;
```

4 + 4 pad + 8 + 8 + 8 + 4 + 4 + 8 = **48 bytes**. Make the payload a union of
`double num` / `char *str` / a pointer to a heap-side list header carrying
`items`/`nitems`/`cap`/`tip`, keeping `type` as the **first** member. 16 bytes.

**Keep the `memset` in `zl_nil`.** `interp.c:44-50` records why: the C backend
has always relied on `zl_nil` zeroing the *whole* struct, because a partially
initialised `Value` gave the interpreter garbage where the backend saw 0, and
the two engines disagreed. A `memset` of 16 bytes preserves that invariant
exactly and is still a 3x cut against the measured 31.2 million memsets / 1.5 GB
of zeroing. The win here is size, not the removal of the zeroing.

**Files:** `runtime.h`, `runtime.c`.

**Expected:** the measured ABI cost is ~1.44 KB of hidden-pointer copying per
`b2_arith` iteration, 3.4 GB for the run; 48→16 cuts that ~3x. Estimated
1.5–2x on the `boxed-C` column of `b2_arith` and `b4_list`.

**Honest caveat I could not verify:** 16 bytes does **not** get `Value` into a
register on Win64 — the ABI passes aggregates in registers only at 1, 2, 4 or 8
bytes, so the hidden pointer stays. Only an 8-byte `Value` (NaN-boxing: payload
in the mantissa of a signalling NaN, which works because Win64 user-mode
pointers are 47 bits — the same fact `design_memory_structs.md` §2.2 leans on)
rides in `rcx`. **NaN-boxing is deliberately deferred**, not rejected: it
touches `interp.c`'s separate `Value` (`interp.c:33-43`, which has an extra
`V_FN` and a `Node *fn`) as well as `runtime.h`'s, and doing it before the type
system exists means doing the invasive change twice. Confirm the
register-passing claim by disassembling one `zl_binop` call before writing the
speedup into a commit message.

**Measured by:** `bench\run_bench.ps1 -Runs 5`, all five benchmarks, `boxed-C`
column. `b4_list` should move most (allocation-heavy).

**What could break:** any read of a field that is not the active union member.
`to_string` and `values_equal` switch on `type` first and are safe by
inspection; **the audit is every other `.num` / `.str` / `.items` read in
`runtime.c`, and I did not do it.** One place in *generated* code touches a
field — `compile.c:162` emits `if (r.type == V_LIST) return zl_calln("contains", ...)` —
and it reads only `type`, which stays first. `interp.c` has its own `Value` and
is not affected; that halves the blast radius.

---

### Stage 3 — Annotations lex and parse, and nothing reads them

Two commits, both green.

**3a (C side):** `lexer.c` gains the `->` two-char case inside `lex_symbol`
after the `-=` ladder (§1.1). `parser.h` gains optional type slots; `parser.c`
parses `name: type` in assignment position, `param: type` in `parse_fn`, and
`-> type` after `)`. **Nothing reads the slots.** `type := NAME | NAME "[" type ("," type)* "]"`
per §3 — the multi-argument form parses now even though nothing produces it.

**3b (`compiler.zl` side):** the mirror `skip_type()` — consume a NAME,
optionally `[` recurse `]` — called after each param, after `)`, and after an
assignment LHS. Per `design_type_system.md` §7.1 this is ~20 lines and the AST
shape does not change: `["fn", name, params, body]` and `["assign", name, value]`
are emitted exactly as before.

**Files:** 3a — `lexer.c`, `parser.c`, `parser.h`. 3b — `compiler.zl`.

**Expected speedup: zero.** That is the point. This stage is the enabler and it
is provably inert.

**Measured by:** not the bench harness. The gate is the **skip-neutrality
check**: for every one of the 111 `.zl` files in the tree, the `out.c` produced
by `compiler.zl` before 3b must be byte-identical to the `out.c` produced after
it. Provable in advance by inspection (`skip_type` is never reached when no
`:`/`->` appears in code position) and checkable in one loop.

**What could break:** `->` lexed too early, mangling comments and
`stdlib/brainfuck.zl`'s string literals — the exact failure §1.1 warns about.
Second risk: the parser accepting `x: int` where the ternary wants
`cond ? a : b`. Both `:` uses are unambiguous by position (ternary `:` follows a
`?` that has already been consumed), but write the test both ways before
trusting it.

*Note on ordering:* this stage touches `compiler.zl` even though the brief says
put fixpoint risk last. The distinction is deliberate. **Teaching `compiler.zl`
to skip annotations is additive and inert, and the skip-neutrality gate proves
it.** *Annotating* `compiler.zl` changes what it emits and is the real fixpoint
risk — that is Stage 9.

---

### Stage 4 — The type pass, check-only

**Changes:** a new `check.c` over `Node*`, wired into `interp.c` and
`compile.c` as a **check that changes no representation**
(`design_type_system.md` §6, "check, don't use" — the only one of three options
that keeps the three-engine test honest). Flags `--no-check` and `--warn` per
`design_types.md` §4.

**This stage owns the §2.1(b) global pre-pass.** Collect every global name
first; an assignment inside a function to a name that has a global is a store
to the global's slot and is checked against the global's declared type.

**Files:** new `check.c`, small hooks in `interp.c`, `compile.c`, `build.bat`.

**Expected speedup: zero, possibly slightly negative** (one extra tree walk at
compile time).

**Measured by:** negative tests — each deliberately-violated annotation
produces the same error text from `interp.exe` and from the C backend. Plus the
Neutrality harness (`design_type_system.md` §6.2): strip annotations, run both,
diff.

**What could break:** false positives on stdlib. Mitigation is structural —
unannotated is `any`, `any` is assignable both directions, so a file with no
annotations cannot produce a diagnostic. If it does, the bug is in the pass, not
the file.

---

### Stage 5 — `compilel.c` consumes types: unboxed `int` and `bool`

**Changes:** `compilel.c` reads the Stage-4 types. A typed function becomes
`define i64 @zl_f(i64 %a)`. Typed locals become `alloca` (so `mem2reg`/SROA
promote them) instead of the current globals. Everything else still exits with
"not supported yet", exactly as the header comment says today: *"Values are
unboxed i64. Supported: arithmetic, comparisons, and/or/not, variables,
if/else, while, break/continue, functions, print of an integer and print of a
string literal."*

**Per §2.1(a), emit `add i64` only when both operand types are `int`.**

**Files:** `compilel.c`.

**Expected:** annotated code reaches the numbers already measured — parity with
`clang -O2` (67 vs 71, 58 vs 61, 33 vs 33, 62 vs 62). The *new* thing is not
the speed, it is that the speed now applies to a program the compiler
understands rather than to a subset it assumes.

**Measured by:** annotated copies of `b1`–`b3` and `c1`–`c4` through the `llvm`
column, against the unannotated originals in the same run. Plus
`llvm(P) == interp(P)` on every one.

**What could break:** the `-O2` assumption. `alloca`-per-local is only free if
`mem2reg` fires; if a local's address escapes it does not, and the annotated
version could be *slower* than the fiat-`i64` version is today. Check the
emitted `.ll` for surviving `alloca`s in the benchmark loops.

**`float` is not in this stage.** A separate agent is landing floats now; the
`float` annotation is the same machinery with `double`/`fadd`/`fcmp` and slots
in immediately after their work lands. It must not be built twice.

---

### Stage 6 — The boundary: box/unbox shims and `any`

**Changes:** `runtime.c` gains `zl_box_int` / `zl_box_bool` / `zl_unbox_int` /
`zl_unbox_bool` / `zl_type_error`. `compilel.c` emits sites B1–B7 and U1–U6 per
`design_type_system.md` §4.2–§4.3, and emits the "crosses a boxing boundary
inside a loop" note from §4.5.

**Files:** `runtime.h`, `runtime.c`, `compilel.c`.

**Expected: no speedup on its own** — and that is worth stating plainly,
because it is the largest single stage in the plan. What it buys is that
`compilel` stops *refusing*: an annotated hot function can now live inside an
unannotated program instead of requiring the whole file to be in the numeric
subset. That is the coverage unlock, and it is what makes Stage 5's parity
usable by real code.

**Measured by:** the anti-pattern loop from `design_type_system.md` §4.5 (typed
`step`, untyped `acc`, one million iterations) must produce the same answer as
`interp.exe`, and the compiler must print the boundary note. Then annotate
`acc` and confirm the boundary disappears from the `.ll` and the time drops.

**What could break:** a silent coercion sneaking into `zl_unbox_int`. It must
trap, never default. A `zl_unbox_int` that returns 0 on a string is the worst
possible failure of Annotation Neutrality — a different answer instead of an
error.

---

### Stage 7 — Records (the Tier B unlock, ~23 files)

**Changes:** a record type in the type system — a named product of typed
fields — that `compilel` lowers to an LLVM struct in an SROA-able `alloca`, and
that `interp.c`/`compile.c` represent as they represent a list today.

**Files:** `lexer.c`/`parser.c` (declaration syntax), `check.c`, `compilel.c`,
`interp.c`, `compile.c`, `runtime.c`, and `compiler.zl` (skip only).

**Expected:** no direct benchmark movement — no benchmark uses a record. The
win is that ~23 stdlib modules become annotatable at all, which is the
`MASTER_PLAN.md` §10 "coverage is the bottleneck" position cashed in.

**Measured by:** count of corpus files that type-check with at least one
non-`any` annotation, before and after. That number, not milliseconds, is this
stage's metric — say so in the commit.

**What could break:** collision with `design_memory_structs.md`. That document
specifies `struct` as *an address plus a compile-time offset table, never a
value*, for FFI. This stage's record is a value with typed fields. **They are
two features sharing a word and v1 must not try to unify them.** Pick two
different spellings, or write the unification design first. I flag this as the
single largest unresolved design question in the plan and I am not resolving it
here.

---

### Stage 8 — Nullables (Tier C, 13 files) and generics (Tier D, ~11 files)

Two commits.

**8a — nullable.** 33 functions across 13 files use `nil()` as a sentinel
return. `T?` (or an equivalent spelling) plus a narrowing rule: after
`if x != nil { ... }`, `x` is `T` inside the block. Without narrowing the
feature is useless, so narrowing is in scope, not deferred.

**8b — `list[T]`.** The 11 container modules (`dict`, `set`, `hashset`,
`deque`, `heapq`, `stack_queue`, `binarytree`, `listx`, `searching`, `sorting`,
`quicksort`). The O(n) `list[T] ↔ any` trap in `design_type_system.md` §4.4
lands here: `list[any]` is bit-identical to today's dynamic list and crosses for
free; `list[int]` is a flat buffer and crossing is a copy plus a malloc per
element, warned about at every site.

**Expected:** `b4_list` is the benchmark that should finally move — it is
`push` and indexed reads in a hot loop, currently 4.1x over the interpreter on
the boxed backend. An annotated `list[int]` version should approach the scalar
numbers.

**Measured by:** `bench\run_bench.ps1 -Only b4` on an annotated copy, plus the
corpus type-check count again.

**What could break:** an accidental O(n) conversion inside a loop. §4.4's answer
is that `list[T]` and `list[any]` are *not assignable* to each other at all, so
the copy can never happen implicitly. Hold that line even when it is annoying.

---

### Stage 9 — Annotate `compiler.zl` (the fixpoint risk, last, optional)

**Changes:** annotate `compiler.zl`'s hot paths. Nothing else in the toolchain.

**Why last:** `verify.ps1` compares gen1 to gen2 *within one run*
(`f(f(x)) == f(x)`), never against a stored hash, and its only input is
`compiler.zl` — so it proves **closure over one file, not coverage**.
`design_selfhost_parity.md` measures that `compiler.zl` mishandles 63 of 110
`.zl` files today while the gate stays green. The gate will not catch a
coverage regression, and it *will* catch this one specific trap:

```
/* compiler.zl:480 */    s = "zl_list_n(" + len(node[1])
/* compiler.zl:509 */    s = "zl_calln(" + cstr(node[1]) + ", " + len(node[2])
```

Both concatenate a **number** onto a **string** to emit C. That routes through
`binop_plus` → `to_string`, whose `V_NUM` arm is:

```c
/* runtime.c, to_string */
if (v.num == (long long)v.num)
    snprintf(buf, sizeof(buf), "%lld", (long long)v.num);
else
    snprintf(buf, sizeof(buf), "%g", v.num);
```

If unboxing ever changes how `len()`'s result formats — `3` becoming `3.0`, or
a `float`-typed length — then gen1 emits `zl_list_n(3,` and gen2 emits
`zl_list_n(3.0,`, the hashes differ, and `verify.ps1` goes red for a *formatting*
reason that looks like a compiler bug. Before annotating anything here, add a
canonical-integer-format test, and never annotate a value that flows into these
two lines as `float`.

**Expected:** faster self-hosted compilation. Genuinely optional — this is the
one stage that can be skipped without leaving the plan incomplete.

**Measured by:** wall-clock of `.\interp.exe compiler.zl`, plus `verify.ps1`
green (which is the actual point).

---

## 5. Summary table

| # | Stage | Files | Expected | Fixpoint risk |
|---|---|---|---|---|
| 1 | de-string `zl_binop` dispatch | `runtime.c` | ~1.3–1.6x on `b2` boxed-C (est.) | none |
| 2 | `Value` 48 → 16 bytes | `runtime.h/.c` | ~1.5–2x on `b2`/`b4` boxed-C (est.) | none |
| 3a | `->` lexes, annotations parse | `lexer.c`, `parser.c/.h` | zero (enabler) | none |
| 3b | `compiler.zl` skips annotations | `compiler.zl` | zero (enabler) | low, gated |
| 4 | type pass, check-only | new `check.c` | zero | none |
| 5 | `compilel` unboxes `int`/`bool` | `compilel.c` | parity with `clang -O2` on annotated code | none |
| 6 | box/unbox boundary + `any` | `runtime.*`, `compilel.c` | zero direct; coverage unlock | none |
| 7 | records (Tier B, ~23 files) | broad | coverage, not ms | medium |
| 8 | nullables + `list[T]` (Tiers C+D) | broad | `b4_list` should move | medium |
| 9 | annotate `compiler.zl` | `compiler.zl` | faster self-host | **high** |

---

## 6. WHAT THIS WILL NOT FIX

**Strings.** `b5_string` is the outlier in the measured table and unboxing does
not touch it: the boxed C backend is **1.3x** faster than the tree-walking
interpreter there, against 7–13x on numeric code. The reason is that both
engines spend essentially all of their time inside the *same* `runtime.c`
string routines. Compiling the control flow is worthless when the work is in
the runtime.

Look at `binop_plus`'s string arm and the problem is obvious without a profiler:

```c
char *ls = to_string(l);
char *rs = to_string(r);
char *out = malloc(strlen(ls) + strlen(rs) + 1);
strcpy(out, ls); strcat(out, rs);
```

Every `+` on strings is two `_strdup`s, a `malloc`, a `strlen` pair, a `strcpy`
and a `strcat`, and then two `free`s — and `b5_string` is quadratic
concatenation, so that whole sequence runs once per character of the growing
result. Annotating the variables `str` changes **none** of it. A `str` type
lets the compiler skip the tag check; it does not make the copy cheaper,
because the copy is the semantics.

Fixing `b5_string` is a different project with a different shape: immutable
string interning or a rope/builder representation, `strlen` caching in the
string header, and a `+`-chain optimisation that computes the final length once
and fills a single buffer. None of that needs a type system and none of it is
in this plan. `b4_list` (4.1x) is the same story at lower volume — some of it
falls out of Stage 8's flat `list[int]`, but the `push`-returns-a-new-list
semantics is the real cost and that is also a runtime question, not a typing
one.

The honest summary: **this plan makes numbers fast and leaves the runtime
alone.** That is the right first target, because numbers are where the measured
4–6x lives and where the benchmarks that compare zl against other languages
will be won. But no stage below should be sold as making zl fast in general
until `runtime.c`'s string and list routines get their own plan.

---

## 7. Things I am not sure about

Stated here rather than asserted confidently above:

- **The Stage 1 and Stage 2 speedup numbers are estimates**, derived from the
  operation counts measured today (46 strcmps/iteration, ~1.44 KB of ABI copy
  per iteration, 31.2M memsets) and not from running the replacement. Treat
  them as hypotheses the bench harness will confirm or refute.
- **Whether MSVC passes a 16-byte struct by hidden pointer on Win64** — I
  believe it does (registers only for 1/2/4/8-byte aggregates), which is why
  Stage 2 is a 3x copy reduction and not an elimination. Verify by disassembly
  before claiming otherwise.
- **The full audit of non-active union member reads in `runtime.c`** for
  Stage 2 was not done. It is the first task of that stage, not an afterthought.
- **The Tier A/B/C/D file counts** come from another agent's corpus analysis
  and I did not re-derive them; I used them only for ordering, where being off
  by a few files changes nothing.
- **Records vs `design_memory_structs.md`'s `struct`** (Stage 7) is genuinely
  unresolved and is the largest open design question the plan depends on.
- **Whether Stage 4's checker should also be written in zl** (as
  `design_types.md` §5.3 sketches) is left open. `design_type_system.md` §7.3
  says `compiler.zl` only ever needs to *skip* types, never check them, and
  this plan follows that — but a zl-side checker is the natural Stage 10 and
  the best self-hosting stress test available.
