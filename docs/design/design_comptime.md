# Design: Compile-Time Execution for zl

**Status:** proposal · **Floor:** 1 (widening the seed), and the **mechanism Floor 2 needs**
**Date:** 2026-08-02
**Scope:** one new keyword (`comptime`), one new node type, one reification function,
one fuel counter, one purity partition over the existing builtin table, and one
on-disk cache format. No code is changed by this document.
Line citations are against the tree as it stood on 2026-08-02. `lexer.c`,
`parser.c` and the numeric backends were being edited concurrently (floats), so
treat line numbers as pointers, not anchors.

**Related:** `design_records.md`, `design_maps_v2.md`, `design_nullable.md`,
`design_error_handling.md`, `design_scoping_decision.md`, `design_type_system.md`,
`design_modules.md`, `PLAN_unboxing.md`. This document assumes records, maps,
nullables and error handling will exist and does not redesign them.

---

## 0. TL;DR

`comptime e` means: **evaluate `e` with the interpreter, during compilation, and
splice the resulting value back into the tree as a literal.** Everything else in
this document is consequences and guard rails.

Six claims, and they are the whole design:

1. **The "zl already has the interpreter in the same binary" argument is
   *nearly* true, and the gap is small but real.** `compile.exe` does **not**
   link `interp.c` today (`build.bat:31`). Closing that gap is one `#ifndef`
   guard and a two-function header — the pattern already exists twice in the
   tree (`lexer.c:412`, `parser.c:1109`). Cost: ~30 lines, not a new evaluator.
   Zig wrote a second evaluator; zl genuinely does not have to. §1.
2. **Comptime runs the *whole* language minus a deny-list**, not a hand-picked
   subset. The deny-list is 11 builtins, all of which are already identifiable
   by name in one `strcmp` ladder (`interp.c:420`). §3.
3. **Values reach runtime code by *reification*: `Value` → `Node`.** Numbers,
   strings, bools and lists of those become `N_NUMBER`/`N_STRING`/`N_BOOL`/
   `N_LIST`. Functions do not reify. The splice mechanism is already in the
   tree — `parse_slot_expr` (`parser.c:151`) does exactly this dance for
   f-strings. §4.
4. **Constant folding is the degenerate case and comes free**, and it
   incidentally dissolves the `+` overloading blocker *for folded expressions
   only* — because a running `eval()` knows both operand types by construction.
   §5.
5. **Comptime does NOT subsume generics in zl, and should not try to in v1.**
   Zig's `fn f(comptime T: type)` needs types-as-values, `.` member access
   (`interp.c:1521` still errors: `"member access (.) isn't supported yet"`),
   and sound per-function scoping (`design_scoping_decision.md`). zl has none of
   the three. §6 says so plainly rather than pretending.
6. **Reproducibility is the whole risk, and an LLM call does not satisfy it.**
   `verify.ps1` proves `f(f(x)) == f(x)` in a single run. A model call breaks
   that on the second run, not the first, which is the worst possible failure
   mode. The fix is not "cache the result" — it is **the compiler is never
   allowed to call a model at all**. A separate command does that, writes the
   answer into the source tree, and a human approves it. §10.

**Recommendation: build §2–§5 and §8–§10 (the core, ~700 lines). Build §11
(`ai_rule`) only after Floor 3's rule language exists. Do not build §6.**

---

## 1. Is zl actually positioned for this?

### 1.1 The claim, checked

The pitch is: *Zig had to write a separate comptime interpreter; zl has one
already, in the same binary as the compiler, so `comptime` is nearly free.*

The first half is true. The second half is **not true as stated today**, and a
design document that repeats it uncorrected would send someone into a build
expecting a free lunch.

`build.bat:31`:

```
cl /nologo /W4 /DBUILD_PARSER /DBUILD_INTERP compile.c parser.c lexer.c /Fe:compile.exe
```

`interp.c` is not on that line. `compile.exe` is lexer + parser + C emitter.
Same for `nativegen.exe` (`build.bat:35`). The interpreter and the compiler share
a **front end**, not an address space.

### 1.2 What it costs to make the claim true

Small, and the pattern is already established twice.

`lexer.c:412` and `parser.c:1109` each wrap their demo `main()` in a build guard
so the file can be linked into a larger program:

```c
#ifndef BUILD_PARSER      /* lexer.c:412  */
#ifndef BUILD_INTERP      /* parser.c:1109 */
```

`interp.c:1646`'s `main()` has **no such guard** — it is the last link in the
chain, so it never needed one. Adding `#ifndef BUILD_COMPTIME` around it, plus a
tiny `interp.h` exposing two entry points, makes `interp.c` linkable into
`compile.exe`, `compilel.exe` and `nativegen.exe`:

