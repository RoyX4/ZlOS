# Design: Nullable Types (`T?`) for zl — the narrowing design

**Status:** proposal / not built · **Floor:** 1 (widening the seed) · **Date:** 2026-08-02

> **On the citations.** Every `file:line` below was read out of the tree on
> 2026-08-02 and every "measured" row was produced by running `interp.exe`, not
> from memory. `src/runtime/interp.c`, `src/runtime/runtime.c` and the backends were being edited
> concurrently by another agent while this was written, so treat line numbers as
> pointers, not anchors — the quoted code is what matters.

**Scope:** one postfix type constructor (`T?`), two builtins (`must`, `or_else`),
one flow analysis (narrowing) in the type pass, and matching checks in
`src/runtime/interp.c` / `src/runtime/runtime.c`. **Zero lexer changes. Zero runtime representation
changes. Zero new reserved words. `struct Node` unchanged.**

---

## 0. TL;DR

1. **Spelling is `T?`** — postfix, on any type: `int?`, `str?`, `list[int]?`,
   `list[int?]`. `any?` is a type error (`any` already contains `nil`).
   `nil` alone is still not a type, upholding `design_type_system.md` §9.
2. **The lexer needs no change.** `?` is already an accepted one-character
   symbol (`src/frontend/lexer.c:256`), added for the ternary. There is no two-character
   hazard, so `?` is strictly cheaper than the `->` that §2.5 of the type-system
   doc had to argue for.
3. **No collision with the ternary.** `?` is *infix with a mandatory `:`* in
   expression grammar (`src/frontend/parser.c:537-549`) and *postfix with no operand* in type
   grammar. Type position is closed — four sites, enumerated in §3.3 — and a
   ternary cannot occur in any of them.
4. **Zero runtime change.** A `T?` is represented exactly as `any` is today: a
   boxed `Value` that may carry `V_NIL`. `T?` is a *type-level* distinction over
   a value shape the runtime already has. This is why Tier C is the cheap
   collapse.
5. **What's legal on a `T?` unwrapped:** `==`, `!=`, `str()`, `print()`,
   `type()`, storage, passing, returning, and truth-testing. Everything the
   surveyed corpus actually does to an untested sentinel (§1.4) stays legal.
6. **What's illegal:** `- * / %`, `< > <= >=`, indexing, `len()`, and — the
   important one — **`+`**. `nil + 1` today produces the string `"nil1"`
   silently (measured, §2). Under an annotation it becomes a compile-time
   rejection. That is a rejection, never a changed answer, so Neutrality holds.
7. **Unwrapping is primarily *narrowing by test*, not new syntax.** The checker
   recognises the four shapes the corpus already writes — including the
   dominant `if type(x) == "nil" { return ... }` early-return idiom (12 of the
   surveyed sites).
8. **The hard constraint — no catchable errors — is answered by
   `refuse at compile time`, with exactly one opt-in abort.** A `T?` reaching a
   `T` context is a **compile-time error**. The only way to abort is to write
   `must(x)` yourself, which is `assert()` with a value and uses the same
   `exit(1)` path that already exists (`src/runtime/interp.c:224-226`, `src/runtime/runtime.c:723`).
   `or_else(x, d)` is the total alternative. **Nothing implicitly aborts and
   nothing implicitly defaults.**
9. **Narrowing is unsound on globals** because of `env_assign`'s global-slot
   rule (`src/runtime/interp.c:180-185`). Containment: parameters are always narrowable;
   other locals are narrowable *iff no global of that name exists*; globals are
   never narrowable. This is decidable statically and is the whole mitigation.

### 0.1 Relationship to `design_type_system.md`

That document (2026-07-30) cut this feature explicitly, twice, in §9:

> | **Union / optional / nullable types** | Every one of them requires
> flow-sensitive narrowing to be usable, and narrowing is its own design. `any`
> covers the need today. |
>
> | **A `nil` type** | Without unions, a `nil`-typed slot holds exactly one
> value. Useless. Cut from `design_types.md`'s set. |

**This document is that narrowing design.** It does not reopen the `nil`-type
cut — `nil` remains un-writable as a standalone type. It adds one *constructor*
over the existing type set and the minimum flow analysis that makes it usable.
Everything in `design_type_system.md` §1–§8 stands unchanged; this is additive.

It also **corrects one stale fact** in that document, see §3.2.

---

## 1. The survey — what `nil()` is actually doing

### 1.1 Method

`grep -rn "nil()"` over `stdlib/*.zl`, `examples/*.zl`, `tests/*.zl` and
`src/selfhost/compiler.zl`, then reading every hit and classifying it as *producer* (returns
`nil()` as "absent"), *consumer* (tests for it), or *value* (uses `nil` as a
first-class datum, not a sentinel). Call sites were then traced by name.
The `tests/` copies of `stdlib` functions are excluded from the counts below —
`tests/test_algorithms.zl` and `tests/test_structures.zl` inline copies of
`dijkstra.zl`, `astar.zl`, `segtree.zl`, `deque.zl` and `heapq.zl`, so counting
them would double-count the same design.

### 1.2 The producers — 22 functions across 11 files (measured)

| File | Functions returning `nil()` as "absent" |
|---|---|
| `stdlib/linalg.zl` | `determinant`, `inverse`, `lu_decompose`, `solve` |
| `stdlib/deque.zl` | `dq_pop_front`, `dq_pop_back`, `dq_peek_front`, `dq_peek_back` |
| `stdlib/segtree.zl` | `seg_get`, `seg_query`, `brute` |
| `stdlib/astar.zl` | `astar`, `bfs_path` |
| `stdlib/heapq.zl` | `heap_peek` (+ `heap_pop` returns `[h, nil()]`) |
| `stdlib/memo.zl` | `memo_get`, `lru_get` |
| `stdlib/statemachine.zl` | `fsm_step`, `fsm_run` |
| `stdlib/dijkstra.zl` | `dj_get` |
| `stdlib/graphx.zl` | `topo_sort` |
| `stdlib/combinat.zl` | `next_permutation` |
| `stdlib/calculus.zl` | `bisect_sign_change` |

Two further files handle `nil` without producing it, which brings the Tier C
file count to **13**:

- `stdlib/jsonw.zl:30` — `if t == "nil" { ... }`, renders JSON `null`.
- `stdlib/sortx.zl:36` — `if sx_t == "nil" { return 0 }`, the type-order rank.

> **Honest note on the count.** The task brief that prompted this document
> states *"13 files and 33 functions"*. 13 files reproduces exactly. 33 functions
> does not: I measure **22** in `stdlib`, and I can only reach the low thirties
> by including the duplicated copies in `tests/`. The design does not depend on
> which number is right — the shape of the sites is identical either way — but
> the smaller number is the one I can defend from the tree.

