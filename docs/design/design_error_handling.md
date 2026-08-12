# Design: Error Handling — a Coverage Decision, Not an Ergonomics One

**Status:** proposal · **Author:** audit pass · **Date:** 2026-08-02
**Scope:** decides *how zl programs and zl tests observe an error*. Recommends
one new PowerShell script and one added stanza in `verify.ps1`. **No code is
changed by this document, and nothing in the language changes.** Line-number
citations are against the tree as it stood on 2026-08-02; `interp.c`,
`runtime.c` and the test suites were being edited concurrently by another agent,
so treat them as pointers, not anchors.

---

## 0. TL;DR

zl has no way to write *"check that this expression errors."* Twenty assertions
have now been deleted from `tests/` and replaced with prose, because the engine
correctly raises on those inputs (§1). Five of them were replaced with something
worse than a deletion: `check(label, "documented", "documented")` — an assertion
that structurally cannot fail and reports **PASS**.

**Recommendation: (c), the out-of-process harness. Ship it this week. Name (d),
errors-as-values, as the successor and block it on nullable/narrowing. Reject
(a) try/catch on engine cost and identity; reject (b) `expect_error` on parity.**

Three measured reasons, in order of weight:

1. **The one thing the tests most want to pin cannot be caught in-language at
   all.** `7 % 0` (`tests/test_math.zl:262-271`) is an integer divide, so the
   CPU kills the process. Measured today: exit code is **not 1**. A `try`, an
   `expect_error` and a `Result` all run *inside* the process that dies. Only a
   harness that watches from outside can tell a raise from a crash — and
   converting crashes into raises is precisely what the current hardening work
   is doing. The harness is the instrument that measures the hardening.
2. **A real `catch` is five different mechanisms, not one** (§3.4). The
   interpreter would use its existing unwind flags; `compile.c` emits straight C
   and would need `setjmp`/`longjmp` (which leaks every boxed `Value` allocated
   in the `try`, and zl has no GC); `compilel.c` emits `ret i64` and would need
   either the Itanium EH ABI or an error-flag on every call ABI — a tax on the
   backend `MASTER_PLAN.md` §10 just named as *the* speed backend;
   `nativert.c` has **no error path whatsoever** (§2.3) so it would need raising
   built first, in hand-written x86-64. Parity across engines is load-bearing
   here — `tests/test_bitwise.zl:319-321` says so out loud.
3. **The harness costs nothing and fixes something larger than the twenty
   assertions.** Nothing runs `tests/*.zl` today (§1.3). `verify.ps1` runs the
   fixpoint and `run_tests.ps1`; `run_tests.ps1` runs six inline integer
   programs. The eight suites are hand-run and ungated. The same script that
   runs error snippets can run the eight suites and assert `0 failed` — turning
   the whole test corpus from documentation into a gate, for one file.

What this does **not** do is give a zl *program* a way to recover. That is a real
gap and §6 trigger 2 is the day it must be closed.

---

## 1. What actually broke, measured

### 1.1 The twenty, by file and line

Counting method, stated before the number: `rg` over `tests/*.zl` for the three
artifact shapes a deleted error-assertion leaves behind — a tautology `check`, a
commented-out `check`, and a commented `print(...)` in a `BUGS` block that
records behaviour which is now a raise. Nothing was executed to produce this
table except the four probes in §2.4.

| kind | count | sites |
|---|---|---|
| tautology `check(l,"documented","documented")` | **5** | `test_lists.zl:100`, `test_lists.zl:140`, `test_math.zl:271`, `test_strings.zl:342`, `test_structures.zl:1067` |
| commented-out `check(...)` | **2** | `test_math.zl:270`, `test_structures.zl:1066` |
| commented repro in a `BUGS` block | **13** | `test_bitwise.zl` ×7 (defects 1–3), `test_strings.zl:339-341` ×3, `test_math.zl:265`, `test_structures.zl:1065`, `test_syntax.zl:646-648` |
| **total** | **20** | |

That is the brief's ~20, accounted for exactly.

### 1.2 The tautologies are worse than deletions

```
check("empty first/last error, see comment", "documented", "documented")
```
— `tests/test_lists.zl:100`

