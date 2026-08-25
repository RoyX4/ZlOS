# Design Proposal: Optional Static Typing for zl

**Status:** proposal / not built
**Author:** design pass, 2026-07-29
**Scope:** a *type-checker pass* that runs between the parser and the backend (interpreter, C backend, or x86-64 backend). Additive and backward-compatible. Touches the parser (to carry annotations) and adds one new pass. Changes no runtime, no codegen, no existing program.

---

## 0. The one-line pitch

> Let a programmer *optionally* write down what type a variable, parameter, or function returns — and get those specific promises checked at compile time — without turning zl into a language that stops you from doing what you want.

Untyped zl keeps working, byte for byte. Typing is a thing you reach for, like the `!` danger marker: opt-in, per-declaration, never imposed.

---

## 1. Motivation

### 1.1 The pain is already real in this repo

`src/selfhost/compiler.zl` is ~700 lines of zl and the README lists two nasty bugs that self-hosting exposed (operator-vs-string, dynamic scoping). Both are the kind of thing a type-checker either catches or narrows. Today the language has **zero** compile-time feedback: every mistake is either a runtime surprise or, worse, a confusing error from the *C compiler* about generated code the author never wrote.

Concretely, three classes of error cost time right now:

1. **Wrong argument count.** User functions compile to fixed-arity C functions (`emit_function` in `src/selfhost/compiler.zl`). Call `emit_expr(node, ind)` when it takes one arg and you get a `cl` error about `zl_fn_emit_expr` — pointing at `out.c`, not your source.
2. **Operator/type nonsense.** `zl_binop` dispatches on runtime tags. `"hi" - 1` or `[1,2] * 3` produce nil or garbage at runtime with no warning.
3. **Typos in names.** A misspelled variable is just a fresh `nil` in the interpreter; a misspelled function is a link error in the C backend.

### 1.2 Why *optional*, specifically

§4.1 of `MASTER_PLAN.md` locks the language's identity: **power > safety**, and — explicitly — *not* Rust ("compile-time walls that stop you"). A mandatory type system would violate a locked decision. So the design constraint is sharp:

> Typing must be a tool the programmer *opts into on a specific declaration*, the same way `!` is a danger the programmer opts into. Unannotated code is never blocked, never nagged.

That framing makes typing philosophically consistent with the rest of zl rather than a bolted-on contradiction: `!` says *"I accept this risk"*; a type annotation says *"I want this promise checked."* Both are opt-in, both are rare-by-default, both keep the untyped fast path pristine.

### 1.3 Bonus: another self-hosting stress test

