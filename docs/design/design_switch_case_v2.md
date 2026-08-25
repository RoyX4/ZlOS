# Design: `switch` for zl — revision 2

**Status:** proposal, revises `docs/design/design_switch_case.md` (2026-08-01) ·
**Wave:** W2 (syntax and surface) · **Date:** 2026-08-02
**Scope:** one new statement form. No code is changed by this document.
Line citations were re-checked against the tree on 2026-08-02, but `src/frontend/parser.c`,
`src/frontend/lexer.c` and the numeric backends were being edited concurrently (floats), so
treat line numbers as pointers, not anchors. Where v1's citations had moved I
say so.

Revision 1 was written before four things happened: the loop forms **shipped**,
so we now have a measurement rather than a prediction about what a desugared
construct costs; and records, maps and nullables acquired designs
(`design_records.md`, `design_maps_v2.md`, `design_nullable.md`,
`design_scoping_decision.md`), each of which touches `switch` somewhere.

This is a revision, not a restart. **§1 lists exactly what changes and why.**
Everything not listed there — §§2-6 of v1 (surface syntax, cases-are-values, no
fallthrough, statement-not-expression, duplicate detection) — still stands, and
I do not restate it.

---

## 0. TL;DR of the deltas

| # | v1 said | v2 says | Why it changed |
|---|---|---|---|
| 1 | Desugar into `if`/`elif`; predicts zero backend work | **Unchanged, and now evidenced** rather than predicted | The three loop forms shipped exactly this way |
| 2 | Always hoist the subject into `__zl_sub<id>` | **Do not hoist when the subject is a bare identifier** | Hoisting defeats `design_nullable.md` §5.2 narrowing; a name test is pure, so the hoist buys nothing |
| 3 | Patterns are out because destructuring and scope don't exist | **Still out — but tag dispatch and shape equality now come free** | Records are lists (`design_records.md` §0), so `values_equal` already does shape comparison |
| 4 | Cases are values, compared with `==` | **Add a hazard: a map-valued arm will silently never match** | `values_equal_depth`'s `default: return 0` (`src/runtime/interp.c:1236-1256`) will swallow a new `V_MAP` |
| 5 | Teaching `src/selfhost/compiler.zl` is "~40 lines in the parser and **zero** in codegen" | **Wrong. Its `emit_stmt` has no `"block"` tag** | Verified: `src/selfhost/compiler.zl:536-574` |
| 6 | Ship two three-line backend additions with this change | **Still right, and the debt is now one release older** | The loops shipped without them; `src/backends/c/compilef.c` and `src/backends/native/nativegen.c` still reject `N_BLOCK` |
| 7 | A real `N_SWITCH` "when a measured dispatch bottleneck appears" | **Never, for jump-table reasons. LLVM is the optimiser** | `MASTER_PLAN.md` §10: LLVM forms the switch itself, at 4 arms |

---

## 1. What shipping the loops taught us

v1 §7.1 was an argument that desugaring would be cheap. It is now an
observation. `src/frontend/parser.c:614-627` is the standing statement of the technique:

```c
/* =============================================================
 * DESUGARING - the three loop forms below are REWRITTEN, here in the
 * parser, into the while/if/assign tree the language already had.
 *
 * That is the whole trick: no new NodeType, so the interpreter, the C
 * emitter and every other backend understand them without learning a
 * single thing. It also means break and continue keep working, because
 * what they end up inside really is an ordinary while.
 */
```

Three forms — `for i = a to b`, `do {} while`, `loop {}` — landed with a
`parse_*` function each and **no `NodeType`, no `src/runtime/interp.c` arm, no `src/backends/c/compile.c`
arm**. That is the strongest evidence available about what a new construct costs
in this codebase, and it points one way. v2 keeps the desugaring recommendation
and, unlike v1, does not need to argue for it.

**But the loops also shipped the hole v1 predicted and nobody closed it.** v1
§7.7 recommended six lines across two backends. Re-checked today:

| Backend | `N_BLOCK` as a statement | `N_TERNARY` |
|---|---|---|
| `src/runtime/interp.c` | yes | yes |
| `src/backends/c/compile.c` | yes | yes |
| `src/backends/llvm/compilel.c` | yes (`:282`) | **no** — `default:` error at `:214` |
| `src/backends/c/compilef.c` | **no** — `emit_stmt` (`:86`) `default:` error at `:120` | **no** — expr `default:` at `:79` |
| `src/backends/native/nativegen.c` | **no** — `gen_stmt` (`:203`) `default:` error at `:276-278` | **no** — same `default:` |

`src/backends/native/nativegen.c:282-289` already has the helper the fix would call:

```c
static void gen_block(Node *block)
{
    if (!block) return;
    if (block->type == N_BLOCK || block->type == N_PROGRAM)
        for (int i=0;i<block->nkids;i++) gen_stmt(block->kids[i]);
    else
        gen_stmt(block);
}
```

So `for i = 1 to 10` still dies on `compilef.exe` and `nativegen.exe` with
*"statement not in the numeric subset"*, four days after it shipped. The reason
nobody noticed is `run_tests.ps1`'s programs stay inside the integer subset by
hand. **v2 upgrades v1's recommendation from "ship it with switch" to "ship it
first, as its own commit."** It is six lines, it repays a debt that is not
switch's, and it is the difference between switch being covered by three engines
and by two. Doing it separately also means that if it breaks something, the
blame is unambiguous.

---

## 2. The correction that matters: do not hoist a bare identifier

This is the one place v1's design is actively wrong now, and it is wrong because
of a document that did not exist when it was written.

### 2.1 What v1 proposed

```
switch SUBJ { A { body } else { other } }

  =>   __zl_sub0 = SUBJ
       if __zl_sub0 == A { body } else { other }
```

The hoist exists to make "the subject is evaluated exactly once" true (v1 §1,
and its test 2 is the regression test for it). For `switch next_token() { ... }`
that is the whole point of the feature and it must stay.

### 2.2 Why it breaks match-on-optional

`design_nullable.md` §5.2 defines the narrowing forms the checker will
recognise, and the first one is:

> | N1 | `x == nil()` / `x != nil()` | `dijkstra.zl:25`, `combinat.zl:168` |

Narrowing refines **a name**. The natural spelling of an optional unwrap under
this design is

```
switch maybe_path {
    nil()  { return -1 }
    else   { return len(maybe_path) }
}
```

and under v1's rewrite the checker sees `__zl_sub0 == nil()`. It refines
`__zl_sub0`, which nothing in the body mentions, and `maybe_path` stays `T?` in
the `else` arm. The unwrap silently does not happen, and the user gets a type
error on a program that looks exactly like the `if` they would have written by
hand. Worse, it is *invisible*: the source contains no `__zl_sub0`.

### 2.3 The fix, and why it is sound

**When the subject is a single `N_IDENT`, emit the comparisons against that
identifier directly and generate no hidden variable at all.**

```
switch x { A { b1 } B { b2 } else { b3 } }

  =>   if x == A { b1 } elif x == B { b2 } else { b3 }
```

Soundness argument, in the order the objections come:

1. **Re-reading a name has no side effects.** zl has no properties, no getters,
   no operator overloading on identifier reads; `N_IDENT` evaluation is an
   `env_find` and a copy. The "evaluate once" guarantee exists to stop `f()`
   running N times, and a name is not a call.
2. **No arm test runs after any arm body.** The desugaring is an `if`/`elif`
   chain: reaching arm *k+1*'s test means arm *k*'s test failed, so arm *k*'s
   body did not run. A body that assigns to `x` therefore cannot be observed by
   a later test. This holds under the global-assignment rule as it stands
   *and* under `design_scoping_decision.md`'s Option 2, because it is a
   property of the control flow, not of where the slot lives.