`check` is `if str(got) == str(want)` (`tests/test_lists.zl:22`). `"documented"`
always equals `"documented"`, so this increments `g_pass` unconditionally. Five
suites now report a pass count that includes five checks asserting nothing. A
deleted assertion is honest about being absent; this one lies in the direction of
health, and the pass count is a number the project reads.

### 1.3 The larger finding: nothing runs `tests/` at all

`verify.ps1` is the single gate (`MASTER_PLAN.md` §10). It has exactly two
checks: the fixpoint (`verify.ps1:19-51`) and `& .\run_tests.ps1`
(`verify.ps1:55`). `run_tests.ps1` builds its own six programs inline as
PowerShell here-strings (`arith`, `fib`, `factorial`, `nested`, `logic`,
`labeled`) and runs each through interp / C backend / native backend.

`rg` over every `.ps1` and `.bat` in the tree for `tests/` or a suite name
returns **nothing**. The eight suites — several thousand `check` calls that
encode hand-computed expected values, plus every `BUGS` block quoted above — are
run by hand or not at all. **The coverage loss in §1.1 is real, but it happened
inside a corpus that was never gated to begin with.** Any answer that does not
also fix that is treating the smaller problem.

### 1.4 The clearest single casualty

`tests/test_bitwise.zl:311-345` records three real defects, all now fixed:
a 53-bit precision cliff across all six bitwise builtins, `bnot` not being an
involution at `INT64_MIN`, and an undefined `double`→`int64` cast for
out-of-range operands. The fix was one policy — *"a bitwise builtin either gives
you the right bits or raises. It never rounds, and it never casts out of
range."* (`test_bitwise.zl:313-315`).

The **fixed** behaviour of all three is recorded as seven shell commands in a
comment. The file says why:

> zl has no way to catch an error, so the refusals cannot be asserted above.
> Each one is reproduced here instead. Put the line in a .zl file and run
> `.\interp.exe` on it; the C backend prints the identical message, which is the
> parity these two files have to keep.
> — `tests/test_bitwise.zl:319-322`

Note the last clause. The file already understands that the contract is
*"identical message from both engines,"* and it already knows the check is a
shell command. Option (c) is not a new idea; it is the thing four test files
already tell the reader to do by hand.

---

## 2. The raise surface, measured

### 2.1 How a raise happens

There are exactly two raise functions, and they are structurally identical:

```c
static void runtime_error(const char *msg)
{
    fflush(stdout);            /* stdout is block-buffered when redirected;
                                  without this the output that explains HOW
                                  we got here dies with the process. */
    fprintf(stderr, "runtime error: %s\n", msg);
    exit(1);
}
```
— `interp.c:219-226`, and byte-for-byte the same body as `rt_error` at
`runtime.c:18-25`.

Call-site counts: **109** in `interp.c`, **101** in `runtime.c`. The eight-site
gap is expected — `interp.c` has `V_FN` and the compiled runtime does not (§7,
R6) — but it has not been reconciled and nothing reconciles it.

### 2.2 There is no unwinding machinery anywhere

`rg` for `setjmp`, `longjmp`, `jmp_buf` across `interp.c`, `runtime.c` and
`compile.c`: **zero hits.** Every raise is a process exit. There is no
intermediate state in which an error exists as a thing a program could inspect.

The interpreter *does* have an unwind mechanism, for a different purpose:

```c
static int   g_returning = 0;
...
static Value g_return_value;
```
— `interp.c:195-198`, with the flags checked at `interp.c:1539`
(`if (g_returning || g_breaking || g_continuing) return;`), `:1603` and `:1613`.

A fourth flag pair — `g_raising` / `g_raise_value` — would slot into that shape
cleanly. This is the honest strongest argument *for* option (a), and §4.1 costs
it properly.

### 2.3 `nativert.c` has no error path at all

`rg` for `exit`, `abort`, `trap`, `panic` in `nativert.c` (1534 lines): the only
hits are `ExitProcess(0)` at `:1382` and the import-table string at `:1507`. The
native runtime cannot report an error, let alone catch one. Any in-language
mechanism has to build raising there before it can build catching.

### 2.4 The three observable failure classes — probed today