### 1.3 Two test idioms, and the one that wins

```
stdlib/astar.zl:66          if type(pc_path) == "nil" {
stdlib/statemachine.zl:48   if type(cur) == "nil" {
stdlib/segtree.zl:52        if type(op_a) == "nil" {
stdlib/graphx.zl:247        if type(soOrder) == "nil" {
stdlib/jsonw.zl:30          if t == "nil" {
stdlib/sortx.zl:36          if sx_t == "nil" { return 0 }
```

versus

```
stdlib/dijkstra.zl:25       return di_d == nil()
stdlib/combinat.zl:168      while p != nil() {
```

**The string test wins roughly 6:1.** Any narrowing design that only understands
`x != nil()` fails on most of the corpus it is meant to serve. §5.2 recognises
both, and the `type(x) == "nil"` form is not an afterthought — it is the primary
one.

### 1.4 The finding that decides the design: most call sites never test

This is the load-bearing measurement. Tracing the call sites of the producers:

```
stdlib/deque.zl:107     print("pop empty:  " + str(dq_pop_front(d_q)))
stdlib/heapq.zl:154     print("peek empty:  " + str(heap_peek(hp)))
stdlib/memo.zl:147      print("memo get sq:9:   " + str(memo_get(m, "sq:9")))
stdlib/linalg.zl:637    print("   inverse(S)     = " + str(inverse(S)) + "      (expect nil)")
stdlib/segtree.zl:185   if seg_query(ck_t, ck_a, ck_b) != brute(ck_xs, ck_a, ck_b, ck_t[2]) {
```

The majority of uses feed the possibly-nil value **straight to `str()`**, or
**compare it with `!=`**, and never branch on it. Both of those operations are
*total* on `nil` (§2), which is why the corpus works.

Two consequences follow directly, and they are the two halves of this design:

- **A `T?` must be usable without unwrapping** for the whole total set, or 13
  files would need edits to gain an annotation they did not ask for. That
  contradicts `design_type_system.md` §3.2's identity property ("the stdlib
  modules need zero edits").
- **Unwrapping must be needed only where the operation is partial** — and the
  survey says that is a *minority* of sites. The cost of this feature is
  therefore proportional to the arithmetic done on sentinels, which is small.

### 1.5 Three real defects the survey turned up

Reported here because a design doc that surveys code and stays silent about what
it found is wasting the survey. None of these are *caused* by this proposal;
two of them are *fixed* by it.

**(a) `dj_get` conflates "unreachable" with "not a node".**

```
stdlib/dijkstra.zl:38-46   fn dj_get(dists, node) { ... return nil() }
tests/test_algorithms.zl:938   check("dij Z unreachable", dj_get(t_r2, "Z"), nil())
tests/test_algorithms.zl:945   check("dij missing node is unreachable", dj_get(t_r1, "nope"), nil())
```

One `nil` answers two different questions, and the test suite *locks that in* by
name. `T?` does not fix this — a single nullable type cannot distinguish two
absences either. Flagged so that nobody expects it to. Fixing it needs a sum
type, which is out of scope and stays out (§10).

**(b) `seg_op`'s `nil` is an identity element, not an absence.**

```
stdlib/segtree.zl:25-29  # The identity element is nil(), not 0 or a big sentinel.
stdlib/segtree.zl:51-57  fn seg_op(op_kind, op_a, op_b) {
                             if type(op_a) == "nil" { return op_b }
                             if type(op_b) == "nil" { return op_a }
```

This is a *value*, deliberately chosen because it cannot collide with a legal
sum/min/max. Its type is genuinely `num?` and its consumer genuinely wants both
cases. It types cleanly under this design (`fn seg_op(k: str, a: int?, b: int?)
-> int?`) — worth stating, because it is the case most likely to be mistaken for
"a sentinel that should have been refactored away". It should not be.

**(c) A latent silent-wrong-answer at `linalg.zl:600`.**

```
stdlib/linalg.zl:596   AINV = inverse(A)
stdlib/linalg.zl:600   PROD = la_mul(A, AINV)
```

`inverse` is declared to return `nil()` on a singular matrix
(`stdlib/linalg.zl:17-18`) and `AINV` is passed to `la_mul` with no test. `A` is
non-singular *in this demo*, so it works. Annotate `la_mul(a: list[list[float]],
b: list[list[float]])` and this becomes a compile-time rejection. That is the
feature earning its keep on code that already exists.

---

## 2. What `nil` does today — measured, not remembered

Every row below was produced by running `interp.exe` on a scratch file, not by
reading code. This table *is* the specification of "legal unwrapped", because
Neutrality (`design_type_system.md` §6.2) forbids an annotation from changing any
of it.

| Expression | Result today | Total? |
|---|---|---|
| `str(nil())` | `"nil"` | yes |
| `print(nil())` | `nil` | yes |
| `type(nil())` | `"nil"` | yes |
| `nil() == nil()` | `true` | yes |
| `nil() != 1` | `true` | yes |
| `[nil()] + [1]` | `[nil, 1]` | yes |
| `len([nil()])` | `1` | yes |
| `if nil() {` | falsy (`src/runtime/interp.c:66-70`) | yes |
| **`nil() + 1`** | **`"nil1"`** | **yes — and that is the problem** |
| `nil() - 1` | `runtime error: this operator needs numbers`, `exit(1)` | **no** |
| `len(nil())` | `runtime error: len needs a string or a list`, `exit(1)` | **no** |
| `nil()[0]` | `runtime error: only lists can be indexed`, `exit(1)` | **no** |

Three conclusions, and they carry most of the argument in this document:

1. **The language already aborts on a failed unwrap.** For `-`, `*`, `/`, `%`,
   `<`, `>`, `<=`, `>=`, `len` and indexing, a `nil` that slipped through
   already ends the process with a message. `src/runtime/interp.c:1347` is the single guard
   (`if (l.type != V_NUM || r.type != V_NUM) runtime_error(...)`), mirrored at
   `src/runtime/runtime.c:320`. So "a failed unwrap aborts" is not a *new* failure mode this
   design introduces — it is the status quo, and this design's job is to move it
   from run time to compile time.
2. **`+` is the sole exception, and it is silent.** `eval_plus`
   (`src/runtime/interp.c:1180-1201`) falls through to `value_to_string` on any operand pair
   that is not num+num or list+list. A leaked `nil` becomes the four characters
   `"nil"` glued into a string, and the program keeps going with a wrong answer.
   This is the one place where the feature turns a silent wrong answer into a
   loud rejection, which is a strict improvement.