3. **The one real divergence is a case-value side effect.** `switch x { f() { }
   g() { } }` where `f()` assigns to a global `x`: hoisted, the second test
   compares the old `x`; unhoisted, the new one. This is genuinely different.
   It is also *exactly* what the `if`-chain being replaced does today —
   `if x == f() { } elif x == g() { }` re-reads `x` — so the unhoisted form is
   the one that preserves the meaning of the code people are migrating. Document
   it; do not engineer around it.

This costs one `if` in `parse_switch` and it deletes a variable from the emitted
C in what will be the common case. It also makes the tree dump readable for the
common case, which partly answers v1 §7.5's first cost.

### 2.4 What it does *not* fix

It does not make `switch f() { nil() {...} }` narrow anything, because there is
no name to narrow — but neither does `if f() == nil() { }`, so nothing is lost.
And it does not make `switch` a *better* unwrap than `if`; it makes it an
equally good one, which is the bar. `design_nullable.md` §5 is explicit that
narrowing is primary and syntax is secondary; a switch that quietly opted out of
narrowing would have been a trap.

**Recommendation: no `match`-specific unwrapping sugar in v1 of this feature.**
Once the subject is not hoisted, `switch` inherits narrowing for free and needs
no entry in the nullable design's table of recognised forms.

---

## 3. Records: what became possible, and what still is not

`design_records.md` §0 settles the representation:

> A record is **a set of names for the positions of a list**. `rec Image { w, h, px }`
> says that in a value of type `Image`, position 0 is called `w`. `img.w` lowers to
> `img[0]` — literally, in the parser, before any engine sees it.

and §7 hands this question over explicitly:

> `switch`/pattern matching over record *shape* is a different feature entirely and
> `design_switch_case.md` owns that question.

Taking ownership. Three things a "match over records" could mean, and they have
three different answers.

### 3.1 Tag dispatch — already free, and it is the real use

```
switch shape.kind {
    "circle" { area = 3.14159 * shape.r * shape.r }
    "rect"   { area = shape.w * shape.h }
    else     { area = 0 }
}
```

`shape.kind` lowers to `shape[0]` in the parser before `parse_switch` ever sees
it, so this is an ordinary value switch on an ordinary expression. **Zero design
work, zero implementation work.** This is what discriminated-union dispatch
looks like in zl and it is available the day both features land.

Note the interaction with §2: the subject here is an `N_FIELD`/`N_INDEX`, not an
`N_IDENT`, so it **is** hoisted — correctly, because `shape[0]` is a bounds-
checked index and evaluating it N times is real work.

### 3.2 Shape equality — free, and slightly dangerous

`values_equal_depth` compares lists structurally (`src/runtime/interp.c:1236-1256`):

```c
case V_LIST:
    if (l.nitems != r.nitems) return 0;
    if (l.items == r.items) return 1;   /* the same list */
    ...
    for (int i = 0; i < l.nitems; i++)
        if (!values_equal_depth(*l.items[i], *r.items[i], depth + 1)) return 0;
    return 1;
```

A record is a `V_LIST`, so `switch cell { [0, 0] { origin() } }` compares shape
and contents and works today with no changes. That is a genuine capability v1
did not notice it was already shipping.

The danger is cost, not correctness: an `N_LIST` arm value allocates its
elements on **every evaluation**, so a switch with list-literal arms inside a
hot loop allocates per iteration per arm tested. v1 §7.2 rejected `sub in [A, B,
C]` for exactly this reason and the same reasoning applies to the arms
themselves. **Recommendation: allow it, document the cost in `REFERENCE.md`, do
not optimise it.** If it shows up in `bench/` the fix is constant-hoisting in
the parser, which is a general optimisation and not switch's business.

### 3.3 Destructuring patterns — still out, and now for a *better* reason

v1 §3 rejected patterns because binding needs a scope decision zl had not made.
That argument has weakened: `design_scoping_decision.md` §6 recommends Option 2,
which *is* the scope decision. So the honest reason has to be restated.