Run against `interp.exe` in the working tree. These are the only executions in
this document.

| snippet | stdout | stderr | exit |
|---|---|---|---|
| `print("before")` / `print(first([]))` | `before` | `runtime error: first needs a non-empty list` | **1** |
| `print("a\0b")` | — | `line 1: a string cannot contain a NUL byte (\0 or \x00) - zl strings are NUL-terminated` | **1** |
| `print(7 % 0)` | — | *(nothing)* | **not 1** |

Three things fall out, and all three matter:

1. **The stdout-before-the-error survives.** The `fflush(stdout)` at
   `interp.c:221` is what makes that true, and it means a harness can assert on
   partial output *and* the error together.
2. **Lex/parse errors and runtime errors are distinguishable by prefix** —
   `line N: ` versus `runtime error: ` — and both exit 1. So the harness can pin
   `tests/test_syntax.zl:645-648`'s NUL case, which no in-language mechanism can
   reach at all, because the file never runs.
3. **`7 % 0` is not a raise.** `interp.c` computes `%` as
   `(double)((long long)a % (long long)b)`, so a zero divisor is a hardware
   integer-divide fault. The process is killed by the CPU; nothing is printed;
   the exit status is the Windows structured-exception code, not 1. This is
   exactly what `tests/test_math.zl:262-269` documents, and it is the single
   most important fact in this document (§4.6).

---

## 3. Constraints that narrow the answer

### 3.1 Identity

`MASTER_PLAN.md:663`, in the **LOCKED** block: *"Systems language, power over
safety, total PC control."*

**Honesty note:** the brief given to this pass also cites *"no hidden control
flow"* as an identity constraint. I could not find that phrase in
`MASTER_PLAN.md` or anywhere under `docs/` — `rg` returns zero hits. I have
treated it as a real design principle because it is consistent with the locked
line and with §3.3, but it is **not** a written commitment and should not be
quoted as one.

The relevant reading of the written line: an exception is safety machinery that
costs power (a non-local jump the compiler must be conservative around) and buys
convenience. The three closest systems languages — Rust, Go, Zig — all declined
exactly this trade. That is not dispositive, but it is the company zl says it
keeps.

### 3.2 The fixpoint is not the blocker people think

`design_selfhost_parity.md` §1 settles this and the finding should not be
re-derived: `verify.ps1` check 1 is `f(f(x)) == f(x)` over **one input**,
`compiler.zl`, and `compiler.zl` uses only what `compiler.zl` implements. Every
W2 feature so far — `elif`, ternary, `in`, f-strings, ranged `for`, `do`/`while`,
`break`/`continue`, compound assignment, index assignment — landed in the C
toolchain and the gate stayed green.

So **new syntax does not break the fixpoint.** Adding `try` would move the
tracked coverage number from `47/110` to `47/111`-ish and nothing else. Per that
document's recommendation (§6, option a′), `compiler.zl` is a frozen bounded
subset that *announces* what it refuses; a `try` it does not implement is
supposed to produce a message and no `out.c`, which is the correct outcome.

This removes the fixpoint from the argument entirely. It does not remove §3.3.

### 3.3 The reserved-word lock is real but already soft

`MASTER_PLAN.md:665`, **LOCKED**: *"11 reserved words (§4.4)."* Alongside it at
`:667`: *"Built-ins are identifiers, not keywords."*

`lexer.c:48-54` currently lists **15**:

```c
"if", "else", "elif", "for", "in", "fn", "return", "while",
"not", "and", "or",          /* word operators, not && || */
"true", "false",
"break", "continue",         /* loop control */
```

So the lock has already drifted by four without a recorded reopen. Adding `try`
and `catch` would make 17. This is a governance cost, not a technical one, and it
should be stated as such rather than used as a veto: the honest position is that
adding two more keywords requires a decision-log entry, and that the list's
current state suggests nobody would notice if it did not get one — which is
itself the reason to insist on it.

### 3.4 Five engines, five mechanisms — the load-bearing constraint

This is the cost that dominates everything below. The engines do not share a
control-flow representation, so a `catch` is not one feature implemented five
times; it is five *different* features:

