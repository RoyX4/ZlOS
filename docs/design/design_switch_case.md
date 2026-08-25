# Design: `switch` for zl

**Status:** proposal · **Wave:** W2 (syntax and surface) · **Date:** 2026-08-01
**Scope:** one new statement form. No code is changed by this document.

`MASTER_PLAN.md` §3.2 lists `switch` in the W2 row, and
`docs/archive/prompts/OVERNIGHT_CAMPAIGN.md:57` spells it out as *"`switch`/`match` with
fallthrough rules"*. This is the design pass for it.

---

## 0. TL;DR

```
switch tok {
    "+", "-"          { emit_add_sub(tok) }
    "*"               { emit_mul() }
    "("               { depth = depth + 1 }
    else              { error("unexpected " + tok) }
}
```

- **`switch` is not a reserved word.** It is recognised positionally, the same
  way `do` and `loop` already are (`src/frontend/parser.c:905-908`).
- **No `case` keyword either.** An arm is `value (, value)* block`.
- **The default arm is `else`** — a keyword the language already owns.
- **Cases are values, compared with `==`.** Not patterns, not ranges.
- **No fallthrough**, for a reason that is specific to zl and not a matter of
  taste: `break` already means "leave the enclosing loop", and under the
  proposed implementation it keeps meaning exactly that.
- **It is a statement**, not an expression. The ternary already covers value
  selection, and there is nowhere to hoist the subject in an expression.
- **Implementation is a desugaring in `src/frontend/parser.c`** into the `assign` + `if` /
  `elif` tree that already exists — so `src/runtime/interp.c` and `src/backends/c/compile.c` need **zero
  changes**, exactly as with `for i = a to b`, `do/while` and `loop`.
- Two of the four backends (`src/backends/c/compilef.c`, `src/backends/native/nativegen.c`) need a **three-line**
  addition each, and that addition retroactively fixes the three loop forms
  that already land there and fail.

---

## 1. Motivation — what `if`/`elif` does not give

zl already has `if`/`elif`/`else` (`src/frontend/parser.c:574-601`), a ternary
(`src/frontend/parser.c:537-549`), and `in` / `not in` (`src/frontend/parser.c:457-487`). Any switch can
be written today. So the question is not "is it possible" but "is the
`if`-chain spelling costing something real".

It is, and the evidence is in the tree already:

- **`src/selfhost/compiler.zl:36-37` is a twelve-way `or` chain** on one line, testing one
  variable against twelve string literals. That is exactly the shape a switch
  exists for. It is not badly written — it is what the language currently
  offers.
- **A dispatch reads as a table, not as a decision tree.** `if tag == "if" ...
  elif tag == "while" ... elif tag == "for"` (`src/selfhost/compiler.zl:550-565`) repeats the
  subject on every line. The repetition is where the typos go: the reader has to
  check twelve times that it is still `tag` being tested.
- **Repeating the subject means re-evaluating it.** `if f() == 1 { } elif f() ==
  2 { }` calls `f()` twice. There is no way to say "evaluate once, then
  dispatch" without inventing a temporary by hand.

That last point is the only one that is a *semantic* gap rather than an
ergonomic one, and it is what makes the feature worth a parser change rather
than a style guide entry.

Scope discipline: this adds **one statement form, zero reserved words, zero new
`NodeType`s, zero built-ins**.

---

## 2. Surface syntax

### 2.1 The word `switch` — keyword or positional?

`src/frontend/lexer.c:29-45` is unusually explicit about the cost of a keyword:

> *A keyword is a word the language steals from you. [...] Every word added here
> is a name your users lose forever.*

The list is fifteen words (`src/frontend/lexer.c:48-54`). `design_modules.md` §2 turned down
a whole feature shape partly to keep from adding to it. So the default answer
should be "not a keyword", and the question becomes whether positional
recognition is actually safe.

`src/frontend/parser.c:905-908` shows the existing technique:

```c
static int starts_block_word(const char *word)
{
    return curtype() == T_IDENT && is_text(word) && next_is_sym("{");
}
```

`do` and `loop` are ordinary identifiers that only lead a statement when a `{`
is glued straight onto them — and `IDENT {` was a syntax error before, so
nothing that used to parse changes meaning. That invariant is stated as a design
rule at `src/frontend/parser.c:623-626` and it is the thing to preserve.

`switch` cannot use the one-token version, because a subject expression sits
between the word and the `{`. Three ways to close that gap:

| | Make `switch` a keyword | One-token guard (next token is IDENT/NUMBER/STRING) | Scan forward to `{` |
|---|---|---|---|
| Reserved words added | 1 | 0 | 0 |
| Preserves "text that parses keeps its meaning" | no (`switch = 1` breaks) | **almost** — breaks `switch x` as a bare statement | **yes** |
| Allows `switch (a + b) {` | yes | no | yes |
| Lines of parser | ~1 | ~3 | ~12 |
| Keeps `src/frontend/lexer.c` and `src/selfhost/compiler.zl:36` keyword lists in sync | **no** (see §8) | yes | yes |

**Recommendation: scan forward to `{`.**

The scan is total and needs no backtracking, because `{` cannot occur inside a
zl expression — there are no map literals, no lambdas, no block-expressions —
and because `T_NEWLINE` is a real token (`src/frontend/lexer.c:267-268`), so an expression
can never cross a line:

```
'switch' leads a switch statement  iff  scanning forward from the next
token, a T_SYMBOL "{" is reached before a T_NEWLINE or T_EOF.
```

Check it against everything that parses today:

- `switch = 5` → newline before any `{` → ordinary assignment. Unchanged.
- `switch(x)` as a call statement → newline first → ordinary call. Unchanged.
- `switch` alone on a line → newline first → ordinary expression statement.
- `switch x { ... }` → `{` first → a switch. This was a syntax error before
  (a bare `{` is not a statement: `parse_statement` falls through to
  `parse_expr`, and `parse_primary` has no `{` case).

So the invariant at `src/frontend/parser.c:623-626` holds exactly, and `switch` stays a legal
variable name everywhere else. `stdlib/` (96 modules) and `tests/` contain no
identifier named `switch`, `case`, or `default` today, so nothing breaks either
way — but the rule is what makes that a guarantee rather than a coincidence.

The one-token guard is the cheap fallback if the scan turns out to be fussy in
`src/selfhost/compiler.zl`'s hand-written lexer loop. It costs only `switch (a + b) {`, which
is spelled `s = a + b` then `switch s {`.

### 2.2 No `case` keyword

Inside the switch body the parser is in a grammar we control completely, so
there is no ambiguity to resolve and therefore nothing for `case` to buy:

```
switch grade {
    "A" { print("top") }
    "B" { print("fine") }
    else { print("try again") }
}
```

An arm is `expr (',' expr)* block`. The comma is unambiguous here — commas
appear only in call arguments and list literals, both of which are already
closed by the time the arm's expression ends.

`case` was considered and dropped for consistency: zl writes `if cond {`, not
`if (cond) then {`. Adding a noise word to every arm would be the only place in
the language where a construct is announced twice. The cost is a slightly worse
error message; the mitigation is that the parser knows it is inside a switch, so
it can still say *"expected a case value or `else`"*.

(The filename of this document says `switch_case`; the feature ships without the
word `case`. Renaming the file is not worth a broken link.)

### 2.3 The default arm is `else`

`else` is already a keyword (`src/frontend/lexer.c:49`) and already reads as "otherwise".
Reusing it costs nothing and — see §7.1 — maps onto the desugaring's `n->c`
slot without any translation at all.

Rules:

- **At most one `else` arm.** Two is a hard parse error.
- **`else` must be last.** An `else` followed by more arms is a hard parse
  error, because the desugaring would silently make those arms unreachable.
- **`else` is optional.** A switch with no matching arm and no `else` does
  nothing. It is not an error — `if` with no `else` does nothing either, and a
  dispatch that quietly ignores what it does not recognise is a legitimate
  program.

Rejected alternative: a bare `_` arm (Rust-ish). `_` is a legal identifier
character (`src/frontend/lexer.c:224`), so `_` is an ordinary variable name and a `_` arm
would be indistinguishable from `switch x { _ { } }` meaning "match against the
variable named `_`". `else` has no such problem.

---

## 3. What a case is: a value

Three candidates, in increasing order of ambition.

**Values (recommended).** An arm holds one or more expressions; the arm is taken
when `values_equal(subject, arm_value)` (`src/runtime/interp.c:1126-1141`). That function
already handles numbers, bools, strings, nil and — structurally — lists, so
`switch` inherits a definition of equality that is already tested and already
shared by `==`, `contains` and `find`. No new comparison semantics enter the
language.

