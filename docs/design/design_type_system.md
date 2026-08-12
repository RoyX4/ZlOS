# Design: a Gradual Type System for zl (the unboxing enabler)

**Status:** proposal
**Author:** language/compiler design pass, 2026-07-30
**Scope:** new optional syntax in `lexer.c` / `parser.c` / `parser.h`, a new type
inference + check pass, and the type consumer in `compilel.c` (the LLVM backend).
Also a ~20-line additive change to `compiler.zl`. No existing program changes
meaning. No code is changed by this document.

---

## 0. TL;DR

zl is slow for one structural reason: **every value is boxed.** `1 + 2` in a
compiled program is `zl_binop("+", zl_num(1), zl_num(2))` — a runtime string
compare, two `Value` structs, and (for anything with a payload) a `malloc`. The
LLVM backend (`compilel.c`) already emits `add i64` for the numeric subset and
proves the payoff, but it can only do that because it *assumes* everything is an
`i64`. That assumption is a lie for the language as a whole.

**Types are the mechanism that makes the assumption true where it is written
down, and keeps the boxed path everywhere else.**

- Optional annotations: `x: int = 5`, `fn f(a: int, b: float) -> int { }`.
- Six types: `int`, `float`, `bool`, `str`, `list[T]`, `any`.
- `any` *is* today's boxed `Value`. Unannotated code is entirely `any` and
  compiles exactly as it does now.
- Annotated code compiles to unboxed native values. Box/unbox happens at
  precisely-enumerated boundaries (§4).
- Full Hindley-Milner is ruled out — `stdlib/jsonw.zl` legitimately builds
  `[1, "two", [3, 4], null]` and HM would reject it (§1.2).
- **Hard requirement: all 54 stdlib modules keep working with ZERO edits.** They
  contain no annotations, so they are 100% `any`, so nothing about them changes.

The governing invariant for the whole feature:

> **Annotation Neutrality.** For any program whose numeric values stay inside the
> shared range (§3.3), *adding or removing annotations must not change its
> output.* Annotated code may be **rejected**; it must never silently produce a
> **different answer**.

Neutrality is mechanically testable — strip the annotations, run both, diff — and
it is the gate every stage in §7 is graded against.

### 0.1 Relationship to `docs/design/design_types.md`

That earlier proposal (2026-07-29) designed optional typing as a **bug-catching
checker**. This one designs it as a **representation**. They agree on the core
posture (gradual, opt-in, `any` as the escape hatch, no new reserved words) and
this document reuses its assignability rule and its built-in signature table
wholesale. It supersedes it on three points, because the goal changed from
*safety* to *speed*:

| | `design_types.md` | this document | why the change |
|---|---|---|---|
| Numeric type | one `num` | `int` **and** `float` | `num` pins no machine representation, so it cannot unbox. Splitting is the entire point. |
| Return spelling | `fn f(x): num {` | `fn f(x) -> int {` | §2.3 — and the "`->` costs a token" objection is answered by the proof in §2.2. |
| Consumer | a `check()` pass in zl | `compilel.c` codegen | the checker is now a *prerequisite*, not the deliverable. |
| `nil` type | present | **cut** | with no union types, a `nil`-typed slot holds exactly one value. Useless. `any` covers it. |

---

## 1. Why gradual, and why not inference

### 1.1 The cost of boxing, measured against what exists

`compilef.c` (unboxed `long long` C) already runs ~2x faster than `compile.c`
(boxed) on `fib`, and it achieves that with **no type system at all** — it simply
declares by fiat that the whole program is `long long`. It works only on the
numeric subset and would silently corrupt any program touching a string.

`compilel.c` has the same shape and the same fiat: `emit_expr` produces
`add i64` / `sdiv i64` / `icmp` and calls `exit(1)` on `N_STRING`, `N_LIST`,
`N_CALL`, `N_FN`, `N_IF`, `N_WHILE` — literally everything else. Its header
comment says "Values are unboxed i64."

So the unboxed backend is not blocked on codegen skill. **It is blocked on not
knowing which values are safe to unbox.** That is the hole this document fills.

### 1.2 Why Hindley-Milner is off the table

HM would infer everything with no annotations, which sounds strictly better. It
is not, because zl's own standard library is legitimately, deliberately dynamic.

`stdlib/jsonw.zl` is the clearest case:

```
fn to_json(v) {
    t = type(v)
    if t == "num"  { return str(v) }
    if t == "bool" { return str(v) }
    if t == "nil"  { return "null" }
    if t == "str"  { return "\"" + json_escape(v) + "\"" }
    if t == "list" { ... for x in v { ... to_json(x) ... } ... }
}
```

`v` has no HM type. It is *by design* a union of every value in the language,
discriminated at runtime by the `type()` builtin. `to_json([1, "two", [3, 4]])`
must work, so `v`'s list case is a heterogeneous list. HM would demand
`list[a]` with a single `a` and reject it.

`stdlib/json_parse.zl` (parse returns "whatever was in the file") and
`stdlib/lisp_interp.zl` (an evaluator whose values are, definitionally, dynamic)
have the same shape. So does `compiler.zl` itself, whose entire AST is
`["bin", "+", L, R]` — a tagged heterogeneous list.

An inference algorithm that rejects the language's own standard library is not a
feature, it is a rewrite. **Gradual typing is not a compromise here; it is the
only design that fits the corpus.**

### 1.3 What "gradual" buys, concretely

You annotate the leaf that is hot and leave everything else alone:

```
fn dist2(x1: float, y1: float, x2: float, y2: float) -> float {
    dx = x2 - x1
    dy = y2 - y1
    return dx * dx + dy * dy
}
```

`dist2` becomes four `double` registers and five SSE instructions with no
allocation. The caller, the printing, the list of points — all still boxed, all
still working, zero edits.