3. **`or` cannot be a default operator.** `eval_binary` returns
   `make_bool(is_truthy(l) || is_truthy(r))` (`src/runtime/interp.c:1334`, mirrored at
   `src/runtime/runtime.c:318`) — zl's `or` yields `true`/`false`, **not the operand**. So
   the Python/JS idiom `x or default` is unavailable and `??` cannot be defined
   as sugar over it. §5.4's `or_else` builtin exists because of this line.

There is a fourth, quieter consequence. `nil` is falsy, but so are `0`, `""`,
`false` and `[]` (`src/runtime/interp.c:66-77`). `if x` is therefore *not* a nil test for
`int?`, `str?`, `bool?` or `list[T]?`. `statemachine.zl:13-14` already documents
having been bitten by the neighbouring version of this:

> `# "No transition" is reported as nil(), which is a distinct value from any`
> `# state name (nil() != 0 and nil() != "")`

§5.2 therefore refuses to treat truthiness as a narrowing form.

---

## 3. Syntax: `T?`

### 3.1 Why `?` and not a named constructor

| Candidate | Verdict |
|---|---|
| **`T?`** | **Chosen.** Zero lexer cost (§3.2), postfix so it reads as a modifier on a type you already wrote, one character, and universally recognised from Kotlin/Swift/C#/TypeScript. |
| `option[T]` | Reads like a generic. zl has exactly one bracketed type (`list[T]`) and it is special-cased, not a general type constructor (`design_type_system.md` §9 cuts generics). Introducing a second bracketed type invites `map[K,V]`, `result[T,E]`, and a real generic system. |
| `nil \| T` | `\|` is not in the accepted symbol set (`src/frontend/lexer.c:256`) — a new token, unlike `?`. And it is a *union*, which is the thing §9 cut wholesale. `T?` is deliberately the degenerate one-member union and nothing more. |
| `T = nil` default-value syntax | Conflates absence with a default. §6.4. |

**`any?` is a type error**, not a synonym for `any`. `any` is defined as "a
pointer to a boxed `Value`" (`design_type_system.md` §3.2) and a boxed `Value`
can already be `V_NIL`. Allowing `any?` would give two spellings for one type
and would immediately raise "does `any` exclude nil, then?". The error message
should say so.

**`T??` is a parse error.** Idempotence is a trap: it lets a macro-ish or
generated annotation stack `?`s silently. One `?` or none.

### 3.2 The lexer is already done — correcting a stale fact

`design_type_system.md` §2.2 and §2.5 state that `:` must be *added* to the
lexer's symbol set, and quote:

```c
/* design_type_system.md §2.2, quoting src/frontend/lexer.c:198 */
if (strchr("(){}[],.+-*/%=!<>", c) == NULL) {
```

**That is no longer the code.** As of today the set already contains both
characters, at `src/frontend/lexer.c:252-258`:

```c
    /* '?' and ':' are only ever the two halves of a ternary. Neither
       starts a two-character symbol, so there is nothing above to
       shadow: adding them here can only turn text that used to be a
       lexer ERROR into text that lexes. */
    if (strchr("(){}[],.+-*/%=!<>?:", c) == NULL) {
        die(line, "I don't know this character:", c);
    }
```

The ternary landed after that document was written and took `:` and `?` with it.
So:

- **`T?` costs zero lexer changes.** Not "one line" — zero.
- The `->` argument in §2.5 of that document still stands and is still needed
  for function signatures, along with its `brainfuck.zl` hazard (R4). `?` has no
  equivalent hazard because it is one character: there is no two-character
  ladder for it to jump ahead of, and comments and string literals are consumed
  by `next_token` before `lex_symbol` is ever reached
  (`src/frontend/lexer.c:265-292`) regardless.

Measured, on the current binary — the annotation position is still a hard parse
error, so nothing that parses today changes meaning:

```
$ ./interp.exe _nulchk2.zl        # y: int = 5
line 1: expected a value (got ':')
$ ./interp.exe _nulchk3.zl        # z: int? = 5
line 1: expected a value (got ':')
$ ./interp.exe _nulchk.zl         # x = 1 ? 2 : 3
2
```

The ternary works; `:` in annotation position is rejected at the *parser*, not
the lexer. That is exactly the state `design_type_system.md` §2.2 wanted, now
reached for free.

### 3.3 Proof that `T?` cannot collide with the ternary

The ternary is parsed at the loosest precedence, `src/frontend/parser.c:537-549`:

```c
static Node *parse_ternary(void)
{
    Node *cond = parse_or();
    if (!is_sym("?")) return cond;

    advance();                       /* eat '?' */
    Node *n = new_node(N_TERNARY);
    n->a = cond;
    n->b = parse_ternary();          /* value if true  */
    expect_text(":");
    n->c = parse_ternary();          /* value if false */
    return n;
}
```

Two structural facts make the collision impossible rather than merely unlikely:

1. **In expression grammar `?` is strictly infix and requires a matching `:`.**
   It is only ever consulted *after* `parse_or()` has produced a left operand,
   and `expect_text(":")` makes the colon mandatory. A `?` with no following `:`
   is already a parse error.
2. **In type grammar `?` is strictly postfix and consumes nothing else.**
   `parse_type` handles it and returns.

The two never meet because **type position is closed**. There are exactly four
sites where `parse_type` is entered, and none of them can contain an expression:

| Site | Grammar | What follows the type |
|---|---|---|
| Variable annotation | `IDENT ':' type '=' expr` | `=` |
| Parameter annotation | `IDENT ':' type` inside `fn f( ... )` | `,` or `)` |
| Return annotation | `')' '->' type` | `{` |
| List element | `'list' '[' type ']'` | `]` |

In every one of them the parser has *already committed* to reading a type before
it sees the `?` — it got there by consuming a `:` after an identifier in a
declaration/parameter, or a `->`, or a `[` after the contextual `list`. There is
no lookahead decision of the form "type or expression?" to get wrong, so there is
no ambiguity to resolve and no disambiguation rule to document.

Worked, in both directions:

```
x: int? = nil()          # parse_type reads 'int', eats '?', sees '=' -> done
fn f(a: int?) -> str? {  # 'int' '?' then ')' ; 'str' '?' then '{'
xs: list[int?] = []      # inner type inside the brackets
ys: list[int]? = nil()   # outer '?' after the ']'
p = c ? 1 : 2            # no ':' after an IDENT at statement start
                         #   -> never enters parse_type at all
```

**The one case worth stating out loud** is `ys: list[int]? = nil()` versus
`xs: list[int?] = []`. They differ only in `?` placement and they mean different
things — a possibly-absent list of ints, versus a present list of
possibly-absent ints. That is not ambiguity (both parse deterministically), it is
a *readability* hazard. The checker should render types back in error messages in
exactly this spelling so the two are visibly distinct.