It is `design_records.md` §7, which turns down multi-assignment (`w, h, px =
img`) on the ground that comma-separated targets are a real grammar change and
`MASTER_PLAN.md` §4.2's whole point is not accumulating forms. **A binding
pattern arm is multi-assignment with extra steps.** Shipping
`switch p { [x, y] { ... } }` would introduce zl's first binding construct
sideways, inside a statement whose stated design is "compare with `==`", and it
would arrive before the general form it is a special case of. That is the wrong
order to build two features in.

It is also not free the way §3.1 and §3.2 are: a binding arm cannot desugar to
`==`. It desugars to a length test plus N assignments plus a nested chain, which
is a different rewrite, needs its own arity checking, and needs an answer for
what happens when a pattern partially matches. That is a second design document,
not a paragraph in this one.

**Out of v1, revisit after multi-assignment exists — not before.**

---

## 4. Maps: one concrete hazard

`design_maps_v2.md` adds a value type. `values_equal_depth` ends with:

```c
    default:     return 0;                  /* V_FN: identity-free */
```

`src/runtime/interp.c:1256`, mirrored at `src/runtime/runtime.c:213-233`. A new `V_MAP` that does not
get its own `case` lands in `default:` and compares **unequal to everything,
including itself**. In an `if` that is a visible bug. In a `switch` it is worse:
the arm is silently never taken and the `else` runs, which looks like a
data problem rather than a language problem.

This is not switch's bug and switch must not try to fix it. What this document
owes is the flag:

- **`design_maps_v2.md` must state whether `==` on maps is defined**, and if it
  is, both copies of `values_equal_depth` must grow a `V_MAP` case — parity
  between `src/runtime/interp.c` and `src/runtime/runtime.c` is load-bearing (both were verified
  identical in shape today).
- Until then, `REFERENCE.md` should say map-valued case arms are undefined.
- Map arms are order-sensitive if defined structurally, because
  `design_maps_v2.md` §8 fixes iteration order as insertion order. `["a": 1,
  "b": 2]` and `["b": 2, "a": 1]` would be two different cases. That is a good
  reason for maps_v2 to define `==` over *contents* or not at all.

The useful thing maps actually give `switch` is in §5.

---

## 5. The central question, re-answered: chain or jump table

v1 §7.6 said a real `N_SWITCH` becomes justified on "a measured dispatch
bottleneck where every arm is a whole-number literal". That trigger should be
**deleted**, and the reason is `MASTER_PLAN.md` §10 (2026-08-02): **LLVM is the
optimiser.** We do not write optimisation passes. The question is therefore not
"at how many arms do we build a jump table" but "at how many arms does someone
else build one for us, and are we emitting something they can recognise".

### 5.1 The crossover, per engine

**`src/runtime/interp.c` — never.** Each arm test is an `eval` of an `N_BINARY` reaching
`values_equal` (`src/runtime/interp.c:1330`). A table would need the subject hashed and an
arm-index dispatch; the hash of a boxed `Value` costs more than several
`values_equal` calls, and the tree-walk overhead around each test dwarfs both.
There is no arm count at which a table wins here. This is the engine `verify.ps1`
runs, and it is not a performance engine.

**`src/backends/c/compile.c` / `src/runtime/runtime.c` — never, and the chain is not the cost.** Every `==`
becomes a `zl_binop` call, and `zl_binop` dispatches on the operator **by
string** (`src/runtime/runtime.c:314-329`):

```c
if (strcmp(op, "+") == 0)  return binop_plus(l, r);
if (strcmp(op, "==") == 0) return zl_bool(values_equal(l, r));
if (strcmp(op, "!=") == 0) return zl_bool(!values_equal(l, r));
```

`==` is the second `strcmp`, so it is cheap by the standards of that function —
but the call still passes two 48-byte `Value`s and returns a third. A 12-arm
switch is 12 such calls. Shortening the chain to a table would remove maybe a
third of the cost of a dispatch whose real cost is boxing, which
`PLAN_unboxing.md` is already about. Wrong lever.

