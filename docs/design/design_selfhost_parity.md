# Design: What `src/selfhost/compiler.zl` Is For — a Parity Decision

**Status:** decision · **Author:** audit pass · **Date:** 2026-08-01
**Scope:** decides the *policy* for `src/selfhost/compiler.zl`. Recommends three small edits to
`src/selfhost/compiler.zl` (~20 lines, no new syntax), one change to `verify.ps1`, and two
corrections to `MASTER_PLAN.md` / `docs/archive/prompts/OVERNIGHT_CAMPAIGN.md`. No code is
changed by this document. Line-number citations are against the tree as it stood
on 2026-08-01; `src/frontend/lexer.c` and `src/frontend/parser.c` were being edited concurrently, so treat
them as pointers, not as anchors.

---

## 0. TL;DR

`src/selfhost/compiler.zl` implements a strict subset of zl. The surface has grown since it
was written and it has not followed. Measured, over all 110 `.zl` files in
`stdlib/`, `tests/` and `examples/`:

| | files | src/selfhost/compiler.zl mishandles |
|---|---|---|
| `stdlib/` | 96 | **49** |
| `tests/` | 8 | **8** |
| `examples/` | 6 | **6** |
| **total** | **110** | **63** |

It cannot compile a single one of the six showcase programs in `examples/`, and
it cannot compile any of the eight test suites — including
`tests/test_syntax.zl`, the file whose entire job is to exercise the surface.

`verify.ps1` is green anyway, and correctly so: the fixpoint proves **closure**
(`src/selfhost/compiler.zl` compiles the language `src/selfhost/compiler.zl` is written in), never
**coverage**. Measured — `src/selfhost/compiler.zl`, `src/selfhost/lexer.zl` and `src/selfhost/parser.zl` score zero on
every divergence in this document. That is the property working, not failing.

**Recommendation: keep `src/selfhost/compiler.zl` as a bounded-subset engine (option a), but
stop letting it guess.** It is not an outlier — four of the five backends are
deliberate subsets. It is the only one that does not *say so*: `src/backends/llvm/compilel.c`,
`src/backends/c/compilef.c`, `src/backends/native/nativegen.c` and `src/backends/native/nativeval.c` all print "not supported yet" and
exit; `src/selfhost/compiler.zl` prints a line to stdout and keeps going, so its three failure
modes today are *hang*, *broken C*, and *C that compiles and does the wrong
thing*. Three edits (~20 lines, no new surface) collapse all three into one
honest one: a printed message and no `out.c`.

Full parity is rejected on a measured cost of ~320 lines that must themselves be
written in the pre-2026-07-30 subset, plus a permanent 2–3× tax on exactly the
two waves (W2, W3) that produce the speed win the project is now steering by.

---

## 1. What the fixpoint proves, and what everyone thinks it proves

`verify.ps1` check 1 is `f(f(x)) == f(x)`: the interpreter compiles
`src/selfhost/compiler.zl` → `gen1.c`; `gen1.c` builds a compiler; that compiler compiles
`src/selfhost/compiler.zl` → `gen2.c`; the two must be byte-identical.

That is a closure property over **one input**. It says nothing about any other
program. `src/selfhost/compiler.zl` uses only what `src/selfhost/compiler.zl` implements, so the fixpoint
holds no matter how far the surface runs ahead.

Two planning documents state the opposite:

> Every one of these touches lexer+parser+interp+compile AND must be mirrored in
> `src/selfhost/compiler.zl` or the fixpoint dies.
> — `docs/archive/prompts/OVERNIGHT_CAMPAIGN.md:64-65`, and again at `MASTER_PLAN.md:742`

The fixpoint does not die. Every W2 feature so far — `elif`, ternary, `in`,
f-strings, ranged `for`, `do`/`while`, `loop`, `break`/`continue`, compound
assignment, index assignment, `\xNN` — landed in `src/frontend/lexer.c` / `src/frontend/parser.c` /
`src/runtime/interp.c` / `src/backends/c/compile.c` and in none of them did `verify.ps1` go red. The
policy in those two lines was never enforced because nothing enforces it, and
nothing *can*: the gate compiles one file, and that file does not use the new
syntax.