**Patterns.** Destructuring is a separate W2 item and does not exist yet. A
pattern arm needs a binding mechanism (`switch pt { [x, y] { ... } }` must
introduce `x` and `y`), and binding needs a decision about scope that zl has not
made — blocks do not create scopes today (`src/runtime/interp.c:1505-1507` runs an `N_BLOCK`
in the *same* `Env`; a new `Env` appears only per function call,
`src/runtime/interp.c:1279`). Designing switch around a feature that does not exist would
mean designing scope by accident. **Out of scope.**

**Ranges.** `1 to 5` is tempting because `to` already exists positionally in
for-headers (`src/frontend/parser.c:731-735`). But: it only works when the subject is
numeric and ordered, it doubles the arm-matching machinery (each arm becomes
"equality *or* an interval test"), and — since `to` is deliberately recognised
only on the for-range path (`src/frontend/parser.c:623-626`) — it would be the first place
`to` means something in a second context. The common cases are covered by
multi-value arms and by an `else` arm with an ordinary `if` inside.
**Out of scope**, cleanly addable later since arms are already expressions.

### 3.1 Arms hold arbitrary expressions, not constants

C requires case labels to be integer constants. zl should not, for three
reasons:

1. **There is nothing to gain.** The point of constant labels is a jump table.
   zl numbers are C `double`s (`src/runtime/runtime.h`, `src/runtime/interp.c` `Value.num`) and zl case
   values may be strings, so no C `switch` can be emitted anyway except in the
   narrow "every arm is a whole-number literal" case. That is a peephole
   optimisation, not a language rule (§7.6).
2. **Dispatching on a computed value is a real use.** `switch c { LOW { } HIGH
   { } }` where `LOW`/`HIGH` are named globals is ordinary code, and zl has no
   `const` to promote them with.
3. **The desugaring is exact either way**, so allowing expressions costs no
   implementation.

**The contract this buys, and its price:** arms are tested **top to bottom, and
testing stops at the first match**. If two arms can match, the earlier one wins.
If an arm's value expression has a side effect, that side effect happens only if
control reaches that arm. Both follow for free from the `if`/`elif` desugaring
and are the behaviour a reader of an `if`-chain already expects. The price is
that duplicate detection becomes partial (§6).

---

## 4. No fallthrough

C falls through by default and needs `break` to stop. Go falls through only on
an explicit `fallthrough`. zl should have **no fallthrough and no way to ask for
it**, and the reason is specific to this codebase.

**`break` already means something, and the desugaring keeps it meaning that.**
`break` sets a flag (`src/runtime/interp.c:1468`) that the enclosing `while`/`for` consumes,
and `src/backends/c/compile.c:380` emits a literal C `break;`. Under the proposed desugaring a
switch is an `if`-chain — not a loop — so:

```
while has_next() {
    switch peek() {
        "}" { break }          # leaves the WHILE. Correct, and obvious.
        ...
    }
}
```

works, today's meaning, no special case. Adopt C semantics instead and that same
`break` would leave the *switch* and spin the `while` forever. There is no
spelling that makes both meanings available without a labelled-break feature zl
does not have and does not need.

The secondary reasons:

- **Fallthrough cannot be desugared.** An `if`-chain has no way to enter arm
  *k+1* after running arm *k* except by duplicating arm *k+1*'s body into arm
  *k* — which is O(N²) tree growth and puts the same `Node` under two parents.
  `src/frontend/parser.c:940-947` records exactly why that is unacceptable: the old
  compound-assignment desugaring did it, and the result was double evaluation
  plus a tree no pass could safely free. The only other route is `goto`, which
  zl has no notion of and which `src/backends/native/nativegen.c` and `src/backends/llvm/compilel.c` would each have
  to grow labels for.
- **Multi-value arms cover the honest use.** In practice C fallthrough is
  overwhelmingly used for *"these three labels, one body"* — written here as
  `"+", "-", "*" { ... }`, which is clearer and cannot be reached by accident.

`docs/archive/prompts/OVERNIGHT_CAMPAIGN.md:57` asks for *"fallthrough rules"*. This is the
rule: there is no fallthrough, because `break` is already spoken for.

---

## 5. Statement, not expression

An expression form is attractive:

```
name = switch code { 1 { "one" } 2 { "two" } else { "many" } }
```

Turn it down for v1, for a concrete reason rather than caution.

**Arms are blocks, and a zl block has no value.** There is no
implicit-last-expression rule; `return` is the only way a value leaves a body
(`src/runtime/interp.c:1180`). An expression switch therefore needs either a new
block-value concept (a language change far larger than switch) or a second,
expression-only arm syntax (`1 -> "one"`), which means two switch grammars.