**Uncertainty, stated:** this analysis assumes annotations always carry an
initializer (`x: T = v`), which is true of every form in
`design_type_system.md` §2.1 and true of zl generally — the language has no
declaration-without-assignment (that document's §5.1 step 4 says so: *"declared
but never assigned — impossible today"*). If a bare `x: int?` declaration is
ever added, re-run this proof; a bare declaration is the one shape that puts a
type at the end of a statement, where a trailing `?` has less context.

### 3.4 Type grammar delta

Against `design_type_system.md` §2.4:

```
type    := base '?'?
base    := 'int' | 'float' | 'bool' | 'str' | 'any'
         | 'list' '[' type ']'
```

with two static rules that are checked after parsing, not encoded in the
grammar, so that the error message can be specific:

- `any?` → error: *"`any` already includes nil; write `any`."*
- `T??` → error: *"a type is nullable or it is not; one `?`."*

`list` with no `[...]` remains shorthand for `list[any]`, so `list?` is
`list[any]?` — a possibly-absent list of anything.

### 3.5 `struct Node` still does not change

`design_type_system.md` §2.6 makes a point of `struct Node` being untouched, and
that property survives. Nullability needs one bit and there is no free field, so
**spell it into the type node's text**:

| zl type | `N_TYPE` node |
|---|---|
| `int` | `{text:"int"}` |
| `int?` | `{text:"int?"}` |
| `list[int]` | `{text:"list", a:{text:"int"}}` |
| `list[int]?` | `{text:"list?", a:{text:"int"}}` |
| `list[int?]` | `{text:"list", a:{text:"int?"}}` |

The `?` is part of the type name string, exactly as `"list"` is. No new node
type beyond the `N_TYPE` that document already appends to the end of the enum,
no new field, no enum renumbering (its R8). Type equality is `strcmp` on the
base plus a recursive compare of `a` — which is what it would have been anyway.

---

## 4. Semantics

### 4.1 Representation: nothing changes at run time

**A `T?` has the same representation as `any`: a boxed `Value` that may carry
`V_NIL`.** Not a tagged union, not a sentinel bit pattern, not a nullable
pointer.

This is the entire reason Tier C is the cheap collapse and it should be said
plainly: `T?` buys **checking**, not **speed**. A `list[int?]` is a boxed list,
same layout and same cost as `list[any]` (`design_type_system.md` §4.4). A
function returning `int?` returns a boxed `Value`.

Two honest consequences:

- **Annotating a sentinel-returning function makes it no faster.** The 22
  functions in §1.2 stay on the boxed path. What they gain is that their
  *callers* get told at compile time when they forgot the test.
- **The unboxing win is at the boundary, not inside.** `fn dq_pop_front(q) ->
  int?` is boxed, but a caller that writes `n: int = must(dq_pop_front(q))` has
  an unboxed `i64` from that point on, and the checker knows it. `must` is
  exactly a boxing boundary in the §4.3 sense — an `any`→`T` unbox with one
  extra tag test.

A future "nullable unboxed `int` in an `i64` with a separate `i1` present-flag"
representation is possible and is deliberately **not** designed here. It would
be a pure backend change under this same surface, which is the point of pinning
the surface first.

### 4.2 Assignability

Extending `design_type_system.md` §3.4:

```
assignable(A, B)  iff  A == B
                    or A == any   or  B == any
                    or (A == int  and B == float)
                    or (A == list[X] and B == list[Y] and X == Y)
                    or (A == T    and B == T?)              # NEW: T widens into T?
                    or (A == int  and B == float?)          # NEW: widening composes
```

Read `assignable(A, B)` as "a value of type `A` may be stored in a slot of type
`B`". The new clauses say: **`T` flows into `T?` freely; `T?` never flows into
`T` implicitly.** That asymmetry is the whole feature.

`nil()` itself has type `nil`, which is *not writable in source* (§3.1) but
exists inside the checker as the type of the `nil()` builtin's result. It is
assignable to every `T?` and to `any`, and to nothing else. This is how §9's "a
`nil` type is useless" cut is honoured — there is still no way to *declare*
a slot that holds only nil.

Invariance is preserved and matters here: `list[int]` is **not** assignable to
`list[int?]`. They have the same boxed layout today, so the restriction costs
nothing at run time, but permitting it would let a caller write `nil()` into a
list the callee believes is `list[int]`. The classic covariant-array hole,
refused for the same reason `design_type_system.md` §3.4 refuses
`list[int]` → `list[any]`.

### 4.3 Legal on a `T?` without unwrapping

Directly from the measurement in §2, and chosen so that **every site in §1.4
type-checks with no source edit**:

| Operation | Result type | Why it is safe |
|---|---|---|
| `x == y`, `x != y` | `bool` | `values_equal` is total (`src/runtime/interp.c:1257`) |
| `str(x)` | `str` | `value_to_string` renders `V_NIL` as `"nil"` (`src/runtime/interp.c:84`) |
| `print(x)` | — | same path |
| `type(x)` | `str` | `src/runtime/interp.c:858-861` handles `V_NIL` in its `default:` |
| assign to a `T?` or `any` slot | — | §4.2 |
| pass to a `T?` or `any` parameter | — | §4.2 |
| `return x` from `-> T?` or `-> any` | — | §4.2 |
| element of a `list[T?]` or `list[any]` | — | §4.2 |
| `if x`, `while x`, `and`, `or`, `not x` | `bool` | `is_truthy` is total (`src/runtime/interp.c:66-77`) |

That last row is legal **but never narrows** — see §5.2. It is legal because
refusing it would break Neutrality: `if x { }` runs today and must keep running
identically.

### 4.4 Illegal on a `T?`

| Operation | Today, if nil leaks | Under `T?` |
|---|---|---|
| `x + y` | silent `"nil1"` | **compile error** |
| `x - y`, `*`, `/`, `%` | `runtime error`, `exit(1)` | compile error |
| `x < y`, `>`, `<=`, `>=` | `runtime error`, `exit(1)` | compile error |
| `x[i]` | `runtime error`, `exit(1)` | compile error |
| `len(x)` | `runtime error`, `exit(1)` | compile error |
| assign `T? → T` slot | (no such concept) | compile error |
| pass `T?` to a `T` parameter | (no such concept) | compile error |

Every error message must name the fix, in the same spirit as R2's requirement
that the `int/int` message literally suggest `int(...)`:

```
type error: 'total' is int? and '+' needs an int
  line 14:  total = total + a[i]
  a nullable value has to be unwrapped first. Either:
      test it       - if total != nil() { ... }
      default it    - or_else(total, 0)
      or assert it  - must(total)          # aborts if it IS nil
```

**Only the `+` row is a behaviour change** in the sense a user will notice, and
it is a rejection rather than a different answer, which is exactly the licence
`design_type_system.md` §3.5 grants: *"annotated code may be refused; it may
never quietly do something else."* The other rows convert an abort-at-run-time
into an abort-at-compile-time on the same program.

---

## 5. Unwrapping

### 5.1 The principle: recognise what the corpus already writes

The cheapest possible unwrap syntax is **none**. The 13 files in §1.2 already
test their sentinels, in two idioms (§1.3), and the design's first obligation is
to understand those idioms rather than ask 13 files to be rewritten in a new
one. So narrowing is primary, and the two builtins in §5.4 exist only for the
cases narrowing provably cannot reach.

### 5.2 The narrowing forms recognised (v1)

A **narrowing test** is a boolean expression the checker can read as "this name
is / is not nil". Exactly four shapes, each chosen because it appears in the
tree:

| # | Shape | Seen at |
|---|---|---|
| N1 | `x == nil()` / `x != nil()` | `dijkstra.zl:25`, `combinat.zl:168` |
| N2 | `type(x) == "nil"` / `type(x) != "nil"` | `astar.zl:66`, `statemachine.zl:48`, `segtree.zl:52`, `graphx.zl:247` |
| N3 | `t = type(x)` then `t == "nil"` — one level of let-binding | `jsonw.zl:29-30`, `sortx.zl:35-36` |
| N4 | `not <N1|N2|N3>` | (defensive; `not` is the only negation, `src/runtime/interp.c:1493`) |

N3 is not gold-plating. Both files that consume `nil` without producing it use
it:

```
stdlib/sortx.zl:35-36
    sx_t = type(sx_v)
    if sx_t == "nil" { return 0 }
```

Without N3 the two *consumer* files in Tier C fail to narrow, and they are the
two files most likely to be annotated first (they are small and total).

Where narrowing applies:

- **In an `if`:** the then-branch and the else-branch each get the refined type.
  `if x != nil() { <x: T> } else { <x: nil> }`.
- **In a `while` header:** the body gets the refined type. `combinat.zl:168`
  needs exactly this.
- **In a ternary:** `x != nil() ? f(x) : 0` — the then-arm gets `x: T`. This is
  the expression-position unwrap and it needs no new syntax, because the
  ternary already exists.
- **After an early exit — the dominant form.** If the then-branch cannot fall
  through (it ends in `return`, `break` or `continue`), the *rest of the
  enclosing block* gets the else-refinement:

  ```
  stdlib/astar.zl:66-72
      fn path_cost(pc_path) {
          if type(pc_path) == "nil" {
              return -1
          }
          if len(pc_path) == 0 {          # <- pc_path is list[...] here, not list[...]?
  ```

  This is what `astar.zl`, `statemachine.zl`, `graphx.zl`, `memo.zl`,
  `linalg.zl` and `segtree.zl` all write. **Without early-exit narrowing the
  feature does not fit the corpus at all.** It is the only genuinely new
  analysis this design requires: a `block_exits(stmt) -> bool` predicate over
  `N_RETURN` / `N_BREAK` / `N_CONTINUE` and the two arms of an `N_IF`. Perhaps
  40 lines, and it is a pure function of the AST.

**Truthiness is deliberately NOT a narrowing form.** `if x { }` does not refine
`x`, because `0`, `""`, `false` and `[]` are falsy too (`src/runtime/interp.c:66-77`). For
`int?` the branch `if x { }` is entered for neither nil *nor* zero, so treating
it as a nil test would be simply wrong for `x == 0`. It stays legal (§4.3) and
inert, and the checker should suggest `x != nil()` when it sees a `T?` in a
condition position and the body then fails to type. `statemachine.zl:13-14`
documents the neighbouring bug; do not reintroduce it.

### 5.3 Where narrowing does not apply — the global-slot problem

This is the second of the two language-level blockers, and it is fatal to naive
narrowing. `env_assign` (`src/runtime/interp.c:180-185`):

```c
static void env_assign(Env *e, const char *name, Value val)
{
    Var *found = env_find(e, name);
    if (found) { found->val = val; return; }
    env_define(e, name, val);
}
```

`env_find` walks the parent chain, so **an assignment inside a function writes
the global slot when a global of that name exists.** It is deliberate — it is
what lets `src/selfhost/compiler.zl` share a cursor across functions.

The consequence for narrowing:

```
cur: int? = 0

fn step() { cur = nil() }        # writes the GLOBAL cur

fn use() {
    if cur != nil() {
        step()
        print(str(cur + 1))      # cur is nil here. Narrowing lied.
    }
}
```

**Containment rule, decidable statically:**

1. **Parameters are always narrowable.** Parameter binding is scoped — the
   only thing in the language that is — so no callee can reach a parameter.
2. **A non-parameter local is narrowable iff no global of that name exists.**
   Function environments parent directly to the global environment
   (`src/runtime/interp.c:1404`: `Env *call_env = env_new(g_global);`), never to the
   caller's, so a callee can only clobber a name that is global or its own.
   The checker already walks the whole file and therefore already knows the
   global name set.
3. **A global is never narrowable.** To use one, copy it into a *local whose
   name is not itself global* and narrow that.
4. **A call inside a narrowed region does not invalidate cases 1 and 2** — by
   construction, those names are unreachable from a callee. It cannot occur in
   case 3, since case 3 never narrows.

Rule 2 is the one that could surprise: adding a top-level `x = 0` to a file can
turn a previously-narrowing `x` inside a function into a non-narrowing one, and
the error appears in a function nobody edited. That is a real wart. It is the
*language's* wart, surfaced rather than created — the same variable was already
being silently shared. The checker's message must name the global:

```
type error: 'cur' cannot be narrowed - a global named 'cur' exists (line 1),
            so any call in this block may reassign it.
            Copy it into a local first:  c = cur
```

**Uncertainty, stated:** rules 1–4 are sound against the *interpreter's*
scoping as it stands today. If `design_type_system.md`'s work ever makes typed
function bodies use true lexical locals in the compiled backends but not in
`src/runtime/interp.c`, the two engines would narrow differently and Neutrality would break
before this feature ever ran. Any change to `env_assign` or to `env_new(g_global)`
must re-check this section.

### 5.4 The two builtins

Narrowing cannot cover everything, and this is provable from the tree:

```
stdlib/memo.zl:121-124
    if memo_has(FIB_CACHE, key) {
        return memo_get(FIB_CACHE, key)
    }
```

`memo_has` implies `memo_get` is non-nil, and no analysis short of a theorem
prover knows that. `design_type_system.md` §5.3 already rules out inference
across function boundaries. So an escape hatch is mandatory, and there are two
shapes of need:

| Builtin | Signature | Behaviour |
|---|---|---|
| `must(x)` | `T? -> T` | if `x` is nil: print a message and `exit(1)`. Otherwise `x`. |
| `or_else(x, d)` | `(T?, T) -> T` | if `x` is nil: `d`. Otherwise `x`. Total. |

Both are ordinary identifiers — zero reserved words, per `MASTER_PLAN.md` §4.3,
exactly like `print` and `assert`. Both must be added to `src/runtime/interp.c`'s
`call_builtin` **and** `src/runtime/runtime.c`'s (parity is load-bearing; `src/runtime/runtime.c:717`
and `src/runtime/interp.c:826` are the matching `nil` entries to copy the shape from).