| engine | what `return` compiles to today | what a `catch` would need |
|---|---|---|
| `interp.c` | global flags, `g_returning` at `:195`, checked `:1539`, `:1603`, `:1613` | a fourth flag pair — **cheap and idiomatic** |
| `compile.c` | a real C `return` (`compile.c:370-373`) | `setjmp`/`longjmp` — a *different* mechanism, and it leaks (§4.1) |
| `compilel.c` | `ret i64` (`compilel.c:326`) — unboxed | Itanium EH (`invoke`/`landingpad`/personality) or an error-flag ABI on every call |
| `compilef.c` | `long long` numeric subset, 185 lines | refuse it (fine — it already refuses, `:79`, `:120`) |
| `nativert.c` | hand-written x86-64; only `ExitProcess(0)` at `:1382` | build raising first, by hand, in assembly |

`design_selfhost_parity.md` §5.2 already quotes `MASTER_PLAN.md:449` on the last
row: *"each one becomes another hand-assembly job."*

And parity is not optional decoration. `tests/test_bitwise.zl:321-322` states the
contract the test suites are written against: the C backend must print *"the
identical message."*

### 3.5 zl cannot spawn a subprocess

```c
static const char *SIMULATED[] = {
    "kill", "start", "rm", "poke", "peek",
    "window", "copy", "move", "run", NULL
};
```
— `interp.c:391-393`. `run` is simulated: it prints what it *would* do
(`interp.c:812`).

Two consequences. First, option (c) cannot be written in zl; it is a PowerShell
file, alongside `run_tests.ps1` and `verify.ps1`, which is where the project's
test orchestration already lives. Second, no in-language option can be
*prototyped* by shelling out either — there is no cheap path to a zl-side
experiment.

One thing that *does* exist and helps: `exit(code)` is a real builtin
(`interp.c:1162-1164`), so a harness snippet can signal a specific status
deliberately when a raise is not what is being tested.

---

## 4. The five options, costed

### 4.1 (a) A `try` / `catch` form

```
try { risky() } catch e { print(e) }
```

**Language cost.** Two reserved words (§3.3), one new `NodeType` appended to
`parser.h` (append at the end — `design_type_system.md` R8 records why
mid-enum insertion is hazardous: `compilel.c` and `compilef.c` print raw `type
%d` in their unsupported-node errors).

**Engine cost.** §3.4's table, and it is worse than the table suggests:

- **`interp.c` is the cheap one and it is still not cheap.** Adding
  `g_raising`/`g_raise_value` and three flag checks is ~30 lines. But
  `runtime_error` is `noreturn` *in practice* at all 109 call sites, and every
  one is written as though it never returns. For example:

  ```c
  if (nargs<1||args[0].type!=V_STR) runtime_error("env needs a string");
  const char *val = getenv(args[0].str);
  ```
  — `interp.c:1165-1167`

  If `runtime_error` starts returning, that `getenv` runs on a `Value` that is
  not a string. Every one of the 109 sites must be audited and given an explicit
  early return. The same audit, 101 sites, in `runtime.c`. **The cost is
  210 audited call sites, not 30 lines of control flow**, and a missed one is a
  silent memory-safety defect rather than a test failure.
- **`compile.c` needs a mechanism the interpreter does not use.** Emitted code is
  straight C with real `return`s, so a `catch` is `setjmp`/`longjmp`. zl is
  boxed: `zl_nil` memsets a fresh 48-byte struct for every value constructed
  (measured, ~31.2M memsets in one `b2_arith` run), and there is no GC. A
  `longjmp` out of a `try` abandons every `Value` allocated inside it. A `try`
  inside a loop leaks unboundedly. That is not a bug to fix later; it is the
  design.
- **`compilel.c` is the one the plan cares about.** `MASTER_PLAN.md` §10,
  2026-08-02: *"LLVM IS THE OPTIMISER. Full stop, for now."* An LLVM error path
  is either the full Itanium EH ABI or a manual error-flag return ABI applied to
  every call — and either one obstructs exactly the `clang -O2` passes the
  backend was adopted for. Adding it taxes the measured parity result (67ms vs
  71, 58 vs 61, 33 vs 33, 62 vs 62) for a feature the tests do not need.