**And the subject could not be hoisted.** The whole reason to prefer switch over
an `if`-chain is "evaluate the subject once" (§1). In the statement form that is
done with a hidden local, exactly as `for i = a to b` hoists its limit and step
into `__zl_lim0`/`__zl_st0` (`src/frontend/parser.c:772-776`). An expression appears in the
middle of another expression — there is no statement position to hoist into, so
an expression switch would have to inline the subject into every comparison and
re-evaluate it, losing the one semantic advantage the feature has.

**The future door is left open and is cheap.** If an expression form is ever
wanted, an arms-are-expressions variant desugars to a right-nested ternary —
`(s == 1) ? "one" : ((s == 2) ? "two" : "many")` — which every engine that
supports `?:` already handles (`src/backends/c/compile.c:259-266`). It would be additive. It is
just not v1, and it is not what W2 asked for.

---

## 6. Duplicate-case detection

**Where: in the parser, before the rewrite.** This is not a preference — after
desugaring there are no "arms", only nested `if`s, and the set of case values no
longer exists as a set anywhere in the tree. The parser is the only place in the
entire toolchain that ever sees them together. Anything not checked there is not
checkable later without reconstructing the switch.

**What is decidable: literal arms.** For every arm value that is `N_NUMBER`,
`N_STRING` or `N_BOOL`, record it and compare against the ones already seen. A
second occurrence is a **hard parse error** naming the value — it is always a
bug, because the second arm is unreachable.

Two details that are easy to get wrong, both coming from the boxed-double value
model:

- **Compare numbers by value, not by text.** `1`, `1.0`, `01` and `1e0` are four
  different `n->text` strings and one value. `values_equal` compares
  `l.num == r.num` (`src/runtime/interp.c:1130-1131`), so the duplicate check must
  `strtod` both sides and compare doubles, or it will pass a switch whose second
  arm is dead. `0` and `-0` are likewise the same case, because `0.0 == -0.0`.
- **Compare strings after escape processing.** The lexer resolves `\n`, `\t`,
  `\xNN` while producing the token (`src/frontend/lexer.c:180-215`), so `n->text` is already
  the real bytes and a plain `strcmp` is correct — but only because the lexer,
  not the parser, owns escapes. Worth stating so nobody "fixes" it later.

**What is not decidable: computed arms.** `switch x { LOW { } HIGH { } }` where
both globals hold `3` is a duplicate that no parse-time check can see. The
runtime rule is simply *first match wins*, silently. That is the documented
consequence of choosing expressions over constants (§3.1), and it is the same
deal `if`/`elif` already offers.

**Also enforced at parse time** (all hard errors, all for the same reason — the
alternative is silently dead code):

- more than one `else` arm;
- an `else` arm that is not last;
- an arm with zero values (a stray `{` where a case value was expected).

---

## 7. Implementation strategy

This is the section that decides whether the feature is a day or a week.

### 7.1 The desugaring

`src/frontend/parser.c:614-627` states the technique and why it is used:

> *the three loop forms below are REWRITTEN, here in the parser, into the
> while/if/assign tree the language already had. That is the whole trick: no new
> NodeType, so the interpreter, the C emitter and every other backend understand
> them without learning a single thing.*

`switch` fits it exactly. Source:

```
switch SUBJ {
    A, B { body1 }
    C     { body2 }
    else  { body3 }
}
```

becomes, as a tree:

```
__zl_sub<id> = SUBJ
if (__zl_sub<id> == A) ? true : (__zl_sub<id> == B) { body1 }
elif __zl_sub<id> == C                              { body2 }
else                                                { body3 }
```

Every piece already exists and is already built by helpers sitting in
`src/frontend/parser.c`:

| Piece | Helper | Existing user |
|---|---|---|
| the hidden name `__zl_sub0` | `hidden_ident()` (`src/frontend/parser.c:636-641`) | `__zl_lim0`, `__zl_st0`, `__zl_first0`, `__zl_again0` |
| `__zl_sub0 = SUBJ` | `assign_to()` (`src/frontend/parser.c:673-679`) | for-range, do-while |
| `__zl_sub0 == A` | `binary()` (`src/frontend/parser.c:664-671`) | for-range's bounds test |
| the arm as an `N_IF` | plain `new_node(N_IF)` | `parse_if` |
| chaining arms | `n->c` = an `N_BLOCK` holding the next `N_IF` | **`parse_if`'s own `elif` handling, `src/frontend/parser.c:581-589`** |
| wrapping assign + chain into one statement | `N_BLOCK` | for-range (`src/frontend/parser.c:772-778`), do-while (`src/frontend/parser.c:843-846`) |