`must` uses the abort path that already exists. `assert` is
`src/runtime/interp.c:830-834` / `src/runtime/runtime.c:720-724`; `must` is that same `fflush` +
`fprintf(stderr, ...)` + `exit(1)`, differing only in returning the value on
success. It should name the source line:

```
nil unwrap failed: must() at memo.zl:122
```

In untyped code both are still useful and both still work — `must` on an
untyped `any` is a runtime tag check, `or_else` a runtime tag check plus a
select. That matters for adoption: a file can start using `must`/`or_else` for
clarity before anyone annotates it.

### 5.5 Non-behaviours

- **No `?.` safe-navigation.** It would need a second new token *and* a
  decision about what `x?.y` returns when `x` is nil, which is a nullable, which
  needs unwrapping anyway. It saves nothing here because zl has no field access
  yet (`design_memory_structs.md` §4.5 is where `.field` is being designed).
  Revisit only after structs land.
- **No `??`.** `or_else(x, d)` is the same thing spelled with letters, and §2's
  third conclusion means `??` could not be defined in terms of `or` anyway.
- **No narrowing through a function call.** `if is_present(x) { ... }` does not
  refine `x`. §5.4 exists precisely for this.
- **No narrowing of list elements or indexed expressions.** `if xs[0] != nil()`
  does not refine `xs[0]`, because `xs` is mutable in place
  (`src/runtime/interp.c:1563-1580`, index-assign) and two evaluations of `xs[0]` need not
  agree. Only bare names narrow.