The gate is also deaf. `verify.ps1:21` is `.\interp.exe src/selfhost/compiler.zl | Out-Null`
— `src/selfhost/compiler.zl`'s diagnostics are discarded, and `src/selfhost/compiler.zl` never sets a
non-zero exit code (§2.4). Even if the fixpoint input *did* trip a parse error,
the gate would not notice.

---

## 2. The measured gap

### 2.1 Reserved words

`src/frontend/lexer.c:48-54` — 15 keywords:

```
if  else  elif  for  in  fn  return  while  not  and  or  true  false
break  continue
```

`src/selfhost/compiler.zl:37` — 12:

```
if  else  while  for  in  fn  return  not  and  or  true  false
```

Missing: **`elif`, `break`, `continue`**.

`src/frontend/parser.c` also recognises four words *positionally* without reserving them —
`to` and `step` (only inside a `for … =` header, `src/frontend/parser.c:731-740`), and `do`
and `loop` (only when glued to a `{`, `src/frontend/parser.c:905-908`). `src/selfhost/compiler.zl` knows
none of the four.

### 2.2 Feature by feature

"C toolchain" here means the reference pair — `src/frontend/lexer.c`+`src/frontend/parser.c` fronting
`src/runtime/interp.c` and `src/backends/c/compile.c`. Between them they cover the whole `NodeType` set in
`src/frontend/parser.h`; the single exception is `.` member access, which `src/backends/c/compile.c` refuses
with an error and an exit (§4), and which nothing in the corpus uses.

| Surface feature | C toolchain | `src/selfhost/compiler.zl` | what happens |
|---|---|---|---|
| `if` / `else` / `while` / `for v in seq` / `fn` / `return` | yes | **yes** | — |
| `not` / `and` / `or`, unary `-`, `==` `!=` `<=` `>=` `<` `>` | yes | **yes** | — |
| list literals, indexing `xs[i]`, calls, strings, ints, bools | yes | **yes** | — |
| `elif` | `src/frontend/parser.c:581-589` | **no** | HANG |
| `else if` | `src/frontend/parser.c:590-599` | **no** (`parse_block` at `:299` demands `{`) | WRONG C |
| ternary `a ? b : c` | `src/frontend/parser.c:537-549` | **no** | HANG |
| `in` / `not in` as an operator | `src/frontend/parser.c:457-487` | **no** (`in` is eaten blind by `parse_for`) | HANG |
| `for i = a to b [step n]` | `src/frontend/parser.c:724-779` | **no** | HANG |
| `do { } while c` | `src/frontend/parser.c:818-847` | **no** | HANG |
| `loop { }` | `src/frontend/parser.c:850-857` | **no** | HANG |
| `break` / `continue` | `src/frontend/parser.c:920-921`, `src/backends/c/compile.c:380-381` | **no** | BROKEN C |
| compound assign `+= -= *= /= %=` | `src/frontend/lexer.c:247-250`, `src/frontend/parser.c:948-956` | **no** | BROKEN C |
| index assign `xs[i] = v` (and nested) | `src/frontend/parser.c:929-935`, `src/backends/c/compile.c:312-329` | **no** | BROKEN C |
| f-strings `f"x is {v}"` | `src/frontend/lexer.c:303-308`, `src/frontend/parser.c:202-264` | **no** | HANG / BROKEN C |
| float literals `3.14` | `src/frontend/lexer.c:110-113` | **no** (digit loop only, `:72-79`) | HANG |
| `\xNN` escapes | `src/frontend/lexer.c:187-195` | **no** — decodes to the letter `x` | **SILENT WRONG OUTPUT** |
| unknown escape `\q` kept verbatim | `src/frontend/lexer.c:199-203` | **no** — drops the backslash | **SILENT WRONG OUTPUT** |
| `\0` refused as a lex error | `src/frontend/lexer.c:205-209` | **no** — becomes the digit `0` | **SILENT WRONG OUTPUT** |
| newlines are tokens (`return` ends at EOL) | `src/frontend/lexer.c:283-288`, `src/frontend/parser.c:889` | **no** — newline is whitespace (`:62`) | wrong tree for a bare `return` |
| `.` member access | `src/frontend/parser.c:347-355` | **no** | HANG |
| `!` danger marker | `src/frontend/parser.c:364-369` | **no** | HANG |
| call with a non-identifier callee | `src/frontend/parser.c:338` (`call->a` is a node) | **no** — `:220` takes `node[1]` | garbage name |
| unknown character is a lex error | `src/frontend/lexer.c:256-258` | **no** — any byte becomes a `SYM` | junk accepted |