That last row is worth dwelling on: `elif` is *already* implemented as "a nested
`N_IF` inside a one-statement `N_BLOCK` in the else slot", and the comment there
says the tree it builds is *"indistinguishable from the one `else if`
produces"*. A desugared switch produces literally the same tree an `if`/`elif`
chain produces. There is no new shape for any consumer to learn — not even a
shape they have never seen before.

`parse_switch` is one function of roughly 70 lines, sitting beside
`parse_for_range` and `parse_do_while`, plus one line in `parse_statement`
(`src/frontend/parser.c:912-925`) next to the existing `starts_block_word` checks.

**Everything downstream is untouched.** `src/runtime/interp.c` walks `N_ASSIGN`/`N_IF`
(`src/runtime/interp.c:1461-1467`); `src/backends/c/compile.c` emits them (`src/backends/c/compile.c:343-349`);
`src/backends/c/compile.c`'s `collect_vars` picks up `__zl_sub0` automatically because it is a
plain `N_ASSIGN` to an `N_IDENT`, which is exactly how `__zl_lim0` already gets
declared. Zero lines change in either.

### 7.2 Multi-value arms: why `or` is wrong and the ternary is right

The obvious condition for `A, B` is `sub == A or sub == B`. It is wrong, and
`src/frontend/parser.c:815-817` already says why, in the do-while comment:

> *(A plain `or` would not do - the C backend evaluates both of its operands.)*

`src/runtime/interp.c:1337-1339` short-circuits `and`/`or` in `eval`; `src/backends/c/compile.c` routes
them through `zl_binop`, which by construction receives both operands already
evaluated. So `switch f() { g(), h() { } }` would call `h()` under the C backend
and not under the interpreter — a three-engine divergence, which is the one
failure mode `verify.ps1` exists to catch.

The codebase's own fix for this exact problem is the ternary, and
`src/backends/c/compile.c:259-266` states the guarantee:

> *C's own `?:` is the short-circuit. The untaken branch is never evaluated,
> exactly as in the interpreter.*

So `A, B, C` becomes a right-nested ternary chain:

```
(sub == A) ? true : ((sub == B) ? true : (sub == C))
```

A single-value arm is just `sub == A` — no ternary at all, which matters in
§7.7. Both `parse_for_range` (`src/frontend/parser.c:758-761`) and `parse_do_while`
(`src/frontend/parser.c:834-837`) already use a ternary for precisely this reason, so this is
following an established rule, not inventing one.

**Rejected: `sub in [A, B, C]`.** It is the prettiest desugaring and the brief
raises it, but `in` on a list becomes `contains(list, sub)`
(`src/runtime/interp.c:1211-1218`), which means (a) all the element expressions are
evaluated regardless — same divergence as `or`, and (b) an `N_LIST` literal
allocates a `Value` per element on every evaluation (`src/runtime/interp.c:1327-1334`), so a
switch in a hot loop mallocs per iteration. Prettier tree, worse semantics,
worse performance.

### 7.3 Why the arm bodies must not be duplicated

The alternative desugaring for `A, B { body }` is two arms sharing one body.
Don't: `src/frontend/parser.c:940-947` documents the bug that came from putting one `Node` in
two places in the tree — a subscript evaluated twice, and *"one Node with two
parents, which no tree pass could safely free"*. Deep-copying the body instead
turns an N-value arm into N copies of its statements, which is unbounded growth
in the emitted C. The ternary condition keeps exactly one body per arm.

### 7.4 What it costs when there are many arms

| | Desugared `if`-chain | A real `N_SWITCH` |
|---|---|---|
| Tree nodes | O(total case values) — ~4 per arm + 3 per extra value, plus the bodies | O(total case values) |
| Comparisons at runtime | up to N, average N/2 | up to N (linear) or O(1) with a table |
| Generated-C nesting | **one brace level per arm** | flat |
| Backends to change | 0 (for the two full engines) | 5 (four backends + interp) |

The runtime cost is a linear scan either way, because a table is not available
(§3.1). N/2 string compares for a fifteen-way keyword dispatch is what
`src/selfhost/compiler.zl:36-37` costs *today* as a hand-written `or` chain — so switch is
not slower than the code it replaces; it is the same cost, spelled better. If
that ever shows up in `bench/`, the fix is §7.6, not a different surface syntax.