```c
/* interp.h - the comptime entry points */
void  zl_interp_init(void);              /* build g_global, define builtins */
Value zl_interp_eval_node(Node *n);      /* eval one expression in g_global */
```

Every other function in `interp.c` is `static` and stays that way.

**Two real costs, stated now rather than discovered later:**

- **Binary size and build-graph coupling.** Every backend that supports
  `comptime` now links the whole interpreter (1,665 lines) plus `os_win.c`.
  `compile.exe` roughly triples in source scope. `nativegen`, whose entire
  selling point is "no C compiler in the output", now contains a tree-walker it
  never runs at runtime. That is acceptable — it is *compile*-time bloat, not
  runtime bloat — but it is not free.
- **`compiler.zl` cannot do this at all.** The self-hosted compiler
  (`compiler.zl`, 716 lines) has no interpreter to call. It reads a file
  (`compiler.zl:712`, `input = read("input.zl")`) and writes C
  (`compiler.zl:715`, `write("out.c", result)`). For `compiler.zl` to support
  `comptime` it would have to **contain a zl interpreter written in zl** — which
  is a real and interesting project, and is exactly how a mature self-hosted
  language ends up, but it is not v1. §12 covers what this means for the gate.

**Verdict: the architecture advantage is real but it is a ~30-line advantage,
not a ~3,000-line one.** The 3,000 lines Zig spent were on a *typed* comptime
evaluator with pointer semantics and memory model; zl's `eval()` is untyped and
boxed, which is why it can be reused as-is. That is the honest version of the
claim.

---

## 2. Syntax

### 2.1 The expression form

```
comptime <expr>
```

`comptime` binds tighter than any binary operator and looser than a call, so:

```
x = comptime fib(30)          # fib(30) runs at compile time
y = comptime a + b            # parses as comptime (a + b)
z = (comptime a) + b          # explicit when you want the other one
```

Precedence choice: **prefix, at the same level as `not`** (`parse_not`,
`compiler.zl:269`, mirrored by `parse_unary` in `parser.c`). This makes
`comptime a + b` fold the whole expression, which is what people mean 95% of the
time, and the parenthesised form is available for the other 5%.

### 2.2 The statement form

```
comptime {
    <statements>
}
```

A comptime block runs for its **effects on the comptime environment**, not for a
value. It is how you build a table that later comptime expressions read:

```
comptime {
    PRIMES = sieve(1000)
}
lookup = comptime PRIMES        # spliced as a 168-element list literal
```

### 2.3 The declaration form (sugar, and it earns its keep)

```
comptime NAME = <expr>
```

Equivalent to a one-line comptime block plus a reified global. This is the form
`ai_rule` uses in §11 and the form conditional compilation uses in §7, and
writing it as `comptime { X = e }` every time is noise.

### 2.4 What is rejected, and why

| Alternative | Why not |
|---|---|
| `@compileTime(...)` (Zig-ish sigil) | `@` is not lexed today. `comptime` costs **one string in one array** — `lexer.c:48`, the `keywords[]` list, which currently holds 14 words. Adding a sigil costs a branch in `lex_symbol` and a new token class. |
| `$expr` (Nim/Terra-ish) | Same lexer cost, worse readability, and `$` is wanted later for shell-adjacent things. |
| `const` implying comptime (C++ `constexpr` drift) | zl has no `const`. Introducing one *and* overloading it with evaluation semantics is two features wearing one word. |
| A separate meta-language | This is the thing zl specifically gets to avoid. Non-starter. |

**Keyword cost, precisely.** `lexer.c:46-58`:

```c
static int is_keyword(const char *word)
{
    static const char *keywords[] = {
        "if", "else", "elif", "for", "in", "fn", "return", "while",
        "not", "and", "or",          /* word operators, not && || */
        "true", "false",
        "break", "continue",         /* loop control */
        NULL
    };
```

One line. And a corpus check is required before landing it, because a keyword is
a word the language steals from you (`lexer.c:32-33` says exactly that): grep the
110 `.zl` files for `comptime` used as an identifier. I have not run that grep;
whoever implements must, and must also check `compiler.zl`'s own `is_keyword`
(`compiler.zl:36`), which is a **separate copy of the same list** and will
silently disagree if only one is edited. That divergence is a live parity hazard
(`design_selfhost_parity.md`).

---

## 3. What is legal at comptime

### 3.1 Deny-list, not allow-list

Two designs:

- **Allow-list:** enumerate the pure builtins, reject everything else. Safe,
  and wrong — it means every new builtin is comptime-hostile until someone
  remembers to add it, and the stdlib's 96 modules would mostly be unusable at
  comptime for no reason.