Nothing on the **builtin** side is missing: `src/selfhost/compiler.zl:509` emits
`zl_calln("<name>", n, …)` for anything that is not a user function, so all 93
builtins reach `src/runtime/runtime.c` unchanged. **The entire gap is syntax.**

### 2.3 The three failure modes, traced

`src/selfhost/compiler.zl` has no error exit. `expect` (`:158-164`) prints and returns.
`parse_primary` (`:202-203`) prints and returns `["num","0"]` **without
advancing the cursor**. Everything downstream carries on. That single missing
`advance()` is what turns most gaps into a hang rather than a message.

**HANG.** `elif x == 2 {` → `elif` parses as an identifier expression; `x == 2`
parses as a second expression statement; then the cursor sits on `SYM {`, which
`parse_primary` cannot start and does not consume. `parse_block`'s
`while not is_sym("}") and kind() != "EOF"` and `parse_program`'s
`while kind() != "EOF"` both spin forever, printing `PARSE ERROR at: {` until
killed. Identical shape for `?`, `in`, `to`, `do`, `loop`, `.`, `!` and the `.`
of a float literal.

**BROKEN C.** `xs[0] = 1` reaches `parse_statement:382-388`, which builds
`["assign", e[1], v]`. For an index target `e` is `["index", ["name","xs"],
["num","0"]]`, so `e[1]` is a *list*, and `emit_stmt:542` computes
`"v_" + node[1]`. zl's `+` stringifies mixed operands (`src/runtime/interp.c:1111-1117`), so
`out.c` gets:

```c
v_[name, xs] = zl_num(1);
```

For this one **no parse error is printed at all** — `cl` is what finally
objects, about a line of C nobody wrote. `break` is equally quiet: it is just an
identifier, so it emits `v_break;` and `cl` reports an undeclared identifier.
`x += 1` does print (`parse_primary` chokes on the bare `=`), then recovers into
`["assign","+",…]` and emits `v_+ = zl_num(1);`.

**WRONG C — the worst class.** `else if` is the one that compiles. `parse_if`
(`:314-317`) eats `else` and calls `parse_block`, which prints
`PARSE ERROR: expected { got if` and then *keeps parsing statements until it
finds a `}`* — which is the enclosing function's closing brace. Everything
after the `if` chain, to the end of the file, is silently absorbed into that
`else` branch. The emitted C is syntactically valid, `cl` accepts it, the
program runs, and `main` is empty. Two lines of diagnostics scroll past on
stdout and the exit code is 0.

### 2.4 The divergence no parse error can catch

`src/selfhost/compiler.zl:39-50`:

```
fn unescape(e) {
    if e == "n" { return "\n" }
    if e == "t" { return "\t" }
    if e == "r" { return "\r" }
    return e
}
```