- **`nativert.c` needs raising before catching**, by hand, in x86-64.

**Fixpoint cost.** Zero, per §3.2.

**Identity cost.** This is the real objection. A `catch` means any call inside the
`try` can transfer control to the handler invisibly, and the compiler must be
conservative about every one of them. That is safety machinery bought with
power, in a language whose one locked identity line is *power over safety*.

**And it still cannot catch `7 % 0`.** §4.6.

**Verdict: rejected for now.** Not because it is wrong — it is the right shape
for the *recovery* problem (§6 trigger 2) — but because it costs the most across
five engines, taxes the speed backend the plan just committed to, and does not
solve the problem actually in front of us.

### 4.2 (b) An `expect_error` builtin, usable only in tests

Three possible shapes. All three fail, for different reasons.

**Value form** — `expect_error(f)` where `f` is a function value. Functions *are*
first-class in the interpreter; probed today:

```
fn twice(n) { return n + n }
g = twice
print(g(4))          # prints 8
```

But `runtime.h:12` is:

```c
typedef enum { V_NIL, V_NUM, V_STR, V_BOOL, V_LIST } ValueType;
```

versus `interp.c:32`:

```c
typedef enum { V_NIL, V_NUM, V_STR, V_BOOL, V_LIST, V_FN } ValueType;
```

**There is no `V_FN` in the compiled runtime.** A function value has no
representation there, so the builtin cannot exist in `runtime.c` at all. The
moment a suite uses it, the interp/compiled parity that
`tests/test_bitwise.zl:321` treats as the contract is broken. (This is a
pre-existing divergence found while costing this option, not a new one — see §7
R6.)

**Source form** — `expect_error("first([])")` needs an `eval` at runtime. The
interpreter carries a parser; a compiled binary does not. Same parity break,
larger.

**Special-form** — `expect_error(<expr>)` recognised at parse time and compiled
to a guarded region. This is not a builtin; it is §4.1's `try` with one arm, a
worse name, and no `catch` binding. It costs everything (a) costs and it
violates `MASTER_PLAN.md:667` (*"Built-ins are identifiers, not keywords"*) by
making an identifier behave like a keyword.

There is also a "tests only" problem worth naming: zl has no notion of a test
build. A builtin that exists only in tests is a builtin that exists, and the
first non-test program to use it will find it.

**Verdict: rejected.** The value form breaks parity by construction; the
special-form is (a) in disguise.

### 4.3 (c) An out-of-process harness

A PowerShell script — call it `test_errors.ps1` — writes each snippet to a temp
`.zl`, runs `.\interp.exe` on it, and asserts on exit code, stderr and stdout.
**No language change at all.**

The contract it asserts against already exists and was measured today (§2.4).
The twenty lost assertions map onto it one-for-one; the seven `test_bitwise.zl`
repros are *already written in exactly this form* and just need a file to live
in.

- **Cost to the five engines: zero.** It links against nothing. It runs the
  binaries the build already produces.
- **Cost to the fixpoint: zero.** No syntax, so `compiler.zl` is untouched and
  the `47/110` number does not move.
- **Cost to identity: zero.** Nothing is added to the language.
- **Cost to write: one file, low hundreds of lines, and the project already has
  two scripts of exactly this shape.** `run_tests.ps1` already does the
  write-snippet / run / compare dance.
- **It also fixes §1.3.** The same script can run the eight suites in `tests/`
  and assert `0 failed` on each. That is the larger win, and it is free.

**The honest limits, stated plainly:**

1. **It is coarse.** You assert on a whole process, not an expression. You cannot
   check that state *after* the error is sane — there is no after. For test
   coverage that is fine, because the property under test is "this refuses"; for
   a program that wants to recover it is useless.
2. **One process spawn per case.** At ~30 cases, negligible next to the `cl`
   invocations `verify.ps1` already does. At 3000 it would be wrong.
3. **It makes the error *strings* part of the contract.** Rewording
   `"first needs a non-empty list"` would fail the gate. This is a real cost and
   it is worth paying: four test files already quote those strings verbatim in
   comments, so they are a de-facto contract already — this makes it de jure and
   therefore visible when it changes. (§7 R1 has the mitigation.)