---

## 2. Syntax

### 2.1 The two forms

```
# variable annotation:  name ':' type '=' value
count: int   = 0
ratio: float = 0.5
name:  str   = "Zac"
ok:    bool  = true
xs:    list[int] = [1, 2, 3]
box:   any   = read_config()        # explicit opt-out

# function annotation: per-parameter ':' type, return '->' type
fn area(w: int, h: int) -> int {
    return w * h
}
```

Every annotation is independent and every one is optional:

```
fn f(n) { }                        # unchanged from today, fully dynamic
fn f(n: int) { }                   # typed param, unstated (= any) return
fn f(n) -> int { }                 # untyped param, typed return
fn f(a: int, b) -> str { }         # mix
```

The variable form annotates the **declaration** (the first assignment). Later
assignments to the same name carry no annotation and are checked against the
declared type:

```
count: int = 0
count = count + 1        # fine
count = "three"          # type error
count: str = "three"     # type error: 'count' is already declared int
```

Re-annotating a name in the same scope is an error, not a redeclaration. zl has
no `let`, so allowing it would make the second `:` look like shadowing when it
is not.

### 2.2 Proof that the spelling collides with nothing (measured)

Both claims below were checked by running `interp.exe`, not by reading code.

**`:` is free.** `lexer.c:198` is

```c
if (strchr("(){}[],.+-*/%=!<>", c) == NULL) {
    die(line, "I don't know this character:", c);
}
```

`:` is not in that set, so *every* program containing a `:` outside a string or
comment is currently a hard lex error. Measured:

```
$ ./interp.exe t_colon.zl          # contains: x: int = 5
line 1: I don't know this character: ':'
```

Therefore adding `:` as a symbol token cannot change the meaning of any
currently-valid program. There is nothing to break.

**`->` is free.** Today `-` and `>` lex as two separate one-char `T_SYMBOL`s.
For `->` to appear in a *valid* program, the parser would have to accept `>` in
a position where an expression begins — but `>` is only ever consumed as an
infix comparison in `parse_cmp`, and both `parse_add`'s infix `-` and
`parse_unary`'s prefix `-` demand a primary next. `>` cannot start a primary.
Measured, both positions:

```
$ ./interp.exe t_arrow.zl          # contains: print(a -> b)
line 3: expected a value (got '>')
$ ./interp.exe t_arrow2.zl         # contains: fn f(a) -> int { return a }
line 1: expected '{' (got '-')
```

So merging `-` `>` into one `->` token cannot break a valid program either.