**`src/backends/llvm/compilel.c` (LLVM) — the crossover is 4, and it is not ours.** LLVM's
SimplifyCFG folds a chain of comparisons of one SSA value against constants into
a `switch` instruction, and the target lowering then chooses among a jump table,
a bit test, and a balanced binary tree. The jump-table threshold is
`MinJumpTableEntries`, whose default is **4**. So a 4-arm integer switch already
gets a table without a line of our code, provided we hand LLVM a chain it
recognises.

*I have not verified this on our output and could not — the build is being
rebuilt by another workflow.* The check is cheap and should be done before this
is treated as fact: compile a 12-arm integer switch through `src/backends/llvm/compilel.c`, dump
the `.ll`, and confirm a `switch` instruction appears rather than twelve
`icmp`/`br` pairs.

Two things could stop the fold, and both are actionable:

1. **The hidden variable.** `__zl_sub0` is emitted as an `alloca`+`store`+`load`
   unless `mem2reg` promotes it. It should — the variable is function-local,
   never address-taken. §2's identifier case sidesteps this entirely.
2. **The multi-value ternary.** v1 §7.2 builds `A, B` as
   `(sub == A) ? true : (sub == B)`, which may lower to a `select` rather than
   branches, and a `select` of comparisons is not the shape SimplifyCFG folds.
   If the `.ll` check shows this, the fix is to keep single-value arms as bare
   comparisons (v1 already does) and accept that multi-value arms are a linear
   probe. That is fine: multi-value arms are for `"+", "-"` groupings of two or
   three, not for forty.

**`src/backends/native/nativegen.c` — never, by policy.** `MASTER_PLAN.md` §10 says nativegen is
maintained, not optimised. It is already ~4x off on tight loops. A jump table
there would be our own optimiser pass, which is the thing we said we would not
write.

### 5.2 The number, if you want one anyway

For a hand-written table to beat a linear chain of *unboxed integer* compares,
the arithmetic is: a predicted compare-and-branch is ~1 cycle, so a chain costs
~N/2 on average; a table costs an indexed load plus an indirect branch, ~4-5
cycles when the branch predicts and ~15-20 when it does not. So:

- **~8-10 arms** if the dispatch is unpredictable (the indirect branch misses).
- **Never**, if one arm dominates dynamically — a chain with the hot arm first
  beats a table at any N, because the hot compare predicts perfectly.

For *string* arms — which is what zl dispatch actually is — no jump table exists
at all. The only table is a hash lookup, and hashing a short key costs roughly
what 2-4 short `strcmp`s cost, putting the crossover around **8-16 arms**. Note
where that lands: `src/selfhost/compiler.zl:36-37` is a twelve-way string dispatch,

```
fn is_keyword(w) {
    return w == "if" or w == "else" or w == "while" or w == "for" or w == "in" or w == "fn" or w == "return" or w == "not" or w == "and" or w == "or" or w == "true" or w == "false"
}
```

twelve words against the C lexer's fifteen (`src/frontend/lexer.c:46-62`). So zl's single
most dispatch-heavy site sits *right at* the crossover, which is the strongest
possible argument that we are not near the point where this matters. And when it
does matter, `design_maps_v2.md` gives the fix in the language rather than the
compiler: a map from keyword to token type, one hash instead of twelve
`strcmp`s, written in zl. **A user-level map beats a compiler-level jump table
here, and it needs nothing from `switch`.**

### 5.3 So: what would still justify an `N_SWITCH`?

One trigger survives from v1's three, and it is not performance:

- **Fallthrough, if it is ever accepted.** It cannot be desugared (v1 §4), so it
  forces a real node the day it is accepted. It is not accepted.