4. **It is PowerShell, not zl** (§3.5), so it is a thing the language cannot test
   itself with. Given `run` is simulated, that is not a choice.

**Verdict: recommended.** §5.

### 4.4 (d) Errors as values, Rust-style

`fn f() -> int | err`, checked at the call site. No hidden control flow; composes
with the nullable design; the right long-term answer.

**It is blocked, and not by anything this document can unblock.**
`design_type_system.md:912` parks the prerequisite explicitly:

> **Union / optional / nullable types** — Every one of them requires
> flow-sensitive narrowing to be usable, and narrowing is its own design. `any`
> covers the need today.

Errors-as-values *is* a union type plus narrowing. It cannot ship before that
does, and that design does not exist yet.

**It also does not solve the problem in front of us.** The 210 raise sites
(§2.1) are almost all inside *builtins*. Turning `first([])` from a raise into a
returned error changes the signature of the entire stdlib and every call site
across 111 `.zl` files. That is not a test-coverage fix; it is the language's
second-largest refactor after the type system itself. And the heterogeneity
finding makes it worse, not better: with no struct, no map and no tuple, an
`err` value would be encoded as a heterogeneous list like every other record in
the corpus — which is exactly the encoding the type-system work is trying to
retire.

**Syntax cost.** `->` has a settled lexing plan with a known hazard: the two-char
check must go *inside* `lex_symbol` after the `-=` ladder, because `->` appears
in comments and inside `stdlib/brainfuck.zl`'s strings. A union `|` is new and
unstudied — and `|` is not currently in the symbol set, so it needs its own
lexer pass.

**And it still cannot catch `7 % 0`.** §4.6.

**Verdict: right, later, named as the successor.** Re-cost it the day
nullable/narrowing lands (§6 trigger 3).

### 4.5 (e) Do nothing

The cost is not stable, which is what makes this the worst option rather than the
neutral one.

- The five tautologies (§1.2) are **already** corrupting the pass count. That is
  not a future cost; it is present and unmeasured.
- The cost compounds with the hardening work *by construction*: every fix that
  converts silently-wrong into a raise moves an assertion out of the checked
  column and into the prose column. `test_bitwise.zl` already lost three fixed
  defects that way, and the float work now in flight will convert more.
- The suites are ungated anyway (§1.3), so nothing detects the erosion.

**Verdict: rejected — but note that (e) is what is currently in effect.** That
sets the bar: anything that ships beats it, and a thing that ships this week
beats a better thing that ships next quarter.

### 4.6 The fact that decides it

`tests/test_math.zl:262-271`:

> ### CRASH - the % operator with a zero right-hand side kills the process.
> `interp.c` does `(double)((long long)a % (long long)b)`, an INTEGER modulo, so
> `b == 0` raises a hardware divide-by-zero: the interpreter dies with
> `0xC0000094` (STATUS_INTEGER_DIVIDE_BY_ZERO) and prints nothing at all.

Confirmed by probe (§2.4): exit status is not 1, stderr is empty.

**Options (a), (b) and (d) are all blind to this by construction.** Their
mechanism — a flag, a `longjmp`, a returned value — lives inside the process the
CPU just terminated. Only (c) sees it, and (c) sees it *for free*, because a
non-1 exit status is exactly what distinguishes a crash from a raise.

That matters far beyond one test. The stated policy of the hardening work is to
convert silently-wrong and crashing into raising —
`test_bitwise.zl:313-315`: *"a bitwise builtin either gives you the right bits or
raises."* **The harness is the only proposed mechanism that can assert that
policy held**, because the policy's subject is the difference between two
process outcomes. An in-language `try` can assert that a raise happened; it can
never assert that a crash *didn't*.

---

## 5. Recommendation

**Ship (c). Name (d) as the successor. Reject (a) and (b) for now; reject (e)
outright.**

Concretely, this week:

1. **`test_errors.ps1`** — a table of cases, each `{ snippet; expect_exit;
   expect_stderr; expect_stdout }`. Seed it with the twenty from §1.1. Assert
   exit code always; assert stderr by full string for the ~20 messages already
   quoted verbatim in test comments, and by prefix elsewhere.