- **No flow-sensitive *re*-widening beyond the block.** A narrowed name reverts
  to `T?` at the end of the block that narrowed it (except for the early-exit
  case, §5.2, whose scope is explicitly the rest of the enclosing block).

---

## 6. The hard constraint: zl has no catchable errors

`runtime_error` is `fflush(stdout); fprintf(stderr, ...); exit(1)`
(`src/runtime/interp.c:224-226`). There is no `try`, no error value convention, no
`errno`-style out-parameter. A failed unwrap therefore cannot throw. The task
brief is right that this forces a choice, and it must be made explicitly.

### 6.1 The three options

| Option | What happens when a `T?` reaches a `T` context | Cost |
|---|---|---|
| **(a) Abort** — implicit unwrap, checked at run time | process dies with a message | Every `T?` use site becomes a potential process death that is invisible in the source. The type annotation would *add* crash sites to a program that previously stringified (`+`) or was already crashing anyway. Worst of both: the checking overhead with none of the guarantee. |
| **(b) Default** — implicit unwrap to `0` / `""` / `[]` | program continues with a made-up value | Silently wrong answers, and it re-creates precisely the bug `dijkstra.zl:16-22` documents having fixed: a magic default that collides with a legal value. See §6.4. |
| **(c) Prove it cannot happen** — reject at compile time | the program does not compile | The programmer must write the test or write `must`. Some correct programs are refused (§6.3). |

### 6.2 Chosen: (c), refuse — with exactly one opt-in abort

**A `T?` reaching a context that needs `T` is a compile-time error.** The
program is not built. Nothing aborts implicitly and nothing defaults implicitly.

The one abort in the design is `must(x)`, and it is *written by the programmer,
at the site, on purpose*. This is the same shape as everything else zl has
decided: `assert()` already aborts, and `design_memory_structs.md` §3.5 puts a
`!` marker on raw memory for the same reason — **the wall is erected by the
programmer, on the declaration they chose** (`design_type_system.md` §9, last
row).

`must` is not a loophole in (c); it is what makes (c) honest. Without it, (c)
would mean "some correct programs are impossible to write", which is the
complaint that sinks strict null checking in practice. With it, (c) means "you
may assert what the checker cannot prove, and the assertion is visible in the
source and named in the crash".

The rejection is also the *only* choice compatible with Neutrality
(`design_type_system.md` §6.2), which is worth spelling out because it is the
decisive argument, not a nicety:

- Option (a) would make an annotated program abort where its annotation-stripped
  twin printed `"nil1"`. **Different output. Neutrality violated.**
- Option (b) would make an annotated program print `0` where its twin printed
  `"nil1"`. **Different output. Neutrality violated.**
- Option (c) refuses the annotated program and never runs it, so no pair of
  outputs can disagree. **Neutrality holds trivially.**

`design_type_system.md` §3.5 states the rule this instantiates: *"annotated code
may be refused; it may never quietly do something else."* Nullable is the
sharpest test of that rule, and it passes.

### 6.3 What (c) costs, stated before anyone discovers it

1. **Correct programs will be refused.** Any invariant the checker cannot see —
   `memo_has` before `memo_get`, a matrix known non-singular by construction, a
   deque known non-empty because you just pushed — is refused until someone
   writes `must`. That is friction on real, working code, and it is the price.
2. **`must` is a crash the type system talked you into.** A programmer under
   deadline will write `must` to make the error go away, and has then
   *converted a silent wrong answer into a crash* — usually an improvement, but
   not always the one they wanted. The error message (§4.4) must list
   `or_else` before `must` for exactly this reason.
3. **Annotating a file is not free.** `linalg.zl` gains real errors at
   `linalg.zl:600` and `linalg.zl:648` the day someone annotates `la_mul`.
   That is the feature working, and it is still work.
4. **There is no partial credit.** A function annotated `-> int?` forces every
   caller to deal with it. Annotating a widely-called producer is a
   whole-callers-list change, which is why the staging in §9 annotates
   consumers before producers.

### 6.4 Why (b), the implicit default, was rejected in particular

Because this codebase already made that mistake once and wrote down what it
cost. `stdlib/dijkstra.zl:16-22`:

```
# UNREACHABLE IS nil, NOT A BIG NUMBER. It used to be 1000000000, which sits
# INSIDE the range of legal distances - the contract only asks for "a
# non-negative number" weight and puts no ceiling on the total. Two ordinary
# weights of 600000000 give a true distance of 1200000000, which is not
# "< 1000000000", so the edge was never relaxed and a reachable node was
# reported unreachable. nil cannot collide with a distance, however large.
```