**The one real hazard, and why it is not one.** `->` occurs over 100 times
across 34 stdlib files — in comments (`# integer -> binary string`) and inside
string literals (`stdlib/brainfuck.zl:151`'s Brainfuck program contains `->`).
This is safe **only because `next_token` handles `#` comments and `"` strings
before it ever reaches `lex_symbol`** (`lexer.c:207-236`). The two-char `->`
check must go **inside `lex_symbol`**, alongside the existing `>=`/`+=` checks —
never earlier in the pipeline. The same ordering holds in `compiler.zl`'s
`next_token` (comments at l.66, strings at l.91, symbol fallback at l.108).
An implementer who "optimizes" by scanning for `->` before the string path will
corrupt `brainfuck.zl` and the failure will look like a Brainfuck bug.

**Type names stay ordinary identifiers.** The keyword list stays frozen at 14.
Measured — all four names are usable as variables today and must stay that way:

```
$ ./interp.exe t_names.zl
7            # int = 5 ; float = 2 ; print(int + float)
[1, 2]       # list = [1,2]
hi           # any = "hi"
3            # print(int(3.7))  <- the BUILTIN still wins in callee position
```

So `int`, `float`, `list`, `any` (and `str`, `bool`) are recognized **only in
type position**: immediately after a `:` in a declaration or parameter, after a
`->`, or inside `list[...]`. Everywhere else they are what they are today — a
variable name or a builtin. This costs zero reserved words and breaks nothing.

### 2.3 Why `->` and not `: T` before the block

`design_types.md` §2.3 chose `fn f(x): num {` on the grounds that `->` costs a
new token. §2.2 shows that cost is zero and provably non-breaking, so the
decision reopens on merit alone:

1. **A colon binds a type to a name. A return type has no name.** `a: int`
   reads "a is an int". `): int` reads as nothing — the colon has no left
   operand. `->` reads "produces".
2. **One line, two jobs.** `fn f(a: int, b: float): float` uses `:` at two
   different nesting levels for two different relationships. `-> float` keeps
   the parameter colons unambiguous to the eye.
3. **It mirrors the target.** The whole point of the annotation is to reach
   `define double @zl_f(i64 %a, double %b)`. `->` maps onto that shape; the
   colon form does not.
4. **Grep.** `grep -n '\->' *.zl` finds every typed signature in the tree.
   `grep '):'` finds nothing useful.

Pick one and only one. **No alias.** Supporting both spellings would double the
parser paths, the `compiler.zl` skip logic, and the test matrix for zero gain.

### 2.4 Type grammar

```
type    := 'int' | 'float' | 'bool' | 'str' | 'any'
         | 'list' '[' type ']'
```

Nesting is natural and allowed (`list[list[int]]`). `list` with no `[...]` is a
shorthand for `list[any]`. Anything else in type position — an unknown name, a
missing `]` — is a parse error naming the offending token, not a silent `any`;
a typo'd type must not degrade into "unchecked".

### 2.5 `lexer.c` changes

Two edits, both inside `lex_symbol`, both additive:

1. Add a two-char case for `-` followed by `>` producing `T_SYMBOL "->"`. It
   goes with the existing two-char ladder at `lexer.c:188-196`, **after** the
   `-=` check (so `-=` still wins) and **before** the one-char fallback.
2. Add `:` to the accepted-character set string at `lexer.c:198`.

Nothing else in the lexer moves. `T_SYMBOL` already carries arbitrary text, so
no new `TokenType` is needed and `lexer.h` is untouched.

### 2.6 `parser.h` / `parser.c` changes

**`struct Node` does not change.** This is worth stating loudly, because it is
what keeps the diff small and the other backends unbroken. Every annotation
fits in an already-unused slot:

| Node | uses today | free slot for the type |
|---|---|---|
| `N_ASSIGN` | `a` = target, `b` = value | **`c`** = declared type (or NULL) |
| `N_FN` | `text` = name, `kids` = params, `a` = body | **`b`** = return type (or NULL) |
| `N_IDENT` (as a param) | `text` = name | **`a`** = param type (or NULL) |

The only `parser.h` edit is one new enum member, **appended at the end** of
`NodeType` so no existing numeric value shifts (`compilel.c:105` and
`compilef.c` print raw `type %d` in their "not supported" errors):

```
N_TYPE      /* text = "int"/"float"/"bool"/"str"/"list"/"any";
               a = element type for list[T], else NULL          */
```

`parser.c` gets one new ~12-line `parse_type()` and three call sites:

- `parse_fn` (l.402-424): after each parameter's `advance()`, if the next token
  is `:`, consume it and hang `parse_type()` on that param's `a`. After
  `expect_text(")")`, if the next token is `->`, consume it and set `n->b`.
- `parse_statement` (l.451): after `Node *expr = parse_expr()`, if the next
  token is `:` **and** `expr` is a bare `N_IDENT`, consume it, parse the type,
  then require `=` and hang the type on the resulting `N_ASSIGN`'s `c`. A `:`
  after anything that is not a bare identifier (e.g. `xs[0]: int = 1`) is a
  parse error — annotating a subscript has no meaning.
- `print_node` gets an `N_TYPE` case so `parser.exe`'s tree dump stays complete.

**Audit of everything that walks the tree**, so Stage 1 provably changes no
behavior:

| Consumer | reaction to the new slots | verdict |
|---|---|---|
| `interp.c` `eval`/`exec` | `N_ASSIGN` reads `a`,`b`; `N_FN` reads `text`,`kids`,`a`. Never `c`/`b`. `N_TYPE` is never evaluated. | inert |
| `compile.c` | same slot discipline | inert |
| `nativegen.c` / `nativeval.c` | same | inert |
| `compilel.c` `collect_vars` (l.26-31) | recurses into `a`,`b`,`c`,`kids`, so it *will* walk `N_TYPE` nodes — but it only reacts to `N_ASSIGN`-with-`N_IDENT`-target | inert |
| `compilef.c` `collect_vars` | same | inert |

---

## 3. The type set

### 3.1 Representation

| zl type | LLVM IR (SSA) | in memory (global / alloca) | notes |
|---|---|---|---|
| `int` | `i64` | `i64` | two's-complement, wraps |
| `float` | `double` | `double` | IEEE-754, same as every engine today |
| `bool` | `i1` | `i8` | `zext`/`trunc` at the memory boundary |
| `str` | `ptr` → `%zlstr = { i64 len, ptr bytes }` | `ptr` | immutable; the same bytes a boxed `Value.str` points at |
| `list[T]` | `ptr` → `%zllist = { i64 len, i64 cap, ptr data }` | `ptr` | `data` is a **flat array of `repr(T)`** — this is the win |
| `list[any]` | `ptr` → today's boxed list object | `ptr` | **bit-identical layout to a dynamic list** — see §4.4 |
| `any` | `ptr` → `Value` (`runtime.h:14`) | `ptr` | today's boxed value, heap-allocated |

**Decision: `any` is `Value*`, not `Value`.** `runtime.h` passes and returns
`Value` *by value* (a 32-byte struct). Reproducing the Win64 struct-by-value ABI
in hand-written LLVM IR is fragile — a 32-byte aggregate is passed indirectly,
and getting that subtly wrong produces silent corruption, not a link error. So
Stage 5 adds a thin pointer-flavoured shim layer to `runtime.c`
(`zl_p_binop(ptr, ptr) -> ptr`, `zl_p_calln(ptr, i32, ...) -> ptr`, …) whose
only job is to heap-allocate and dereference, and the IR only ever passes `ptr`.
This costs one indirection on the already-slow boxed path and buys ABI safety on
the fast one. That is the right side of the trade.

### 3.2 What `any` means

`any` is not "unknown" and it is not a top type in a subtyping lattice. **`any`
is a concrete representation: a pointer to a boxed `Value`.** Everything that is
true of zl today is true of an `any` value: it carries a runtime tag, `type()`
reports on it, `zl_binop` dispatches on it, `+` stringifies mixed operands.

The consequences follow directly:

- Unannotated variable, parameter, return, list element, builtin result → `any`.
- An unannotated program is 100% `any` and compiles to exactly the code it
  compiles to today. **This is why the 54 stdlib modules need zero edits** — not
  as a compatibility promise, but as an identity.
- `any` is assignable in **both** directions (§3.4), so annotated and
  unannotated code interleave freely and no annotation can propagate an error
  into untyped code.

### 3.3 Numeric semantics (the part that can break Neutrality)

The interpreter stores every number as a C `double` (`interp.c:35`). An unboxed
`int` is an `i64`. These agree exactly for `|n| < 2^53` and diverge above it,
and on overflow.

**Rule:** `int` means "an integer-valued zl number". Arithmetic on it is i64
two's-complement. **A program whose integer intermediates exceed 2^53 is outside
the three-engine agreement guarantee** — precisely the same posture
`run_tests.ps1` already takes toward `100 / 3` ("use exact divisions here so all
3 engines agree"). This is documented, tested around, and revisited only if
`design_floats.md` §2.1's single-`f64` decision is ever reopened.

Operator typing, chosen so Neutrality holds:

| Operator | `int,int` | `int,float` / `float,float` | `str,str` | mixed with `any` |
|---|---|---|---|---|
| `+` | `int` | `float` | `str` (concat) | `any` (§4.2 site B7) |
| `- * ` | `int` | `float` | error | `any` |
| `%` | `int` (`srem`) | `float` (`frem`) | error | `any` |
| `/` | **`float`** | `float` | error | `any` |
| `== != < > <= >=` | `bool` | `bool` | `bool` | `any`→`bool` via `zl_truthy` |
| `and or not` | `bool` | `bool` | `bool` | `any` |

**`int / int` produces `float`, and that is not negotiable.** `/` is true
division in the interpreter and the C backend (`7 / 2 == 3.5`). If annotating
made it `sdiv`, the annotated program would print `3` and the unannotated twin
`3.5` — a direct Neutrality violation, and exactly the bug
`design_floats.md` §1.1 documents in the native backend. So:

```
n: int = 7
d: int = 2
q: float = n / d        # 3.5   — fine
q2: int  = n / d        # TYPE ERROR: float is not assignable to int
q3: int  = int(n / d)   # 3     — say what you mean
```

This is the one place the type system forces a source change, and it forces you
to write down which of two different answers you wanted. Good trade.

**`int` widens to `float` implicitly** (in binary operands, arguments, returns,
and assignments) because every number in the interpreter is already a double, so
the widening cannot change an answer in the guaranteed range. **`float` never
narrows to `int` implicitly** — that would lose information silently. Use
`int(x)`.

### 3.4 Assignability

Unchanged from `design_types.md` §3.2, plus widening:

```
assignable(A, B)  iff  A == B
                    or A == any  or  B == any
                    or (A == int and B == float)
                    or (A == list[X] and B == list[Y] and X == Y)
```

Note the last clause is **invariant**, not covariant: `list[int]` is *not*
assignable to `list[any]`. It cannot be — they have different memory layouts
(§4.4). Attempting it is an error with a message that says so and points at
`list[any]` or an explicit copy.

There is no other subtyping. No coercion. Predictable beats clever.

### 3.5 Mixed-type operations are rejected, not reinterpreted

`"n=" + 1` works today (the interpreter's `+` stringifies mixed operands) and
must keep working — it is unannotated, so it is `any + any` and it takes the
boxed path unchanged. But in fully-annotated position:

```
tag: str = "n=" + count      # count: int  ->  TYPE ERROR
tag: str = "n=" + str(count) # write it down
```

This is a *rejection*, not a changed answer, so Neutrality holds. Neutrality is
about outputs, not about acceptance: **annotated code may be refused; it may
never quietly do something else.**

---

## 4. Boxing boundaries — the crux

This is the section that decides whether the feature is fast or merely typed. A
boundary is not free: **boxing costs a `malloc`; unboxing costs a tag load, a
compare and a branch.** Boxing is the expensive direction by an order of
magnitude.

### 4.1 Where the boundary is decided

Per-expression, at codegen time, from the static type of each operand. The rule
is mechanical:

> Every expression has exactly one static type. When an expression of type `T`
> flows into a position that requires type `U`, emit the conversion for
> `(T → U)`. `T == U` emits nothing. `T → any` boxes. `any → T` unboxes and
> checks. `T → U` where both are concrete and unequal is either a widen
> (`int → float`, `sitofp`) or a compile error.

### 4.2 Every site that boxes (`T` → `any`)

| # | Site | Example | Emitted |
|---|---|---|---|
| B1 | argument to an **unannotated user function** | `shout(r)` where `fn shout(x)` | `zl_box_int(%r)` per arg; call returns `ptr` |
| B2 | argument to a **builtin not in the fast table** (§4.6) | `push(dyn, r)` | `zl_box_*` per arg |
| B3 | assignment to an **`any` variable** (declared `any`, or unannotated in an untyped body) | `log = count` | `zl_box_int` before the store |
| B4 | element stored into a **`list[any]`** | `xs[0] = count`, `push(xs, count)` | `zl_box_int` on the element |
| B5 | `return` from a function declared **`-> any`** or undeclared | `fn f(n: int) { return n }` | `zl_box_int` before `ret ptr` |
| B6 | element of a **list literal that unified to `list[any]`** | `[1, "two"]` | box each element |
| B7 | **typed operand meeting an `any` operand** | `count + dyn` | box the typed side, `zl_p_binop`, result `any` |

### 4.3 Every site that unboxes (`any` → `T`)

| # | Site | Example | Emitted |
|---|---|---|---|
| U1 | `any` expression assigned to a typed variable | `k: int = read_num()` | `zl_unbox_int` (checked) |
| U2 | `any` expression passed to a **typed parameter** | `hot(dyn)` where `fn hot(n: int)` | `zl_unbox_int` per arg (checked) |
| U3 | `any` expression returned from a fn declared `-> T` | `fn f() -> int { return dyn }` | `zl_unbox_int` (checked) |
| U4 | `any` element pushed into a `list[T]` | `push(nums, dyn)` where `nums: list[int]` | `zl_unbox_int` (checked) |
| U5 | `any` used as an index into a `list[T]` or `str` | `nums[dyn]` | `zl_unbox_int` (checked) |
| U6 | `any` used as a **condition** | `if dyn { }` | **no unbox** — `zl_truthy(ptr) -> i1` |

`zl_unbox_int(ptr)` is: load the tag, compare to `V_NUM`, branch; on the hot
path load the `double` and `fptosi` it; on the cold path tail-call
`zl_type_error("int", <actual>, "<site>")`, which prints
`type error: expected int, got str` and exits 1. **It does not coerce and it
does not return a default.** A silent fallback would turn a type system into a
guessing game and would break Neutrality in the worst possible way — a different
answer instead of an error.

### 4.4 The list problem — the one place the boundary is O(n)

`list[int]` is a flat `i64` buffer. A dynamic list is an array of pointers to
boxed `Value`s. **They cannot alias.** So converting between them is a copy,
proportional to length, plus a `malloc` per element in the boxing direction.

This is a real performance trap: `print(nums)` on a `list[int]` of a million
elements would allocate a million `Value`s. The design answer is three-part:

1. **`list[any]` is bit-identical to today's dynamic list.** So `list[any] ↔ any`
   is O(1) — a tag add/remove, nothing copied. If you need to hand lists across
   the boundary constantly, annotate `list[any]` and you pay nothing.
2. **`list[int]` / `list[float]` / `list[bool]` are the opt-in flat ones**, and
   crossing to `any` is O(n). The compiler **emits a warning** at every such
   site naming the variable, because the cost is invisible in the source.
3. **`list[T]` ↔ `list[any]` is not assignable at all** (§3.4). You must write
   the copy yourself, so the O(n) never happens by accident.

Guidance for users, stated in the reference doc when this ships: **annotate a
whole call chain, not one function in the middle.** Boundaries belong at the
outside edge of the hot region, where they are crossed once, not inside a loop.

### 4.5 Worked examples

**No boundary — a fully typed leaf.**

```
fn add3(a: int, b: int) -> int {
    return a + b + 3
}
```

```llvm
define i64 @zl_add3(i64 %a, i64 %b) {
entry:
  %t1 = add i64 %a, %b
  %t2 = add i64 %t1, 3
  ret i64 %t2
}
```

Compare with today's boxed C backend for the same function: two `zl_binop` calls
with a `strcmp` on the operator string and two 32-byte struct copies each.

**A boxing boundary (B1).**

```
fn hot(n: int) -> int { return n * n }
fn shout(x) { print(x) }          # unannotated
r: int = hot(7)
shout(r)
```

```llvm
  %t3 = call i64 @zl_hot(i64 7)
  store i64 %t3, ptr @v_r
  %t4 = load i64, ptr @v_r
  %t5 = call ptr @zl_box_int(i64 %t4)      ; <-- B1: malloc + tag
  %t6 = call ptr @zl_shout(ptr %t5)
```

**An unboxing boundary (U1).**

```
data = read_config()      # unannotated call -> any
k: int = data             # checked unbox
```

```llvm
  %t7 = call ptr @zl_read_config()
  %t8 = call i64 @zl_unbox_int(ptr %t7)    ; <-- U1: tag check, traps on mismatch
  store i64 %t8, ptr @v_k
```

**Both directions in one loop — the anti-pattern to warn about.**

```
fn step(x: int) -> int { return x * 2 + 1 }
acc = 0                     # unannotated -> any
i: int = 0
while i < 1000000 {
    acc = step(acc)         # U2 unbox in, B3 box out — every iteration
    i = i + 1
}
```

Two boundary crossings and one `malloc` per iteration. Annotating
`acc: int = 0` removes all three and the loop becomes pure registers. The
compiler should say so: *"note: 'acc' is `any` and crosses a boxing boundary
inside a loop; annotating it `int` removes 2000000 conversions."*

### 4.6 The fast-builtin table (v1)

Builtins are the most common boundary, so a small table keeps the obvious cases
unboxed. Everything not listed takes the boxed path (B2) and is `any -> any`.

| builtin | fast signature(s) | lowers to |
|---|---|---|
| `print` | `(int)`, `(float)`, `(bool)`, `(str)` | direct `zl_print_i64` / `_f64` / `_bool` / `_str` |
| `len` | `(str) -> int`, `(list[T]) -> int` | one load from the header |
| `push` | `(list[T], T) -> list[T]` | inline grow, no box |
| `at` | `(str, int) -> str` | one-char slice |
| `abs` `min` `max` | `int`/`float` overloads | LLVM intrinsic |
| `sqrt` `floor` `ceil` | `(float) -> float` | LLVM intrinsic |
| `int` | `(float) -> int`, `(any) -> int` | `fptosi` / `zl_unbox_int` |
| `str` | `(int) -> str`, `(float) -> str` | canonical formatter (see `design_floats.md` §2.3) |

Keep this table short in v1. Every entry is a place the fast path can drift from
the boxed path, and every drift is a Neutrality bug.

---

## 5. Local inference inside a typed body

**Only signatures need annotating.** Locals are inferred, so the annotation
burden is O(functions), not O(variables).

**Scope of inference (v1):** a function body is inferred only if the function is
**typed** — at least one parameter or the return carries an annotation. Bodies of
fully-unannotated functions are compiled exactly as today, fully boxed, with no
inference at all. Top-level statements are inferred too, seeded by whatever
`x: T` declarations exist.

Rationale for the restriction: it makes "did my function get fast?" answerable by
looking at one line, and it keeps the Stage 3 diff to `compilel.c` contained.
Inferring inside untyped bodies is a real later win (§9), not a v1 obligation.

### 5.1 The algorithm

The type lattice is shallow — `⊥ < {int, float, bool, str, list[T]} < any`, with
`int < float` — so a fixed point is reached in at most 3 iterations.

```
unify(⊥, T)             = T
unify(T, T)             = T
unify(int, float)       = float          # and symmetrically
unify(list[X], list[X]) = list[X]
unify(anything else)    = any            # incl. list[X] with list[Y], X != Y
```

1. **Seed.** Parameters get their declared type (unannotated → `any`). Globals
   get their declared type or `any`. Every other local starts at `⊥`.
2. **Iterate.** Walk the body in source order. For each `N_ASSIGN` to a bare
   identifier: `t = typeof(rhs)`; `local = unify(local, t)`. For each `N_FOR`:
   `loopvar = unify(loopvar, elem(typeof(seq)))`, where `elem(list[T]) = T` and
   `elem(anything else) = any`. Reading a local still at `⊥` yields `⊥`, which
   unifies away harmlessly.
3. **Repeat** until nothing changes. Cap at 4 passes; if anything is still
   moving, set the remaining movers to `any` and continue. (The cap can never
   fire given the lattice height; it exists so a future lattice extension
   degrades to "slow but correct" instead of looping.)
4. Any local still `⊥` at the end (declared but never assigned — impossible
   today, since zl has no declaration without assignment) becomes `any`.

Literals: a `T_NUMBER` whose text contains `.` (or `e`, if `design_floats.md`
§2.5 lands) is `float`; otherwise `int`. `T_STRING` is `str`, `true`/`false` are
`bool`, `[...]` is `list[unify of all elements]`.

### 5.2 Why the iteration is needed, not just one forward pass

```
fn dot(a: list[float], b: list[float]) -> float {
    total = 0                       # pass 1: int
    i = 0                           # pass 1: int
    while i < len(a) {
        total = total + a[i] * b[i] # rhs is float -> unify(int, float) = float
        i = i + 1                   # unify(int, int) = int
    }
    return total                    # float, matches declared -> float
}
```

With a single forward pass, `total` would be pinned `int` at line 2 and the loop
body would be a spurious type error on a program that is obviously correct. The
second pass promotes it to `float` and everything type-checks. `i` stays `int`
and the loop counter stays in a register. **This is why `int → float` widening
and the fixed point are both load-bearing, not decoration.**

### 5.3 Explicit non-behaviors

- **No flow-sensitive narrowing.** `if type(x) == "int" { ... }` does not refine
  `x` inside the branch. `x` stays `any`. That is a real feature and a real
  design (occurrence typing); it is not v1.
- **No inference across function boundaries.** An unannotated parameter is `any`
  even when every call site passes an `int`. Whole-program inference is exactly
  the HM-shaped thing §1.2 rules out.
- **A variable that unifies to `any` is not an error.** It is just slow, and the
  compiler may say so (§4.5). Silence is the default; noise is opt-in.

---

## 6. What the interpreter does with annotations

**Proposal: the interpreter CHECKS annotations and IGNORES them for
representation.** It keeps its boxed `Value` model exactly as it is today, and
at each annotated binding point (`x: T = e`, typed parameter bind, `return` from
a typed function) it performs a runtime tag check and errors on mismatch with
the same message the LLVM backend's `zl_unbox_*` produces.

The C backend (`compile.c`) does the same thing, by emitting a
`zl_check_type(v, "int", "x")` call before the corresponding store.

### 6.1 Why this specific choice keeps the three-engine test honest

There are three options and only one of them is safe:

| Option | What happens to a program that violates its annotations | Verdict |
|---|---|---|
| **Ignore** annotations in interp | runs fine on `interp.exe`, traps on `compilel` output | **The engines disagree, and the disagreement is invisible until someone compiles.** The test suite would go green on a broken program. Rejected. |
| **Use** annotations in interp (switch to unboxed storage) | agrees with LLVM, but via a *second independent implementation* of unboxing, in a different language, with its own bugs | Two implementations of one semantics is exactly how `design_floats.md` §1.1's `100/3` divergence happened. Rejected. |
| **Check, don't use** | same accept/reject set as the LLVM backend; one representation; the interpreter stays the simple reference | **Chosen.** |

"Check, don't use" gives the property the test suite actually needs: **all three
engines accept exactly the same set of programs and produce exactly the same
output for them.** The interpreter stays slow — that is fine, it is the reference
implementation, not the product.

It also means the annotation check is written twice at most (once over `Node*`
for `interp.c`/`compile.c`, once in `compilel.c`'s type pass), not three times,
and a discrepancy between them shows up as a `run_tests.ps1` failure rather than
as a silent wrong answer.

### 6.2 The Neutrality test harness

Because annotations are erasable by construction, the strongest test in the plan
is mechanical and cheap:

```
for each annotated test program P:
    P' = strip_annotations(P)          # a ~30-line zl or PowerShell filter
    assert interp(P)  == interp(P')
    assert compile(P) == compile(P')
    assert llvm(P)    == interp(P)
```

Any Neutrality violation — a changed division result, a widened literal, a
formatting difference — surfaces here as a diff instead of as a mystery in
someone's program six months later.

---

## 7. The self-hosting risk

`compiler.zl` must still reach a byte-identical fixpoint (`verify.ps1` check
1/2). Annotations are new syntax, so `compiler.zl`'s own lexer and parser must
at minimum **skip** them.

### 7.1 Exactly what `compiler.zl` needs (~20 lines, all additive)

1. **`:` needs nothing.** `next_token`'s final fallback is
   `spos = spos + 1; return ["SYM", c]` (l.113-114), so `:` already lexes as
   `["SYM", ":"]` today. Confirmed by inspection of l.108-114.
2. **`->` needs a two-char case.** The existing check (l.109) only covers
   `= ! > <` followed by `=`. Add: if `c == "-"` and `c2 == ">"`, consume two
   and return `["SYM", "->"]`. It must go **after** the comment (l.66) and
   string (l.91) branches, which it naturally does.
3. **New `fn skip_type()`** — consume a `NAME`; if the next token is `[`,
   consume it, recurse, expect `]`. ~10 lines.
4. **`parse_fn` (l.338-356)** — after each `params = push(params, text())`
   / `advance()` pair, add `if is_sym(":") { advance() skip_type() }`. After
   `expect(")")`, add `if is_sym("->") { advance() skip_type() }`.
5. **`parse_statement` (l.382-388)** — after `e = parse_expr()`, before the
   `if is_sym("=")` check, add `if is_sym(":") { advance() skip_type() }`.

The AST shape does not change — `["fn", name, params, body]` and
`["assign", name, value]` are emitted exactly as before — so
`compile_program` and every emitter below it are untouched.

### 7.2 Why the fixpoint is safe

`verify.ps1` compares **gen1 to gen2**, not to a stored hash, and says so in its
header comment: *"The hash legitimately changes whenever compiler.zl changes."*
So editing `compiler.zl` cannot break the fixpoint property by itself — it can
only break it if the edit makes the compiler non-deterministic or wrong.

A stronger, sharper gate is available and should be added for this change:

> **Skip-neutrality gate.** For every unannotated `.zl` file in the tree (all 54
> stdlib modules, all test programs, `compiler.zl` itself), the `out.c` produced
> by `compiler.zl` **before** the §7.1 edit must be byte-identical to the `out.c`
> produced **after** it.

This is provable in advance by inspection (`skip_type` is never called when no
`:`/`->` appears in code position) and checkable in one loop. It is a much
tighter statement than "the fixpoint still holds", and it is what actually
guarantees the stdlib is untouched.

### 7.3 Rules for the build

- **Do not annotate `compiler.zl` in Stages 1-6.** It stays fully dynamic. The
  skip logic exists so it can *read* annotated files, not so it can contain
  them.
- Annotating `compiler.zl`'s hot paths is Stage 8, optional, and gated on the
  fixpoint still holding afterward.
- `compiler.zl` only ever needs to *skip* types, never to check or use them. A
  type checker written in zl is a separate project (`design_types.md` §5.3
  sketches it) and is not on this critical path.

---

## 8. Staged implementation plan

Each stage is independently shippable and ends with `.\verify.ps1` green plus a
stage-specific gate. Smallest shippable slice first.

### Stage 0 — Lock the spec (no code)

Ratify §2 (spelling), §3 (type set + representation + operator table), §4.2/§4.3
(the boundary tables), §6 (interpreter checks). Write the Neutrality harness
(§6.2) and a golden file of annotated/unannotated program pairs.

**Verify:** `.\verify.ps1` green (nothing changed). Golden file frozen.

### Stage 1 — Syntax only, semantics nowhere

`lexer.c` (§2.5), `parser.h` + `parser.c` (§2.6), `compiler.zl` (§7.1).
Annotations parse, attach to free slots, and **nothing reads them**. Programs
with annotations run identically to their unannotated twins on all engines
because every engine ignores the new slots.

**Verify:** `.\verify.ps1` green. Plus: the skip-neutrality gate (§7.2) over all
54 stdlib modules. Plus: for every pair in the Stage-0 golden file,
`interp(P) == interp(P')`.

This stage is the highest-value/lowest-risk slice: it makes the syntax real
across the whole toolchain while being provably inert.

### Stage 2 — The type pass + the interpreter's checks

Build the inference/check pass (§3.4, §5) over `Node*`. Wire it into `interp.c`
and `compile.c` as a *check only* (§6). Add `--no-check` to skip the pass and
`--warn` to downgrade errors, per `design_types.md` §4.

**Verify:** `.\verify.ps1` green. Plus negative tests: each deliberately-violated
annotation produces the same error text on `interp.exe` and the C backend. Plus
the Neutrality harness passes for every valid program.

### Stage 3 — `compilel.c`: unboxed `int` and `bool`

The first speed win. `compilel.c` consumes the type pass; typed functions become
`define i64 @zl_f(i64 %a)`; typed locals become `alloca` (so `mem2reg`/SROA
promote them to registers) instead of the current globals. Everything outside
the numeric subset still exits with "not supported yet", exactly as today.

**Verify:** `.\verify.ps1` green (unchanged — `compilel` is not in it yet). Plus:
`llvm(P) == interp(P)` for the integer golden programs. Plus a benchmark:
annotated `fib(30)` through `compilel` vs `compile.exe`, number recorded in the
commit message.

### Stage 4 — `float`

`double` arithmetic, `int → float` widening (`sitofp`), `int / int -> float`,
and the canonical number formatter. **Depends on `design_floats.md` Stage 1** —
without one agreed float-to-string format, `llvm(P) == interp(P)` cannot hold
for anything fractional.

**Verify:** as Stage 3, extended to the float golden vectors from
`design_floats.md` Stage 0.

### Stage 5 — `any`, the boundary, and the runtime shim

Add the `zl_p_*` pointer shim layer to `runtime.c` (§3.1), the `zl_box_*` /
`zl_unbox_*` helpers, `zl_type_error`, and every B/U site from §4.2 and §4.3.
`compilel` output now links `runtime.c` + `os_win.c`, so mixed typed/untyped
programs compile end to end.

**Verify:** a program with a typed hot function called from untyped code
produces identical output on all three engines *and* on `compilel`. Plus: each
of U1-U5 has a negative test that traps with the right message.

### Stage 6 — `str` and `list[T]`

`%zlstr`, `%zllist`, the flat-buffer representation, the fast-builtin table
(§4.6), the O(n) conversion warning (§4.4), and the invariance error message for
`list[int]` → `list[any]`.

**Verify:** a typed transcription of `stdlib/stats.zl` (a copy — the original is
never edited) produces byte-identical output to the original under `interp.exe`,
and runs measurably faster through `compilel`.

### Stage 7 — Wire `compilel` into the gate

Add `compilel` as a fourth engine in `run_tests.ps1` for the programs it
supports, and add the Neutrality harness to `verify.ps1`.

**Verify:** `.\verify.ps1` green with four engines.

### Stage 8 (optional) — Annotate `compiler.zl`

Annotate the hot paths (`next_token`, `parse_expr`, the emitter's string
building) and measure the self-compile time.

**Verify:** the fixpoint still holds, byte-identically. If it does not, revert —
this stage is a nice-to-have and is never worth risking Floor 1 for.

---

## 9. What we are explicitly NOT doing

Cut to keep this implementable, and to honor the repo's "slow correct code is a
finished floor" discipline. Each of these is a defensible future project, not an
oversight.

| Not doing | Why |
|---|---|
| **Hindley-Milner / whole-program inference** | Rejects `jsonw.zl`, `json_parse.zl`, `lisp_interp.zl`, and `compiler.zl`'s own AST. §1.2. |
| **Inference across function boundaries** | Same reason, smaller scale. An unannotated parameter is `any`, full stop. |
| **Inference inside untyped function bodies** | Would speed up unannotated code for free, but doubles the surface area of Stage 3 and makes "is my function fast?" unanswerable from the signature. Revisit after Stage 7. |
| **Generics / type parameters** | `list[T]` is the only parametric type and `T` is always concrete at the use site. `fn map(xs: list[T], ...)` needs monomorphization or dictionaries — a whole compiler subsystem. |
| **Traits / interfaces / any subtyping beyond `int → float`** | No inheritance in the language, nothing to be polymorphic over. Invariance (§3.4) is deliberate. |
| **Union / optional / nullable types** | Every one of them requires flow-sensitive narrowing to be usable, and narrowing is its own design. `any` covers the need today. |
| **A `nil` type** | Without unions, a `nil`-typed slot holds exactly one value. Useless. Cut from `design_types.md`'s set. |
| **Occurrence typing** (`if type(x) == "int"` narrowing `x`) | The single most requested follow-up, and the right one to build second. Not v1. §5.3. |
| **Records / structs / typed fields** | The language has no record literal. Typing members waits for members to exist. |
| **Function types / first-class functions** | zl has no first-class function values at all. Nothing to type. |
| **Sized integers (`i32`, `u8`), unsigned types** | One `int` = `i64`. Sizes matter for FFI and packed data; neither exists yet. |
| **Overloading / type-directed dispatch** | Would make `int + str` resolve differently by annotation — a direct Neutrality violation. |
| **Type aliases (`type Name = ...`)** | Needs a reserved word or a convention. Defer until there is demand. |
| **Changing `type()`'s output** | It reports the runtime tag and must keep reporting `"num"` for both `int` and `float`, or every stdlib module that dispatches on it breaks. |
| **Making annotations mandatory anywhere, ever** | `MASTER_PLAN.md` §4.1: power > safety, not Rust. The wall is erected by the programmer, on the declaration they chose. |

---

## 10. Risks

| # | Risk | Mitigation |
|---|---|---|
| R1 | **`int` is `i64` but the interpreter is `double`** — divergence above 2^53 and on overflow. | Documented in §3.3; the same posture `run_tests.ps1` already takes on division. Test programs stay in range. Revisit only with `design_floats.md` §2.1. |
| R2 | **`int / int -> float` surprises people** who annotate an existing program and get a type error. | It is a *rejection with a message naming `int(...)`*, never a changed answer. The error text must literally suggest the fix. |
| R3 | **Two implementations of the type rules** (the `Node*` pass for interp/C, the pass in `compilel.c`) drift apart. | Share one pass over `Node*`; `compilel.c` consumes its output rather than re-deriving. If they must be separate, the Neutrality harness (§6.2) is what catches drift. |
| R4 | **The `->` token corrupts `brainfuck.zl`** and the 100+ other stdlib comments/strings containing `->`. | §2.2: the two-char check goes *inside `lex_symbol`*, which is only reached after the comment and string branches. Add a regression test that runs `brainfuck.zl` immediately after the lexer change. |
| R5 | **Boxing a `list[int]` inside a loop** silently costs O(n) mallocs per iteration. | §4.4: warn at every `list[T] → any` site; make `list[T]`/`list[any]` non-assignable so it can never happen implicitly. |
| R6 | **`Value`-by-value ABI** in hand-written IR is silently wrong on Win64. | §3.1: `any` is always `ptr`; add the `zl_p_*` shim rather than reproducing the struct ABI. |
| R7 | **`compiler.zl` edits break the fixpoint.** | §7.2: `verify.ps1` compares gen1 to gen2, and the skip-neutrality gate proves the stdlib output is unchanged. Do not annotate `compiler.zl` before Stage 8. |
| R8 | **Enum renumbering** — inserting `N_TYPE` mid-enum shifts values that `compilel.c`/`compilef.c` print in error messages. | §2.6: append at the end. |
| R9 | **Scope creep into a full checker.** `design_types.md`'s arity checks, operator checks, and diagnostics are tempting. | They are welcome, but they are Stage 2's *optional* second half. The critical path is representation, not diagnostics. |

---

## 11. Definition of done

1. `x: int = 5` and `fn f(a: int, b: float) -> int { }` lex, parse, and run on
   all three existing engines, with `struct Node` unchanged.
2. All 54 stdlib modules run **with zero edits** and, when compiled by
   `compiler.zl`, produce byte-identical `out.c` to before the change.
3. `compiler.zl` skips annotations and still reaches a byte-identical fixpoint.
4. The interpreter and the C backend **check** annotations and reject violations
   with the same message; neither changes its value representation.
5. `compilel.c` emits unboxed `i64` / `double` / `i1` for annotated code and
   boxed `ptr` for `any`, with every B/U site from §4.2 and §4.3 implemented.
6. **Annotation Neutrality holds**: for every program in the golden file,
   stripping the annotations changes no output on any engine.
7. A recorded benchmark shows annotated code through `compilel` beating the
   boxed C backend on `fib(30)`, with the number in the commit message.
8. `.\verify.ps1` is green.