2. **Delete the five tautologies** and replace each with a one-line comment
   pointing at its case in `test_errors.ps1`. A prose comment that points at a
   real assertion is fine; a fake assertion is not.
3. **Run the eight suites from the same script**, asserting `0 failed` on each —
   but see §7 R4: measure the current baseline first and gate on *"no worse than
   baseline"*, because I have **not** run them and do not know that they pass.
4. **Add one stanza to `verify.ps1`**, after `run_tests.ps1`. The gate stays one
   command.
5. **Record in `MASTER_PLAN.md` §10**: *"Error observation is out-of-process
   (`test_errors.ps1`). zl has no in-language error handling and none is
   planned until nullable/narrowing lands; the successor is errors-as-values
   (d), not exceptions (a)."*

Nothing about this precludes (a) or (d) later. That is the point: it is the only
option that buys back the coverage without spending a decision the project is
not ready to make.

Sketch, to make the shape concrete — not proposed code, just the shape:

```powershell
$cases = @(
  @{ name="first of empty";  src='print(first([]))'
     exit=1; err='runtime error: first needs a non-empty list' }
  @{ name="NUL in a string"; src='print("a\0b")'
     exit=1; errPrefix='line 1: a string cannot contain a NUL byte' }
  @{ name="modulo by zero is a CRASH, not a raise"; src='print(7 % 0)'
     exitNot=1; err='' }          # flips to exit=1 the day % is hardened
)
```

That third case is the one worth pausing on. It is written as an assertion about
the *current, wrong* behaviour, and it will go red the moment `%` is hardened —
which is precisely when someone should be told to come and update it. No
in-language mechanism can express that case at all.

---

## 6. What would change my mind

Concrete, checkable. Any one of these re-opens the decision.

1. **Somebody needs to recover, not just observe.** This is the big one and it is
   a *product* trigger, not a test trigger. Everything above is about tests. The
   moment a real zl program must keep running after an error — a REPL that
   should not die on a typo (`examples/calc_repl` is already the canonical
   irreducibly-dynamic program), a long-running script, anything with a retry —
   the harness is irrelevant and (a) or (d) becomes mandatory. Nothing in this
   document argues zl should never have error handling; it argues the *tests*
   are not the reason to build it.