An implicit default is a magic value, and a magic value collides with a legal
one. That is the whole reason `nil` is used as a sentinel in this tree in the
first place. A design that answered "what happens on a failed unwrap?" with
"you get `0`" would be reintroducing the bug at the language level, where it
would be unfixable rather than merely wrong.

Defaults are still available — as `or_else(x, 0)`, written at the site, by the
person who knows that `0` is safe there.

### 6.5 Cross-reference: `design_error_handling.md`

`docs/design/design_error_handling.md` (2026-08-02) landed while this document
was being written. It settles the assumption §6 rests on, and the two documents
turn out to be **mutually blocking**, so the relationship is worth stating
precisely rather than gestured at.

**What it confirms.** Its recommendation is *(c), the out-of-process harness* —
a PowerShell script that watches exit codes from outside — and it explicitly
rejects `try`/`catch` on engine cost:

> A real `catch` is five different mechanisms, not one … `src/backends/native/nativert.c` has **no
> error path whatsoever** so it would need raising built first, in hand-written
> x86-64.

So the status quo this document assumed is not merely the current state, it is
the *decided* state for the foreseeable future. §6.1's option (a) "abort" and
option (c) "refuse" are being weighed in a language that will not gain a
recoverable failure mode any time soon. That strengthens (c): a compile-time
refusal is the only failure this design can offer that a program could ever
have *acted* on.

**What it blocks on.** Its §6 trigger 2 names this document as the gate:

> **Nullable/Option lands with flow-sensitive narrowing.** Then (d) costs a
> fraction of today's price and must be re-costed immediately.

where (d) is errors-as-values (`fn f() -> int | err`).

**The honest correction.** That trigger over-reads what shipping `T?` provides.
Errors-as-values needs **two** things — flow-sensitive narrowing *and* a general
union type — and this document supplies only the first. §10 cuts general unions
deliberately, and `T?` is specifically the *degenerate* one-member union chosen
because it is the one the corpus demonstrably needs. `int | err` is a real
union: it carries a payload, `|` is not in the symbol set (`src/frontend/lexer.c:256`), and
its narrowing forms are not N1–N4. When trigger 2 fires, the re-costing should
find that the narrowing machinery in §5.2 and the block-exit analysis are
reusable more or less as-is, and that the type-level work is still ahead. That
is a genuine discount, but it is not the whole bill.

**One point in the other direction.** Its §4.6 observes that `7 % 0` is an
integer divide that the CPU kills the process over — *"exit code is not 1"* —
so some existing failure paths are worse than an orderly abort. `must(x)`
failing produces a message on stderr and a clean `exit(1)` through the path at
`src/runtime/interp.c:224-226`, which the harness in that document can observe and assert
on. So the one abort this design introduces is, by that document's own
instrument, a *testable* failure rather than an untestable crash.

---

## 7. Interaction with the `+` blocker

`+` is overloaded and it is a *tested guarantee*, not an accident.
`tests/test_syntax.zl:372-378`:

```
s = "n="
s += 5
check("string += number",  s, "n=5")
n = 5
n += "x"
check("number += string",  n, "5x")    # + is a join as soon as one side is text
```

`eval_plus` (`src/runtime/interp.c:1180-1201`) implements it and `src/runtime/runtime.c:290-310`
mirrors it. Note that both directions are locked, so `+` cannot be narrowed to
"numeric unless the *left* side is a string" either.

For nullable, the interaction is simpler than for the general type system,
because of the measured asymmetry in §2:

| Operand types | Today | Under annotation |
|---|---|---|
| `any + any` | stringify / add / concat by tag | unchanged — boxed path, `zl_binop` |
| `int + int` | add | add, unboxed |
| `str + int` | stringify | rejected (`design_type_system.md` §3.5, already decided) |
| **`int? + int`** | **`"nil1"` if nil, else adds** | **rejected here** |
| `str? + str` | `"nil..."` if nil, else concatenates | rejected here |

The nullable rows are the ones this document adds, and they are the *only* rows
where today's behaviour is a silently wrong answer rather than a loud one. That
makes `+` the single best argument for the feature: it is precisely where the
existing runtime does not protect anyone.

Note carefully what is **not** claimed: this does not make `+` decidable in
general. `design_type_system.md`'s constraint stands — you cannot emit an
integer `add` without knowing both operand types, and unannotated code stays on
`zl_binop` forever. `T?` narrows *which* annotated programs are accepted; it does
not widen what can be unboxed.

---

## 8. Engine behaviour: check, don't use

`design_type_system.md` §6 chose **"the interpreter CHECKS annotations and
IGNORES them for representation"**, with the three-way table at its §6.1. That
choice carries over unchanged and needs no re-argument, but it has three
concrete obligations here:

1. **`src/runtime/interp.c` and `src/runtime/runtime.c` both gain `must` and `or_else`.** Parity between
   them is load-bearing — they are two implementations of one builtin table, and
   `run_tests.ps1` is what catches drift. Add both in the same commit.
2. **The interpreter does not need the narrowing analysis.** Narrowing decides
   *acceptance*, and acceptance is decided by the type pass over `Node*` that
   §6 says is shared. `src/runtime/interp.c` runs the accepted program with boxed values
   exactly as today. There is no second implementation of narrowing to drift.
3. **The Neutrality harness (§6.2 of that document) covers this feature for
   free.** `strip_annotations(P)` deletes `: T?` and `-> T?` the same way it
   deletes `: T`. It must **not** delete `must`/`or_else` — they are ordinary
   calls with runtime meaning, not annotations. Worth a test: a program using
   `or_else` must produce identical output annotated and stripped.