The other two — the measured bottleneck (§5.1 kills it) and a source-fidelity
formatter (still not on any roadmap) — should come out. And the asymmetry v1
identified still decides it: a desugaring is one function per parser; an
`N_SWITCH` is a `NodeType` in `src/frontend/parser.h` plus arms in `src/runtime/interp.c`, `src/backends/c/compile.c`,
`src/backends/llvm/compilel.c`, `src/backends/c/compilef.c`, `src/backends/native/nativegen.c`, plus a tree tag *and* a codegen arm
in `src/selfhost/compiler.zl` — eight places that must agree, against a three-engine gate.

**Recommendation unchanged: desugar. Do not add `N_SWITCH`. Do not write a jump
table.**

---

## 6. `src/selfhost/compiler.zl` — correcting v1 §8

v1 §8's conclusion was:

> the work is **one `parse_switch` function of ~40 lines in `src/selfhost/compiler.zl`'s parser
> and zero lines in its codegen.**

**The "zero lines in its codegen" half is wrong.** `src/selfhost/compiler.zl:536-574`,
`emit_stmt`, dispatches on five tags — `"expr"`, `"assign"`, `"return"`,
`"if"`, `"while"`, `"for"` — and ends:

```
    return pad(ind) + emit_expr(node) + ";\n"
}
```

There is **no `"block"` tag.** A `["block", ...]` node reaching `emit_stmt`
falls through to `emit_expr`, which will not recognise it either. `emit_block`
exists (`src/selfhost/compiler.zl:528`) but is only ever called from the `if`, `while` and
`for` arms with a block it already knows about — it is never reached from
statement position.

The desugaring's outer wrapper is exactly such a block: `__zl_sub0 = SUBJ`
followed by the `if` chain, wrapped in one `N_BLOCK` so it is one statement. So
teaching `src/selfhost/compiler.zl` switch needs **a `"block"` arm in `emit_stmt` as well** —
three lines, calling `emit_block(node, ind)` — and that arm is also what the
already-shipped `for`-range and `do`/`while` will need when their turn comes. It
is shared debt, not switch's.

Two smaller notes while correcting this section:

- `src/selfhost/compiler.zl`'s assign arm is `"v_" + node[1] + " = " + emit_expr(node[2])`
  (`:541-543`), so **assignment targets must be bare names**. `__zl_sub0` is a
  bare name, so it is fine — but this confirms the desugaring must never assign
  to an index.
- §2's identifier case removes the wrapper entirely for `switch x { ... }`,
  because with no hoist there is no assignment to pair with the chain — the
  rewrite is a single `N_IF`. So the most common shape needs **no** `"block"`
  arm. That is a second, unlooked-for benefit of not hoisting.

v1 §8's other three claims are re-verified and stand: the fixpoint is unaffected
as long as `src/selfhost/compiler.zl` does not itself use `switch` (`src/selfhost/compiler.zl:366-389`,
`parse_statement`, still handles only `if`/`while`/`for`/`fn`/`return`/assign/
expr — no `do`, no `loop`, no for-range, no `elif`); programs using `switch`
remain C-toolchain-only; and positional recognition is what keeps the two
keyword lists from drifting (`src/frontend/lexer.c:46-62` has 15 words, `src/selfhost/compiler.zl:36-37`
has 12 — they are already three apart, and a keyword would make that gap
semantic instead of cosmetic).

---

## 7. Revised checklist and size

| File | Change | Δ from v1 |
|---|---|---|
| `src/frontend/lexer.c` | none | — |
| `src/frontend/parser.h` | none | — |
| `src/frontend/parser.c` | `leads_switch()` forward scan (~12); `parse_switch()` (~75, up from 70: the identifier/hoist branch); duplicate detection with `strtod` (~30); one dispatch line at `parse_statement` (`:911-925`) | +15 |
| `src/runtime/interp.c` | none | — |
| `src/backends/c/compile.c` | none | — |
| `src/backends/c/compilef.c` | `case N_BLOCK:` in `emit_stmt` (`:86`) — **ship separately, before switch** | moved out |
| `src/backends/native/nativegen.c` | `case N_BLOCK: gen_block(n); break;` in `gen_stmt` (`:203`) — same | moved out |
| `src/backends/llvm/compilel.c` | none | — |
| `src/selfhost/compiler.zl` | none now. Later: `parse_switch` (~45) **and a `"block"` arm in `emit_stmt` (~3)** | v1 said 0 in codegen |
| `docs/REFERENCE.md` | the form; C-toolchain-only; list-literal arm cost (§3.2); map arms undefined (§4) | +3 lines |
| `tests/` | v1 §10's eleven programs, plus the three in §7.1 below | +3 |