- **Deny-list:** run everything, reject a named set. Requires being right about
  the set, but the set is small, closed, and already visible in one place.

**Choose the deny-list.** The whole point of reusing `eval()` is that
`sort`, `join`, `split`, `range`, `sqrt`, `hex` and the other ~90 builtins work
at comptime for free. An allow-list throws that away.

### 3.2 The deny-list, with line numbers

`call_builtin` (`interp.c:420`) dispatches by name through a `strcmp` ladder.
Eleven of those names are not functions of their arguments:

| Builtin | `interp.c` | Why denied |
|---|---|---|
| `read` | 728 | Filesystem read — build depends on tree state. §9. |
| `write` | 742 | Filesystem **write** — a compile that mutates the world. |
| `write_bytes` | (near 742) | Same. |
| `dir` | 779 | Directory listing — varies by machine. |
| `procs` | 797 | Running processes — varies by *second*. |
| `now` | 1159 | Wall clock. The canonical reproducibility bug. |
| `env` | 1165 | Environment variables — varies by shell. |
| `input` | 435 | Blocks the build on a human. |
| `exit` | 1162 | Kills the compiler mid-parse. |
| `random` | 838 | Nondeterministic by definition. |
| `randint` / `seed` | 839 / 837 | `seed(k)` then `random()` *is* deterministic, but only if nothing else in the build touched `rand()`. Global `srand` state makes this order-dependent. Deny both; a comptime PRNG belongs in the stdlib as a pure LCG. |

`read` is the interesting one and §9 splits it out — it is the only denial that
costs real expressiveness, and the only one where "deny" is arguably the wrong
call.

### 3.3 Everything else is legal

Loops, recursion, `for`, `while`, list mutation, string building, user functions,
records (`design_records.md`), maps (`design_maps_v2.md`), nullables
(`design_nullable.md`). If `interp.exe` can run it, `comptime` can run it, with
the fuel limit of §8 and the deny-list above.

**One asymmetry worth naming:** comptime code runs on the *boxed* interpreter,
which the benchmarks put at 2–5× slower than CPython. A comptime loop that would
take 40 ms in the LLVM backend takes seconds in `eval()`. Comptime is for
*computing tables and text*, not for moving work off the runtime clock as a
general optimisation strategy. Say so in the docs or people will be surprised.

---

## 4. How comptime values reach runtime code

### 4.1 Reification: `Value` → `Node`

This is the one genuinely new function. `Value` (`interp.c:32-45`) is a runtime
struct:

```c
typedef enum { V_NIL, V_NUM, V_STR, V_BOOL, V_LIST, V_FN } ValueType;
```

`Node` (`parser.h:33-42`) is the tree the backends consume. Reification maps one
to the other:

| `ValueType` | Reifies to | Notes |
|---|---|---|
| `V_NUM` | `N_NUMBER`, `n->text` = printed double | Must round-trip. §4.2. |
| `V_STR` | `N_STRING` | Must fit `MAX_TEXT` (128). §4.2. |
| `V_BOOL` | `N_BOOL`, text `"true"`/`"false"` | Matches `eval`'s `strcmp(n->text,"true")` at `interp.c:1440`. |
| `V_LIST` | `N_LIST` with reified kids | Recursive; cycle-checked. |
| `V_NIL` | `N_CALL` to `nil` | zl has no nil literal; `nil` is a builtin. |
| `V_FN` | **error** | §4.3. |

### 4.2 Three sharp edges, all real

**(a) `MAX_TEXT` is 128** (`lexer.h:10`), and `Node.text` is a fixed
`char text[MAX_TEXT]` (`parser.h:35`). A comptime expression that produces a
2 KB string — which is *exactly* what `ai_rule` produces — **cannot be reified
into one `N_STRING` node** without either raising `MAX_TEXT` (touches every
token, every node, both `.exe` sizes and `compiler.zl`'s parallel structures) or
splitting into a chain of `N_BINARY("+")` concatenations. I lean to the chain,
because it changes no shared structure; but it makes `str()` of a generated rule
ugly if anyone prints the tree. **This is unresolved and I am flagging it rather
than guessing.**

**(b) Doubles must round-trip.** `N_NUMBER` stores text and `eval` does
`atof(n->text)` (`interp.c:1438`). Print with `%.17g`, and add a self-check in
the reifier: `atof(printed) == original`, else hard error. Without this, a
comptime float silently loses a bit and the *fixpoint* is what discovers it.

**(c) String escaping.** Reifying `V_STR` means writing a source-level string
literal, so quotes, backslashes and newlines must be re-escaped to whatever the
lexer accepts. `unescape` exists in `compiler.zl:39`; the inverse does not exist
anywhere and must be written, once, and shared.

### 4.3 Functions do not reify — and why that is fine

`V_FN` holds `Node *fn` (`interp.c:34-44`). Reifying a closure would mean
emitting the function's whole body, which is a different feature (staging /
quasiquotation) and pulls in the environment-capture problem that
`design_scoping_decision.md` has not resolved. zl has no closures anyway. **A
comptime expression that evaluates to a function is a compile error**, with the
message naming the function. That is a real limitation and it is the one that
blocks §6.