**`src/selfhost/compiler.zl`** needs the same ~20-line additive skip logic
`design_type_system.md` §7.1 describes, extended by one character: after
skipping a type name, skip an optional `?`. The fixpoint risk called out in
`design_selfhost_parity.md` — `src/selfhost/compiler.zl` concatenating a *number* onto a
*string* to emit C, where unboxing could flip `"3"` to `"3.0"` — is untouched by
this proposal, because `T?` changes no numeric representation and `must` /
`or_else` emit ordinary boxed calls. Do not annotate `src/selfhost/compiler.zl` (that
document's R7 and Stage 8 still apply).

---

## 9. Interaction with the other designs

| Document | Interaction |
|---|---|
| `design_type_system.md` | This is its §9 "nullable" row, unblocked. §1–§8 unchanged. One stale fact corrected (§3.2 here). |
| `design_maps.md` | Direct hit. `get(m, k)` is specified to return "value for `k`, or `nil` if absent" — that is `V?` where `V` is the value type, and `has_key(m,k)` is its companion test. When maps land, `get` should be typed `-> any?` and the N1/N2 narrowing forms apply unchanged. `get(m, k, default)` is `or_else` by another name; consider defining one in terms of the other rather than shipping two defaulting mechanisms. |
| `design_memory_structs.md` | **Not the same thing.** A struct there is an address in a `double`, and "no struct" would be address `0`, not `V_NIL`. `T?` deliberately does not extend to struct handles — a null-pointer type is a separate design and should not be smuggled in under this syntax. |
| `design_selfhost_parity.md` | `?` in annotations is one more feature `src/selfhost/compiler.zl` must skip. It widens the measured 63-of-110 gap by zero files today (nothing uses annotations yet) and by one line of skip logic once they do. |
| `design_floats.md` | `float?` is well-formed and needs nothing special. NaN is *not* a nil and must not be conflated with one — `nil != NaN`, and `type(NaN) == "num"`. |
| `design_switch_case.md` | If a `switch` on a `T?` lands, a `nil` arm should narrow the other arms to `T`. Out of scope here; noted so it is not designed twice. |

---

## 10. What we are explicitly not doing

| Not doing | Why |
|---|---|
| **General unions (`A \| B`)** | `design_type_system.md` §9's cut, still right. `T?` is the single degenerate case that 13 files demonstrably need; nothing in the corpus needs `int \| str`. |
| **Distinguishing two absences** | §1.5(a)'s `dj_get` wants "unreachable" and "no such node" to differ. That needs a sum type with payloads. Genuinely out of scope. |
| **`?.` safe navigation** | §5.5. No field access exists yet. |
| **`??`** | §5.5. `or_else` covers it and `or` cannot implement it (`src/runtime/interp.c:1334`). |
| **Non-null-by-default** | Would mean every unannotated `any` is non-null, which is false — `any` includes `V_NIL` today and must keep doing so. Nullability is opt-in on the annotation, like everything else. |
| **Narrowing through calls** | Needs effect analysis. §5.4's `must` is the escape. |
| **Narrowing globals** | §5.3. Unsound as the language stands. |
| **An unboxed nullable representation** | §4.1. Pure backend work, deliberately deferred so the surface can be pinned first. |
| **Making `nil` a writable type** | §9 of the type-system doc cut it and the cut still holds: a slot that can only hold nil is useless. |
| **Changing `type()`'s output** | It must keep returning `"nil"`, because six files branch on that exact string. |
| **Auto-migrating the stdlib** | The 13 files gain annotations by hand, consumer-first, one PR at a time (§11). |

---

## 11. Risks

| # | Risk | Mitigation |
|---|---|---|
| R1 | **Early-exit narrowing is the whole feature and it is the only new analysis.** Get it wrong and 6 of 13 files fail to annotate. | It is a pure AST predicate (`block_exits`). Test it directly on `astar.zl:66`, `statemachine.zl:48`, `graphx.zl:247`, `memo.zl:51`, `segtree.zl:52` before any of them is annotated. |
| R2 | **The global-slot rule silently disables narrowing** when an unrelated top-level name matches a local (§5.3, rule 2). | The error message names the offending global and its line. Do not fail silently by treating the value as `any`. |
| R3 | **`must` becomes the default fix** because it is the shortest, converting silent wrongness into crashes. | Error text lists `or_else` first and `must` last, with the word "aborts" attached (§4.4). |
| R4 | **`list[int]?` vs `list[int?]` confusion.** | Render types in the canonical spelling in every message; add both to the test corpus. |
| R5 | **`src/runtime/interp.c` / `src/runtime/runtime.c` builtin drift** on `must` / `or_else`. | Same commit, and a `run_tests.ps1` case exercising both engines. This is the documented failure mode for every builtin so far. |
| R6 | **`seg_op`'s identity-`nil` gets "refactored away"** by someone who reads it as a sentinel. | §1.5(b) says out loud that it is a value, and its annotation `-> int?` is correct as written. |
| R7 | **No error design exists** (§6.5), so §6's whole decision rests on an assumption. | Stated as an assumption, not a fact. Revisit if a `design_errors.md` ever lands. |
| R8 | **Uncertainty about the count** of affected functions (§1.2, 22 vs the brief's 33). | The design is insensitive to it — the site *shapes* are what matter and they are enumerated. Recount before quoting a number in a changelog. |

---

## 12. Staged plan and definition of done

**Stage N0 — spec only (this document).** No code.

**Stage N1 — syntax.** `parse_type` accepts a trailing `?` and encodes it in the
type node's `text` (§3.5). `any?` and `T??` are errors. `src/selfhost/compiler.zl`'s skip
logic skips the `?`. Nothing checks anything yet. *Done when:* `x: int? = nil()`
and `fn f(a: int?) -> str? { ... }` parse on all engines, `verify.ps1` is still
green, and every stdlib module still runs byte-identically.

**Stage N2 — the builtins.** `must` and `or_else` in `src/runtime/interp.c` and
`src/runtime/runtime.c`, with tests that they behave identically. Usable in untyped code
immediately. *Done when:* `run_tests.ps1` covers `must` on nil (exit 1, correct
message) and non-nil, and `or_else` both ways, on both engines.

**Stage N3 — assignability.** §4.2's two new clauses, plus the §4.3 / §4.4
legal/illegal split. No narrowing yet, so `T?` is nearly unusable — that is
expected and is why N4 follows immediately. *Done when:* the `+` rejection fires
with the §4.4 message.

**Stage N4 — narrowing.** N1–N4 forms, block-scoped, plus the early-exit rule
and the §5.3 containment. *Done when:* all five of R1's named sites narrow
correctly, and a global-shadowed local produces R2's message rather than a wrong
answer.

**Stage N5 — annotate the corpus, consumers first.** `sortx.zl` and `jsonw.zl`
(consumers, small, total), then `statemachine.zl` and `astar.zl` (self-contained
producer+consumer pairs), then the rest. `linalg.zl` last — it has the most
call sites and §1.5(c)'s real defect. **Do not annotate `src/selfhost/compiler.zl`.**

**Definition of done for the feature:**

1. `T?` parses, checks, and runs identically on `interp.exe`, the C backend and
   the LLVM backend.
2. All 96 stdlib modules run **with zero edits** — nullable is opt-in and an
   unannotated file is unaffected, by construction (§4.1).
3. The Neutrality harness passes: for every annotated test program,
   `interp(P) == interp(strip_annotations(P))`.
4. `verify.ps1` reaches the same byte-identical fixpoint, with `src/selfhost/compiler.zl`
   unannotated.
5. Annotating `astar.zl` and `statemachine.zl` requires **no change to their
   bodies** — the existing `if type(x) == "nil"` tests are accepted as-is. If
   either needs a body edit, §5.2 got the narrowing forms wrong and the
   analysis, not the file, is what should change.
6. `stdlib/linalg.zl:600` is a compile error once `la_mul` is annotated, and the
   error names `must` and `or_else`.