**Size: ~120 lines of C, all in `src/frontend/parser.c`** (v1 said ~90; the extra is the
identifier branch and a fuller duplicate check), **plus 6 lines across two
backends shipped as a separate prior commit**, plus ~48 lines of zl in
`src/selfhost/compiler.zl` whenever the self-hosted compiler catches up.

**Depends on:** nothing. It can land today against the language as it stands.
It *interacts* with `design_records.md` (§3, free), `design_nullable.md` (§2,
requires the no-hoist rule), `design_maps_v2.md` (§4, requires them to define or
disclaim `==`) and `design_scoping_decision.md` (§2.3, unaffected either way).
None of those is a prerequisite. If switch ships first, records and nullables
inherit a working dispatch statement; if they ship first, nothing about this
design changes.

### 7.1 Three tests to add to v1 §10

12. **Identifier subject emits no hidden variable** — `switch x { 1 {} }`
    compiled by `src/backends/c/compile.c`; assert `out.c` contains no `__zl_sub`. This is the
    regression test for §2, and it is the one that fails if someone
    "simplifies" `parse_switch` back to always hoisting.
13. **Non-identifier subject still hoists** — `switch bump() { ... }` with a
    printing counter, asserting `1`. v1's test 2, kept, because §2 narrows its
    scope rather than removing it.
14. **Record tag dispatch** — once `rec` exists, `switch s.kind { "circle" {} }`
    across all engines that have both features. Guards the §3.1 claim that this
    needs no switch-specific work.

---

## 8. Open questions and things I am not sure about

- **Does LLVM actually fold our chain?** §5.1's crossover-of-4 rests on
  `MinJumpTableEntries` defaulting to 4 and on SimplifyCFG recognising the shape
  we emit. I could not run a build in this session. **Verify by dumping the
  `.ll` for a 12-arm integer switch before quoting the number as fact.** If it
  does not fold, the answer is still "desugar" — we just stop claiming a table.
- **Does the ternary block the fold?** §5.1's second point. Unknown, testable in
  the same `.ll` dump. Low stakes: it only affects multi-value arms.
- **Floats.** Another workflow is adding floats to the fast backends. Numeric
  case arms are compared as C `double`s (`src/runtime/interp.c:1240-1241`), so `switch`
  inherits whatever `==` on floats ends up meaning, including that `1` and `1.0`
  are the same case (v1 §6's `strtod` point, which becomes *more* load-bearing
  once float literals are common). I have not read the float work and cannot say
  whether it changes `values_equal`; if it does, this section is stale.
- **`switch` on the LHS of the forward scan in `src/selfhost/compiler.zl`.** v1 §2.1 offered
  the one-token guard as a fallback if the forward scan is awkward to mirror in
  `src/selfhost/compiler.zl`'s hand-written token loop. Having now read that loop
  (`src/selfhost/compiler.zl:135-165` — `kind()`, `text()`, `advance()`, `is_sym()`), a
  forward scan needs an index-based lookahead over the token list, which that
  code does have. I believe it is fine, but I have not written it.
- **Should `else` be spelled `else` when the subject is nullable?**
  `switch p { nil() { } else { } }` reads slightly oddly — the `else` is the
  *non*-nil case. I do not think this justifies a second default-arm keyword,
  but it is the kind of thing that looks fine in a design doc and grates in real
  code. Flagging it rather than deciding it.