### 4.4 The splice mechanism already exists

`parser.c:151`, `parse_slot_expr` — the f-string implementation:

```c
static Node *parse_slot_expr(const char *src, int line)
{
    int    nsub = 0;
    Token *sub  = lex_text(src, &nsub);

    Token *save_toks  = toks;
    int    save_ntoks = ntoks;
    int    save_pos   = pos;

    toks = sub; ntoks = nsub; pos = 0;
```

This is a compiler that **lexes a string produced during parsing, parses it into
the same tree, and restores its cursor**. It was written for `f"hi {name}"`. It
is, mechanically, the entire "generated source becomes real source" pathway, and
`lex_text` is already exported for it (`lexer.h:43`). §11's `ai_rule` reuses this
function unchanged.

That precedent matters more than it sounds: it means text-to-AST splicing is
already load-bearing in a shipping feature, so the risk is known rather than new.

### 4.5 Pipeline

```
source.zl
   |
   v
 lex  -->  parse  -->  TREE with N_COMPTIME nodes
                            |
                            v
                     COMPTIME PASS  <-- links interp.c (§1.2)
                       - walk tree
                       - for each N_COMPTIME: eval() it, with fuel (§8)
                       - reify Value -> Node (§4.1)
                       - REPLACE the N_COMPTIME node in place
                            |
                            v
                     TREE with no N_COMPTIME nodes
                            |
              +-------------+-------------+
              v             v             v
          compile.c     compilel.c    nativegen.c
```

**The backends do not change.** That is the design's best property: a comptime
pass that runs to completion leaves a tree the four existing backends already
know how to compile. If `N_COMPTIME` survives into a backend, that is an internal
error, and each backend gets a one-line `default:` case saying so.

---

## 5. Constant folding is the trivial case

### 5.1 It falls out

`comptime 2 + 3` evaluates to `V_NUM 5` and reifies to `N_NUMBER "5"`. That is
constant folding, spelled explicitly. Implicit folding — the compiler folding
`2 + 3` without being asked — is then a **one-line policy change**: try the
comptime pass on any `N_BINARY` whose operands are already literals, and keep the
result if it succeeds.

### 5.2 It dissolves the `+` blocker, but only locally

The stated language-level blocker is that `+` is overloaded and tested:
`add(2,3) == 5` and `add("n=",5) == "n=5"`, so no integer add can be emitted
without knowing both operand types.

At comptime that problem does not exist, because `eval_plus` (`interp.c:1180`)
has the actual `Value`s in hand:

```c
static Value eval_plus(Value l, Value r)
```

It dispatches on `l.type` and `r.type` at runtime, which is always available
during comptime evaluation. So **any expression the comptime pass folds emerges
with its type known**, and the backend sees a literal.

Be precise about the size of this win: it fixes folding, **not inference**. It
tells you nothing about `+` inside a function whose parameters are unknown. The
type system (`design_type_system.md`) still has to solve that. Comptime is not a
back door around the annotation work.

### 5.3 What the backends get

For `compilel.c` and `nativegen.c` — both integer-oriented — folded literals mean
loop bounds, mask constants and lookup tables arrive as immediates. Given
MASTER_PLAN §10's ruling that **LLVM is the optimiser and we never write our own
passes**, note that LLVM would have folded most of this anyway. The value of
explicit `comptime` is not the folding; it is that folding is *guaranteed and
visible*, and it works on `nativegen` too, which has no optimiser at all and is
~4× off on tight loops precisely because of that.

---

## 6. Does comptime subsume generics? In Zig yes. In zl, no.

### 6.1 Zig's model

In Zig a generic is a function whose parameter is a type, known at comptime:

```zig
fn max(comptime T: type, a: T, b: T) T { return if (a > b) a else b; }
```

The compiler evaluates `T` at comptime, monomorphises the body, and emits one
specialised function per instantiation. There is no separate generics feature —
`type` is a first-class comptime value and that is the whole mechanism. It is
genuinely elegant and it is why this section exists.

### 6.2 Three things zl lacks, all load-bearing