The one real scaling cost is **nesting in the emitted C**. `src/backends/c/compile.c:347` emits
the else-branch as `else { ... }` with `indent + 1`, so an N-arm switch produces
N levels of nested braces and N levels of indentation in `out.c`. At 200 arms
that is 800 columns of leading whitespace and 200 nested compound statements —
past what C89 guarantees (15) and heading toward MSVC's real limit. Note that
this is **not new**: a 200-arm `if`/`elif` chain does it today. Switch just
makes writing one comfortable. If it bites, the fix is a peephole in
`src/backends/c/compile.c` — when an `N_IF`'s else-block is a single `N_IF`, emit `else if (`
instead of `else {\n if (` — which is ~4 lines, benefits `elif` too, and needs
no AST change.

### 7.5 What desugaring costs, honestly

1. **The tree dump shows the rewrite, not the source.** `print_node`
   (`src/frontend/parser.c:1061` region, used by `parser.exe`) will print nested `N_IF`s and a
   `__zl_sub0` variable. This is already true of `for i = 1 to 10`, `do/while`
   and `loop`; switch joins them. Accepted.
2. **Parse errors inside a switch report positions, not context.** No worse than
   any other construct today.
3. **The hidden variable is a real variable.** Blocks do not create scopes
   (`src/runtime/interp.c:1505-1507`), so `__zl_sub0` becomes an ordinary function-local or
   global. This is load-bearing rather than a leak — it is exactly what lets
   `collect_vars` declare it in the emitted C — but it means a program can
   observe `__zl_sub0` if it goes looking. The `__zl_` prefix plus the per-
   construct counter (`src/frontend/parser.c:632-641`) is the whole defence, and it is the
   same defence the loop forms already rely on.
4. **`g_hidden` must stay deterministic.** It is reset per `parse()` so the same
   source yields the same tree (`src/frontend/parser.c:630-632`) — which is what keeps the
   self-hosting fixpoint byte-stable. `parse_switch` must take its id the same
   way, from the same counter, and must not renumber on re-parse.

### 7.6 When a real `N_SWITCH` node would be justified

Not now. The triggers, stated in advance so the decision is not re-litigated on
taste:

- **A measured dispatch bottleneck** where every arm is a whole-number literal,
  so a genuine C `switch` / jump table is emittable. `bench/` has to show it
  first; `MASTER_PLAN.md` §3.2 puts runtime optimisation in W4 and is explicit
  that the numbers steer the work.
- **Fallthrough is ever wanted.** It cannot be desugared (§4), so it would force
  a real node the day it is accepted. It is not accepted.
- **A source-fidelity tool** — a formatter, or a linter that wants to say "this
  switch is missing a case" — needs the original shape. A formatter is not on
  any roadmap.

Note the asymmetry that makes this an easy call: a desugaring is **one new
function in each parser and nothing else**. A real `N_SWITCH` is a new
`NodeType` in `src/frontend/parser.h`, an arm in `src/runtime/interp.c`, an arm in `src/backends/c/compile.c`, an arm in
`src/backends/llvm/compilel.c`, an arm in `src/backends/native/nativegen.c`, an arm in `src/backends/c/compilef.c`, a new tree tag in
`src/selfhost/compiler.zl`'s parser **and** a new arm in `src/selfhost/compiler.zl`'s codegen — eight
places that must agree, against a three-engine test gate. That is the cost of
switching later, and it is worth paying only against a measurement.

### 7.7 The narrow backends — the one place code must change

"Zero backend changes" is true of the two engines that implement the whole
language (`src/runtime/interp.c`, `src/backends/c/compile.c`). It is not true of the other three, and this
document should not pretend otherwise.

| Backend | Handles `N_BLOCK` as a statement? | Handles `N_TERNARY`? |
|---|---|---|
| `src/runtime/interp.c` | yes (`:1506`) | yes (`:1372`) |
| `src/backends/c/compile.c` | yes (`:376`) | yes (`:259`) |
| `src/backends/llvm/compilel.c` (LLVM IR) | yes (`:244`) | **no** — falls to the error exit at `:176` |
| `src/backends/c/compilef.c` (unboxed C) | **no** | **no** — error exits at `:79` and `:120` |
| `src/backends/native/nativegen.c` (x86-64 PE) | **no** | **no** — error exit at `:277` |

Two consequences:

1. **This is a pre-existing hole, not one switch digs.** `for i = a to b` and
   `do/while` already desugar into an `N_BLOCK` wrapper containing an
   `N_TERNARY`, so they already fail on those three backends. `run_tests.ps1`
   (three engines, `interp` + `compile` + `nativegen`) passes today only because
   its integer-subset programs do not use those forms.