2. **Nullable/Option lands with flow-sensitive narrowing.** Then (d) costs a
   fraction of today's price and must be re-costed immediately. `design_type_
   system.md:912` is the gate; when that line is deleted, come back here.
3. **`nativert.c` grows a raise path for any other reason.** The worst row in
   §3.4's table is then already paid, and (a) gets substantially cheaper.
4. **The harness gets slow or large** — say >5 seconds or >200 cases. Per-case
   process spawn is then the wrong shape, and the special-form variant of (b)
   becomes worth its engine cost despite §4.2.
5. **The error strings start churning.** If §4.3's limit 3 becomes a real
   maintenance drag, that is evidence the messages are not yet a contract, and
   the harness should drop to exit-code-plus-prefix everywhere.
6. **A second implementation of `%` (or any operator) is hardened and the
   harness cannot tell.** If §4.6's argument stops holding — i.e. every crash
   class has been converted to a raise — then the harness's unique capability is
   gone and the comparison must be redone on ergonomics alone.

Explicitly **not** triggers: `try`/`catch` being familiar from other languages;
the five tautologies being ugly (that is fixed by step 2 of §5 regardless); a
reviewer preferring exceptions.

---

## 7. Risks

| # | Risk | Mitigation |
|---|---|---|
| R1 | Asserting on error text makes the harness brittle; a wording change goes red. | Assert exit code **always**; assert full text only for the ~20 messages already quoted verbatim in test comments (they are a de-facto contract already); prefix-match everything else. A red from a deliberate reword is a 30-second fix and is *supposed* to be visible. |
| R2 | (c) is read as the permanent answer and (d) never gets designed. | §6 trigger 1 is a product trigger and will fire on its own. Step 5 of §5 records (d) as the named successor in the decision log so the deferral is dated, not silent. |
| R3 | The harness spawns `interp.exe` while another agent is rebuilding it — flaky. | Run it inside `verify.ps1`, after the build, never concurrently. Same discipline `run_tests.ps1` already relies on. |
| R4 | **Putting `tests/*.zl` into the gate turns eight unknown suites into a blocker.** I have **not** run them; I do not know that they pass. | Measure first. Run all eight by hand once, record the pass/fail baseline the way `design_selfhost_parity.md` §6.4 records `47/110`, and gate on *"no worse than baseline"* before gating on zero. If the baseline is not zero-fail, that is itself a finding worth more than this document. |
| R5 | The exit-code contract is asserted from `interp.exe` only. `rt_error` (`runtime.c:18-25`) is structurally identical, so compiled binaries *should* match — but **I did not build and check**, per this pass's read-only constraint. | The harness's first job is to verify it: run each case through the C backend too, exactly as `run_tests.ps1` runs its six programs through three engines. If they diverge, that is a parity bug and the harness found it on day one. |
| R6 | *(Recorded, out of scope — found while costing option (b).)* `runtime.h:12`'s `ValueType` has no `V_FN`; `interp.c:32` does. Functions are first-class in the interpreter (probed: `g = twice; print(g(4))` prints `8`) and have no representation in the compiled runtime. `compile.c:210` emits a bare `v_<name>` for an identifier read, which for a function name is the C function symbol, not a `Value`. | Not fixed here. It is a live interp/compiled divergence and deserves its own fix, in the same spirit `design_selfhost_parity.md` R6 recorded `nativeval.c`'s compound-assignment miscompile rather than losing it. |
| R7 | The `%`-by-zero case (§5 sketch) asserts *wrong* behaviour and someone "fixes" the harness instead of the engine. | Name it in the case label — `"modulo by zero is a CRASH, not a raise"` — and add a one-line comment saying the correct response to a red is to update the case, not to loosen it. |
| R8 | Twenty cases is a lower bound; more prose-only error facts exist that this pass did not enumerate. | Very likely true. §1.1 states its counting method so the number can be re-derived; treat 20 as a floor and add cases as the `BUGS` blocks are read properly. |

---

## 8. Definition of done

1. `test_errors.ps1` exists, is table-driven, and covers all twenty sites in
   §1.1 by file:line reference.
2. Every case asserts an exit code. Message text is asserted in full only where
   the string is already quoted in a test comment; elsewhere by prefix.
3. Each case runs through **both** `interp.exe` and the C backend, and the two
   must agree — the parity `tests/test_bitwise.zl:321` already claims.
4. The `7 % 0` case exists and asserts the current *crashing* behaviour, with a
   comment saying a red means the engine changed.
5. The five `check(l,"documented","documented")` tautologies are gone, each
   replaced by a comment naming its harness case.
6. The eight suites in `tests/` run from the harness. A baseline pass/fail count
   is recorded and dated, exactly as `47/110` is.
7. `verify.ps1` calls the harness and goes red if it fails. The gate is still one
   command.
8. `MASTER_PLAN.md` §10 records the decision and names (d) as the successor,
   blocked on nullable/narrowing.
9. `.\verify.ps1` is green.

---

## 9. Uncertainties, stated rather than guessed

Things this pass could not establish, listed so nobody reads confidence into
them:

- **Whether the eight suites currently pass.** Not run (R4).
- **Whether compiled binaries reproduce the interpreter's exit/stderr contract.**
  `rt_error` and `runtime_error` are structurally identical, but nothing was
  built (R5).
- **The exact Windows exit status for `7 % 0`.** Observed as non-1 through a
  Git-Bash shell, which truncates; `tests/test_math.zl:264` says `0xC0000094`
  and that is almost certainly right, but the raw `NTSTATUS` was not read
  directly.
- **Whether `g = twice` even compiles under `compile.c`.** The interpreter probe
  succeeded; the compiled side was reasoned about from `runtime.h:12` and
  `compile.c:210`, not built (R6).
- **Whether "no hidden control flow" is a real project commitment.** It is not
  written anywhere I could find (§3.1). If it *is* a commitment, someone should
  write it down, because it is the single cleanest argument against (a) and
  right now it is folklore.