1. **Types are not values.** `ValueType` (`interp.c:32`) has six members and none
   of them is "a type". There is no `V_TYPE`, no way to write `int` as an
   expression, and `type(x)` (`interp.c`, in the builtin ladder) returns a
   *string*. Making types first-class is a bigger change than `comptime` itself.
2. **`.` does not work.** `interp.c:1521`:

   ```c
   case N_MEMBER:
       runtime_error("member access (.) isn't supported yet");
   ```

   Zig's comptime type manipulation is almost entirely `@typeInfo(T).Struct.fields`
   — member access on comptime values. `design_records.md` lowers `img.w` to
   `img[0]` *in the parser*, which is right for records and gives us nothing for
   reflection.
3. **Monomorphisation needs sound per-function scoping.** Specialising a function
   body means substituting into it and re-typing it. Today an assignment inside a
   function writes the **global** slot when a global of that name exists
   (`interp.c:180`, `env_assign`) — `design_scoping_decision.md` establishes that
   this makes per-function inference unsound and that the engines already
   disagree about it. You cannot soundly specialise a function whose body's
   meaning depends on which globals happen to exist at the call site.

### 6.3 Recommendation

**Do not build generics-via-comptime in v1, and do not design v1's `comptime`
around eventually doing so.** The design in §2–§5 is forward-compatible with it
(a `V_TYPE` added later slots into reification as one more case) but does not
require it.

When generics do arrive, the order is: `design_scoping_decision.md` resolved →
type system → `V_TYPE` and `N_MEMBER` → monomorphisation. That is three
prerequisites deep. Claiming comptime gives zl generics today would be the single
most misleading sentence this document could contain.

---

## 7. Conditional compilation

### 7.1 `comptime if`

```
comptime if TARGET == "kernel" {
    fn log(m) { kdbg(m) }
} else {
    fn log(m) { print(m) }
}
```

The comptime pass evaluates the condition and **replaces the whole `N_IF` with
the taken branch's block**. The untaken branch is deleted from the tree before
any backend sees it — so it does not have to compile, does not have to typecheck,
and can reference builtins that only exist on the other target. That last
property is the actual reason to have this feature rather than a runtime `if`.

Deleting an untaken branch means it is also **never parsed for errors beyond
syntax**. That is the standard `#ifdef` tax: a branch nobody builds rots. Mitigate
by having CI build every value of every comptime flag — not by weakening the
feature.

### 7.2 Where the flag comes from — and where it must not

`env()` is on the deny-list (§3.2, `interp.c:1165`) and this is exactly why. If
`TARGET` came from the environment, two developers with different shells would
compile different programs from identical source, and `verify.ps1` would still
pass for both. That is a reproducibility hole disguised as a convenience.

**Flags come from the command line and are recorded in the output.**

```
compile.exe prog.zl -DTARGET=kernel
```

The pass predefines those names as comptime globals, and the emitted C/IR carries
a comment line listing every flag and value. The build is then a function of
(source, flags), both of which are visible.

---

## 8. Termination

A comptime expression can loop forever. `while true { }` at comptime hangs the
*compiler*, which is worse than hanging a program, because build systems have no
good story for it.

### 8.1 Fuel, not a timeout

Use a **step counter**, decremented in `eval`/`exec`, that hard-errors at zero.
Not a wall-clock timeout. The distinction is the whole point:

- A timeout makes compilation **machine-dependent**. The same source compiles on
  the fast machine and fails on the slow one. That is precisely the class of bug
  §10 exists to prevent.
- Fuel is deterministic: the same source burns the same fuel everywhere, so a
  program either compiles for everyone or fails for everyone.

Default `COMPTIME_FUEL = 10_000_000` steps, overridable per-file by a comptime
declaration so a legitimately expensive table can raise it *in the source*, where
it is reviewable.

### 8.2 The precedent is already there

`interp.c:207` already does the analogous thing for recursion:

```c
#define MAX_CALL_DEPTH 2000
static int   g_depth = 0;
```

and the comment above it (`interp.c:200-206`) records exactly why: without the
counter the process was "KILLED (STATUS_STACK_OVERFLOW) with no message and with
every buffered line of output lost." A comptime infinite loop is the same failure
wearing a different hat, and deserves the same fix.

### 8.3 Cost

One `if (--g_fuel < 0)` at the top of `eval` and one in `exec`. It is on the hot
path of the interpreter, which is already the slow engine, and it can be compiled
out of `interp.exe` with `#ifdef BUILD_COMPTIME` so the ordinary interpreter pays
nothing. Measure before assuming it is free; I have not.

---

## 9. The filesystem