2. **Single-value-arm switches need only `N_BLOCK`.** The ternary appears only
   for multi-value arms (§7.2). So adding `case N_BLOCK: emit_block(n); break;`
   to `src/backends/c/compilef.c`'s `emit_stmt` and the equivalent to `src/backends/native/nativegen.c`'s
   `gen_stmt` — three lines each, and `src/backends/native/nativegen.c:282-289` already has the
   `gen_block` helper to call — makes plain switches work on all five engines
   *and* retroactively unblocks `for`-range, `do/while` and `loop` there.

**Recommendation: make those two three-line additions part of this change.** It
is the smallest possible edit, it is the difference between switch being
testable by the actual gate (`verify.ps1` → `run_tests.ps1`, three engines) and
being testable by two engines, and it repays a debt the previous W2 features
left behind. `N_TERNARY` in the narrow backends is a larger job (a `select` /
branch-and-phi in `src/backends/llvm/compilel.c`, a conditional jump in `src/backends/native/nativegen.c`) and should
stay out of scope; multi-value arms simply remain outside the integer subset,
which is documented behaviour for those backends anyway.

---

## 8. What happens to `src/selfhost/compiler.zl`

`src/selfhost/compiler.zl` is zl's compiler written in zl, and it implements a **subset** of
the surface syntax. Concretely, today:

- its keyword list (`src/selfhost/compiler.zl:36-37`) has **twelve** words against the C
  lexer's fifteen — no `elif`, no `break`, no `continue`;
- `parse_if` (`src/selfhost/compiler.zl:309-319`) handles `else` but **not `elif`**;
- `parse_statement` (`src/selfhost/compiler.zl:366-389`) handles only `if`, `while`, `for`,
  `fn`, `return`, assignment and expression statements — no `do`, no `loop`, no
  for-range, no ternary.

So there is already a queue of W2 features the self-hosted compiler does not
understand. Switch joins the back of it. Three things follow, and all three
should be stated in the commit rather than discovered later.

**1. The self-hosting fixpoint is unaffected.** `verify.ps1` check 1 is
`f(f(x)) == f(x)`: the interpreter runs `src/selfhost/compiler.zl` to produce `gen1.c`, that
builds a compiler, and that compiler recompiles `src/selfhost/compiler.zl` to `gen2.c`, which
must be byte-identical. A `src/frontend/parser.c` change cannot move that, because
`src/selfhost/compiler.zl`'s *source* is unchanged and `src/selfhost/compiler.zl`'s own parser is what
produces `gen2.c`. The fixpoint holds as long as **`src/selfhost/compiler.zl` itself does not
use `switch`** — which is the same standing rule that already applies to `elif`,
`break`, `continue`, ternary and the three loop forms.

**2. Programs that use `switch` cannot be compiled by the self-hosted
compiler.** They work under `interp.exe` and `compile.exe` and fail under a
`zlc.exe` built from `src/selfhost/compiler.zl`. That is the existing two-tier situation, and
the honest way to record it is a line in `docs/REFERENCE.md` marking which
surface features are C-toolchain-only.

**3. Positional recognition is what keeps the two lexers from drifting.** This
is the sharpest argument in §2.1 and it belongs here. `src/frontend/lexer.c:46-62` and
`src/selfhost/compiler.zl:36-37` are two independent copies of the keyword list. Make
`switch` a keyword in one and not the other, and a program containing `switch`
as a *variable* lexes as `T_KEYWORD` under one toolchain and `T_IDENT` under the
other — the same text, two parses, which is precisely the class of divergence
`verify.ps1` cannot catch (the fixpoint only checks `src/selfhost/compiler.zl` against
itself). Recognising `switch` positionally means the keyword lists stay
identical and the question never arises.

**Teaching `src/selfhost/compiler.zl` switch, when the time comes, is small** — and this is
the strongest single argument for desugaring over an `N_SWITCH` node. Because
the rewrite emits only `["assign", ...]`, `["if", cond, then, else]` and
`["block", ...]`, all of which `src/selfhost/compiler.zl`'s codegen already handles
(`src/selfhost/compiler.zl:550-565`), the work is **one `parse_switch` function of ~40 lines
in `src/selfhost/compiler.zl`'s parser and zero lines in its codegen.** A real `N_SWITCH`
node would need a new tree tag *and* a new codegen arm there, plus matching arms
in four C backends.