Writing the checker *in zl* (§4.5's spirit) exercises records-as-tagged-lists and recursion harder than anything so far, and the checker would immediately be run against `src/selfhost/compiler.zl` itself — a real dogfooding loop.

---

## 2. Proposed syntax

### 2.1 The core rule: no new keywords, no new tokens

§4.4 locks the reserved-word list at eleven and says every added word is a name users lose forever. **This proposal adds zero reserved words.** Type names (`num`, `str`, `bool`, `list`, `any`, `nil`) are ordinary identifiers — exactly like built-ins (`print`, `dir`, `poke`) are ordinary identifiers per §4.3.

It also adds **zero new lexer tokens**. The annotation punctuation — `:`, `[`, `]` — already lexes today: the parser uses `[` `]` for lists, and `:` falls through `next_token`'s single-char `["SYM", c]` case for free. Verified against `src/selfhost/compiler.zl`'s lexer: `:` produces `["SYM", ":"]` with no change.

### 2.2 Variable annotation — `name: type = value`

```
count: num = 0
label: str = "hits: "
names: list = ["Zac", "Sam"]
flag: bool = false
```

The `: type` is optional. `count = 0` (untyped) and `count: num = 0` (typed) are both legal; the second asks the checker to enforce that `count` stays a `num`.

### 2.3 Function annotation — params and return

Parameters take the same `name: type` form. The return type follows the parameter list with `: type` before the block:

```
fn double(n: num): num {
    return n * 2
}

fn greet(who: str): str {
    return "hi " + who
}

fn log(msg: str): nil {
    print(msg)
}
```

Annotations are per-slot and independent. All of these are legal:

```
fn f(n) { ... }                 # fully untyped — unchanged from today
fn f(n: num) { ... }            # typed param, unstated return
fn f(n): num { ... }            # untyped param, typed return
fn f(a: num, b): str { ... }    # mix and match
```

> **Style note.** Colon-before-block (`): num {`) is chosen over an arrow (`-> num`) precisely because `:` already lexes and `->` would cost a new two-char token (only `==`, `!=`, `>=`, `<=` are multi-char today). If a later aesthetic pass wants `->`, it is a one-line lexer addition — but it is not needed and is out of scope here.

### 2.4 Type grammar

```
type   := name                       # num | str | bool | list | any | nil
        | "list" "[" type "]"        # list[num], list[str]  (Phase 2)
```

The base type names and their meaning:

| Type   | Means                          | Backs onto runtime |
|--------|--------------------------------|--------------------|
| `num`  | number (int or float)          | `zl_num`           |
| `str`  | string                         | `zl_str`           |
| `bool` | `true` / `false`               | `zl_bool`          |
| `list` | a list (elements `any` in v1)  | `zl_list`          |
| `nil`  | the empty/no-value type        | `zl_nil`           |
| `any`  | opt-out / unknown — the escape | (whatever it is)   |

`any` is the whole reason this stays optional; see §3.2.

---

## 3. Semantics

Deliberately the simplest system that catches §1.1's real bugs. No subtyping, no generics (v1), no inference beyond literals. KISS on purpose.

### 3.1 Every unannotated thing is `any`

An unannotated variable, parameter, return, or built-in result has type `any`. This is what makes the feature *gradual*: `examples/hello.zl
R100
R100` and `src/selfhost/compiler.zl` type-check today with no edits and no diagnostics, because everything in them is `any`.

### 3.2 The single assignability rule

`A` is assignable to `B` if and only if:

```
A == B     OR     A is any     OR     B is any
```

That's the entire compatibility relation. `any` is compatible with everything in both directions, so annotated and unannotated code interleave freely and no annotation can ever "infect" untyped code with errors. There is no numeric widening, no coercion — predictable over clever.

### 3.3 Literal and expression typing

| Expression                | Inferred type |
|---------------------------|---------------|
| `5`, `3.14`               | `num`         |
| `"hi"`                    | `str`         |
| `true`, `false`           | `bool`        |
| `[ ... ]`                 | `list`        |
| a name                    | its env type (declared, or `any`) |
| `-x`                      | `num` (operand must be `num`/`any`) |
| `not x`                   | `bool`        |

Operators (checked only when neither operand is `any`):

| Operator          | Signature                                   |
|-------------------|---------------------------------------------|
| `+`               | `num,num -> num`  **or**  `str,str -> str`  |
| `- * / %`         | `num,num -> num`                            |
| `== != < <= > >=` | `T,T -> bool`                               |
| `and` `or`        | `bool,bool -> bool`                         |

`+` intentionally carries both the arithmetic and the string-join meaning, matching how `zl_binop` already behaves. `"hi" - 1` is `str,num` under `-` → **error**. `"a" + "b"` and `1 + 2` are fine; `"a" + 1` (both annotated) is an error.

### 3.4 Statements

- **Typed assignment** `x: T = e`: `typeof(e)` must be assignable to `T`; `x` is bound to `T` in the environment. A later `x = e2` (re-assign) must keep `e2` assignable to `T`.
- **Untyped assignment** `x = e`: binds `x` to `any` (v1 does no flow inference — see §6 non-goals). Never an error.
- **Call** `f(args...)`:
  - If `f` is a known user function or a known built-in with a signature: **arity must match**, and each arg assignable to the corresponding param type. Call's type = the declared return type (or `any`).
  - If `f` is unknown (arbitrary built-in / `any`): treated as variadic `any -> any`. No arity check, no arg check. This is what stops false positives on `print`, `poke`, `window`, etc.
- **Return** `return e` inside a function with a declared return `R`: `typeof(e)` must be assignable to `R`. `return` with no value → `nil`. If the function declared no return type, returns are unchecked.
- **`if` / `while`**: condition may be any type (zl is truthiness-based; §4 keeps `if 5 { }` legal). No condition-type error — that would be a wall, not a promise.
- **`for x in seq`**: if `seq` is annotated `list` (or `list[T]`), bind `x` to element type (`T` or `any`); otherwise `x` is `any`.

### 3.5 Built-in signature table (starter set)

A small table lets the common built-ins participate without over-firing. Everything not listed is `any -> any` variadic.

```
print : (any...)     -> nil       # variadic — never arity-checked
len   : (any)        -> num
push  : (list, any)  -> list
at    : (str, num)   -> str
read  : (str)        -> str
write : (str, str)   -> nil
num   : (str)        -> num       # string-to-number built-in
```

(The `num` *built-in* and the `num` *type name* share a spelling harmlessly — one is a callee position, one is a type position; the parser already distinguishes them by context.)

### 3.6 What it catches — worked example

```
fn area(w: num, h: num): num {
    return w + h          # ← WARN/ERROR: returns a sum but logically fine;
}                         #   type-correct (num,num->num). Not flagged.

r: str = area(3, 4)       # ← ERROR: num not assignable to str
box = area(3)             # ← ERROR: area expects 2 args, got 1
tag: str = "n=" - 1       # ← ERROR: str,num under '-'
print(box, r, tag)        # OK: print is variadic any
```

Four of the exact failure modes from §1.1, caught at check time with source-level messages, before a single line of `out.c` is generated.

---

## 4. How it stays optional (the load-bearing section)

Three independent layers, any one of which is enough to keep untyped users unaffected:

1. **Per-declaration opt-in.** No annotation → `any` → never checked (§3.1, §3.2). You pay only for what you annotate.
2. **`any` as a first-class escape.** Even in annotated code, `x: any = whatever` disables checking for that binding. There is always a way to say "don't check this."
3. **A pass you can skip.** The checker is a *separate stage*. Compiler flags:
   - default: run the checker; a violated annotation **fails the build** (you signed a contract by annotating).
   - `--warn`: downgrade all type errors to warnings and compile anyway (full power-mode, honoring §4.1 — nothing stops you).
   - `--no-check`: skip the pass entirely.

The default *does* fail on a violated annotation, and that is consistent with §4.1, not a contradiction: the wall was erected **by the programmer, on the specific declaration they chose**, exactly like `!`. Rust's walls are the anti-pattern because they are imposed on everyone everywhere. These are chosen, local, and removable with `any` or `--warn`.

---

## 5. Implementation sketch

### 5.1 Where it slots in

`src/selfhost/compiler.zl`'s `main` today is:

```
prog   = parse_program(input)
result = compile_program(prog)
```

It becomes:

```
prog   = parse_program(input)
errs   = check(prog)              # NEW pass
report(errs)                      # print diagnostics; exit if strict and non-empty
result = compile_program(prog)    # unchanged
```

The C-side tools (`src/runtime/interp.c`, `src/backends/c/compile.c`) get the mirror-image pass over `Node*` (a `check.c`) called in the same spot. The zl version is the one that matters for self-hosting; the C version keeps the interpreter and C backend honest during bootstrap.

### 5.2 Parser changes (small, backward-compatible)

The AST is tagged lists; **appending** an optional slot never breaks existing consumers (they read fixed indices).

- **Assignment.** In `parse_statement`, after the LHS name, if the next token is `["SYM", ":"]`, parse a type and emit `["assign", name, value, TYPE]`. Existing `emit_stmt` reads `node[1]`/`node[2]` and ignores the new `node[3]`. Untyped assignments stay 3-element — unchanged.
- **Functions.** In `parse_fn`, allow `: type` after each param name and after the `)`. Keep `params` as the same flat name-list the codegen already iterates, and return `["fn", name, params, body, ptypes, rettype]`. `emit_function` reads `[1..3]` and ignores `[4]`/`[5]` — codegen untouched.
- **Type parsing.** A one-function `parse_type()`: read a NAME; if followed by `[`, recurse and expect `]`. ~10 lines.

Backward-compat is provable by inspection: every new element is at an index the current codegen never reads.

### 5.3 The checker (in zl, matching existing style)

The codebase already uses **parallel lists + linear scan** for scoping (`g_funcs`, `g_globals`, `is_func`, `is_global`). The checker reuses that pattern rather than introducing maps:

```
g_errors = []                     # collected diagnostics
env_names = []                    # variable name -> type, parallel lists
env_types = []

fn env_lookup(name) { ... }       # linear scan, returns "any" if absent
fn env_bind(name, ty) { ... }

fn type_of(node) {                # expression -> type string
    tag = node[0]
    if tag == "num"  { return "num" }
    if tag == "str"  { return "str" }
    if tag == "bool" { return "bool" }
    if tag == "list" { return "list" }
    if tag == "name" { return env_lookup(node[1]) }
    if tag == "bin"  { return check_bin(node) }
    if tag == "call" { return check_call(node) }
    ...
}

fn assignable(a, b) {             # the §3.2 rule
    return a == b or a == "any" or b == "any"
}

fn check_stmt(node) { ... }       # walks, extends env, pushes to g_errors
fn check(prog) {
    collect_fn_signatures(prog)   # first pass: name -> (ptypes, rettype)
    for it in prog[1] { check_stmt(it) }
    return g_errors
}
```

Two-pass so functions can be called before they're defined (mirrors how `compile_program` pre-scans `g_funcs`). Signatures for user functions come from the parser's `ptypes`/`rettype` slots; built-ins come from the §3.5 table (itself just parallel lists).

Estimated size: **~150–200 lines of zl**, no new runtime primitives required — it uses only lists, strings, `==`, recursion, and `for`, all of which already self-host.

### 5.4 Diagnostics

Each error is a string with the offending name/op and the two types, e.g.:

```
type error: 'area' expects 2 args, got 1
type error: cannot assign num to 'r' declared str
type error: operator '-' wants num,num, got str,num
```

Line numbers require threading `Token.line` (already on every C token; the zl token is `[kind, text]` and would need a third slot `[kind, text, line]` to match — a clean, separate, optional improvement, not required for v1).

---

## 6. Non-goals (YAGNI — explicitly cut from v1)

Kept out to stay implementable and to honor §3 of the coding rules (simplicity, no speculative generality):

- **Flow-sensitive inference** — untyped `x = 5` does *not* infer `num`. It stays `any`. Inference is a large, separate design and not needed to catch §1.1's bugs.
- **Generics / type parameters** beyond the single `list[T]` sugar (which is itself Phase 2).
- **Union / nullable types**, `nil`-safety analysis.
- **Records/structs typing** — the language has no record literal yet (`f.age` member access exists but records aren't declared); type-checking members waits for records to be a real feature.
- **`.` member and `[]` index type-checking** — light, deferrable to Phase 2.
- **Exhaustive missing-return analysis** — codegen already appends `return zl_nil()`, so falling off the end is defined behavior; flagging it is a nicety, not a correctness need.

---

## 7. Phasing

| Phase | Delivers | Catches |
|-------|----------|---------|
| **1** | parser slots + `check()` in zl; base types; assignability; operator + call/arity + return checks; `--warn`/`--no-check` | §1.1's three bug classes |
| **2** | `list[T]`, `for`-element typing, `.`/`[]` checks, token line numbers for messages | element-type and indexing mistakes; better diagnostics |
| **3** | mirror `check.c` for interpreter/C-backend parity; run checker as a gate in `run_tests.ps1` | regressions; keeps all three engines honest |

Phase 1 alone is a complete, shippable improvement and can be demoed by pointing the checker at a deliberately-broken copy of `src/selfhost/compiler.zl`.

---

## 8. Open questions (decide when building)

- **Default severity.** Ship default-fail-on-typed-error (proposed §4), or default-`--warn` and let strictness be opt-in via a flag? Leaning fail-by-default because annotating *is* the opt-in, but this is the one genuinely debatable knob.
- **`num` splitting into `int`/`float`.** The lexer currently only lexes integer digits; the runtime is one `zl_num`. Keep the single `num` until floats are real, then refine. (Matches the §10 OPEN item "durations/hex/floats — decide when the parser needs them.")
- **Type aliases** (`type Name = ...`) — would want a reserved word or an identifier convention; defer until there's demand.

---

## 9. Fit against locked decisions

| Locked decision (`MASTER_PLAN.md`) | This proposal |
|------------------------------------|---------------|
| §4.1 power > safety, *not* Rust-style walls | Walls are opt-in per-declaration and removable (`any`, `--warn`). Untyped code is never walled. ✅ |
| §4.4 eleven reserved words, no more | Zero new reserved words; type names are identifiers. ✅ |
| §4.3 built-ins are identifiers, not keywords | Same treatment for type names. ✅ |
| §4.2 syntax locked, no re-litigation | Purely additive; no existing syntax changes meaning. `x = 5` still parses and runs identically. ✅ |
| `!` = the one novel opt-in-danger idea | Typing is its mirror: opt-in-*guarantee*. Same philosophy, different direction. ✅ |

The proposal is a strict superset: every existing zl program is still a valid zl program with identical behavior.