`read` (`interp.c:728`) is the denial that costs something real. Reading a data
file at compile time and baking it into the binary is one of comptime's genuinely
great uses — Zig's `@embedFile`, Rust's `include_str!`.

Three options:

| Option | Behaviour | Verdict |
|---|---|---|
| **A. Deny outright** | `read` errors at comptime. | Safe, and loses `@embedFile`. |
| **B. Allow freely** | `read` works. | The build now depends on unstated files. Change a data file, `verify.ps1` still green, binary silently different. **No.** |
| **C. Allow, but record** | `read` works and every path+hash read is written to a manifest emitted alongside the output. | Chosen. |

**Option C, precisely.** The comptime pass keeps a list of `(path, sha256)` for
every file read. It writes that list into the output as a comment block and into
a sidecar `.deps` file. A rebuild that finds a changed hash **fails loudly**
rather than silently producing a different binary. The build is a function of
(source, flags, manifest), and all three are on disk and diffable.

`write` (`interp.c:742`) and `write_bytes` are **denied unconditionally, no
option C**. A compiler that mutates the filesystem as a side effect of
compilation is not a compiler, and the failure modes (two parallel builds, a
comptime block that overwrites its own source) are unbounded. §10's blessing
command is allowed to write; the compiler is not.

---

## 10. Reproducibility, the fixpoint, and why the compiler may never call an LLM

**This is the section that matters.** Everything above is engineering; this is the
part that can quietly destroy the project's central invariant.

### 10.1 What the gate actually proves

`verify.ps1:6-10`:

```
#   1. SELF-HOSTING FIXPOINT.  compiler.zl compiled by the interpreter
#      (gen1.c) builds a compiler that reproduces its own source
#      byte-identically (gen2.c). The property is  f(f(x)) == f(x)  --
#      compare gen1 to gen2, NOT to a hash from last week. The hash
#      legitimately changes whenever compiler.zl changes.
```

Two properties are bundled there and they must be separated:

- **Determinism:** compiling the same source twice gives byte-identical output.
- **Closure:** the compiler compiled by itself reproduces itself.

`verify.ps1` runs `interp.exe compiler.zl`, saves `out.c` as `_fx_gen1.c`, builds
it, runs the result, saves `out.c` as `_fx_gen2.c`, and compares SHA-256
(`verify.ps1:22-35`). Both runs happen **in the same invocation, seconds apart**.

### 10.2 Why that is the worst possible detector for an LLM

An LLM call is nondeterministic across runs but *usually* stable within a few
seconds at temperature 0 with a warm cache. So a `comptime ai_rule(...)` inside
`compiler.zl` would:

- pass `verify.ps1` today,
- pass it tomorrow,
- and fail three weeks later when the provider ships a new model checkpoint —
  at which point the fixpoint goes red, and the person debugging it has every
  reason to believe they broke the parser.

A green gate that goes red for reasons unrelated to the change under test is
worse than no gate. This is not a hypothetical: model checkpoints are versioned
on the provider's schedule, not ours.

### 10.3 The rule

> **The compiler never calls a model. Not cached, not conditionally, not
> behind a flag. There is no network code in `compile.exe`.**

Caching-with-fallback ("call the model on a cache miss") is the tempting design
and it is wrong, because a cache miss is precisely the moment you are least
prepared to notice the output changed. It converts a loud failure into a silent
one.

### 10.4 The protocol instead

Three states, and the compiler only ever sees two of them:

```
1. UNBLESSED   source says   comptime R = ai_rule("...")
               no cache entry for sha256("...")
               ==> COMPILE FAILS, with a message naming the exact
                   command to run. Build is red. Nothing is guessed.

2. BLESSING    a SEPARATE program - `zl bless prog.zl` - calls the model,
               writes the generated zl into rules/<sha256>.zl, and
               writes the pair into blessed.lock. This program is the
               ONLY thing in the project that touches the network.
               It is not invoked by build.bat or verify.ps1.

3. BLESSED     cache entry exists, hash matches, file present.
               ==> comptime reads rules/<sha>.zl, parses it, splices it.
                   No network. Fully deterministic. Offline.
```

`blessed.lock` and `rules/*.zl` are **committed to the repo**. The generated rule
is source code: reviewable, diffable, hand-editable. If a human edits
`rules/<sha>.zl`, that is fine and intended — the hash keys the *English input*,
not the output, so the human's edit survives every subsequent build. (A second
hash over the output would let the build detect the edit; do **not** add one,
because "the AI's version wins over the human's" is exactly backwards.)

### 10.5 What this buys