---

## 9. What lands where — checklist

| File | Change |
|---|---|
| `src/frontend/lexer.c` | **none** — no new keyword, no new symbol |
| `src/frontend/parser.h` | **none** — no new `NodeType` |
| `src/frontend/parser.c` | `leads_switch()` forward-scan predicate (~12 lines) next to `starts_block_word` (`:905`); `parse_switch()` (~70 lines) next to `parse_do_while` (`:818`); one dispatch line in `parse_statement` (`:924`) |
| `src/runtime/interp.c` | **none** |
| `src/backends/c/compile.c` | **none** (optional 4-line `else if` peephole, §7.4) |
| `src/backends/c/compilef.c` | `case N_BLOCK:` in `emit_stmt` (3 lines) — also fixes for-range/do-while/loop there |
| `src/backends/native/nativegen.c` | `case N_BLOCK: gen_block(n); break;` in `gen_stmt` (3 lines) — same |
| `src/backends/llvm/compilel.c` | **none** (already has `N_BLOCK` at `:244`) |
| `src/selfhost/compiler.zl` | **none now.** Must not use `switch` until it learns it (§8) |
| `docs/REFERENCE.md` | document the form; mark it C-toolchain-only |
| `tests/test_syntax.zl` | the programs in §10 |

Roughly 90 new lines of C, all in `src/frontend/parser.c`, plus six lines across two
backends.

---

## 10. Test plan — the pass/fail gate

The bar is the project's standing one: **`verify.ps1` green** — self-hosting
fixpoint holds, and `run_tests.ps1` is 6/6 with the interpreter, the C backend
and the x86-64 backend printing identical output.

Programs, each asserted byte-identical across the engines that support it:

1. **Basic dispatch** — three string arms plus `else`; hit each in turn. All
   five engines once §7.7 lands (single-value arms, no ternary).
2. **Subject evaluated exactly once** — `switch bump()` where `bump` increments
   and prints a global. Print the counter after. This is the semantic reason the
   feature exists; if it prints `1` the hoist works, if it prints N the
   desugaring inlined the subject.
3. **Multi-value arm** — `"+", "-" { ... }`; check both values reach the body,
   and check the arm's *second* value expression is not evaluated when the first
   matches (the `or`-vs-ternary bug, §7.2). Interpreter + C backend only.
4. **No fallthrough** — an arm whose body is followed by another matching arm;
   assert only the first body runs.
5. **`break` inside a switch inside a `while`** — asserts §4: it must leave the
   loop, not the switch. This is the regression test that a future "add
   fallthrough" patch would trip.
6. **`continue` inside a switch inside a `for`** — same reasoning, and it also
   exercises the `N_BLOCK` wrapper on the `continue` path.
7. **No match, no `else`** — falls through to the next statement, prints
   nothing, does not error.
8. **Nested switches** — asserts `g_hidden` numbering keeps `__zl_sub0` and
   `__zl_sub1` apart, the same property `src/frontend/parser.c:629-632` guarantees for nested
   for-ranges.
9. **Numeric duplicate detection** — `switch n { 1 { } 1.0 { } }` must be a
   parse error, not a program with a dead arm (§6, the `strtod` point).
10. **`switch` is still an identifier** — a program with `switch = 5`,
    `print(switch)` and a function `fn switch_on(x)` must still run unchanged.
    This is the §2.1 invariant, and it is the test that fails if someone
    "simplifies" the forward scan into a keyword.
11. **Structural errors** — two `else` arms; an `else` that is not last; an arm
    with no case value. Each a clean parse error.

Test 2 and test 10 are the two that must never be deleted: the first is the
feature's only semantic justification, the second is its only compatibility
claim.

---

## 11. Open questions (decide at implement time)

- **Forward scan or one-token guard (§2.1)?** Recommended: forward scan, because
  it preserves the `src/frontend/parser.c:623-626` invariant exactly. The guard is the
  fallback if the scan proves awkward to mirror in `src/selfhost/compiler.zl` later.
- **Ship the two three-line backend additions with this change (§7.7)?**
  Recommended yes — it is what makes switch testable by the real gate.
- **The `else if` peephole in `src/backends/c/compile.c` (§7.4)?** Not now. Do it when a
  generated file actually nests deep enough to bother MSVC, and do it as its own
  commit since it changes output for existing `elif` chains and will move the
  fixpoint hash.
- **Range arms (`1 to 5`)?** Deliberately out (§3). Arms are already
  expressions, so this stays addable without redesign.