`\\` → `\` and `\"` → `"` agree with `src/frontend/lexer.c` by luck of the fall-through. But:

| written | `src/frontend/lexer.c` | `src/selfhost/compiler.zl` |
|---|---|---|
| `"\x1b"` | ESC (0x1B) | the two characters `x` … then `1b` copied literally → `x1b` |
| `"\q"` | `\q` (backslash kept, `src/frontend/lexer.c:199-203`) | `q` |
| `"\0"` | hard lex error (`src/frontend/lexer.c:205-209`) | `0` |

`stdlib/ansi.zl:25-27` is `ESC = "\x1b"`, `CSI = "\x1b["`, `RESET = "\x1b[0m"`.
Compiled by `src/selfhost/compiler.zl` that module produces valid C, links, runs, and prints
`x1b[0m` where an escape sequence belongs. This is the only class the fixes in
§6 cannot convert into a diagnostic, because no parser sees it.

---

## 3. The consequence, measured

### 3.1 Method — state it before the numbers

A ~120-line Python scanner (scratch, not committed) reads all 110 `.zl` files
under `stdlib/` (96), `tests/` (8) and `examples/` (6). Per line it strips `#`
comments and string literals with a small state machine, keeping the literals
aside so escape sequences can be examined separately, then pattern-matches the
remaining code text. `for v in seq` headers are excluded from the `in`-operator
count by a look-behind, so only genuine operator uses are counted.

Three honest caveats:

1. **Nothing was executed.** `src/selfhost/compiler.zl` hard-codes `read("input.zl")` and
   `write("out.c")`, and this pass was read-only in the main checkout with
   another agent editing engine source. Every failure mode in §2.3 is derived by
   tracing `src/selfhost/compiler.zl`'s parser by hand, not by running it.
2. **Counts are a lower bound on constructs.** A pattern can miss; it rarely
   invents. Per-file verdicts could in principle carry a false positive, so the
   three largest signals (float literals, index assignment, `else if`) were
   spot-checked line by line against `rg` output on the raw files.
3. **Features nobody uses are still counted as gaps** in §2.2 but contribute
   zero to §3.2 — which is the whole point of option (c) below.

### 3.2 Per-feature usage

Files containing at least one use / total occurrences:

| feature | stdlib | tests | examples | occurrences |
|---|---|---|---|---|
| float literal | 15 | 7 | 3 | **475** |
| index assignment | 26 | 4 | 6 | **294** |
| compound assignment | 3 | 2 | 5 | **291** |
| `break` | 11 | 4 | 6 | 74 |
| `else if` | 17 | 1 | 0 | 65 |
| ternary `? :` | 5 | 1 | 4 | 63 |
| `elif` | 3 | 1 | 1 | 37 |
| `continue` | 6 | 1 | 3 | 35 |
| `in` / `not in` operator | 0 | 3 | 1 | 35 |
| `\xNN` escape | 1 | 3 | 1 | 32 |
| f-string | 0 | 1 | 3 | 4 |
| unknown escape `\q` | 0 | 1 | 0 | 2 |
| bare `return` before EOL | 0 | 0 | 1 | 1 |
| `for i = a to b` | 0 | 0 | 0 | **0** |
| `do { } while` | 0 | 0 | 0 | **0** |
| `loop { }` | 0 | 0 | 0 | **0** |
| `.` member access | 0 | 0 | 0 | **0** |
| `!` danger marker | 0 | 0 | 0 | **0** |
| statement starting `-` `(` `[` (the newline hazard) | 0 | 0 | 0 | **0** |

### 3.3 Files, by what would happen to them

The three failure columns overlap — most affected files use several features —
so they do not sum to **any**. A file's real fate is its *worst* column, and a
hang wins, because the run never reaches codegen at all.

| | files | hang | broken C | wrong C | **any** | clean |
|---|---|---|---|---|---|---|
| `stdlib/` | 96 | 17 | 31 | 18 | **49** | **47** |
| `tests/` | 8 | 7 | 4 | 4 | **8** | 0 |
| `examples/` | 6 | 6 | 6 | 2 | **6** | 0 |

Ten `stdlib` modules land in the **wrong-C-only** bucket — they use nothing but
`else if`, so `src/selfhost/compiler.zl` emits code that builds and misbehaves:
`base64`, `binarytree`, `brainfuck`, `csv_parse`, `csvw`, `diff`, `encoding`,
`lisp_interp`, `rpn_calc`, `uuid`.

The 47 clean modules are real and worth naming as an asset, not a consolation:
`base_convert bignum bitops calculus date_calc dijkstra fibonacci fraction
game_of_life graph_search hashset huffman json_pretty jsonw lcs levenshtein
linkedlist list_reduce listx markdown math_basics mathx matrix_ops maze_solve
numtheory primes quicksort regex_match roman_math roman_numerals run_length
searching set sorting stack_queue statemachine stats string_analysis
string_utils strx sudoku template_engine testlib text_table tic_tac_toe
tiny_calc vec`.

### 3.4 The control

`src/selfhost/compiler.zl`, `src/selfhost/lexer.zl` and `src/selfhost/parser.zl` score **zero** on every row of §3.2.
That is closure, measured. The self-hosted compiler is not accidentally
consistent — it is exactly, and only, consistent with itself.

---

## 4. The reframe: `src/selfhost/compiler.zl` is not the odd one out

Before choosing, the honest comparison. `src/frontend/parser.h` defines 23 `NodeType`s. Six
engines consume them, and only one consumes all 23:

| engine | distinct `case N_*` arms | announces its subset? |
|---|---|---|
| `src/runtime/interp.c` | 22 (+`N_PROGRAM` handled as a block) — the reference | n/a |
| `src/backends/c/compile.c` (boxed C) | 22 — everything but `N_MEMBER` | yes — `src/backends/c/compile.c:282-284` exit(1) |
| `src/backends/native/nativeval.c` (native self-host) | 16 | mostly — `src/backends/native/nativeval.c:401` |
| `src/backends/llvm/compilel.c` (LLVM IR) | 14 | yes — `src/backends/llvm/compilel.c:176,312` "not supported yet" + exit |
| `src/backends/c/compilef.c` (unboxed C) | 11 | yes |
| `src/backends/native/nativegen.c` (x86-64) | 10 | yes — `src/backends/native/nativegen.c:277` |
| **`src/selfhost/compiler.zl`** | **18 equivalent forms** | **no** |

Read `src/selfhost/compiler.zl`'s 18 sceptically — **node counting flatters it.** It has no
`N_TERNARY`, `N_DANGER`, `N_MEMBER`, `N_BREAK` or `N_CONTINUE`, which is where
the 18 comes from; but the metric is blind to every lexer-level gap (floats,
`\xNN`, f-strings) and to the sub-form gaps inside nodes it *does* have — an
`N_ASSIGN` whose target is an index, an `N_ASSIGN` carrying a compound operator,
an `N_IF` reached via `elif`. §2.2 is the real measure; this table is only here
to place `src/selfhost/compiler.zl` in the right class.

`MASTER_PLAN.md:146` already concedes the general shape of this —
*"Coverage is the real gap. The fast engines are fast partly because they refuse
to compile most of the language."* The fast engines **refuse**. `src/selfhost/compiler.zl`
guesses.

Two further facts from the same reading, both of which bear directly on the
decision:

- **`src/backends/native/nativeval.c:160-170` wires exactly seven builtins** — `len push at has read
  write print` — with the comment *"Only the ones the self-host needs."*
  `src/selfhost/compiler.zl` is not merely inside a syntax budget; it is inside a **builtin**
  budget of seven. It cannot call `exit()`, which is why the fix in §6.1 does
  not use one.
- **`src/backends/native/nativeval.c:319-323` handles `N_ASSIGN` by ignoring `n->text`.** For a
  compound assignment `x += 1`, `src/frontend/parser.c:948-956` puts the operator in `text`
  and the right-hand value in `b` — so the native backend would silently emit
  `x = 1`. Not reachable from `src/selfhost/compiler.zl` (which never writes `+=`), but it is
  a live silent-miscompile for any other program fed to `nativeval.exe`, and it
  is evidence that the drift is systemic rather than a `src/selfhost/compiler.zl` problem.

---

## 5. The three options, costed

The project direction is no longer "delete C". `MASTER_PLAN.md:118-120`:
*"the tower is no longer the bottleneck — capability and speed are"*, and
`:328` records step 10 ("delete the C") as **"deliberately deprioritised
2026-07-30 in favour of speed + capability"**. Every cost below is weighed
against that.

### 5.1 (a) Frozen subset — accept permanent drift

`src/selfhost/compiler.zl` stays a self-hosting kernel: the proof that zl-the-subset compiles
zl-the-subset, held at its current surface forever.

- **Cost, W2–W5:** zero. Every syntax feature lands in four C files, none in zl.
- **Cost, honesty:** "self-hosting" needs a qualifier in every sentence that uses
  it. Today the qualifier is "47 of 110 corpus files".
- **Cost, deferred:** the day "delete C from the source" is re-promoted, the
  whole bill arrives at once — and it will be bigger than today's ~320 lines,
  because W3 adds `:` and `->` and W5 adds structs and raw memory.
- **Cost, unmanaged:** as literally stated ("accept drift"), nothing bounds it,
  nothing measures it, and the failure modes stay silent. This is the defect
  §6 fixes.

### 5.2 (b) Full parity — every feature implemented twice

Measured estimate against the current `src/selfhost/compiler.zl` (716 lines):

| gap | lines in `src/selfhost/compiler.zl` | note |
|---|---|---|
| f-strings | ~65 | brace scanner + `{{`/`}}` + **save/restore of the four parser globals** (`src`, `spos`, `toks`, `pos`) for the nested parse |
| `for i = a to b step n` | ~50 | full desugar; needs ternary and `break` to exist first |
| `do { } while` | ~28 | same dependency |
| compound assignment | ~27 | lexer two-char + parse + emit, incl. the index-target form |
| `in` / `not in` | ~26 | needs a lookahead helper `src/selfhost/compiler.zl` does not have |
| `\xNN` + escape fidelity | ~20 | `unescape` must return (char, chars-eaten) — a 2-element list |
| newline tokens | ~30 | touches **every** parse function |
| index assignment | ~18 | keep the target node; emit `zl_set` |
| ternary | ~16 | `?` and `:` already lex via the `SYM` fallback (`:113-114`) |
| `.` member / `!` danger | ~12 | danger is a pass-through |
| `elif` + `else if` | ~12 | |
| `break` / `continue` | ~10 | keywords + parse + emit |
| float literals | ~8 | |
| `loop` | ~6 | |
| **total** | **~320** | a 45 % growth of the file |

The line count is not the expensive part. Two structural costs are:

1. **It must be written in the subset it does not yet have.** A brace scanner for
   f-strings, written without f-strings, without a ternary, without `in`,
   without `+=`, without `xs[i] = v`. Not impossible — the existing 716 lines
   prove it — but it means the *implementation language* of the compiler is
   frozen at its pre-2026-07-30 shape permanently, because implementing feature
   X can never use X.
2. **It forks into two sub-options, and one of them is dangerous.**
   - **(b1) implement, never use.** `src/backends/native/nativeval.c` is untouched, because it only
     has to compile what `src/selfhost/compiler.zl` *writes*. This is the safe form, and it
     is also the strange one: you build `elif` in zl and are then forbidden to
     type it.
   - **(b2) implement and use.** The natural, human thing. But then
     `src/backends/native/nativeval.c` must learn each feature in **hand-written x86-64** — ternary,
     `break`, `continue`, `zl_set` stores, compound assignment are all absent
     today (§4) — and Floor 4's trophy, `nvout.exe`, is in the blast radius of
     every single W2 item. `MASTER_PLAN.md:449` is explicit about what that
     costs: *"each one becomes another hand-assembly job."*

Against a project steering by speed, (b) taxes W2 and W3 — the two waves that
produce the 4–6× — by roughly 2× (b1) or 3× (b2), to buy a capability that was
deliberately deprioritised two days ago.

### 5.3 (c) Parity with what the corpus actually uses

Attractive in the abstract. The measurement kills it.

The corpus uses **10 of the 14** cost items in §5.2. The four it never touches —
ranged `for`, `do`/`while`, `loop`, `.`/`!` — are worth ~96 of the ~320 lines.
So "corpus parity" saves **30 %** of the work of full parity, and in exchange:

- The obligation becomes a **moving target**: the first `stdlib` module that
  uses `loop { }` silently re-opens it, and nothing detects that.
- It produces a `src/selfhost/compiler.zl` that is neither minimal (so it can no longer be
  read as a teaching artifact) nor complete (so it can never be trusted as a
  compiler). It has the documentation story of neither.
- It requires the §3 measurement to be re-run and re-argued on every wave.

70 % of the cost, none of the finality, plus a recurring meeting. Rejected.

---

## 6. Recommendation — (a′): bounded, announced, gated

**Keep `src/selfhost/compiler.zl` at its current subset. Move it explicitly into the same
class as `src/backends/llvm/compilel.c` and `src/backends/native/nativegen.c` — a partial engine that refuses what it
cannot do.** Freezing the *surface* is right; leaving the *behaviour*
undiagnosed is not, and that is a separate and much cheaper problem.

### 6.1 Three code changes to `src/selfhost/compiler.zl` (~20 lines, no new syntax)

None of these grows the subset, so none of them can be the feature that breaks
the fixpoint, and none needs a new builtin (which matters — `src/backends/native/nativeval.c` wires
only seven, §4).

1. **Guarantee progress.** Add `advance()` to `parse_primary`'s error path
   (`:202-203`) before it returns the dummy node. One line. The entire HANG
   class — `elif`, `?`, `in`, `to`, `do`, `loop`, `.`, `!`, floats, f-strings —
   becomes a finite run that prints errors and terminates.
2. **Refuse to emit.** A global `g_err = false`; set it in `expect` (`:158-164`)
   and in `parse_primary`'s error path; in the main block (`:712-717`), if
   `g_err` then print one line and **do not call `write("out.c", …)`**. Four
   lines. This deletes the WRONG-C class outright — including the `else if`
   swallow-the-rest-of-the-file case, which is the only failure in this document
   that currently produces a *running program with different behaviour*.
3. **Tell the truth about escapes.** Rewrite `unescape` (`:39-50`) to decode
   `\xNN` and to preserve the backslash on an unknown escape, matching
   `src/frontend/lexer.c:180-203`. ~15 lines, all inside the subset. This is the one
   divergence neither (1) nor (2) can catch (§2.4), and `stdlib/ansi.zl` is a
   live victim of it.

After these three, `src/selfhost/compiler.zl` has exactly one failure mode: *it prints what it
does not support and produces nothing*, which is what the other four partial
backends already do.

### 6.2 One change to the gate

`verify.ps1:21` discards `src/selfhost/compiler.zl`'s stdout. Capture it instead and fail
check 1 if it contains `PARSE ERROR` — or, once §6.1(2) lands, simply keep the
existing `if (-not (Test-Path out.c))` arm, which then fires correctly for the
first time. The gate stays two checks and one command.

### 6.3 Two documentation corrections

1. Delete the claim at `MASTER_PLAN.md:742` and
   `docs/archive/prompts/OVERNIGHT_CAMPAIGN.md:64-65` that new syntax "must be mirrored in
   `src/selfhost/compiler.zl` or the fixpoint dies." It is false, it has never been followed,
   and leaving it in place means every future reader mis-scopes their W2 work.
   Replace it with the subset policy: *new surface lands in `src/frontend/lexer.c`,
   `src/frontend/parser.c`, `src/runtime/interp.c`, `src/backends/c/compile.c`; `src/selfhost/compiler.zl` is frozen at its documented
   subset and must only be edited to stay inside it.*
2. Write the subset down where someone will see it — a block comment at the top
   of `src/selfhost/compiler.zl` listing its 12 keywords, its statement forms, its escape set,
   and — explicitly — what it does *not* accept. `src/backends/llvm/compilel.c`'s header comment
   ("Values are unboxed i64") is the model: one paragraph, stating the limit
   before the reader discovers it.

### 6.4 One tracked number

Keep the §3 scanner as a committed script and print one line: *"src/selfhost/compiler.zl
subset: 47/110 corpus files."* It is allowed to fall only as a recorded
decision, never by accident. This converts drift from invisible to boring, which
is the only thing wrong with option (a) as usually stated.

### 6.5 What this costs, stated plainly

- The word "self-hosting" carries a footnote from now on. The footnote is
  `47/110` and it will get worse: W3 alone adds `:` and `->`, W5 adds structs.
- The bill for "delete C from the source" keeps accruing, and this document is
  the record of it. Today: ~320 lines in `src/selfhost/compiler.zl` (b1), or ~320 plus four
  new hand-assembled forms in `src/backends/native/nativeval.c` (b2).
- A second independent implementation of the grammar has real value as a spec
  cross-check — `design_type_system.md` §2.2 found the `->`-inside-`brainfuck.zl`
  hazard precisely by reasoning about both lexers. Freezing `src/selfhost/compiler.zl` gives
  that up.

That is the trade, and against POWER-and-SPEED it is the right side of it.

---

## 7. What would have to become true to change this

Concrete, checkable triggers. Any one of them re-opens the decision in favour of
(b1):

1. **"Delete C from the source" returns to the critical path** (`MASTER_PLAN.md`
   Floor 1 step 10 un-deprioritised). Then `src/selfhost/compiler.zl` must compile the
   stdlib, not just itself, and parity stops being optional. This is the big one.
2. **Anyone ships a zl program built by the zl compiler.** The moment
   `src/selfhost/compiler.zl` is the tool of record for anything other than its own fixpoint,
   `47/110` is a defect and not a policy.
3. **The corpus starts failing `src/backends/c/compile.c` too.** Today the gap is
   `src/selfhost/compiler.zl`-only; the reference pair tracks `src/frontend/parser.c` (bar `N_MEMBER`,
   which nothing uses). If `src/runtime/interp.c` and `src/backends/c/compile.c` ever fall behind
   `src/frontend/parser.c` on something real, the problem is no longer about `src/selfhost/compiler.zl`
   and this document does not govern it.
4. **A `src/selfhost/compiler.zl` rewrite becomes mechanical** — e.g. the parser is
   generated from a shared grammar table rather than hand-written twice. That
   collapses the ~320 lines to near zero and parity becomes free. Nothing in the
   plan proposes this; if something does, revisit.
5. **The tracked number in §6.4 falls without a decision.** That is drift the
   policy failed to bound, and the policy then needs to change rather than the
   number.

Explicitly **not** triggers: `src/selfhost/compiler.zl` looking dated; a new syntax feature
being easy to add; the number `47/110` being aesthetically annoying.

---

## 8. Risks

| # | Risk | Mitigation |
|---|---|---|
| R1 | §6.1's edits change `src/selfhost/compiler.zl` and break the fixpoint. | All three are on paths never taken while compiling a valid program, and none adds syntax. `verify.ps1` after each one, separately, per the standing serial rule. |
| R2 | §6.1(3) changes `unescape`, which *is* on the hot path for every string literal in `src/selfhost/compiler.zl`'s own source. | This is the only one with real fixpoint exposure. `src/selfhost/compiler.zl` contains no `\xNN` and no unknown escapes (measured: zero), so the new branches are inert for the fixpoint input — but verify it alone, and diff `gen1.c` against the previous `gen1.c` to confirm the emitted C is byte-identical. |
| R3 | Freezing is read as abandoning, and `src/selfhost/compiler.zl` rots into something that no longer even self-hosts. | It is still gated by `verify.ps1` check 1 on every commit. Frozen means "surface frozen", not "unmaintained". |
| R4 | The 47/110 number is treated as a target and someone "improves" it feature by feature — sliding into option (c) by accident. | §5.3 is the standing rejection. Coverage moves only as a recorded decision. |
| R5 | The §3 numbers are pattern-derived and one of the per-file verdicts is wrong. | Directionally irrelevant — `tests/` is 8/8 and `examples/` is 6/6, and those hold even if several stdlib verdicts are wrong. The three biggest signals were spot-checked (§3.1). |
| R6 | `src/backends/native/nativeval.c`'s silent compound-assignment miscompile (§4) is left alone because this document declared it out of scope. | It is out of scope *here* and should be its own fix. Recorded so it is not lost. |

---

## 9. Definition of done

1. `src/selfhost/compiler.zl` cannot hang on any input: the unknown-token path advances.
2. `src/selfhost/compiler.zl` never writes an `out.c` it knows is wrong: any parse error
   suppresses emission and the process says so.
3. `src/selfhost/compiler.zl`'s string escapes agree with `src/frontend/lexer.c` character for character,
   including `\xNN` and unknown escapes.
4. `verify.ps1` fails if `src/selfhost/compiler.zl` reports a parse error on its own source.
5. `src/selfhost/compiler.zl`'s header states its subset — 12 keywords, its statement forms,
   its escape set, and what it refuses — the way `src/backends/llvm/compilel.c`'s header states
   its own.
6. `MASTER_PLAN.md:742` and `docs/archive/prompts/OVERNIGHT_CAMPAIGN.md:64-65` no longer claim
   that new syntax must be mirrored or the fixpoint dies, and state the subset
   policy instead.
7. One committed script prints the corpus coverage number, and `47/110` is
   recorded as the 2026-08-01 baseline.
8. `.\verify.ps1` is green.