`build.bat` and `verify.ps1` keep working with the network unplugged. Determinism
is not "usually". Step 5 of MASTER_PLAN §5.2 — *"Prove determinism: rebuild 10×
offline, get byte-identical output every time"* — is satisfied trivially, because
after blessing there is nothing nondeterministic left to prove anything about.

---

## 11. `ai_rule` — the Floor 2 mechanism

MASTER_PLAN §5.2 wants: you write a threat in English, an AI turns it into a
Floor-3 rule at compile time, and the output is **frozen into the source,
readable and editable**. Its "Hard constraint" is that AI output is a draft a
human approves, never something that auto-installs into a kernel driver.

Comptime plus §10's protocol is exactly that machine.

### 11.1 What you write

```
comptime RANSOMWARE = ai_rule(
    "anything spawned by an Office app that rapidly encrypts or
     renames large numbers of user files")

detect "ransomware" { RANSOMWARE }
```

### 11.2 What happens, step by step

1. Parser produces `N_COMPTIME` wrapping a call to `ai_rule` with one string.
2. Comptime pass evaluates it. `ai_rule` is a **comptime-only builtin** — it does
   not exist at runtime, and calling it outside `comptime` is a compile error.
3. It computes `h = sha256(prompt_text || model_id || prompt_template_version)`.
   All three go into the hash: a new template or a new model is a *different
   question* and must miss the cache rather than silently reuse an old answer.
4. Looks up `h` in `blessed.lock`.
   - **Miss:** compile fails: `ai_rule: no blessed output for <h>. Run: zl bless prog.zl`.
   - **Hit:** reads `rules/<h>.zl`, feeds it to `lex_text` (`lexer.h:43`) and the
     parser's slot machinery (`parser.c:151`) — the same path f-strings already
     use — and splices the resulting block in place of the `N_COMPTIME` node.
5. From here it is ordinary zl. The backends see a rule they cannot distinguish
   from a hand-written one, because it *is* one.

### 11.3 What gets frozen

`rules/<h>.zl`, committed, with a generated header:

```
# GENERATED by `zl bless` on 2026-08-02 from:
#   "anything spawned by an Office app that rapidly encrypts or
#    renames large numbers of user files"
# model: <id>   prompt-template: v3   sha256: 5f2a...
# REVIEWED-BY: <name>   REVIEWED-ON: <date>
# Edit this file freely. Regeneration only happens if the ENGLISH changes.

if it.parent in OFFICE_APPS
   and it.writes > 50
   and it.window < 10s
   and it.entropy_delta > 0.7 {
    block(it)!
}
```

`REVIEWED-BY` empty ⇒ **compile fails** when the target is the kernel. That is
MASTER_PLAN §5.2's step 4 (*"nothing AI-written reaches the kernel unreviewed"*)
enforced by the compiler rather than by discipline. It is one `strstr` on the
header and it is the highest-value ten lines in this whole document.

### 11.4 Why this is a language feature and not a script

A script that runs an LLM and pastes output into a file gets you the same text.
What it does not get you:

- **The compiler enforces the review gate.** A script can be skipped.
- **The English lives next to the rule it produced**, in one file, and drifts
  visibly when someone edits one and not the other.
- **The cache key is computed by the thing that consumes it**, so a stale
  blessing is impossible by construction rather than by convention.
- **Unblessed code cannot ship.** With a script, forgetting to run it produces a
  binary missing a detection. Here it produces a red build.

That last one is the argument. Everything else is convenience.

### 11.5 What it needs that does not exist

Honest list:

- **Floor 3's rule language.** `it.parent`, `OFFICE_APPS`, `10s`, `block(it)!` —
  none of that parses today. `.` is unimplemented (`interp.c:1521`) and duration
  literals are an OPEN item in MASTER_PLAN §10. **`ai_rule` is unbuildable until
  Floor 3 lands**, which is exactly what MASTER_PLAN §6 says (*"Floor 2 now sits
  above floor 3"*). This document does not change that ordering; it specifies the
  mechanism so that Floor 2 is a small piece of work when its turn comes.
- **The `MAX_TEXT` problem (§4.2a).** A generated rule is far longer than 128
  characters. Splicing via `lex_text` sidesteps it — the rule never becomes a
  single `N_STRING` — which is a second reason to prefer source-splicing over
  string reification here.
- **A prompt template with a version number**, so §11.2 step 3's hash is
  meaningful.

---

## 12. Risks, and things I am not sure about

**Stated as uncertainties rather than guessed at.**

1. **`compiler.zl` cannot execute `comptime`** (§1.2). So the moment `comptime`
   appears in `compiler.zl` itself, the fixpoint breaks — `interp.exe` would fold
   it and the self-hosted compiler would not. Two ways out: (a) forbid `comptime`
   in `compiler.zl` by convention and add a grep to `verify.ps1`; (b) have
   `compiler.zl` treat `comptime e` as `e` (ignore the keyword), which is
   semantically wrong the instant anything impure or fuel-bounded is involved. I
   lean to (a) and I am not confident. **This needs a decision before
   implementation, not during.**
2. **The gate does not cover this feature.** `verify.ps1`'s only input is
   `compiler.zl`, so it proves closure over one file — and `compiler.zl` already
   mishandles 63 of 110 `.zl` files while staying green. Comptime is tested by
   `run_tests.ps1` or it is untested. Budget a suite; the 8 existing suites and
   2,107 checks are the bar.
3. **Order of evaluation between comptime blocks is unspecified above.** If two
   comptime blocks both assign `X`, which wins? Source order is the obvious
   answer and probably right, but it interacts with the global assignment rule
   (`interp.c:180`) in ways I have not traced. Flagging, not deciding.
4. **Comptime error messages will be bad at first.** An error inside a comptime
   evaluation is reported by `runtime_error`, which knows nothing about the
   compile-time context. A user will see `'x' doesn't exist yet` with no
   indication it came from a comptime block three files away — and zl has no
   module system yet (`design_modules.md` is still a proposal), so "three files
   away" is not even expressible. Needs a wrapper that prefixes the origin.
5. **I did not measure the fuel counter's cost** (§8.3), and `eval` is already
   the slow path. If it turns out to matter, the `#ifdef` escape in §8.3 is the
   answer, but that means two builds of the interpreter with different semantics,
   which is its own hazard.
6. **`MAX_TEXT` (§4.2a) is genuinely unresolved.** I proposed the concatenation
   chain and I am not sure it is right.

---

## 13. Implementation size and dependencies

Honest estimate, in the style of the tree's actual density (~5,370 lines of C for
the whole system, so these numbers are not small in relative terms).

| Piece | Lines | Where | Confidence |
|---|---|---|---|
| `comptime` keyword | 1 | `lexer.c:48` | high |
| `is_keyword` parity fix | 1 | `compiler.zl:36` | high |
| `N_COMPTIME` node type | 1 | `parser.h:11-30` | high |
| Parsing all three forms (§2.1–2.3) | ~60 | `parser.c` | high |
| `#ifndef BUILD_COMPTIME` + `interp.h` | ~30 | `interp.c:1646`, new header | high |
| Deny-list check in `call_builtin` | ~25 | `interp.c:420` | high |
| Fuel counter | ~15 | `interp.c` `eval`/`exec` | medium |
| Reification `Value`→`Node` | ~140 | new `comptime.c` | medium — §4.2 unresolved |
| The comptime pass (tree walk + replace) | ~150 | new `comptime.c` | medium |
| `comptime if` branch deletion | ~40 | `comptime.c` | medium |
| `-D` flag plumbing + manifest emission | ~60 | `compile.c` and friends | medium |
| Option-C read manifest (§9) | ~70 | `comptime.c` | medium |
| Backend `default:` guards (×4) | ~12 | all backends | high |
| Test suite | ~250 | `tests/` | high |
| **Core subtotal** | **~855** | | |
| `zl bless` (separate program, network) | ~300 | new `bless.c` or a script | low — unscoped |
| `ai_rule` comptime builtin + splice | ~120 | `comptime.c` | low — blocked on Floor 3 |
| Review-gate header check | ~10 | `comptime.c` | high |
| **Floor-2 subtotal** | **~430** | | |

**Core: ~855 lines, of which ~600 is new code in one new file.** That is
comparable to `compile.c` (496) or `nativegen.c` (509) — a real feature, not a
weekend.

**Depends on:**

- **Nothing, to start.** §2–§5, §7–§10 build on the tree as it stands today.
  That is unusual among the open designs and is the main argument for doing this
  soon.
- **`design_scoping_decision.md`** — not blocking, but §12.3 (comptime block
  ordering) cannot be settled until it is.
- **`design_records.md` / `design_maps_v2.md`** — not blocking. Reification gains
  one case each when they land; if they land first, that case is written once
  instead of retrofitted.
- **Floor 3's rule language** — hard-blocks §11 and only §11.

**Blocks:** MASTER_PLAN Floor 2 entirely. Floor 2's four sub-steps 2–5
(*compile-time hook, cache by content hash, freeze into source, prove
determinism*) are §10 and §11 of this document and have no other home.

**Recommendation, restated: build the core now, in the order §2 → §4 → §8 → §5 →
§7 → §9 → §10. Ship `ai_rule` when Floor 3 exists. Do not build §6.**
